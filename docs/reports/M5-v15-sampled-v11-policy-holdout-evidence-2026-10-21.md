# M5 v15 — sampled holdout PPO v11

## Thay đổi

Thêm `ai/evaluation/realm_policy_holdout_v11.py` để chạy candidate PPO, initial policy, baseline, hold và random-legal trong cùng C++ Realm với cùng world seed và actor RNG. Policy lấy mẫu categorical theo mask mới; run chỉ nhận training output hoàn tất và đúng contract v11. Evaluator thu utility chuẩn hóa actor đại diện, survival/health/cash/debt/insolvency/obligations/defaults và action/sản lượng xưởng.

`realm_coplay_v11.py` bổ sung outcome của actor đại diện. Evaluator khóa R1 false nếu thiếu ba training seeds, 100 holdout worlds, cải thiện paired so với initial, noninferiority so baseline trong biên 0,05, hoặc không bảo toàn survival/insolvency/default outcomes.

## Kiểm chứng

- Python syntax check pass cho trainer, full-world co-play evaluator và sampled holdout.
- Tạo một output trainer smoke economic (1 update, 1.024 decisions) để kiểm đường nạp checkpoint; chạy 2 holdout worlds × 2 ngày × baseline/learned/initial/hold/random.
- Tất cả Realm invariant pass; chạy lại cùng seeds và weights tạo cùng mọi metric ngoài thời gian đo. Utility khác biệt bằng 0 ở horizon 2 ngày vì focal actor không có quyết định mới trong khoảng đó.
- Evaluator báo `r1_pass=false`, do chỉ một training seed và hai holdout; baseline/initial CI bằng 0. Đây là hành vi chặn đúng, không phải đánh giá chất lượng policy.

## Giới hạn

Chưa có ba training seeds, 100 holdout worlds, model selection trên validation, peer eligibility, R2/R3 interventions, ONNX parity/R5 hoặc profile B. Smoke model một update không được dùng làm learned policy/peer. Không chạy training dài.
