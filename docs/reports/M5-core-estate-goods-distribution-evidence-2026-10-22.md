# Bằng chứng — chia tiền và hàng trong hồ sơ di sản

Ngày 2026-10-22. Các kế hoạch: [chia tiền](../design/M5-core-estate-cash-distribution-plan.md) và [chuyển hàng](../design/M5-core-estate-goods-distribution-plan.md). Đây là cập nhật tiếp theo trên chu trình tử vong đã có; thay đổi vẫn ở C++ core.

## Hành vi sau thay đổi

Khi một cư dân chết, Realm ghi tiền và hàng của riêng người đó vào hồ sơ di sản. Realm thử trả hết các khoản vay được ghi trong hồ sơ nếu có đủ tiền; không lấy tiền của hộ gia đình hoặc người sống. Sau khoản trả nợ, phần tiền còn lại được chia bằng nhau cho nhóm thân nhân còn sống đầu tiên theo thứ tự luật: con hợp pháp, phối ngẫu, cha mẹ, anh chị em. Mỗi đơn vị tiền không thể chia đều được trao cho ID nhỏ nhất trước.

Mỗi loại hàng được chia riêng với cùng số lượng bằng nhau cho nhóm người nhận. Phần dư mỗi loại cũng thuộc ID nhỏ nhất trước. Nếu không còn người thân đủ điều kiện, tiền vào quỹ di sản công tổng hợp, còn hàng được cộng vào kho công của vùng người chết. Không bán, định giá hoặc tiêu hao hàng. Hồ sơ vẫn tồn tại để các lát sau xử lý quyền đất, quyền sử dụng đất, khoản vay và xưởng; sau khi chuyển, các trường tiền và hàng của hồ sơ bằng 0.

Realm chốt tử vong và phân phối ở ranh giới cuối pha xử lý cư dân của ngày đó. Trước hết, nó thu thập tất cả người chết trong ngày, tạo hồ sơ và đánh dấu họ đã chết; sau đó mới trả nợ và chọn người hưởng. Vì vậy, người thừa kế không thể dùng khoản tiền mới nhận để mua thức ăn trong cùng ngày. Người cũng chết trong ngày được loại khỏi danh sách người hưởng dù ID của họ đứng trước hay sau người để lại di sản.

Người nhận được cộng tiền và hàng vào tài khoản cá nhân. Tiền/quyền của hộ gia đình không đổi. Tiền người chưa thành niên cũng được ghi cho chính cá nhân đó; quyền người giám hộ dùng hoặc quản lý tài khoản vẫn chưa được thêm.

## Cách giao dịch được bảo vệ

`EstateManager` tính số dư cuối cùng trước khi sửa Realm. Kế hoạch ghi lại số dư ban đầu của hồ sơ, quỹ di sản công, từng người nhận và kho công vùng. Khi áp dụng, nó kiểm tra các số dư ấy chưa thay đổi. Nếu người nhận không còn sống, tiền/hàng của họ bị sửa bởi giao dịch khác, kho công đã đổi, tổng quỹ sai, dữ liệu âm hoặc phép cộng vượt giới hạn số nguyên, giao dịch từ chối trước lần ghi đầu tiên. Realm gọi giao dịch sau bước trả nợ trong pha dân cư.

Save schema không đổi: tiền/hàng hồ sơ, hàng Person/Province và tổng quỹ đã được lưu. Không đổi action, observation, reward, model hay binding. Không có thay đổi nào yêu cầu Python/native/Godot gọi API mới.

## Kiểm tra đã chạy

- Windows Release build cho `polyec_manager_tests` và `polyec_realm_tests`: exit code 0.
- `polyec_manager_tests.exe`: pass. Oracle kiểm chia tiền/hàng theo từng loại, chia đơn vị dư theo ID, chuyển tiền sang sổ công và hàng vào kho công khi không có người thân, tràn số dư tiền/hàng, số dư người hưởng đã cũ và kho công đã đổi. Các trường hợp reject giữ nguyên toàn bộ dữ liệu được đưa vào giao dịch.
- `polyec_realm_tests.exe`: **20.634 checks pass**. Fixture Realm tạo người chết có khoản vay 20 và 5 Fiber. Theo lịch ngày trong fixture, người chết mua một phần thức ăn giá 1 trước khi chết; khoản vay 20 được trả kế tiếp và **979 tiền cùng cả 5 Fiber** được trao cho phối ngẫu. Người phối ngẫu bắt đầu ngày với 0 tiền và không thể tiêu khoản thừa kế trong cùng ngày. Save/load giữ đúng tiền, hàng, khoản vay và fingerprint. World invariant và tổng tiền giữ nguyên. Runner cũng chạy profile A có sẵn: 10.000 người, 19.743 quan hệ thưa, 90 ngày. Đây không phải profile B M5.
- `git diff --check` cho thay đổi code của lát này: exit code 0; chỉ có cảnh báo chuyển CRLF của file test hiện tồn tại.

## Giới hạn còn lại

- Chưa chạy full CTest, Debug trên source mới, Python save/replay, GDExt, Godot UI/smoke, Realm 64 ngày có model, learned policy, benchmark profile B hoặc package Windows. Các lớp adapter sẽ được kiểm ở mốc estate/ownership ổn định theo yêu cầu gom kiểm tra của người dùng.
- Chưa chuyển quyền sở hữu đất, quyền sử dụng đất hoặc xưởng. Chưa chia tài sản đồng sở hữu, xử lý khả năng nhận đất/xưởng của người chưa thành niên, hoặc quy định người giám hộ được chi gì. Chưa hoàn thành M5 hay core refactor toàn phần.
- Review do Codex tự thực hiện; chưa có reviewer độc lập. Working tree đã có nhiều thay đổi trước đó; test chỉ xác nhận các đường gọi và fixture nêu trên, không chứng nhận toàn bộ mọi thay đổi trong các file chung. Không tạo commit.

## Lệnh kiểm tra

```powershell
& 'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe' --build build/m5-core --config Release --target polyec_manager_tests polyec_realm_tests --parallel 4
build/m5-core/core/Release/polyec_manager_tests.exe
build/m5-core/core/Release/polyec_realm_tests.exe
```

Build và cả hai test runner trả exit code 0. Không chạy toàn bộ project.

Manifest SHA-256 của các file có liên quan: [m5-core-estate-goods-distribution](../../artifacts/m5-core-estate-goods-distribution-2026-10-22-manifest.json). Hash là toàn file trong working tree dùng chung; chúng không khẳng định mọi thay đổi trong các file chung đều thuộc lát này.
