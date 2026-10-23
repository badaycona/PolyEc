# Báo cáo M5.3: chặn kết hôn giữa anh chị em cùng cha hoặc mẹ

## Người chơi sẽ thấy gì

Realm từ chối kết hôn giữa hai người có chung cha hoặc mẹ, kể cả khi dữ liệu không có một cạnh `Sibling` nối trực tiếp hai người. Quy tắc anh chị em được xác định từ quan hệ cha/mẹ–con đã lưu, đồng thời các cạnh anh chị em tường minh cũ vẫn có hiệu lực. Quy tắc cấm tổ tiên–hậu duệ nhiều thế hệ vừa được kiểm tra ở lát trước tiếp tục dùng cùng bộ kiểm tra quan hệ.

Việc từ chối xảy ra ở cả hai bước. Trước tiên, bảng đánh dấu hành động hợp lệ (`action mask`) báo cho chính sách máy tính rằng không được chọn kết hôn với người đó. Sau đó, Realm kiểm tra lại khi nhận lệnh để dữ liệu cũ hoặc lệnh gửi trực tiếp cũng không vượt qua luật. Khi từ chối, mã đối chiếu trạng thái toàn Realm (`fingerprint`), tiền ký quỹ và quan hệ không đổi. Sau khi lưu rồi tải game, Realm dựng lại chỉ mục từ quan hệ đã lưu; khi Realm thêm quan hệ cha/mẹ–con mới trong phiên hiện tại, chỉ mục anh chị em cũng được cập nhật.

## Vì sao cần sửa

Kế hoạch gameplay ở `docs/IMPLEMENTATION_PLAN.md` §2 cấm kết hôn giữa anh chị em. Realm lưu quan hệ `Sibling` theo cả hai chiều, nhưng dữ liệu khởi tạo có thể chỉ nối từng cặp kế bên thành một chuỗi thay vì ghi đủ quan hệ giữa mọi cặp. Hai người ở hai đầu chuỗi vì vậy có thể không có quan hệ anh chị em trực tiếp trong dữ liệu dù cùng cha/mẹ. Trước thay đổi, bảng lựa chọn hợp lệ chỉ tra quan hệ trực tiếp và không suy ra đầy đủ anh chị em từ cha/mẹ chung.

Giờ Realm giữ hai bảng tra cứu được tính từ quan hệ gia đình: tra cha/mẹ của từng người và tra các người con của mỗi cha/mẹ. Khi gặp một quan hệ cha/mẹ–con, nó liên kết người con mới với những người con khác đã biết của cùng cha/mẹ. Quan hệ `Sibling` tường minh cũ vẫn được giữ. Hai bảng tra cứu không được ghi thêm vào file lưu; khi tải, Realm dựng lại chúng từ quan hệ gia đình trong file. Quan hệ tự trỏ từ một người về chính họ không tạo ra anh chị em.

## Các trường hợp đã kiểm tra

| Trường hợp | Kết quả |
|---|---|
| Hai người trưởng thành có chung cha/mẹ nhưng không có quan hệ anh chị em trực tiếp trong dữ liệu | Bảng lựa chọn hợp lệ chặn đề nghị; lệnh Realm từ chối và mã đối chiếu trạng thái không đổi. |
| Bổ sung một cạnh con → cha/mẹ cho người thứ ba trong cùng phiên | Fixture C++ gọi chính phương thức cập nhật chỉ mục mà pha sinh con sử dụng; hành động kết hôn với người con đã có bị chặn. |
| Lưu rồi tải cặp có cha/mẹ chung | Realm tải lại và bảng lựa chọn hợp lệ tiếp tục chặn. |
| Anh chị em được lưu bằng quan hệ `Sibling` tường minh | Kiểm tra cũ vẫn chặn. |
| Cha/mẹ–con trực tiếp và ông/bà–cháu | Các kiểm tra hôn nhân đã có trong bộ Court tiếp tục pass. |
| Người không có quan hệ bị cấm, chấp thuận và ký quỹ | Đường hợp lệ hiện có tiếp tục được kiểm trong Court. |

Fixture chung cha/mẹ cố ý không tạo cạnh `Sibling` giữa hai người đang thử, để phép kiểm phát hiện đúng lỗ hổng trước đây. Fixture cập nhật tăng dần được tạo riêng trong test: nó thêm cạnh mới rồi gọi phương thức riêng của Realm dùng để lập chỉ mục; nó không giả nhận đã chạy một ca sinh con đầy đủ. Bộ Realm riêng vẫn kiểm pha sinh con thật bổ sung quan hệ cha/mẹ–con, nhưng không khẳng định chính ca đó sinh ra anh/chị/em để kiểm tra. Các cạnh được lưu theo hướng con → cha/mẹ, cùng hướng dùng trong Realm.

## Bằng chứng kiểm tra

