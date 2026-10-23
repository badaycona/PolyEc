# M5.4 — truy vấn đường nối được khai báo trong atlas

## Mục tiêu và căn cứ

`world_atlas.hpp` đã lưu 27 lãnh địa và 31 liên kết tối thiểu theo IMPLEMENTATION_PLAN §12.3, nhưng caller chưa thể hỏi hai ID vùng có nối được qua các liên kết đã mô tả hay không. Thêm phép truy vấn đường trên topology tĩnh, có thể tạm đánh dấu liên kết bị đóng để tìm đường vòng. Native map và kiểm tra dữ liệu sau này dùng cùng cách hiểu về hướng liên kết và trạng thái khả dụng do caller cung cấp.

Đường trả về chỉ là chuỗi ID vùng nối nhau bằng các liên kết atlas. Chỉ đi theo cạnh một chiều đúng hướng hoặc cạnh hai chiều đã khai báo; bỏ qua cạnh chưa xác định hướng. Phương thức vận tải chưa xác định không ngăn việc biết hai vùng được nối về mặt địa lý. Kết quả không tuyên bố tuyến hoạt động, có thể đi trong mùa hiện tại, có thời gian, sức tải, phí, an toàn hay chủ quyền.

## Phạm vi và invariant

- `core/include/polyec/world_atlas.hpp`: thêm helper xác định một liên kết có cho phép hướng A→B không và hàm BFS trả tối đa 27 ID vùng.
- `core/include/polyec/world_atlas.hpp`: nhận mask theo 31 liên kết; bit đóng loại cạnh đó khỏi phép tìm đường, không đổi dữ liệu atlas nền.
- `core/tests/test_world_atlas.cpp`: kiểm đường sông xuôi dòng, không duyệt ngược qua chính cạnh sông, mọi cặp trong 24 lãnh địa chư hầu có đường nối hợp lệ, đóng đoạn đường 5↔4 rồi tìm vòng qua tuyến bắc, đóng mọi liên kết thì không có đường, xử lý liên kết triều đình chưa rõ hướng, cùng ID hợp lệ và ID ngoài atlas.
- Không sửa Realm, Campaign, save, gameplay, giá trị kinh tế, binding hoặc UI.

## Kiểm tra

Build riêng `polyec_world_atlas_tests` Windows Release rồi chạy runner trực tiếp. Không build toàn project vì helper nằm trong dữ liệu atlas header-only và target atlas test là consumer trực tiếp duy nhất trong lát này.

## Review

Tự kiểm tra mỗi bước BFS qua `AtlasLinkDirection`; các liên kết có mode `Unspecified` nhưng hướng hai chiều được dùng cho nối địa lý, còn hướng `Unspecified` bị bỏ qua. Đây không phải phép tìm đường vận tải trong gameplay và chưa có reviewer độc lập.
