# Bằng chứng M5 — di sản đất của trẻ vị thành niên và quyền thao tác của giám hộ

Ngày 2026-10-22. Plan: [kiểm chuỗi Realm giữa thừa kế đất và giám hộ](../design/M5-core-estate-minor-guardian-land-plan.md). Đây là một test tích hợp C++ cho hai luật đã được triển khai trước đó; không thêm quy tắc gameplay mới.

## Điều người chơi sẽ thấy

Khi người đứng tên một holding chết, nếu chỉ có một người hưởng hợp pháp và khoản vay trong hồ sơ đã trả xong, holding có thể được chuyển sang người con chưa đủ 18 tuổi. Người sử dụng đất đang được cấp phép vẫn giữ quyền hiện có. Sau đó, người lớn còn sống đang được hệ thống xác định là giám hộ trong cùng household có thể thu hồi giấy phép bằng lệnh Realm; title vẫn thuộc trẻ. Giao dịch không chuyển tiền hoặc quyền sở hữu sang người giám hộ.

## Thay đổi trong test

`core/tests/test_realm.cpp` có fixture test-only tạo household gồm cha/mẹ, con chưa thành niên và một người lớn còn sống làm giám hộ; fixture đánh dấu đúng cạnh cha–con hợp pháp, thêm title, và cấp quyền sử dụng ban đầu qua Realm command. Test chạy pha cư dân tử vong thật để xử lý di sản; sau đó người giám hộ dùng `LandUseRevokeCommand`. Các đối chiếu bao gồm:

- người đã chết không còn sống; title chuyển cho người con chứ không chuyển cho guardian;
- quyền sử dụng còn sau thừa kế, cùng holding/grantee nhưng grantor cập nhật theo title-holder mới;
- guardian vẫn được suy ra từ household; command revoke ghi fact với đúng actor và xóa permission;
- trẻ vẫn đứng tên title, tổng tiền và invariant không đổi vì việc thu hồi permission không chuyển tiền;
- save/load giữ title, quyền đã thu hồi, guardian được suy ra và fingerprint của Realm.

Không sửa source gameplay, save schema, API, binding, action, observation, reward, model, Python, native hoặc Godot.

## Kiểm tra đã chạy

- Windows Visual Studio Release build: `polyec_realm_tests` và `polyec_land_title_tests` thành công.
- `polyec_realm_tests.exe`: **20.645 checks passed**, bao gồm chuỗi mới và workload profile A 10.000 người/19.743 quan hệ thưa/90 ngày. Đây không phải profile B.
- `polyec_land_title_tests.exe`: **88 checks passed**, giữ lại các nhánh title/guardian grant/revoke trước đó.
- Core Release CTest ngay trước lát fixture này: **19/19 pass** sau khi sửa token-count oracle liên quan đến section parentage rỗng. Không chạy lại toàn bộ CTest chỉ vì thêm fixture vào Realm test; chạy trực tiếp hai executable bị ảnh hưởng.
- Review do Codex tự thực hiện; chưa có reviewer độc lập.

## Chưa kiểm và giới hạn

Chưa chạy Debug, Python binding/replay, native runtime, GDExtension/Godot, learned policy, profile B hoặc package trên source mới. Các lớp adapter sẽ được kiểm sau khi nhóm ownership/di sản đủ ổn định; không dùng kết quả cũ để chứng nhận source này. Bài test xác nhận một người con hưởng duy nhất, không đặt luật cho nhiều người hưởng, phân chia holding, thanh lý tài sản khi nợ thiếu tiền hoặc guardian quản lý tiền/xưởng. M5 và core refactor tích hợp vẫn còn mở. Không commit/push.

Lệnh và hash theo từng file được ghi trong manifest: [M5 estate minor guardian land](../../artifacts/m5-core-estate-minor-guardian-land-2026-10-22-manifest.json).
