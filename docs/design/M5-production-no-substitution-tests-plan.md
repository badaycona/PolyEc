# M5.2 — kiểm phép sản xuất không thay thế hàng cùng nhóm

## Mục tiêu

Kiểm tra rằng ba công thức đã được duyệt dùng đúng `GoodId`, không chấp nhận sản phẩm khác chỉ vì cùng `GoodGroup`. Nguồn quy tắc là bảng source/variant ở `M5-T2.4-gate4-production-plan.md` và working recipe trong `IMPLEMENTATION_PLAN.md` §3.1/§15.3.

## Phạm vi

- Chỉ thêm fixture/assertion vào `core/tests/test_production.cpp`.
- Beeswax không thay Fiber trong Cloth Workshop.
- Copper, Silver hoặc Gold không thay Iron trong Tool Workshop, kể cả khi Fuel còn đủ.
- Silk không thay Cloth trong Military-supplies Workshop, kể cả khi Tools còn đủ.
- Mỗi fixture có 16 tick làm tại đúng Establishment để chứng minh planner đã có một unit đủ điều kiện; kiểm output bằng 0 và hàng thay thế không bị tiêu thụ.
- Không đổi recipe, GoodId/GoodGroup, giá, nguồn hàng, transfer, action/observation, save schema hoặc UI.

## Kiểm tra ngay

Windows Release build target `polyec_production_tests`, chạy executable trực tiếp. Không chạy toàn bộ CTest/Debug vì chỉ đổi pure planner tests; các giao dịch Realm/save và adapter sẽ được kiểm tại mốc tích hợp M5.2 nếu production source/API thay đổi.

## Kết quả

Release target build thành công; direct runner đạt **43 checks**. Năm fixture đều cấp đủ 16 tick làm tại đúng xưởng. Planner ghi nhận một unit đã xét nhưng không tạo batch: Beeswax thay Fiber; Copper/Silver/Gold thay Iron khi Fuel vẫn đủ; Silk thay Cloth khi Tools vẫn đủ. Mỗi trường hợp giữ nguyên hàng thay thế và hàng phụ trợ, không trừ input, không cộng output. Không chạy full CTest hoặc Debug vì không đổi production source.

## Giới hạn

Phép thử chỉ khóa các hàng thay thế cho đầu vào của ba recipe đang chạy. Nó không tạo hoặc kiểm recipe cho Copper/Silver/Gold, Livestock/Horse/WarHorse, Silk, hay 22 sản phẩm deferred khác. Self-review; chưa có reviewer độc lập.
