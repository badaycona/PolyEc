# M5 v26 — audit utility khi mua hàng

## Câu hỏi

PPO v23 diagnostic history ghi Buy với reward trung bình cao hơn đáng kể WorkAtSite. Realm utility cộng giá trị hàng cá nhân theo giá tham chiếu cố định, trong khi Buy trả giá địa phương cộng phí một đồng mỗi món. Xác định trên lệnh C++ thật liệu mua hàng tạo utility ngay, và liệu quỹ hộ gia đình có trả một phần chi phí nhưng không xuất hiện trong utility Economic.

## Phạm vi

- Audit-only runner cho Buy candidate hợp lệ trong snapshot Realm v11 và paired one-decision `RealmBatch`.
- Phép đo tức thời tạo world mới từ cùng seed/draft, chạy đúng một Buy candidate qua `world.step`, rồi đo utility, actor cash, debt và household fund trước/sau.
- Với mỗi Buy legal trong bốn reset context v23, so transition train 7 ngày với Hold bằng hai `RealmCoPlayBatch` mới cùng seed/context/dev peer policy và RNG khởi tạo.
- Ghi GoodId, quantity, giá tham/reference, tiền actor/quỹ hộ đã chi, observed utility delta, sai số so với tài sản được ghi nhận.
- Không đổi reward, giá cả, luật mua, policy hoặc huấn luyện. Không promotion.

## Kiểm chứng

- Python compile và evaluator checks.
- Chạy ít nhất 20 seed snapshot, lấy mọi GoodId có candidate Buy hợp lệ; invariant phải đúng sau mỗi lệnh.
- So `utility_delta ×100` với chênh inventory-reference value trừ actor cash chi. Chênh còn lại khớp household fund đã chi thì báo rõ như một khoảng trống trong mục tiêu utility, chưa tự sửa.
- Báo paired reward sau 7 ngày riêng với utility tức thời; đây là chẩn đoán một bước, không phải đánh giá policy.
