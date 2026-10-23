# M5.2 — biến thể cùng nhóm không thay đầu vào công thức

Ngày 2026-10-22. Plan: [production no-substitution tests](../design/M5-production-no-substitution-tests-plan.md). Quy tắc nguồn nằm trong [T2.4 Gate 4](../design/M5-T2.4-gate4-production-plan.md) và bảng hàng hóa của [IMPLEMENTATION_PLAN §15.2–15.3](../IMPLEMENTATION_PLAN.md).

## Hành vi được kiểm tra

Ba recipe đã có trong C++ yêu cầu đúng sản phẩm đầu vào, không chỉ đúng nhóm hàng:

| Xưởng | Đầu vào chính xác theo công thức | Trường hợp được thử thay thế | Kết quả |
|---|---|---|---|
| Dệt vải | Fiber → Cloth | Beeswax (cùng nhóm xơ sợi) | Không tạo Cloth, không tiêu Beeswax |
| Dụng cụ | Iron + Fuel → Tools | Copper + Fuel; Silver + Fuel; Gold + Fuel | Không tạo Tools, không tiêu kim loại thay thế hoặc Fuel |
| Quân nhu | Tools + Cloth → MilitarySupply | Tools + Silk (cùng nhóm vải) | Không tạo MilitarySupply, không tiêu Tools hoặc Silk |

Mỗi fixture cấp cho actor 16 tick có mặt tại đúng cơ sở, tương ứng một work unit theo contract hiện tại. Vì vậy kết quả không batch không phải do vắng mặt hoặc thiếu thời gian. Planner đã xét unit đó, nhưng hàng cùng nhóm không đáp ứng đầu vào `GoodId` được yêu cầu. Tồn kho thay thế và hàng phụ trợ không đổi; ledger không ghi tiêu thụ hoặc sản lượng giả.

## Kiểm tra đã chạy

- Windows Visual Studio Release build riêng `polyec_production_tests`: exit 0.
- Chạy trực tiếp `build/m5-core/core/Release/polyec_production_tests.exe`: **43 checks** pass.
- `py -3.12 scripts/check-repository-links.py`: **1.164 tham chiếu nội bộ** pass.
- `git diff --check` scoped tới tệp văn bản của lát này pass.
- Chỉ sửa test planner và tài liệu. Không chạy full CTest, Debug, Python/native/GDExtension/Godot vì production code, save schema và binding không đổi.

## Điều chưa được chứng minh

Phép thử khóa đúng các đầu vào của ba recipe đang hoạt động. Nó không thêm recipe hoặc nguồn hàng cho đồng/bạc/vàng, tơ lụa, sáp ong, gia súc/ngựa chiến hay các sản phẩm deferred khác; cũng không chứng minh market supply hoặc toàn bộ M5.2. Không commit, push hoặc publish. Tự review; chưa có reviewer độc lập.
