# Kiểm tra Realm cho đủ ba công thức khởi đầu

Ngày 2026-10-22. Kế hoạch: [nối ba công thức khởi đầu qua Realm](../design/M5-realm-all-starter-recipes-integration-plan.md). Tham chiếu công thức: [IMPLEMENTATION_PLAN §15.3](../IMPLEMENTATION_PLAN.md#153-sản-xuất-và-nhu-cầu). Manifest ghi kết quả và hash: [manifest](../../artifacts/m5-realm-all-starter-recipes-integration-2026-10-22-manifest.json).

## Vì sao kiểm tra

Realm đã có test tích hợp công thức dệt vải: người làm gửi ca, thời gian trong game đi qua ngày, xưởng nhận Fiber và tạo Cloth, save/load giữa ca cho cùng kết quả như chạy liền mạch. Hai công thức còn lại mới có test ở planner. Lát này bổ sung kiểm tra Realm tương tự cho công cụ và quân nhu để phát hiện lỗi có thể chỉ xuất hiện khi lệnh, lịch làm việc, kho xưởng, thời gian và lưu/tải chạy cùng nhau.

## Cách dựng hai trường hợp

Mỗi trường hợp tạo một Realm riêng và một xưởng tư do chính người làm sở hữu. Xưởng thứ nhất thuộc ngành công cụ; xưởng thứ hai thuộc ngành quân nhu. Người làm được cấp quyền tại xưởng và gửi một lịch Work dài 32 tick.

Test tạo đầu vào bằng cách đọc save thật của Realm, điền đúng hai hàng vào kho xưởng và tăng cùng lượng ở ledger hàng của Province. Nhờ vậy, nguồn gốc hàng được cân bằng trong fixture và không có hàng hay tiền nào được tạo ra ngoài sổ. Cách làm này chỉ thiết lập điều kiện đầu vào; nó không tuyên bố thế giới hiện đã có cơ chế cung ứng những hàng ấy.

Sau 16 tick đầu, test lưu Realm, tải sang đối tượng mới rồi chạy 48 tick còn lại. Một bản Realm thứ hai chạy liên tục đủ 64 tick để làm đối chiếu độc lập.

## Kết quả cần thấy và đã thấy

- Với công thức **1 Iron + 1 Fuel → 1 Tools**, kho xưởng dùng hết Iron và Fuel, có đúng một Tools sau ngày đầu tiên.
- Với công thức **1 Tools + 1 Cloth → 1 MilitarySupply**, kho xưởng dùng hết Tools và Cloth, có đúng một MilitarySupply sau ngày đầu tiên.
- Cả hai ca ghi hai work unit của người làm và đúng ngày sản xuất. Không tạo thêm batch khi tải bản save sau sản xuất rồi chạy thêm ngày mà không có công mới.
- Realm tiếp tục sau save/load giữa ca cho fingerprint giống hoàn toàn với Realm chạy liên tục.
- Save ngay sau sản xuất được tải lại và serialize lại thành đúng cùng chuỗi byte; invariant kiểm tra tiền, hàng và cấu trúc toàn Realm đều đúng.
- Chủ xưởng rút đầu ra từ xưởng vào kho cá nhân, rồi gửi lệnh Sell đến chợ trong cùng vùng. Chợ nhận đúng một sản phẩm; người bán nhận số tiền theo quy tắc hiện hành là giá tham chiếu trừ một đồng nhưng tối thiểu một đồng; tổng tiền của Realm không đổi.
- Trường hợp thiếu đầu vào có Iron nhưng không có Fuel cho xưởng công cụ. Sau một ngày, Iron vẫn còn nguyên, Fuel và Tools bằng 0; hai work unit được ghi nhận đã xét theo luật hiện có, không có batch dở dang và invariant Realm đúng.

## Lệnh và kết quả chạy

Windows Visual Studio 18, cấu hình Release:

```powershell
cmake --build build/m5-core --config Release --target polyec_realm_tests
.\build\m5-core\core\Release\polyec_realm_tests.exe
```

Build target `polyec_realm_tests` thành công. Executable chạy trực tiếp và báo **20.829 checks passed**; kèm workload ổn định profile A gồm 10.000 người, 19.743 quan hệ thưa và 90 ngày. Đây không phải profile B 100.000 người.

Lần chạy đầu dừng ở assertion tạo xưởng vì test chọn một chủ thể công quyền không có authority phù hợp với vùng của fixture. Tôi đổi fixture sang xưởng tư của chính người làm, build lại và chạy lại target. Khi nối thêm bước bán, assertion đầu tiên dùng giá tham chiếu thay vì giá bán thực tế; oracle được sửa theo quy tắc hiện hành (giảm tối đa một đồng, tối thiểu một đồng), không sửa gameplay. Lần chạy cuối đạt như trên.

## Phạm vi chưa được xác nhận

Chỉ chạy Realm Release target và executable trực tiếp. Không chạy full CTest, Debug, Python, native runtime, GDExtension hoặc Godot ở lát này vì chỉ thay test C++ nội bộ. Không benchmark riêng hoặc kiểm tra learned policy.

Kết quả xác nhận ba công thức khởi đầu đều chạy được qua Realm với nguyên liệu đã có trong kho xưởng; công cụ và quân nhu còn được rút ra và bán qua lệnh Realm; kết quả giữa ca tiếp tục đúng sau save/load. Khi thiếu một trong hai nguyên liệu công cụ, nguyên liệu còn lại không bị tiêu dở dang. Kiểm tra này không xác nhận nguồn cung thực tế cho Iron, Fuel, Tools hay Cloth; cũng không xác nhận thị trường tổng quát, vận tải, sản xuất theo mùa, mặt hàng ngoài ba công thức, profile B hoặc M5 hoàn tất. Các giới hạn này được giữ mở trong [CURRENT_TASK](../CURRENT_TASK.md).
