# M5 v22 — worksite action choices in controlled contexts

Plan: [worksite context audit](../design/M5-v22-worksite-context-policy-audit-plan.md). This reads three v20 Economic checkpoints that failed selection; none is eligible as a learned peer.

## Method

- Construct one deterministic world through the same C++ Realm binding and typed action path.
- Capture three snapshots: starting world; after creating a private Cloth workshop without Fiber; and after buying Fiber into personal inventory.
- For each v20 seed, calculate the masked categorical probabilities and take 256 fixed-seed categorical samples per snapshot. This matches runtime action selection; it is a one-snapshot diagnostic, not a return evaluation.
- Check all Realm invariants and verify Fiber transfer is unavailable before the input purchase and legal after it.

## Results

The Fiber-transfer choice was unavailable in the starting world and in the workshop without Fiber for all seeds. Once Fiber was held, it became legal. Each seed assigned 2.61–2.72% probability to that single transfer candidate; categorical sampling selected it 5, 7, and 6 times in 256 draws. In the same snapshot, the aggregate probability for creating another establishment was 23.5–24.1%, compared with 17.9–18.1% for Buy. Authorize and Work each received about 2.7% probability in the available-candidate set.

Interpretation: the policies have a low continuation preference for transfer/worksite actions in this controlled state and retain a substantial preference for creating more establishments. This is consistent with the weak production seen in full-world holdouts, but the snapshot does not prove the cause or estimate long-run outcomes. No checkpoint was selected or promoted.

## Verification and limits

- Python syntax and all three v20 checkpoint/manifest locks passed.
- Three seeds × three contexts; Realm invariants true and masked probabilities summed to one.
- Existing shared-core worksite command test passed; Windows Release Realm test covers authorization, work schedule, save/load continuation, payroll/production, and output withdrawal.
- Source and JSON hashes: [manifest](../../artifacts/m5-v22-worksite-context-policy-audit-2026-10-21-manifest.json); result: [audit data](../../artifacts/m5-v22-worksite-context-policy-audit-2026-10-21.json).
- Self-review by Codex; no independent reviewer.

Next: change the training distribution so Economic policies repeatedly experience material availability, authorization, paid shifts, and production under different partner compositions, then re-run sampled validation before attempting policy promotion. Keep full-world 100-world R1 and all remaining M5 gameplay/performance work open.
