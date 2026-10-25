from pathlib import Path
import argparse,shutil,zipfile,hashlib,json
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--destination',type=Path,required=True);args=p.parse_args()
PACKAGE=ROOT/'outputs/PolyEc-b1';manifest=json.loads((PACKAGE/'MANIFEST.json').read_text(encoding='utf-8'))
assert manifest['rules']=='trial-b1' and manifest['save_version']==3
for name,digest in manifest['files'].items():assert hashlib.sha256((PACKAGE/name).read_bytes()).hexdigest()==digest,name
native=ROOT/'outputs/PolyEc-Windows-x64-b1.zip';source=ROOT/'outputs/PolyEc-source-b1.zip'
with zipfile.ZipFile(native) as z:
    assert z.testzip() is None
    for name,digest in manifest['files'].items():assert hashlib.sha256(z.read('PolyEc-b1/'+name)).hexdigest()==digest,name
with zipfile.ZipFile(source) as z:
    assert z.testzip() is None
    for p in (ROOT/'core').glob('*'):
        if p.is_file():assert z.read(p.relative_to(ROOT).as_posix())==p.read_bytes(),str(p)
    assert all(not any(part in ['.git','work','outputs','.godot'] for part in Path(n).parts) for n in z.namelist())
DEST=args.destination.resolve();DEST.mkdir(parents=True,exist_ok=True)
output=DEST/'PolyEc-b1'
for p in PACKAGE.rglob('*'):
    if p.is_file() and '.godot' not in p.relative_to(PACKAGE).parts:
        target=output/p.relative_to(PACKAGE);target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,target)
shutil.copy2(native,DEST/native.name);shutil.copy2(source,DEST/source.name)
report={'rules':'trial-b1','manifest_files_verified':len(manifest['files']),'source_crc':True,'native_crc':True,'source_core_byte_exact':True,'canonical':str(ROOT),'delivered':str(output)}
(output/'evidence/package-audit-b1.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8');print(json.dumps(report,ensure_ascii=False))
