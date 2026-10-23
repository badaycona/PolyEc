# Core refactor — phân giải thứ tự người hưởng di sản

## Nguồn quy tắc

`IMPLEMENTATION_PLAN.md` §2 quy định trả nghĩa vụ từ di sản trước, sau đó phân phần còn lại đều cho con hợp pháp, nếu không có thì phối ngẫu, cha mẹ, anh chị em; không có người hưởng thì vào quỹ di sản công có sổ. Lát này chỉ thực hiện phân giải nhóm người hưởng còn sống từ các quan hệ typed đã lưu, không phân phối tài sản.

## Phạm vi

- Thêm query thuần trong `EstateManager`, nhận ID người chết, vector Person và relation edges.
- Chọn đúng tier ưu tiên đầu tiên còn người sống; sắp ID tăng dần và loại trùng.
- ParentChild được đọc theo hướng `child -> parent`; Spouse và Sibling đọc hai chiều. Sibling cũng được suy ra khi hai người có cùng parent edge.
- Không đổi death transition, tài sản, nợ, quan hệ, save schema, binding, reward, action hoặc gameplay.
- Thay đổi nếu có sẽ giới hạn trong `estate_manager.hpp` và direct manager oracle.

## Invariants và kiểm chứng

ID/edge sai cấu trúc bị từ chối trước khi trả danh sách. Người chết, ID trùng và người đã chết không được đưa vào danh sách hưởng. Mỗi tier fallback chỉ khi tier trước không còn người sống. Test đủ từng tier, nhiều đồng hưởng, thứ tự xác định, cha/mẹ chung, người chết và không có người hưởng. Build Release/Debug manager target; full CTest Release.

Đây là phần chuẩn bị cho probate ledger; chưa chứng minh luật thừa kế đã chạy trong Realm. Codex tự review.
