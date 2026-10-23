# M5 v26 — kiểm tra utility của lệnh mua hàng

## Mục tiêu

PPO v23 log reward trung bình dương cho một số quyết định Buy. Vì Realm định giá hàng trong kho cá nhân bằng giá tham chiếu, kiểm tra liệu mua hàng ở giá địa phương có tự tạo utility ngay, hoặc lệnh mua có thể dùng quỹ hộ gia đình mà utility cá nhân không trừ phần đó.

## Kết quả

- Trên 20 seed deterministic, tìm và chạy 140 candidate Buy hợp lệ (7 loại hàng ×20). Từng lệnh chạy một lần trong Realm C++ rồi đo utility, tiền actor và quỹ hộ ngay sau command, không advance ngày.
- Cả 140 lệnh đều giảm utility; không có mẫu nào dùng quỹ hộ. Chênh utility khớp chính xác với giá trị inventory theo bảng giá tham chiếu trừ actor cash đã trả; sai số lớn nhất khoảng `1,3e-15` điểm.
- Giá trị tham chiếu của lượng mua là 80–360, chi phí quan sát là 92,8–423 mỗi lệnh, vì vậy không có lợi nhuận utility tức thời trong các snapshot này.
- Để so với horizon train, chạy thêm 321 cặp lệnh Buy–Hold trong bốn reset context v23, dùng đúng C++ `RealmCoPlayBatch`, dev peers và base seed của 16 môi trường ×3 training seeds. Mỗi cặp advance 7 ngày. Trong reset snapshots đã kiểm, Buy cho reward thấp hơn Hold ở cả bốn context; không có cặp Buy nào cao hơn Hold.

## Kết luận và phần chưa giải thích

Không có bằng chứng rằng Buy tự tạo tài sản miễn phí từ giá tham chiếu hoặc rút quỹ hộ trong các trạng thái kiểm. Kết quả paired ở reset snapshots cũng thấp hơn Hold, trong khi PPO history gộp reward dương cho Buy trên toàn rollout 25 update. Vì vậy số dương trong history là tương quan của hành động với những trạng thái về sau, không chứng minh Buy gây tăng reward; cần audit cặp Buy–Hold trên checkpoint states đang phát triển trong rollout để truy nguyên phần lệch này. Chưa đổi utility, reward, giá hay luật mua; không train/promotion. Invariant pass mọi lệnh.

Codex tự review, chưa có reviewer độc lập. Python compile pass; evaluator adapter 28/28 pass.

- Kế hoạch: [M5 v26 valuation audit](../design/M5-v26-economic-inventory-valuation-audit-plan.md)
- Dữ liệu: [audit.json](../../build/m5-v26-economic-inventory-valuation-training-seeds/audit.json)
- Manifest/hash: [m5-v26 manifest](../../artifacts/m5-v26-economic-inventory-valuation-2026-10-21-manifest.json)
