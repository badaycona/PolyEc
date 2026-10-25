# Luật thử một lãnh thổ — trial-a1 / save 1 / command 1

Đây là tham số thử do agent chọn cho vertical slice, chưa là cân bằng đã duyệt. Bước 10 phút, 144 bước/ngày, bắt đầu 08:00. Chọn bước này để thấy rõ nấu ăn 10–30 phút, di chuyển 10 phút, lao động khoảng 1 giờ. Pause/focus/sleep không chạy bù. Seed mặc định 251025; seed chỉ đổi xu hướng, không nhân tài sản.

## Người, quyền và tài nguyên

5 Person ID 1 vua (60 đồng), 2 chủ ruộng (220), 3 chủ xưởng (160), 4 An (12), 5 Binh (4). Tổ chức lãnh thổ ID100 có 100 đồng; vua giữ Office chi công, không sở hữu công quỹ. Tổng 556 đồng; không mint. ID bền, không tái sử dụng. Territory 1 có cung10, ruộng11, xưởng12, chợ13, giếng14, nhà15. Tất cả địa điểm công khai. Người ở nhà15 có quyền cư trú vô thời hạn của chủ2 (fixture cho phép cả5); không rent/eviction trong slice.

Mỗi Person có tài khoản và kho riêng. Địa điểm11,12,14 có kho và chủ riêng; owner không tự di chuyển hàng từ kho site vào túi. Ruộng2 sở hữu: 20 đơn vị giống; xưởng3: 12 gỗ. Giếng do100 quản lý, 120 đơn vị nước, public access một đơn vị/lần; hữu hạn, không tự refill. Ruộng/xưởng dùng công cụ cố định một chiếc/site, chủ sở hữu, chỉ là điều kiện sản xuất; chưa mô phỏng hao mòn. Đất/giống/gỗ/nước và 6 thực phẩm/2 công cụ mở đầu được kê khai; không có nguồn spawn sau khởi tạo.

## Giao dịch, sản xuất và cam kết

Tiền là đồng nguyên; hàng là đơn vị nguyên. Giá chào fixture có consent chủ: thực phẩm ruộng 4 đồng/đơn vị, công cụ xưởng 8; thuế bán hàng 1 đồng/mỗi đơn vị (người mua trả thêm), về100. Standing offer công khai, buyer phải tới site, số lượng 1–20, không bán hàng cá nhân tự động. Một market service khớp các chào bán tại site; chợ13 là địa điểm gặp/truyền tin.

Chủ2/3 đăng một job qua offer tại site (fixture đã đăng sẵn), wage6, một slot, vốn chủ thật. Worker tự chấp nhận qua work khi ở site và biết chào việc. Một mẻ ruộng: 1 giống → 3 thực phẩm, 6 đơn vị công; xưởng: 2 gỗ → 1 công cụ, 8 công. Bắt đầu reserve wage từ chủ vào Activity escrow, input từ kho vào Activity. Không có worker thì không sản xuất. Hoàn tất mới có output vào kho site và wage về worker. Cancel trả nguyên liệu/chủ và wage/chủ, giữ kinh nghiệm theo công thực đã làm; không thưởng tiền hoặc tạo output dở dang. Slot giải phóng; mỗi offer chỉ nhận một lần, owner phải đăng lại. Công cụ cố định và quyền chủ site kiểm trước nhận.

Làm mỗi bước tăng mệt2, cộng kinh nghiệm nhóm ruộng/xưởng theo thời gian thực tham gia (1 practice/bước). Hiệu quả công/bước = (1 + 0.4*xp/(xp+30)) * (0.97 + vui/100*0.06); hạnh phúc tối đa ±3% để đối chứng nhỏ, không nhân tiền/output. Mệt>=95 hoặc sức khỏe<=0 không nhận việc; đang làm tới95 thì dừng trước bước công tiếp theo, trả escrow/input và ghi nguyên nhân. Skill có một tác dụng tốc độ, không tạo consent/hàng/lương. Không skill chung mọi nghề.

## Cơ thể, thời gian, thông tin

