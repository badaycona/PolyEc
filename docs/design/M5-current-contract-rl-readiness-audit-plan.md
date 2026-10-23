# M5 — kiểm tra mức sẵn sàng huấn luyện theo contract Realm hiện tại

Ngày 2026-10-23. Đây là audit read-only trước khi sửa trainer hoặc tạo run mới. Mục tiêu là xác định policy/dữ liệu đang có có thể dùng cho Realm save51, household contract v30, timeline contract v32, observation2.935 và220 action hay không.

## Phạm vi đọc

- Điều kiện bắt đầu train, selector và holdout tại `docs/design/M5-integration-and-tuning-gates.md`, các plan `M5-v12`, `M5-v16` và báo cáo RL v30/v31/T5.6.
- Model/trainer/env/checks dưới `ai/training`, `ai/evaluation`, `ai/tests`.
- Version/shape hiện tại trong `core/include/polyec/realm.hpp`, model manifests/runtime models và các run/selection/holdout artifacts đã ghi.
- Không sửa trainer, không khởi tạo train/validation/holdout mới, không export model. Đây là audit nguồn và contract, không phải xác nhận run mới.

## Câu hỏi cần kết luận

1. Trainer có gọi đúng Realm C++ và nhận đầy đủ observation, legal mask, transition và reward không?
2. Tensor/action IDs, model contract, config fingerprint và reward metadata của trainer/bundle có khớp source hiện tại không?
3. Nguồn dữ liệu train, validation, holdout, selector và visualization có thể truy xuất và tách theo seed không?
4. Policy nào đang được coi là learned, policy nào chỉ là dev/test baseline, và có run pass selector/holdout theo contract hiện tại không?
5. Chọn bước code kế tiếp nhỏ nhất; nếu train còn bị điều kiện gameplay chặn thì chỉ chuẩn bị khả năng tương thích, không chạy ngân sách huấn luyện dài.

## Điều kiện đóng

Report phải dẫn đến source và artifact cụ thể, phân biệt điều đã được chạy trước đây với suy luận từ source hiện tại, nêu rõ incompatibility và một next action duy nhất. Không dùng tên `realm-v11` chung làm bằng chứng tương thích khi model tensor/action/model-contract khác. Mọi lần train/eval tương lai cần lưu nguồn dữ liệu, manifest/version/hash, seed split, selector, holdout, chart và dữ liệu gốc của chính run.
