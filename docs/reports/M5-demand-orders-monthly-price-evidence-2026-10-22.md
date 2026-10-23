# M5: đơn mua, ký quỹ, phân phối hàng và giá theo tháng

## Kết quả dễ thấy trong mô phỏng

Phần lõi C++ nay nhận được lệnh mua hàng có giới hạn giá và lượng mua. Khi lệnh được chấp nhận, tiền được rút khỏi tiền riêng của người mua trước; nếu chưa đủ thì lấy tiếp từ quỹ hộ. Khoản tiền này được giữ riêng cho tới ngày quyết toán kế tiếp. Chợ chỉ giao hàng đang có thật ở vùng đó, thu thuế theo sổ thuế hiện hành, rồi trả lại phần tiền giữ dư hoặc tiền của phần hàng không mua được.

Nếu nhiều người cùng đặt mua một món mà kho không đủ, chợ chia lượng hàng theo mức ưu tiên đã quy định. Trong cùng một mức, mỗi người nhận phần theo tỷ lệ lượng họ yêu cầu; những món lẻ còn dư lần lượt theo ID lệnh tăng dần. Hệ thống không tự tạo thêm hàng. Người mua chỉ nhận hàng đã được lấy khỏi kho vùng và được cộng vào kho cá nhân.

Cuối mỗi tháng mô phỏng, tổng lượng mua đã được bảo đảm bằng tiền, lượng không mua được và lượng hàng còn lại được dùng để điều chỉnh giá. Hệ số hiện tại là giá thử để kiểm luật, chưa được hiệu chỉnh bằng thực nghiệm. Lưu/tải giữ nguyên lệnh đang chờ, tiền giữ lại và số liệu trong tháng; lệnh không được quyết toán lần hai sau khi tải.

Đây mới là phần luật trong core. Giao diện Godot, Python trainer và native runtime chưa gọi lệnh mới, vì vậy báo cáo này không khẳng định người chơi đã có nút đặt lệnh trong màn hình game hoặc policy đã học được cách dùng lệnh.

## Quy tắc được triển khai

- Một lệnh thông thường chỉ có hiệu lực tới lần quyết toán ngày kế tiếp. Người mua có thể hủy lệnh còn mở; hủy lệnh lần hai, ID sai, lệnh của người khác hoặc lệnh đã được quyết toán bị từ chối mà không đổi thế giới.
- Tiền giữ lại là giá tối đa mỗi món nhân số lượng. Khi giá hiện tại cao hơn giới hạn thì không giao hàng. Nếu giá thấp hơn, chỉ thu giá hiện tại. Nếu không có món hàng nào được giao, toàn bộ khoản giữ lại vẫn phải được trả về đúng tài khoản ban đầu.
- Chợ phân bổ theo ba mức: nghĩa vụ hợp đồng có hàng bảo đảm (để hệ thống hợp đồng sau này dùng), nhu cầu lương thực do engine tạo, rồi tới lệnh mua thông thường. Trong nhóm thị trường hiện tại chưa có producer nào tạo nghĩa vụ hợp đồng; báo cáo không coi hệ thống hợp đồng là đã hoàn thành.
- Mỗi lệnh hợp lệ được tính vào tổng cầu tháng đúng một lần. Phần thiếu hàng được ghi vào lượng cầu chưa đáp ứng. Lệnh bị từ chối hoặc bị hủy không làm tăng cầu.
- Cuối ngày 30, 60, ... giá dùng công thức đã duyệt: `q = clamp(U/D + (max(1,D)-S)/max(1,D), -1, 1)`, `r = clamp(0.10*q, -0.10, 0.10)`, sau đó làm tròn nửa lên `giá_cũ × (1+r)` và giới hạn vào khoảng 1 đến 1.000.000. `D` là cầu đã ghi nhận, `U` là lượng cầu thiếu, `S` là tồn kho cuối ngày. Nếu `D=0`, giá không đổi. C++ dùng phép toán số nguyên có kiểm tràn.
- Cấu hình mới lưu phiên bản luật chợ, độ nhạy giá 10%, giới hạn đổi giá mỗi tháng 10%, sàn giá 1 và trần giá 1.000.000. Save schema 26 ghi lệnh, quỹ ký gửi, nguồn tiền của lệnh, số liệu tháng và mốc ngày/tháng. Save schema 7–25 được nạp với luật giá lịch sử và không tự bật lệnh mới.

## Lỗi hạch toán đã tìm và sửa

Kiểm tra không có hàng đã bắt được một lỗi thật: trước sửa, nếu lệnh đến ngày quyết toán mà không nhận được món nào, engine xóa lệnh và xóa khoản giữ khỏi sổ ký gửi nhưng không cộng lại tiền cho người mua. Tổng tiền của thế giới vì thế giảm. Bản sửa hoàn lại cả phần giữ dư khi lượng giao là 0; test gọi đúng trường hợp này và kiểm tra tổng tiền không đổi.

