# M5 v21 — PPO action-intent coverage evidence (2026-10-21)

Plan: [action-intent diagnostics](../design/M5-v21-action-intent-coverage-plan.md). This checks logging only; it is not an evaluation of learned-policy quality.

## Delivered

- Each PPO update records legal candidate opportunities, selected decisions, immediate reward mean, GAE advantage mean, and positive-advantage fraction for all 29 `Intent` values.
- The diagnostic version is recorded in run manifest and checkpoint identity. The summarizer rejects observation shape, action mask, selected-action legality, or Intent encoding drift.
- No rollout sampling, observation data, reward, model, optimizer, or C++ transition changed.

## Windows verification

- Python syntax compilation passed.
- Synthetic aggregation checks passed: 29 Intent values and 9 assertions, including no-opportunites/no-selection and invalid encoded Intent rejection.
- A 1-update sampled PPO smoke produced a row for 1,024 decisions with semantic coverage summaries.
- Resumed that checkpoint to update 2 with the same identity; resume succeeded and produced the second summary.
- The local default Python lacked ONNX. The already-present `build/validation-python-deps` was added to `PYTHONPATH`; no dependency was installed.
- Self-review by Codex; no independent reviewer.

## Observation and next action

In the 1-region, 2-day smoke, creating establishments was selected 315 times, while site authorization and work-at-site were selected 4 and 5 times. No material-transfer candidate was available in that short run. This is too small and too short to infer policy quality.

The controlled shared-core Python check isolates availability: before a site exists, the Fiber-transfer action is unavailable; after a Cloth site exists but before Fiber is in personal inventory, it is still unavailable; after buying Fiber into personal inventory, the candidate becomes legal. Its observation identifies `TransferEstablishmentGoods`, the Cloth industry, same-region access, and positive personal Fiber stock. The real command deposits the input successfully. The corresponding C++ Realm test then authorizes a present worker, accepts a one-day work schedule, saves and reloads, advances payroll/production, verifies one Fiber becomes one Cloth, and withdraws the Cloth. The added no-Fiber assertion passed in Windows Release `realm_tests` (1/1 CTest target); the Python controlled binding check also passed. This confirms why a rollout may have no transfer opportunity: the policy must first create a site and hold a recipe input. The remaining question is how policies learn this sequence against varied peers; do not start another long PPO screen before controlled-context coverage is extended.
