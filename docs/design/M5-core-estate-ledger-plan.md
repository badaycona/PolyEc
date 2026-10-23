# M5 core — hồ sơ di sản và save schema 24

## Contract

Khi Person chết, chuyển cash và inventory còn đứng tên cá nhân vào một hồ sơ di sản theo ID người chết; ghi ngày, vùng, các land title, establishment cá nhân, land-use rights do người đó cấp và loan record còn gắn với họ. `estate_` vẫn là tổng cash trong các hồ sơ cộng phần estate aggregate cũ được di trú. Household fund giữ nguyên là tài khoản chung, không đưa vào tài sản cá nhân.

Hồ sơ chỉ giữ quyền sở hữu/claim hiện có và bảo toàn tài sản; chưa thanh toán loan, chia cho người hưởng hoặc chuyển title/xưởng. Estate beneficiary resolver là nguồn xác định tier ở bước sau. Chưa biến loan thành nghĩa vụ của người thừa kế.

## Files / ownership

- `core/include/polyec/realm.hpp`: `EstateCase`, vector case, legacy aggregate marker và read-only accessor.
- `core/include/polyec/managers/estate_manager.hpp`, `core/src/realm_lifecycle.cpp`: prepare/apply tử vong nguyên tử; validate Person, estate cash và case read-set.
- `core/include/polyec/estate_section_codec.hpp`, `core/src/realm.cpp`: schema24 `ESTATE_1`; schema7–23 migrate old aggregate thành unassigned legacy estate balance; write/read case vectors.
- `core/src/realm.cpp`: goods/money/ownership-reference invariants include estate subledger; totals không đếm đôi cash/assets.
- `core/tests/test_managers.cpp`, `core/tests/test_realm.cpp`: exact read-set, asset references, inventory conservation, legacy/new schema roundtrip, corrupted/stale rows, living assets untouched.

## Invariants

1. `estate_ == legacy_unassigned_estate + sum(case.cash)`; case IDs là unique deceased Person IDs.
2. Person cash/inventory bằng zero sau khi tài sản được đưa vào case; case inventory cộng vào goods conservation, cash không tính hai lần.
3. Mỗi case tham chiếu đúng owner decedent: land title, Person-owned site, land-use grantor và debtor của loan; household-fund account không thuộc case.
4. Failure khi chuẩn bị hoặc apply stale case read-set không đổi Person, Province, estate cash hoặc case vector.
5. Schema7–23 load được; schema24 save/load byte-stable theo token/case order, fingerprint và invariant.

## Kiểm chứng

Windows Release/Debug manager tests, Release full CTest, Python binding replay/save migration, GDExt build và Godot headless smoke. Full R1/policy không được gọi là pass từ slice ledger này. Codex tự review; không independent reviewer.