Bộ kiểm tra luật hôn nhân Court trên Windows Release được biên dịch riêng, rồi chạy trực tiếp:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/windows --config Release --target polyec_court_tests
& .\build\windows\core\Release\polyec_court_tests.exe
```

Kết quả: mã thoát `0`; **565 phép kiểm đạt**.

Do thay đổi chỉ mục còn đi qua sinh con, khởi tạo và save/load, Realm Release được build và chạy trực tiếp:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/windows --config Release --target polyec_realm_tests
& .\build\windows\core\Release\polyec_realm_tests.exe
```

Kết quả: mã thoát `0`; **21.081 phép kiểm đạt**. Chương trình này còn chạy một thế giới mô phỏng quy mô A gồm 10.000 người, 19.743 quan hệ và 90 ngày. Đây chưa phải quy mô B hoặc phép đo hiệu năng toàn chu trình.

Vì thêm một bảng riêng vào kiểu dữ liệu C++ `Realm`, tôi biên dịch lại các phần C++ sử dụng trực tiếp kiểu này: `polyec_manager_tests`, thư viện suy luận native `polyec_ai` và chương trình kiểm tra tương thích runtime `polyec_realm_contracts`. Bộ manager chạy thành công. Chương trình runtime nhận ba model thử nghiệm v6 theo đúng lệnh trong cấu hình CTest và đạt **24 phép kiểm**; nó từ chối bộ model cũ đúng như dự kiến. Workspace chưa có bộ model v11 đã huấn luyện tương thích, nên không chạy mạng nơ-ron để chọn hành động và không đánh giá chất lượng chính sách đã học.

Cấu hình `build/windows` có `POLYEC_BUILD_PYTHON=OFF`, nhưng workspace cũng có cấu hình tích hợp riêng `build/m5-python-ai` với Python bật. Tôi build lại `polyec_env` và `polyec_realm_transition_replay` trên source hiện tại, đặt `PYTHONPATH` trỏ đúng thư mục Release rồi chạy:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-python-ai --config Release --target polyec_env polyec_realm_transition_replay
$env:PYTHONPATH = (Resolve-Path 'build/m5-python-ai/Release').Path
& 'C:\Users\Hi\AppData\Local\Programs\Python\Python312\python.exe' ai/tests/realm_binding_checks.py --replay build/m5-python-ai/core/Release/polyec_realm_transition_replay.exe
```

Kết quả: **273 phép kiểm**. Cùng một chuỗi mô phỏng 64 ngày được chạy trong C++ và qua extension Python gọi chính C++ core; các mã đối chiếu trạng thái, tiền, dữ liệu quan sát và bảng hành động hợp lệ trùng nhau. Phép kiểm cũng xác nhận lệnh sai trong batch bị từ chối mà không thay đổi thế giới. Bản build cung cấp 1.676 giá trị quan sát và 129 lựa chọn hành động. Đây là kiểm tra đường gọi Python–C++, không chạy mạng nơ-ron hoặc chứng nhận chính sách đã học.

Phần mở rộng C++ nối game với Godot (`GDExtension`) được biên dịch riêng bằng target `polyec`, kiểm tra rằng giao diện native vẫn biên dịch với kiểu `Realm` hiện tại. Sau đó Godot 4.5.1 chạy không mở cửa sổ giao diện, dùng `realm_worksite_smoke.gd` và đạt **5/5 phép kiểm tương thích model**. Nó nhận định dạng lưu hiện tại và từ chối bộ model pilot cũ. Phép kiểm không thao tác màn hình cưới, không kiểm quan hệ anh chị em qua giao diện và không gọi chính sách đã học. Không chạy toàn bộ CTest hoặc Debug; thay vào đó đã build và chạy riêng các bộ C++ chịu ảnh hưởng, một replay Python và một smoke GDExtension/Godot giới hạn.

## Giới hạn còn mở

Sửa đổi cấm anh chị em có chung ít nhất một cha/mẹ và các cặp đã được nối bằng quan hệ `Sibling`. Nó không tự cấm anh/chị/em họ, không thêm luật nhận con nuôi và không thay luật thừa kế. Không đổi cấu trúc file lưu, số lựa chọn hành động, số giá trị quan sát, phiên bản tương thích model hoặc API Python; bố cục nội bộ của kiểu C++ `Realm` có đổi vì thêm bảng tra cứu. Tôi tự đối chiếu mã và fixture, chưa có người review độc lập; không commit hoặc push.

Mã băm và kích thước file nguồn, kiểm thử, tài liệu và binary đã build nằm trong [manifest của lượt này](../../artifacts/m5-core-marriage-common-parent-sibling-2026-10-22-manifest.json). Kế hoạch theo phạm vi nằm tại [M5-core-marriage-common-parent-sibling-plan.md](../design/M5-core-marriage-common-parent-sibling-plan.md).
