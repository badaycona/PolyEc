# M5.2/M5.4 — phân loại đặc sản cho đủ 24 lãnh địa

## Mục tiêu và căn cứ

Catalog atlas hiện chỉ cấu trúc hóa bảy đặc sản. `IMPLEMENTATION_PLAN.md` §12.2 liệt kê 24 đặc sản; §15.2 định nghĩa 16 nhóm hàng, một số biến thể và nói rõ tàu là tài sản vận tải, không phải nhóm hàng. Lát này phân loại toàn bộ 24 tên bằng catalog `GoodGroup`/`GoodId` hiện hành, không cấp hàng hoặc tạo hành vi kinh tế.

## Quy tắc phân loại

- Dùng nhóm hàng khi tên đặc sản khớp trực tiếp một trong 16 nhóm.
- Gắn `GoodId` riêng chỉ khi atlas gọi đúng biến thể hiện có (ví dụ ngựa chiến → `WarHorse`, bạc → `Silver`). Nếu chỉ biết nhóm, để product rỗng.
- Tàu đi biển được phân loại là `TransportAsset` theo §15.2, không gán `GoodGroup`.
- Da thuộc không có nhóm hoặc product tương ứng rõ trong catalog; thủy tinh và đồ sứ không nêu rõ hai biến thể “thủy tinh trang trí”/“đồ sứ tốt” được liệt kê trong §15.2. Ba mục này giữ `Unresolved` thay vì ép vào hàng không khớp.
- Phân loại không tạo nguồn cung, producer, giá, quyền độc quyền hoặc trade action.

## Bảng kết quả dự kiến

| ID | Đặc sản | Phân loại | Nhóm | Product |
|---|---|---|---|---|
| L01 | Gạo nước | Hàng | DryFood | Chưa xác định |
| L02 | Cá khô | Hàng | DryFood | Chưa xác định |
| L03 | Dầu hạt | Hàng | SupplementaryFood | Chưa xác định |
| L04 | Vải gai | Hàng | Cloth | Chưa xác định |
| L05 | Len trắng | Hàng | Fiber | Chưa xác định |
| L06 | Sắt rèn | Hàng | Iron | Iron |
| L07 | Gỗ đóng tàu | Hàng | Timber | Timber |
| L08 | Đồng thỏi | Hàng | NonferrousMetal | Copper |
| L09 | Muối biển | Hàng | Salt | Salt |
| L10 | Tàu đi biển | Tài sản vận tải | — | — |
| L11 | Thủy tinh | Chưa xác định | — | — |
| L12 | Trầm hương | Hàng | Luxury | Incense |
| L13 | Lụa nhuộm | Hàng | Cloth | Silk |
| L14 | Giấy tốt | Hàng | Paper | Paper |
| L15 | Trà | Hàng | Luxury | Tea |
| L16 | Đồ sứ | Chưa xác định | — | — |
| L17 | Ngựa chiến | Hàng | Livestock | WarHorse |
| L18 | Da thuộc | Chưa xác định | — | — |
| L19 | Lúa mạch | Hàng | DryFood | Chưa xác định |
| L20 | Binh khí | Hàng | MilitarySupply | Weapon |
| L21 | Dược liệu | Hàng | Medicine | Chưa xác định |
| L22 | Sáp ong | Hàng | Fiber | Beeswax |
| L23 | Đá xây | Hàng | ConstructionMaterial | Chưa xác định |
| L24 | Bạc | Hàng | NonferrousMetal | Silver |

Tổng: 20 hàng đã có nhóm (12 có GoodId cụ thể, 8 chỉ có nhóm), 1 tài sản vận tải, 3 chưa phân loại. Nhóm hàng “thủy tinh trang trí/đồ sứ tốt” chưa gán cho hai tên atlas thiếu tính chất đó.

## File và kiểm tra

- `core/include/polyec/world_atlas.hpp`: thay bảng bảy mục bằng 24 bản ghi phân loại; lookup theo lãnh địa; validator kiểm ID duy nhất/đủ miền, kiểu và trường nhóm/product tương thích catalog.
- `core/tests/test_world_atlas.cpp`: đối chiếu đủ 24 tên theo bảng, 20 nhóm hàng, 12 mã sản phẩm, một tàu, ba mục chưa xác định, lookup vùng triều đình và các ca dữ liệu sai.
- Build và chạy riêng `polyec_world_atlas_tests` Windows Release. Không chạy adapter/Godot/native vì đây là dữ liệu C++ tĩnh, không thay command, observation hoặc save.

## Review

Đã rà đối chiếu tên, nhóm và biến thể với §12.2, §15.2 và `good_catalog.hpp`. Các trường hợp không khớp chắc chắn giữ chưa xác định. Đây là ánh xạ taxonomy, không thêm luật nguồn cung/thị trường. Tự review; chưa có reviewer độc lập.
