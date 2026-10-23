# M5.5 / Core — tái dùng candidate trong decision snapshot: evidence (2026-10-21)

Plan: [tái dùng candidate snapshot](../design/M5-core-runtime-observation-candidate-reuse-plan.md). Baseline source `dff2781a41ae52a4bd476b4b1460ad0f09ef5dc3`; checkout đang dirty bởi nhiều lát M5/core, không commit/push. Self-review, không có reviewer độc lập.

## Thay đổi

- `Realm::observation(actor, snapshot)` tạo một candidate array và lấy mask từ chính array đó; API observation/mask cũ giữ nguyên qua wrapper. Runtime giữ snapshot đến sau inference và revalidate bằng candidate/mask mới trước mỗi commit.
- Batch reserve observation, memory, candidate và mask vectors trước khi nạp. Không cache qua command, không bỏ revalidation, không thay thứ tự action/RNG/transaction.
- CMake option `POLYEC_ENABLE_REALM_PHASE_PROFILE` mặc định OFF đo phase Realm/runtime chỉ trong build chẩn đoán. Binding Python chỉ rõ overload API công khai cũ sau khi compiler báo con trỏ hàm mơ hồ.
- `decision_snapshot_tests` so sánh mọi candidate field, observation values, mask và feature mask cho Governance/Economic/Civilian trước/sau mutation: 451 checks.

## Gate

| Gate | Kết quả |
|---|---|
| Windows full Release build, gồm GDExtension | pass |
| Windows full Release CTest | 19/19, 44.41 giây |
| Windows full Debug build, gồm GDExtension | pass |
| Windows full Debug CTest | 19/19, 154.35 giây |
| Candidate snapshot direct tests | Release 451/451; Debug 451/451 |
| Python binding + C++ replay | Release/Debug 263 checks, 64 ngày mỗi cấu hình |
| Learned training preflight | Release/Debug 476 checks mỗi cấu hình |
| v7 PyTorch–ONNX Runtime–C++ parity | 507 checks; batch 1/3/32/64; logits max abs `5.96e-7`, memory max abs `2.69e-7` |
| Godot 4.5.1 headless Realm/session/UI smoke | 473/473; model pilot + fixture chuyển title 64 ngày tường minh |
| Benchmark A, Release, 10k, 31 ngày, seed 37 | p95 15.7568 ms; `money_conserved`, invariant, timing relation pass |

## M5.5 chẩn đoán sau thay đổi

Ba lượt benchmark Release không instrumented cho mỗi workload, 100k người, 31 ngày, seed 37, cùng pilot v7/R1=false. Mọi lần có population 100k, money/invariant pass, đủ inference, route shock ngày 30; output và model lineage nằm trong manifest. p95 mỗi sample 31 ngày và chịu nhiễu máy:

| Complex actors | Ba p95 (ms) | Median p95 | Mục tiêu ngày thường |
|---:|---:|---:|---|
| 128 | 240.705, 154.789, 270.511 | 240.705 | ≤80 ms — fail |
| 512 | 242.808, 246.494, 267.776 | 246.494 | ≤80 ms — fail |

Một profile build có instrumentation ghi observation pack 1.255 s/31 ngày ở 128 actors và 2.457 s ở 512; policy inference, action validation, Realm advance và invariant cũng tốn đáng kể. Đây là một lần chẩn đoán mỗi workload, timer làm đổi latency, không phải bằng chứng tăng tốc hay profile B acceptance. Bản instrumented có `phase_times_consistent=true`, money/invariant true. WPR CPU sampling trước đó không khởi động được do Windows 0xc5585011 (`Failed to enable the policy to profile system performance`); không coi đó là profile.

Candidate/mask oracle, full Release/Debug regression, Python parity/replay, inference parity và Godot smoke giữ nguyên; không có sai khác behavior được quan sát. Snapshot reuse không đóng M5.5: p95 hiện tại vượt 80 ms, bundle R1=false, và workload chưa bao gồm renderer, save/load, 24+3 vùng, long-run hay package offline. Tiếp tục phân tích hotspot và tối ưu có oracle; không gắn pilot thành model sản phẩm.

Raw outputs: [10k A](../../artifacts/m5-t5.1-snapshot-release-10k-2026-10-21.json), [100k/128 lượt 1–3](../../artifacts/m5-t5.1-snapshot-release-100k-128-2026-10-21.json) / [lượt 2](../../artifacts/m5-t5.1-snapshot-release-100k-128-repeat2-2026-10-21.json) / [lượt 3](../../artifacts/m5-t5.1-snapshot-release-100k-128-repeat3-2026-10-21.json), [100k/512 lượt 1–3](../../artifacts/m5-t5.1-snapshot-release-100k-512-2026-10-21.json) / [lượt 2](../../artifacts/m5-t5.1-snapshot-release-100k-512-repeat2-2026-10-21.json) / [lượt 3](../../artifacts/m5-t5.1-snapshot-release-100k-512-repeat3-2026-10-21.json), [phase 128](../../artifacts/m5-t5.1-phase-after-snapshot-100k-128-2026-10-21.json), [phase 512](../../artifacts/m5-t5.1-phase-after-snapshot-100k-512-2026-10-21.json). Hash checkpoint: [manifest](../../artifacts/m5-core-runtime-observation-candidate-reuse-2026-10-21-manifest.json).
