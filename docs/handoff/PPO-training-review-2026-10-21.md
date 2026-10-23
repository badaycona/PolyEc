# Nhận xét quy trình PPO từ trao đổi phụ — 2026-10-21

Người dùng yêu cầu chuyển nhận xét sang tab chính để điều chỉnh kịp thời. Trao đổi phụ chỉ đọc source và kết quả; không huấn luyện, chạy test hoặc sửa implementation. Đây không phải review độc lập để xác nhận hoàn thành M5. Tab chính cần đối chiếu source mới nhất trước khi sửa.

## Kết quả đã đọc

Nguồn: `ai/training/realm_train_v8.py`, `ai/training/realm_model_v8.py`, `bindings/python/realm_binding.cpp`, `native/realm_runtime.cpp`, và `artifacts/models/realm-v8-dev-2026-10-21/governance/evaluation.json`.

Governance selected seed 1:

| So với | Chênh utility chuẩn hóa | CI95 |
|---|---:|---|
| Initial | +0.0018247 | [-0.0110935, 0.0150619] |
| Baseline | -0.0617487 | [-0.1106162, -0.0186386] |
| Random legal | -0.0819983 | [-0.1374018, -0.0329260] |

`initial_improvement=false`, `baseline_noninferiority=false`, `full_R1_pass=false`. Chưa chứng minh cải thiện so với initial; kém baseline và random theo evaluator hiện tại. Đây là utility chuẩn hóa, không phải tiền hoặc accuracy. Evaluator dùng argmax nên chưa thể đồng nhất kết luận với sampled gameplay. Economic chưa có evaluation.json tại thời điểm đọc; không kết luận về nhóm đó.

## Những điều cần sửa hoặc kiểm chứng

1. **Môi trường đối tác chưa đại diện game.** RealmBatch train một focal actor; civilian peers dùng Work/Study cố định, governance/economic peers uniform legal. Cần đánh giá và huấn luyện với tập checkpoint đối tác đóng băng, nhiều seed và thành phần quần thể; cross-play rồi chuyển sang full world. Giữ mục tiêu cá nhân và xung đột lợi ích, không ép toàn xã hội dùng một reward.
2. **Chưa bao phủ chu trình xưởng M5.** Adapter khởi tạo Realm mặc định 128 người và execute candidate actions; không dựng tạo/fund xưởng → cấp nguyên liệu → Work có worksite → payroll/production. Cần audit observation/action/reward để policy nhìn thấy và lựa chọn được xưởng, hợp đồng và đối tác thật. Thêm action hoặc khớp metadata không chứng minh đã học cơ chế. Marriage slot 72 chỉ có target cố định theo ID cũng giới hạn việc học chọn người nhận.
3. **Đánh giá lệch cách chơi.** evaluate() dùng argmax; runtime categorical sampling. Hoàn tất sampled evaluation đúng runtime, dùng nguồn ngẫu nhiên ghép cặp khi so policy. Argmax chỉ là phép chẩn đoán bổ sung.
4. **Curriculum chưa đo năng lực.** Vùng tăng 1→2→6 theo update. Cần tình huống nhỏ có đánh đổi, can thiệp chỉ đổi một yếu tố, holdout khác tổ hợp/đối tác và phép kiểm chuyển giao. Đổi seed đơn thuần chưa chứng minh generalization.
5. **Thiếu số đo nguyên nhân không học.** Thêm approximate KL, clip fraction, critic explained variance/value error, phân bố advantage/reward, phân bố hành động theo hoàn cảnh và coverage cơ chế. Phân biệt thiếu thông tin/hành động, reward khó học và update không hiệu quả.
6. **Budget thời gian làm số mẫu lệch.** Ba seed nhận 208896 / 161792 / 124928 decisions. So chất lượng giữa seed/cấu hình nên dùng cùng sample budget, kèm timeout bảo vệ. Tách thời gian C++ tạo trải nghiệm với thời gian cập nhật mạng trước quyết định dùng GPU.
7. **Chuẩn bị chạy dài.** Kiểm đủ dependency export trước train. Resume hiện tải weights rồi reset critic/optimizer; để khôi phục chính xác cần lưu optimizer, RNG và tiến độ. Kaggle phải biên dịch cùng C++ core trên Linux, kiểm parity; không dùng Windows .pyd hoặc viết luật khác bằng Python.

## Thứ tự đề nghị

Audit khả năng quan sát/lựa chọn và coverage → thêm diagnostics + sampled evaluation → tình huống có đánh đổi và holdout → nhiều learned partners/cross-play → full-world transfer. Chưa có căn cứ chỉ tăng GPU hoặc giờ train để giải quyết các thiếu sót trên. Giữ toàn bộ mục tiêu M5/core và yêu cầu R1–R5; chưa gọi pilot là policy sản phẩm.

Công cụ gửi tin sang thread chính đã lỗi hai lần với `sendRequest.bind is not a function`; tệp này được tạo làm bản bàn giao thay thế, không sửa CURRENT_TASK hoặc source đang có người làm.
