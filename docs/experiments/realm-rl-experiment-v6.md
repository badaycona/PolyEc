# Realm v6: frozen protocol

All transitions use the C++ core. Recurrent PPO continues each seed's own v3
checkpoint, resets its critic, and uses no imitation. Three training seeds per
family; curriculum 1/2/6 regions. 32 worlds, four independent core workers, two
Torch threads, 64 decision rollouts and 16 step BPTT with episode reset masks.
Normalized potential reward equals terminal utility change divided by absolute
opening utility (minimum denominator 1); death terminates the focal trajectory.
Economic horizon 512 weekly decisions; governance/civilian 128 monthly decisions.
Training partners are labelled test controllers: uniform legal governance and
economic actions, civilian work/study. Native release co-play uses only learned
policies. Fixed-family training excludes the initial civil service applicants;
long native co-play includes appointments, resignations and role memory resets.

Warm starts: realm-v3/{family}/seed-{1,2,3}.pt. Configurations, initial checkpoints,
selection lock, histories, manifests, and SOURCE-LOCK.json preserve provenance.
CPU budget per seed: governance180s/240updates; economic240s/900updates;
civilian240s/600updates. An update and evaluation may exceed the time threshold;
record elapsed time rather than claiming an exact wall limit.

Validation14000..14031 selects checkpoints/seed before test160000..160099.
Separate categorical audit170000..170099; agency probes180000..180099;
eight untouched native20year holdouts200001..200008. Never retune against these.
Noninferiority margin0.05 in normalized utility; improvement requires paired
bootstrap95%CI lower bound>0. Full R1/R2/R3 require independent evidence and stay
false where diagnosed. An accounting invariant is not a survival quality gate.
V3 eight20year runs ended alive0; retain those negative results.

R0 for credit/matching augments is calculated lazily from a365day copy of the
opening economy, fixed reference weather, no strategy, no births or stochastic
aging. It does not read campaign future randomness. The reference is a prototype
balance convention, not a forecast of future revenue. Twelve typed augments have
funded actions, material costs, time/capacity limits, independent counterparty
consent and serialized court state, covered by core court tests. Stat effects
are mechanics, not evidence of human-like intelligence or R3 strategic reasoning.

Execution note: governance completed first. PowerShell5 treated an ONNX exporter
warning as an error; its selected checkpoint/evaluation were preserved and
realm_export.py exported them without retraining or reselection. Economic and
civilian then ran concurrently, each retaining the same stated per-process
thread limits and per-seed budget. stderr is retained separately and process
exit codes plus model/parity checks decide success, not warning silence.

The exact six compiler/model input files at training freeze are preserved under
artifacts/models/realm-v6/source/ and checked by SOURCE-LOCK.json. After fitting began,
a read-only offers(observer) accessor was added for UI consent details. It does
not change transitions, observations, masks, reward or serialization. Native
runtime additionally intersects inference-time/current candidate masks and terms;
player commands can carry an expected world fingerprint. These production commit
checks are tested independently from the frozen training core.
