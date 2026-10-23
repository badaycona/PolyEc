# Báo cáo: chuỗi lệnh thật từ mua nguyên liệu đến bán sản phẩm qua chợ

## Kết quả nhìn từ gameplay

Một cư dân có thể mua nguyên liệu tại chợ bằng lệnh Realm, lập một xưởng dệt và dùng lệnh chuyển hàng để đưa nguyên liệu của mình vào kho xưởng. Sau đó chính cư dân đó gửi lịch làm tại xưởng, tiến thời gian trong game và được trả lương bằng tiền trong tài khoản xưởng. Khi công việc đủ điều kiện, core tiêu Fiber và tạo Cloth theo công thức đã có. Sau khi lưu giữa ca và tải lại vào Realm mới, diễn biến còn lại cho kết quả giống bản chạy liên tục. Chủ xưởng rút Cloth, bán tại chợ; một cư dân trưởng thành khác trong cùng vùng mua sản phẩm và khoản thuế mua hàng được hạch toán vào ngân khố.

Đây là một bài kiểm C++ điều khiển bằng lệnh thật. Nó không chứng minh AI đã tự chọn được các lệnh này, không thêm nguồn hàng mới và không tạo một hệ thống thị trường hoặc vận tải liên vùng.

## Chuỗi và số liệu được đối chiếu

Test bắt đầu với Realm seed 71, hai vùng hoạt động và 128 người; actor 65 làm chủ xưởng trong vùng hiện tại của mình. Hàng được mua từ tồn kho Province bằng `Intent::Buy`; sau khi mua, test kiểm tra việc chuyển cùng hai Fiber khỏi kho cá nhân sang kho xưởng qua `EstablishmentTransferCommand`. Xưởng được tạo bằng command với mức cấp vốn hiện hành. Việc mua và chuyển nội bộ không làm thay đổi tổng tiền hoặc tổng lượng Fiber của Realm.

Actor gửi một lịch Work tại đúng xưởng, dài 32 tick theo fixture hiện có. Test tiến Realm tới 64 tick như chuỗi chuyển xưởng hiện tại; sản xuất chỉ chạy ở pha ngày theo luật đang có. Bài kiểm lưu một nhánh sau 16 tick, tải vào đối tượng Realm mới rồi chạy thêm 48 tick; fingerprint cuối khớp bản chạy liên tục. Ở mốc cuối, xưởng còn một Fiber và có một Cloth: recipe đã xét hai đơn vị công mới, nhưng capacity mặc định chỉ cho phép một batch trong ngày. Test xác nhận tiền công đã được trả từ quỹ xưởng, tổng tiền/invariant còn đúng.

Chủ xưởng rút một Cloth qua lệnh chuyển hàng. Trước khi bán, test chụp tồn kho/giá/số dư Province; giá bán kỳ vọng được tính là `max(1, giá vùng − 1)`. Sau lệnh Sell, tiền người bán tăng đúng số đó, tiền doanh nghiệp vùng giảm đúng số đó và kho chợ tăng một Cloth.

Test tìm một cư dân khác còn sống, trưởng thành, đang có mặt trong cùng vùng, thuộc nhóm Civilian và đủ khả năng mua theo giá hiện tại. Giá mua tổng là `giá vùng + 1`. Thuế dùng đúng công thức số nguyên: cộng phần dư thuế trước đó vào `giá mua tổng × thuế suất`, chia nguyên cho 100 để lấy tiền thuế, và giữ phần dư modulo 100 cho lần tính sau. Test so từng thay đổi của tiền/người mua, số Cloth trong kho cá nhân, tồn kho chợ, tiền doanh nghiệp, ngân khố, biên nhận thuế, phần dư thuế và tổng tiền. Oracle lấy giá và thuế suất thật ngay trước command, không thay bằng giá giả lập.

Sau giao dịch cuối, test lưu Realm và tải sang một Realm mới. Fingerprint toàn trạng thái và invariant phải trùng với trạng thái trước khi lưu.

## Bằng chứng chạy

Đã build riêng target Windows Release `polyec_establishment_transfer_tests` và chạy runner trực tiếp:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/windows --config Release --target polyec_establishment_transfer_tests
& .\build\windows\core\Release\polyec_establishment_transfer_tests.exe
```

Kết quả: build thành công; runner thoát mã `0`, báo **217 phép kiểm**. Lần thử đầu dùng lệnh `cmake` không kèm đường dẫn, nhưng executable không có trong PATH của phiên PowerShell này nên lệnh không chạy. Sau đó dùng CMake đi cùng Visual Studio tại đường dẫn đầy đủ ở trên; đây là lần build/run có kết quả được báo cáo.

Chỉ `core/tests/test_establishment_transfers.cpp` được sửa trong code. Không đổi gameplay source, save schema, API/binding, action/observation/reward, Python, native, Godot hoặc model. Vì vậy chỉ kiểm tra runner chuyển hàng xưởng bị ảnh hưởng; không chạy full CTest, Debug, replay Python/Godot hoặc toàn bộ app. Những kiểm tra tích hợp cũ không được dùng thay cho kết quả mới này.

## Phần đã được kiểm và phần còn thiếu

Đã kiểm đường C++ thật nối mua nguyên liệu → tạo/cấp vốn xưởng → chuyển nguyên liệu → lịch làm và thời gian → trả lương/sản xuất → lưu/tải giữa ca → rút hàng → bán → mua → thuế → lưu/tải hoàn tất. Các bước dùng cùng command/transition C++ và giữ invariant tiền/hàng.

Chưa có giá tháng điều chỉnh theo nhu cầu/tồn kho mục tiêu; tài liệu chưa chốt các hệ số và giới hạn biến động để triển khai đúng mà không tự đặt balance. Chưa có nhu cầu cạnh tranh nhiều người theo vòng ưu tiên, hợp đồng được bảo đảm hàng, giao dịch đang vận chuyển giữa vùng, nguồn sản xuất cho các hàng còn deferred, UI chợ 27 vùng hoặc quyết định do learned policy. Bài test này không đóng M5.2/M5.4, không phải Python parity, không phải nghiệm thu M5. Tác vụ tự review; không có reviewer độc lập, không commit/push.

Kế hoạch: [M5-command-driven-workshop-to-market-chain-plan](../design/M5-command-driven-workshop-to-market-chain-plan.md). Hash và kích thước file được ghi trong [manifest](../../artifacts/m5-command-driven-workshop-to-market-chain-2026-10-22-manifest.json).
