import sys, numpy as np, torch
from pathlib import Path
sys.path.insert(0,str(Path('D:/Code/PolyEc/ai/training')))
import polyec_env as c
from realm_model import Policy
from realm_train_v6 import training_sequence
assert c.REALM_SCHEMA=='realm-v6'
checks=0
for role in range(3):
 a=c.RealmBatch(16,14500,role,6,8,1);b=c.RealmBatch(16,14500,role,6,8,4)
 total=np.zeros(16)
 for step in range(16):
  oa,ma=a.observe();ob,mb=b.observe();assert np.array_equal(oa,ob) and np.array_equal(ma,mb)
  actions=np.asarray([np.flatnonzero(m)[(i+step)%int(m.sum())] for i,m in enumerate(ma)],dtype='int64')
  ra=a.step(actions,False);rb=b.step(actions,False)
  for x,y in zip(ra,rb):assert np.array_equal(x,y)
  total+=ra[2];checks+=7
  if step>=7:assert ra[3].all() and np.allclose(total,ra[4],atol=1e-5)
 assert a.outcomes()==b.outcomes();checks+=1
 # Fresh reset paths, including ordered seed assignment under different worker counts.
 a=c.RealmBatch(16,14600,role,6,3,1);b=c.RealmBatch(16,14600,role,6,3,4)
 for step in range(9):
  for x,y in zip(a.step(np.zeros(16,dtype='int64')),b.step(np.zeros(16,dtype='int64'))):assert np.array_equal(x,y);checks+=1
policy=Policy();obs=torch.randn(4,16,512);h=torch.randn(4,64);reset=torch.zeros(4,16,dtype=torch.bool);reset[:,8]=True
with torch.no_grad():
 l,v,_=training_sequence(policy,obs,h,reset)
 changed=obs.clone();changed[:,:8]*=4
 l2,v2,_=training_sequence(policy,changed,h*3,reset)
 assert torch.allclose(l[:,8:],l2[:,8:],atol=1e-6) and torch.allclose(v[:,8:],v2[:,8:],atol=1e-6);checks+=2
print(f'V6 PREFLIGHT: {checks} checks; 1/4 worker determinism, terminal freeze, normalized telescoping, reset seed ordering, recurrent episode isolation')
