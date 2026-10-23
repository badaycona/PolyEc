# Báo cáo M5.4: tìm chuỗi liên kết địa lý đã khai báo trong atlas

## Người chơi hoặc giao diện sẽ nhận được gì

Atlas đã có tên và ID cho 24 lãnh địa cùng ba khu triều đình, và đã lưu 31 đoạn nối giữa một số vùng. Core C++ giờ có thể trả lời câu hỏi hẹp: “Có chuỗi liên kết địa lý nào đã khai báo, đi đúng chiều, từ vùng A đến vùng B không?” Kết quả là danh sách ID vùng đi qua.

Đây là nền dữ liệu cho bản đồ và các kiểm tra tiếp theo. Nó không nói rằng một đoàn hàng có thể đi ngay: dữ liệu chưa có thời gian, sức tải, phí, thời tiết, tình trạng đóng/mở, an ninh hoặc quyền kiểm soát cho các liên kết quốc gia.

## Cách truy vấn hiểu dữ liệu

Mỗi liên kết mang hai thông tin độc lập: phương thức đi lại có thể đã biết hoặc chưa xác định; hướng có thể một chiều, hai chiều hoặc chưa xác định. Truy vấn đường dựa vào hướng:

- Đoạn một chiều chỉ đi từ đầu đến cuối đã ghi. Ví dụ các đoạn sông lớn cho phép đi xuôi theo thứ tự trong atlas; không được tự đảo chiều.
- Đoạn hai chiều có thể được duyệt theo cả hai hướng. Điều này áp dụng cả khi phương thức đi lại của một tuyến đông/bắc/tây chưa xác định: ta biết hai vùng được nối trong sơ đồ, nhưng chưa biết đi bằng đường bộ, sông hay phương thức nào.
- Đoạn chưa xác định hướng bị bỏ qua. Năm liên kết tới hoặc giữa các khu triều đình hiện thuộc nhóm này, nên truy vấn không tự đoán hướng của chúng.

Tìm đường dùng duyệt theo lớp (BFS: lần lượt mở rộng mọi vùng cách điểm xuất phát một liên kết, rồi hai liên kết, v.v.). Vì vậy đường trả về có ít đoạn nối nhất theo topology hiện có. Số lượng tối đa là 27 ID vùng; trường hợp nguồn và đích giống nhau trả về đúng một ID. ID ngoài 0–26 hoặc đích chưa nối được trả kết quả không tới.

## Các trường hợp đã kiểm tra

| Trường hợp | Kết quả |
|---|---|
| Đường xuôi theo sông lớn từ vùng 0 đến vùng 8 | Trả chuỗi vùng `[0, 2, 1, 8]`, đúng ba đoạn sông đã khai báo. |
| Đảo chiều riêng một đoạn sông | Helper xác nhận đoạn 0→2 không cho đi ngược 2→0. Mọi cạnh trong chuỗi trả về đều được kiểm lại theo hướng đã khai báo. |
| Đoạn đường cải thiện 5↔4 | Helper cho phép cả 5→4 và 4→5. Khi đánh dấu đoạn này tạm đóng, truy vấn tìm đường vòng 5→19→18→17→16→4 qua tuyến bắc. |
| Đóng mọi liên kết trong một truy vấn | Trả kết quả không tới; không trả lại tuyến đã bị caller đánh dấu đóng. |
| Liên kết triều đình chưa rõ hướng | Không được dùng làm bước đi; từ ID 24 tới ID 0 hiện trả “không có đường xác định”. |
| Đi từ một vùng tới chính nó | Trả thành công với một ID vùng duy nhất. |
| ID 27 | Bị từ chối vì atlas chỉ có ID 0–26. |

Các dữ liệu cũ cũng được kiểm lại trong cùng target: đủ 27 vùng, sáu tổng nhóm địa lý, 31 liên kết, 24 phân loại đặc sản, dân số bối cảnh 4.185 nghìn người và diện tích 212.000 km². Dân số trong atlas là bối cảnh, không được biến thành số cá thể NPC.

Test còn hỏi đủ 24 × 24 cặp điểm xuất phát/đích trong 24 lãnh địa chư hầu. Cả 576 cặp đều có đường nối theo những liên kết có hướng rõ, và test xác nhận lại từng đoạn của từng đường không đi ngược cạnh một chiều. Nó cũng đánh dấu liên kết 5↔4 tạm đóng trong một truy vấn, xác nhận đường trực tiếp biến mất nhưng vẫn có đường vòng qua tuyến bắc. Việc đóng chỉ nằm trong dữ liệu đầu vào của truy vấn; nó không sửa atlas toàn cục hay save. Điều này chỉ xác nhận các vùng chư hầu hiện nối được trong sơ đồ tĩnh; ba vùng triều đình chưa được đưa vào kiểm tra thông đường vì hướng liên kết của chúng chưa được mô tả.

Khi tất cả 31 liên kết bị đánh dấu không khả dụng, cùng phép truy vấn trả về không có đường giữa vùng 0 và vùng 8.

## Kiểm tra đã chạy

Build và chạy trực tiếp target kiểm atlas trong cấu hình Windows Release:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/windows --config Release --target polyec_world_atlas_tests
& .\build\windows\core\Release\polyec_world_atlas_tests.exe
```

Build thoát mã `0`, executable cũng thoát mã `0`; dòng kết quả xác nhận 27 vùng, 31 liên kết tối thiểu, truy vấn topology có hướng và catalog nền. Target tự kiểm bằng mã thoát theo từng nhóm điều kiện, không in tổng số phép so sánh riêng.

## Những phần chưa được kiểm hoặc chưa có

Đây chỉ là khả năng tìm chuỗi nối trong dữ liệu atlas tĩnh. Truy vấn chưa kiểm loại địa hình, sông có thể qua theo mùa, sức tải, phí, thời gian, độ an toàn, chủ quyền, tuyến bị hỏng hoặc hàng đang chiếm chỗ. Nó không gọi `Realm`, không tạo giao dịch, không thay đổi save hay trạng thái người chơi, và chưa được nối vào native map/Godot.

Do chỉ thêm helper inline và test atlas, build/kiểm tra trực tiếp target C++ tiêu thụ dữ liệu này là đủ cho lượt hiện tại. Không chạy full CTest, Debug, Python, native app, GDExtension hay Godot. Bảng nối địa lý đã dùng trong phép tính này chưa phải bản đồ quốc gia vận hành và chưa đóng M5.4. Tự review, không có reviewer độc lập; không commit/push. Kế hoạch: [M5-atlas-explicit-topology-query-plan.md](../design/M5-atlas-explicit-topology-query-plan.md). Manifest gồm mã nguồn, test, tài liệu và bản build: [m5-atlas-explicit-topology-query-2026-10-22-manifest.json](../../artifacts/m5-atlas-explicit-topology-query-2026-10-22-manifest.json).
