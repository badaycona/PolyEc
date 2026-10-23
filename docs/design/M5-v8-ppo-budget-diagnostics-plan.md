# M5 — PPO budget and training diagnostics revision

## Purpose

Apply the 2026-10-21 PPO review before another long run. Training comparisons must use equal decision budgets, stop cleanly when a wall-clock cap prevents that budget, and expose enough measurements to distinguish rollout, policy-update, and critic problems. This slice does not claim learned-policy acceptance and does not change gameplay, action schema, reward, or world transitions.

## Files

- `ai/training/realm_train_v8.py`: dependency preflight before training; fixed update/decision budget for every seed; `--seconds` is a hard per-seed guard that marks the run incomplete and prevents selection/evaluation/export; record rollout, actor action sampling, C++ environment-step and optimizer timing, approximate KL, clip fraction, explained variance/value error, reward/advantage ranges, action counts and legal-action-count context.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`: record behavior and evidence after the source change.
- `docs/reports/M5-v8-ppo-budget-diagnostics-evidence-2026-10-21.md`: report checks and limitations.

## Invariants and behavior

- An accepted training run has the same `updates * envs * rollout_steps` decisions for each of seeds 1, 2, 3. The elapsed-time limit protects resources; it never makes unequal seeds comparable.
- Any seed that cannot finish the configured update count is explicitly incomplete. Main exits before model selection, evaluation, or ONNX export.
- ONNX exporter dependency is checked before creating training artifacts or starting environment rollouts.
- Approximate KL and clipping are measured from the same sampled rollout actions and update log-probabilities. Critic explained variance compares pre-update rollout values to GAE returns. Timers separate actor action sampling, C++ environment stepping and network optimization.
- Action counts and legal-choice-count bins describe this adapter only. They do not imply worksite, production, payroll, or representative partner coverage.
- Existing reward, action/model contract, selection criterion, and policy weights are unchanged by diagnostics.

## Gate

1. Python syntax and focused synthetic metric checks.
2. A tiny bounded training invocation with all three seeds completes at exactly the configured decision budget, writes diagnostics, and passes preflight.
3. An intentionally tiny timeout exits nonzero and writes no selection lock, final evaluation, or model export.
4. Review resulting diff and update evidence. Do not start full-length training until the action/observation coverage and partner-set gaps from the PPO review are addressed.

## Review note

Self-review only; no independent reviewer is available. This is process hardening, not an R1 pass.
