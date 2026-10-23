import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import time
import numpy as np
import onnxruntime as ort
import torch
import polyec_env as core
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/"training"))
from model import Policy,HIDDEN

torch.set_num_threads(2)
parser=argparse.ArgumentParser()
parser.add_argument("--model-dir",type=Path,required=True)
parser.add_argument("--native-replay",required=True)
args=parser.parse_args()
manifest=json.loads((args.model_dir/"manifest.json").read_text())
path=args.model_dir/"economic.onnx"
assert hashlib.sha256(path.read_bytes()).hexdigest()==manifest["sha256"]
model=Policy();model.load_state_dict(torch.load(args.model_dir/f"seed-{manifest['selected_seed']}.pt",weights_only=True));model.eval()
options=ort.SessionOptions();options.intra_op_num_threads=2;options.inter_op_num_threads=1
runtime=ort.InferenceSession(str(path),sess_options=options,providers=["CPUExecutionProvider"])
native=core.NativePolicy();assert native.load(str(path.resolve())),native.error()
rng=np.random.default_rng(196)
maximum_logit_error=0.;maximum_memory_error=0.;checks=0
for size in (1,3,32,64):
    env=core.BatchEnv(size,456789,6,64)
    obs,mask=env.observe();hidden=rng.normal(0,.2,(size,HIDDEN)).astype(np.float32)
    with torch.no_grad(): logits,_,next_hidden=model(torch.from_numpy(obs),torch.from_numpy(hidden))
    reference=(logits.numpy(),next_hidden.numpy())
    python_output=runtime.run(None,{"obs":obs,"hidden":hidden})
    native_output=native.predict(obs,hidden)
    for output in (python_output,native_output):
        for actual,expected in zip(output,reference):
            np.testing.assert_allclose(actual,expected,atol=1e-5,rtol=1e-3);checks+=1
        maximum_logit_error=max(maximum_logit_error,float(np.abs(output[0]-reference[0]).max()))
        maximum_memory_error=max(maximum_memory_error,float(np.abs(output[1]-reference[1]).max()))
        probability=torch.softmax(torch.from_numpy(output[0]).masked_fill(torch.from_numpy(mask)<.5,-1e9),-1).numpy()
        expected_probability=torch.softmax(logits.masked_fill(torch.from_numpy(mask)<.5,-1e9),-1).numpy()
        np.testing.assert_allclose(probability,expected_probability,atol=1e-5,rtol=1e-3)
        assert float(probability[mask<.5].sum())==0
        checks+=2
    if size==32: timing_obs,timing_hidden=obs,hidden
durations=[]
for _ in range(100):
    start=time.perf_counter();native.predict(timing_obs,timing_hidden);durations.append((time.perf_counter()-start)*1000)
#Independent C++ executable performs both ORT inference and the world transition.
rows=subprocess.check_output([args.native_replay,str(path.resolve())],text=True).strip().splitlines()
world=core.TradeWorld(876543,6,64);hidden=torch.zeros(1,HIDDEN)
for index,row in enumerate(rows):
    step,action,fingerprint,cash=map(int,row.split())
    obs=np.array(world.observation(),dtype=np.float32)[None,:]
    mask=np.array(world.mask(),dtype=np.float32)[None,:]
    with torch.no_grad(): logits,_,hidden=model(torch.from_numpy(obs),hidden)
    expected=int(logits.masked_fill(torch.from_numpy(mask)<.5,-1e9).argmax(-1))
    assert action==expected and step==index
    world.step(action)
    assert world.fingerprint()==fingerprint and world.view()["cash"]==cash
assert len(rows)==64
checks+=64
bad=timing_obs.copy();bad[0,0]=np.nan
try:native.predict(bad,timing_hidden)
except RuntimeError:checks+=1
else:raise AssertionError("NaN input did not stop inference")
assert not native.load(str(args.model_dir/"missing.onnx"));checks+=1
report={"checks_passed":checks,"native_runtime":native.version(),"python_runtime":ort.__version__,
        "providers":runtime.get_providers(),"batches":[1,3,32,64],"atol":1e-5,"rtol":1e-3,
        "max_abs_logits_error":maximum_logit_error,"max_abs_memory_error":maximum_memory_error,
        "masked_invalid_probability":0,"native_rollout_steps_matched":64,
        "native_batch32_latency_ms_p50":float(np.quantile(durations,.5)),
        "native_batch32_latency_ms_p95":float(np.quantile(durations,.95)),
        "model_sha256":manifest["sha256"]}
(args.model_dir/"native-parity.json").write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))





