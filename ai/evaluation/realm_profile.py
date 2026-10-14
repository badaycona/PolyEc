import argparse,hashlib,json,subprocess,time,os,atexit
from pathlib import Path
import psutil,sys
p=argparse.ArgumentParser();p.add_argument('--models',type=Path,required=True);p.add_argument('--benchmark',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--days',type=int,default=365);p.add_argument('--seed',type=int,default=37);a=p.parse_args()
paths=[];hashes=[]
for family in ('governance','economic','civilian'):
    base=a.models/family;paths.append(str((base/f'{family}.onnx').resolve()));hashes.append(json.loads((base/'manifest.json').read_text())['sha256'])
# Coordinate concurrent benchmark jobs and reuse only complete identical evidence.
lock=a.output.with_suffix(a.output.suffix+'.running')
while True:
    try:
        fd=os.open(lock,os.O_CREAT|os.O_EXCL|os.O_WRONLY);os.write(fd,str(os.getpid()).encode());os.close(fd);break
    except FileExistsError:
        try:owner=int(lock.read_text())
        except (ValueError,OSError):time.sleep(.1);continue
        if not psutil.pid_exists(owner):lock.unlink(missing_ok=True)
        else:time.sleep(.1)
atexit.register(lambda:lock.unlink(missing_ok=True))
if a.output.exists():
    existing=json.loads(a.output.read_text())
    if existing.get('seed')==a.seed and existing.get('days')==a.days and existing.get('model_hashes')==hashes and existing.get('benchmark_executable_sha256')==hashlib.sha256(a.benchmark.read_bytes()).hexdigest() and 'peak_process_bytes' in existing:
        print('REUSED identical completed native evidence: '+str(a.output));sys.exit(0)
    raise RuntimeError('Existing benchmark evidence differs; choose a new output path')
child=psutil.Popen([str(a.benchmark.resolve()),*paths,''.join(hashes),str(a.output.resolve()),str(a.days),str(a.seed)],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
peak=0;t=time.perf_counter()
while child.poll() is None:
    try:peak=max(peak,child.memory_info().peak_wset)
    except psutil.Error:pass
    time.sleep(.02)
out,err=child.communicate()
if child.returncode:raise RuntimeError(err)
report=json.loads(a.output.read_text());report.update({'seed':a.seed,'peak_process_bytes':peak,'wall_seconds':time.perf_counter()-t,'model_hashes':hashes,'benchmark_executable_sha256':hashlib.sha256(a.benchmark.read_bytes()).hexdigest(),'hardware':'Windows / Intel i7-12700K,12cores20threads,15.78GiB RAM','onnx_runtime':'1.31.0 CPU,2intra/1inter threads per family','note':'Includes core+observations+inference+sampling+commit+invariants; excludes render/input/save. Not profileB/C evidence.'})
a.output.write_text(json.dumps(report,indent=2));print(out);print(json.dumps(report,indent=2))
