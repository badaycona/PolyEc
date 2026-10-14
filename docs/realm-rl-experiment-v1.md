# Realm v1 experiment — locked before training/test

Same C++ Realm runs app and training. Three actor families, shared weights per
family and private recurrent memory/sampling. PPO starts from random initialized
networks and learns from actual funded transitions; no imitation or runtime
baseline. 512 inputs (128 public/private state + 32 candidates x12), GRU64.

Training: seeds1/2/3 per family; curriculum1/2/6 provinces plus crown. Seeds>=2^32;
validation10000..10031; checkpoint/seed selection only validation. Test40000..40099
is run after selection lock. Gamma1; reward=(utility_after-utility_before)/10;
64 decisions/episode; governance/civilian30days, economic7days. No intermediate
bonus for signing/cancelling/loan principal. Utility includes real debt+interest.

Metrics: terminal utility delta, invalid actions, hold fraction, action counts,
paired bootstrap95%CI against initial/hold/random/development baseline. Fixed
noninferiority margin5 utility; initial improvement lowerCI>0. These are early
family gates, not substitutes for complete R1. R1 also requires survival,
obligation, negotiation and co-play evaluation; R2 requires controlled age,
resource/capacity response and separate sampling; R3 requires paired stat/goal
ablations and multistep reasoning evidence. Failed gates must remain explicit.

Initial training budget:160 updates or180s per seed (9seed runs), CPU2threads,
32environments, bounded recurrent BPTT16. No paid compute. Preserve config,
initial/selected checkpoints, histories, exact source snapshot and hashes.
Training128people/world is not evidence for10k policy throughput; benchmark
native10k with actual observation/inference/commit separately.

Current schema limitations to diagnose: firms use laws for base production;
proposal consideration only on actor turn; limited flat32candidates; incomplete
augment semantics, intelligence forecasting and advanced political charter
conflict. Do not silently mark the plan's full milestones complete.
