import argparse
import json
from pathlib import Path
import subprocess
parser=argparse.ArgumentParser()
parser.add_argument("--model-dir",type=Path,required=True)
parser.add_argument("--native-replay",required=True)
parser.add_argument("--output",type=Path,required=True)
args=parser.parse_args()
manifest=json.loads((args.model_dir/"manifest.json").read_text())
rows=subprocess.check_output([args.native_replay,str((args.model_dir/"economic.onnx").resolve())],text=True).strip().splitlines()
steps=[]
for row in rows:
    step,action,fingerprint,cash=row.split()
    steps.append({"index":int(step),"action":int(action),"fingerprint":fingerprint,"cash":int(cash)})
assert len(steps)==64
args.output.write_text(json.dumps({"seed":876543,"model_sha256":manifest["sha256"],"steps":steps},indent=2))





