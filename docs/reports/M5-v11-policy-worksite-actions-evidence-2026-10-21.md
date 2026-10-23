# M5 — policy cấp vật tư và gửi ca tại xưởng

Ngày 2026-10-21. Lát này nối các action policy với giao dịch vật tư, quyền làm việc, lịch Work và production của xưởng đã tạo ở v10.

## Người chơi/agent nhận được gì

Policy có các lựa chọn để đưa đúng nguyên liệu vào xưởng, rút sản phẩm, cấp quyền làm có thời hạn cho một worker khác đang ở cùng vùng, và gửi ca Work gắn với chính xưởng đó. Xưởng phải có nguyên liệu và worker phải hiện diện; command từ chối action hết quyền, sai vùng, thiếu hàng hoặc trùng lịch mà không đổi một phần ledger. Khi bật Work timeline, Godot và Python vẫn nhận được action list, mask và observation để policy tiếp tục quyết định.

Action surface có tối đa tám dòng xưởng được lấy mẫu ổn định theo actor, lượt lấy mẫu và ngày mô phỏng. Observation theo dòng xưởng gồm ngành, vùng, tồn kho liên quan, tiền/công suất, và tuổi/skill worker đang được chọn. Trong quá trình self-review phát hiện các đặc trưng này bị ghi đè bởi phần điền feature mặc định; đã chuyển phần mặc định lên trước, thêm assert riêng cho site stock, cash, capacity và worker skill.

## Thay đổi contract

- Chuyển từ `realm-v10` 89 action/1.196 observation sang `realm-v11` 129 action/1.676 observation; slots 89–128 là tám site × năm thao tác.
- Nạp/rút hàng dùng `EstablishmentTransferCommand`; cấp quyền dùng `WorkAuthorizationCommand`; gửi ca một ngày (64 tick) dùng `ScheduleCommand`. Candidate preflight trong Realm và typed command đều kiểm tra lại trước khi ghi state.
- Model contract lên v5. Native manifest chỉ nhận schema v11 với shape 1.676/129 và từ chối v10 trở xuống kể cả development override. Có architecture module `realm_model_v11.py` để kiểm shape; chưa tạo trainer, optimizer hoặc checkpoint v11.
- Gỡ điều kiện khiến Realm và GDExt ẩn candidates, mask, observation hoặc action list sau khi timeline contract bật.

## Đã kiểm tra

- Core CTest Release **18/18** và Debug **18/18**. Thêm oracle Cloth: policy mua 1 Fiber, đưa vào xưởng, mở Work timeline, cấp quyền cho worker, gửi ca 64 tick, qua một ngày production tạo đúng 1 Cloth, rồi rút Cloth. Lưu giữa ca, nạp lại và chạy tiếp cho fingerprint khớp.
- Python Debug/Release: model contract 14; training preflight **479** gồm shape recurrent cũ và v9/v10/v11; clock **173** + 7 checkpoint; gameplay config **49** + 7; workshop policy path; transfer/production **32** + 7; work authorization 7 checkpoint; migration **40**; household fund 11 + school study 7.
- Native `runtime_contracts`: Debug **1/1**, Release **1/1**; kiểm tra manifest v11 hợp lệ và model shape v10/v9/v8 bị từ chối.
- Windows GDExtension Release build thành công. Godot 4.5.1 headless old-bundle compatibility guard **5/5**.

## Còn thiếu

Self-review only, không có reviewer độc lập. Godot smoke hiện xác nhận loader từ chối bundle cũ; chưa thể chạy action list và gameplay UI bằng model v11 vì chưa có checkpoint tương thích. Realm core và Python đã chạy chu trình xưởng; learned peer policies/cross-play và trainer/export v11 vẫn chưa có. Không huấn luyện hoặc tuyên bố R1 đạt. M5/core refactor còn mở.

Không commit/push vì working tree có nhiều lát M5/core của tác vụ khác và không thể tách sạch.
