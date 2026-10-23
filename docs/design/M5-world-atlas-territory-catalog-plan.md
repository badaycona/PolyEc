# M5.4 — danh mục tham chiếu 24 lãnh địa và 3 khu triều đình

## Mục tiêu và nguồn dữ liệu

Đưa bảng thiết kế ban đầu trong `docs/IMPLEMENTATION_PLAN.md` §12.1–12.3 thành dữ liệu C++ có thể kiểm tra khi xây quốc gia 27 vùng. Giữ nguyên 24 lãnh địa chư hầu, 6 cụm địa lý, ba khu triều đình, dân số bối cảnh tính bằng nghìn người, diện tích tính bằng km², tên đặc sản và phần mô tả lợi thế/nút thắt. Ba khu triều đình hiện chỉ có tên và vai trò mô tả; tài liệu chưa cấp số dân hoặc diện tích riêng nên hai trường này phải là giá trị không có, không dùng số 0 làm giá trị thay thế.

## Phạm vi

- Thêm header chỉ chứa dữ liệu tĩnh và validator `core/include/polyec/world_atlas.hpp`; dùng `std::optional` để ba khu triều đình thể hiện rõ dân số/diện tích chưa được chỉ định.
- Thêm `core/tests/test_world_atlas.cpp` và target CTest riêng để kiểm đếm đủ 27 vùng, đúng 24/3 loại, sáu cụm chư hầu với bốn lãnh địa mỗi cụm, ID/tên duy nhất, đơn vị, số liệu dân số/diện tích theo từng cụm và tổng cộng theo bảng đã công bố.
- Không tạo Person từ số dân bối cảnh; IMPLEMENTATION_PLAN §12.2 yêu cầu chốt riêng ánh xạ này trước khi sinh quốc gia.
- Không thêm tọa độ, polygon, địa hình, đường/tuyến, nguồn sản xuất, kho, nghĩa vụ, quyền sở hữu hoặc save record. Những dữ liệu đó chưa được chốt cho 27 vùng trong bảng hiện tại.
- Không thay `Realm` mặc định 7 vùng hoặc M4 Campaign pilot; không đổi action, observation, reward, model, binding hay UI.

## Kiểm tra ngay và kiểm tra về sau

Build `polyec_world_atlas_tests` Release và chạy executable trực tiếp. Sau khi một lát khác nối danh mục vào khởi tạo quốc gia, cần kiểm riêng việc tạo Realm 27 vùng, serialization, đường đi và legal actions; đây chưa phải kiểm tra vận hành 27 vùng.

## Kết quả kiểm tra

Windows Release build target `polyec_world_atlas_tests` và chạy trực tiếp executable: danh mục xác nhận 27 vùng, gồm 24 vassal và 3 royal domain; các tổng dân số/diện tích theo sáu cụm và toàn bảng phải khớp số liệu nguồn. Không build toàn project hoặc chạy suite khác vì header chỉ được target mới và include C++20 tiêu chuẩn. `git diff --check` và repository link checker được chạy sau khi cập nhật report. Tự review; không có reviewer độc lập.

## Cách báo cáo

Nêu đủ loại dữ liệu có trong từng bản ghi và đơn vị của các con số. Nói rõ đây là catalog tham chiếu, chưa là quốc gia đang chạy trong game; dân số atlas không được tính thành NPC. Tự review; không có reviewer độc lập.
