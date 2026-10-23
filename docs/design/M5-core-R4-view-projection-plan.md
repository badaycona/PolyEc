# R4 — tách phép chiếu RealmView

## Baseline và dirty scope

- HEAD khi lập kế hoạch: đang giữ toàn bộ dirty state M5 hiện có; không stage/ghi đè file ngoài ranh giới dưới.
- `Realm::view(observer)` hiện tạo summary copy cho UI/runtime và lọc facts bằng `Realm::knows`. Các codec R4 đã tách từng section save; report `R4-realm-primary-header-codec-evidence` ghi view projection còn mở.

## Trước → sau

Trước: `Realm::view` tự vừa chọn facts được observer nhìn thấy, vừa tính tổng người/sống/trẻ em/quyết định, vừa copy province/draft/money vào `RealmView`.

Sau: Realm vẫn áp dụng quyền biết thông qua `knows(observer, fact)` và chuẩn bị danh sách facts nhìn thấy; một hàm projector thuần nhận snapshot đọc-only rồi dựng `RealmView`. Hàm projector không giữ state, không chọn policy, không truy cập save hoặc command.

## File và trách nhiệm

- Mới `core/include/polyec/realm_view_projector.hpp`, `core/src/realm_view_projector.cpp`: `detail::project_realm_view` nhận ngày/vùng, dữ liệu người/địa bàn, facts đã được Realm lọc, draft và tổng tiền; trả bản copy.
- `core/src/realm.cpp`: giữ filter `knows`, gọi projector; không đổi thứ tự hoặc luật visibility.
- `core/tests/test_realm_view_projector.cpp`, `core/CMakeLists.txt`: oracle số học thuần cho counts, money, provinces/facts/draft và empty case.
- Không đổi command/action, observation, save/load, visibility rules, reward/model, UI fields hoặc public binding.

## Ownership, units và rejects

- Realm tiếp tục sở hữu toàn bộ mutable state và quyết định fact nào observer được đọc.
- Projector nhận ngày game nguyên `int`, tổng tiền đơn vị cash `int64`, số người từ record vector; không mutate arguments.
- Không có reject path hoặc giao dịch. Không thêm ID, index, save field hay cache.

## Kiểm chứng

- Unit test projector với fixture độc lập xác nhận sống/trẻ/quyết định, giá trị địa bàn/facts/draft, tổng tiền; input rỗng giữ summary đúng.
- Windows Visual Studio 18 Release build + full CTest, Realm test và diff check.
- Chỉ self-review Codex theo source/evidence hiện hành; đây là refactor data projection, không nghiệm thu gameplay hoặc toàn M5.
