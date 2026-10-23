# Core refactor — tách truy vấn người giám hộ và thẩm quyền hộ

Ngày 2026-10-21. Lát kế tiếp cho M5.0/1 và core refactor. Bằng chứng: [report](../reports/M5-core-household-authority-query-evidence-2026-10-21.md); checkpoint source: [manifest](../../artifacts/m5-core-household-authority-query-2026-10-21-manifest.json).

## Trước → sau

Hiện `Realm` giữ chỉ mục thành viên hộ được suy ra từ Person, đồng thời chứa hai vòng lặp chọn người giám hộ của trẻ và người có thẩm quyền của hộ. Chuyển hai phép truy vấn thuần sang `HouseholdIndex`, nơi đã dựng nhóm hộ nhưng không sở hữu state thế giới. `Realm` tiếp tục là nơi duy nhất sở hữu Person và `households_`, kiểm tra ID công khai rồi gọi helper.

Không tạo Guardianship hay Household được lưu bền. Giữ nguyên hành vi: thành niên khi `age_days >= 18*365`; chỉ người lớn còn sống đủ điều kiện; trẻ vị thành niên không tự là người giám hộ của mình; người đứng đầu hộ được ưu tiên làm authority nếu đủ điều kiện, nếu không thì chọn người lớn tuổi nhất. Nếu nhiều người lớn bằng tuổi, chọn ID nhỏ nhất do danh sách thành viên tăng dần và chỉ thay kết quả khi tuổi mới lớn hơn (`>`).

## File và quyền sở hữu

- `core/include/polyec/managers/household_index.hpp`: thêm truy vấn thuần `guardian` và `authority`, nhận `const std::vector<Person>&` cùng `unordered_map` dẫn xuất.
- `core/src/realm.cpp`: thay vòng lặp chọn người bằng lời gọi helper; giữ validation `guardian(id)` qua `person(id)` và hành vi truy vấn authority hiện có.
- `core/tests/test_managers.cpp`: kiểm tie-break khi guardian/authority bằng tuổi. Realm tests hiện tại tiếp tục làm oracle tích hợp; không thêm Realm lớn vào stack frame `main` vốn đã lớn.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`, plan này và evidence report.

## Invariant và gate

- Helper chỉ đọc state được truyền vào. Không sửa Person, membership, tiền hay save; không tự chọn luật người giám hộ mới.
- Guardian và authority được tính lại theo tuổi, vòng đời và membership hiện tại mỗi lần truy vấn. Người lớn chết, trẻ thành niên hoặc đổi membership không để lại quan hệ suy ra đã cũ.
- ID Person công khai không hợp lệ vẫn giữ cách lỗi hiện tại của `person(id)`; household ID không biết trả 0.
- Member ID trong index được dựng theo thứ tự tăng dần. Helper kiểm giới hạn tham chiếu trước khi index để không đọc vượt vector nếu map dẫn xuất lỗi.
- Không đổi save schema/token, binding, command, action, observation, reward hoặc model.
- Build và chạy Realm/manager tests Release, Debug; chạy full core CTest khi cấu hình đã có sẵn; kiểm tra diff trong phạm vi.

## Giới hạn review

Plan và code được Codex tự review trong lượt này, không có reviewer độc lập. Lát này chỉ tách truy vấn guardian/authority đang có; chưa triển khai chuyển hộ, consent/guardianship administration, quản lý tài sản dependent, đất/title, phân chia di sản hoặc hoàn tất M5.
