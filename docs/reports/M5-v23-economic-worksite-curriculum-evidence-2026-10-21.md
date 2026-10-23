# M5 v23 — Economic worksite curriculum screen

Plan: [curriculum screen](../design/M5-v23-economic-worksite-curriculum-plan.md). This is a diagnostic training screen, not model acceptance.

## Delivered and checked

- Added optional `economic-worksite-reset-v1`; default `none` keeps existing worlds unchanged. For Economic episodes, four consecutive reset types cycle evenly: normal Realm; funded Cloth workshop without Fiber; workshop with personally held Fiber; and an owner whose authorized worker has a legal site Work candidate.
- Setup uses C++ Realm candidate/typed-command paths, then checks accounting invariants. Curriculum version and scenario labels are saved in environment state, trainer checkpoint identity, training manifest/set, and selector comparison. PPO history now groups action/reward/advantage diagnostics by the episode's reset context.
- Environment checks: 17 passed across 16 contexts; frozen-peer environment: 22; action-diagnostic aggregation: 9. One-update smoke had 1,024 decisions; exact contract resume continued to update 2.
- Training completed: three Economic seeds ×25 updates ×25,600 decisions, with 16 environments, 64-step rollout, 16-sequence mini-batches, 3 epochs, LR 0.0003, reward v19, and dev-baseline partners. The curriculum reset distribution was the only experiment factor.
- Sampled selection: 20 new worlds (193000–193019) ×128 days; invariants passed. Selector returned no eligible seed; exit 1 means no checkpoint satisfied the existing rule. No model was promoted and R1 was not run.

## Results

| Seed | Utility vs baseline (95% CI) | Utility vs initial | Cash delta | Debt delta | Insolvencies added | Workshops | Paid site work units | Fiber / Cloth at end |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | -0.427 [-0.480, -0.375] | +0.010 (CI crosses 0) | -350 | +110 | +0.60 | 21.9 | 9.4 | 0.30 / 0.05 |
| 2 | -0.463 [-0.514, -0.411] | +0.012 (CI crosses 0) | -373 | +120 | +0.75 | 23.6 | 11.2 | 0.15 / 0.10 |
| 3 | -0.434 [-0.488, -0.379] | +0.019 (CI crosses 0) | -345 | +94 | +0.50 | 22.8 | 8.2 | 0.00 / 0.10 |

The reset contexts produced many legal worksite choices during training; for example, the three seeds recorded 794/858/1,030 TransferEstablishmentGoods decisions in natural/no-input/with-Fiber reset contexts respectively (the “no-input” label is the episode's starting context, so later actions can change its stock). Still, full-world validation showed about 22 workshops per world, only 8–11 paid site-work units, almost no Fiber/Cloth at the end, lower cash, more debt and more insolvencies. The policy still does not improve reliably over initial weights or baseline.

The v23 validation worlds are new and disjoint from v20's set; compare the pass/fail and failure patterns, not small differences in their aggregate means. These checkpoints remain `peer_eligible=false`.

## Evidence and limits

**Ghi chú v24:** đánh giá lại source C++ xác nhận slot `>=89` hiện chỉ chứa các lệnh xưởng; bộ đếm cũ đúng theo layout v11. Decoder theo Intent được thêm để giảm phụ thuộc vào vị trí slot. Lượt chạy lại trên cùng 20 world cho các con số và kết luận không đổi. Giả thuyết ban đầu về lệnh kết hôn ở slot cao dựa trên fixture tổng hợp không thể phát sinh từ Realm và đã được rút lại. Xem [báo cáo v24](M5-v24-semantic-worksite-counts-evidence-2026-10-21.md).

- Full runset, per-update diagnostics, selection JSON, and source hashes: [manifest](../../artifacts/m5-v23-economic-worksite-curriculum-2026-10-21-manifest.json); [selection data](../../build/m5-v23-economic-curriculum-selection/selection.json) is in the local build directory.
- This was self-review by Codex; no independent reviewer. It does not satisfy learned partner diversity, full-horizon/100-world R1, or remaining M5 gameplay and population/performance/package requirements.
- Do not start another general training increase from this result. Next work must inspect why full-world actors spend cash and accrue debt without running enough paid production, using the existing per-context action and accounting evidence.
