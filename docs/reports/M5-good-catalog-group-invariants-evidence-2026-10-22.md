# M5.2 — kiểm tra danh mục đủ 16 nhóm sản phẩm

Ngày 2026-10-22. Plan: [kiểm tra invariant danh mục hàng](../design/M5-good-catalog-group-invariants-plan.md). Contract nguồn: danh sách 16 nhóm trong [IMPLEMENTATION_PLAN §15.2](../IMPLEMENTATION_PLAN.md) và 27 sản phẩm trong [plan Gate 3](../design/M5-T2.4-gate3-goods-catalog-plan.md).

## Thay đổi

Trước đây validator đã kiểm mỗi `GoodId` nằm đúng vị trí trong bảng, nhóm của sản phẩm nằm trong miền 0–15, giá tham chiếu dương và bảy ID hàng cũ ánh xạ hợp lệ. Validator chưa tự phát hiện một nhóm bị bỏ trống hoặc hai sản phẩm dùng chung một tên.

Giờ validator compile-time đếm sản phẩm theo từng nhóm và yêu cầu cả 16 nhóm đều có ít nhất một mặt hàng. Nó cũng từ chối nhãn null/rỗng và tên trùng. Test của target sản xuất kiểm chính xác số sản phẩm theo thứ tự nhóm: lương thực khô 1; thực phẩm bổ sung 1; muối 1; gỗ 1; nhiên liệu 1; sắt 1; kim loại màu/quý 3; xơ sợi 2; vải 2; gia súc/ngựa 3; công cụ 1; quân nhu/vũ khí 2; vật liệu xây dựng 1; giấy 1; dược liệu 1; hàng xa xỉ 5. Cộng lại là 27 sản phẩm.

Giữ nguyên ID, tên, nhóm, giá tham chiếu, mapping hàng cũ, công thức sản xuất, giao dịch, action slots, observation, reward, save, binding và UI. Không sản phẩm hay tài nguyên mới nào được tạo trong mô phỏng.

## Kiểm tra đã chạy

- Windows Visual Studio Release: build riêng `polyec_production_tests` thành công; vì header catalog nằm trong core, target cũng build lại `polyec_core`.
- Chạy trực tiếp `build/m5-core/core/Release/polyec_production_tests.exe`: **28 checks** pass.
- `py -3.12 scripts/check-repository-links.py`: **1.155 liên kết nội bộ** pass.
- Kiểm khoảng trắng giới hạn vào các tệp của lát này pass; Git có nhắc quy tắc chuyển CRLF/LF hiện hành của hai file docs, không báo dòng có khoảng trắng cuối.
- Không chạy full CTest, Debug, Python, native, GDExtension hoặc Godot vì không đổi API, binding, schema, action, observation, reward hay luật giao dịch.

## Phạm vi kết luận

Kết quả xác nhận bảng dữ liệu có đủ 16 nhóm và giữ đúng số mặt hàng đã khai báo. Nó không chứng minh các hàng đều đã có nguyên liệu, nhà sản xuất, mùa vụ, giá hiệu chỉnh, lượng cung ở chợ hoặc hành động học nghề. Các phần này vẫn cần contract M5.2 cụ thể. Đây là self-review; chưa có reviewer độc lập. Không commit, push hoặc publish.
