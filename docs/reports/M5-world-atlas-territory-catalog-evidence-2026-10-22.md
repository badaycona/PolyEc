# M5.4 — danh mục tham chiếu cho 24 lãnh địa và 3 khu triều đình

Ngày 2026-10-22. Đưa bảng atlas thiết kế trong `IMPLEMENTATION_PLAN.md` §12.2 vào một catalog tĩnh C++ để các bước xây quốc gia có thể tham chiếu và kiểm tra tính đầy đủ. Catalog này chưa tạo quốc gia đang chạy trong game.

## Dữ liệu được thêm

Mỗi bản ghi chứa ID nội bộ, tên, cụm địa lý, dân số bối cảnh theo nghìn người, diện tích theo km², đặc sản và phần mô tả lợi thế cùng nút thắt; bản ghi cũng cho biết đây có phải đất triều đình hay không. 24 lãnh địa được xếp thành sáu cụm, mỗi cụm bốn địa danh. Ba khu triều đình có tên và vai trò mô tả. Tài liệu thiết kế không cấp dân số hay diện tích riêng cho ba khu này; code biểu diễn rõ trường chưa có bằng `std::optional` rỗng, nên không thể nhầm số 0 là dân số hoặc diện tích thật.

ID trong code bắt đầu từ 0; cột ID thiết kế L01–L24 được viết ở dạng một-based để đối chiếu với bảng tài liệu. Các con số dân số dưới đây là hàng nghìn người, diện tích là km²:

| ID thiết kế | Lãnh địa / cụm | Dân (nghìn) | Diện tích | Đặc sản | Lợi thế và nút thắt |
|---|---|---:|---:|---|---|
| L01 | Thanh Điền / lưu vực | 420 | 8.000 | Gạo nước | Đất màu, sông lớn; lũ và quyền thủy lợi tập trung |
| L02 | Liên Trạch / lưu vực | 210 | 5.000 | Cá khô | Đầm hồ, vận tải thủy; bệnh và thiếu đất khô |
| L03 | Phù Sa / lưu vực | 310 | 7.000 | Dầu hạt | Ruộng luân canh, gần chợ; phụ thuộc đê điều |
| L04 | Đông Viên / lưu vực | 260 | 6.000 | Vải gai | Đông dân, vùng nguyên liệu; năng suất ruộng thấp |
| L05 | Bạch Khâu / cao nguyên | 150 | 14.000 | Len trắng | Đồng cỏ; thiếu gạo và thợ dệt |
| L06 | Hắc Sơn / cao nguyên | 110 | 11.000 | Sắt rèn | Quặng tốt; đường đèo, nhập lương thực |
| L07 | Tùng Lĩnh / cao nguyên | 95 | 13.000 | Gỗ đóng tàu | Rừng, sông thả gỗ; tái sinh rừng chậm |
| L08 | Xích Nham / cao nguyên | 80 | 9.000 | Đồng thỏi | Mỏ đồng; cần nhiên liệu và vốn |
| L09 | Nam Cảng / duyên hải | 280 | 3.000 | Muối biển | Cảng lớn; phụ thuộc gạo nhập và an ninh biển |
| L10 | Hải Châu / duyên hải | 190 | 4.000 | Tàu đi biển | Thợ tàu, bến sâu; cần gỗ, sắt, vải buồm |
| L11 | Bạch Sa / duyên hải | 130 | 4.500 | Thủy tinh | Cát và thương nhân; tiêu hao nhiên liệu |
| L12 | Trầm Hải / duyên hải | 120 | 6.000 | Trầm hương | Rừng đặc hữu, hàng giá trị cao; cầu xa xỉ biến động |
| L13 | Vân Lụa / đông lộ | 240 | 6.000 | Lụa nhuộm | Thợ và mạng khách hàng; thương hội hạn chế gia nhập |
| L14 | Cẩm Giang / đông lộ | 230 | 5.500 | Giấy tốt | Sông, kỹ năng; tranh nước với ruộng và xưởng |
| L15 | Hồng Diệp / đông lộ | 160 | 8.000 | Trà | Khí hậu, thương hiệu; mùa vụ và phí đường |
| L16 | Bích Lò / đông lộ | 200 | 5.000 | Đồ sứ | Đất sét, thợ lò; cần nhiên liệu và vận tải ít hư hao |
| L17 | Bắc Mã / biên bắc | 90 | 20.000 | Ngựa chiến | Đồng cỏ, quân sự; dân thưa, phòng thủ đắt |
| L18 | Phong Ải / biên bắc | 120 | 15.000 | Da thuộc | Đàn gia súc, cửa ải; nguy cơ giao thương bị đóng |
| L19 | Hoàng Nguyên / biên bắc | 170 | 18.000 | Lúa mạch | Ruộng rộng; khô hạn và xa thị trường |
| L20 | Thiết Quan / biên bắc | 140 | 10.000 | Binh khí | Thợ quân giới, vị trí phòng thủ; lệ thuộc sắt và đơn đặt hàng |
| L21 | Dược Cốc / tây lộ | 100 | 9.000 | Dược liệu | Khí hậu đa dạng; khó tiếp cận, chất lượng không đồng đều |
| L22 | Lâm Mật / tây lộ | 115 | 10.000 | Sáp ong | Rừng và sản phẩm phụ; quyền sử dụng rừng tranh chấp |
| L23 | Thạch Kiều / tây lộ | 180 | 7.000 | Đá xây | Mỏ và nút giao; hàng nặng, phụ thuộc công trình |
| L24 | Ngân Khê / tây lộ | 85 | 8.000 | Bạc | Khoáng sản giá trị cao; dễ lệ thuộc thu nhập mỏ |
| — | Kinh Kỳ / triều đình | Chưa chỉ định | Chưa chỉ định | Hành chính và tiêu dùng | Dân số và sản xuất thuộc tổng quốc gia nhưng hạch toán riêng |
| — | Hoàng Điền / triều đình | Chưa chỉ định | Chưa chỉ định | Ruộng và kho dự trữ | Dân số và sản xuất thuộc tổng quốc gia nhưng hạch toán riêng |
| — | Trực Lộ / triều đình | Chưa chỉ định | Chưa chỉ định | Hậu cần và xưởng nhà nước | Dân số và sản xuất thuộc tổng quốc gia nhưng hạch toán riêng |

