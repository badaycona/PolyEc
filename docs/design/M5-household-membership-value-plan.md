# M5 HouseholdMembership value composition — approved slice

Ngày review: 2026-10-20. Đây là một lát cấu trúc nhỏ cho quan hệ Person–nhóm hộ hiện có, theo `docs/IMPLEMENTATION_PLAN.md` M5.0/1 và `docs/design/core-class-reference.md` §§3.2, 8.5, 13.1.

## Phạm vi

- Thay scalar công khai `Person::household` bằng `Person::household_membership`, một value record `HouseholdMembership { uint64_t household_id=0; }`.
- `household_id` giữ nguyên nghĩa hiện tại: nhãn hộ mang ID người sáng lập; `HouseholdIndex` vẫn là chỉ mục dẫn xuất nhãn → danh sách member ID tăng dần.
- Giữ nguyên API Realm `household_of`, `household_members`, `same_household`, `household_authority`, mọi quyền/transfer, sinh và authority hiện hành.
- Đây là thay đổi C++ source-level cho truy cập field trực tiếp trên `Person`; mọi call site được cập nhật. Không đổi Python binding vì binding hiện không expose `Person::household`.
- Không tạo Household owner entity, household fund, quyền mới, lifecycle mới, typed ID riêng hay luật gameplay.

## Ownership, defaults và validation

Realm tiếp tục sở hữu vector Person và `HouseholdIndex`. `Person::household_membership.household_id` mặc định 0 như field cũ; validation hiện có (nhãn khác 0, không vượt số người) giữ nguyên. Index vẫn dùng scalar `uint64_t` làm map key; không sở hữu Person.

## Save compatibility

Realm save là luồng văn bản có token tách bằng whitespace, không phải binary memory dump. Serializer/deserializer tiếp tục ghi/đọc scalar `household_id` đúng vị trí token cũ của `Person::household`; giữ nguyên header/schema, thứ tự token, spacing và API save. Không phụ thuộc layout byte trong `Person`.

## Files

- `core/include/polyec/realm.hpp`: định nghĩa value record, cập nhật field `Person` và comment current-state dùng `Person::household`.
- `core/include/polyec/managers/household_index.hpp`: đọc `household_membership.household_id`, giữ validation/key behavior.
- `core/src/realm.cpp`: mọi khởi tạo, copy khi sinh, truy vấn, authority, invariant và serialize/deserialize.
- `core/tests/test_managers.cpp`, `core/tests/test_schedule.cpp`: fixtures và fingerprint text dùng nested member, giữ thứ tự token.
- `gdext/polyec_session.cpp`: đọc nested member khi tạo PersonView, giữ nguyên tên/key Godot `household`.
- `native/realm_benchmark.cpp`: thống kê singleton household đọc nested member.
- `docs/design/core-class-reference.md`: cập nhật Person field, HouseholdIndex source field và quan hệ hiện trạng.

Không có direct `Person::household` access trong `bindings/python/realm_binding.cpp`; Python không expose field này. `gdext` giữ nguyên key `household` trong PersonView. Integration behavior ở Realm tests vẫn là regression gate.

## Gates

- Release build + full CTest dùng `build/m5-core`.
- Debug build + targeted CTest `realm_tests|schedule_tests|manager_tests|realm_clock_replay` dùng `build/m5-core`.
- Build `polyec_realm_benchmark` và `polyec_realm_contracts` trong `build/m5-runtime`; chạy CTest `runtime_contracts`.
- Python `polyec_env`/`polyec_realm_transition_replay` build và clock binding replay Release + Debug trong `build/m5-python-ai` để kiểm tra consumer C++/Python hiện hành.
- Giữ save/fingerprint oracle hiện có; `git diff --check`; kiểm tra không còn direct `.household` field access (ngoại trừ tên trong comment nếu chưa cập nhật).

## Ngoài phạm vi

Không đổi HouseholdIndex key type/API, C++ Realm query signatures, Python keys, save schema/token order, money, decision rights, support/guardian rules, household boundaries, migration, ownership, land, campaign, action/observation/reward/model. Không commit/push.
