"""Real observations; three learned actor exports, actual C++ ORT CPU."""
import argparse,json,sys,time
from pathlib import Path
import numpy as np
import torch
import onnxruntime as ort
import polyec_env as core
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'training'))
from realm_model import Policy
torch.set_num_threads(2)
p=argparse.ArgumentParser();p.add_argument('--models',type=Path,required=True);a=p.parse_args()
report={'schema':'realm-v6','checks':0,'families':{},'provider':'CPUExecutionProvider','absolute_tolerance':1e-5,'relative_tolerance':1e-3}
def check(ok,msg):
    report['checks']+=1
    if not ok:raise AssertionError(msg)
for role,family in enumerate(('governance','economic','civilian')):
    base=a.models/family;manifest=json.loads((base/'manifest.json').read_text());seed=manifest['selected_seed']
    model=Policy();model.load_state_dict(torch.load(base/f'seed-{seed}.pt',weights_only=True));model.eval()
    runtime=core.RealmNativePolicy();check(runtime.load(str(base/f'{family}.onnx')),'load '+family)
    session=ort.InferenceSession(str(base/f'{family}.onnx'),providers=['CPUExecutionProvider'])
    maxima=[0.,0.];times=[]
    for n in (1,3,32,64):
        env=core.RealmBatch(n,60000,role,6,64);obs,mask=env.observe();hidden=np.random.default_rng(59).normal(0,.2,(n,64)).astype('float32')
        for step in range(8):
            with torch.no_grad():ptlogits,_,pthidden=model(torch.from_numpy(obs),torch.from_numpy(hidden))
            pylogits,pymemory=session.run(None,{'obs':obs,'hidden':hidden})
            t=time.perf_counter();native,memory=runtime.predict(obs,hidden);times.append((time.perf_counter()-t)*1000)
            maxima[0]=max(maxima[0],float(np.abs(native-ptlogits.numpy()).max()));maxima[1]=max(maxima[1],float(np.abs(memory-pthidden.numpy()).max()))
            for x,y in ((native,ptlogits.numpy()),(memory,pthidden.numpy()),(native,pylogits),(memory,pymemory)):check(np.allclose(x,y,atol=1e-5,rtol=1e-3),'numerical parity '+family)
            logits=np.where(mask>.5,native,-1e9);probs=np.exp(logits-logits.max(1,keepdims=True));probs/=probs.sum(1,keepdims=True)
            check(np.all(probs[mask<.5]==0),'invalid action probability')
            actions=logits.argmax(1).astype('int64');obs,mask,_,done,_=env.step(actions);hidden=memory*(~done)[:,None]
    check(not core.RealmNativePolicy().load(str(base/'missing.onnx')),'missing model fails closed')
    invalid=np.zeros((1,core.REALM_OBS),dtype='float32');invalid[0,0]=np.nan
    try:runtime.predict(invalid,np.zeros((1,64),dtype='float32'));raise AssertionError('NaN accepted')
    except RuntimeError:check(True,'NaN rejected')
    report['families'][family]={'max_abs_logits':maxima[0],'max_abs_memory':maxima[1],'batch_sizes':[1,3,32,64],'inference_p95_ms':float(np.percentile(times,95))}
(a.models/'native-parity.json').write_text(json.dumps(report,indent=2));print(json.dumps(report,indent=2))





