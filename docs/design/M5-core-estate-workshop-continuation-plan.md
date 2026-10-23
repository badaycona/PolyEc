# Kế hoạch M5 — tiếp tục ca làm sau khi người thừa kế nhận xưởng

Ngày 2026-10-22. Đây là phép kiểm tích hợp C++ cho chuỗi đã có luật trong `IMPLEMENTATION_PLAN.md` §2, §7 và hợp đồng T2.4 Gate 6b. Không thêm luật kế vị, lương hay sản xuất.

## Hành vi cần chứng minh

1. Một người trưởng thành duy nhất đủ điều kiện thừa kế đã có quyền làm việc tại xưởng của người sắp chết và lên lịch một ca 64 tick qua `ScheduleCommand`.
2. Người đó thực hiện ca 16 tick mỗi công việc, tổng cộng bốn công. Trong ngày chuyển giao, Realm xử lý cái chết, trả khoản vay từ hồ sơ, rồi chuyển xưởng cho người hưởng trưởng thành theo luật hiện hành.
3. Khi tiến một ngày qua `advance_ticks`, ca đã gửi vẫn được tính tại xưởng đó. Realm trả bốn đơn vị công từ tài khoản xưởng, không lấy tiền người thừa kế hoặc Province; đồng thời tiêu Fiber và tạo Cloth theo công thức hiện hành và capacity.
4. Title/xưởng có chủ mới, cash và quyền của người làm không bị reset; sổ công/lương và lượng hàng đúng. Tổng tiền và invariant được giữ.
5. Save sau ngày làm việc tải vào Realm mới với fingerprint và các record kinh tế giống hệt.

Người làm được ủy quyền trước khi thừa kế. Bài kiểm không giả định guardian có quyền quản lý tiền/xưởng của trẻ, không chia quyền xưởng cho nhiều người, không đổi model/action/observation/reward/save schema.

## Phạm vi sửa

- Chỉ thêm fixture và hàm test vào `core/tests/test_realm.cpp`; không đổi source gameplay hoặc API.
- Ghi plan, report, CURRENT_TASK/HISTORY và manifest sau khi kiểm.

## Kiểm ngay

- Build Release `polyec_realm_tests` và `polyec_worksite_payroll_tests` bằng `build/m5-core`.
- Chạy trực tiếp hai executable. Realm test bao gồm workload profile A 10.000 người/90 ngày, không phải profile B.
- Kiểm độc lập: ca Work thật được gửi trước lúc chủ chết; sau death/claim, `advance_ticks(64)` phải phát sinh tiền công và công thức sản xuất. Đối chiếu cash xưởng/người/vùng, inventory, paid units, attendance, output, invariant, tổng tiền và roundtrip save/load.
- Không rebuild Python/native/GDExt hay chạy Godot: không đổi source, API, schema hoặc binding kể từ mốc tích hợp R1–R4 vừa ghi; bước này chỉ thêm test C++. Adapter M5 gameplay đầy đủ vẫn là gate còn mở, không lấy kết quả cũ để nhận xét tính năng mới.
- Full core CTest vừa qua 19/19 trước khi thêm fixture-only; không lặp lại cho một thay đổi chỉ có test. Mốc ổn định tiếp theo vẫn có thể chạy lại suite khi có source gameplay cần xác nhận.
