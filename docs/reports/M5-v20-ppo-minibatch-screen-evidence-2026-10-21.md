# M5 v20 — PPO mini-batch screen

## Phạm vi và người chơi/NPC thấy gì

Đây là thay đổi cách gom chuỗi recurrent vào PPO optimizer, không đổi gameplay, reward v19, action, observation, checkpoint tensor hoặc luật Realm. Mặc định 64 chuỗi giữ hành vi cũ; cấu hình screen 16 chuỗi tạo 4 mini-batch mỗi epoch thay vì 1. Manifest và exact-resume identity khóa giá trị này; selector từ chối gộp run khác cấu hình.

Plan: [M5 v20](../design/M5-v20-ppo-minibatch-screen-plan.md). Self-review Codex, không có reviewer độc lập.

Source, run và selection SHA-256: [manifest](../../artifacts/m5-v20-ppo-minibatch-screen-2026-10-21-manifest.json).

## Gate triển khai

- Python syntax: trainer/runset/selector pass.
- Smoke 1 update ở batch 16: 1.024 quyết định, 12 optimizer mini-batch step; manifest/checkpoint identity ghi `minibatch_sequences=16`.
- Batch size 17 bị từ chối trước rollout vì không chia hết 64 chuỗi.
- Ba seed Economic đủ 25 updates ×25.600 quyết định/seed; cùng 16 environments, rollout 64, 3 epochs, LR 0,0003 và reward version v19.
- Sampled paired validation: 20 thế giới 192000–192019 ×128 ngày; invariants true cho cả ba run.

## Kết quả

| Seed | KL median / max | Clip fraction lớn nhất | Utility so baseline (CI95) | Utility so initial (CI95) | Cash delta | Debt delta | Insolvencies thêm | Xưởng bình quân | Công site trả | Cloth cuối kỳ |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 1,17e-5 / 1,27e-4 | 0 | -0,425 [-0,473; -0,376] | +0,006 [-0,032; +0,044] | -348,9 | +125,0 | +0,65 | 20,4 | 12,0 | 0,20 |
| 2 | 6,61e-6 / 7,55e-5 | 0 | -0,416 [-0,472; -0,359] | +0,017 [-0,038; +0,072] | -352,9 | +114,6 | +0,60 | 20,0 | 12,0 | 0,30 |
| 3 | 3,52e-5 / 1,43e-4 | 0 | -0,431 [-0,484; -0,379] | -0,003 [-0,058; +0,053] | -367,7 | +109,6 | +0,75 | 20,9 | 12,4 | 0,10 |

Đếm thêm các thao tác mỗi thế giới: seed 1 mua Fiber 2,4 lần, nạp đầu vào 0,35 lần, cấp quyền 3,2 lần, gửi ca 3,0 lần và không rút sản phẩm; seed 2 lần lượt 2,05 / 0,30 / 3,35 / 3,0 / 0; seed 3 là 2,10 / 0,35 / 3,40 / 3,10 / 0,05. Đây là số lần chọn action, không phải số đơn vị hàng. Policy tạo khoảng 20 xưởng nhưng rất ít lần nạp Fiber và rút Cloth, phù hợp với tồn kho Cloth cuối kỳ gần 0.

Selector không chọn seed nào. Cả ba policy đều kém baseline; CI so initial còn cắt 0; tiền thấp, nợ/insolvency cao, và output xưởng vẫn gần 0. Mini-batch nhỏ tạo KL cao hơn ở một số update nhưng không làm clip fraction khác 0 và không tạo policy đủ chuẩn. Các holdout v19 và v20 là hai tập thế giới rời nhau nên chỉ dùng làm hai lần chẩn đoán; không diễn giải chênh lệch mean của chúng như một phép so sánh paired trực tiếp.

Không chạy R1/peer promotion. R1 vẫn cần ứng viên qua selection, đủ training set, full horizon và tối thiểu 100 thế giới độc lập. Kế tiếp là xem lại credit/advantage signal và quan sát quyết định kinh tế theo tình huống; không tiếp tục tăng cấu hình mini-batch hoặc giờ huấn luyện theo phỏng đoán.
