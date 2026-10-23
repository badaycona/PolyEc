# M5.2 — kiểm hàng không thay thế qua Realm và ngày mô phỏng

## Mục tiêu

Nối năm trường hợp đã kiểm bằng `ProductionManager` thuần qua Realm: hàng chuẩn được nhập vào kho xưởng bằng fixture save bảo toàn sổ, ca làm được nhận, ngày game chạy pha sản xuất, rồi save/load giữ đúng kho và kết quả. Hành vi cần giữ theo Gate 4 là recipe chỉ nhận chính xác `GoodId`, không nhận sản phẩm cùng nhóm.

## Phạm vi

- Chỉ sửa fixture/test `core/tests/test_realm.cpp` và tài liệu/bằng chứng.
- Năm trường hợp: Beeswax/Fiber; Copper/Iron; Silver/Iron; Gold/Iron; Silk/Cloth. Các recipe còn lại có đủ đầu vào phụ trợ (Fuel hoặc Tools) để cô lập đúng hàng bị thay thế.
- Mỗi Realm tạo cơ sở loại phù hợp bằng command hiện có, cấp quyền làm và nhận Work interval 32 tick. Stock thử được thêm vào fixture save cùng opening goods ledger để invariant giữ nguyên.
- Sau một ngày mô phỏng: không có batch/output; hàng thay thế và đầu vào phụ trợ vẫn ở trong xưởng; hai work unit được ghi nhận; tiền tổng, invariant và save/load không đổi ngoài các hiệu ứng ngày đã có.
- Không thêm recipe, nguồn hàng, market transfer, action/observation, save schema hoặc luật cân bằng.

## Kiểm tra ngay

Windows Release build riêng `polyec_realm_tests` và chạy executable trực tiếp. Đây là test-only bổ sung; không chạy full CTest, Debug hoặc adapter vì production/API/save schema không đổi. C++ Realm command→work→day boundary→save/load được kiểm ngay trong fixture. Python/native/Godot nằm ở mốc tích hợp M5.2 khi production behavior hoặc bề mặt gọi được mở rộng.

## Kết quả

Windows Visual Studio Release build `polyec_realm_tests` thành công; executable đạt **20.799 checks**. Năm fixture tạo xưởng đúng loại qua command, authorize worker, nhận ca 32 tick, stage hàng thay thế và hàng phụ trợ trong serialized fixture cùng opening ledger, rồi chạy Realm 64 tick. Cả năm không tạo batch, không tiêu stock thay thế/phụ trợ, ghi nhận hai unit đã xét, giữ money/goods invariant và khớp fingerprint/kho/watermark sau save/load. Runner báo profile A 10.000 người/19.743 quan hệ thưa/90 ngày; không phải profile B. Chỉ test và tài liệu đổi; chưa chạy full CTest, Debug hoặc adapters.

## Giới hạn

Phép kiểm chỉ bao phủ ba recipe hiện có và các biến thể thay thế nêu trên; không chứng minh source/producer cho hàng deferred, 16 nhóm hàng có cung ứng, hay toàn bộ M5.2. Tự review; chưa có reviewer độc lập.
