# M5 v21 — PPO action-intent coverage diagnostics

## Baseline and scope

Baseline is the current dirty M5 working tree, including PPO v11 training and the just-verified RealmView projection extraction. Do not reset or stage unrelated changes.

The PPO history currently records action-slot counts and selected-action counts grouped by the number of legal choices. Since each observation carries the encoded `Intent` for every candidate, add a per-Intent summary showing: legal opportunities, selected decisions, immediate reward average, GAE advantage average, and the fraction of selected decisions with positive advantage. This addresses the next diagnostic step after the v19/v20 full-world screens, where worksite setup actions occurred but material transfer and output remained rare.

## Before → after

Before: PPO history can show that a slot was selected, but not whether a semantic action was available or whether its reward/advantage signal was positive.

After: each update row includes versioned `action_intent_diagnostics`, derived from already-recorded observations/masks/actions/rewards/advantages. Training samples, policy logits, rewards, optimizer, checkpoints, and environment transitions remain byte-for-byte behaviorally unchanged.

## Files and invariants

- `ai/training/realm_train_v11.py`: helper to classify candidate Intent IDs from the documented observation feature; aggregate counts and signal summaries; include diagnostic version in manifest/history identity.
- `ai/tests/realm_ppo_action_diagnostics_checks.py`: synthetic deterministic helper checks, including unavailable actions and zero-positive-advantage cases.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`, and evidence report: explain diagnostic limits and record verification.

Intent IDs mirror `enum class Intent` in `core/include/polyec/realm.hpp`. Validate IDs are integral and in range before aggregation; fail with a clear error if observation encoding drifts. Opportunities count only legal candidate slots, and selected counts must sum to the number of decisions.

## Verification and limits

Run Python syntax, synthetic diagnostics checks, and a one-update training smoke that confirms row/manifest fields and existing exact-resume identity. Do not run another long training screen until these diagnostics have been used to select a controlled follow-up. Aggregate advantage/reward is correlational training evidence, not causal action value or proof of policy capability. Self-review must be identified as such.
