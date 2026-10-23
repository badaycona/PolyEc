# M5 — đánh giá cross-play trên Realm đầy đủ

## Mục tiêu

Thêm evaluator sampled chạy ba policy family trong cùng một `Realm` C++ qua Python binding. Mỗi người đến lượt theo `due(family)`, đọc observation/mask hiện hành, lấy mẫu action categorical và gọi `Realm.execute` qua `Realm.step`. Evaluator ghi utility của actor đại diện mỗi family, sinh tồn, invariant, phân bố action, seed, schema và cấu hình.

Chạy paired các nhóm trên cùng seed: baseline, hold, random legal, initial policy, cả ba họ dùng learned checkpoint, và từng họ mục tiêu dùng learned policy trong khi hai họ còn lại là baseline được gắn nhãn. Nguồn ngẫu nhiên tách ổn định theo seed/actor để so sánh hành động categorical cùng actor. In paired difference và CI95 theo từng actor đại diện; không gọi kết quả này là R1 nếu chưa đủ holdout và criteria đã duyệt.

## Files và ranh giới

- Thêm `ai/evaluation/realm_coplay_v11.py`; không sửa luật hoặc action selection trong C++. Mỗi action đọc lại observation/mask ngay trước khi gọi command để action của peer vừa chạy không làm snapshot kế tiếp bị cũ.
- Thêm script checks nhỏ cho shape, categorical mask, seed determinism, role coverage và worksite candidate availability khi timeline chạy.
- Cập nhật CURRENT_TASK/HISTORY cho plumbing smoke nhưng ghi rõ chưa có checkpoint được huấn luyện; chỉ gọi learned evaluation sau khi có model v11 thực. Không ghi evaluation result giả bằng random weights.

## Contract / hành vi

- Chỉ nhận checkpoint `state_dict` v11 được huấn luyện, cùng metadata family, schema `realm-v11`, model contract, observation/action shape và config fingerprint. Categorical action lấy từ đúng mask runtime.
- Baseline giữ nhãn dev-only, áp theo family; không dùng làm bằng chứng learned policy.
- Mỗi seed dựng một Realm đầy đủ, chốt draft, chạy actor theo quyết định và tiến ngày C++ thật. So sánh paired trên cùng seed, số ngày và cùng nguồn random theo actor.
- Báo riêng `utility` đã chuẩn hóa, coin balance và biến động tiền, survival, action counts, worksite/action cấp quyền/action gửi Work, tồn kho sản phẩm, work đã trả lương, invariant và thời gian. Không trộn utility với tiền.
- Reject seed trùng, config mismatch, checkpoint cũ, action mask rỗng/sai hoặc transition lỗi. Fail-fast khi checkpoint v11 chưa tồn tại; không tự tạo/training checkpoint.

## Kiểm chứng và giới hạn

- Unit smoke dùng trọng số khởi tạo chỉ xác nhận plumbing; output không được gắn nhãn trained/learned.
- Chạy evaluator tối thiểu một seed/day cho cả ba mode, xác nhận mask legality, determinism, exact schema metadata và invariant. Sau khi có trained checkpoint mới chạy paired holdout nhiều seed và tính CI.
- Self-review chỉ. Chưa triển khai trainer v11, resume optimizer/RNG, performance budget, PPO R1 hoặc Godot live UI.
