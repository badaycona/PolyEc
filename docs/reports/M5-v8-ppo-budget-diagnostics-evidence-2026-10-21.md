# M5 — PPO ngân sách mẫu và diagnostics

Ngày 2026-10-21. Đã áp dụng review trong [handoff](../handoff/PPO-training-review-2026-10-21.md) cho trainer/evaluator v8. Đây là hardening công cụ và audit policy phát triển, không phải nghiệm thu R1 hoặc full-world.

## Thay đổi

- Trainer kiểm tra import ONNX trước khi tạo output hoặc bắt đầu rollout; yêu cầu output rỗng để tránh checkpoint cũ bị nhầm với run mới.
- Mỗi run cần ít nhất 25 update để tới curriculum sáu vùng; mọi seed phải hoàn thành cùng `updates × envs × 64` quyết định. `--seconds` là giới hạn bảo vệ. Seed không đạt ngân sách sẽ ghi `INCOMPLETE.json`, dừng với mã 2 và không tạo selection lock, final evaluation hoặc ONNX.
- Mỗi update ghi riêng thời gian rollout, lấy action từ policy, gọi bước môi trường C++ và tối ưu mạng. Bổ sung approximate KL, clip fraction, explained variance, critic MSE, thống kê reward/advantage, action counts và action counts theo số lựa chọn hợp lệ.
- Sampled evaluator nhận danh sách family, giữ categorical sampling và common uniform RNG. Có thể kiểm các checkpoint hiện có dù civilian chưa hoàn tất.

## Đã kiểm tra

- Python 3.12 `py_compile` trên trainer và evaluator: pass.
- Smoke gọi `fit` ba seed với 16 môi trường × 64 bước × 1 update: cả ba hoàn thành đúng 1.024 quyết định và ghi đủ diagnostics. Đây chỉ là smoke của hàm fit, không phải run huấn luyện được chọn model.
- Timeout qua CLI với `--updates 25 --envs 16 --seconds 0`: seed dừng tại 1.024/25.600 quyết định, có `INCOMPLETE.json`, exit code 2; xác nhận không có selection/evaluation/ONNX/manifest cuối.
- ONNX 1.23.2, Torch 2.5.1+cpu, NumPy và extension Python C++ nạp được trong `PYTHONPATH` của build cục bộ.
- Sampled categorical governance/economic trên hai seed × 100 episode/family: 200 episode mỗi nhóm, 0 action invalid. So khởi tạo: governance -0.02504 (CI95 [-0.04149,-0.01011]); economic -0.70635 ([-0.78205,-0.62897]). Economic kém baseline -2.53433 ([-2.69522,-2.37547]). Raw: [runtime-sampling-audit.json](../../artifacts/models/realm-v8-dev-2026-10-21/runtime-sampling-audit.json).
- `full_R1_pass=false` cho cả hai; sampled run vẫn dùng RealmBatch một actor trọng tâm và partner setup cũ.

## Giới hạn và việc kế tiếp

Self-review; không có reviewer độc lập. Chưa chạy training dài sau khi review. Action contract vẫn không cho chọn nhiều đối tượng hôn nhân, tạo/fund/xuất nhập vật tư xưởng hoặc chọn worksite làm nơi Work; reward/partner scenario không kiểm cross-play. Cần sửa action/observation/command contract và đưa M5 workshop cycle vào adapter trước, sau đó mới làm tradeoff scenario/holdout/cross-play và chạy training có fixed budget.
