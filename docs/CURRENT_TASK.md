# CURRENT_TASK — ghép các mảng M5 thành một lượt chơi có thể tiếp tục

Cập nhật: 2026-10-25. Mục tiêu đã giao: hoàn thiện base M5, tiếp đó core refactor R1–R4, cuối cùng mới xây dữ liệu và huấn luyện/đánh giá RL. Làm liên tục, không chia lịch theo giờ; sau một nhóm xong tự chọn việc kế theo từ khoảng trống M5. Báo cáo tiếng Việt giải thích đầy đủ cho người đọc. Mọi trial setting phải có phiên bản, đơn vị, miền hợp lệ, lý do và phép kiểm; không gọi là cân bằng đã được chứng minh.

## Baseline source hiện tại

- Save writer `POLYEC_REALM_76`; schema/bundle tag `realm-v45`. Household model contract: `realm-household-fund-goods30-config-v54-atlas-north-pass-v1-salted-meal-v1`; timeline model contract dùng tiền tố `realm-worksite-payroll-goods30-config-v54` và cùng hai suffix. Schema `realm-v45` và chuỗi model contract là hai nhãn riêng.
- Tensor và reward:382 state,400 candidate actions,5.182 observation values,reward14. Không có learned policy được train/evaluate với contract hiện tại; không sửa tên hoặc manifest model cũ để giả tương thích.
- Atlas có27 settlement, polygon/picking, route preview và Sovereign route governance; Travel/Migrate, shipment, trade và raid dùng chung planner. Đèo Bắc link15 đóng hai chiều ngày273–364 trong chu kỳ365 ngày; closure chỉ vào mùa đông.
- Chính quyền hội đồng vẫn theo cơ chế đã có: cư dân đủ điều kiện tự nộp hồ sơ; Sovereign có thể bổ nhiệm hoặc từ chối một đại diện mỗi settlement; nhiệm kỳ thử1.460 ngày; đại diện chỉ dùng quỹ đô thị cùng site cho nhà ở. Không có phổ thông đầu phiếu.
- Realm CTest rộng từng dừng tại check434 vì oracle byte/hash save v14 lịch sử. Không sửa oracle. Khi tới mốc ổn định phải chạy lại và giữ nguyên kết quả thực tế.

## Những mảng đã nối với nhau và giới hạn bằng chứng

| Mảng | Chuỗi đã chạy qua cùng C++ Realm | Còn thiếu để gọi base M5 hoàn chỉnh |
|---|---|---|
| Dân cư, hộ và xã hội | ID/vòng đời, cư trú, hộ/quỹ, consent lao động, học nghề, đồ ăn và consumer xã hội. Saltworks→Fuel+Work→muối→market→Civilian mua→bữa ăn đã chạy trong cùng Realm. | Một lượt chơi nhiều năm nối thừa kế, vụ án, nhà/thuê, chính quyền, thiếu hàng và chiến tranh; kiểm tra click-through người chơi rộng hơn formatter/smoke. |
| Hàng và cơ sở | Timber khai thác→chuyến hàng→Paper Mill; specialty supply→Atelier; Ranch→Butchery→DryFood; các consumer có smoke producer/market/consumer cụ thể. Kiểm kê đủ30 hàng ở [audit nguồn và nơi dùng](reports/M5.2-current-good-source-consumer-integration-audit-2026-10-25.md). | Chưa có lượt dài đo nhiều nguồn, giá, tồn, cạnh tranh capacity và thiếu hàng cùng lúc; một số hàng legacy chưa có nguồn Atlas tái sinh hoặc đầy đủ quyền sở hữu. |
| Bản đồ và tuyến | Travel/Migrate, hàng hóa, foreign trade và raid qua planner chung; route preview/action UI; closure theo mùa đèo Bắc. | Không có terrain raster/passability địa lý dùng chung; thiếu playthrough dài gồm tuyến đóng trong lúc hàng/đơn vị chờ; không suy km từ map_unit. |
| Quân sự và ngoại giao | Consent→mobilization→battle→occupation/garrison/supply→người chơi phản công/rút lui; nhiều vùng và logistics có smoke riêng. | Chưa có tù binh, đầu hàng, đình chiến; chưa có đối tác tự ra quyết định chiến lược; chưa kiểm campaign nhiều vùng dài. Không thay quyết định đối tác bằng hardcode. |
| Chính quyền và nhà ở | Quỹ đô thị, housing project, một đại diện hội đồng do Sovereign chỉ định/từ chối. | Chưa ghép housing + kinh tế + chính quyền trong một save dài. Mọi mở rộng cơ chế chính trị phải phù hợp setting vương quốc phong kiến và contract quyền đang có. |
| Tích hợp/phát hành/AI | C++ là luật chung cho Python/GDExtension/Godot; contract gates từ chối model cũ. | Chưa có lượt Atlas tổng hợp nhiều mảng; Profile B/stress, hiệu năng toàn chu trình và Windows offline package; chưa learned policy qua R1–R4.

