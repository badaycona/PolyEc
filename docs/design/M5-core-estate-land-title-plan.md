# Core — chuyển title đất trong hồ sơ di sản khi chỉ có một người hưởng

Ngày 2026-10-22. Lát kế tiếp trong chu trình di sản sau khi trả khoản vay, chia tiền và hàng. Quy tắc nguồn là `IMPLEMENTATION_PLAN.md` §2: trả nghĩa vụ từ di sản trước; thứ tự người hưởng giữ nguyên; người phụ thuộc có thể sở hữu và giám hộ không sở hữu thay.

## Hành vi và ranh giới

- Khi hồ sơ không còn khoản vay chưa trả và hàng người hưởng hợp pháp hiện tại có đúng một người, title đất của người chết chuyển sang người đó. Người hưởng có thể là trẻ vị thành niên: tuổi không làm mất quyền sở hữu.
- Nếu còn khoản vay chưa trả, có từ hai người hưởng trở lên, hoặc không có người hưởng, title tiếp tục đứng tên người chết và ID holding tiếp tục nằm trong hồ sơ. Không chia diện tích, không chọn một người trong nhóm, không chuyển title vào quỹ công khi chưa có contract cho loại tài sản này.
- Quyền sử dụng đang có hiệu lực theo title sang cùng người hưởng; người nhận được ghi làm grantor hiện tại. Grantee, holding, diện tích và vùng giữ nguyên. Điều này bảo toàn quyền đang có thay vì âm thầm hủy nó; chủ title mới quản lý/thu hồi theo luật land-use hiện hành.
- Sau khi chuyển title, ID holding bị gỡ khỏi danh sách estate case. Không đổi tiền, hàng, khoản vay, cơ sở, action, observation, reward, model, binding hoặc save schema.

## Ownership và file

- `core/include/polyec/managers/estate_manager.hpp`: mở rộng plan phân phối để stage title và grantor của land-use; chỉ lập chuyển khi một người hưởng và mọi khoản vay đã quyết toán đủ; validate read-set trước mọi mutation.
- `core/src/realm_lifecycle.cpp`: truyền loan ledger hiện có vào prepare/apply phân phối.
- `core/tests/test_managers.cpp`: kiểm single heir, minor beneficiary, land-use preservation, multi-heir/no-heir/loan shortfall retention, overflow/stale input và reject atomic.
- `core/tests/test_realm.cpp`: death pipeline chuyển title và quyền sử dụng khi đủ điều kiện; save/load, invariant và title của người chết còn nguyên trong trường hợp giữ lại.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`, report và manifest chỉ cập nhật sau khi kiểm core pass.

## Kiểm tra và giới hạn

- Ngay: build Release manager/Realm targets; chạy direct manager và Realm tests. Realm test là chuỗi C++ từ tử vong → claim → phân phối → save/load.
- Không đổi schema/binding nên không chạy Python/GDExt/Godot ở lát này; gom cùng nhóm estate/ownership ổn định. Chưa đóng nhóm tới khi có contract cho nhiều đồng thừa kế, xưởng, nghĩa vụ không đủ tiền và xử lý tài sản không có người hưởng.
- Codex tự review source và oracle; chưa có reviewer độc lập. Không commit/push.
