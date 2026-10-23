# Kế hoạch M5 — khóa lệnh rút quỹ hộ khi chỉ còn trẻ vị thành niên

Ngày 2026-10-22. Contract tài khoản quỹ hộ chỉ cho phép người trưởng thành đang là `household_authority` rút tiền. Death settlement chuyển tiền cá nhân sang di sản, không chuyển quyền sở hữu quỹ chung cho guardian hoặc người hưởng. Lát này kiểm tra đúng trường hợp hộ không còn thành viên trưởng thành.

## Hành vi cần chứng minh

1. Tạo household có chủ hộ trưởng thành và một người con chưa thành niên; chuyển 37 tiền từ Province firm vào quỹ hộ.
2. Cho chủ hộ qua đời bằng pha Realm. Người con là người hưởng cá nhân theo parentage hợp pháp, nhưng vẫn chưa đủ 18 tuổi.
3. Authority query trả 0 vì không có người lớn còn sống. Quỹ hộ giữ 37 và không nằm trong cash estate của người chết.
4. Lệnh rút 5 do người con gửi bị Realm từ chối nguyên tử; fingerprint, cash người con, quỹ hộ, total money và invariant không đổi.
5. Save/load giữ nguyên quỹ, không có authority và estate case.

Không thêm quyền guardian, allowance, chi hộ hoặc cơ chế household dissolution.

## Phạm vi và kiểm tra

- Chỉ test/fixture ở `core/tests/test_realm.cpp` và plan/report/CURRENT_TASK/HISTORY/manifest.
- Build target `polyec_realm_tests` và chạy trực tiếp trên Windows Release và Debug: mỗi cấu hình **20.695 checks** pass, profile A 10.000 người/19.743 quan hệ thưa/90 ngày.
- Full core CTest Release/Debug **19/19** đã pass trước fixture-only addition. Sau khi thêm test, chạy trực tiếp Realm target cả hai cấu hình; không thay production/API/save/binding nên adapter không chạy lại.
- Không chạy adapter vì không sửa production/API/schema/binding.
