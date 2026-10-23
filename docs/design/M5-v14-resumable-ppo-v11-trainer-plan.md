# M5 v14 — trainer PPO v11 có thể tiếp tục chính xác

## Mục tiêu

Tạo trainer recurrent PPO cho contract `realm-v11`, dùng categorical sampling và C++ RealmCoPlayBatch. Có thể nạp policy peer đã qua đánh giá; peer thiếu checkpoint chỉ dùng baseline dev có nhãn. Khi chạm giới hạn thời gian, trainer lưu checkpoint ở ranh giới update và khôi phục chính xác lần cập nhật kế tiếp.

## Phạm vi

- Thêm `ai/training/realm_train_v11.py`; không sửa luật kinh tế Python.
- Kiểm contract v11 và ONNX dependency trước rollout.
- Nạp peer bundle theo family/schema/shape/model contract/config fingerprint; chỉ chấp nhận bundle đã được đánh dấu đủ điều kiện làm peer sau evaluator.
- Checkpoint gồm policy, Adam optimizer, Torch/NumPy/Python RNG, trạng thái C++ Realm của từng env, focal hidden state, peer hidden state và RNG.
- Ghi rollout/optimizer time, quyết định/giây, lượt peer, KL, clip fraction, critic explained variance, reward và RSS.
- Kiểm tra một lượt chạy liền và lượt bị dừng sau update rồi resume: toàn bộ checkpoint cuối phải bitwise-identical.

## Ngoài phạm vi lát này

Không chọn model, không export ONNX, không cấp nhãn peer-eligible, không đánh giá R1/R2/R3, không chạy budget training dài. Các quyết định nhỏ trong smoke chỉ xác nhận đường code và resume.

## Điều kiện nghiệm thu

1. Python syntax/preflight hợp lệ; mismatched realm contract, peer metadata hoặc resume identity bị từ chối.
2. Một update PPO dùng candidate mask mới và peer frozen qua C++ Realm; mọi world invariant giữ đúng.
3. Resume sau update tái tạo cùng model, optimizer, world, hidden memory và RNG như chạy liền.
4. Throughput được báo cùng role, vùng, số env, rollout length và peer decisions; không ngoại suy sang profile B.

Self-review only trên shared dirty checkout; giữ nguyên mọi dirty work sẵn có.
