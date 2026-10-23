import argparse,json,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--models',type=Path,required=True);p.add_argument('--native-replay',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
paths=[];bundle=''
for family in ('governance','economic','civilian'):
    root=a.models/family;paths.append(str((root/f'{family}.onnx').resolve()));bundle+=json.loads((root/'manifest.json').read_text())['sha256']
subprocess.run([str(a.native_replay.resolve()),*paths,bundle,str(a.output.resolve())],check=True)
rows=json.loads(a.output.read_text());assert len(rows)==64 and rows[-1]['day']==64
print('Native platform fixture:',a.output)





