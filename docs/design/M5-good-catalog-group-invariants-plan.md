# M5.2 — kiểm tính đầy đủ của danh mục nhóm hàng

## Mục tiêu

Đảm bảo danh mục C++ luôn biểu diễn đủ 16 nhóm hàng đã liệt kê trong `IMPLEMENTATION_PLAN.md` §15.2; mỗi sản phẩm có ID đúng vị trí, một nhóm hợp lệ, nhãn không rỗng và không trùng. Kiểm này bảo vệ cấu trúc của danh mục, không thêm luật sản xuất, thay thế hàng, giá hay thị trường.

## Phạm vi

- Tệp core: `core/include/polyec/good_catalog.hpp`.
- Kiểm tra trực tiếp qua target người dùng danh mục: `core/tests/test_production.cpp`, target `polyec_production_tests`.
- Validator compile-time phải từ chối catalog thiếu nhóm, nhóm ngoài miền, nhãn rỗng/trùng, ID lệch vị trí, giá tham chiếu không dương hoặc legacy mapping sai.
- Runtime test đối chiếu số sản phẩm trong mỗi nhóm với 27 dòng catalog hiện hành, kiểm đủ 16 nhóm và nhãn duy nhất.
- Không sửa recipe, tồn kho, giá, action slots, observation, reward, save schema, binding hoặc UI.

## Kiểm tra ngay

Windows Release: build riêng `polyec_production_tests`, sau đó chạy executable trực tiếp. Đây là thay đổi validator/catalog test; không chạy ALL_BUILD hoặc toàn bộ CTest. Tại mốc M5.2 tích hợp các luật hàng hóa vào Realm/Campaign/Python, chạy lại regression của transaction, sản xuất, migration và adapter theo source thay đổi.

## Kết quả

Target Windows Release `polyec_production_tests` build thành công; runner trực tiếp đạt **28 checks**. Validator compile-time kiểm đủ 16 nhóm, các ID liên tiếp theo catalog, nhóm hợp lệ, tên có nội dung và không trùng, giá tham chiếu dương và bảy ánh xạ legacy hợp lệ/không trùng. Runtime test kiểm số sản phẩm theo nhóm lần lượt là `1, 1, 1, 1, 1, 1, 3, 2, 2, 3, 1, 2, 1, 1, 1, 5`; tổng đúng 27. `polyec_core` được build lại cùng target do header catalog được dùng chung. Không chạy full CTest, Debug hoặc Python/native/Godot; không đổi API/binding/save/action/observation/reward.

## Giới hạn báo cáo

Kết quả chỉ xác nhận catalog có cấu trúc đủ 16 nhóm và giữ cardinality sản phẩm hiện hành. Nó không chứng minh mọi hàng đã có nguồn sản xuất, giá cân bằng, cung ứng thị trường hoặc chu trình M5.2 hoàn tất. Tự review; chưa có reviewer độc lập.
