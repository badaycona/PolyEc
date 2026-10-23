# Bằng chứng M5 — quỹ hộ không nhập vào di sản cá nhân

Ngày 2026-10-22. Kế hoạch: [kiểm quỹ hộ và hồ sơ di sản](../design/M5-core-estate-household-fund-isolation-plan.md). Tài liệu ledger đã quy định rõ household fund là tài khoản chung tách khỏi tài sản cá nhân. Lát này chỉ bổ sung fixture Realm kiểm tra khi một thành viên chết; không sửa gameplay source, schema, API hoặc binding.

## Chuỗi được kiểm tra

- Realm fixture có một người sắp chết với cash riêng, hàng, khoản vay, title, quyền sử dụng và xưởng. Một khoản **37 tiền** được chuyển từ Province firm sang household fund của đúng hộ bằng test helper; helper chuyển giữa tài khoản, không tạo tiền, và Realm invariant vẫn đúng trước sự kiện.
- Pha tử vong mở hồ sơ của người chết, trả khoản vay hiện hành và phân phối phần tiền cá nhân còn lại cho người hưởng duy nhất. Test đối chiếu số dư người hưởng với oracle tiền cá nhân, xác nhận hồ sơ không còn cash, đồng thời quỹ hộ vẫn là 37.
- Tổng tiền toàn Realm và invariant được giữ nguyên.
- Save/load vào Realm mới giữ đúng household fund, estate case, cash cá nhân của người hưởng và fingerprint.

Test không cấp guardian quyền rút hoặc chi tiền cá nhân của trẻ; không đổi ai có thể dùng quỹ hộ theo command hiện có.

## Kiểm tra

- Windows Visual Studio Release build target `polyec_realm_tests`: pass.
- Chạy `build/m5-core/core/Release/polyec_realm_tests.exe`: **20.675 checks pass**. Runner kèm profile A 10.000 người, 19.743 quan hệ thưa, 90 ngày; không phải profile B.
- Full core Release CTest **19/19** và full core Debug CTest **19/19** đã pass trước khi thêm fixture này (Debug 429,58 giây; lượt Release tích hợp trước đó 37,74 giây). Sau khi bổ sung fixture-only, chạy trực tiếp Realm Release để kiểm đúng target bị đổi; không chạy lại 19 nhóm.
- Python/native/GDExt/Godot không chạy lại vì không đổi source xử lý, save schema, API hoặc binding. Những kết quả adapter trước đó không được dùng để tuyên bố test fixture mới chạy qua Python/Godot.
- Review do Codex tự thực hiện, không có reviewer độc lập.

[Bản kê hash](../../artifacts/m5-core-estate-household-fund-isolation-2026-10-22-manifest.json) ghi file test, plan, report và trạng thái bàn giao. Không commit, push hoặc publish.
