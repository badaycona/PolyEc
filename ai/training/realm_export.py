import argparse,json,hashlib,sys
from pathlib import Path
import torch
import polyec_env as core
from realm_model import Policy,ActorExport,OBS,ACTIONS
p=argparse.ArgumentParser();p.add_argument('--models',type=Path,required=True);a=p.parse_args();base=a.models
config=json.loads((base/'config.json').read_text());report=json.loads((base/'evaluation.json').read_text());lock=json.loads((base/'SELECTION-LOCK.json').read_text());seed=lock['seed'];family=config['family'];assert report['selected_seed']==seed
assert config.get('model_contract')==core.Realm(1,6,128).model_contract, 'checkpoint was not trained for the current C++ model contract'
model=Policy();model.load_state_dict(torch.load(base/f'seed-{seed}.pt',weights_only=True));model.eval()
torch.onnx.export(ActorExport(model),(torch.zeros(1,OBS),torch.zeros(1,64)),str(base/f'{family}.onnx'),input_names=['obs','hidden'],output_names=['logits','next_hidden'],dynamic_axes={name:{0:'batch'} for name in ('obs','hidden','logits','next_hidden')},opset_version=17,dynamo=False)
manifest={**config,'sha256':hashlib.sha256((base/f'{family}.onnx').read_bytes()).hexdigest(),'selected_seed':seed,'trained_with_rl':True,'imitation':False,'observation':OBS,'actions':ACTIONS,'hidden':64,'normalizer':'clamp[-10,10]','full_R1_pass':False,'quality':{'initial_improvement':report['initial_improvement'],'baseline_noninferiority':report['baseline_noninferiority']}}
(base/'manifest.json').write_text(json.dumps(manifest,indent=2));print('EXPORTED '+family+' '+manifest['sha256'])
