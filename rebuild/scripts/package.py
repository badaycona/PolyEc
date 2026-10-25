from pathlib import Path
import zipfile, json, hashlib, shutil
root=Path.cwd()
package=root/'outputs/PolyEc'
shutil.copytree(root/'docs',package/'docs',dirs_exist_ok=True)
for name in ['project.godot','main.gd','territory_map.gd','main.tscn','polyec_core.exe']:
    shutil.copy2(root/'game'/name,package/'game'/name)

with zipfile.ZipFile('outputs/PolyEc-source.zip','w',zipfile.ZIP_DEFLATED) as z:
    for folder in ['core','tests','scripts','docs']:
        for p in (root/folder).rglob('*'):
            if p.is_file(): z.write(p,p.relative_to(root))
    for name in ['AGENTS.md','CMakeLists.txt']:
        z.write(root/name,name)
    z.write(root/'outputs/PolyEc/README.md','README.md')
    for name in ['project.godot','main.gd','territory_map.gd','main.tscn']:
        z.write(root/'game'/name,'game/'+name)
package=root/'outputs/PolyEc'
files={str(p.relative_to(package)).replace('\\','/'):hashlib.sha256(p.read_bytes()).hexdigest() for p in package.rglob('*') if p.is_file() and p.name!='MANIFEST.json' and '.godot' not in p.relative_to(package).parts}
(package/'MANIFEST.json').write_text(json.dumps({'rules':'trial-a2','save_version':2,'godot':'4.5-stable','files':files},indent=2),encoding='utf-8')
with zipfile.ZipFile('outputs/PolyEc-Windows-x64.zip','w',zipfile.ZIP_DEFLATED) as z:
    for p in package.rglob('*'):
        if p.is_file() and '.godot' not in p.relative_to(package).parts:
            z.write(p,Path('PolyEc')/p.relative_to(package))
print('Source zip:',Path('outputs/PolyEc-source.zip').stat().st_size)
print('Native zip:',Path('outputs/PolyEc-Windows-x64.zip').stat().st_size)
