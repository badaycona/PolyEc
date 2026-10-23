# M5 v18 — diagnostics PPO sau optimizer và sàng lọc learning rate

## Thay đổi

Sửa `realm_train_v11.py`: KL, clip fraction và mean ratio giờ được tính lại sau optimizer step trên minibatch vừa cập nhật. Log thêm policy loss, value loss, mean gradient norm và phân vị reward, advantage thô, advantage đã chuẩn hóa, return. Không thay công thức huấn luyện, action sampling hay reward.

## Kết quả phép đo

Run seed 1 với LR gốc 0,0003 được chạy lại đủ 25 updates / 25.600 quyết định. SHA policy là `221907b95978b84d1fd1a29ef28d82cf8a32821b644e93d5b1a2ba2a293aa876`, trùng tuyệt đối policy seed 1 đã được đánh giá trước đó. Vì vậy quality result của validation không đổi. Sau optimizer step, KL min/median/max là `1,35e-7 / 1,97e-6 / 1,07e-5`; clip fraction luôn 0; critic explained variance min/median/max `-0,939 / 0,027 / 0,553`.

Thí nghiệm một biến tăng LR lên 0,0012, giữ cùng seed, horizon, peer, 25 updates và 25.600 quyết định. Trên 20 world seeds thăm dò ghép cặp, utility so baseline là `-0,554` (CI95 `[-0,621, -0,486]`), utility so initial `+0,026` (CI95 `[-0,052, +0,104]`, chưa rõ cải thiện). Cash cuối kỳ thấp hơn baseline `367,55`; debt cao hơn `119,8`; insolvency cao hơn `0,6` người/world. KL sau optimizer min/median/max `6,62e-6 / 3,25e-5 / 1,50e-4`, clip fraction luôn 0; critic explained variance `-0,728 / 0,020 / 0,566`.

Kết quả LR cao chỉ cải thiện utility so baseline khoảng 0,029 so với run LR gốc, vẫn thua baseline lớn, không cải thiện initial có ý nghĩa và làm debt/insolvency xấu hơn. Không có căn cứ chạy trainset ba seed hoặc mở model làm peer với cấu hình này. Tập 20 world đã xem là thăm dò và không thể dùng cho lựa chọn cuối.

## Kiểm chứng và giới hạn

- `py_compile` pass; smoke economic 2 vùng / 1.024 quyết định ghi đủ các trường mới.
- Run seed 1 LR gốc hoàn tất 25/25 và khớp SHA đã validation.
- Run seed 1 LR 0,0012 hoàn tất 25/25; evaluator sampled 20 world × 128 ngày giữ invariants, `r1_pass=false` do chỉ 1 training seed, 20 holdouts và horizon ngắn.
- Self-review trên dirty checkout; không có reviewer độc lập. Chưa có R1, chưa có learned peer đủ điều kiện, chưa đo profile B. Thí nghiệm chỉ bác bỏ việc tăng LR gấp bốn lần như lời giải đủ dùng; chưa cô lập vấn đề reward/curriculum/critic.

Plan: [v18](../design/M5-v18-ppo-update-diagnostics-and-lr-screen-plan.md). Nguồn code: `ai/training/realm_train_v11.py`.
