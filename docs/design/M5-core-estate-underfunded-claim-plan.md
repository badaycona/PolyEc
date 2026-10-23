# Kế hoạch M5 — kiểm di sản không đủ tiền trả khoản vay trong Realm

Ngày 2026-10-22. Đây là kiểm chuỗi C++ cho contract đã ghi trong các plan estate cash-claim/distribution, không bổ sung thứ tự ưu tiên chủ nợ.

## Mục tiêu và hành vi

Dùng fixture death-settlement Realm hiện có với người thừa kế trưởng thành duy nhất, title đất, quyền dùng đất, xưởng và khoản vay. Làm khoản vay lớn hơn cash của hồ sơ. Sau khi cư dân chết:

- khoản vay không được trả một phần và số `Loan::paid` không đổi;
- case vẫn giữ khoản vay, title, quyền đất và xưởng; title/xưởng tiếp tục đứng tên người chết vì claim chưa được quyết toán;
- cash và hàng cá nhân chuyển theo quy tắc estate distribution hiện có; khoản nợ không bị trừ khỏi tài khoản người hưởng;
- invariant, tổng tiền và save/load giữ nguyên các kết quả này.

Quy tắc này dùng đúng contract hiện hành: nợ không tự chuyển thành khoản nợ người hưởng; không thanh lý tài sản hiện vật hay tự đặt thứ tự giữa các chủ nợ.

## Phạm vi

- `core/tests/test_realm.cpp`: tham số hóa helper estate hiện có để tạo một khoản vay vượt số tiền trong hồ sơ; bổ sung test death → claim shortfall → distribution/property retention → save/load.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`, report và manifest sau khi kiểm.
- Không sửa logic gameplay hoặc save schema.

## Kiểm tra

Ngay: build Release target `polyec_realm_tests`, chạy executable trực tiếp; chạy `polyec_manager_tests` để giữ kiểm thuần all-or-none creditor behavior. CTest core 19/19 vừa pass trước các fixture-only thay đổi, nên không chạy lại toàn bộ suite ngay. Không cần Debug/adapter/package vì không đổi core behavior, API hoặc schema.
