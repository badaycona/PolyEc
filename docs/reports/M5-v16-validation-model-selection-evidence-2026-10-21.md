# M5 v16 — chọn checkpoint bằng validation

## Thay đổi

`ai/evaluation/realm_select_v11.py` nhận ít nhất ba output train hoàn tất của cùng family, model contract, peer setup và sample budget; sau đó chạy cùng validation seeds trên từng policy. Chỉ chọn nếu paired CI so initial có cận dưới >0 và baseline noninferiority cận dưới ≥ -0,05; ưu tiên mean cải thiện baseline cao nhất. Bundle được copy riêng và luôn ghi `peer_eligible=false` cho đến khi holdout R1 riêng đạt.

`realm_policy_holdout_v11.py` nhận thêm `--selected-bundle`: chỉ mở `peer_eligible` khi các run/seed khớp selection record, tập holdout không giao validation, và đầy đủ ba training seeds/100 holdout cùng mọi tiêu chí R1.

## Kiểm chứng và giới hạn

- Python syntax check pass cho trainer/runset/evaluator/selector.
- Runset chạy ba seed economic smoke tuần tự, mỗi seed 1 update/1.024 decisions; `TRAINING-SET.json` xác nhận 3 seed khác nhau và budget bằng nhau.
- Selector chạy trên cùng hai validation world, mỗi 2 ngày. Không có seed nào đủ 25-update floor hoặc CI cải thiện initial; kết quả `eligible_seeds=[]`, `selected_seed=null`, không tạo bundle.
- Holdout smoke một seed, hai thế giới × hai ngày giữ invariants, utility CI = [0,0], `r1_pass=false`.
- Các run 1 update chỉ kiểm đường dữ liệu và chặn đúng theo gate; không đánh giá quality. R1/R2/R3/R5 và Profile B còn mở.