## Thay đổi mã và kiểm tra

Thêm [world_atlas.hpp](../../core/include/polyec/world_atlas.hpp) với 27 bản ghi tĩnh, ID ổn định từ 0 đến 26, số lượng 24/3, validator kiểm tên/ID duy nhất, sáu cụm địa lý với bốn lãnh địa mỗi cụm và việc chỉ 24 lãnh địa có số liệu dân số/diện tích. Ba khu triều đình lưu hai trường này dưới dạng “không có giá trị”, không phải 0. Target `polyec_world_atlas_tests` cộng độc lập số dân và diện tích từng cụm rồi kiểm tổng: 4.185 nghìn người theo số liệu bối cảnh và 212.000 km². Việc cộng này chỉ phát hiện sai số chép bảng; không chuyển dân số thành Person, không mô phỏng quy mô quốc gia. Không sửa Realm, Campaign, API, save schema, binding, model hoặc UI. Kế hoạch và giới hạn: [M5-world-atlas-territory-catalog-plan.md](../design/M5-world-atlas-territory-catalog-plan.md).

Windows Release build target `polyec_world_atlas_tests` thành công; executable trực tiếp xác nhận 27 lãnh địa/khu, sáu tổng cụm, tổng dân số bối cảnh 4.185 nghìn người và diện tích 212.000 km². Chỉ build target này; không chạy ALL_BUILD hoặc full CTest. Đây là kiểm tra dữ liệu/validator, không phải kiểm tra mô phỏng 27 vùng.

## Điều chưa được chứng minh

Dân số atlas không được chuyển thành số NPC; chưa có contract ánh xạ. Catalog chưa có tọa độ, ranh giới, địa hình, nút đường, sức tải, nguồn tài nguyên, kho khởi đầu hay tuyến thương mại; các dữ liệu đó cần được nối riêng theo §12.1–12.4. Realm mặc định vẫn là pilot 7 vùng. Các luật M5.4 như thành thị, quân sự, ngoại thương, giao diện nhà vua và các chuỗi nguyên nhân–kết quả còn mở. Việc thêm catalog không làm M5 hoàn tất.
