from pathlib import Path
import subprocess,json,time
exe=Path('game/polyec_core.exe').resolve()
state=Path('work/bridge-kiểm.pe').resolve()
count=0
samples=[]
def call(op,extra=(),viewer=1,debug=0):
    global count
    start=time.perf_counter()
    p=subprocess.run([str(exe),str(state),op,str(viewer),str(debug),*map(str,extra)],capture_output=True,encoding='utf-8')
    r=json.loads(p.stdout)
    samples.append((time.perf_counter()-start)*1000)
    count+=1
    return p,r
p,r=call('reset',[251025]);assert p.returncode==0 and r['ok']
original=state.read_bytes()
p,r=call('command',[4,'goal',0,1,0,0,'mục tiêu riêng']);assert not r['ok'] and state.read_bytes()==original
p,r=call('command',[4,'goal',0,1,0,0,'mua công cụ'],4,1);assert r['ok'] and 'mua công cụ' in state.read_text(encoding='utf-8')
original=state.read_bytes()
p,r=call('command',[4,'goal',0,1,0,1,'bad-viewer'],999,1);assert not r['ok'] and state.read_bytes()==original
checkpoint=Path('work/lưu giữa lượt.pe').resolve()
p,r=call('save',[checkpoint],4,1);assert r['ok']
p,r=call('step',[3],4,1);assert r['ok'] and r['world']['tick']==3
p,r=call('load',[checkpoint],4,1);assert r['ok'] and r['world']['tick']==0 and state.read_bytes()==original
for _ in range(20):
    p,r=call('view',[],4,1);assert r['ok']
views=samples[-20:].copy()
before=state.read_bytes()
p,r=call('command',[4,'propose',5,1,0,1,'__EMPTY__','price=2','price=3'],4,1)
assert not r['ok'] and state.read_bytes()==before
p,r=call('command',[4,'propose',5,1,0,1,'__EMPTY__','unknown=1'],4,1)
assert not r['ok'] and state.read_bytes()==before
p,r=call('command',[4,'propose',5,1,0,1,'__EMPTY__','price=2','duration=20'],4,1)
assert r['ok'] and r['world']['proposals'][0]['price']==2
revision=r['world']['revision']
p,r=call('command',[6,'travel',1,1,0,revision,'__EMPTY__'],6,1);assert r['ok'] and r['world']['fixture_regions']==2
p,r=call('step',[6],6,1);assert r['ok']
revision=r['world']['revision']
p,r=call('command',[6,'pause_journey',1,1,0,revision,'__EMPTY__'],6,1);assert r['ok']
journey_checkpoint=Path('work/hành trình giữa đường.pe').resolve()
p,r=call('save',[journey_checkpoint],6,1);assert r['ok']
p,r=call('step',[2],6,1);assert r['ok']
p,r=call('load',[journey_checkpoint],6,1);assert r['ok']
merchant=next(p for p in r['world']['people'] if p['id']==6)
assert merchant['journey']['coordinate']==6 and merchant['journey']['paused'] and merchant['journey']['toll_paid']==2 and merchant['money']==48
report={'rules':'trial-b1','journey_roundtrip':True,'bridge_checks':count,'unicode_paths_and_goal':True,'actor_permission_atomic':True,'invalid_viewer_atomic':True,'save_load':True,'view_process_ms_median':sorted(views)[10],'view_process_ms_max':max(views)}
Path('outputs/PolyEc-b1/evidence/bridge-b1.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print(json.dumps(report,ensure_ascii=False))

