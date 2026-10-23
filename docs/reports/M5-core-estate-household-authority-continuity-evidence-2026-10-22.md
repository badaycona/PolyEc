# Bằng chứng M5 — quỹ hộ và người được quyền rút sau khi chủ hộ qua đời

Ngày 2026-10-22. Kế hoạch: [quỹ hộ sau khi authority qua đời](../design/M5-core-estate-household-authority-continuity-plan.md). Test Realm C++ nối các contract đã có: quỹ là tài khoản chung gắn với household ID; người lớn đang được Realm chọn làm `household_authority` mới là người được rút; cái chết đưa tài sản cá nhân vào hồ sơ di sản nhưng không lấy quỹ hộ.

## Tình huống và kết quả mong đợi

- Fixture chọn hai người trưởng thành và đặt họ trong cùng một household label. Người có ID trùng label là chủ hộ hiện tại, do đó bắt đầu là người được quyền rút. Fixture tách những người còn lại vốn được gom vào household khởi đầu để thử chính xác chuyển quyền giữa hai người đã chọn.
- **37 tiền** được chuyển từ Province firm vào quỹ chung, không tạo tiền. Chủ hộ có cash cá nhân, bị đánh dấu đã chết ở pha cư dân, và người còn sống được gắn quan hệ phối ngẫu để death settlement có thể xử lý phần tài sản cá nhân theo luật hiện hành.
- Sau pha Realm, người chết không còn sống; hồ sơ di sản của họ có cash 0 sau khi phần cash cá nhân được phân phối. `household_authority` bây giờ trả ID người lớn còn sống cùng household label. Quỹ hộ vẫn 37, tổng tiền và invariant không đổi.
- Người có quyền mới gửi `HouseholdFundCommand` rút đúng **5 tiền**. Realm cộng 5 vào tài khoản cá nhân của họ, quỹ còn 32, tổng tiền vẫn giữ nguyên.
- Save/load giữ household fund 32, authority mới, hồ sơ di sản và fingerprint chính xác.

## Lượt sửa fixture

Lần chạy đầu cho thấy hộ khởi tạo mặc định có thể chứa tới bốn cư dân; một người lớn khác cùng label lớn tuổi hơn người kế tiếp mà test chọn. Realm đã chọn đúng người lớn nhất theo `HouseholdIndex` hiện hành, vì thế phép khẳng định ban đầu về successor cụ thể không đúng với fixture rộng đó. Fixture được thu gọn về hai người đã chọn và đặt người kế tiếp lớn hơn chủ hộ một năm; không đổi code gameplay hoặc quy tắc chọn người có quyền. Lượt build/test cuối qua toàn bộ Realm checks.

## Kiểm tra và giới hạn

- Windows Visual Studio Release build target `polyec_realm_tests`: pass.
- Chạy `build/m5-core/core/Release/polyec_realm_tests.exe`: **20.689 checks passed**. Kèm profile A: 10.000 người, 19.743 quan hệ thưa, 90 ngày; không phải profile B.
- Windows Visual Studio Debug build cùng target và chạy `build/m5-core/core/Debug/polyec_realm_tests.exe`: **20.689 checks passed**. Executable chạy trực tiếp mất 241,52 giây; profile A giống Release.
- Full core CTest Release **19/19 trong 38,54 giây** và Debug **19/19 trong 391,31 giây** đã pass trước fixture-only này. Sau khi thêm test mới, build/chạy trực tiếp Realm target ở cả Release và Debug; không lặp toàn CTest vì các nhóm khác không đổi.
- Không chạy Python/native/GDExt/Godot vì không đổi source sản xuất, schema, API hoặc binding. Self-review do Codex thực hiện, không có reviewer độc lập.

Lát này không đổi chuẩn authority, không chuyển quỹ vào di sản, không trao cho guardian quyền chi tiền riêng của người khác và không đóng toàn M5. [Manifest hash](../../artifacts/m5-core-estate-household-authority-continuity-2026-10-22-manifest.json) ghi checkpoint của test và tài liệu. Không commit, push hoặc publish.
