# M5 v23 — Economic worksite curriculum screen

## Objective and evidence

The v22 controlled audit found that three failed v20 Economic policies gave low probability to Fiber purchase, transfer, authorization, and work, while continuing to prefer creating workshops. The action sequence itself is legal and produces Cloth in the shared C++ core. Test whether PPO receives enough credit on these sparse intermediate actions by changing only the training reset distribution.

## Before → after

Before: every Economic episode starts in an ordinary full Realm; production actions may be far from the initial state, and full-world rollouts rarely reach the input/authorized-work stages.

After: an explicit `--worksite-curriculum v1` option assigns 1/4 of Economic reset seeds to each of: ordinary Realm, newly created Cloth workshop without Fiber, workshop with Fiber held by its owner, and a distinct authorized worker with a site-bound Work candidate. Every setup uses typed commands/candidates in the same C++ Realm and passes invariants. The remaining 1/4 preserve the ordinary full-world distribution. The flag is locked into run manifests, checkpoint/resume identity, environment state, runset, and model selection.

## Boundaries

- Files: `ai/training/realm_coplay_env_v11.py`, `ai/training/realm_train_v11.py`, `ai/training/realm_train_set_v11.py`, `ai/evaluation/realm_select_v11.py`, and the environment/trainer checks.
- No game rules, action/observation/model/reward contract or save schema change. Reward stays normalized focal utility delta. No setup-generated goods, jobs, money, or authorization bypass the typed C++ command paths.
- Each update logs action-intent diagnostics split by the reset context; exact checkpoint resume restores the context label and current Realm snapshots.
- `none` remains the default for existing runs; curriculum v1 only applies to Economic. Failed v20 models remain peer-ineligible.

## Verification and screen

First run environment setup/mask/invariant/resume checks and a 1-update smoke proving the four context labels appear with the expected legal candidates. If they pass, run three Economic seeds at the same v20 sample budget and optimizer settings (25 updates ×25,600 decisions per seed), with only reset distribution changed. Validate on 20 fresh worlds ×128 days using the existing sampled selector; report workshop/work/payroll/output and utility against baseline and initial. No candidate promotion unless existing selector criteria pass. Self-review and no independent reviewer must be stated.

## Limits

This is a curriculum screen, not final M5 evidence. It does not supply learned partners, replace full-world training/evaluation, or close 100-world/full-horizon R1, remaining gameplay contracts, or population/performance/package work. Stop the screen if setup or invariant checks fail; do not relax gates.
