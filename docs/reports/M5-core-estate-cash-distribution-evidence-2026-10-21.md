# Bằng chứng — chia tiền mặt di sản sau khi trả nợ

Ngày 2026-10-21. Plan: [M5-core-estate-cash-distribution](../design/M5-core-estate-cash-distribution-plan.md). Thay đổi nối bước phân phối tiền vào chu trình xử lý tử vong của Realm.

## Người chơi sẽ thấy gì

Khi một cư dân chết, tiền mặt và khoản vay còn dư của chính người đó đã được ghi vào hồ sơ di sản. Realm thử trả toàn bộ khoản vay từ tiền trong hồ sơ nếu số tiền đủ. Phần tiền còn lại sau đó được chia bằng nhau cho nhóm người thân đầu tiên còn sống theo thứ tự luật hiện hành: con hợp pháp, phối ngẫu, cha mẹ, rồi anh chị em. Nếu số tiền không chia hết, mỗi đơn vị còn dư được trao theo thứ tự ID tăng dần để kết quả luôn lặp lại giống nhau. Nếu không còn người thân đủ điều kiện, tiền được cộng vào quỹ di sản công tổng hợp.

Phần tiền chuyển cho mỗi người được cộng vào tài khoản cá nhân của chính người đó. Quỹ hộ gia đình và tiền của những người khác không bị dùng. Hồ sơ của người chết vẫn tồn tại để theo dõi hàng hóa, quyền đất, quyền sử dụng đất, khoản vay và xưởng; sau khi tiền được chuyển, trường tiền mặt của hồ sơ bằng 0. Hàng, đất và xưởng chưa được chuyển trong thay đổi này. Người giám hộ chưa được thêm quyền chi hoặc ký thay cho người chưa thành niên.

## Thay đổi trong mã

- `EstateManager` chuẩn bị trước toàn bộ số dư cuối cùng. Nó kiểm tra hồ sơ, tổng quỹ, quan hệ người hưởng và khả năng chứa tiền của từng người trước khi ghi bất kỳ số dư nào.
- Khi có người hưởng, số dư tiền mặt của hồ sơ giảm đúng bằng tổng phần chia và tổng tiền cá nhân tăng đúng bằng số đó.
- Khi không có người hưởng, số dư hồ sơ giảm về 0 và cùng lượng tiền được cộng vào `legacy_estate_unassigned_`, phần số dư quỹ di sản công tổng hợp đã được lưu trong save. Tổng tài khoản di sản không đổi ở nhánh này.
- Trong `Realm::day_people`, phân phối được gọi ngay sau quyết toán khoản vay. Không đổi schema save, binding Python/GDExt, action, observation, reward hoặc model.

## Kiểm tra đã chạy

- Windows Release build target `polyec_manager_tests` và `polyec_realm_tests`: exit code 0.
- `polyec_manager_tests.exe`: pass; kiểm tra preparation không ghi sớm, chia đều 11 đơn vị cho hai người với phần dư thuộc ID nhỏ hơn, chuyển tiền không người nhận vào quỹ công, tràn tài khoản từ chối trước khi đổi hồ sơ và số dư người hưởng đã thay đổi làm giao dịch từ chối nguyên tử.
- `polyec_realm_tests.exe`: **20.634 kiểm tra pass**. Bổ sung chuỗi thật: cư dân chết, khoản vay 20 được trả từ hồ sơ 1.000, 980 còn lại chuyển cho phối ngẫu còn sống, rồi save/load giữ nguyên hồ sơ, khoản vay, số dư và fingerprint. Tổng tiền Realm và invariant được kiểm tra.
- `git diff --check` trên bốn file code trong phạm vi: exit code 0; Git phát cảnh báo chuẩn hóa CRLF của `test_realm.cpp`, không báo whitespace error.

## Chưa kiểm và giới hạn

- Chưa chạy full CTest, Debug của source mới, Python replay/migration, GDExt build, Godot UI/smoke, Realm 64 ngày, learned policy, benchmark hoặc package. Đây là bước core; kiểm tra Python/native/Godot được gom tới khi nhóm estate/ownership ổn định theo chỉ dẫn 2026-10-21.
- Save schema không đổi vì hồ sơ tiền, tổng estate và số dư không gắn vào một hồ sơ đã được ghi trước đó. Migration riêng không phát sinh trong lát này.
- Tiền của người chưa thành niên được ghi vào tài khoản cá nhân thuộc sở hữu của họ; quyền người giám hộ quản lý hay sử dụng tài khoản đó vẫn chưa được thiết kế.
- Phân phối hàng hóa, quyền đất, quyền sử dụng đất, xưởng, xử lý đồng sở hữu, yêu cầu của chủ nợ khi di sản thiếu tiền, và toàn bộ M5 vẫn còn mở.
- Review do Codex tự thực hiện; chưa có reviewer độc lập. Working tree có nhiều thay đổi trước đó; các file được kiểm không đồng nghĩa mọi thay đổi trong file thuộc riêng lát này. Không tạo commit.

## Lệnh và kết quả

```powershell
& 'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe' --build build/m5-core --config Release --target polyec_manager_tests polyec_realm_tests --parallel 4
build/m5-core/core/Release/polyec_manager_tests.exe
build/m5-core/core/Release/polyec_realm_tests.exe
```

Build và hai chương trình test đều trả exit code 0. Realm runner báo `20.634 checks passed` và profile A 10.000 người, 19.743 quan hệ thưa, 90 ngày. Profile A là phần đo có sẵn trong Realm test runner; nó không thay cho profile B M5.
