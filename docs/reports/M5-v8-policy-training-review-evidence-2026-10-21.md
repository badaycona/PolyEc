# Rà soát PPO realm-v8 sau khi chạy thử

Ngày 2026-10-21. Review bàn giao đọc trước khi tiếp tục nằm tại [PPO-training-review-2026-10-21.md](../handoff/PPO-training-review-2026-10-21.md). Review đó không được xem là nghiệm thu độc lập. Bản chụp các source/evidence dùng ở đây: [manifest](../../artifacts/m5-v8-ppo-training-review-2026-10-21-manifest.json).

## Kết quả huấn luyện đã chạy

Hai nhóm đã hoàn tất 3 seed × 300 giây trên Windows CPU bằng RealmBatch một nhân vật trọng tâm. Cùng người được kiểm tra trong 100 world dùng một seed; controller được đánh giá trong script bằng argmax, không phải cách lấy mẫu categorical ở runtime. Các đối tác phụ vẫn theo cấu hình cũ trong RealmBatch, vì thế đây chưa phải đánh giá cross-play hoặc toàn xã hội.

| Nhóm | Quyết định theo seed | Model được chọn | So với khởi tạo | So với giữ | So với ngẫu nhiên hợp lệ | So với baseline | Kết luận |
|---|---:|---:|---:|---:|---:|---:|---|
| Quản trị | 208,896 / 161,792 / 124,928 | 1 | +0.0018; CI95 [-0.0111, 0.0151] | -0.0369; [-0.0894, 0.0087] | -0.0820; [-0.1374, -0.0329] | -0.0617; [-0.1106, -0.0186] | Không cải thiện rõ; thua baseline |
| Kinh tế | 282,624 / 278,528 / 278,528 | 3 | +1.8141; [1.6050, 2.0436] | +2.2125; [2.0490, 2.3803] | +2.6085; [2.4098, 2.8076] | -0.5819; [-0.6571, -0.5087] | Cải thiện so với khởi tạo, nhưng thua baseline và hai controller khác |

Điểm là utility chuẩn hóa của nhân vật, không phải tiền. `full_R1_pass=false` ở cả hai model. Số quyết định khác nhau giữa seed do script dừng theo thời gian, nên không được dùng để kết luận một seed/cấu hình học hiệu quả hơn. Không chạy `realm_sampled_v8.py`; hai nhóm chưa có môi trường đối tác đại diện để kết quả đó làm căn cứ gameplay.

### Categorical sampled audit hoàn tất

Evaluator nhận danh sách family để chạy được hai nhóm có checkpoint; seed và RNG được ghép cặp giữa các chính sách. Mỗi family có 200 episode trên hai seed 170000 và 170099, categorical sampling đúng kiểu runtime, 0 hành động không hợp lệ.

| Nhóm | Utility learned | Learned trừ khởi tạo (CI95) | Learned trừ hold (CI95) | Learned trừ random (CI95) | Learned trừ baseline (CI95) | Kết luận sampled |
|---|---:|---:|---:|---:|---:|---|
| Quản trị | -0.16872 | -0.02504 [-0.04149, -0.01011] | +0.02170 [0.00323, 0.03825] | -0.02029 [-0.03716, -0.00521] | -0.00235 [-0.01558, 0.01237] | Không cải thiện so với khởi tạo; thua random; nằm trong biên không kém baseline |
| Kinh tế | +0.24513 | -0.70635 [-0.78205, -0.62897] | -0.33481 [-0.43364, -0.23234] | -0.70237 [-0.77872, -0.62265] | -2.53433 [-2.69522, -2.37547] | Thua khởi tạo, hold, random và baseline |

