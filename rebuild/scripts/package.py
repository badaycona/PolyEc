from pathlib import Path
import argparse,shutil,zipfile,hashlib,json
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--assemble-only',action='store_true');args=p.parse_args()
OUT=ROOT/'outputs';PACKAGE=OUT/'PolyEc-b1'
for folder in ['game','runtime','docs','evidence']:(PACKAGE/folder).mkdir(parents=True,exist_ok=True)
for name in ['project.godot','main.gd','territory_map.gd','main.tscn']:
    shutil.copy2(ROOT/'game'/name,PACKAGE/'game'/name)
shutil.copy2(ROOT/'work/cmake/rebuild/polyec_core.exe',PACKAGE/'game/polyec_core.exe')
shutil.copy2(ROOT/'work/cmake/rebuild/PolyEc.exe',PACKAGE/'PolyEc.exe')
shutil.copy2(ROOT/'work/tools/godot/Godot_v4.5-stable_win64.exe',PACKAGE/'runtime/Godot.exe')
for name in ['Godot-LICENSE.txt','Godot-COPYRIGHT.txt']:
    shutil.copy2(ROOT/'outputs/PolyEc/runtime'/name,PACKAGE/'runtime'/name)
shutil.copytree(ROOT/'docs',PACKAGE/'docs',dirs_exist_ok=True)
shutil.copy2(ROOT/'docs/PLAYER_B.md',PACKAGE/'README.md')
for source,name in [('work/cmake/Testing/Temporary/LastTest.log','cpp-b1.log'),('work/build-b-final.log','ninja-b1.log'),('work/vs-build-b-final.log','visual-studio-b1.log'),('work/ui-local-b1.log','ui-local-b1.log'),('work/ui-actions-b1.log','ui-actions-b1.log'),('work/native-journey-b1.log','native-journey-b1.log')]:
    if (ROOT/source).exists():shutil.copy2(ROOT/source,PACKAGE/'evidence'/name)
if args.assemble_only:print(PACKAGE);raise SystemExit
with zipfile.ZipFile(OUT/'PolyEc-source-b1.zip','w',zipfile.ZIP_DEFLATED) as z:
    for folder in ['core','tests','scripts','docs']:
        for file in sorted((ROOT/folder).rglob('*')):
            if file.is_file() and '__pycache__' not in file.parts and file.suffix not in ['.pyc','.exe','.obj']:
                z.write(file,file.relative_to(ROOT))
    for name in ['AGENTS.md','CMakeLists.txt']:z.write(ROOT/name,name)
    z.write(ROOT/'docs/PLAYER_B.md','README.md')
    for name in ['project.godot','main.gd','territory_map.gd','main.tscn']:z.write(ROOT/'game'/name,'game/'+name)
files={p.relative_to(PACKAGE).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(PACKAGE.rglob('*')) if p.is_file() and p.name!='MANIFEST.json' and '.godot' not in p.relative_to(PACKAGE).parts}
(PACKAGE/'MANIFEST.json').write_text(json.dumps({'rules':'trial-b1','save_version':3,'godot':'4.5-stable','files':files},indent=2),encoding='utf-8')
with zipfile.ZipFile(OUT/'PolyEc-Windows-x64-b1.zip','w',zipfile.ZIP_DEFLATED) as z:
    for file in sorted(PACKAGE.rglob('*')):
        if file.is_file() and '.godot' not in file.relative_to(PACKAGE).parts:z.write(file,Path('PolyEc-b1')/file.relative_to(PACKAGE))
print(json.dumps({'package':str(PACKAGE),'files':len(files),'source_bytes':(OUT/'PolyEc-source-b1.zip').stat().st_size,'windows_bytes':(OUT/'PolyEc-Windows-x64-b1.zip').stat().st_size}))