Kiểm tra tiếp theo rút hết số dư quỹ hộ vào khoản giữ. Trường hợp biên này phát hiện tài khoản hộ bằng 0 còn bị lưu trong bản đồ số dư, trong khi luật sổ sách yêu cầu bỏ tài khoản đã cạn. Cả lệnh thường và lệnh lương thực nội bộ nay xóa mục quỹ hộ khi số dư bằng 0. Test dùng lệnh nạp quỹ hộ thật, đặt lệnh mua dùng toàn bộ quỹ rồi kiểm tra hoàn tiền.

## Bằng chứng chạy trên Windows

Các lệnh được chạy trực tiếp trên executable của target; không chạy toàn bộ `ALL_BUILD` hoặc toàn bộ CTest sau từng thay đổi.

| Nhóm kiểm tra | Release | Debug | Điều được xác nhận |
|---|---:|---:|---|
| Tính phân bổ và giá tháng độc lập | 39/39 | 39/39 | Ưu tiên, chia phần thiếu hàng, phân xử phần dư, giới hạn giá, làm tròn và trường hợp số lớn |
| Cấu hình gameplay | 40/40 | 40/40 | Đọc cấu hình mới, từ chối miền sai và giữ tương thích cấu hình cũ |
| Chuỗi lệnh mua riêng | 33/33 | 33/33 | Giữ tiền, nạp từ quỹ hộ, hoàn khi không có hàng, giao hàng thật, hủy, từ chối hủy lặp, save/load trước quyết toán, từ chối save có ký quỹ lệch, migration schema 25, đổi giá cuối tháng và không tính tháng lần hai sau khi tải |
| Realm core | 18.834 kiểm tra | 18.834 kiểm tra | Hồi quy Realm rộng gồm tiền, kho, lịch, save/load, migration và chu trình nhân vật |
| Campaign M4 | 281/281 | Chưa chạy | Tích hợp core giữ được hạch toán Campaign; workload đi kèm là 10.000 người trong 102 ngày, khoảng 401 ms ở lượt Release cuối. Đây là smoke workload, không phải benchmark chuẩn |
| Court | 565 kiểm tra | Chưa chạy | Hồi quy quyền và sổ sách court sau thay schema |
| Land title | 88 kiểm tra | Chưa chạy | Hồi quy quyền sở hữu đất và migration |
| Household fund | 36 kiểm tra | Chưa chạy | Hồi quy quỹ hộ, quyền rút/nạp và save/load |
| Schedule | 3.717 kiểm tra | Chưa chạy | Hồi quy lịch, lưu và các save cũ |

Oracle giá tháng trong test riêng không gọi lại hàm tính sản phẩm để tự xác nhận chính nó: test đặt một đơn thiếu đúng một món, lấy tồn kho cuối ngày 30 và tính trực tiếp biểu thức nhỏ bằng số nguyên, rồi so với giá core. Test cũng lưu thế giới sau khi đổi giá, nạp lại, đi thêm một ngày và xác nhận giá không đổi lần nữa.

Target runner và nội dung nguồn được ghi trong [manifest của nhóm thị trường](../../artifacts/m5-demand-orders-monthly-price-2026-10-22-manifest.json). Source đã được tự review theo kế hoạch và các oracle riêng; không có reviewer độc lập trong lượt này.

## Giới hạn còn lại và bước kế tiếp

- Chưa nối lệnh mua với Python, native runtime hoặc UI Godot; chưa chạy replay Python/Godot trên source này. Những consumer đó phải được kiểm ở mốc tích hợp khi API được nối vào.
- Chưa có bên tạo nghĩa vụ hợp đồng bảo đảm hàng; mức ưu tiên hợp đồng chỉ là điểm mở rộng cho nhóm hợp đồng tương lai.
- Đây chưa phải vận tải quốc gia, 16 nhóm hàng, learned policy hay hoàn tất M5. Lệnh chỉ mua kho vùng hiện tại; hàng chưa được chuyển qua vùng.
- Nhóm tiếp theo là vận tải quốc gia. Dữ liệu tuyến theo cạnh atlas, chiều đi, thời gian, sức tải/ngày, phí và bên nhận phí sẽ theo quyết định trong IMPLEMENTATION_PLAN; không lấy thông số Campaign M4 làm luật mặc định cho mọi cạnh.

## Lệnh chính đã chạy

Build theo target core liên quan:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/windows --config Release --target polyec_market_manager_tests polyec_gameplay_config_tests polyec_market_order_tests polyec_realm_tests polyec_campaign_tests polyec_court_tests polyec_land_title_tests polyec_household_fund_tests polyec_schedule_tests
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/windows --config Debug --target polyec_market_manager_tests polyec_gameplay_config_tests polyec_market_order_tests polyec_realm_tests
```

Các runner tương ứng nằm trong `build/windows/core/Release/` và `build/windows/core/Debug/`. Chỉ bốn test cấu hình, quản lý chợ, lệnh mua và Realm được chạy ở Debug theo rủi ro số học/save của nhóm này; Campaign và UI/native không được tuyên bố đã kiểm ở Debug.