Đói/khát/mệt/vui/sức khỏe thang0–100. Mỗi bước đói+0.7, khát+1, mệt+0.15. Đói hoặc khát>=90 mất sức khỏe1/bước. Sức khỏe0: không nhận lệnh hoạt động; không tự ăn/uống/nghỉ. Ăn tự chọn tier0 dùng1 thực phẩm/1 bước, tier1 dùng2/3 bước; food reserve ngay, hoàn tất đói-35 cả hai, vui+2 hoặc+10. Cancel bữa mất thực phẩm đã chuẩn bị, không giảm đói. Chi phí mua từ tài sản người đó, không engine mua hộ. Uống ở giếng tiêu1 nước, một bước, khát-45 khi hoàn tất. Nghỉ 3 bước tại nhà có quyền, mỗi bước mệt-8; không teleport. Di chuyển giữa site mất1 bước, không physics; chưa có hành trình liên vùng. Một người một Activity, mục tiêu chuỗi text do command goal thay đổi, không ép kiên định.

Thông tin Fact có event id, nội dung, source, thời điểm sự việc/nhận. Khi đến nơi: nhìn được chào bán/việc và người hiện diện, không thấy ví người khác. Sự kiện chỉ các bên và người có mặt biết; thuế/chi công là thông báo công có nguồn, không công khai mọi giao dịch riêng. Fact không tự cập nhật giá cũ. Talk là lời mời một người cùng site; recipient phải accept; cả hai bận trong 2 bước giảm dần còn1 theo practice giao tiếp. Hoàn tất truyền tối đa3 Fact mới mỗi chiều có nguồn gốc, relation mỗi hướng+1, practice theo bước thật; không tự accept từ relation. Request có pending/accepted/fulfilled/cancelled thật; cancel hoạt động nói chuyện kết thúc cả hai và cam kết. Thông báo công có bản lưu tại cung; người tham gia và người đang ở cung nhận ngay, người khác phải đến đọc hoặc nghe kể. Hai công báo khởi tạo được giao sẵn cho cả5 để biết luật và cơ hội ban đầu. Học ở cung: 2 bước đọc tối đa3 thông báo công chưa biết (không mở dữ liệu riêng).

Vua lệnh grant đại diện100, target người thật, amount1–50; trừ công quỹ/cộng cá nhân, ghi công báo. Không suy quyền từ debug actor. Không triển khai hành vi phạm pháp/chiếm đoạt trong slice này; reject quyền ghi rõ khác với thiếu nguồn lực. Không quân sự/chiến tranh/thuế tùy ý.

## Ownership code và kiểm

model.hpp: Person/components, Territory/Location, Account/Inventory, Activity, Organization/Office, Fact/Request. ledger: Transaction kiểm tổng nguồn/đích; production: job/reservation/output; life: move/meal/drink/rest; civic: quyền/chi/trao đổi tin; world: registry/atomic staging/tick, không chứa luật mọi action; persistence: save/load staging bounded; view: player/debug projection; bridge: CLI native cùng core. Không model RL/Python luật thứ hai.

Command: actor, verb, target, amount, option, expected revision; reject stale/actor/target/amount/consent/resources nguyên tử. UI/test dùng chung World::submit. Tick chỉ tiến activity đã chọn và cơ thể, không chọn việc tiếp. Revision mỗi command thành công/bước. Save có schema/rules version/seed/revision; lưu hoạt động, escrow, resources, facts/relations/goals/requests, validate staging và không nhận save cũ. Giới hạn lưu hữu hạn, không load một phần.

Kiểm ngay: reject không đổi serialized state; tranh slot; nguồn thiếu; escrow/cancel; thuế/chi; skill/vui đối chứng; thông tin/accept; death/exhaustion; save giữa hoạt động/replay/chunk time; malformed load. Kiểm consumer sau core: UI bấm chọn site/người, lệnh thật, pause/play/step/focus, save/load/reject; fixture demo có nhãn SCRIPT KIỂM LUẬT, không learned AI. Nhóm A chỉ đóng khi có bằng chứng vòng thật; hạn chế phải ghi report.
