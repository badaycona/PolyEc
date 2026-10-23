# M5 v31 — kéo dài Governance/Civilian Realm v11 từ checkpoint

## Lý do

## Thứ tự thực hiện hiện hành — yêu cầu người dùng 2026-10-22

Không resume kế hoạch v31 cho tới khi T1–T3 và toàn bộ luật/các chu trình gameplay M5.0/1–M5.4 hoàn tất trong C++ core, rồi được nối vào Python training adapter. Trước khi train lại phải chạy smoke nhỏ C++→Python để kiểm observation, legal actions/action slots, reward và việc adapter gọi cùng C++ core. M5.5 (profile B, stress/long-run, hiệu năng toàn chu trình và package Windows) cùng UI/native presentation thực hiện sau training. Đủ điều kiện bắt đầu train không đồng nghĩa toàn bộ M5 đã hoàn tất; các gate M5.5 và sản phẩm vẫn cần để kết luận M5. Không chạy plan này trong khi các điều kiện trên chưa đạt.

V30 đã huấn luyện ba seed mỗi family đủ 25 update, nhưng selector độc lập loại cả sáu: utility so với baseline thấp, còn utility so với policy ban đầu chưa cho thấy cải thiện có ý nghĩa thống kê. Không có lỗi invariant hoặc manifest. V31 kiểm tra một giả thuyết hẹp: liệu policy cần thêm lượt học với cùng luật/reward/optimizer để đạt tiêu chí hiện hành hay không. Không đổi gameplay, reward, observation, action, model contract, baseline, optimizer hay tiêu chí chọn.

## Kế hoạch chạy

- Resume checkpoint v30 của Governance và Civilian, seed 1, 2, 3; giữ nguyên Realm v11, 6 vùng, horizon 128, 16 environment, 64 rollout step, 64 chuỗi/minibatch, 3 PPO epoch, learning rate `0,0003`, entropy `0,01`, curriculum `none`, dev baseline đối tác.
- Đặt tổng đích 75 update/seed. Checkpoint v30 đã hoàn tất update 25, nên lượt mới tạo thêm 50 update ×1.024 =51.200 quyết định/seed; cộng lượt trước là 76.800 quyết định/seed. Tổng thêm 307.200 quyết định cho sáu seed.
- Không đặt giới hạn theo thời gian. Trainer chạy cho đến khi đạt đúng 75 update; checkpoint vẫn ghi sau từng update để có thể resume nếu người dùng dừng hoặc máy gặp gián đoạn. Không validation seed thiếu update và không chạy seed song song.
- Lưu kết quả vào `build/m5-v31-extended-training/`, tách khỏi output v30. Copy `initial.pt` v30 vào output mới để selector so với cùng policy khởi đầu; resume dùng checkpoint v30 và kiểm tra training identity bằng trainer. Không sửa output v30.
- Tạo training-set lock mới, ghi hash policy/manifest, số update/decision, config và nguồn checkpoint. Chỉ validation nếu toàn bộ ba seed của family đạt 75 update và cùng số quyết định.

## Dải validation và tiêu chí

Governance dùng world seed **196000–196019**; Civilian dùng **197000–197019**; mỗi dải gồm 20 world, chạy 128 ngày. Dải 194000–195019 của v30 không tái sử dụng. Trước khi chạy, rà artifacts/history để xác nhận hai dải mới chưa được dùng trong đánh giá policy này.

Giữ nguyên selector và các ngưỡng v30: cận dưới CI 95% utility so baseline ≥ -0,05; cận dưới CI utility so policy ban đầu >0; cận dưới chênh lệch survival ≥0; cận trên insolvency/default ≤0; tối thiểu 25 update. Dù training đạt 75 update, không được gọi policy là đạt chất lượng nếu selector loại. Không chạy holdout hoặc export model khi family bị loại.

## Phạm vi kiểm tra

Ngay trước chạy: xác nhận hash checkpoint/config identity, output mới, các seed validation chưa dùng và dependency ONNX. Mỗi lượt hoàn tất phải xác nhận manifest/training-set lock 75 update, 76.800 quyết định tổng/seed, invariant và `peer_eligible=false`. Sau đó chạy selector validation-only riêng cho family đủ seed. Không build lại toàn project vì chỉ dùng binding/trainer hiện có; Python adapter đã được kiểm trực tiếp trong v30 sau khi sửa khóa candidate. Native/Godot chờ tới khi đủ ba family được selector chấp nhận.

## Giới hạn

Đây là huấn luyện thăm dò có lượng công việc đo bằng update: sáu seed ×50 update bổ sung =300 update, 307.200 quyết định focal, không phải tác vụ training production. Không đặt timer hay lịch dừng theo giờ. Không có cam kết rằng thêm update sẽ cải thiện utility. Nếu không có candidate, bước kế tiếp phải giải thích từ rollout/evaluation diagnostics xem hành động nào, utility term nào hoặc môi trường nào tạo hụt điểm trước khi đề xuất thay đổi thuật toán/reward; không được điều chỉnh theo cùng tập validation hoặc nới tiêu chí.

