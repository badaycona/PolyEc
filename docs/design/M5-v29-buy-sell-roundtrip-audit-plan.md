# M5 v29 — kiểm giao dịch Buy rồi Sell trong cùng vùng

Ngày 2026-10-22. Đây là bước tiếp theo của phân tích v27/v28: xác định khoản tăng utility lúc mua có tồn tại sau khi hàng được bán thật hay chỉ là giá trị tồn kho chưa thu được bằng tiền.

## Câu hỏi và phương pháp

- Khôi phục chính xác ba checkpoint Economic v23 ở update 25, dùng C++ Realm qua binding Python.
- Chọn một candidate Buy hợp lệ cho mỗi bối cảnh/loại hàng theo cùng thứ tự ổn định với audit v27/v28; không lấy thêm mẫu ngẫu nhiên. Bộ dữ liệu phải khớp 15 mẫu đã lưu từ seed 1 và seed 3.
- Trên từng Realm snapshot, chụp fingerprint, tiền cá nhân, quỹ hộ, tồn kho hàng, utility, giá và Province firm. Gọi command Buy thật cho đúng 4 món; ngay sau đó gọi command Sell thật cho đúng 4 món ở cùng vùng, trước khi tiến game time hoặc cho agent nào khác ra quyết định.
- Đối chiếu lượng hàng cuối bằng đầu, tiền cá nhân mất đúng chênh lệch giữa giá mua và giá bán, utility sau vòng giao dịch, Province firm, tổng tiền, invariant và fingerprint đầu vào trước nhánh.
- Ghi SHA-256 checkpoint/policy và fingerprint từng Realm để người đọc tái xác định dữ liệu. Đây là diagnostic transaction counterfactual, không phải đánh giá policy hay huấn luyện.

## Phạm vi file

- Thêm `ai/evaluation/realm_buy_sell_roundtrip_audit_v11.py`.
- Thêm plan, report, CURRENT_TASK/HISTORY và manifest hash.
- Không sửa C++ gameplay/reward, binding/API, checkpoint/model, dữ liệu huấn luyện hoặc file manifest model.

## Kiểm tra

1. Python compile cho script mới.
2. Chạy script trên đúng ba `build/m5-v23-economic-curriculum-r1-runset/economic/seed-{1,2,3}`; phải tạo đúng 15 mẫu, từng mẫu Buy và Sell đều được Realm chấp nhận, không lệch tiền/hàng/invariant.
3. `git diff --check` cho file mới và chạy `scripts/check-repository-links.py`.

## Giới hạn

Giao dịch bán diễn ra ngay trong cùng ngày và vùng; không đo thay đổi giá, hành vi đối tác, chi phí thời gian hoặc reward qua horizon 7 ngày. Vòng Buy–Sell xác nhận được số tiền thực thu và spread theo luật hiện có, không chọn cách ghi nhận giá vốn của hàng còn tồn, không giải quyết giá vốn của hàng được sản xuất/nhận chuyển, và không cấp căn cứ để sửa utility. Không train hoặc promotion model.