Kết luận ghép: nhiều chuỗi chức năng cụ thể đã nối trong cùng Realm, nhưng chưa chứng minh một campaign/save kéo dài nơi hàng hóa, cư trú, chính quyền, tuyến và quân sự cùng tác động rồi tiếp tục đúng sau save/load. Các smoke riêng không được cộng lại thành bằng chứng cho một playthrough dài.

## Quyết định thiết kế đã rà lại: không đưa phổ thông đầu phiếu vào game

Trong lượt làm ngày25-10, một implementation thử `municipal-election-3seat-v1` đã được code và chạy kiểm tra. Khi rà theo setting vương quốc phong kiến, thiết chế phổ thông đầu phiếu không có căn cứ trong hợp đồng đã duyệt. Vì vậy code gameplay đó đã được gỡ hoàn toàn; schema/save/model contract và cơ chế hội đồng đang dùng vẫn là phiên bản trước, một đại diện do Sovereign bổ nhiệm hoặc từ chối. Không dùng kết quả smoke của implementation thử để nói tính năng bầu cử có trong game. Hồ sơ plan và report được giữ để truy nguyên quyết định, với trạng thái “không được nhận vào sản phẩm”: [plan thử đã supersede](design/M5.4-municipal-election-3seat-v1-plan.md), [report thử nghiệm và lý do loại bỏ](reports/M5.4-municipal-election-3seat-evidence-2026-10-25.md).

## Việc đang chọn tiếp: một kịch bản tích hợp trong cùng Realm/save

Trước core refactor cần xác định rõ phần nào chỉ có test nhóm và phần nào đã cùng tồn tại trong một lượt game. Lát kế là lập plan kiểm chứng headless cho một Atlas seed cố định, qua cùng Realm: đường đi/hàng hóa, production/market/consumer, cư trú/housing/chính quyền, ít nhất một tình huống quân sự và save/load ở giữa. Dùng số liệu thật từ cùng run, kiểm invariant và fingerprint trước/sau reject; không tạo success giả hoặc dùng baseline để quyết định chiến lược thay learned policy. Không cần bundle cả app sau mỗi sửa.

Trước khi code, đọc `docs/design/M5-integration-and-tuning-gates.md` cùng audit hàng hóa và các API/test hiện có; viết plan ngắn ghi rõ thao tác thật, những fixture được phép chuẩn bị, dữ liệu run lưu lại, failure handling và giới hạn kết luận. Nếu một cơ chế còn thiếu trong lúc chạy, dừng và mở task code có ranh giới cho chính cơ chế đó thay vì nới assertion. Sau khi chuỗi tích hợp ổn mới chọn khoảng trống sản phẩm tiếp theo; M5.5 Profile B, stress/performance và package vẫn phải hoàn thành trước khi gọi M5 xong. Không chạy RL ở bước này.

## Thứ tự còn lại

1. Tích hợp và đo một chuỗi Realm/save có nhiều hệ thống, sau đó xử lý thiếu hụt M5 trực tiếp mà nó bộc lộ.
2. Hoàn thành các cơ chế sản phẩm M5 còn thiếu theo setting phong kiến (ngoại giao/quân sự/city services theo roadmap; không tự thêm quyền/consent trái contract).
3. Mốc tích hợp đầy đủ: Python/native/Godot theo nhóm ổn định; nhiều năm/profile, stress/performance, package Windows offline; kiểm model gate và điều kiện learned policy đã chốt.
4. Chỉ khi base M5 hoàn tất mới làm core refactor R1–R4 theo kế hoạch; sau đó tạo dataset, huấn luyện, chọn/validation/holdout RL. Mỗi run RL phải lưu source metrics và chart đúng run/version, báo rõ cách sinh dữ liệu và không trộn train/validation/holdout.

## Nguồn và ranh giới kiểm

Roadmap/decision: `IMPLEMENTATION_PLAN.md`; bài giải thích kiến trúc: `DEVELOPER_GUIDE.md`; hệ vấn đề/phép kiểm: `TECHNICAL_PROBLEMS.md`; cách build/test: `DEVELOPMENT_WORKFLOW.md`; nhật ký: `DEVELOPMENT_HISTORY.md`; bằng chứng theo revision/run ở `docs/reports/` và `artifacts/`. Kiểm nhỏ đúng target; gom Python/native/Godot vào khi nhóm ổn định; không chạy lại bộ kiểm trên source không đổi nếu không có lý do. Không đưa model/checkpoint/artifact chưa được giao vào commit.

Ảnh chụp CURRENT_TASK trước khi hợp nhất trạng thái ngày25-10 được lưu tại [archive snapshot](archive/CURRENT_TASK-2026-10-25-before-consolidation.md); đây chỉ là lịch sử, không phải chỉ dẫn triển khai.
