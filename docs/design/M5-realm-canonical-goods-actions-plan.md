# M5 — chuẩn hóa action hàng hóa của Realm

Ngày 2026-10-21. Bằng chứng: [report](../reports/M5-realm-canonical-goods-actions-evidence-2026-10-21.md); checkpoint source: [manifest](../../artifacts/m5-realm-canonical-goods-actions-2026-10-21-manifest.json).

## Mục tiêu và ranh giới

Tạo một candidate Buy và Sell riêng cho mỗi canonical `GoodId` trong catalog 27 sản phẩm. Giữ nguyên vị trí candidate 0–31 và mọi byte save. Thêm candidate 32–71 cho 20 sản phẩm chưa có trong action interface lịch sử bảy hàng. Đổi model interface Realm thành `realm-v7` (72 action, 992 observation) và yêu cầu model contract mới. Bundle realm-v6 cũ giữ nguyên và không tương thích.

Lát này đưa sản phẩm canonical qua giao dịch Realm. Lát này không khẳng định mọi sản phẩm đã có producer/recipe, độ sâu thị trường, giá đã hiệu chỉnh hay policy đã train. Các nội dung đó vẫn thuộc gate M5.2 và learned-policy.

## File trong phạm vi

- `core/include/polyec/realm.hpp`: hằng schema/action/observation và contract.
- `core/src/realm.cpp`: ánh xạ slot xác định, legality/settlement theo canonical GoodId và observation features.
- `core/tests/test_realm.cpp`: lệnh Buy/Sell ở cấp command cho từng GoodId catalog, dùng fixture stock có ledger cân bằng.
- `native/include/polyec/model_manifest.hpp`: kiểm schema và dimension v7.
- `native/realm_contracts.cpp`: fixture tương thích manifest v7 và từ chối model lịch sử.
- `ai/training/realm_model_v7.py`, `ai/training/realm_train_v7.py`, `ai/training/realm_export_v7.py`: dimension policy mới và lối train/export có version riêng; giữ script và artifact realm-v6 làm lịch sử.
- `ai/tests/realm_training_checks.py`, `ai/tests/realm_parity_v7.py`, `ai/evaluation/realm_sampled_v7.py` cùng binding/model contract checks: kiểm output shape/schema/contract và chuẩn bị parity/sampling gate v7. Giữ công cụ v6 riêng làm lịch sử.
- `ai/evaluation/realm_agency.py`: lấy action count hiện hành từ C++ binding.
- `godot_project/ui/realm/realm_panel.gd`, `godot_project/tests/realm_smoke.gd`: yêu cầu và báo contract bundle v7.
- `godot_project/models/realm-v7/README.md`: ghi rõ thư mục bundle để trống tới khi policy thật qua train/export.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`, evidence report và artifact manifest.

## Contract

- Slot Buy 1–7 và Sell 8–14 giữ nghĩa và vị trí candidate. `Candidate.good` đổi từ legacy index sang canonical GoodId qua `kLegacyGoodToCanonical`.
- Slot Buy 32–51 và Sell 52–71 ánh xạ tăng dần theo canonical GoodId, loại các ID đã nằm trong `kLegacyGoodToCanonical`.
- Role không phải Economic giữ Hold ở slot 32–71; role mask vẫn chỉ do `legal` tính.
- Buy/Sell chỉ hợp lệ khi canonical GoodId nằm trong `[0, kGoodIdCount)`, cùng vùng, đủ stock/inventory/cash chính xác và thỏa capacity/firm hiện có. Execute dùng trực tiếp cùng chỉ số canonical.
- Candidate feature `good` chuẩn hóa theo `kGoodIdCount - 1`; feature market price/stock/actor inventory đọc cùng canonical GoodId. Chuẩn hóa đổi với tất cả action, được version trong schema/model contract mới.
- Model có 128 state + 72×12 candidate features = 992 observation values, 72 logits, hidden size 64. Schema `realm-v7`; hậu tố contract ghi thay đổi canonical-goods. Manifest v6 vẫn bị từ chối theo schema/dimension/contract.
- Save schema vẫn là 20 vì không đổi persistent state hoặc token serialization.

## Atomicity và trường hợp biên

Canonical ID sai phải bị từ chối trước khi đọc stock, price hoặc inventory. Sinh candidate phải phủ mỗi GoodId đúng một lần trong dải Buy và Sell, không đè nghĩa action khác. Slot 15–31 giữ nguyên. Buy/Sell bị từ chối phải giữ nguyên fingerprint, inventory, stock, cash của firm và actor. Candidate Economic ngoài catalog mapping cùng các slot mới của role khác là Hold.

## Gate

1. Realm tests Release/Debug: slot mapping xác định và chuyển stock/inventory Buy/Sell cho đủ 27 sản phẩm; GoodId sai và hết stock bị từ chối atomic; slot cũ giữ nghĩa; role mask sạch.
2. Native model-contract tests Release/Debug: nhận đúng bundle v7; từ chối realm-v6, shape sai, contract rỗng hoặc khác. Bỏ v6 dev override vì tensor 512×32 không chạy được trên interface 992×72.
3. Python training checks Release/Debug: observation 992/action 72, contract lấy từ C++ binding và exporter kiểm contract.
4. Build GDExtension và chạy Godot headless preflight/smoke; bundle v6 phải bị chặn fail-closed.
5. Chạy full Release/Debug CTest phù hợp cùng scoped diff/link checks; ghi rõ gate chưa chạy.

## Review và giới hạn

Plan được tác giả triển khai viết từ catalog, action code và model loader hiện hành. Chỉ tự review, không có reviewer độc lập. Interface mới chưa có model train; chỉ nghiệm thu learned-policy sau lượt train có ngân sách cụ thể. M5 vẫn mở.
