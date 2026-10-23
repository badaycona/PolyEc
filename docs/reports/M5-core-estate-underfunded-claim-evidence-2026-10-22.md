# Bằng chứng M5 — Realm xử lý di sản không đủ tiền trả khoản vay

Ngày 2026-10-22. Plan: [kiểm khoản vay thiếu tiền trong Realm](../design/M5-core-estate-underfunded-claim-plan.md). Đây là test nối các luật claim, phân phối và lưu/tải đã có; không thêm thứ tự ưu tiên chủ nợ.

## Hành vi được kiểm

Fixture cho một người trưởng thành qua đời, để lại khoản vay lớn hơn tiền mặt của chính hồ sơ. Người phối ngẫu còn sống là người hưởng đầu tiên. Khi pha tử vong thật chạy:

- tiền mặt trong hồ sơ không đủ trả toàn bộ khoản vay, nên không trả một phần; `Loan::paid` vẫn bằng 0 và case giữ tham chiếu khoản vay;
- cash còn trong hồ sơ và hàng Fiber được chia cho người phối ngẫu theo luật phân phối hiện có; nợ không được ghi lên người hưởng;
- vì claim chưa quyết toán, holding, quyền sử dụng đất đang hoạt động và xưởng không đổi chủ; các ID tiếp tục được tham chiếu trong hồ sơ;
- tổng tiền, invariant, title, permission, xưởng và khoản vay roundtrip qua save/load.

Test dùng đúng contract hiện hành: tiền mặt không đủ thì không chọn chủ nợ ưu tiên và không trả một phần; nghĩa vụ không chuyển cho người thân; tài sản hiện vật không bị tự động bán để trả nợ. Test không quyết định cách chia title/xưởng giữa nhiều người hưởng.

## Thay đổi trong test

`core/tests/test_realm.cpp` cho phép cấu hình số tiền khoản vay trong helper estate đang có, mặc định giữ nguyên 20 cho fixture cũ. Test mới cấp khoản vay 2.000 khi hồ sơ có ít hơn số đó, gọi pha cư dân thật rồi kiểm tra các trường hợp phía trên. Đây chỉ là thay đổi fixture/test; không sửa logic Realm hoặc định dạng save.

## Kiểm tra đã chạy

- Windows Visual Studio Release build target `polyec_realm_tests` và `polyec_manager_tests`: thành công.
- `polyec_realm_tests.exe`: **20.652 checks passed**. Runner chạy profile A 10.000 người, 19.743 quan hệ thưa, 90 ngày; đây không phải profile B.
- `polyec_manager_tests.exe`: pass, giữ lại kiểm tra thuần xác nhận khoản vay không bị trả từng phần khi số dư hồ sơ thiếu.
- Core Release CTest **19/19** vừa pass ở mốc R4 trước các fixture-only estate additions. Không chạy lại full suite sau thay đổi test Realm; target Realm và manager bị ảnh hưởng đã build và chạy trực tiếp.
- Review do Codex tự thực hiện, chưa có reviewer độc lập.

## Chưa kiểm và giới hạn

Không chạy Debug, Python/native/GDExtension/Godot, learned policy, profile B hoặc package trên source mới. Chưa giải quyết luật đồng sở hữu/định giá/thanh lý title và xưởng khi nhiều người hưởng hoặc khi nợ không đủ tiền; tài sản vẫn được giữ trong hồ sơ theo contract hiện tại. Kết quả này chỉ xác nhận nhánh một người phối ngẫu hưởng và khoản vay thiếu tiền. M5 và core refactor tích hợp còn mở. Không commit/push.

Manifest SHA-256: [underfunded estate claim](../../artifacts/m5-core-estate-underfunded-claim-2026-10-22-manifest.json).
