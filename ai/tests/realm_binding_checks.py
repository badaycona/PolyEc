import argparse,subprocess,json,time
import numpy as np
import polyec_env as core
p=argparse.ArgumentParser();p.add_argument('--replay',required=True);a=p.parse_args()
rows=subprocess.check_output([a.replay],text=True).splitlines();w=core.Realm(37,6,256)
checks=0
for day,row in enumerate(rows):
    for family in (0,1,2):
        for actor in w.due(family):
            mask=w.mask(actor);action=(actor+3*day)%32
            if not mask[action]:action=0
            assert w.step(actor,action)[0]
    w.advance();d,fp,money=map(int,row.split());v=w.view()
    assert v['day']==d and w.fingerprint()==fp and v['money']==money and w.invariant();checks+=4
batch=core.RealmBatch(32,37,2,6,64);before,_=batch.observe();invalid=np.zeros(32,dtype='int64');invalid[-1]=33
try:batch.step(invalid);raise AssertionError('Invalid action accepted')
except ValueError:pass
after,_=batch.observe();assert np.array_equal(before,after);checks+=1
state=w.serialize('binding-check');fingerprint=w.fingerprint();assert not w.deserialize(state,'wrong')[0] and w.fingerprint()==fingerprint;checks+=1
t=time.perf_counter()
for _ in range(64):batch.step(np.zeros(32,dtype='int64'))
elapsed=time.perf_counter()-t
print(json.dumps({'checks':checks,'cpp_python_replay_days':64,'invalid_batch_atomic':True,'bundle_mismatch_atomic':True,'decisions':2048,'seconds':elapsed,'decisions_per_second':2048/elapsed,'schema':core.REALM_SCHEMA,'scope':'training binding world/partner steps, no neural inference'}))
