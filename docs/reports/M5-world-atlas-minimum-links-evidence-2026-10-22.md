# Báo cáo: các liên kết bắt buộc ban đầu trong atlas

## Thay đổi

`core/include/polyec/world_atlas.hpp` hiện lưu 31 đoạn nối tĩnh trong catalog atlas 27 vùng. Mỗi đoạn chỉ ghi vùng đầu, vùng cuối, hành lang được mô tả trong tài liệu, phương thức đã biết hoặc chưa xác định, và đoạn đó có cho phép đi hai chiều theo ký hiệu nguồn hay không. ID 0–23 là L01–L24 theo đúng thứ tự bảng; ID 24–26 lần lượt là Kinh Kỳ, Hoàng Điền và Trực Lộ.

| Hành lang được nêu trong kế hoạch | Các đoạn nối theo thứ tự | Phương thức lưu | Hướng trong catalog |
|---|---|---|---|
| Sông lớn | Thanh Điền → Phù Sa; Phù Sa → Liên Trạch; Liên Trạch → Nam Cảng | Sông | Theo mũi tên tài liệu |
| Đường cần cải thiện | Hắc Sơn ↔ Bạch Khâu; Bạch Khâu ↔ Thanh Điền | Đường bộ | Hai chiều |
| Ven biển | Nam Cảng ↔ Hải Châu; Hải Châu ↔ Bạch Sa; Bạch Sa ↔ Trầm Hải | Vận tải ven biển | Hai chiều |
| Tuyến đông | Thanh Điền ↔ Đông Viên; Đông Viên ↔ Vân Lụa; Vân Lụa ↔ Cẩm Giang; Cẩm Giang ↔ Hồng Diệp; Hồng Diệp ↔ Bích Lò; Bích Lò ↔ Hải Châu | Chưa xác định | Hai chiều, theo ký hiệu ↔ |
| Tuyến bắc | Bạch Khâu ↔ Bắc Mã; Bắc Mã ↔ Phong Ải; Phong Ải ↔ Hoàng Nguyên; Hoàng Nguyên ↔ Thiết Quan; Thiết Quan ↔ Hắc Sơn | Chưa xác định | Hai chiều, theo ký hiệu ↔ |
| Tuyến tây | Hắc Sơn ↔ Tùng Lĩnh; Tùng Lĩnh ↔ Xích Nham; Xích Nham ↔ Ngân Khê; Ngân Khê ↔ Thạch Kiều; Thạch Kiều ↔ Dược Cốc; Dược Cốc ↔ Lâm Mật; Lâm Mật ↔ Phù Sa | Chưa xác định | Hai chiều, theo ký hiệu ↔ |
| Kết nối ba khu triều đình | Kinh Kỳ — Thanh Điền; Kinh Kỳ — Trực Lộ; Hoàng Điền — Thanh Điền; Hoàng Điền — Phù Sa; Trực Lộ — Thiết Quan | Chưa xác định | Chưa xác định; nguồn chỉ viết “nối” |

Các tuyến triều đình không dùng ký hiệu ↔ trong nguồn; catalog giữ chiều kết nối chưa xác định thay vì tự suy ra đi được hai chiều. Chúng chưa phải luật đi lại. Phương thức và chiều đi cho các tuyến đông, bắc, tây và năm kết nối triều đình được để trống về mặt dữ liệu (`Unspecified`), không tự gán thành đường bộ hoặc đường thủy.

## Kiểm tra đã chạy

Đã build riêng target Windows Release `polyec_world_atlas_tests` và chạy trực tiếp `build/windows/core/Release/polyec_world_atlas_tests.exe`. Kết quả: thành công; thông báo xác nhận đủ 27 vùng, sáu tổng cụm, 31 liên kết và các tổng dân số/diện tích bối cảnh 4.185 nghìn người/212.000 km². Kiểm tra mới xác nhận số liên kết ở từng hành lang là 3/2/3/6/5/7/5; kiểm từng cặp kề nhau của toàn bộ sáu chuỗi trong tài liệu và cả năm kết nối triều đình; vùng đầu-cuối hợp lệ; không có đoạn lặp trong cùng hành lang; và phương thức/hướng được ghi đúng hoặc để chưa xác định theo nguồn.

Lệnh build đã dùng CMake cài kèm Visual Studio 18 với cấu hình Release và target `polyec_world_atlas_tests`. Do cây build Windows phải cấu hình lại từ thay đổi CMakeLists đã cũ, target phụ thuộc `polyec_core` cũng được biên dịch lại trước runner atlas. Không chủ ý chạy `ALL_BUILD`, toàn bộ CTest, Debug, ứng dụng, Godot, Python hoặc native app.

## Điều kết quả chứng minh và chưa chứng minh

Đồ thị atlas giờ có thể cung cấp danh sách liên kết hành lang tối thiểu đã chỉ định để bước sau nối vào bản đồ. Các liên kết không tạo dân cư, vùng vận hành mới hoặc thay đổi quyết định của người chơi/NPC. Realm vẫn là pilot 7 vùng.

Đây chưa phải tuyến thương mại hay bản đồ vận hành. Catalog chưa có tọa độ, chiều dài, thời gian đi, sức tải, chi phí, độ an toàn, chủ quyền, phí đường, mùa vụ, địa hình, cầu/bến, tìm đường hoặc lệnh vận tải. Tài liệu §12.3 nhắc sông có công suất theo mùa nhưng chưa đưa số liệu; không có kiểm tra lưu lượng hoặc vận chuyển hàng trong thay đổi này. Chưa có đường đi từ các liên kết này chạy qua Realm hoặc UI.

## Tài liệu đối chiếu

- Nguồn tuyến: `docs/IMPLEMENTATION_PLAN.md` §12.1 và §12.3.
- Quy tắc không bịa thuộc tính vận tải: [kế hoạch triển khai](../design/M5-world-atlas-minimum-links-plan.md).
- Kiểm tra atlas nền 27 vùng: [báo cáo danh mục lãnh địa](M5-world-atlas-territory-catalog-evidence-2026-10-22.md).
- Mã băm SHA-256 và kích thước của source, test, report, executable và tracker: [manifest](../../artifacts/m5-world-atlas-minimum-links-2026-10-22-manifest.json).
