# Atlas tham khảo cho bước mở rộng27 vùng

Trích và biên tập từ catalog thiết kế cũ ngày2026-10-22. Đây là dữ liệu bối cảnh có thể tái sử dụng, không là cấu hình gameplay đã cân bằng hoặc nghĩa vụ phải xây đủ27 ngành sản xuất ngay.

24 lãnh địa thuộc sáu cụm; ba khu còn lại thuộc triều đình. Số dân là **nghìn người theo bối cảnh**, diện tích là km². Không dùng số này làm số NPC đã mô phỏng, không cấp tiền/hàng bằng phép nhân mặc định. Tên đặc sản gợi lợi thế trao đổi; có thể gộp vào nhóm hàng ở mốc A–C rồi mới mở rộng dữ liệu ở D.

| ID tham khảo | Vùng / cụm | Dân nghìn | Diện tích km² | Lợi thế bối cảnh | Nút thắt kinh tế chính trị |
|---|---|---:|---:|---|---|
| 0 | Thanh Điền / lưu vực | 420 | 8000 | Gạo nước | Lũ, quyền thủy lợi tập trung |
| 1 | Liên Trạch / lưu vực | 210 | 5000 | Cá khô | Đầm hồ, thiếu đất khô, điều kiện sức khỏe |
| 2 | Phù Sa / lưu vực | 310 | 7000 | Dầu hạt | Phụ thuộc đê điều |
| 3 | Đông Viên / lưu vực | 260 | 6000 | Vải gai | Đông dân, năng suất ruộng thấp |
| 4 | Bạch Khâu / cao nguyên | 150 | 14000 | Len | Thiếu thực phẩm và thợ dệt |
| 5 | Hắc Sơn / cao nguyên | 110 | 11000 | Sắt | Đèo khó đi, nhập lương thực |
| 6 | Tùng Lĩnh / cao nguyên | 95 | 13000 | Gỗ đóng tàu | Tái sinh rừng chậm |
| 7 | Xích Nham / cao nguyên | 80 | 9000 | Đồng | Nhiên liệu và vốn |
| 8 | Nam Cảng / duyên hải | 280 | 3000 | Muối, cảng | Nhập gạo, an ninh đường biển |
| 9 | Hải Châu / duyên hải | 190 | 4000 | Đóng tàu | Cần gỗ, sắt, vải buồm |
| 10 | Bạch Sa / duyên hải | 130 | 4500 | Thủy tinh | Tiêu hao nhiên liệu |
| 11 | Trầm Hải / duyên hải | 120 | 6000 | Trầm hương | Cầu xa xỉ thay đổi |
| 12 | Vân Lụa / đông lộ | 240 | 6000 | Lụa | Thương hội hạn chế gia nhập |
| 13 | Cẩm Giang / đông lộ | 230 | 5500 | Giấy | Tranh nước giữa ruộng và xưởng |
| 14 | Hồng Diệp / đông lộ | 160 | 8000 | Trà | Mùa vụ và phí đường |
| 15 | Bích Lò / đông lộ | 200 | 5000 | Đồ sứ | Nhiên liệu và vận tải hàng dễ vỡ |
| 16 | Bắc Mã / biên bắc | 90 | 20000 | Ngựa | Dân thưa, phòng thủ tốn kém |
| 17 | Phong Ải / biên bắc | 120 | 15000 | Da thuộc, cửa ải | Giao thương có thể bị đóng |
| 18 | Hoàng Nguyên / biên bắc | 170 | 18000 | Lúa mạch | Khô hạn, xa thị trường |
| 19 | Thiết Quan / biên bắc | 140 | 10000 | Binh khí | Phụ thuộc sắt và đơn hàng |
| 20 | Dược Cốc / tây lộ | 100 | 9000 | Dược liệu | Khó tiếp cận, chất lượng không đều |
| 21 | Lâm Mật / tây lộ | 115 | 10000 | Sáp ong | Tranh chấp quyền dùng rừng |
| 22 | Thạch Kiều / tây lộ | 180 | 7000 | Đá xây | Hàng nặng, phụ thuộc công trình |
| 23 | Ngân Khê / tây lộ | 85 | 8000 | Bạc | Dễ phụ thuộc thu nhập mỏ |
| 24 | Kinh Kỳ / triều đình | Chưa chỉ định | Chưa chỉ định | Hành chính, tiêu dùng | Cần hạch toán nguồn cung và chi công riêng |
| 25 | Hoàng Điền / triều đình | Chưa chỉ định | Chưa chỉ định | Ruộng, kho dự trữ | Dự trữ có nguồn và người quản lý |
| 26 | Trực Lộ / triều đình | Chưa chỉ định | Chưa chỉ định | Hậu cần, xưởng công | Vật tư, lao động và nghĩa vụ |

24 lãnh địa có tổng dân bối cảnh4185 nghìn và diện tích212000 km²; ba khu triều đình chưa có con số riêng. Không dùng0 để thay dữ liệu chưa được chỉ định. Định danh mới có thể dùng enum/ID khác nếu mapping rõ; không buộc kế thừa thứ tự ID vào save/model mới.

Ở bước một/hai lãnh thổ, dùng tên fixture rõ và chỉ những bất cân xứng cần kiểm. Khi đưa Atlas vào phải có topology/tuyến thật, phân bố tài nguyên/tài sản, người quản trị và đường can thiệp; hình vẽ hoặc bảng tên không thay bằng chứng world hoạt động. Không suy tuyến/sức tải chỉ từ tên cụm hoặc vị trí trên bản đồ.
