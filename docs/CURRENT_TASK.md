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
| Dân cư, hộ và xã hội | ID/vòng đời, cư trú, hộ/quỹ, consent lao động, học nghề, đồ ăn và consumer xã hội. Saltworks→Fuel+Work→muối→market→Civilian mua→bữa ăn đã chạy trong cùng Realm. Sawmill→nhà ở công→council bổ nhiệm/cấp quỹ→save/load/hoàn tất cũng đã chạy cùng Realm. | Một campaign dài nối thừa kế, vụ án, nhà/thuê, chính quyền, thiếu hàng và chiến tranh; kiểm tra click-through người chơi rộng hơn formatter/smoke. |
| Hàng và cơ sở | Timber→shipment→Paper Mill→Paper→Governance mua→Investigation→Knowledge đã chạy cùng Realm/save. Forestry→Sawmill tạo ConstructionMaterial; mỏ Copper→shipment→ToolWorkshop tạo Tools; cả hai được dùng trong nhà ở công cùng Realm. Specialty supply→Atelier; Ranch→Butchery→DryFood. | Chưa có lượt dài đo nhiều nguồn/giá/tồn/cạnh tranh capacity/thiếu hàng cùng lúc; Iron và Fuel vẫn dựa tồn kho legacy, một số hàng chưa có nguồn Atlas tái sinh/đầy đủ quyền sở hữu. Xem [audit hàng hóa](reports/M5.2-current-good-source-consumer-integration-audit-2026-10-25.md). |
| Bản đồ và tuyến | Travel/Migrate, hàng hóa, foreign trade và raid qua planner chung; route preview/action UI; closure theo mùa đèo Bắc. | Không có terrain raster/passability địa lý dùng chung; thiếu playthrough dài gồm tuyến đóng trong lúc hàng/đơn vị chờ; không suy km từ map_unit. |
| Quân sự và ngoại giao | Consent→mobilization→battle→occupation/garrison/supply→người chơi phản công/rút lui; nhiều vùng và logistics có smoke riêng. | Chưa có tù binh, đầu hàng, đình chiến; chưa có đối tác tự ra quyết định chiến lược; chưa kiểm campaign nhiều vùng dài. Không thay quyết định đối tác bằng hardcode. |
| Chính quyền và nhà ở | Quỹ đô thị, housing project, một đại diện hội đồng do Sovereign chỉ định/từ chối. | Chưa ghép housing + kinh tế + chính quyền trong một save dài. Mọi mở rộng cơ chế chính trị phải phù hợp setting vương quốc phong kiến và contract quyền đang có. |
| Tích hợp/phát hành/AI | C++ là luật chung cho Python/GDExtension/Godot; contract gates từ chối model cũ. | Chưa có lượt Atlas tổng hợp nhiều mảng; Profile B/stress, hiệu năng toàn chu trình và Windows offline package; chưa learned policy qua R1–R4.

Kết luận ghép: Paper–Investigation hiện đã nối và save/resume trong cùng Realm; các tuyến Forestry/shipment, Work, market, consumer và Knowledge đã thật sự chuyền cùng hàng. Tuy nhiên chưa chứng minh campaign/save kéo dài nơi housing, council, nhiều nguồn hàng, thiếu hụt, tuyến mùa vụ và quân sự cùng tác động rồi tiếp tục đúng sau save/load. Không cộng smoke riêng thành bằng chứng cho một playthrough dài.

## Quyết định thiết kế đã rà lại: không đưa phổ thông đầu phiếu vào game

Trong lượt làm ngày25-10, một implementation thử `municipal-election-3seat-v1` đã được code và chạy kiểm tra. Khi rà theo setting vương quốc phong kiến, thiết chế phổ thông đầu phiếu không có căn cứ trong hợp đồng đã duyệt. Vì vậy code gameplay đó đã được gỡ hoàn toàn; schema/save/model contract và cơ chế hội đồng đang dùng vẫn là phiên bản trước, một đại diện do Sovereign bổ nhiệm hoặc từ chối. Không dùng kết quả smoke của implementation thử để nói tính năng bầu cử có trong game. Hồ sơ plan và report được giữ để truy nguyên quyết định, với trạng thái “không được nhận vào sản phẩm”: [plan thử đã supersede](design/M5.4-municipal-election-3seat-v1-plan.md), [report thử nghiệm và lý do loại bỏ](reports/M5.4-municipal-election-3seat-evidence-2026-10-25.md).

