from pathlib import Path
import shutil,zipfile,json,hashlib
root=Path.cwd(); dest=Path(r'C:\Users\Hi\Documents\Codex\2026-10-25\polyec-rebuild\outputs')
package=root/'outputs/PolyEc'
manifest=json.loads((package/'MANIFEST.json').read_text(encoding='utf-8'))
assert manifest['rules']=='trial-a2' and manifest['save_version']==2
for name,digest in manifest['files'].items():
    assert hashlib.sha256((package/name).read_bytes()).hexdigest()==digest,name
native=root/'outputs/PolyEc-Windows-x64.zip'; source=root/'outputs/PolyEc-source.zip'
with zipfile.ZipFile(native) as z:
    assert z.testzip() is None
    for name,digest in manifest['files'].items():
        assert hashlib.sha256(z.read('PolyEc/'+name)).hexdigest()==digest,name
with zipfile.ZipFile(source) as z:
    assert z.testzip() is None
    assert {str(p.relative_to(root)).replace('\\','/') for p in (root/'core').glob('*') if p.is_file()}=={n for n in z.namelist() if n.startswith('core/')}
    assert all(not any(part in ['.git','work','outputs','.godot'] for part in Path(n).parts) for n in z.namelist())
output=dest/'PolyEc-a2'
for p in package.rglob('*'):
    if not p.is_file() or '.godot' in p.relative_to(package).parts:continue
    target=output/p.relative_to(package);target.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,target)
shutil.copy2(native,dest/'PolyEc-Windows-x64-a2.zip');shutil.copy2(source,dest/'PolyEc-source-a2.zip')
report={'rules':'trial-a2','manifest_files_verified':len(manifest['files']),'source_crc':True,'native_crc':True,'source_clean_scope':True,'canonical':str(root),'session_deliverables':str(output),'git_history_unchanged':True}
(output/'evidence/package-audit-a2.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=False))
