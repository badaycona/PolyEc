# Realm v7 pilot, parity và profile B 512 actor — 2026-10-21

## Phạm vi và kết luận

Đã train ba họ recurrent PPO v7 bằng cùng C++ `RealmBatch`, chạy sampled holdout hai seed, parity PyTorch/Python ONNX/C++ và benchmark Realm 100k/512 complex actors. Kết quả **không nghiệm thu learned-policy R1 hoặc M5.5**: cả ba họ đều không thắng đối chứng chiến lược; profile B p95 vượt mục tiêu và workload chưa có renderer/save/20-year.

Checkpoint và logs nằm trong `artifacts/models/realm-v7-pilot-2026-10-21/`; đây là pilot có `full_R1_pass=false`, không nằm trong `godot_project/models/realm-v7`. Model v6 trong `artifacts/models/realm-v6` có input 512 và không tương thích với Realm v7 (992 input/72 actions), nên không dùng nó cho benchmark v7 này.

## Training và holdout

Mỗi họ train ba PPO seeds, `envs=32`, `updates<=80`, ngân sách 120 s/seed, domain 9. Thời gian thực mỗi seed là khoảng 120–123 s; peak RSS training cao nhất 478 MB. Checkpoint selection chỉ dùng validation. Official sampled evaluator đã được sửa: trước đây report khai báo seed 170099 nhưng chỉ tạo RealmBatch với 170000. Bản sửa chạy thật cả 170000 và 170099; mỗi seed có 100 episode, common uniform stream, và report pooled 200 trajectories/mode. Mọi mode có 0 invalid action.

| Họ | Checkpoint chọn | Learned utility | Learned − initial (95% CI) | Learned − baseline (95% CI) | Kết quả |
|---|---:|---:|---:|---:|---|
| Governance | 3 | -0,161 | -0,014 `[-0,030; 0,000]` | -0,006 `[-0,018; 0,007]` | Không chứng minh cải thiện |
| Economic | 3 | -0,379 | 0,107 `[0,041; 0,176]` | -3,203 `[-3,383; -3,017]` | Thua baseline rõ |
| Civilian | 1 | -0,235 | 0,478 `[0,333; 0,626]` | -1,453 `[-1,704; -1,202]` | Thua baseline rõ |

Các metric là normalized terminal utility theo script, không phải coin. Economic vượt holdout baseline đơn giản của trainer trên seed 160000 nhưng thua baseline trên 170000/170099; giữ nguyên kết quả trái chiều, không chọn seed/test theo kết quả đẹp. Hold và random cũng được lưu cùng kết quả trong `runtime-sampling-audit.json`. Full R1 vẫn false.

## PyTorch–ONNX–native parity

`ai/tests/realm_parity_v7.py` chạy ba checkpoint đã train: 507 checks tổng; batch 1/3/32/64, 8 bước recurrent mỗi batch, action mask và reject NaN/missing model. Python ONNX Runtime 1.31.0 và C++ `RealmNativePolicy` khớp PyTorch trong `atol=1e-5`, `rtol=1e-3`:

| Họ | Max absolute error logits | Max absolute error memory | Native inference p95 |
|---|---:|---:|---:|
| Governance | 2,98e-7 | 1,79e-7 | 1,950 ms |
| Economic | 1,19e-7 | 2,09e-7 | 1,853 ms |
| Civilian | 5,96e-7 | 2,68e-7 | 2,186 ms |

Đây là parity model plumbing; kết quả không đảo ngược thất bại quality trên.

## Profile B population và actor count

Thêm `complex_actors` tuỳ chọn cuối constructor/runtime. `0` giữ genesis cũ; tường minh phải chẵn trong 128–2048, không vượt dân số; nửa đầu governance, nửa sau economic. Save schema và model contract không đổi. Realm test xác nhận default 128, explicit 512 = 256+256, invalid count bị reject, invariant và sổ tiền đúng.

Cùng Release build, seed 37, dân số 100.000, 31 ngày, cùng ba checkpoint v7 pilot, route shock ngày 30. Chạy thêm ba lần sau lần đầu cho thấy tail latency biến động lớn giữa các lần chạy; mỗi số p95 dưới đây là thống kê của 31 ngày (p95 nằm sát ngày chậm nhất), không phải ước lượng ổn định của tải thường. Cả tám lần giữ population 100.000, `money_conserved=true`, `invariant=true`, `route_shock_applied=true`:

