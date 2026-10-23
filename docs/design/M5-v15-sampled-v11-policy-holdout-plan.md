# M5 v15 — sampled holdout và đánh giá checkpoint v11

## Mục tiêu

Đánh giá checkpoint PPO v11 bằng cùng cách lấy mẫu categorical của gameplay trên Realm C++ đầy đủ. Mỗi checkpoint được ghép cặp theo cùng world seed và RNG actor với initial, baseline, hold và random-legal. Đo utility actor đại diện, sống sót/sức khỏe/tiền/nợ/obligation/default, hành động policy và sản lượng xưởng.

## Phạm vi

- Thêm `ai/evaluation/realm_policy_holdout_v11.py` dùng `run_variant` trong evaluator Realm co-play.
- Nạp output của trainer theo schema/family/model contract/config fingerprint; từ chối run chưa hoàn thành hoặc schema sai.
- Chạy từng training seed qua cùng danh sách holdout; report paired CI95 riêng cho baseline và initial.
- Tạo kết luận R1 false trừ khi đủ 3 training seeds, 100 holdout worlds, cải thiện so initial, noninferiority so baseline trong biên 0,05 và không giảm survival/insolvency/default outcomes.
- R1 chỉ xét profile 6 vùng và full focal horizon: 512 quyết định economic × 7 game-day = 3.584 Realm-day; 128 quyết định governance/civilian × 30 game-day = 3.840 Realm-day; mỗi run có tối thiểu 25 PPO updates.
- Thêm dữ liệu outcome per-family cho evaluator C++ hiện có.

## Ngoài phạm vi

Không cấp `peer_eligible` chỉ từ smoke/validation; không chọn winner trên holdout; không export ONNX hoặc train dài. Chất lượng policy phải được xác nhận theo full R1, không dựa vào smoke ngẫu nhiên.

## Điều kiện nghiệm thu

1. Candidate actions categorical và hợp lệ theo mask mới trong cùng Realm.
2. Paired replay cho seed/mode giống nhau tạo ra kết quả ổn định; C++ invariant đúng.
3. Báo đủ paired utility và outcome của người chơi, kèm số episode/seeds thực chạy.
4. Thiếu số training seeds/holdouts hoặc CI không đạt thì `r1_pass=false`.

Self-review only trên shared dirty checkout; không thay đổi dữ liệu người dùng khác.
