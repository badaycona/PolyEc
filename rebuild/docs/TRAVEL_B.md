# Mốc B — trial-b1 / save3 / command3

Luật thử do agent chọn, không phải cân bằng đã duyệt. Kế thừa10 phút/bước, needs và command pipeline. Fixture1 giữ5 người/6 site/556 đồng để regression (constructor mặc định core); fixture2 của app có8 người,2 lãnh thổ,926 đồng. Save3 chỉ nhận version3, không giả tương thích save1/2.

## Nguồn và ownership

Vùng1 Thanh Điền: site10..15, ngân quỹ100, các tài sản a2 giữ nguyên. Vùng2 Tùng Lâm: cung20/người quản quỹ7, bãi gỗ21/chủ7 (Wood20, giá3), xưởng22/chủ7 (Wood6, Tool1 bán giá6, tool lắp1), chợ23, giếng24 (Water100), trọ25/chủ8 (Food3 bán giá9,2 quyền thuê, thường trú8). Nhà15 có2 quyền thuê như trước. Mỗi Location thuộc đúng Territory; các location dọc tuyến thuộc Route.

Người6 Luu/merchant bắt đầu tại chợ13 với50 đồng,Food2/Water2 và mục tiêu giao thương; người7 Tung ở xưởng22 có180 đồng/Food1; người8 Lan ở trọ25 có80 đồng/Food1. Ngân quỹ200 có60 đồng, Office holder7, độc lập ví7. Tổng926; nguồn hàng đầu kỳ Food13/Seed20/Wood38/Tool6 (gồm3 lắp)/Water222. Không tự refill/rừng sinh gỗ; bãi gỗ là stock hữu hạn ban đầu, chưa có nghề lâm nghiệp. Nguồn thực phẩm vùng1 có sản xuất, vùng2 thiếu thực phẩm và có gỗ rẻ; nghề chế tác dùng cùng recipe ở cả hai vùng.

Recipe dùng chung input/output/work/skill, không hardcode luật theo IDsite. Sale có good/price tại Location; thuế1/đơn vị về treasury của vùng nơi bán (proposal hoặc standing sale). Grant có organization chỉ định100/200, phải đúng Office holder; appointment/dismiss hiện vẫn cho ngân quỹ100 theo luật a2. Công báo đọc được tại cung mỗi vùng, không truyền mọi chuyện riêng.

## Tuyến và hành trình

Route1 nối chợ13 ↔23, dài12 bước (120 phút), camp16 tại tọa độ6 và road17 cho vị trí giữa đường. Sức tải mỗi người12 đơn vị trọng lượng: Food/Seed/Water1,Wood2,Tool3. Tính cả hàng đang reserve cho hoạt động cá nhân; escrow hàng ở một site không là hành lý. Phí2 đồng khi khởi hành một chuyến, trả thật cho100; giữ qua pause/đổi hướng/save. Controller1 có thể mở/đóng tuyến bằng command; không suy quyền từ debug.

Travel chỉ khởi hành tại endpoint, còn sức khỏe/mệt<95, đủ tiền và tải. Đưa Person lên road17, Journey lưu route/origin/destination/coordinate/steps/paused; Activity chỉ chạy phần đường đã chọn. Mỗi bước đi tăng mệt1 ngoài base và đi1 tọa độ. Không teleport, không auto-buy/eat/rest. Direct move chỉ nội vùng; không đi thẳng sangsite khác vùng hoặc từroad để né travel.

Pause_journey ở bất kỳ tọa độ; tại6 ở camp16, còn lại road17. Có thể ăn/uống nước túi/nghỉ dọc đường3 bước (mệt giảm4/bước), không tự lấy nước/thuê trọ. Resume tiếp tục phần còn lại, không phí mới. Turnback đổi đích về origin, tốn đủ bước ngược về; được rút về khi tuyến đóng, không hoàn phí. Cancel một ActivityTravel chỉ pause, không xóa Journey/hàng. Đổi goal không buộc tiếp tục hay hủy hợp đồng.

Tuyến đóng hoặc cơ thể không đi tiếp: trước bước đường tiếp theo, dừng ở vị trí hiện tại và ghi nguyên nhân cho người bị ảnh hưởng; không chạy bù/carry on tự động sau khi mở. Resume hướng đi mới cần tuyến mở; rút về origin được phép dù đóng. Pause/meal/rest/Journey/goal/fee/coordinate lưu qua save. Hết health không tự cứu actor.

Không cho giao dịch đề nghị tại road17 (không có điểm lưu hàng cố định). Camp16 là địa điểm cố định, có thể gặp người đúng tọa độ. Co-location ở road yêu cầu cùng tuyến/tọa độ; không vì cùng IDroad mà trao đồ hoặc biết chuyện cách xa nhau. Thông tin hành trình và hàng chỉ chủ/debug thấy; bản đồ tuyến, chi phí và địa điểm là công khai, trạng thái đóng/mở chỉ khi trực tiếp thấy hoặc controller/debug (tin cũ vẫn ở knowledge).

## Trọ và lựa chọn kinh tế

Dùng Proposal/Lease hiện có tại nhà/trọ có capacity: owner chào, tenant tự accept và trả rent trọn kỳ từ ví mình; không tự chọn trọ/ăn. Hết hạn không xóa người. Người buôn mang food thật sang vùng2, bán tại chợ hoặc site, mua gỗ thật quay về rồi bán/giao cam kết; mọi phí/lương/thuế/rent có đối ứng. Giá giao dịch cá nhân do actors đặt/consent; bảng giá fixture chỉ là cơ hội thử.

## Ownership và kiểm

geography: Recipe/Location/Territory; transport: Route/Journey + travel/pause/resume/turnback/route handlers. Person ghép Journey. World giữ vùng/organizations có chủ và dispatch, không duplicate luật2 vùng. Contracts/life/production dùng helpers location/treasury/co-location và residence tương ứng. Save3 staging + bounds/crosslinks, view riêng, UI cùng registry.

Kiểm trước consumer: regression fixture1; tổng nguồn fixture2; buy→travel có fee/tải→pause/meal/rest→arrive→sale/rent→Wood return→profit/cash đối soát; delivery chưa hoàn thành/save giữa đường/fulfilled; closure/return/overload/no-money/stale/wrong-region/co-location reject nguyên tử; time chunking/replay; malformed Journey/territory/escrow. Sau core ổn định: Godot2 vùng, chọn person/site/route, command controls, scenario UI thật và save/load, pause/focus không chạy bù. Chưa RL, không kết luận policy tự làm được từ script.

### Hàng sale ký gửi khi owner đi xa
Terminal sale không tự trả goods vào túi owner ở nơi khác. Goods ở lại proposal escrow tại site; claim chỉ khi owner quay về. Nếu đang trên tuyến và hoàn hàng làm vượt tải, cũng giữ escrow chờ claim. Tiền escrow dùng ledger account, không được tính là hàng vật lý.
