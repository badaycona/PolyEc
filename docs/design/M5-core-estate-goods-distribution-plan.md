# Core — chuyển hàng hóa di sản sau khi trả nợ

Ngày 2026-10-21; cập nhật ranh giới cuối pha cư dân ngày 2026-10-22. Bước tiếp theo của [CURRENT_TASK](../CURRENT_TASK.md), theo `IMPLEMENTATION_PLAN.md` §2: sau nghĩa vụ, phần còn lại của di sản được chia cho cùng nhóm người hưởng hợp pháp đã dùng cho tiền mặt.

## Trước và sau

- Trước: death transition lưu hàng cá nhân trong `EstateCase.inventory`; cash được quyết toán và phân phối, nhưng hàng còn trong hồ sơ.
- Sau: với từng `GoodId`, số lượng trong hồ sơ được chia đều giữa cùng nhóm người thân còn sống đầu tiên. Số lượng nguyên dư thuộc người có ID thấp nhất. Nếu không có người hưởng, toàn bộ hàng chuyển vào `Province::stock` của vùng đã ghi trong hồ sơ. Không định giá, mua bán hoặc tiêu hao hàng.
- Sau chuyển, số tồn của hồ sơ bằng 0; hồ sơ tiếp tục giữ ID để theo dõi đất/quyền sử dụng đất, khoản vay và xưởng. Người nhận sở hữu hàng trong inventory cá nhân. Hành vi AI và UI chưa được đổi ở lát này.
- Pha xử lý cư dân thu thập mọi trường hợp tử vong trong ngày trước khi phân phối. Realm lập hồ sơ và đánh dấu tất cả những người này đã chết, sau đó mới quyết toán nợ/chia tài sản. Người thừa kế là người còn sống tại ranh giới cuối pha: họ không dùng tiền thừa kế để mua thức ăn trong cùng ngày, và người cũng chết trong ngày không được tính là người hưởng chỉ vì đứng sau người chết khác theo ID.

## Ownership và file

- `core/include/polyec/managers/estate_manager.hpp`: mở rộng prepare/apply giao dịch phân phối để stage cả cash và inventory, recipient cash/inventory, public-fund balance và regional public stock; mọi read-set được kiểm trước lần ghi đầu tiên.
- `core/src/realm_lifecycle.cpp`: truyền Province ledger vào bước phân phối sau khi trả claim.
- `core/tests/test_managers.cpp`: từng good, chia đều, remainder theo ID, fallback kho công, stale inventory/stock, overflow và reject atomic.
- `core/tests/test_realm.cpp`: death pipeline chuyển nhiều loại hàng cho spouse, bảo toàn goods invariant và save/load.
- Không đổi schema: tồn hồ sơ, Person, Province, goods ledger và quy tắc save đã chứa tất cả số dư. Không đổi action/observation/reward/model/binding.

## Kiểm ngay và kết thúc lát

- Build Release manager/Realm test targets, chạy trực tiếp cả hai.
- Realm test phải xác nhận trước/sau mỗi loại GoodId, invariant hàng hóa và continuation save/load. Đây là chuỗi C++ liên quan cần chạy ngay.
- Full Release CTest, Python migration/replay, GDExt/Godot smoke được chạy ở mốc estate/ownership ổn định. Không chạy lại bộ test cũ không bị ảnh hưởng.
- Lát đóng khi mỗi loại hàng được chuyển đúng một lần, tổng lượng giữ nguyên, reject không ghi dở và save/load tiếp tục chính xác. Đất, xưởng, đồng sở hữu, giám hộ, policy và package vẫn ngoài lát này.

Review: Codex tự review sau diff/evidence; chưa có reviewer độc lập.
