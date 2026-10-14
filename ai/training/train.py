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
from model import Policy, ActorExport, masked_distribution, OBS, ACTIONS, HIDDEN

torch.set_num_threads(2)
torch.set_num_interop_threads(1)
REFERENCE = np.array([20, 14, 35, 45, 18, 65, 90], dtype=np.float32)

def evaluate(policy, start, count, mode="learned", random_seed=818):
    env = core.BatchEnv(count, start, 6, 64)
    obs, masks = env.observe()
    hidden = torch.zeros(count, HIDDEN)
    action_counts = np.zeros(ACTIONS, dtype=np.int64)
    totals = np.zeros(count, dtype=np.float64)
    rngs = [np.random.default_rng(random_seed + start + i) for i in range(count)]
    policy.eval()
    for t in range(64):
        if mode in ("learned", "initial"):
            with torch.no_grad():
                logits, _, hidden = policy(torch.from_numpy(obs), hidden)
                actions = logits.masked_fill(torch.from_numpy(masks) < .5, -1e9).argmax(-1).numpy()
        elif mode == "hold":
            actions = np.zeros(count, dtype=np.int64)
        elif mode == "random":
            actions = np.array([r.choice(np.flatnonzero(m)) for r, m in zip(rngs, masks)], dtype=np.int64)
        else:  # labelled development baseline only, never exported
            f = obs[:, 100:].reshape(count, ACTIONS, 10)
            score = np.zeros((count, ACTIONS), dtype=np.float32)
            score[:, 1:8] = (1-f[:, 1:8, 4])*REFERENCE*f[:, 1:8, 5]*4/100
            score[:, 8:15] = (f[:, 8:15, 4]-1)*REFERENCE*f[:, 8:15, 5]*4/100
            score[:, 15:] = f[:, 15:, 8]-f[:, 15:, 9]
            actions = np.where(masks > .5, score, -1e9).argmax(-1).astype(np.int64)
        assert np.all(masks[np.arange(count), actions] > .5)
        action_counts += np.bincount(actions, minlength=ACTIONS)
        obs, masks, reward, done, profits = env.step(actions)
        totals += reward
    assert done.all()
    assert np.allclose(totals*100, profits, atol=.02), "reward does not telescope to actual terminal profit"
    return {"mean_profit": float(profits.mean()), "profits": profits.tolist(),
            "hold_rate": float(action_counts[0]/action_counts.sum()),
            "trades": int(action_counts[1:15].sum()), "travels": int(action_counts[15:].sum()),
            "action_counts": action_counts.tolist(), "invalid_actions": 0}

