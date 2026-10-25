# Báo cáo mốc B — trial-b1, 2026-10-25

## Nội dung đã thay

Theo thứ tự người dùng giao: đã đưa rewrite vào checkout C:\prc\PolyEc (code duy nhất ở rebuild/), thay tree hiện hành GitHub main bằng commit17c052912208627bf481de27a2c6f64f349b3ff2, rồi triển khai hai lãnh thổ. Push bình thường, giữ toàn bộ lịch sử; không xóa/move recursive các file legacy local hoặc đụng worktree ngoài. GitHub tree mới không chứa source/model/assets/build legacy. Snapshot Documents không ghi source nữa.

Mốc B có8 NPC thật trong2 Territory,926 đồng đầu kỳ, nguồn goods kê khai trong TRAVEL_B.md. Thanh Điền sản xuất thực phẩm, Tùng Lâm có stock gỗ rẻ hữu hạn và xưởng dùng cùng Recipe. Person/Account/Inventory/Body/Experience/Goal/Knowledge tiếp tục dùng cùng core. Location giữ sale_good/price/recipe/capacity/region; không suy nghề/sản phẩm từ IDsite. Tài khoản200 có Office holder7 và tiền riêng60, khác ví7; thuế về đúng quỹ vùng nơi bán, grant kiểm tư cách với nguồn100/200.

Route/Journey mới giữ endpoint, origin/destination, tọa độ, bước thực hiện, paid toll và paused. Tuyến12 bước (120 phút), tải12, phí2 trả100. Travel không teleport; move chỉ nội vùng. Pause/cancel trên tuyến giữ hàng/goal/fee; actor phải tự ăn/uống/nghỉ và resume. Camp ở mốc6, road actors chỉ gặp/trao tin/hàng khi đúng tọa độ. Đóng tuyến hoặc kiệt sức dừng trước bước đường tiếp theo; quay về vẫn tốn quãng ngược, không hoàn phí. Sau mở đường không tự đi tiếp.

Đến vùng2 có thể chào bán hàng thật, mua gỗ, thuê trọ bằng consent và trả rent, nghỉ rồi đi về; expiry không xóa occupant. Hàng escrow của sale khi người bán đi xa vẫn tại site cũ, không trả thẳng vào túi: owner phải quay về claim. Hàng pending/terminal escrow vẫn nằm trong đối soát toàn world. Recipe đọc từ save phải khớp luật để không tạo food miễn phí do đổi input.

Save3 lưu cả hai vùng, organizations, routes, camp/road, Recipe, Journey và mọi cam kết a2. Load staging/bounds/liên kết, không giả tương thích save1/2; app dùng live-b1/checkpoint-b1 riêng. View giấu ví/goal/hàng/journey của người khác và người xa; bản đồ địa điểm/tuyến công khai, trạng thái tuyến chỉ hiện khi trực tiếp thấy hoặc controller/debug. NPC roster cũng lọc theo view, không lộ người xa. Ngân quỹ là số công khai như luật thử a2. Godot hiển thị2 vùng, đường/camp, tọa độ tiến độ, tải/hàng/phí, nguồn thuế/rent; cùng36 verb trong registry, không engine chọn thay NPC.

## Kết quả có đối soát

Chuỗi cùng world: người6 từ50 đồng, mua3 food (15 gồm thuế3), trả toll2, tự dừng/ăn/nghỉ giữa đường, bán3 food giá9 (27 cho seller/3 thuế về200), thuê trọ3, mua3 Wood (12 gồm thuế3), trả toll2 chuyến về, bán3 Wood giá7 (21 cho seller/3 thuế về100). Kết quả tiền6=64, tiền3=136, tiền7=159, tiền8=83, quỹ100=110/quỹ200=66. Food đã dùng và wood tới người mua được kiểm bằng lượng độc lập, không chỉ gọi lại helper luật.

Chuỗi cam kết riêng: chủ xưởng3 escrow reward30; merchant6 nhận lời giao Wood3, đi sang mua hàng, quay lại gặp3 mới fulfill; source cash3=130/6=64 sau tiền mua12 và toll4. Nhận lời/save giữa đường không tạo hàng hoặc nhận thưởng sớm.

## Kiểm đã chạy

