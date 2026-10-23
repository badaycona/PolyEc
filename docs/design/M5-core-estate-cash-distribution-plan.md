# Core — chia tiền mặt di sản sau khi trả nợ

Ngày 2026-10-21. Plan tiếp theo trong [CURRENT_TASK](../CURRENT_TASK.md), theo `IMPLEMENTATION_PLAN.md` §2: trả nghĩa vụ trước, rồi chia phần còn lại bằng nhau cho nhóm người hưởng ưu tiên đầu tiên; không có người hưởng thì đưa tiền vào quỹ di sản công có sổ.

## Trước và sau

- Trước: khi một người chết, tiền mặt và khoản vay còn dư được ghi vào `EstateCase`; các khoản vay chỉ được trả trọn vẹn nếu tiền mặt trong chính hồ sơ đủ trả toàn bộ. Tiền còn trong hồ sơ chưa được chuyển cho ai.
- Sau: ngay sau khi quyết toán khoản vay, `EstateManager` dùng resolver hiện có để chọn con hợp pháp, phối ngẫu, cha mẹ hoặc anh chị em còn sống theo thứ tự. Tiền được chia bằng nhau; phần dư do chia nguyên được cộng lần lượt cho người có ID nhỏ nhất. Người chết, người không còn sống và quan hệ cha–con chưa được xác nhận không nhận tiền. Nếu không có người hưởng, toàn bộ tiền mặt của hồ sơ được chuyển sang số dư quỹ di sản công tổng hợp (`legacy_estate_unassigned_`), không bị mất khỏi tổng tiền.
- Hồ sơ giữ nguyên để tiếp tục theo dõi hàng hóa, đất, quyền sử dụng đất, khoản vay và xưởng. Tiền mặt trong hồ sơ về 0 sau khi chuyển. Khoản nợ thiếu tiền vẫn nằm ở khoản vay của người chết và không trừ vào phần người hưởng.
- Tiền của người hưởng được ghi vào đúng tài khoản cá nhân; không chạm quỹ hộ, tài khoản người khác, Province hay tổ chức. Chưa thêm quyền guardian chi hoặc ký thay cho người chưa thành niên.

## Phạm vi và file

- `core/include/polyec/managers/estate_manager.hpp`: thêm plan prepare/apply thuần; snapshot hồ sơ, quỹ công, số dư tiền người hưởng và kiểm tra đầy đủ trước khi ghi.
- `core/src/realm_lifecycle.cpp`: gọi phân phối sau khi `prepare_cash_claims` đã được áp dụng.
- `core/tests/test_managers.cpp`: equal split, remainder ordering, ưu tiên/fallback, public fund, stale recipient/case, overflow và atomicity.
- `core/tests/test_realm.cpp`: death thật có khoản vay và người hưởng, continuation sau save/load hoặc xác nhận save roundtrip giữ các số dư mới.
- Không đổi save schema: `EstateCase.cash`, `estate_` và `legacy_estate_unassigned_` đã được serialize cùng nhau; kiểm tra invariant tiếp tục ràng buộc tổng của chúng. Không đổi action, observation, reward, model hoặc binding.

## Kiểm tra và ranh giới

- Chạy ngay: Release build các target `polyec_manager_tests` và `polyec_realm_tests`; chạy test trực tiếp tương ứng, gồm death transition, khoản vay và save/load.
- Sau khi nhóm estate/ownership ổn định: full Release CTest; Python migration/replay, GDExt build và Godot smoke. Realm smoke 64 ngày chỉ khi bundle `realm-v11` đúng contract có sẵn.
- Kết thúc lát này khi cash distribution, atomic reject, bảo toàn tiền, death continuation và save/load đều pass trong C++; chưa coi hàng, quyền đất/xưởng, guardian administration hoặc toàn M5 là xong.

Tự review Codex; chưa có reviewer độc lập.
