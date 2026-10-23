# Core — trả khoản vay từ tiền trong hồ sơ di sản

Ngày 2026-10-21. Lát này thực hiện quy tắc `IMPLEMENTATION_PLAN.md` §2 rằng nghĩa vụ được trả từ di sản trước khi chia; không chuyển nợ sang người hưởng.

## Phạm vi và giả định bảo toàn luật

- Ngay sau khi lập hồ sơ khi một Person chết, Realm thử quyết toán đầy đủ các khoản vay còn dư được hồ sơ tham chiếu.
- Chỉ tiền mặt trong chính hồ sơ của người chết được dùng. Không lấy quỹ household, tiền của người còn sống, tiền từ hồ sơ khác, hàng hóa hay tài sản chưa được định giá.
- Nếu tiền mặt của hồ sơ không đủ trả toàn bộ các khoản vay còn dư, lần quyết toán này không thay đổi gì: không ưu tiên một creditor, không trả một phần, không chia di sản. Các khoản còn dư vẫn gắn với debtor đã chết và ID loan trong hồ sơ, nên phần nợ không trở thành nghĩa vụ cá nhân của người hưởng.
- Khi đủ tiền, trả toàn bộ từng khoản còn dư theo creditor đã ghi trên Loan: Province firm hoặc credit pool nếu khoản vay dùng pool. Chỉ `Loan::paid`, số dư nguồn/đích và tiền của đúng EstateCase thay đổi; không xóa Loan record hoặc đổi `defaulted`.
- Không thanh lý inventory, holding hay establishment ở bước này. Chúng vẫn là asset reference của case và cần lát định giá/chuyển quyền riêng trước phân phối.

## File và ownership

- `core/include/polyec/managers/estate_manager.hpp`: plan thuần prepare/apply, snapshot toàn bộ loan rows, các số dư creditor liên quan, `estate_` và EstateCase vector; kiểm tra read-set đầy đủ trước mutation.
- `core/src/realm_lifecycle.cpp`: gọi quyết toán ngay sau `apply_death`; Realm vẫn sở hữu các account và phase order.
- `core/tests/test_managers.cpp`: đủ tiền, không đủ tiền, creditor pool/Province, dữ liệu stale, overflow và atomicity.
- `core/tests/test_realm.cpp`: death transition thật, money/invariant, save-load và xác nhận khoản nợ được trả hoặc còn nằm trên loan gốc.
- Không đổi save schema: case cash và Loan paid đã lưu trong schema24. Không đổi action, observation, reward, model hoặc binding.

## Kiểm ngay và kiểm nhóm

Ngay: build Release manager/Realm targets, chạy direct manager + Realm death/save-load tests. Sau khi core ổn định: full Release CTest, Python binding/migration replay và GDExt/Godot smoke. Godot `realm_smoke` chỉ chạy khi có bundle realm-v11 tương thích; không thay bằng bundle cũ đã bị runtime từ chối.

Tự review Codex; chưa có reviewer độc lập.