- `scripts/verify-root.cmd`: CMake root/Ninja Release, đúng targets core_tests/action_tests/journey_tests/polyec_core/polyec_launcher;3/3 CTest pass.191 checks nền,277 action/domain,293 journey =761 checks. Log gói evidence/cpp-b1.log và ninja-b1.log.
- `scripts/verify-vs.cmd`: generator Visual Studio18/Release,3 CTest pass. Đây là kiểm consumer build phù hợp CI, không chỉ Ninja. Source/target launcher đổi tên thành polyec_launcher, output vẫn PolyEc.exe để tránh case collision với library polyec trên VS.
- Journey tests gồm khứ hồi/tax/fee/rent/profit, nguồn thiếu tiền và tải, tải đúng12, gift gây quá tải reject nguyên tử, closure/retreat/reopen, exhaustion, không auto-consume, cấm local move xuyên vùng, goal/đồ còn qua save, share-road khác tọa độ không co-location, delivery thật, ngân quỹ200 có quyền, same Recipe tại vùng2, forged coordinate/toll/region/initialgoods/recipe và save cũ reject staging. Save/replay giữa hành trình/camp/rest/accepted obligation và chia tick có đối chiếu.
- Bridge CLI36 lượt: Unicode path/goal, actor/viewer permission, duplicate/unknown params reject, native save/load, Journey mốc6 paused/fee2 giữ đúng. evidence/bridge-b1.json. Thời gian process nhỏ chỉ tham khảo, không benchmark quốc gia.
- Godot: vòng cơ bản24 bước cùng kiểm save/load/player/focus/sleep; action37 records pass và catalog36/UI36; hai vùng46 bước và64 records pass. Có dùng các control/tín hiệu nút gửi lệnh thật cho travel/control/resume-reject/turnback/claim; so serialized file bytes khi reject. Chuỗi có đóng tuyến giữa return trip, lưu trước mở lại và tiền cuối64/quỹ110/66. Đã chạy renderer OpenGL native, chụp viewport tại camp với hàng, needs, fee và hai vùng thật. Không dùng mockup hoặc coi build là chứng nhận chơi. Đây là scripted UI test, không tuyên bố toàn bộ36 hành động đã được bấm chuột thủ công hay learned AI tự chơi.

## Lỗi tìm được và sửa trong workflow

CI của commit overwrite ban đầu17c0529 lỗi LNK1181 do library polyec và executable PolyEc trùng tên project khi không phân biệt hoa/thường trên VS. Đọc log job121168155816 qua GitHub connector, đổi target launcher và kiểm cả VS/Ninja; không tắt CI. Trạng thái run cuối xem GitHub và evidence/publish-b1.json. Các warning Node20 của checkout@v4 không phải nguyên nhân lỗi link.

Regression phát hiện roster mới lộ tên NPC xa, đã lọc lại projection thay vì nới oracle privacy. Kiểm vùng2 phát hiện study còn target10, đã đổi sang cung tại chỗ. Kiểm nguồn escrow phát hiện khả năng trả hàng vào túi người đi xa, đã giữ hàng tại site và thêm claim. Native capture phát hiện so sánh int/String trong đoạn script chụp ảnh, đã sửa và chạy lại renderer. Hai fixture test mới ban đầu chọn actor còn đủ phí/đi tới nơi đang đứng, đã chỉnh setup để kiểm đúng điều kiện, không sửa oracle tiền/hàng để lấy pass. Logs cuối và tool history ghi kết quả; không dùng số pass của bản cũ chứng nhận B.

## Giới hạn

Đây là fixture hai vùng có luật và vòng giao thương hoạt động, chưa là kinh tế đã cân bằng dài hạn. Gỗ hữu hạn ban đầu, chưa có lâm nghiệp/refill; chưa network nhiều tuyến, khí hậu/xe/đội hộ tống, chiến tranh/cưỡng chế hoặc27 vùng. Đường đóng là chặn tuyến trừu tượng, không physics quân lính/gate. Chưa có learned policy/binding Python/training; script điều khiển phải tự chọn mọi bước, không chứng minh NPC biết chiến lược. Bridge process+serialize mỗi lệnh phù hợp8 NPC, chưa có benchmark quy mô lớn. Trọ là quyền nghỉ tại site trừu tượng; chưa phòng/eviction. Save3 cố ý không migration save1/2. Runtime portable dùng editor Godot4.5 chính thức chạy project, chưa là export tối ưu dung lượng.
