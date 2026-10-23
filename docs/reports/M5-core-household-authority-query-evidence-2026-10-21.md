# Core refactor — household guardian and authority queries

Ngày 2026-10-21. Plan: [household query extraction](../design/M5-core-household-authority-query-plan.md); exact source checkpoint: [manifest](../../artifacts/m5-core-household-authority-query-2026-10-21-manifest.json). Codex tự review; không reviewer độc lập, không commit/push.

## Kết quả

- `HouseholdIndex` nhận hai truy vấn thuần `guardian(...)` và `authority(...)`; `Realm` vẫn sở hữu dữ liệu, derived index, public ID validation và chỉ delegate kết quả.
- Giữ nguyên luật hiện có: người lớn từ 18 tuổi game time, phải còn sống; guardian không phải chính minor; authority ưu tiên household head nếu đủ điều kiện; nếu không thì người lớn cao tuổi nhất. Thành viên bằng tuổi giữ tie-break ID thấp nhất do iteration tăng dần và phép so sánh strict `>`.
- Helper bỏ qua member ID ngoài vector để không dereference nếu derived index hỏng. Đường build/invariant Realm hiện tại tạo/kiểm ID hợp lệ.
- Không đổi save schema/token, command, binding, action, observation, reward, model hay gameplay. Không thêm persistent Guardianship hoặc quyền quản lý tài sản dependent.
- Thêm helper unit checks cho equal-age tie-break, guardian/authority đổi khi adult chết, household không còn adult, household head trưởng thành và child tự trưởng thành; Realm fixtures hiện có tiếp tục kiểm birth, membership thưa, load/save, guardian còn sống và household không có người lớn. Query được tính lại từ state hiện hành nên không để stale Guardianship relation.

## Gates đã chạy

- Release build + Realm/manager CTest: **2/2 pass**; sau khi bổ sung lifecycle assertions, Release `manager_tests` chạy lại.
- Debug build + Realm/manager CTest: **2/2 pass** (Realm 129,53 giây); sau khi bổ sung lifecycle assertions, Debug `manager_tests` chạy lại.
- Release full core CTest: **16/16 pass** (38,04 giây).
- Debug full core CTest: **16/16 pass** (310,76 giây).
- Scoped `git diff --check` pass.

Trong quá trình kiểm tra, một fixture ban đầu ghi nhầm field save và bị từ chối `Save role`; sau khi sửa đúng offset, Release pass. Một bản thử đặt thêm `Realm` lớn trong hàm test Realm gây stack overflow Debug (`0xC00000FD`); fixture đó được chuyển sang `manager_tests`, suite Realm Debug và full Debug sau đó đều pass. Đây là lỗi bố trí test fixture, không phải lỗi runtime helper.

## Giới hạn

Đây là trích xuất trách nhiệm truy vấn, không thực hiện chuyển hộ, kết thúc/đổi guardianship theo consent, lệnh giám hộ tài sản, quyền đất, thừa kế hoặc household dissolution. Các quyền này chưa có đủ working rule để triển khai trong lát này. M5 và core refactor tổng thể vẫn đang mở.
