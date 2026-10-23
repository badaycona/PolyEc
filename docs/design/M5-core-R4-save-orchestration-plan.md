# Kế hoạch R4 — tách điều phối save/load Realm khỏi phần mô phỏng

Ngày 2026-10-22. Đây là một lát cấu trúc trong R4 của [hợp đồng refactor core](core-architecture-refactor.md), tiếp nối các codec section và bộ chiếu `RealmView` đã được tách. Không thay luật chơi hoặc định dạng lưu game.

## Mục tiêu và hành vi

Các hàm `Realm::serialize`, `Realm::deserialize`, `Realm::save`, `Realm::load` và `Realm::fingerprint` hiện nằm ở cuối `core/src/realm.cpp`, cùng translation unit với command, mô phỏng, giao dịch và invariant. Chuyển nguyên các định nghĩa này sang `core/src/realm_save.cpp` để lưu/tải có một translation unit riêng. Realm vẫn là chủ sở hữu dữ liệu và các codec vẫn chịu trách nhiệm đọc/ghi từng section; chỉ nơi đặt phần điều phối thay đổi.

Người chơi phải tiếp tục lưu, tải, di chuyển save schema 7–25 và so sánh fingerprint như trước. Không sửa thứ tự section, tên schema, migration, checksum, kiểm invariant hoặc cách tải nguyên tử vào Realm đang chạy.

## Phạm vi file

- `core/src/realm.cpp`: bỏ các định nghĩa save/load/fingerprint đã chuyển; giữ các hàm mô phỏng và command.
- `core/src/realm_save.cpp`: thêm các định nghĩa thành viên được di chuyển và helper checksum nội bộ.
- `core/CMakeLists.txt`: thêm translation unit mới vào `polyec_core`.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`: cập nhật sau khi kiểm tra đạt.
- Tạo báo cáo kỹ thuật đầy đủ và manifest SHA-256 cho đúng các file thuộc lát này.

## Giới hạn giữ nguyên

- Không sửa nội dung các hàm hoặc thuật toán; helper checksum giữ cùng thuật toán FNV-1a 64-bit và cùng kiểu số.
- Không đổi public header/API, save schema/bytes, binding, action, observation, reward, model hoặc gameplay.
- Không thêm một codec tổng quát mới, không sao chép Realm state, không chạy cả profile dân số hoặc package.
- Giữ mọi thay đổi chưa commit trong checkout; không commit/push.

## Kiểm tra

Ngay sau khi tách: build Windows Release targets `polyec_realm_tests` và `polyec_manager_tests`; chạy hai executable trực tiếp. Realm test phải xác nhận roundtrip, schema migration, reject save hỏng không ghi đè thế giới đích và continuation. Manager test là kiểm tra biên phụ thuộc trực tiếp đến chuỗi di sản mà checkout hiện đang phát triển. Kiểm tra `git diff --check` chỉ trên file thuộc lát này.

Chưa chạy full CTest, Debug, Python/native/GDExt/Godot, model/policy, benchmark hoặc package vì không đổi API/schema/adapter và đây là thao tác di chuyển code. Báo cáo phải nói rõ source revision này chưa được các lớp đó kiểm.
