# Báo cáo M5.2: tách bộ lập kế hoạch tuyến và đặt chỗ vận tải

## Kết quả người chơi sẽ thấy

Không có thay đổi nhìn thấy trong giao diện hoặc luật Campaign. Đoàn hàng tiếp tục chọn tuyến, chia khối lượng theo sức tải từng ngày, chờ chỗ đã được chuyến khác đặt, cộng thời gian đi và phí theo đúng cách cũ.

Thay đổi ở cấu trúc C++: phần tính đường trước đây nằm trong `Campaign::preview()` nay là một bộ tính độc lập của core. Nó có thể nhận tối đa 27 vùng cùng danh sách tuyến và chỗ đã đặt do caller cung cấp. Campaign M4 gọi bộ tính này với bảy vùng của chính mình. Atlas có thể dùng bộ tính sau khi dữ liệu tuyến thực tế được xác định; bài này chưa tự cấp thời gian, sức tải hoặc phí cho 31 liên kết atlas.

## Đầu vào và kết quả của bộ tính

Caller cung cấp:

1. Số vùng, ngày game hiện tại, vùng gửi và vùng nhận.
2. Số hàng muốn chở.
3. Từng tuyến: hai đầu vùng, số ngày đi, lượng hàng tối đa có thể xếp mỗi ngày, số ngày chậm thêm nếu có, và tuyến đang mở hay đóng.
4. Các khoảng ngày đã đặt bởi hàng chưa giao.

Helper trả về có tìm được đường hay không, số ngày từ hiện tại tới lúc hàng tới, danh sách vùng trung gian và với mỗi đoạn tuyến ngày bắt đầu xếp, ngày kết thúc xếp và ngày đến. Nếu dữ liệu vùng/tuyến/đặt chỗ không hợp lệ hoặc không còn đường mở, helper trả kết quả không tới và không sửa đầu vào.

Hàng được chia thành số mẻ ngày bằng cách làm tròn lên `số hàng ÷ sức tải/ngày`. Ví dụ 20 đơn vị qua tuyến có sức tải 10 đơn vị/ngày cần hai ngày xếp hàng. Khoảng xếp tính cả ngày đầu lẫn ngày cuối. Hai chiều của cùng tuyến dùng chung các chỗ này. Nếu một khoảng mới đè lên chỗ đã đặt, helper thử lại từ ngày sau khi khoảng bận kết thúc; nó lặp tới khi tìm được khoảng trống.

Khi có nhiều đường, helper chọn đường đến sớm nhất. Nếu ngày đến bằng nhau, thứ tự tìm vẫn là vùng ID tăng dần rồi thứ tự tuyến được caller đưa vào; đây là cách Campaign M4 cũ phá hòa. Helper không tự chọn “rẻ hơn”, “an toàn hơn” hay chiến lược kinh tế. Phí cũng không thuộc helper: Campaign tiếp tục áp dụng nguyên công thức cũ `số lượng × (1 + tổng ngày) ÷ 10 + 5`.

## Các phép kiểm độc lập cho helper

Mạng thử có tuyến 0↔1 mất hai ngày, sức tải 10/ngày; tuyến 1↔2 mất ba ngày, sức tải 10/ngày; đường trực tiếp 0↔2 mất 10 ngày. Khi chở 20 đơn vị từ 0 tới 2 ở ngày 0, helper chọn 0→1→2. Đoạn đầu xếp ngày 0–1 và đến ngày 3; đoạn sau xếp ngày 3–4 và đến ngày 7. Đi ngược từ 2 về 0 vẫn có đường 2→1→0 với cùng thời gian.

Các ca còn lại xác nhận ngày chậm thêm được dùng đúng khi caller truyền, tuyến đóng bị bỏ qua, đi từ một vùng tới chính nó không cần tuyến, input sai bị từ chối, đặt chỗ ngược chiều vẫn chặn cùng mặt đường, hai khoảng bận liên tiếp đẩy chuyến mới tới ngày 5, và khối lượng cần nhiều ngày hơn miền số nguyên cho phép không tạo lịch tràn. Một reservation không khớp tuyến hiện có cũng bị từ chối. Input không bị thay đổi sau khi tính.

