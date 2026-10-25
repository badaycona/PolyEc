# Class và action đang chạy — trial-b1

Code chính: C:\prc\PolyEc\rebuild, thuộc checkout C:\prc\PolyEc và origin https://github.com/badaycona/PolyEc.git. Source trong ZIP là snapshot review, không phải nơi ghi song song. Registry C++ cung cấp cả definition và handler; Godot đọc danh sách này. Hiện có36 verb chạy được cho fixture hai lãnh thổ (fixture một vùng giữ để regression). Đây không phải tuyên bố đã làm mọi hành động của toàn roadmap.

## Các file class và ownership

| File | Class/struct có thật | Trách nhiệm đang dùng |
|---|---|---|
| core/types.hpp | RuleError, Id/Money | Kiểu, giới hạn và lỗi luật |
| core/economy.hpp | Inventory, Account, Transaction, MoneyLeg, GoodsLeg, Good | Sở hữu tiền/hàng, kiểm chuyển đối ứng |
| core/person.hpp | Person, Body, Experience, Tendencies, Fact, Activity | Một danh tính ghép các thành phần; hoạt động/tiến độ/input/escrow/started và thông tin riêng |
| core/geography.hpp | Recipe, WorkSkill, SiteKind, Location, Territory | Kho/site/chủ/cư trú/job/thiết bị và lãnh thổ |
| core/transport.hpp | Route, Journey | Hành lý, thời gian, phí, tọa độ, pause/return/closure và camp |
| core/institutions.hpp | Office, Organization | Tư cách quản lý tài sản công, có thể bổ nhiệm/bãi nhiệm |
| core/contracts.hpp | Request, Proposal, Debt, Lease | Consent, đề nghị, trả giá, escrow, nợ và quyền thuê có hạn |
| core/actions.hpp | Command, ActionDefinition, RegisteredAction, Result | Cùng command và tham số named cho UI/test; catalog gắn handler |
| core/world.hpp | State, Event, World | Sở hữu world, staging commit, clock, save/view facade |
| core/model.hpp | Umbrella include | Giữ include đơn giản cho consumer; không chứa một bản model khác |

Implementation: actions.cpp registry; ledger.cpp transaction; resources.cpp chuyển kho/tài sản/thiết bị; production.cpp việc và sản xuất; life.cpp cơ thể/đi/ăn/uống/nghỉ/lấy nước/chăm sóc; civic.cpp chức vụ/tin/mời/đọc/quan sát/mục tiêu; contracts.cpp thương lượng/nợ/thuê/cam kết; validation.cpp kiểm state; persistence.cpp save3; view.cpp lọc quan sát; bridge.cpp adapter native Unicode; launcher_win.cpp mở app native. Không có class rỗng cho ngành/vùng chưa làm.

## Catalog điều kiện chính

Tất cả lệnh kiểm actor, revision và giới hạn tham số; thực thi trên staging. Sai điều kiện thì reject nguyên tử, không thay tài sản/progress/facts. Hoạt động dài chỉ tiến khi clock được bước, không tự chọn việc tiếp.

