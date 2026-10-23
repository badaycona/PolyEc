# M5 — nhiều lựa chọn người nhận hôn nhân trong policy contract

## Vấn đề được xác nhận

`Realm::candidates()` đặt một action `Marry` ở slot 72 với target suy ra trực tiếp từ actor ID. Model vì vậy không thể so sánh nhiều người nhận trong cùng quyết định; feature target ID còn bị chuẩn hóa sai miền. Review PPO coi đây là thiếu coverage của action/observation.

## Phạm vi lát đầu

- `core/include/polyec/realm.hpp`: chuyển policy schema lên `realm-v9`, thay một target slot cũ bằng 8 target slot ở chỉ số 72–79, tổng 80 action/1.088 observation, và cập nhật model contract để checkpoint v8 bị từ chối.
- `core/src/realm.cpp`: tạo tối đa 8 target khác nhau bằng mẫu có giới hạn, xác định từ ID actor và bộ đếm sampling; không thay world state. Core legal mask vẫn quyết định target nào thật sự hợp lệ. Feature hôn nhân đưa tuổi và vùng công khai của target, không đưa tiền/hộ/quan hệ riêng.
- `native/include/polyec/model_manifest.hpp`, `native/realm_contracts.cpp`, Python binding checks: kiểm kích thước/schema mới và từ chối manifest v8.
- `ai/training/realm_model_v9.py`: cùng kiến trúc recurrent candidate scorer với kích thước contract v9; chưa chạy training vì action xưởng M5 chưa được nối vào policy.
- `core/tests/test_realm_court.cpp`, focused observation/model contract checks: nhiều target phân biệt, mask hợp lệ, feature không bão hòa theo ID, đề nghị/chấp thuận/escrow vẫn qua cùng command.

## Invariants

- Tám target khác nhau, không bao gồm actor; không sửa Person, tiền, proposal hay RNG khi dựng candidates/observation.
- Chỉ core legality cấp mask. Target không hợp lệ không thể được gửi tới commit.
- Snapshot candidate phải bằng candidate mới dựng khi revalidate; stale target bị từ chối như các action còn lại.
- Các policy v8 hiện có không được nạp với schema/shape mới. Không tạo hoặc gọi model v9 là trained trước khi training và R1 nghiệm thu.
- Lát này chỉ sửa chọn target hôn nhân. Chưa bao phủ worksite commands, learned partners, cross-play hay full-world.

## Gate

1. Release/Debug core tests cho court, candidate snapshot, action mask và contract.
2. Rebuild Python binding; kiểm REALM_SCHEMA/ACTIONS/OBS và v8 manifest rejection.
3. Chạy công cụ Python parity liên quan và Godot compatibility guard để xác nhận old model fail-closed.
4. Self-review; cập nhật evidence/manifest. Chưa huấn luyện dài.
