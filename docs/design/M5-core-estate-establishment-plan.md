# Core — chuyển quyền sở hữu xưởng trong hồ sơ di sản cho một người lớn duy nhất

Ngày 2026-10-22. Lát tiếp sau title đất. `IMPLEMENTATION_PLAN.md` §2 yêu cầu tài sản sau chết được xử lý sau nghĩa vụ và không tước tài sản người phụ thuộc. Core hiện ghi ID xưởng cá nhân trong hồ sơ di sản; contract này chuyển quyền điều hành cá nhân chỉ trong trường hợp đơn nghĩa.

## Hành vi và ranh giới

- Sau khi khoản vay trong hồ sơ đã trả hết, nếu chỉ có đúng một người hưởng và người đó còn sống, đủ 18 game years, các xưởng cá nhân của người chết chuyển sang người đó.
- Nếu có nhiều người hưởng, không có người hưởng, còn khoản vay chưa trả, hoặc người hưởng duy nhất còn nhỏ, các xưởng tiếp tục đứng tên người chết và ID tiếp tục ở hồ sơ. Không tự chọn đồng sở hữu, người đại diện, người giám hộ hoặc thanh lý để trả nợ.
- Khi chuyển xưởng, chỉ đổi `owner_id`. Không thay `owner_kind`, vùng, ngành, capacity, tiền, hàng, ca làm/quyền cho công nhân, số công đã xử lý, khoản trả lương hoặc tiến độ ngày sản xuất. Worker đã được cấp quyền vẫn làm việc theo hợp đồng hiện có.
- Sau khi chuyển, ID xưởng được gỡ khỏi hồ sơ di sản. Realm cập nhật chỉ mục dẫn xuất chủ sở hữu cá nhân để định giá tài sản và quan sát dùng đúng chủ mới. Không đổi save schema, command/action, observation, reward, model hoặc binding.
- Người nhận phải trưởng thành ở lát này vì lệnh quản lý xưởng hiện chưa có quyền giám hộ tương ứng. Đây chỉ là giới hạn của chuyển xưởng trong lát này; không tước quyền sở hữu của trẻ nói chung.

## Ownership và file

- `core/include/polyec/managers/estate_manager.hpp`: stage đổi owner cho các establishment được hồ sơ ghi; kiểm toàn bộ case/loan/estate/holding/reference trước commit; không chạm số dư hoặc worksite rows khác.
- `core/src/realm_lifecycle.cpp`: truyền danh sách xưởng vào cùng giao dịch di sản; sau các ca tử vong trong ngày, rebuild chỉ mục chủ xưởng cá nhân đúng một lần.
- `core/tests/test_managers.cpp`: một người lớn hưởng; bảo toàn toàn bộ trường xưởng; giữ khi minor/multiple/no heir/unpaid claim; stale ownership reject atomic.
- `core/tests/test_realm.cpp`: death → claim → estate transfer → owner command → save/load/invariant; xưởng và quyền lao động còn nguyên.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`, report và manifest sau khi kiểm core pass.

## Kiểm tra và giới hạn

- Ngay: Windows Release manager/Realm targets và direct tests. Realm test kiểm chuỗi C++ thật và save/load.
- Không đổi binding, schema hoặc quyết định AI; Python/GDExt/Godot được gom vào mốc estate/ownership ổn định. Chưa đóng nhóm tới khi có contract nhiều người hưởng, quản lý tài sản của trẻ, xưởng không người hưởng và khoản vay không đủ tiền.
- Tự review bằng invariant, state copy và command thật; không commit/push.
