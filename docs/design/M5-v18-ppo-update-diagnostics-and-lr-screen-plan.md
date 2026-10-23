# M5 v18 — đo PPO sau cập nhật và sàng lọc một tham số

## Mục tiêu

Sửa diagnostics của `realm-v11` để KL, clip fraction và ratio đo policy sau optimizer step. Bổ sung loss riêng của policy/critic, độ lớn gradient và phân bố reward, advantage thô/chuẩn hóa, return. Dùng một thí nghiệm có cùng seed, số quyết định và đối tác để kiểm tra một learning rate cao hơn; đây là thăm dò, không phải lựa chọn model.

## Phạm vi

- `ai/training/realm_train_v11.py`: không đổi cách lấy mẫu/action/reward hay optimizer update; chỉ sửa thời điểm đo diagnostics và bổ sung trường log.
- Hai run economic, seed 1, 6 vùng, 16 environments, 64 steps/update, 25 updates (25.600 quyết định); run A LR=0,0003, run B LR=0,0012. Giữ nguyên mọi cấu hình khác.
- Đánh giá run B trên cùng 20 world seeds đã dùng cho validation trước đây chỉ như tập thăm dò đã xem; không dùng cho lựa chọn cuối/R1.

## Điều kiện hoàn thành

1. Python syntax check và smoke 1 update ghi đầy đủ metric mới.
2. Run đủ ngân sách chứng minh logging sau optimizer và không thay đổi model cho cấu hình LR gốc; policy SHA phải trùng checkpoint đã đánh giá.
3. Run LR cao giữ bằng sample budget, holdout diagnostic ghi `r1_pass=false` khi thiếu 3 training seeds/100 worlds/full horizon.
4. Ghi rõ giới hạn self-review; không cấp `peer_eligible` hoặc tuyên bố policy đạt.
