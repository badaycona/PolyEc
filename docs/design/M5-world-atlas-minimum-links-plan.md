# M5.4 — mạng nối tối thiểu trong atlas

## Mục tiêu và căn cứ

Chuyển các tuyến bắt buộc ở `docs/IMPLEMENTATION_PLAN.md` §12.3 thành 31 liên kết dữ liệu tĩnh giữa 27 ID lãnh địa đã có trong `world_atlas.hpp`: 3 đoạn sông lớn, 2 đoạn đường cần cải thiện, 3 đoạn ven biển, 6 đoạn tuyến đông, 5 đoạn tuyến bắc, 7 đoạn tuyến tây và 5 kết nối khu triều đình.

ID 0–23 tương ứng L01–L24; ID 24–26 tương ứng Kinh Kỳ, Hoàng Điền, Trực Lộ. Ba loại được tài liệu gọi rõ tên là sông, đường cần cải thiện và ven biển được ghi đúng loại. Các tuyến đông/bắc/tây và kết nối triều đình chỉ nêu thứ tự vùng, không nêu phương thức đi lại; chúng mang giá trị `Unspecified`. Hướng dòng sông giữ đúng thứ tự mũi tên tài liệu; tuyến có ký hiệu ↔ được đánh dấu hai chiều; phần “nối” khu triều đình không ghi hướng nên giữ hướng chưa xác định.

## Phạm vi thay đổi

- `core/include/polyec/world_atlas.hpp`: thêm loại hành lang, phương thức tuyến và liên kết có ID đầu/cuối, loại hành lang, phương thức đã biết/chưa biết và cờ hai chiều; thêm kiểm tra cấu trúc compile-time.
- `core/tests/test_world_atlas.cpp`: kiểm số liên kết từng hành lang, mọi cặp vùng liền kề của sáu chuỗi đường/sông/biển, đủ năm kết nối triều đình, loại đã biết/chưa biết và hướng.
- Không thêm thời gian, chiều dài, sức tải, chi phí, an toàn, chủ quyền, phí, mùa vụ, tọa độ, địa hình hoặc cơ chế tìm đường. Những giá trị này không được §12.3 chỉ định.
- Không nối liên kết vào Realm, thị trường, vận tải, UI hay save; chưa biến catalog thành bản đồ/quốc gia vận hành.

## Kiểm tra

Build và chạy riêng `polyec_world_atlas_tests` Release. Kiểm này xác nhận dữ liệu mới và catalog nền; không chứng minh đường đi, thời gian vận tải hay gameplay M5.4.

## Review

Tự rà đối chiếu từng chuỗi vùng với §12.3; phạm vi dữ liệu giữ nguyên các thuộc tính chưa có quyết định ở trạng thái chưa xác định. Chưa có reviewer độc lập.
