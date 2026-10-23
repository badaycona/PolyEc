"""Bounded CPU recurrent PPO; core simulation lives in polyec_env.pyd."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import time
import numpy as np
import psutil
import torch
import polyec_env as core
from realm_model import Policy, ActorExport, masked_distribution, OBS, ACTIONS, HIDDEN

torch.set_num_threads(2)
torch.set_num_interop_threads(1)
def training_sequence(policy,obs,hidden,resets):
    if not bool(resets.any()):return policy.sequence(obs,hidden)
    logits=[];values=[]
    for t in range(obs.shape[1]):
        hidden=hidden*(~resets[:,t]).float().unsqueeze(-1)
        l,v,hidden=policy(obs[:,t],hidden);logits.append(l);values.append(v)
    return torch.stack(logits,1),torch.stack(values,1),hidden

ROLE=0
class Adapter:
    def __init__(self,n,seed,regions,horizon):
        self.env=core.RealmBatch(n,seed,ROLE,regions,512 if ROLE==1 else 128,4)
    def observe(self): return self.env.observe()
    def step(self,a,auto_reset=True): return self.env.step(a,auto_reset)

def evaluate(policy,start,count,mode="learned",random_seed=818):
    env=Adapter(count,start,6,64);obs,masks=env.observe();hidden=torch.zeros(count,HIDDEN)
    totals=np.zeros(count,dtype=np.float64);counts=np.zeros(ACTIONS,dtype=np.int64);active=np.ones(count,dtype=bool)
    rng=np.random.default_rng(random_seed)
    for t in range(512 if ROLE==1 else 128):
        if mode in ("learned","initial"):
            with torch.no_grad():
                logits,_,hidden=policy(torch.from_numpy(obs),hidden)
                actions=logits.masked_fill(torch.from_numpy(masks)<.5,-1e9).argmax(-1).numpy()
        elif mode=="hold":actions=np.zeros(count,dtype=np.int64)
        elif mode=="random":actions=np.array([rng.choice(np.flatnonzero(m)) for m in masks],dtype=np.int64)
        else:
            # Labelled test controller. It receives only public actor observations.
            scores=np.zeros((count,ACTIONS),dtype=np.float32)
            if ROLE==0:scores[:,2]=.2;scores[:,4]=2;scores[:,5]=1.5;scores[:,6:13]=np.where((obs[:,99]<.9)[:,None],4,0)
            elif ROLE==1:
                f=obs[:,128:].reshape(count,ACTIONS,12)
                scores[:,1:8]=1-f[:,1:8,8]
                scores[:,8:15]=f[:,8:15,8]-1;scores[:,30]=.3*(1+np.floor(obs[:,4]*4))
            else:
                scores[:,1:8]=1;scores[:,15]=np.where(obs[:,1]<.18,3,.5);scores[:,16]=np.where(obs[:,3]<.6,4,0)
            actions=np.where(masks>.5,scores,-1e9).argmax(-1).astype(np.int64)
        assert np.all(masks[np.arange(count),actions]>.5)
        counts+=np.bincount(actions[active],minlength=ACTIONS)
        obs,masks,reward,done,score=env.step(actions,False);totals+=reward;active=~done
        if done.all():break
    assert done.all() and np.allclose(totals,score,atol=1e-4)
    return {"mean_profit":float(score.mean()),"profits":score.tolist(),"hold_rate":float(counts[0]/counts.sum()),"action_counts":counts.tolist(),"invalid_actions":0,"metric":"normalized actor utility delta / abs(opening utility); NOT coin profit","outcomes":list(env.env.outcomes())}

def fit(seed, args, output):
    torch.manual_seed(seed)
    np_rng = np.random.default_rng(seed)
    policy = Policy()
    if args.resume_dir:
        policy.load_state_dict(torch.load(args.resume_dir/f"seed-{seed}.pt",weights_only=True))
    torch.nn.init.zeros_(policy.critic.weight);torch.nn.init.zeros_(policy.critic.bias)
    torch.save(policy.state_dict(), output / f"initial-{seed}.pt")
    optimizer = torch.optim.Adam(policy.parameters(), lr=3e-4, eps=1e-5)
    started = time.perf_counter()
    best = -float("inf")
    history = []
    n, t = args.envs, 64
    regions = 0
    for update in range(args.updates):
        next_regions = 1 if update < 6 else (2 if update < 24 else 6)
        if next_regions != regions:
            regions = next_regions
            # Training seed domain is strictly above 2**32; eval seeds are low.
            env = Adapter(n, 2**32 + args.domain*10**9 + seed*10**8 + update*100000, regions, 64)
            obs, masks = env.observe()
            hidden = torch.zeros(n, HIDDEN)
        policy.train()
        observations=[]; mask_history=[]; memories=[]; acts=[]; old_logs=[]; values=[]; rewards=[]; dones=[]
        episode_profits=[]
        with torch.no_grad():
            for step in range(t):
                o=torch.from_numpy(obs); m=torch.from_numpy(masks)
                observations.append(o); mask_history.append(m); memories.append(hidden)
                logits, value, next_hidden = policy(o, hidden)
                distribution=masked_distribution(logits,m)
                action=distribution.sample()
                acts.append(action); old_logs.append(distribution.log_prob(action)); values.append(value)
                obs,masks,reward,done,profit=env.step(action.numpy())
                rewards.append(torch.from_numpy(reward)); dones.append(torch.from_numpy(done))
                hidden=next_hidden*(~torch.from_numpy(done)).float().unsqueeze(-1)
                if done.any(): episode_profits.extend(profit[done].tolist())
            _, bootstrap, _ = policy(torch.from_numpy(obs), hidden)
        reward_tensor=torch.stack(rewards); value_tensor=torch.stack(values); done_tensor=torch.stack(dones)
        advantage=torch.zeros_like(reward_tensor); last=torch.zeros(n)
        for step in reversed(range(t)):
            next_value=bootstrap if step==t-1 else value_tensor[step+1]
            live=(~done_tensor[step]).float()
            delta=reward_tensor[step]+next_value*live-value_tensor[step]
            last=delta+.95*live*last
            advantage[step]=last
        returns=advantage+value_tensor
        advantage=(advantage-advantage.mean())/(advantage.std()+1e-8)
        #16-decision BPTT. Reset masks also isolate deaths within a chunk.
        def chunks(tensor):
            shape=tensor.shape[2:]
            return tensor.transpose(0,1).reshape(n*4,16,*shape)
        o_all=chunks(torch.stack(observations)); m_all=chunks(torch.stack(mask_history))
        a_all=chunks(torch.stack(acts)); lp_all=chunks(torch.stack(old_logs))
        adv_all=chunks(advantage); ret_all=chunks(returns)
        h_all=torch.stack(memories)[::16].transpose(0,1).reshape(n*4,HIDDEN)
        resets_all=chunks(torch.cat((torch.zeros_like(done_tensor[:1]),done_tensor[:-1])))
        losses=[]; entropies=[]
        for epoch in range(3):
            for indices in np_rng.permutation(n*4).reshape(-1,64):
                logits,value,_=training_sequence(policy,o_all[indices],h_all[indices],resets_all[indices])
                dist=masked_distribution(logits,m_all[indices])
                ratio=(dist.log_prob(a_all[indices])-lp_all[indices]).exp()
                adv=adv_all[indices]
                policy_loss=-torch.minimum(ratio*adv,ratio.clamp(.8,1.2)*adv).mean()
                value_loss=(value-ret_all[indices]).square().mean()
                entropy=dist.entropy().mean()
                loss=policy_loss+.5*value_loss-.01*entropy
                optimizer.zero_grad(set_to_none=True); loss.backward()
                torch.nn.utils.clip_grad_norm_(policy.parameters(),.5); optimizer.step()
                losses.append(float(loss.detach())); entropies.append(float(entropy.detach()))
        elapsed=time.perf_counter()-started
        row={"seed":seed,"update":update+1,"regions":regions,"decisions":(update+1)*n*t,
             "seconds":elapsed,"episodes_finished":len(episode_profits),"train_mean_profit":float(np.mean(episode_profits)) if episode_profits else 0.0,
             "loss":float(np.mean(losses)),"entropy":float(np.mean(entropies)),
             "peak_rss_bytes":int(psutil.Process().memory_info().peak_wset)}
        if (update+1)%20==0 or update==args.updates-1 or elapsed>args.seconds:
            validation=evaluate(policy,14000,32)
            row["validation_mean_profit"]=validation["mean_profit"]
            if regions==6 and validation["mean_profit"]>best:
                best=validation["mean_profit"]
                torch.save(policy.state_dict(),output/f"seed-{seed}.pt")
                (output/f"seed-{seed}-selection.json").write_text(json.dumps({**row,"validation":validation},indent=2))
            print(json.dumps(row),flush=True)
        history.append(row)
        if elapsed>args.seconds and regions==6: break
    (output/f"seed-{seed}-history.json").write_text(json.dumps(history,indent=2))
    return {"seed":seed,"best_validation_profit":best,"seconds":time.perf_counter()-started,
            "decisions":history[-1]["decisions"],"peak_rss_bytes":history[-1]["peak_rss_bytes"]}

def paired_ci(difference):
    d=np.array(difference,dtype=np.float64)
    rng=np.random.default_rng(491)
    boot=d[rng.integers(0,len(d),(5000,len(d)))].mean(1)
    return {"mean":float(d.mean()),"ci95":[float(x) for x in np.quantile(boot,[.025,.975])]}

def main():
    global ROLE
    p=argparse.ArgumentParser();p.add_argument('--schema',default='realm-v6');p.add_argument('--role',type=int,required=True);p.add_argument('--output',type=Path,required=True)
    p.add_argument('--updates',type=int,default=240);p.add_argument('--envs',type=int,default=32);p.add_argument('--seconds',type=int,default=300)
    p.add_argument('--resume-dir',type=Path);p.add_argument('--domain',type=int,default=4);p.add_argument('--test-start',type=int,default=160000)
    args=p.parse_args();ROLE=args.role;args.output.mkdir(parents=True,exist_ok=True)
    family=['governance','economic','civilian'][ROLE]
    model_contract=core.Realm(1,6,128).model_contract
    config={'warm_start':str(args.resume_dir) if args.resume_dir else None,'imitation':False,'family':family,'schema':args.schema,'model_contract':model_contract,'reward':'normalized-utility-potential-v6','partners':'civilian Work/Study; governance/economic uniform legal, training-only','algorithm':'recurrent PPO gamma1 GAE.95','lr':3e-4,'optimizer':'Adam eps1e-5 reset on warm start','clip':.2,'epochs':3,'entropy_coef':.01,'rollout_steps':64,'bptt_steps':16,'envs':args.envs,'torch':torch.__version__,'metric':'(terminal utility-opening utility)/max(1,abs(opening utility))','death_terminal':True,'critic_reset_on_warm_start':True,'horizon_decisions':512 if ROLE==1 else 128,'world_workers':4,'noninferiority_margin':.05,'seeds':[1,2,3],'train_seed_min':2**32,'validation':[14000,14031],'test':[args.test_start,args.test_start+99],'budget_seconds_per_seed':args.seconds,'updates':args.updates,'decision_days':7 if ROLE==1 else 30,'gate':'baseline noninferiority margin0.05 normalized utility; initial improvement lowerCI>0; full R2/R3 separate'}
    (args.output/'config.json').write_text(json.dumps(config,indent=2))
    results=[fit(seed,args,args.output) for seed in (1,2,3)]
    winner=max(results,key=lambda r:r['best_validation_profit'])['seed']
    (args.output/'SELECTION-LOCK.json').write_text(json.dumps({'seed':winner,'training':results,'selection':'validation only'},indent=2))
    evaluations={}
    for seed in (1,2,3):
        policy=Policy();policy.load_state_dict(torch.load(args.output/f'seed-{seed}.pt',weights_only=True));evaluations[f'learned_seed_{seed}']=evaluate(policy,args.test_start,100)
        if seed==winner:chosen=policy
    initial=Policy();initial.load_state_dict(torch.load(args.output/f'initial-{winner}.pt',weights_only=True))
    evaluations['initial']=evaluate(initial,args.test_start,100,'initial')
    for mode in ('hold','random','baseline'):evaluations[mode]=evaluate(chosen,args.test_start,100,mode)
    selected=evaluations[f'learned_seed_{winner}'];comparisons={name:paired_ci(np.array(selected['profits'])-np.array(evaluations[name]['profits'])) for name in ('initial','hold','random','baseline')}
    report={'family':family,'selected_seed':winner,'training':results,'evaluation':evaluations,'paired_utility_improvement':comparisons,'initial_improvement':comparisons['initial']['ci95'][0]>0,'baseline_noninferiority':comparisons['baseline']['ci95'][0]>=-.05,'full_R1_pass':False,'note':'integrated single focal actor training, full multi-policy co-play and R2/R3 still require evaluation'}
    (args.output/'evaluation.json').write_text(json.dumps(report,indent=2))
    chosen.eval();torch.onnx.export(ActorExport(chosen),(torch.zeros(1,OBS),torch.zeros(1,HIDDEN)),str(args.output/f'{family}.onnx'),input_names=['obs','hidden'],output_names=['logits','next_hidden'],dynamic_axes={name:{0:'batch'} for name in ('obs','hidden','logits','next_hidden')},opset_version=17,dynamo=False)
    manifest={**config,'sha256':hashlib.sha256((args.output/f'{family}.onnx').read_bytes()).hexdigest(),'selected_seed':winner,'trained_with_rl':True,'imitation':False,'observation':OBS,'actions':ACTIONS,'hidden':64,'normalizer':'clamp[-10,10]','full_R1_pass':False,'quality':{'initial_improvement':report['initial_improvement'],'baseline_noninferiority':report['baseline_noninferiority']}}
    (args.output/'manifest.json').write_text(json.dumps(manifest,indent=2));print('FINAL '+json.dumps({'family':family,'normalized_utility':selected['mean_profit'],'paired':comparisons}),flush=True)
if __name__=='__main__':main()
