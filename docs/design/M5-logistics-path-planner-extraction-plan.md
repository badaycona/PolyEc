# M5.2 — tách phép tìm đường và đặt chỗ vận tải dùng chung

## Mục tiêu và căn cứ

`Campaign::preview()` hiện tự tính đường, chia lô theo sức tải từng ngày, né chỗ đã được hàng khác đặt và áp dụng thời gian của từng đoạn. Đây là luật vận tải thật của Campaign M4 nhưng bị giữ riêng trong wrapper, khiến core không có phép tính tuyến có thể tái dùng cho các vùng atlas. Tách phép lập kế hoạch thành helper C++ thuần, nhận đầy đủ thời gian/sức tải/tình trạng tuyến từ caller; Campaign chuyển dữ liệu hiện có vào helper và giữ nguyên công thức phí M4 ở wrapper.

Helper không gán thông số mặc định cho tuyến quốc gia. Nếu caller chưa cung cấp route data thì chưa thể tính tuyến. `days`, sức tải, số ngày chậm thêm, khoảng đặt chỗ và ngày bắt đầu đều là dữ liệu đầu vào; helper chỉ thực hiện cách tìm đường đang dùng trong Campaign.

## Contract cần giữ

- Tối đa 27 vùng, ID từ 0 đến `region_count-1`; Campaign truyền 7 vùng gồm 6 vùng và kho triều đình.
- Route đi được hai chiều như Campaign hiện hành; route đóng bị bỏ qua. Caller có thể truyền thời gian chậm thêm (Campaign dùng 4 ngày khi còn quyền thu phí).
- Một chuyến dùng `ceil(quantity/capacity)` ngày tải trên mỗi đoạn. Khoảng đặt chỗ tính hai đầu; hàng cùng đoạn theo hai hướng dùng chung sức tải.
- Các đặt chỗ đang hoạt động có thể đẩy ngày bắt đầu tới sau; đặt chỗ đã giao không cản chuyến mới.
- Thuật toán giữ quy tắc chọn đường ít ngày hiện tại. Khi bằng ngày, vùng được xét theo ID tăng dần, rồi tuyến theo thứ tự đầu vào; không thay bằng một quy tắc tie mới. Fee không nằm trong helper: Campaign tiếp tục tính công thức phí hiện có từ số lượng và tổng ngày.
- Input không hợp lệ hoặc không có đường trả `reachable=false`, không mutate đầu vào.

## Phạm vi file

- Thêm `core/include/polyec/logistics_manager.hpp`, `core/src/logistics_manager.cpp` và `core/tests/test_logistics_manager.cpp`.
- Đổi `core/src/campaign.cpp` để chuyển routes/cargo đang đi vào helper rồi tiếp tục tính fee và dựng `RoutePreview` như trước.
- Thêm source/test target vào `core/CMakeLists.txt`.
- Không đổi save schema, cấu trúc Campaign route/cargo, hành động, fee, route data, Realm action/observation, Python, native hoặc Godot.

## Kiểm tra ngay

Build và chạy trực tiếp `polyec_logistics_manager_tests`, `polyec_campaign_tests` Windows Release. Dedicated tests kiểm invalid ID/quantity/route data, vùng nguồn bằng đích, tuyến đóng, tuyến hai chiều, delay, chia batch, tránh đặt chỗ chồng nhau, đường đến cùng ngày có kết quả ổn định và input không bị sửa. Campaign regression xác nhận kết quả preview, cứu trợ, hàng đến, lưu/tải chỗ đã đặt và hồ sơ sau giao hàng vẫn đúng.

Không chạy full CTest/Debug hoặc UI/Python vì đây là refactor C++ thuần; API manager chưa được gọi từ các consumer đó. Nếu sau này nối Python/Realm hoặc đổi data model công khai, sẽ build consumer bị ảnh hưởng tại mốc tích hợp.

## Review

Tự đối chiếu từng nhánh với thuật toán cũ trong `Campaign::preview()`; đặc biệt bảo toàn ngày tải hai đầu, ngày đến, đường hai chiều, cách xử lý thời gian chậm thêm và tie theo ID vùng rồi thứ tự tuyến. Đây là lần review trong cùng lượt, không có reviewer độc lập.
