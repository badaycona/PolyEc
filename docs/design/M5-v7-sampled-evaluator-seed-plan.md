# M5 — sửa coverage seed của Realm v7 sampled evaluator

## Vấn đề

`ai/evaluation/realm_sampled_v7.py` công bố `test_seeds=[170000,170099]` nhưng `evaluate()` luôn tạo `RealmBatch` bằng seed 170000 và chỉ chạy 100 episode. Bằng chứng hiện tại vì vậy chưa bao phủ seed thứ hai như report hứa.

## Thay đổi

- Thêm seed rõ ràng vào `evaluate()`; chạy learned, initial, hold, random và baseline trên từng test seed đã khai báo.
- Giữ common environment seed và cùng uniform stream giữa các policy/controller để so sánh paired.
- Report kết quả từng seed và pooled 200 episode cho metrics/action counts; pooled CI dùng N thực, không hard-code 100.
- Không thay model selection, training artifacts, core transition, reward hoặc ngưỡng. Giữ `full_R1_pass=false` cho đến khi toàn bộ gate R1 được nghiệm thu.

## Gate

1. Static syntax check.
2. Chạy evaluator trên ba checkpoint v7 pilot thật sau training; kiểm JSON ghi đúng cả hai seed cho cả năm mode, 200 utility mỗi mode, paired metrics, không action invalid.
3. So sánh mỗi paired baseline theo cùng seed và lưu raw outcome để review.

## Review

Plan được self-reviewed theo quyền thực thi hiện hành. Phạm vi chỉ là sửa lỗi evaluator đã quan sát; chưa chứng nhận policy.
