"""Controlled initial-condition probes. No game rules implemented in Python."""
import argparse,json
from pathlib import Path
import numpy as np
import polyec_env as core

p=argparse.ArgumentParser();p.add_argument('--models',type=Path,required=True);a=p.parse_args()
policy=core.RealmNativePolicy();assert policy.load(str(a.models/'civilian/civilian.onnx'))
variants={
    'child_school':dict(age=8,stat=50,goal=0,budget=100,schools=30,jobs=30),
    'adult_school':dict(age=30,stat=50,goal=0,budget=100,schools=30,jobs=30),
    'child_no_school':dict(age=8,stat=50,goal=0,budget=100,schools=0,jobs=30),
    'adult_no_jobs':dict(age=30,stat=50,goal=0,budget=100,schools=30,jobs=0),
    'child_no_budget':dict(age=8,stat=50,goal=0,budget=0,schools=30,jobs=30),
    'adult_hungry':dict(age=30,stat=50,goal=0,budget=100,schools=30,jobs=30,nutrition=0,health=50),
    'adult_fed':dict(age=30,stat=50,goal=0,budget=100,schools=30,jobs=30,nutrition=100,health=50),
    'adult_social_goal':dict(age=30,stat=50,goal=2,budget=100,schools=30,jobs=30),
}
report={'scenario_seeds':[180000,180099],'samples_per_state':64,'variants':{},'R2_full_pass':False,'R3_full_pass':False}
all_probs={}
for name,config in variants.items():
    worlds=[core.Realm.scenario(seed,129,**config) for seed in range(180000,180100)]
    obs=np.asarray([w.observe(129) for w in worlds],dtype='float32');masks=np.asarray([w.mask(129) for w in worlds],dtype='float32')
    logits,_=policy.predict(obs,np.zeros((100,64),dtype='float32'));logits=np.where(masks>.5,logits,-1e9);probs=np.exp(logits-logits.max(1,keepdims=True));probs/=probs.sum(1,keepdims=True);all_probs[name]=probs
    rng=np.random.default_rng(719);draws=np.array([[rng.choice(32,p=probs[i]) for _ in range(64)] for i in range(100)])
    counts=np.bincount(draws.ravel(),minlength=32);invalid=int((masks[np.arange(100)[:,None],draws]<.5).sum());assert invalid==0
    report['variants'][name]={'mean_distribution':probs.mean(0).tolist(),'unique_actions':int((counts>0).sum()),'invalid':invalid,'work_probability':float(probs[:,1:8].sum(1).mean()),'school_probability':float(probs[:,15].mean()),'same_state_distinct_actions_mean':float(np.mean([len(set(row)) for row in draws]))}
report['mean_total_variation_vs_adult']={name:float(np.abs(prob-all_probs['adult_school']).sum(1).mean()/2) for name,prob in all_probs.items()}
report['age_and_capacity_masks_pass']=report['variants']['child_school']['work_probability']==0 and report['variants']['child_no_school']['school_probability']==0 and report['variants']['adult_no_jobs']['work_probability']==0
report['private_sampling_variation_pass']=report['variants']['adult_school']['same_state_distinct_actions_mean']>1
report['R2_diagnosis']='Age/budget/school/job masks and first-decision distributions tested. Household funding is implemented; full child-adult learned rollout transfer and multi-policy holdout survival still require validation; full R2 not claimed.'
report['R3_diagnosis']='INT changes school skill gain; ADM project duration; INTG investigation delay; MIL unrest reduction. ECO affects procurement preparation; DIP affects offer preparation. An independent multistep INT strategy task is not established. Mechanical stat effects do not prove reasoning; full R3 not claimed.'
(a.models/'agency-stat-report.json').write_text(json.dumps(report,indent=2));print(json.dumps({k:v for k,v in report.items() if k!='variants'},indent=2))
