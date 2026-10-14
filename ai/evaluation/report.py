"""Render frozen experiment evidence; never selects or changes a checkpoint."""
import argparse
import json
from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

parser=argparse.ArgumentParser()
parser.add_argument("--experiment",type=Path,required=True)
parser.add_argument("--previous",type=Path,required=True)
parser.add_argument("--output",type=Path,required=True)
args=parser.parse_args();args.output.mkdir(parents=True,exist_ok=True)
report=json.loads((args.experiment/"evaluation.json").read_text())
lock=json.loads((args.experiment/"economic-gate-lock.json").read_text())
parity=json.loads((args.experiment/"native-parity.json").read_text())
manifest=json.loads((args.experiment/"manifest.json").read_text())
comparison=report["paired_profit_improvement"]
gate={"scope":"economic-only curriculum; full R1 remains false",
      "noninferiority_margin_coins":lock["noninferiority_margin_coins"],
      "vs_greedy_ci_lower":comparison["greedy"]["ci95"][0],
      "baseline_noninferior":comparison["greedy"]["ci95"][0]>=-lock["noninferiority_margin_coins"],
      "positive_profit":comparison["hold"]["ci95"][0]>0,
      "improves_initial":comparison["initial"]["ci95"][0]>0,
      "invalid_actions":report["evaluation"][f"learned_seed_{report['selected_seed']}"]["invalid_actions"],
      "native_parity_checks":parity["checks_passed"],"full_R1_pass":False}
gate["economic_experiment_pass"]=gate["baseline_noninferior"] and gate["positive_profit"] and gate["improves_initial"] and gate["invalid_actions"]==0
(args.experiment/"economic-gate-result.json").write_text(json.dumps(gate,indent=2))

plt.rcParams.update({"figure.facecolor":"#0b191f","axes.facecolor":"#14262b","axes.edgecolor":"#425b5e",
    "axes.labelcolor":"#c5d6cd","xtick.color":"#91a7a4","ytick.color":"#91a7a4","text.color":"#e7e2cd",
    "font.size":10,"grid.color":"#31494c","legend.facecolor":"#14262b","legend.edgecolor":"#425b5e"})
fig,(learning,heldout)=plt.subplots(1,2,figsize=(14,5),gridspec_kw={"width_ratios":[1.3,1]})
colors=["#74b7a5","#d1b575","#a4c7dd"]
for seed,color in zip((1,2,3),colors):
    old=json.loads((args.previous/f"seed-{seed}-history.json").read_text())
    new=json.loads((args.experiment/f"seed-{seed}-history.json").read_text())
    for data,offset,style in ((old,0,"--"),(new,old[-1]["decisions"],"-")):
        rows=[r for r in data if "validation_mean_profit" in r]
        learning.plot([(r["decisions"]+offset)/1e6 for r in rows],[r["validation_mean_profit"] for r in rows],
                      style,color=color,alpha=.9,label=f"Seed {seed}" if style=="-" else None)
learning.set(title="Validation during recurrent PPO",xlabel="Agent decisions processed per seed (millions)",ylabel="Mean terminal cash profit (coins)")
learning.axhline(0,color="#718382",lw=1);learning.grid(alpha=.4);learning.legend()
names=["initial","hold","random","greedy",f"learned_seed_{report['selected_seed']}"]
labels=["Before\nfine-tuning","Hold","Random\nvalid","Greedy\nbaseline","Learned\nPPO"]
means=[];errors=[];rng=np.random.default_rng(491)
for name in names:
    profits=np.array(report["evaluation"][name]["profits"])
    sampled=profits[rng.integers(0,len(profits),(5000,len(profits)))].mean(1)
    lower,upper=np.quantile(sampled,[.025,.975]);mean=profits.mean()
    means.append(mean);errors.append([mean-lower,upper-mean])
heldout.bar(labels,means,color=["#64797b","#64797b","#64797b","#d1b575","#74b7a5"],
            yerr=np.array(errors).T,error_kw={"ecolor":"#e7e2cd","capsize":4})
heldout.set(title="100 untouched scenarios (test30000-30099)",ylabel="Terminal cash profit;95% bootstrap CI")
heldout.axhline(0,color="#718382",lw=1);heldout.grid(axis="y",alpha=.35)
fig.suptitle("PolyEc / economic RL experiment — actual PPO checkpoints",fontsize=16,x=.06,ha="left")
fig.text(.06,.015,"One learned merchant / six markets. Validation selects checkpoints. This does not establish campaign governance or civilian RL.",color="#91a7a4",fontsize=9)
fig.tight_layout(rect=(.02,.04,.99,.92))
fig.savefig(args.output/"PolyEc-RL-evidence.png",dpi=150)
selected=report["evaluation"][f"learned_seed_{report['selected_seed']}"]
total_decisions=sum(r["decisions"] for r in report["training"])
seconds=sum(r["seconds"] for r in report["training"])
summary=f'''# PolyEc — kết quả RL kinh tế

Policy GRU64 đã được train bằng PPO, dùng chính C++ core của app qua pybind11.
Không imitation hoặc heuristic fallback trong executable. Đây là curriculum
giao thương một merchant, không phải M1/M2/M3 hoàn chỉnh hay R1 đầy đủ.

## Kết quả test đã khóa

- Test100 scenario seeds30000–30099; chọn checkpoint/seed bằng validation10000–10031.
- Seed được chọn: {report['selected_seed']}.
- Lợi nhuận policy RL: {selected['mean_profit']:.2f} đồng/phiên.
- Baseline greedy: {report['evaluation']['greedy']['mean_profit']:.2f} đồng/phiên.
- Policy trước fine-tuning: {report['evaluation']['initial']['mean_profit']:.2f} đồng/phiên.
- Chọn ngẫu nhiên hợp lệ: {report['evaluation']['random']['mean_profit']:.2f} đồng/phiên.
- Giữ tiền: {report['evaluation']['hold']['mean_profit']:.2f} đồng/phiên.
- Chênh lệch so greedy: {comparison['greedy']['mean']:.2f},95% CI {comparison['greedy']['ci95']}.
- Economic-only gate: {gate['economic_experiment_pass']}; full R1: False.
- Action invalid: {selected['invalid_actions']}.

## Công sức và native parity

- Ba seed tiếp tục PPO độc lập; {total_decisions:,} quyết định ở lượt v2, {seconds:.1f} giây.
- Hai luồng CPU; peak training process {max(r['peak_rss_bytes'] for r in report['training'])/1024**2:.1f} MiB.
- Native C++ ORT/Python ORT/PyTorch: {parity['checks_passed']} kiểm tra, batch1/3/32/64.
- Sai số logits tối đa {parity['max_abs_logits_error']:.3g}; memory {parity['max_abs_memory_error']:.3g}.
- Native inference batch32 p95 {parity['native_batch32_latency_ms_p95']:.3f} ms (không phải benchmark toàn game).
- SHA256 model: `{manifest['sha256']}`.

## Giới hạn

Firm/household dùng hợp đồng và basket cố định làm luật environment. Chưa có
learned governance/civilian/child policy, credit/project/knowledge đầy đủ, stat,
augment, campaign hoặc100k/1m người có agency. Spike1m chỉ là48MB record/graph,
không chứng minh RL ở quy mô đó. Windows được kiểm tra; Linux chưa kiểm tra.
V1 được giữ nguyên: nó có lợi nhuận dương nhưng thua baseline; v2 dùng tập test mới.
'''
(args.output/"PolyEc-RL-report.md").write_text(summary,encoding="utf-8")
print(json.dumps(gate,indent=2))