| Complex actors | Inferences | p95 ngày các lần chạy | Median p95 | Max quan sát |
|---:|---:|---:|---:|---:|
| 128 (mặc định) | 103.484 | 143,319–287,534 ms (4 lần) | 145,932 ms | 299,070 ms |
| 512 | 104.098 | 151,182–287,605 ms (4 lần) | 284,231 ms | 298,140 ms |

Ba lượt sau khi cập nhật harness lần lượt đo p95 128 actors là 145,389 / 146,474 / 287,534 ms và 512 actors là 151,182 / 284,620 / 283,841 ms. Số lần đo chưa đủ để quy biến động cho actor count hay xác định nguyên nhân; cần profiler và workload nhiều ngày/lần lặp trước khi cân chỉnh performance. Mọi lần đều có đủ 103.484/104.098 inference tương ứng, nên số ngày chạy và số quyết định không đổi.

Các run được gắn `B-population-scale-only; manifest_R1_false`. Đo gồm day core/observation/inference/validation/commit; không gồm renderer, save/load, focus/pause hoặc long-run. Tất cả p95 ghi nhận (151,182–287,605 ms) đều vượt mục tiêu ngày thường ≤80 ms; workload cũng chưa là 24+3 regions. Đo không phải full profile B acceptance.

Raw outputs: lần đầu [128 actor](../../artifacts/m5-t5.1-profile-b-v7-pilot-100k-128-2026-10-21.json), [512 actor](../../artifacts/m5-t5.1-profile-b-v7-pilot-100k-512-2026-10-21.json); chạy lại [128 actor](../../artifacts/m5-t5.1-profile-b-v7-pilot-100k-128-2026-10-21-rerun.json), [512 actor](../../artifacts/m5-t5.1-profile-b-v7-pilot-100k-512-2026-10-21-rerun.json), [repeat 2/3 — 128](../../artifacts/m5-t5.1-profile-b-v7-pilot-100k-128-2026-10-21-repeat2.json), [repeat 2/3 — 512](../../artifacts/m5-t5.1-profile-b-v7-pilot-100k-512-2026-10-21-repeat2.json), [repeat 3/3 — 128](../../artifacts/m5-t5.1-profile-b-v7-pilot-100k-128-2026-10-21-repeat3.json), [repeat 3/3 — 512](../../artifacts/m5-t5.1-profile-b-v7-pilot-100k-512-2026-10-21-repeat3.json), [sampled evaluator](../../artifacts/models/realm-v7-pilot-2026-10-21/runtime-sampling-audit.json), [native parity](../../artifacts/models/realm-v7-pilot-2026-10-21/native-parity.json). Sau khi harness đọc cờ R1 từ manifest, JSON mới ghi `model_manifest_full_R1=false` và `model_quality_validation=not_performed_by_benchmark`. Checkpoint file hashes: [manifest](../../artifacts/m5-v7-pilot-training-profile-b-manifest.json). Scratch Python dependencies chỉ nằm trong `build/validation-python-deps`; không thay dependency dự án.

## Test và giới hạn

Windows Release `polyec_realm_benchmark` build pass; `polyec_realm_tests` sau bổ sung actor save/load: **24.422 checks pass** ở Release và Debug. Full CTest tích hợp **17/17 Release** và **17/17 Debug** pass trước ba assertion cuối; các assertion cuối được chạy trong direct Realm suite hai cấu hình. Python extension được rebuild Release/Debug; mỗi cấu hình `realm_training_checks.py` **476 checks** và `realm_binding_checks.py` **259 checks / 64 ngày** pass; parity model v7 chạy lại với extension hiện tại **507 checks** pass. GDExtension Windows Release build pass. Các benchmark JSON parse/validate được; dân số, conservation, invariant và route shock đều đúng. Godot headless scene chưa chạy trong lượt này; thay đổi benchmark không đổi Godot API/flow nhưng đây không được tính là Godot runtime gate. Không có reviewer độc lập; đây là self-review. Không commit/push.
