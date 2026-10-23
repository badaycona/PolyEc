# M5 v16 — chọn checkpoint v11 bằng validation

## Mục tiêu

Chọn policy từ ít nhất ba training seeds dựa trên validation đã định trước, trước khi mở holdout độc lập. Checkpoint được đóng thành bundle `peer_eligible=false`; bộ holdout riêng mới có thể đánh dấu đủ điều kiện làm frozen peer khi R1 đạt.

## Phạm vi

- Thêm `ai/training/realm_train_set_v11.py` để chạy ít nhất ba seed tuần tự với cấu hình/budget giống nhau và tạo runset lock.
- Thêm `ai/evaluation/realm_select_v11.py` dùng chung sampled evaluator, seed world và RNG actor.
- Kiểm family/contract, fixed decision budget đã hoàn tất, ba training seeds khác nhau và validation seeds duy nhất.
- Loại ứng viên dưới 25 PPO updates; R1 vẫn yêu cầu sáu vùng và full horizon theo họ.
- Lưu metric từng run, rule chọn và seed được chọn; không dùng holdout để chọn.
- Tạo bundle riêng policy/initial/manifest và giữ `peer_eligible=false`.
- Holdout chỉ đánh dấu eligibility khi selection record hợp lệ, validation/holdout tách biệt và đầy đủ tiêu chí R1.

## Điều kiện nghiệm thu

1. Chạy validation từng training seed trên cùng world seeds.
2. Training set xác nhận đủ seed distinct, bằng số quyết định và cấu hình trước khi selector đọc.
3. Chỉ ứng viên có initial improvement, baseline noninferiority và survival/insolvency/default không xấu hơn mới đủ điều kiện; chọn theo paired mean baseline tốt nhất.
4. Không có ứng viên đạt thì không tạo selected bundle.
5. Manifest lưu split, hash checkpoint, cấu hình và trạng thái chưa qua holdout.

Self-review only; không truy cập/chọn trên holdout trong selector.