Kết quả sampled khác hẳn argmax: governance chuyển từ chưa rõ so với khởi tạo sang giảm utility có CI âm; economic chuyển từ cải thiện +1.814 theo argmax sang kém khởi tạo -0.706 theo sampled. Vì vậy các model hiện có không đạt initial-improvement R1; economic cũng không đạt baseline noninferiority. Dữ liệu vẫn đến từ RealmBatch một actor trọng tâm với đối tác cũ, nên không đại diện cross-play hoặc toàn thế giới. Raw report: `artifacts/models/realm-v8-dev-2026-10-21/runtime-sampling-audit.json`.

Nhóm dân sự dừng theo yêu cầu đọc review, sau khoảng 191 giây ở seed 1. Còn vài checkpoint thử trong thư mục; không có lịch sử seed đầy đủ, đánh giá cuối, manifest hoặc ONNX. Chúng không được tính là model hoàn tất.

## Audit khả năng nhìn và chọn hành động

- Action 72 chỉ đưa ra một đối tượng cưới xác định theo ID người hành động. Feature của đối tượng chỉ là target ID đã chuẩn hóa chung với target vùng; không có tuổi, quan hệ, hộ hoặc lợi ích của người nhận. Policy không thể chọn giữa nhiều người nhận có thể kết hôn.
- Action hiện có không cung cấp thao tác tạo/fund xưởng, chuyển vật tư, cấp phép lao động hoặc liên kết Work với establishment ID. RealmBatch không dựng chu trình xưởng; các lần huấn luyện không học được payroll/sản xuất M5.
- Observation/state và reward là bản C++ Realm hiện tại; reward utility cá nhân giữ khác biệt theo họ. Tuy nhiên training partners là dân sự Work/Study cố định và người quản trị/kinh tế chọn ngẫu nhiên hợp lệ; môi trường không có cross-play learned policy.
- Trainer hiện chọn update theo số giây, làm số quyết định mỗi seed khác nhau. Thiếu approximate KL, clip fraction, explained variance/value error và coverage theo hành động/tình huống. Evaluator tích hợp dùng argmax, khác runtime categorical sampling.
- Kiểm tra ONNX đã được thực hiện sau khi huấn luyện governance. `onnx==1.23.2` được cài vào thư mục phụ thuộc cục bộ trong `build/m5-python-ai`; môi trường Python chung không được sửa. Không có kiểm tra export trước lần chạy đầu, và chưa chạy so khớp Torch/Python/Native ONNX cho các model này.

## PPO trainer đã sửa và kiểm tra

- ONNX được import-check trước khi tạo output/rollout; output phải là thư mục sạch để tránh checkpoint cũ bị nhầm là kết quả của run mới. Training chỉ chạy tối thiểu 25 update để tới giai đoạn chọn model ở sáu vùng; env count phải chia hết cho 16.
- Ba `fit` smoke seed với 16 môi trường × 64 bước chạy cùng 1.024 quyết định/seed và ghi diagnostics. Timeout CLI ở mức 25 update/0 giây dừng sau 1.024/25.600 quyết định, ghi `INCOMPLETE.json`, thoát mã 2 và không tạo selection, evaluation hay ONNX.
- Mỗi update ghi rollout, policy action sampling, C++ environment step và optimizer time; approximate KL, clip fraction, explained variance, critic MSE, reward/advantage statistics, action histogram và histogram theo số lựa chọn hợp lệ.
- Python 3.12 `py_compile` cho trainer/evaluator pass. Đây là kiểm tra thay đổi công cụ PPO, không kiểm tra học thật và không tạo policy release.

## Quyết định tiếp theo

Không chạy huấn luyện dài hoặc chuyển GPU lúc action/observation còn không cho chọn người nhận hôn nhân và chưa biểu diễn tạo/fund/cấp hàng/Work-site/payroll trong adapter; nhóm đối tác cũng chưa đại diện. Ưu tiên tiếp theo là nâng action/observation contract và đưa giao dịch xưởng qua candidate/command chung cho runtime lẫn Python. Sau đó làm scenario đánh đổi, learned partner/cross-play, holdout chuyển giao, chạy fixed sample budget và kiểm tra full-world. Cả hai v8 pilot hiện thất bại R1.
