# Kế hoạch M5 — kiểm chuỗi thừa kế đất cho trẻ và thao tác của người giám hộ

Ngày 2026-10-22. Lát tích hợp C++ cho hai quy tắc đã có: title đất có thể thuộc người chưa thành niên sau khi nhận di sản; người giám hộ hiện tại có thể quản lý giấy phép sử dụng đất hiện có trong giới hạn cho phép.

## Mục tiêu và hành vi

Tạo fixture Realm thật có một người lớn đã xác nhận là cha/mẹ hợp pháp, một người con còn sống dưới 18 tuổi và một người lớn còn sống trong cùng household để làm giám hộ. Người lớn qua đời với một holding và quyền sử dụng đã được cấp. Pha tử vong phải chuyển title cho đúng trẻ và cập nhật grantor của quyền đang có. Sau chuyển giao, giám hộ hiện tại dùng lệnh Realm `LandUseRevokeCommand`; quyền bị gỡ, còn title vẫn đứng tên trẻ. Save/load phải tiếp tục đúng dữ liệu và thẩm quyền suy ra từ household.

Chuỗi này chỉ xác nhận sự kết hợp của luật đã ghi trong `IMPLEMENTATION_PLAN` §2 và contract quản lý land-use của guardian. Không cấp cho guardian title, tiền, quyền bán/chuyển deed hoặc quyền tự cấp đất sử dụng cho mình.

## Phạm vi

- `core/tests/test_realm.cpp`: fixture test-only chọn household hợp lệ, đánh dấu cạnh cha–con hợp pháp, thêm holding/quyền sử dụng, chạy pha cư dân thật và command revoke; kiểm quyền sở hữu, actor fact, invariant, tiền không đổi và save/load.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`, report và manifest sau khi kiểm.
- Không sửa gameplay source, schema, public API, Python/GDExt/Godot hoặc model.

## Kiểm tra

Ngay: build `polyec_realm_tests` Release, chạy trực tiếp executable để kiểm success/death/save-load; chạy `polyec_land_title_tests` để giữ regression các trường hợp guardian grant/revoke hiện có. Nếu code chỉ là fixture test và không đổi core behavior, không chạy full CTest lại ngay sau R4 CTest vừa pass.

Python/native/GDExt/Godot thuộc mốc tích hợp estate/ownership ổn định. Core Release hiện có đầy đủ CTest sau R4; lượt này không cần lặp các target không đổi. Không có benchmark hoặc package trong phạm vi.
