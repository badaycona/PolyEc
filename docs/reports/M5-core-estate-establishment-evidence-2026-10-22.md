# Bằng chứng M5: chuyển xưởng trong di sản cho một người hưởng trưởng thành duy nhất

Ngày 2026-10-22. Plan: [chuyển quyền sở hữu xưởng trong hồ sơ di sản](../design/M5-core-estate-establishment-plan.md). Đây là bước tiếp trong nhóm ownership/di sản; working tree đã có nhiều thay đổi ngoài lát này. Báo cáo chỉ xác nhận hành vi và kiểm tra ghi dưới đây. Codex tự review, không có reviewer độc lập. Không commit hoặc push.

## Người chơi sẽ thấy gì

Nếu người chết còn một người thừa kế hợp pháp duy nhất, người đó đã đủ 18 game years và các khoản vay được ghi trong hồ sơ đã thanh toán hết, người thừa kế trở thành chủ sở hữu các xưởng cá nhân của người chết. Người mới có thể dùng các lệnh Realm hiện có để quản lý xưởng. Ví dụ đã kiểm: người nhận dùng lệnh rút 1 tiền từ tài khoản xưởng, và 1 tiền được cộng đúng vào tài khoản cá nhân.

Khi đổi chủ, xưởng giữ nguyên vùng, ngành sản xuất, công suất, tiền, hàng, quyền làm việc đã cấp cho công nhân, số công đã được ghi nhận, khoản lương còn lại và ngày sản xuất gần nhất. Xưởng vẫn nằm trong C++ Realm duy nhất; không tạo bản sao ở Python hoặc giao diện.

Nếu người thừa kế duy nhất chưa đủ 18 tuổi, có nhiều người cùng hàng, không có người hưởng hoặc còn khoản vay chưa thanh toán, xưởng vẫn đứng tên người chết và ID vẫn ở trong hồ sơ. Đây là cách bảo toàn tài sản cho tới khi có quy tắc về nhiều chủ, giám hộ xưởng hoặc xử lý xưởng để trả phần nợ chưa đủ tiền. Lát này không thanh lý, chia xưởng hay tự chọn người đại diện.

## Phần code thay đổi

- `EstateManager` stage ID xưởng của hồ sơ và chủ sở hữu ban đầu. Khi đủ điều kiện, nó chỉ đổi `owner_id` từ người chết sang người hưởng; `owner_kind` và các trường còn lại không đổi. Hồ sơ bỏ ID xưởng sau khi chuyển.
- Giao dịch đối chiếu tất cả khoản vay, hồ sơ, người hưởng và title trước lần ghi đầu tiên. Với từng xưởng sắp chuyển, nó xác nhận ID còn tồn tại, vẫn là xưởng của cá nhân và vẫn thuộc người chết. Nếu chủ đã đổi giữa lúc chuẩn bị và áp dụng, toàn bộ chuyển tiền, hàng và quyền sở hữu bị từ chối.
- Sau khi xử lý các ca tử vong trong ngày, Realm dựng lại chỉ mục nội bộ từ ID chủ sở hữu sang ID xưởng. Chỉ mục đó được dùng để cộng tài sản xưởng vào đánh giá tài sản cá nhân; việc dựng lại bảo đảm người thừa kế mới nhận đúng giá trị xưởng.
- Không đổi save schema, action, observation, reward, model hoặc binding. Lưu/tải tiếp tục dùng các trường xưởng đang có.

## Kiểm tra đã chạy

- Windows Release build cho `polyec_manager_tests` và `polyec_realm_tests`: exit code 0.
- `polyec_manager_tests.exe` pass. Direct checks xác nhận người lớn duy nhất nhận xưởng; tiền, hàng, quyền lao động, sổ công và tiến độ sản xuất giữ nguyên; minor heir, nhiều người hưởng, không người hưởng và khoản nợ còn dư không làm xưởng rời hồ sơ; stale owner từ chối trước khi thay số dư.
- `polyec_realm_tests.exe` pass với **20.639 checks**. Fixture C++ tạo xưởng có 10 tiền và giấy phép lao động, làm người chết để chạy chuỗi Realm thật, trả khoản vay 20, chia tiền/hàng, chuyển title đất và quyền sử dụng, chuyển xưởng cho phối ngẫu trưởng thành, rồi dùng lệnh Realm sẵn có rút 1 tiền xưởng. Invariant và tổng tiền vẫn đúng. Save/load giữ fingerprint, quyền đất, khoản vay đã trả và số xưởng; Realm test runner chạy kèm profile A 10.000 người, 19.743 quan hệ thưa, 90 ngày. Đây không phải profile B.

## Chưa được kiểm trong lượt này

Không chạy Debug, full CTest, Python binding/replay, GDExtension, Godot, Realm 64 ngày có model, learned policy, profile B hoặc package Windows. API binding, schema, action/observation/reward/model không đổi nên adapter được để mốc estate/ownership ổn định theo yêu cầu gom kiểm tra. Chưa có quy tắc hoàn chỉnh cho nhiều người hưởng, xưởng của trẻ, không người hưởng hoặc khoản nợ thiếu tiền; do đó chưa tuyên bố M5, core refactor toàn phần hay RL đã hoàn thành.

Plan nêu đủ file, trường dữ liệu, tình huống giữ tài sản và phạm vi test. Hash source và báo cáo tại thời điểm hoàn tất nằm trong [manifest](../../artifacts/m5-core-estate-establishment-2026-10-22-manifest.json).

## Lệnh đã chạy

```powershell
& 'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe' --build build/m5-core --config Release --target polyec_manager_tests polyec_realm_tests --parallel 4
& 'C:/prc/PolyEc/build/m5-core/core/Release/polyec_manager_tests.exe'
& 'C:/prc/PolyEc/build/m5-core/core/Release/polyec_realm_tests.exe'
```