## Nhóm vừa hoàn tất: Sawmill + Tools + council housing trong cùng Realm

Trong seed20261024, Forestry→Sawmill tạo4 ConstructionMaterial; Copper vùng7 được chuyển tới ToolWorkshop để làm2 Tools; cư dân tự ứng cử, Sovereign bổ nhiệm và cấp quỹ, đại diện dùng dự án nhà ở công. Save/load giữa xây dựng khớp và hai world hoàn tất cùng ngày79 với8 giường mới. Không sửa gameplay code. Test đánh dấu275 Tools mở đầu đã tiêu thụ để chỉ hai Tools mới có thể cấp cho dự án, đồng thời dựng thiếu giường công bằng capacity0; report ghi rõ hai fixture. Báo cáo: [M5 Sawmill–council housing](reports/M5-sawmill-council-housing-integration-evidence-2026-10-25.md). Debug C++: build hai target pass; CTest2/2; 56+42 checks.


## Nhóm kế tiếp: Iron mine tại Hắc Sơn và nguồn sắt cho các xưởng

Audit M5.2 xác nhận Iron vẫn chỉ có tồn kho mô phỏng mở đầu, dù các xưởng Tools, Weapon Forge và quân vụ cần Iron. Atlas ghi Hắc Sơn vùng5 có quặng tốt và chuyên môn sắt. Nhóm kế tiếp thêm Mining nguồn Iron hữu hạn ở vùng5 theo setting có phiên bản và nối nó tới cùng ToolWorkshop/market/route. Plan: [Iron source](design/M5.2-iron-mine-source-plan.md). Sẽ giữ Copper ở vùng7, Silver ở vùng23; không cho thay thế kim loại.

Đổi nguồn Mining làm candidate mask và cặp region/source hợp lệ khác trước; trước code phải nâng model contract và cân nhắc save tag để runtime/save reader cũ từ chối rõ. Kiểm ngay C++ mine/production/save readers và contract gate; sau core pass build/kiểm consumer Python/native/Godot theo plan. M5.5 Profile B, stress/performance và package vẫn bắt buộc; chưa chạy RL trước M5 + core refactor.

## Thứ tự còn lại

1. Tích hợp và đo một chuỗi Realm/save có nhiều hệ thống, sau đó xử lý thiếu hụt M5 trực tiếp mà nó bộc lộ.
2. Hoàn thành các cơ chế sản phẩm M5 còn thiếu theo setting phong kiến (ngoại giao/quân sự/city services theo roadmap; không tự thêm quyền/consent trái contract).
3. Mốc tích hợp đầy đủ: Python/native/Godot theo nhóm ổn định; nhiều năm/profile, stress/performance, package Windows offline; kiểm model gate và điều kiện learned policy đã chốt.
4. Chỉ khi base M5 hoàn tất mới làm core refactor R1–R4 theo kế hoạch; sau đó tạo dataset, huấn luyện, chọn/validation/holdout RL. Mỗi run RL phải lưu source metrics và chart đúng run/version, báo rõ cách sinh dữ liệu và không trộn train/validation/holdout.

## Nguồn và ranh giới kiểm

Roadmap/decision: `IMPLEMENTATION_PLAN.md`; bài giải thích kiến trúc: `DEVELOPER_GUIDE.md`; hệ vấn đề/phép kiểm: `TECHNICAL_PROBLEMS.md`; cách build/test: `DEVELOPMENT_WORKFLOW.md`; nhật ký: `DEVELOPMENT_HISTORY.md`; bằng chứng theo revision/run ở `docs/reports/` và `artifacts/`. Kiểm nhỏ đúng target; gom Python/native/Godot vào khi nhóm ổn định; không chạy lại bộ kiểm trên source không đổi nếu không có lý do. Không đưa model/checkpoint/artifact chưa được giao vào commit.

Ảnh chụp CURRENT_TASK trước khi hợp nhất trạng thái ngày25-10 được lưu tại [archive snapshot](archive/CURRENT_TASK-2026-10-25-before-consolidation.md); đây chỉ là lịch sử, không phải chỉ dẫn triển khai.
