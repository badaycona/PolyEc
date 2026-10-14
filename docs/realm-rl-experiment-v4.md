# Realm v4 — preregistered correction and longer-horizon RL

V3 long co-play failure:8/8 seeds lost all population by7308days despite accounting
invariants. Preserve all reports. Diagnostics with identical frozen v3 weights
and corrected funded role incomes/pro-rata payroll retain5122/10000 people after
20years on seed1 (before training), so do not attribute this to RL improvement.

Changes: existing governor/director appointments have20coin monthly contractual
salary, funded by local treasury/firm. Actual teacher jobs (Work accepts posted
local job) pay2/day from treasury; they do not also produce industrial goods.
Wage clearing is pro-rata when cash is short, rounding priority rotates by day;
no money created, emergency bailout, or strategy fallback. Economic actors may
choose paid local work. School course skill cap100 and road level cap4 are
physical/content limits, not permission to mask unwanted strategies.

Governance fiscal utility saturates at a public reserve target based on local
population; it cannot value unlimited hoarding above everybody surviving.
Economic survival has a real penalty. Birth hazard applies to every healthy
eligible adult (6%yearly with funded household), removing arbitrary ID400 steps.
Save/observation contracts pinned under realm-v4. Native snapshots/models will
only use this version once trained/parity/evaluation are ready.

PPO: three independently continued seeds per family, from own v3 checkpoint;
redo curriculum1/2/6. Governance/civilian128decisions x30days; economic512 x7days,
so horizon includes roughly10years rather than only the period before failure.
64decision recurrent rollouts,16step BPTT; gamma1, critic bootstrap between
rollouts, termination at actual episode end. Training-only Work/Study partner
baseline stays explicit; app co-play uses three learned models only.

CPU budget: governance240updates/180s perseed, economic900updates/240s,
civilian600updates/240s; at most1980seconds learner time across9runs plus bounded
evaluation.4independent C++ world workers +2PyTorch threads.1vs4worker64step
outputs proved bit-identical before training; seed allocation/reset is ordered.

New validation11000..11031, argmax test100000..100099 after selection lock;
additional categorical deployment audit110000..110099, agency120000..120099.
No old failing test selects a v4 checkpoint. Same normalized utility margin5,
paired bootstrap95%CI; survival/health/defaults/hold reported separately. Full
R1/R2/R3 not certified unless evidence actually supports each requirement.
