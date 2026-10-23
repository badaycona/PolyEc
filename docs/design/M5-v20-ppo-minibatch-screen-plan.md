# M5 v20 — PPO mini-batch screen với reward v19

## Mục tiêu

Review PPO đề nghị đo hiệu quả update trước khi tăng thời gian huấn luyện. Ở v19, mỗi PPO epoch cập nhật một batch gồm 64 chuỗi recurrent (1.024 bước), tạo KL sau optimizer thường khoảng 10⁻⁶ và clip fraction bằng 0. Screen một yếu tố: giảm còn 16 chuỗi mỗi batch, giữ nguyên sample budget, reward, partner, seed, optimizer, learning rate và số epoch.

## Thay đổi

- `ai/training/realm_train_v11.py`: thêm `--minibatch-sequences` (16..64, chia hết số chuỗi rollout), dùng làm chiều batch của hoán vị trong mỗi epoch; ghi vào resume identity và manifest. Mặc định 64 giữ đường chạy cũ.
- `ai/training/realm_train_set_v11.py`: truyền và ghi tham số cố định trong runset.
- `ai/evaluation/realm_select_v11.py`: khóa số chuỗi mini-batch khi đối chiếu runset.
- Không đổi dữ liệu rollout, categorical sampling, reward v19, recurrent sequence length 16, loss, optimizer, learning rate, epochs, action/observation/model contract/save.

## Screen

- Economic, seeds 1/2/3; 6 vùng; 16 environments; 64 rollout steps; 25 updates; 3 epochs; LR 0,0003; 25.600 quyết định mỗi seed.
- V19 comparison: 64 chuỗi mỗi mini-batch (một batch/epoch, ba optimizer steps/update). Screen: 16 chuỗi mỗi mini-batch (bốn batch/epoch, 12 optimizer steps/update).
- Sampled paired validation trên 20 thế giới mới 192000–192019, 128 ngày; cùng seed để so định lượng với v19 diagnostic nhưng khác toàn bộ validation worlds đã xem trước đó.
- Đọc post-step KL/ratio/clip, loss/gradient, explained variance, action coverage, worksite/payroll/output và utility/cash/debt/invariants. Không selection/peer promotion nếu không đạt điều kiện hiện hữu.

## Gate và giới hạn

- Unit smoke kiểm biến thể 16/32/64, reject batch size không chia hết, manifest/resume khóa đúng budget; Python syntax và trainer 1 update.
- Chạy đủ ba seed rồi selector validation-only. Dừng nếu không đủ budget hoặc metrics cho thấy lỗi rollout/contract. Đây là một screen optimizer, không là kết luận PPO hoàn tất; partner diversity, curriculum/transfer, 100 holdouts/full horizon và các gate M5/core còn mở.
- Review là self-review Codex theo v19 diagnostic, không phải review độc lập.