| Verb | File | Điều kiện và hậu quả chính |
|---|---|---|
| move | life.cpp | Rảnh, đích khác chỗ và cùng vùng, không trên Journey;10 phút |
| travel | transport.cpp | Tại endpoint, tuyến mở, đủ tiền/tải/sức; bắt đầu hành trình và trả phí thật |
| pause_journey | transport.cpp | Đang đi, dừng đúng tọa độ, giữ hàng/goal/toll |
| resume_journey | transport.cpp | Đang dừng, body/tuyến cho phép; đi phần còn lại, không phí mới |
| turnback | transport.cpp | Dừng trước khi đổi hướng; đi ngược về origin kể cả đường đóng, không teleport |
| route | transport.cpp | Controller đổi mở/đóng; ngắt người đi trước bước tiếp theo |
| claim | contracts.cpp | Đúng owner của hàng sale terminal còn ký gửi, tới đúng site để nhận lại |
| offer | production.cpp | Chủ tại site, không có mẻ/job; wage1..100 đủ vốn; chào một việc |
| revoke_job | production.cpp | Chủ hiện diện, job chưa có người nhận; thu hồi consent chào việc |
| work | production.cpp | Người lao động tại site, job/công cụ/input/lương đủ; reserve input và wage |
| produce | production.cpp | Chủ tự làm tại site, có input/tool/slot và cơ thể; không giả lương |
| buy | production.cpp | Buyer tới site chào bán; hàng/tiền đủ; chuyển hàng, giá và thuế |
| manage | resources.cpp | Chủ tại kho mình; take/store theo good/amount; không mượn kho công |
| transfer | resources.cpp | Tặng từ ví/túi/kho thuộc quyền cho người khác cùng nơi; không tạo nợ/consent giả |
| equip | resources.cpp | Chủ tại site, không mẻ bận; tool thật thay vào, cũ tiêu hủy;12 lượt dùng |
| propose | contracts.cpp | Sale/loan/lease/delivery/appointment đúng chủ/thẩm quyền, đối tác và terms; giữ tài sản phía đưa đề nghị |
| counter | contracts.cpp | Đúng lượt, sale/lease pending, tới site; đổi giá, bên kia phải phản hồi |
| respond | contracts.cpp | Đúng bên chấp nhận/từ chối/thu hồi; revalidate tiền/hàng/quyền/sức chứa; không tự accept |
| fulfill | contracts.cpp | Delivery đã nhận, đúng người mang hàng thật, gặp requester, trước hạn; đổi hàng lấy reward escrow, phải cùng vị trí thật kể cả trên road |
| repay | contracts.cpp | Đúng borrower, amount không vượt dư nợ, có tiền riêng; giảm nợ đúng tiền đã trả |
| end_lease | contracts.cpp | Tenant của lease active tự kết thúc; không tự refund hay teleport |
| meal | life.cpp | Food riêng đủ; reserve1/2,10/30 phút; no và vui thay đổi riêng |
| drink | life.cpp | Chọn giếng tại chỗ hoặc nước túi; dùng1 thật,10 phút |
| collect | life.cpp | Tại giếng còn nước; reserve1,10 phút về túi; cancel trả nguồn |
| rest | life.cpp | Quyền cư trú tại thời điểm bắt đầu;30 phút, giảm mệt qua từng bước |
| care | life.cpp | Nhà có quyền, health chưa đầy, food1/water1 riêng;60 phút, chỉ hồi phục nếu không thiếu nhu cầu |
| talk | civic.cpp | Hai người khác nhau cùng nơi và rảnh; tạo lời mời, chưa bắt đối phương bận |
| accept | civic.cpp | Recipient đúng invitation pending, cùng nơi, cả hai rảnh; trao đổi tin có thời gian |
| invitation | civic.cpp | Recipient decline hoặc inviter withdraw đúng lời mời pending |
| study | civic.cpp | Tại cung,20 phút đọc tối đa3 công báo chưa biết; không lộ dữ liệu riêng |
| inspect | civic.cpp | Rảnh, ghi trải nghiệm tại chỗ; không quan sát toàn thế giới |
| grant | civic.cpp | Holder hiện tại được chi1..50 từ công quỹ tới người thật; không từ ví riêng |
| dismiss | civic.cpp | Vua bãi nhiệm holder, thu hồi appointment pending; tài sản tổ chức không đổi chủ |
| announce | civic.cpp | Vua/holder ở cung đăng lời phát biểu có nguồn; không làm nội dung thành sự thật kinh tế |
| goal | civic.cpp | Mục tiêu không rỗng, actor còn sức khỏe; không buộc hủy cam kết |
| cancel | world.cpp + domain | Activity có thật; hoàn escrow/input job, mất phần chuẩn bị bữa/care, trả nước collect, dừng cả cặp talk |

## Điều kiện theo thời gian và save

Pending hết hạn trả escrow cho nguồn. Delivery trễ hạn ghi default và trả reward, không tạo hàng/tiền phạt. Debt quá hạn ghi overdue, không tự trừ tiền. Lease hết hạn chỉ mất quyền bắt đầu nghỉ mới; occupant không biến mất, phần nghỉ đã bắt đầu hợp lệ được hoàn tất. Tool giảm1 lượt khi hoàn mẻ, không giảm khi hủy. Mọi hồ sơ cùng save3 có kiểm liên kết/giới hạn và đối soát tiền/hàng; save1/2 bị từ chối rõ và vẫn giữ file cũ.

Luật nền: [ACTIONS_A2](ACTIONS_A2.md), luật hai vùng: [TRAVEL_B](TRAVEL_B.md). Kiểm/giới hạn hiện hành: [REPORT_B](REPORT_B.md). Scope chưa mở: chiến tranh/cưỡng chế/27 vùng/RL theo roadmap, không tạo placeholder để giả hoàn thành.

Mốc B tổng quát hóa kho/recipe/sale/thuế/quyền trọ theo Territory, không sao chép luật mỗi vùng. Source goods ký gửi không teleport về người đang đi; terminal escrow giữ tại site cho tới claim. Lệnh grant thêm organization100/200, phải đúng Office holder của nguồn. Co-location trên road yêu cầu đúng tuyến/tọa độ.
