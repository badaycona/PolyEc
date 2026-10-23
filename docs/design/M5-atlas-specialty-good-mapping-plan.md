# M5.2/M5.4 — ánh xạ đặc sản atlas vào catalog hàng

## Căn cứ và mục tiêu

IMPLEMENTATION_PLAN §15.2 quy định các ánh xạ đặc sản: len vào nhóm xơ sợi; lụa vào vải cao cấp; trà, trầm, thủy tinh trang trí và đồ sứ tốt vào hàng xa xỉ; sáp ong là biến thể nguyên liệu có công dụng xác định. Catalog hiện có 27 `GoodId` thuộc 16 `GoodGroup`, nhưng atlas mới chỉ giữ tên đặc sản dạng chữ.

Thêm dữ liệu liên kết giữa ID lãnh địa và nhóm/sản phẩm đã được nêu rõ để các bước sản xuất/thị trường sau có thể dùng chung định danh hàng. Len chỉ có ánh xạ nhóm `Fiber` vì catalog chưa có một `GoodId` riêng cho len. Các ánh xạ nhóm còn lại lấy từ nhóm của `GoodId` tương ứng trong catalog hiện hành.

## Ánh xạ được phép

| ID vùng | Đặc sản | Nhóm | Sản phẩm cụ thể |
|---|---|---|---|
| L05 | Len trắng | Fiber | Chưa có ID sản phẩm len |
| L11 | Thủy tinh | Luxury | Chưa chỉ định |
| L12 | Trầm hương | Luxury | Incense |
| L13 | Lụa nhuộm | Cloth | Silk |
| L15 | Trà | Luxury | Tea |
| L16 | Đồ sứ | Luxury | Chưa chỉ định |
| L22 | Sáp ong | Fiber | Beeswax |

Atlas chỉ gọi hai mặt hàng là “thủy tinh” và “đồ sứ”; nó không xác nhận chúng là biến thể “trang trí” hoặc “tốt” trong tên sản phẩm `DecorativeGlass`/`FinePorcelain`, nên chỉ ghi nhóm hàng xa xỉ mà không gán hai mã này. Không ánh xạ các đặc sản còn lại trong bảng atlas theo suy luận tên gọi. Không biến mapping thành nguồn nguyên liệu, quyền độc quyền, ngành sản xuất, giá đặc biệt hoặc hành động gameplay.

## Phạm vi file và phép kiểm

- `core/include/polyec/world_atlas.hpp`: thêm bảng ánh xạ tĩnh, lookup theo ID vùng và validator kiểm ID/nhóm/sản phẩm trỏ vào `good_catalog.hpp`.
- `core/tests/test_world_atlas.cpp`: kiểm đủ bảy dòng ở trên gắn đúng tên lãnh địa/đặc sản, trường hợp len không có ID sản phẩm, lookup vùng không có mapping, nhóm của product khớp catalog, và từ chối vùng lặp/vùng triều đình/mã sản phẩm sai/nhóm không khớp.
- Build và chạy riêng `polyec_world_atlas_tests` Windows Release. Không chạy Python/Godot/native/full CTest vì đây chỉ là catalog C++ không đổi binding hoặc luật giao dịch.

## Review

Đối chiếu từng dòng với §15.2 và `good_catalog.hpp` trước khi sửa source; không suy rộng ra các đặc sản khác. Tự review; không có reviewer độc lập.
