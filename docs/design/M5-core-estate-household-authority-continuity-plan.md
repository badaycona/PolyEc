# Kế hoạch M5 — quỹ hộ khi người có quyền rút qua đời

Ngày 2026-10-22. Tài khoản quỹ hộ và death settlement đã có các contract riêng. Lát này kiểm tra đúng phần giao nhau đã được quy định: số dư được gắn với household label; quyền rút do người trưởng thành hiện giữ `household_authority` quyết định; death settlement chuyển tài sản cá nhân và không sở hữu quỹ chung.

## Hành vi cần chứng minh

1. Fixture chọn một household head là người trưởng thành, đang có quyền rút, và có ít nhất một người trưởng thành khác còn sống cùng household label.
2. Chuyển thật 37 tiền từ Province firm vào fund trước death, cân đối tổng tiền; cho head chết ở pha Realm đã dùng.
3. Sau pha death, fund vẫn là 37 dưới cùng household label. Người lớn còn sống có quyền rút mới; tiền fund không xuất hiện trong hồ sơ di sản tiền cá nhân.
4. Người có quyền mới rút 5 qua `HouseholdFundCommand`; số dư fund còn 32, đúng người nhận thêm 5, total money và invariant không đổi.
5. Save/load bảo toàn fund, người có quyền rút, hồ sơ di sản và fingerprint.

Không thêm luật về việc ai được quản lý tiền riêng của trẻ; không thay household membership, tiêu chuẩn authority, estate distribution hoặc schema.

## Phạm vi và kiểm tra

- Chỉ thêm test fixture vào `core/tests/test_realm.cpp`, plan, report, CURRENT_TASK/HISTORY và manifest.
- Build Release `polyec_realm_tests`, chạy trực tiếp executable. Các suite Release/Debug đầy đủ vừa pass sau lát Realm fixture gần nhất; không lặp toàn suite cho test-only addition.
- Kiểm tra invariant/tổng tiền trước và sau death/withdrawal, exact authority, exact household balance, estate cash, save/load.
- Không chạy Python/native/GDExt/Godot vì không đổi production source/schema/API/binding.
