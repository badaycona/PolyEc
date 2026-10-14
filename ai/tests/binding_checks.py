import argparse
import json
import subprocess
import time
import psutil
import numpy as np
import polyec_env as core

parser=argparse.ArgumentParser()
parser.add_argument("--replay")
parser.add_argument("--spike",type=int)
args=parser.parse_args()
if args.spike:
    process=psutil.Process()
    baseline=process.memory_info().rss
    result=core.citizen_spike(args.spike)
    result.update(baseline_rss_bytes=baseline,peak_rss_bytes=process.memory_info().peak_wset,
                  kind="synthetic storage/sparse-graph primitive, NOT learned citizens or S1")
    print(json.dumps(result))
else:
    rows=subprocess.check_output([args.replay],text=True).strip().splitlines()
    env=core.TradeWorld(876543,6,64)
    for t,row in enumerate(rows):
        expected_step,action,fingerprint,reward,cash,region=row.split()
        actual_reward,done=env.step(int(action))
        assert int(expected_step)==t
        assert env.fingerprint()==int(fingerprint)
        assert abs(actual_reward-float(reward))<1e-6
        assert env.view()["cash"]==int(cash) and env.view()["region"]==int(region)
        assert env.invariant()
    assert done and len(rows)==64
    #Batch validation is atomic even if the final element is invalid.
    batch=core.BatchEnv(16,123,6,64)
    before,mask=batch.observe()
    invalid=np.zeros(16,dtype=np.int64);invalid[-1]=99
    try: batch.step(invalid)
    except ValueError: pass
    else: raise AssertionError("invalid batch was accepted")
    after,_=batch.observe()
    assert np.array_equal(before,after)
    start=time.perf_counter()
    for _ in range(512): batch.step(np.zeros(16,dtype=np.int64))
    seconds=time.perf_counter()-start
    print(json.dumps({"cpp_python_transition_steps_matched":64,"invalid_batch_atomic":True,
                      "rollout_decisions":512*16,"rollout_seconds":seconds,
                      "decisions_per_second":512*16/seconds,"schema":core.SCHEMA}))
