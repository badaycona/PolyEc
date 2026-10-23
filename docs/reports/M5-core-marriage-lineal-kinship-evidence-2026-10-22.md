# Báo cáo: chặn kết hôn giữa tổ tiên và hậu duệ qua nhiều thế hệ

## Thay đổi người chơi sẽ nhận thấy

Realm giờ từ chối đề nghị kết hôn nếu hai người có quan hệ tổ tiên–hậu duệ, dù giữa họ có một thế hệ hay nhiều thế hệ. Ví dụ, cha/mẹ và con vẫn bị từ chối như trước; ông/bà và cháu cũng bị từ chối. Quy tắc cấm anh chị em vẫn giữ nguyên.

Kiểm tra áp dụng cho các quan hệ cha/mẹ–con mà Realm đang lưu dưới dạng quan hệ gia đình có kiểu rõ ràng. Cờ xác nhận quyền thừa kế không làm thay đổi việc kiểm tra hôn nhân: quan hệ họ hàng trong đồ thị vẫn được tính theo luật hiện có. Quan hệ anh/chị/em họ không tự bị cấm chỉ vì có chung tổ tiên; hiện không có luật nào trong kế hoạch tổng thể yêu cầu suy ra hoặc cấm nhóm đó.

Realm kiểm tra quan hệ ở cả hai thời điểm: khi một người gửi đề nghị và khi người kia chấp thuận. Vì vậy, nếu dữ liệu quan hệ thay đổi trong lúc đề nghị đang chờ, chấp thuận vẫn bị từ chối khi hai người lúc đó có quan hệ trực hệ. Lệnh bị từ chối không chuyển tiền, không tạo quan hệ vợ chồng và không sửa thế giới. Để kiểm tra này không phải quét toàn bộ quan hệ cho từng ứng viên cưới, Realm giữ một chỉ mục dẫn xuất: chỉ mục được tạo khi mở thế giới, cập nhật khi có em bé mới, và dựng lại từ quan hệ đã lưu khi tải game. Chỉ mục không được ghi thành một bản dữ liệu thứ hai trong file save.

## Lý do sửa

Kế hoạch dự án, [mục 2](../IMPLEMENTATION_PLAN.md), cấm hôn nhân giữa người cùng dòng trực hệ hoặc là anh chị em. Kiểm tra cũ chỉ nhìn một quan hệ cha/mẹ–con trực tiếp. Quan hệ ông/bà–cháu không nhất thiết có một cạnh trực tiếp nối hai người, nên có thể vượt qua kiểm tra đó. Mã hiện đã lần theo chuỗi quan hệ cha/mẹ–con tới các thế hệ trước để phát hiện quan hệ tổ tiên–hậu duệ.

## Các trường hợp đã kiểm tra

| Trường hợp | Kết quả đã kiểm tra |
|---|---|
| Hai người trưởng thành là cha/mẹ và con trực tiếp | Đề nghị bị từ chối; bài kiểm hiện có vẫn pass. |
| Hai người trưởng thành là anh chị em | Đề nghị bị từ chối; bài kiểm hiện có vẫn pass. |
| Hai người trưởng thành là ông/bà và cháu, với một thế hệ ở giữa | Action mask báo lệnh không hợp lệ và Realm command bị từ chối; fingerprint trước/sau giống nhau. |
| Đề nghị được gửi trước khi quan hệ ông/bà–cháu được thêm vào fixture | Người nhận không thể chấp thuận sau khi quan hệ xuất hiện; lệnh từ chối và fingerprint không đổi. |
| Chuỗi ông/bà–cha/mẹ–cháu được lưu rồi tải vào Realm mới | Realm tải thành công và action mask sau tải vẫn từ chối cặp ông/bà–cháu, xác nhận chỉ mục được dựng lại từ quan hệ đã lưu. |
| Realm tạo một em bé mới cùng cạnh cha/mẹ–con | Chỉ mục được cập nhật ngay khi sinh; kiểm tra trực tiếp xác nhận quan hệ mới được nhận diện. Sau save/load, quan hệ đó vẫn được nhận diện từ dữ liệu đã lưu. |
| Hai người trưởng thành không có quan hệ bị cấm | Có thể gửi đề nghị; các kiểm tra escrow, chấp thuận và hoàn tất hôn nhân hiện có tiếp tục chạy trong cùng bộ kiểm Court. |
| Một bên chưa đủ 18 năm game | Vẫn bị từ chối như trước. |

