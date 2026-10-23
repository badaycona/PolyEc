# M5 — evaluator co-play trên Realm đầy đủ

Ngày 2026-10-21. Thêm đường đánh giá policy nhiều họ trong một world chạy bằng cùng C++ core, xử lý điểm review rằng RealmBatch trước đây dùng peer civilian cố định và governance/economic peer uniform.

## Hành vi kiểm chứng

Evaluator chạy từng actor theo lịch `due(family)`, đọc observation và legal mask mới ngay trước quyết định, lấy mẫu categorical bằng mask đó, rồi gửi action qua `Realm.step` để C++ Realm xác thực lại. Mỗi actor giữ recurrent memory riêng; memory reset khi family của actor đổi. Cùng một seed chạy paired các controller: baseline dev, hold, random legal, initial policy, ba policy learned cùng chạy, và từng policy family learned với hai nhóm peer baseline.

Report lưu utility chuẩn hóa riêng từng actor đại diện, tổng tiền/biến động tiền, sống sót, action count, action worksite/cấp quyền/gửi Work, tồn kho xưởng, Work đã trả lương, invariant, thời gian mỗi lần chạy, chênh paired theo family và CI95. Utility không bị mô tả thành coin. Loader yêu cầu manifest v11 đúng family, model contract, tensor shape, config fingerprint, trạng thái RL và selected seed; evaluator checkpoint chỉ nhận khi có ít nhất hai seed holdout.

## Đã kiểm tra

- `realm_coplay_v11_checks.py`: **16** checks cho schema/shape, action mask, baseline legality, categorical mask, kết quả lấy mẫu lặp ổn định và observation/mask còn dùng được sau Work timeline.
- Full evaluator plumbing chạy 2 seed × 2 ngày × 8 controller mode trên Realm 128 người; mọi run invariant đúng. Chạy lại cùng tham số cho kết quả giống nhau.
- Lượt này dùng trọng số khởi tạo ngẫu nhiên cho plumbing và chỉ hai ngày. Số utility hầu hết bằng 0 vì thời gian quá ngắn để tạo tác động; các số này không dùng so sánh chất lượng. Không có checkpoint v11 được huấn luyện, nên chưa chạy checkpoint loader, sampled holdout dài hay R1.

## Còn thiếu

Self-review only. Đây là evaluator co-play, chưa thay trainer `RealmBatch`; hiện chưa có cách train focal actor với checkpoint learned peers đóng băng trong rollout. Chưa có trainer/export v11, optimizer/RNG resume, holdout chuyển giao, Godot live-play hay R1. Không dùng random weights làm policy learned và không chạy training dài.
