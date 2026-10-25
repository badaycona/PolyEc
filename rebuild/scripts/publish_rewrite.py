# Publish source-only replacement without deleting local legacy files.
from pathlib import Path
import os, subprocess, json, argparse, shutil
ROOT=Path(__file__).resolve().parents[2]
ACTIVE=ROOT/'rebuild'; WORK=ACTIVE/'work'; WORK.mkdir(exist_ok=True)
def git(*args,env=None,input=None):
    p=subprocess.run(['git','-C',str(ROOT),*args],input=input,capture_output=True,env=env)
    if p.returncode: raise RuntimeError(p.stderr.decode('utf-8','replace'))
    return p.stdout.decode('utf-8').strip()
p=argparse.ArgumentParser();p.add_argument('--publish',action='store_true');p.add_argument('--message',default='Replace legacy application tree with native C++ Godot rewrite');args=p.parse_args()
if not args.publish:
    git('fetch','origin','main')
    head=git('rev-parse','HEAD'); remote=git('rev-parse','origin/main')
    env=os.environ.copy();env['GIT_INDEX_FILE']=str(WORK/'publish.index')
    git('read-tree','--empty',env=env)
    files=[ROOT/n for n in ['AGENTS.md','README.md','CMakeLists.txt','WORK_STATUS.md','.gitignore','.gitattributes']]
    files.append(ROOT/'.github/workflows/rewrite.yml')
    for folder in ['core','tests','scripts','docs']:
        files += sorted((ACTIVE/folder).rglob('*'))
    files += [ACTIVE/'AGENTS.md',ACTIVE/'CMakeLists.txt']
    files += [ACTIVE/'game'/n for n in ['main.gd','territory_map.gd','main.tscn','project.godot']]
    paths=[]
    for file in files:
        if not file.is_file() or file.suffix in ['.pyc','.exe','.obj'] or '__pycache__' in file.parts: continue
        path=file.relative_to(ROOT).as_posix();data=file.read_bytes().replace(b"\r\n",b"\n")
        assert len(data)<2_000_000 and not path.endswith('.env'),path
        blob=git('hash-object','-w','--stdin',input=data)
        git('update-index','--add','--cacheinfo','100644',blob,path,env=env);paths.append(path)
    tree=git('write-tree',env=env)
    top=git('ls-tree','--name-only',tree).splitlines()
    assert set(top)=={'.gitattributes','.github','.gitignore','AGENTS.md','CMakeLists.txt','README.md','WORK_STATUS.md','rebuild'},top
    plan={'head':head,'remote':remote,'tree':tree,'files':paths,'branch':git('symbolic-ref','HEAD'),'message':args.message}
    (WORK/'publish-plan.json').write_text(json.dumps(plan,indent=2),encoding='utf-8')
    print(json.dumps({'tree':tree,'files':len(paths),'top':top,'parent':head,'main':remote}))
else:
    plan=json.loads((WORK/'publish-plan.json').read_text(encoding='utf-8'))
    assert git('rev-parse','HEAD')==plan['head'],'Local HEAD moved; prepare again'
    assert git('ls-remote','origin','refs/heads/main').split()[0]==plan['remote'],'Remote main moved; prepare again'
    ancestors=subprocess.run(['git','-C',str(ROOT),'merge-base','--is-ancestor',plan['remote'],plan['head']]).returncode==0
    parents=['-p',plan['head']]+([] if ancestors else ['-p',plan['remote']])
    commit=git('commit-tree',plan['tree'],*parents,input=(plan['message']+'\n').encode())
    # Push first: no local ref/index mutation on rejection.
    print(git('push','origin',commit+':refs/heads/main'))
    assert git('ls-remote','origin','refs/heads/main').split()[0]==commit,'Remote verification failed'
    index=Path(git('rev-parse','--git-path','index'))
    if not index.is_absolute():index=ROOT/index
    if index.exists():shutil.copy2(index,WORK/'index-before-rewrite')
    git('update-ref',plan['branch'],commit,plan['head'])
    git('read-tree',commit) # index only; does not delete/checkout any local path
    git('config','branch.'+plan['branch'].removeprefix('refs/heads/')+'.remote','origin')
    git('config','branch.'+plan['branch'].removeprefix('refs/heads/')+'.merge','refs/heads/main')
    result={'commit':commit,'remote_main_verified':True,'history_preserved':True,'filesystem_legacy_retained':True}
    (WORK/'published.json').write_text(json.dumps(result,indent=2),encoding='utf-8');print(json.dumps(result))
