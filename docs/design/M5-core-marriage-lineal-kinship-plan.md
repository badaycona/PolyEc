# M5.3 — chặn kết hôn giữa mọi thế hệ cùng dòng trực hệ

## Căn cứ và vấn đề

`docs/IMPLEMENTATION_PLAN.md` §2 nói hôn nhân cấm quan hệ trực hệ hoặc anh chị em. Mã hiện tại trong `Realm::marriage_related` chỉ chặn một cạnh cha/mẹ–con trực tiếp và cạnh anh chị em đã ghi. Vì cạnh cha/mẹ–con lưu theo chiều người con → cha/mẹ, một cặp ông/bà với cháu có thể không có cạnh trực tiếp nối hai người và hiện lọt qua kiểm tra.

## Thay đổi dự kiến

- Sửa `core/src/realm.cpp` để tìm quan hệ tổ tiên–hậu duệ bằng cách lần theo các cạnh `ParentChild` từ người con lên cha/mẹ, có tập đã thăm để kết thúc an toàn nếu dữ liệu có chu trình. Dùng chỉ mục quan hệ dẫn xuất để kiểm tra action mask không dựng lại toàn bộ đồ thị cho từng ứng viên; tạo lại chỉ mục khi khởi tạo/tải và cập nhật khi sinh cạnh cha/mẹ–con mới.
- Cấm đề nghị hôn nhân nếu một người là tổ tiên của người kia ở bất kỳ số thế hệ nào; tiếp tục cấm cạnh `Sibling` trực tiếp như hiện tại.
- Xét mọi cạnh cha/mẹ–con có kiểu này, không chỉ cạnh đã đánh dấu parentage hợp pháp, để nhất quán với kiểm tra hiện có và quan hệ huyết thống được lưu; không mở rộng sang anh/chị/em họ hoặc suy luận gia phả khác.
- Thêm Realm court test dùng ba người trưởng thành và hai cạnh tạo quan hệ ông/bà–cháu. Kiểm action mask từ chối, lệnh gửi đề nghị và chấp thuận đều từ chối nguyên tử, không tạo/settle escrow hoặc quan hệ vợ chồng; save/load không làm mất kết quả từ chỉ mục dẫn xuất.
- Không đổi save schema, action/observation shape, thời gian chờ, escrow, quan hệ sibling hoặc xử lý adoption/ly hôn/con ngoài hôn nhân.

## Kiểm tra ngay và kiểm tra để mốc sau

- Build và chạy trực tiếp `polyec_court_tests` Windows Release cho luật hôn nhân; chạy `polyec_realm_tests` Windows Release để kiểm khởi tạo, sinh con và save/load có giữ chỉ mục dẫn xuất đúng.
- Không đổi dữ liệu lưu, số action/observation, binding hoặc nội dung mô hình. Chưa chạy Python/native/Godot hoặc toàn CTest ở lát nhỏ này; tích hợp các lớp đó vẫn là mốc nhóm chức năng ổn định.
- Tự review call path offer/accept để xác nhận cùng một hàm xác thực được gọi tại cả lúc gửi đề nghị và lúc chấp thuận; kiểm tra trường hợp nhiều thế hệ và dữ liệu có chu trình không lặp vô hạn.

## Ranh giới hoàn thành

Hoàn tất lát này khi mọi tổ tiên/hậu duệ trong đồ thị `ParentChild` đều bị chặn ở đề nghị hoặc chấp thuận, quan hệ anh chị em trực tiếp vẫn bị chặn, và command bị từ chối không sửa tiền hay quan hệ. Đây là sửa lỗi áp dụng quy tắc hiện có; không tự thêm luật hôn nhân nào khác. Tự review, không có reviewer độc lập.
