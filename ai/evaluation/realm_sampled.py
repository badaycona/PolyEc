import argparse,json,sys
from pathlib import Path
import numpy as np
import torch
import polyec_env as core
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'training'))
from realm_model import Policy
torch.set_num_threads(2)
p=argparse.ArgumentParser();p.add_argument('--models',type=Path,required=True);p.add_argument('--original',type=Path,required=True);a=p.parse_args()
def evaluate(policy,role,mode):
    batch=core.RealmBatch(100,170000,role,6,512 if role==1 else 128);obs,masks=batch.observe();h=torch.zeros(100,64);rng=np.random.default_rng(491);totals=np.zeros(100);counts=np.zeros(32,dtype='int64');active=np.ones(100,dtype=bool)
    for step in range(512 if role==1 else 128):
        uniforms=rng.random(100)
        if mode=='sampled':
            with torch.no_grad():logits,_,h=policy(torch.from_numpy(obs),h)
            scores=np.where(masks>.5,logits.numpy(),-1e9);probs=np.exp(scores-scores.max(1,keepdims=True));probs/=probs.sum(1,keepdims=True)
            actions=(np.cumsum(probs,axis=1)<uniforms[:,None]).sum(1).clip(0,31).astype('int64')
        elif mode=='hold':actions=np.zeros(100,dtype='int64')
        elif mode=='random':actions=np.array([np.flatnonzero(m)[min(int(u*sum(m)),int(sum(m))-1)] for m,u in zip(masks,uniforms)],dtype='int64')
        else:
            score=np.zeros((100,32),dtype='float32')
            if role==0:score[:,2]=.2;score[:,4]=2;score[:,5]=1.5;score[:,6:13]=np.where((obs[:,99]<.9)[:,None],4,0)
            elif role==1:
                f=obs[:,128:].reshape(100,32,12);score[:,1:8]=1-f[:,1:8,8];score[:,8:15]=f[:,8:15,8]-1;score[:,30]=.3*(1+np.floor(obs[:,4]*4))
            else:score[:,1:8]=1;score[:,15]=np.where(obs[:,1]<.18,3,.5);score[:,16]=np.where(obs[:,3]<.6,4,0)
            actions=np.where(masks>.5,score,-1e9).argmax(1).astype('int64')
        assert np.all(masks[np.arange(100),actions]>.5)
        counts+=np.bincount(actions[active],minlength=32);obs,masks,r,done,score=batch.step(actions,False);totals+=r;active=~done
        if done.all():break
    assert done.all() and np.allclose(totals,score,atol=1e-4)
    outcomes=list(batch.outcomes())
    return {'utility':score.tolist(),'mean_utility':float(score.mean()),'hold_rate':float(counts[0]/counts.sum()),'action_counts':counts.tolist(),'survival_rate':float(np.mean([x['alive'] for x in outcomes])),'mean_health':float(np.mean([x['health'] for x in outcomes])),'cash_shortfall_vs_debt_rate':float(np.mean([x['insolvent'] for x in outcomes])),'obligations_due':sum(x['obligations_due'] for x in outcomes),'defaults':sum(x['defaults'] for x in outcomes),'invalid_actions':0}
def ci(d):
    d=np.asarray(d);rng=np.random.default_rng(491);boot=d[rng.integers(0,100,(5000,100))].mean(1);return {'mean':float(d.mean()),'ci95':np.quantile(boot,[.025,.975]).tolist()}
report={'test_seeds':[170000,170099],'sampling':'categorical; common uniform seed491','selection':'validation only, frozen before this audit','families':{},'metric':'normalized terminal utility delta / abs(opening utility), not coins','full_R1_pass':False}
for role,family in enumerate(('governance','economic','civilian')):
    base=a.models/family;manifest=json.loads((base/'manifest.json').read_text());seed=manifest['selected_seed']
    selected=Policy();selected.load_state_dict(torch.load(base/f'seed-{seed}.pt',weights_only=True));selected.eval()
    original=Policy();original.load_state_dict(torch.load(a.original/family/f'initial-{seed}.pt',weights_only=True));original.eval()
    warm=Policy();warm.load_state_dict(torch.load(base/f'initial-{seed}.pt',weights_only=True));warm.eval()
    evaluations={'learned':evaluate(selected,role,'sampled'),'original_untrained':evaluate(original,role,'sampled'),'before_v6':evaluate(warm,role,'sampled')}
    for mode in ('hold','random','baseline'):evaluations[mode]=evaluate(selected,role,mode)
    comparisons={name:ci(np.array(evaluations['learned']['utility'])-np.array(value['utility'])) for name,value in evaluations.items() if name!='learned'}
    report['families'][family]={'selected_seed':seed,'evaluation':evaluations,'paired':comparisons,'original_improvement':comparisons['original_untrained']['ci95'][0]>0,'baseline_noninferiority':comparisons['baseline']['ci95'][0]>=-.05}
    print(family,json.dumps({'mean_utility':evaluations['learned']['mean_utility'],'paired':comparisons}),flush=True)
(a.models/'runtime-sampling-audit.json').write_text(json.dumps(report,indent=2))
