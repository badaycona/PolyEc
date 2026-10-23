# M5 v28 — đối chứng cách định giá hàng trong checkpoint Economic

Ba checkpoint v23, mỗi checkpoint tại update 25, được khôi phục và đọc bằng cùng Python binding/C++ Realm. Audit v27 có 15 candidate Buy hợp lệ trên seed 1 và 3; seed 2 không có candidate trong các snapshot đã lưu.

## Buy theo hai cách ghi giá trị hàng

- Theo giá tham chiếu, 15 mẫu Buy có điểm utility dự kiến dương như v27 đã đo: Fiber +0,44, Fuel +0,60, Iron +1,68, Tools +2,48, Horse +3,48 và DryFood +0,68.
- Theo giá bán tại vùng hiện tại, tính `max(1, local price - 1)` cho mỗi món rồi trừ đúng giá mua, cả 15 mẫu đều thành −0,08 điểm. Giá bán ở các snapshot này thấp hơn chi phí mua một đơn vị 2 tiền, nhân 4 món chia hệ số utility 100.

## Ảnh hưởng khi người có hàng đổi vùng

Trong 52 tổ hợp snapshot/hàng có tồn kho cá nhân thuộc bảy nhóm hàng legacy, 11 có giá trị đánh dấu khác nhau giữa các vùng. Tổng chênh lệch max–min của các mẫu là 585 tiền; mẫu lớn nhất chênh 192. Đây là phép tính định giá trên giá quan sát được, không chạy Travel hoặc Sell. Nó cho thấy cách đánh dấu theo giá địa phương có thể làm utility đổi chỉ vì actor sang vùng khác, trước khi giao dịch bán thực sự xảy ra.

## Kết luận và giới hạn

Định giá thanh lý tại vùng dập tắt lợi ích Buy ở các checkpoint đã phát triển, nhưng đưa giá vùng vào wealth khiến chuyển vùng có thể thay đổi utility của hàng đang mang. Không chọn công thức mới và không đổi reward/gameplay trong lượt này. Muốn giữ một giá trị ổn định mà vẫn ghi nhận lãi/lỗ khi bán cần một contract rõ cho giá vốn của hàng mua, hàng sản xuất và hàng nhận qua chuyển kho; đây là phần thiết kế/triển khai còn mở.

Audit chỉ là counterfactual trên checkpoint update 25, không phải đánh giá policy mới hoặc kết luận tất cả hàng/hoàn cảnh. Codex tự review, không có reviewer độc lập.

- Kế hoạch: [M5 v28](../design/M5-v28-inventory-marking-counterfactual-plan.md)
- Dữ liệu: [audit.json](../../build/m5-v28-inventory-marking-counterfactual/audit.json)
- Manifest: [m5-v28 manifest](../../artifacts/m5-v28-inventory-marking-counterfactual-2026-10-21-manifest.json)