Fixture ông/bà–cháu dùng ba người trưởng thành. Hai cạnh cha/mẹ–con được lưu theo hướng con → cha/mẹ, đúng với định dạng quan hệ của Realm. Fixture đánh dấu hai cạnh này chưa xác nhận tính hợp pháp; kiểm tra hôn nhân vẫn chặn dựa trên quan hệ gia đình đã lưu, cùng cách hoạt động như kiểm tra trực tiếp trước đây.

## Kiểm tra đã chạy

Đã build và chạy trực tiếp bộ kiểm Court C++ trên Windows Release:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/windows --config Release --target polyec_court_tests
& build/windows/core/Release/polyec_court_tests.exe
```

Kết quả: mã thoát `0`; dòng kết quả là `COURT TESTS: 557 checks passed`. Bộ kiểm gồm kiểm tra cũ về kết hôn, tiền ký quỹ, từ chối/hoàn tiền, chờ thời gian, lưu/tải và các ca mới cho quan hệ nhiều thế hệ.

Do thay đổi có thêm chỉ mục dẫn xuất được cập nhật khi sinh người và khi tải, tôi cũng build và chạy riêng Realm runner Windows Release để kiểm tra đường khởi tạo, sinh con, lưu/tải và các luật C++ liên quan:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/windows --config Release --target polyec_realm_tests
& build/windows/core/Release/polyec_realm_tests.exe
```

Kết quả: mã thoát `0`; `REALM TESTS: 20846 checks passed`. Runner này chạy kèm profile A gồm 10.000 người, 19.743 quan hệ thưa và 90 ngày. Đây không phải profile B hoặc benchmark toàn chu trình.

Ngoài test Realm, do `Realm` là kiểu C++ được thư viện native sử dụng, tôi build riêng Release `polyec_ai` để kiểm tra consumer C++ này vẫn biên dịch với bố cục class mới. Tôi cũng build `polyec_realm_contracts` và chạy đúng lệnh hợp đồng native dùng trong CTest, truyền ba model pilot v6 cùng tên bundle và đường dẫn file kết quả. Kết quả là `RUNTIME CONTRACTS: 24 checks passed`; runtime từ chối bundle lịch sử đúng như mong đợi và không chạy neural inference vì workspace không có bundle đã train tương thích v11. Đây là kiểm tra nạp model/contract, không phải kiểm chất lượng policy.

Python binding được tắt trong cấu hình build hiện có (`POLYEC_BUILD_PYTHON=OFF`), vì vậy không thể kiểm extension hoặc replay Python ở lượt này. Godot cũng chưa chạy. Không chạy toàn bộ CTest hay Debug. Chỉ thay đổi luật kiểm tra quan hệ trong C++ Realm, chỉ mục quan hệ dẫn xuất và bài kiểm Court; không đổi dữ liệu save, số action/observation, binding hoặc nội dung mô hình. Kiểm tra native vừa chạy xác nhận consumer biên dịch và guard bundle hiện tại, nhưng không thay cho Python/Godot smoke ở mốc tích hợp nhóm chức năng ổn định.

## Phạm vi còn lại

Thay đổi này khép lỗ hổng ông/bà–cháu theo luật trực hệ hiện hành. Nó không thêm luật nhận con nuôi, ly hôn, anh/chị/em họ, con ngoài hôn nhân hoặc quyền giám hộ kết hôn. Các phần đó không được tự suy ra từ sửa đổi này. Tự review mã và fixture, không có reviewer độc lập; không commit/push. Mã băm và kích thước các tệp kiểm tra có trong [manifest](../../artifacts/m5-core-marriage-lineal-kinship-2026-10-22-manifest.json).
