# Báo cáo: cách xác định người kế vị tước

## Kết quả cho người đọc

Đã bổ sung vào lõi mô phỏng C++ một phép tra cứu để xác định con nào đủ điều kiện kế vị tước của một người đã chết. Quy tắc theo kế hoạch dự án là chọn người con hợp pháp lớn tuổi nhất, không phân biệt giới tính. Người con vẫn được xét nếu chưa đủ tuổi trưởng thành, miễn là còn sống và quan hệ cha/mẹ–con đã được xác nhận là hợp pháp.

Nếu chỉ có một người con lớn tuổi nhất, phép tra cứu trả về người đó. Nếu không có người con đủ điều kiện, hoặc có từ hai người trở lên cùng lớn tuổi nhất, kết quả yêu cầu mở hồ sơ chờ công nhận. Khi có nhiều ứng viên đồng tuổi, tất cả ID của họ được giữ lại để một bước công nhận sau này xử lý; phép tra cứu không tự chọn người thắng bằng cách dựa vào ID.

Đây là phần xử lý bên trong chương trình, chưa phải chức năng người chơi có thể dùng. Hiện Realm — đối tượng C++ đang giữ và vận hành thế giới game — chưa lưu danh sách tước, chưa tạo hồ sơ công nhận khi người giữ tước chết, và chưa có lệnh để nhà vua công nhận người kế vị. Vì vậy tước quý tộc hiện chưa tự truyền trong game; phần này chỉ xác nhận cách tìm ứng viên, không hoàn tất M5.3.

## Cách xác định ứng viên

Phép tra cứu nhận ba đầu vào: người được xem là đã giữ tước và vừa chết; danh sách cư dân; và danh sách quan hệ giữa cư dân. Mỗi cư dân có một ID định danh ổn định. Mỗi quan hệ cũng có ID riêng để dữ liệu lỗi hoặc trùng có thể được phát hiện.

Một người con chỉ được xét khi đồng thời thỏa mãn tất cả điều kiện sau:

1. Quan hệ là cha/mẹ–con trực tiếp, không suy ra qua ông bà hay họ hàng xa.
2. Cạnh quan hệ lưu theo chiều từ người con tới người cha/mẹ.
3. Cạnh được đánh dấu đã xác nhận quan hệ cha/mẹ hợp pháp.
4. Người cha/mẹ trong cạnh chính là người giữ tước đã chết.
5. Người con còn sống.

Trong những người đủ điều kiện, tuổi được so sánh bằng số ngày tuổi đã lưu trong dữ liệu. Tuổi trưởng thành không phải điều kiện của luật kế vị này. Nếu cùng một quan hệ bị lưu lặp, phép tra cứu chỉ tính người con đó một lần. Cạnh chưa xác nhận và người con đã chết bị bỏ qua, kể cả khi người đó lớn tuổi hơn những người còn sống.

Nếu đầu vào có cấu trúc sai — ví dụ ID quan hệ vượt ngoài danh sách quan hệ, ID cư dân không đúng vị trí chuẩn trong danh sách, tuổi âm, hoặc người được khai báo là người giữ tước vẫn còn sống — phép tra cứu từ chối bằng lỗi thay vì đưa ra kết quả có thể gây hiểu nhầm.

## Các trường hợp đã kiểm tra

| Trường hợp | Kết quả mong đợi và đã kiểm tra |
|---|---|
| Có nhiều con hợp pháp còn sống, tuổi khác nhau | Chọn đúng người lớn tuổi nhất, kể cả khi người đó chưa đủ 18 tuổi game. |
| Quan hệ cha/mẹ–con ghi ngược chiều từ người giữ tước tới cha/mẹ của họ | Không coi cha/mẹ của người chết là con kế vị; chỉ đọc cạnh theo chiều con → cha/mẹ. |
| Người lớn tuổi nhất có cạnh chưa được xác nhận là hợp pháp | Không dùng người đó; xét những người có quan hệ đã được xác nhận. |
| Người lớn tuổi nhất đã chết | Bỏ qua người đó; xét những người con còn sống. |
| Một quan hệ hợp pháp xuất hiện lặp trong dữ liệu | Không biến cùng một người thành hai ứng viên và không tạo ra hòa giả. |
| Hai người con còn sống cùng có số ngày tuổi lớn nhất | Trả cả hai ID theo thứ tự tăng dần và yêu cầu công nhận; không tự chọn một người. |
| Không có con hợp pháp còn sống | Yêu cầu công nhận nhưng không trả ứng viên nào. |
| ID quan hệ nằm ngoài danh sách, ID cư dân không khớp vị trí chuẩn, tuổi cư dân âm, gắn cờ cha/mẹ hợp pháp lên một loại quan hệ khác, hoặc người giữ tước vẫn sống | Từ chối đầu vào không hợp lệ. |

## Kiểm tra đã chạy

Đã build target kiểm thử C++ chuyên cho các manager, sau đó chạy trực tiếp chương trình kiểm thử đó trên Windows Release:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/windows --config Release --target polyec_manager_tests
& build/windows/core/Release/polyec_manager_tests.exe
```

Kết quả lệnh chạy trực tiếp: mã thoát `0`, dòng kết quả `manager tests passed`. Bộ kiểm tra xác nhận các trường hợp trong bảng trên, gồm cả các ca dữ liệu cư dân và quan hệ không hợp lệ. Manifest ghi mã băm của mã nguồn, bài kiểm tra, báo cáo và chương trình đã chạy để có thể đối chiếu đúng các tệp được kiểm tra.

Không chạy toàn bộ CTest, cấu hình Debug, adapter Python, chương trình native hoặc Godot trong lát này. Thay đổi chỉ thêm phép tra cứu C++ thuần và bài kiểm tra manager; không đổi API Realm, dữ liệu lưu, binding hoặc giao diện mà các lớp đó gọi. Kiểm tra tích hợp sẽ cần làm khi bổ sung tước vào Realm và các lớp sử dụng nó.

## Phần còn thiếu trước khi đây trở thành chức năng game

- Realm cần có bản ghi tước riêng. Tước không thể dùng nhầm bản ghi quyền sở hữu đất: sở hữu đất và địa vị quý tộc là hai loại dữ liệu khác nhau.
- Khi người giữ tước chết, Realm cần ghi nhận người kế vị hoặc danh sách chờ công nhận trong cùng một giao dịch với vòng đời nhân vật.
- Cần có lệnh công nhận, quy tắc ai được thực hiện lệnh đó, và kiểm tra thành công/từ chối nguyên tử. Quy tắc hiện có chưa chỉ rõ các chi tiết này.
- Bản ghi tước và hồ sơ chờ công nhận cần được lưu, tải lại và kiểm tra dữ liệu lỗi trong C++.
- Sau khi Realm có dữ liệu và lệnh, cần nối thông tin phù hợp tới Python hoặc giao diện nếu các lớp đó phải hiển thị hay chọn hành động liên quan.

Không tự thêm các chi tiết còn thiếu trong báo cáo này. Plan triển khai giới hạn của lát hiện tại ở phép tra cứu ứng viên: [kế hoạch](../design/M5-peerage-successor-resolution-plan.md). Luật kế vị được đối chiếu từ [kế hoạch dự án, mục 2](../IMPLEMENTATION_PLAN.md). Mã băm và kích thước các tệp được ghi trong [manifest](../../artifacts/m5-peerage-successor-resolution-2026-10-22-manifest.json).
