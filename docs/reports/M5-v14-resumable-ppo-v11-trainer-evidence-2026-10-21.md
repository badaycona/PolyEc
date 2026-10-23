# M5 v14 — trainer PPO v11 và resume

## Thay đổi

`ai/training/realm_train_v11.py` thêm masked categorical recurrent PPO cho `realm-v11`, dùng cùng C++ `RealmCoPlayBatch`. Peer checkpoint phải khớp family, schema, kích thước model, model contract và config fingerprint; hiện loader chỉ nhận bundle được evaluator đánh dấu `peer_eligible`. Peer thiếu checkpoint được ghi rõ là dev baseline. ONNX dependency được kiểm tra trước rollout.

Mỗi update trainer ghi `checkpoint.pt` nguyên tử, gồm policy, Adam optimizer, tiến độ update, trạng thái từng Realm đã serialize, focal/peer recurrent memory và RNG của Torch, NumPy, Python, peer actors. History ghi thông lượng, thời gian simulation/update, peer decisions, KL, clip fraction, critic explained variance, reward và RSS. Timeout giữa lịch cập nhật đánh dấu run chưa hoàn tất để gọi lại với `--resume`.

## Kiểm chứng chạy

- `py -3.12 -m py_compile` trên trainer, adapter và test: pass; ONNX import 1.23.2.
- Hai lượt binding adapter tiếp tục pass 14/14 ở Release và Debug.
- Smoke economic, 2 vùng, 16 env, 64 rollout bước/update, 2 update: 2.048 quyết định focal; mỗi update tạo khoảng 33.250 peer decisions. Release throughput lần chạy cuối 167,26 và 172,51 quyết định/giây; rollout 5,80/5,62 giây; optimizer 0,32/0,30 giây; RSS đỉnh khoảng 397/402 MB.
- Run thứ hai cùng seed bị cắt theo giới hạn 1 giây sau update 1; checkpoint ghi `INCOMPLETE`. Tiếp tục từ checkpoint hoàn tất update 2 với tốc độ 167,07 quyết định/giây. So sánh với run liền mạch: policy, optimizer, serialized Realm, focal/peer memory, RNG và identity đều giống hệt. Smoke chưa cấp peer checkpoint, nên trường fingerprint peer bundle trong identity để trống.
- Scoped `git diff --check` trên source, plan, evidence và CURRENT_TASK: pass; chỉ còn warning đổi line ending trong file binding đã có dirty work sẵn.

## Giới hạn

Đây là smoke của trainer với 2 update/2.048 decisions, không phải ngân sách training hay bằng chứng năng lực policy. Không có lựa chọn model, export ONNX, evaluation holdout/R1, cross-family learned peer hoặc benchmark profile B. Không được dùng checkpoint smoke như peer đủ điều kiện. Tác vụ M5 và core refactor tiếp tục mở.
