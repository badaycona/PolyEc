# M5 learned policy — versioned model bundle contract loader

Ngày 2026-10-21. Plan: [model contract bundle loader](../design/M5-model-contract-bundle-loader-plan.md); checkpoint [manifest](../../artifacts/m5-model-contract-bundle-loader-2026-10-21-manifest.json). Codex self-review, không có reviewer độc lập. Không train, commit hoặc push.

## Thay đổi

- Runtime đọc `manifest.json` cạnh mỗi ONNX; kiểm family theo thứ tự governance/economic/civilian, schema `realm-v6`, observation/actions/hidden `512/32/64`, SHA-256 format, `trained_with_rl=true`, `imitation=false`, và `model_contract` trùng C++ Realm.
- Pilot cũ không có `model_contract` bị từ chối mặc định. Chỉ explicit development override mới nạp được manifest legacy; override không cho qua manifest khai contract sai, family sai, hay bundle trộn legacy/current. Restore kiểm contract với model đã load.
- Trainer lấy nhãn từ shared C++ Realm qua Python binding. Exporter từ chối checkpoint cũ thiếu contract hoặc khác contract. Không sửa ba manifest realm-v6 hiện có.
- GDExtension công bố model contract của core; Godot UI preflight từ chối bundle không khớp trước khi nạp. GDExtension tiếp tục so SHA-256 từng ONNX với giá trị trong manifest.

## Bằng chứng chạy thực

- Release native targets `polyec_realm_contracts` và `polyec_realm_benchmark` build; CTest `runtime_contracts` **1/1 pass**. Có kiểm tra metadata contract hợp lệ, family mismatch, contract mismatch kể cả khi bật dev override, contract rỗng, legacy pilot bị từ chối mặc định, và restore/dev override.
- Python binding Release và Debug build. `model_contract_binding_checks.py`: **13 checks pass** mỗi cấu hình; xác nhận C++ trả `realm-household-fund-v1`, ba manifest lịch sử không bị sửa và vẫn ghi `full_R1_pass=false`.
- Python syntax compile cho `realm_train_v6.py` và `realm_export.py` pass.
- GDExtension Release `polyec` build pass. Godot 4.5.1 headless guard: **5/5 pass**, gồm query model contract và lỗi runtime chính xác khi thử nạp bundle lịch sử. Project main scene khởi động headless 2 frame và exit 0, không báo GDScript parse/runtime error.
- `git diff --check` scoped pass; trailing-whitespace scan cho file mới pass; `scripts/check-repository-links.py`: **837 local references passed**.

## Giới hạn

Không có model đã huấn luyện/đánh giá dưới contract hiện hành. Kiểm tra metadata không chứng minh ONNX weights được train đúng; chỉ pipeline huấn luyện, holdout quality gates R1–R3 và parity mới xác nhận điều đó. Native manifest reader kiểm định dạng digest; đường Godot sản phẩm mới thực hiện so sánh digest bytes. Native benchmark vẫn là development-only. App chưa được nghiệm thu bằng model tương thích mới, và không chạy training trong lát này.

Một build GDExtension đầu tiên gặp `LNK1104` do tiến trình Godot headless smoke trước đó còn giữ DLL mở sau khi in kết quả. Xác minh command line cho thấy chỉ hai process do smoke này sinh ra; dừng đúng hai process đó, build lại và hoàn tất thành công. Không dừng app/process khác.
