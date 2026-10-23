# Báo cáo: phân loại đặc sản của toàn bộ 24 lãnh địa

## Kết quả

Catalog C++ hiện có một bản ghi phân loại cho từng lãnh địa L01–L24. Trong 24 dòng, 20 đặc sản có nhóm hàng đã xác định; 12 dòng trong số đó có thêm mã sản phẩm cụ thể. “Tàu đi biển” là tài sản vận tải chứ không phải hàng hóa. Ba tên “thủy tinh”, “đồ sứ” và “da thuộc” chưa được xếp vào nhóm/mã sản phẩm do thiếu căn cứ phù hợp trong catalog hiện hành.

| ID và vùng | Đặc sản | Phân loại hiện ghi | Nhóm hàng | Mã sản phẩm |
|---|---|---|---|---|
| L01 — Thanh Điền | Gạo nước | Hàng | DryFood (lương thực khô) | Chưa xác định |
| L02 — Liên Trạch | Cá khô | Hàng | DryFood (lương thực khô) | Chưa xác định |
| L03 — Phù Sa | Dầu hạt | Hàng | SupplementaryFood (thực phẩm bổ sung) | Chưa xác định |
| L04 — Đông Viên | Vải gai | Hàng | Cloth (vải) | Chưa xác định |
| L05 — Bạch Khâu | Len trắng | Hàng | Fiber (xơ sợi) | Chưa có GoodId riêng cho len |
| L06 — Hắc Sơn | Sắt rèn | Hàng | Iron (sắt) | Iron |
| L07 — Tùng Lĩnh | Gỗ đóng tàu | Hàng | Timber (gỗ) | Timber |
| L08 — Xích Nham | Đồng thỏi | Hàng | NonferrousMetal (kim loại màu/quý) | Copper |
| L09 — Nam Cảng | Muối biển | Hàng | Salt (muối) | Salt |
| L10 — Hải Châu | Tàu đi biển | Tài sản vận tải | Không phải nhóm hàng | Không áp dụng |
| L11 — Bạch Sa | Thủy tinh | Chưa xác định | Chưa xác định | Chưa xác định |
| L12 — Trầm Hải | Trầm hương | Hàng | Luxury (hàng xa xỉ) | Incense |
| L13 — Vân Lụa | Lụa nhuộm | Hàng | Cloth (vải) | Silk |
| L14 — Cẩm Giang | Giấy tốt | Hàng | Paper (giấy) | Paper |
| L15 — Hồng Diệp | Trà | Hàng | Luxury (hàng xa xỉ) | Tea |
| L16 — Bích Lò | Đồ sứ | Chưa xác định | Chưa xác định | Chưa xác định |
| L17 — Bắc Mã | Ngựa chiến | Hàng | Livestock (gia súc/ngựa) | WarHorse |
| L18 — Phong Ải | Da thuộc | Chưa xác định | Chưa xác định | Chưa xác định |
| L19 — Hoàng Nguyên | Lúa mạch | Hàng | DryFood (lương thực khô) | Chưa xác định |
| L20 — Thiết Quan | Binh khí | Hàng | MilitarySupply (quân nhu/vũ khí) | Weapon |
| L21 — Dược Cốc | Dược liệu | Hàng | Medicine (dược liệu) | Chưa xác định |
| L22 — Lâm Mật | Sáp ong | Hàng | Fiber (xơ sợi) | Beeswax |
| L23 — Thạch Kiều | Đá xây | Hàng | ConstructionMaterial (vật liệu xây dựng) | Chưa xác định |
| L24 — Ngân Khê | Bạc | Hàng | NonferrousMetal (kim loại màu/quý) | Silver |

Mã sản phẩm cụ thể được ghi khi tên đặc sản khớp một biến thể có sẵn: Iron, Timber, Copper, Salt, Incense, Silk, Paper, Tea, WarHorse, Weapon, Beeswax và Silver. Các nhóm DryFood, SupplementaryFood, Cloth, Fiber, Medicine và ConstructionMaterial có thể đã xác định dù mã sản phẩm cụ thể chưa có. Nhờ vậy, gạo, cá khô và lúa mạch cùng nhóm lương thực khô nhưng chưa bị khẳng định là cùng một biến thể có thể thay thế nhau.

## Phạm vi và cách dùng

Mỗi mục lưu ID lãnh địa, loại phân loại, nhóm hàng nếu biết và product ID nếu đúng biến thể. Lookup theo ID vùng trả về bản ghi của L01–L24; ba khu triều đình không có đặc sản nên lookup trả về rỗng. Validator kiểm tra đủ 24 lãnh địa đúng một lần, ID trong miền, loại phân loại hợp lệ, trường nhóm/sản phẩm đúng cấu trúc và nhóm của product ID khớp `good_catalog.hpp`.

Phân loại này chỉ cho phép code khác tham chiếu tên đặc sản theo cùng hệ thống nhóm hàng. Nó không tạo số hàng trong kho, năng lực sản xuất, giá, quyền độc quyền, hàng giao dịch hay vận đơn. Nó cũng không thay đổi 27 bản ghi atlas hoặc 31 liên kết vùng.

## Những mục giữ chưa xác định

- **Tàu đi biển:** §15.2 nói tàu là tài sản vận tải tạo bằng dự án, không phải hàng hóa tiêu dùng. Bản ghi vì vậy dùng loại `TransportAsset`, không giả làm sản phẩm thứ 28.
- **Thủy tinh:** atlas không xác nhận “thủy tinh trang trí”; §15.2 chỉ xếp biến thể có tính chất đó vào Luxury. Chưa gán nhóm hay `DecorativeGlass`.
- **Đồ sứ:** atlas không ghi “đồ sứ tốt”; chưa gán nhóm Luxury hay `FinePorcelain`.
- **Da thuộc:** 16 nhóm và 27 sản phẩm hiện không có loại da thuộc tương ứng đủ rõ; không ép vào nhóm gia súc, xơ sợi hay hàng xa xỉ.
- Với các nhóm hàng đã xác định nhưng không có product variant riêng, catalog chưa tuyên bố tính thay thế giữa các sản phẩm trong cùng nhóm.

## Kiểm tra đã chạy

Build Windows Release riêng target `polyec_world_atlas_tests` rồi chạy trực tiếp `build/windows/core/Release/polyec_world_atlas_tests.exe`: exit code 0. Test đối chiếu toàn bộ 24 ID và tên đặc sản, phân bố 20 hàng/1 tài sản/3 chưa xác định, đúng 12 product ID, nhóm của từng mã sản phẩm và lookup khu triều đình; test còn từ chối bản ghi lỗi gồm ID lặp, ID triều đình, product ID ngoài catalog và nhóm hàng không khớp product. Runner cũng tiếp tục xác nhận catalog 27 vùng, 31 liên kết tuyến và số liệu nền atlas. Không chạy toàn bộ CTest, Debug, Python, native hay Godot vì không đổi consumer hoặc luật gameplay.

## Tài liệu nguồn

- Danh sách và đặc sản L01–L24: `docs/IMPLEMENTATION_PLAN.md` §12.2.
- 16 nhóm, các biến thể và quy tắc tàu không phải hàng hóa: `docs/IMPLEMENTATION_PLAN.md` §15.2.
- ID/nhóm sản phẩm hiện hành: `core/include/polyec/good_catalog.hpp`.
- Phạm vi đã rà trước khi triển khai: [kế hoạch phân loại](../design/M5-atlas-complete-specialty-classification-plan.md).
- Mã băm và kích thước source, test, report, executable và tracker: [manifest](../../artifacts/m5-atlas-complete-specialty-classification-2026-10-22-manifest.json).