Target helper trực tiếp đạt **14/14 checks**. Có một ca hai đường đều tới sau hai ngày nhưng qua vùng trung gian ID 2 và ID 1; helper giữ đường qua ID 1, giống quy tắc Campaign cũ chọn vùng chưa hoàn tất có ID nhỏ hơn trước.

## Đối chiếu Campaign trước và sau khi tách

Campaign seed 37 lúc bắt đầu có tuyến thu phí 1↔2 mất năm ngày, sức tải 80/ngày và thêm bốn ngày do quyền thu phí; các tuyến khác giữ dữ liệu M4 hiện có. Chuyến 200 đơn vị từ vùng 6 tới vùng 1 vẫn đi 6→0→2→1, tới ngày thứ 16. Các đoạn được đặt vào ngày 0, ngày 2 và các ngày 5–7; ngày hàng tới theo từng đoạn lần lượt là 2, 5 và 16. Phí tổng vẫn là 345.

Sau khi nhà vua trả khoản thỏa thuận và công trình đường hoàn tất, đoạn 1↔2 đổi theo dữ liệu M4 thành hai ngày và sức tải 300/ngày, không còn thêm bốn ngày thu phí. Preview vẫn dùng đường 6→0→2→1, nhưng chuyến tương đương tới sau 7 ngày và phí là 165. Test Campaign còn tiếp tục kiểm lệnh cứu trợ, hàng trên đường, tránh đặt chỗ của chuyến trước, ngày giao, lưu/tải khi đang vận chuyển và đóng hồ sơ thiếu sinh kế.

Target Campaign sau refactor đạt **281/281 checks**. Runner chạy workload Campaign 10.000 người trong 102 ngày; thời gian hiển thị ở lượt cuối khoảng **357 mili giây**. Đây là workload smoke kèm theo, không phải benchmark chuẩn hoặc phép đo ứng dụng đầy đủ.

## Build và kiểm tra đã chạy

Windows Release, build đúng core và hai target bị ảnh hưởng:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/windows --config Release --target polyec_logistics_manager_tests polyec_campaign_tests
& .\build\windows\core\Release\polyec_logistics_manager_tests.exe
& .\build\windows\core\Release\polyec_campaign_tests.exe
```

Core và hai executable build thành công. Cả hai runner thoát mã `0`; kết quả đã xác nhận là **14/14** và **281/281**. Campaign direct run kiểm tra helper sau khi tách cùng các giá trị đường/ngày/fee được khóa trong test; phép kiểm tie cuối cùng chỉ đổi target helper độc lập và được chạy lại sau đó.

Lần build đầu biên dịch `polyec_core` thành công nhưng phát hiện hai dòng trong test mới truyền vector đặt chỗ vào vị trí vector tuyến. Tôi sửa đúng kiểu đối số trong fixture rồi build lại; không sửa thuật toán để né lỗi. Bài kiểm cuối sau đó qua.

## Phạm vi chưa kiểm

Không đổi luật game, Campaign routes, phí, save schema, command, action mask, observation, Python binding, native consumer, GDExtension hay Godot. Không chạy toàn CTest hoặc Debug vì thay đổi này chỉ liên quan bộ tính C++ mới và Campaign caller; không có binding hoặc ABI consumer mới cần kiểm trong lát này. Target atlas chưa dùng LogisticsManager, vì các liên kết atlas chưa có thông số vận hành. Tuyến quốc gia, nguồn hàng thị trường đầy đủ và ba chuỗi M5.4 vẫn mở.

Đây là refactor để dùng chung thuật toán định tuyến, không phải hoàn tất vận tải M5.2. Tự review đối chiếu với `Campaign::preview()` cũ; không có reviewer độc lập và không commit/push. [Kế hoạch](../design/M5-logistics-path-planner-extraction-plan.md). Manifest ghi mã băm/kích thước của source, test, docs và build: [m5-logistics-path-planner-extraction-2026-10-22-manifest.json](../../artifacts/m5-logistics-path-planner-extraction-2026-10-22-manifest.json).