def fit(seed, args, output):
    torch.manual_seed(seed)
    np_rng = np.random.default_rng(seed)
    policy = Policy()
    if args.resume_dir:
        policy.load_state_dict(torch.load(args.resume_dir/f"seed-{seed}.pt",weights_only=True))
    torch.save(policy.state_dict(), output / f"initial-{seed}.pt")
    optimizer = torch.optim.Adam(policy.parameters(), lr=3e-4, eps=1e-5)
    started = time.perf_counter()
    best = -float("inf")
    history = []
    n, t = args.envs, 64
    regions = 0
    for update in range(args.updates):
        next_regions = 6 if args.resume_dir else (1 if update < 6 else (2 if update < 24 else 6))
        if next_regions != regions:
            regions = next_regions
            # Training seed domain is strictly above 2**32; eval seeds are low.
            env = core.BatchEnv(n, 2**32 + args.domain*10**9 + seed*10**8 + update*100000, regions, 64)
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
        #16-decision BPTT sequences. Episodes end exactly at64, never inside a chunk.
        def chunks(tensor):
            shape=tensor.shape[2:]
            return tensor.transpose(0,1).reshape(n*4,16,*shape)
        o_all=chunks(torch.stack(observations)); m_all=chunks(torch.stack(mask_history))
        a_all=chunks(torch.stack(acts)); lp_all=chunks(torch.stack(old_logs))
        adv_all=chunks(advantage); ret_all=chunks(returns)
        h_all=torch.stack(memories)[::16].transpose(0,1).reshape(n*4,HIDDEN)
        losses=[]; entropies=[]
        for epoch in range(3):
            for indices in np_rng.permutation(n*4).reshape(-1,64):
                logits,value,_=policy.sequence(o_all[indices],h_all[indices])
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
             "seconds":elapsed,"train_mean_profit":float(np.mean(episode_profits)),
             "loss":float(np.mean(losses)),"entropy":float(np.mean(entropies)),
             "peak_rss_bytes":int(psutil.Process().memory_info().peak_wset)}
        if (update+1)%20==0 or update==args.updates-1 or elapsed>args.seconds:
            validation=evaluate(policy,10000,32)
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
    parser=argparse.ArgumentParser()
    parser.add_argument("--output",type=Path,required=True)
    parser.add_argument("--updates",type=int,default=240)
    parser.add_argument("--envs",type=int,default=64)
    parser.add_argument("--seconds",type=int,default=600)
    parser.add_argument("--resume-dir",type=Path)
    parser.add_argument("--test-start",type=int,default=20000)
    parser.add_argument("--domain",type=int,default=0)
    args=parser.parse_args(); args.output.mkdir(parents=True,exist_ok=True)
    if args.envs%16: raise ValueError("env count must be divisible by16")
    config={"algorithm":"recurrent PPO,16-step BPTT","gamma":1.0,"gae_lambda":.95,
            "epochs":3,"clip":.2,"lr":3e-4,"entropy_coef":.01,"seeds":[1,2,3],
            "train_seed_domain":">=2**32","validation_seeds":[10000,10031],
            "test_seeds":[args.test_start,args.test_start+99],"schema":core.SCHEMA,"reward":core.REWARD,
            "torch":torch.__version__,"arguments":{k:str(v) if isinstance(v,Path) else v for k,v in vars(args).items()}}
    (args.output/"config.json").write_text(json.dumps(config,indent=2))
    results=[fit(seed,args,args.output) for seed in (1,2,3)]
    winner=max(results,key=lambda r:r["best_validation_profit"])["seed"]
    # Selection is frozen from validation before test is ever touched.
    (args.output/"SELECTION-LOCK.json").write_text(json.dumps({"seed":winner,"reason":"highest validation mean terminal cash profit","training":results},indent=2))
    evaluations={}
    for seed in (1,2,3):
        policy=Policy(); policy.load_state_dict(torch.load(args.output/f"seed-{seed}.pt",weights_only=True))
        evaluations[f"learned_seed_{seed}"]=evaluate(policy,args.test_start,100)
        if seed==winner:
            chosen=policy
            initial=Policy();initial.load_state_dict(torch.load(args.output/f"initial-{seed}.pt",weights_only=True))
    evaluations["initial"]=evaluate(initial,args.test_start,100,mode="initial")
    for baseline in ("hold","random","greedy"):
        evaluations[baseline]=evaluate(chosen,args.test_start,100,mode=baseline)
    learned=evaluations[f"learned_seed_{winner}"]
    comparisons={name:paired_ci(np.array(learned["profits"])-np.array(evaluations[name]["profits"])) for name in ("initial","hold","random","greedy")}
    report={"selected_seed":winner,"training":results,"test_scenarios":100,
            "selection":"validation only; test run after SELECTION-LOCK.json",
            "evaluation":evaluations,"paired_profit_improvement":comparisons,
            "initial_improvement_gate":comparisons["initial"]["ci95"][0]>0,
            "positive_profit_gate":comparisons["hold"]["ci95"][0]>0,
            "full_R1_pass":False,"full_R1_note":"Only one economic merchant curriculum. Governance/civilian/stat scenarios and full M1/M2 absent."}
    (args.output/"evaluation.json").write_text(json.dumps(report,indent=2))
    chosen.eval()
    torch.onnx.export(ActorExport(chosen),(torch.zeros(1,OBS),torch.zeros(1,HIDDEN)),
        str(args.output/"economic.onnx"),input_names=["obs","hidden"],output_names=["logits","next_hidden"],
        dynamic_axes={name:{0:"batch"} for name in ("obs","hidden","logits","next_hidden")},
        opset_version=17,dynamo=False)
    manifest={"schema":core.SCHEMA,"reward":core.REWARD,"family":"economic",
              "scope":"early six-region merchant curriculum, not campaign RL",
              "observation":OBS,"actions":ACTIONS,"hidden":HIDDEN,"gamma":1,
              "sha256":hashlib.sha256((args.output/"economic.onnx").read_bytes()).hexdigest(),
              "selected_seed":winner,"cpu_only":True,"quantized":False,
              "trained_with_rl":True,"imitation_warm_start":False,"full_R1_pass":False,
              "warm_start":str(args.resume_dir) if args.resume_dir else None,
              "quality":{k:report[k] for k in ("initial_improvement_gate","positive_profit_gate")}}
    (args.output/"manifest.json").write_text(json.dumps(manifest,indent=2))
    print("FINAL "+json.dumps({"selected_seed":winner,"learned_profit":learned["mean_profit"],"paired":comparisons,"manifest":manifest}),flush=True)

if __name__=="__main__": main()
