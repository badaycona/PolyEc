# Bằng chứng M5: chuyển title đất từ di sản cho một người hưởng duy nhất

Ngày 2026-10-22. Plan: [chuyển title đất trong hồ sơ di sản](../design/M5-core-estate-land-title-plan.md). Thay đổi này thuộc cùng nhóm di sản đang được nối từng phần trong core. Working tree đã có nhiều sửa đổi từ các phiên trước; kết quả dưới đây chỉ nói về các hành vi và đường kiểm tra được mô tả trong báo cáo này. Codex tự review, không có reviewer độc lập. Không tạo commit hoặc push.

## Người chơi sẽ thấy gì

Khi một nhân vật qua đời, hồ sơ của người đó đã ghi lại các holding đất mà họ sở hữu và mọi giấy phép sử dụng đang gắn với các holding đó. Sau khi các khoản vay trong hồ sơ được thanh toán, nếu chỉ còn đúng một người thân hợp pháp đủ điều kiện nhận di sản, title của từng holding chuyển sang người này. Người nhận có thể chưa đủ 18 tuổi: luật hiện hành cho phép người chưa thành niên sở hữu đất. Nếu holding đang có giấy phép sử dụng đất, giấy phép vẫn còn hiệu lực và người nhận mới được ghi làm người cấp hiện tại; người được cấp phép, holding, diện tích và vùng không đổi.

Core giữ title trong hồ sơ nếu có nhiều người cùng hàng thừa kế, không có người hưởng, hoặc còn khoản vay chưa trả. Như vậy, hệ thống không tự chọn một người trong số các con, không chia một holding không thể chia bằng dữ liệu hiện tại và không làm biến mất tài sản. Cách chia title cho nhiều người, cách xử lý đất khi không có người thừa kế, cũng như cách dùng đất để trả khoản vay còn thiếu vẫn cần contract riêng.

## Phần code thay đổi

- `EstateManager` chuẩn bị title cuối cùng cùng với tiền và hàng trong một kế hoạch duy nhất. Trước khi ghi bất cứ số dư nào, bước áp dụng so sánh hồ sơ, khoản vay, holding, giấy phép sử dụng, người nhận và kho vùng với dữ liệu đã dùng lúc lập kế hoạch. Nếu một đầu vào liên quan đã đổi, giao dịch từ chối mà không chia một phần tiền hoặc hàng.
- Khi đủ điều kiện chỉ có một người hưởng, mọi holding được hồ sơ tham chiếu chuyển cùng lúc sang người đó và ID holding được bỏ khỏi hồ sơ. Mọi giấy phép sử dụng gắn với holding đó giữ nguyên grantee và nhận `grantor` hiện tại mới là người thừa kế. Không có phép định giá, bán, thuế, chia diện tích hay sản lượng mới.
- `Realm::day_people` truyền sổ khoản vay, title và giấy phép hiện hành qua cùng giao dịch di sản sau khi xử lý mọi ca tử vong trong ngày. Chạy xử lý cuối pha bảo đảm tất cả người chết trong ngày bị loại khỏi danh sách hưởng trước khi chuyển tài sản.
- Không đổi save schema: `LandHolding`, `LandUseRight` và ID đang có sẵn trong Realm save; khi chuyển, chỉ holder/grantor và tham chiếu trong hồ sơ được cập nhật theo quy tắc hiện hành.

## Kiểm tra đã chạy

- Windows Release build chỉ cho hai target liên quan: `polyec_manager_tests` và `polyec_realm_tests`; build trả exit code 0.
- `polyec_manager_tests.exe` pass. Direct checks xác nhận: chuẩn bị không sửa dữ liệu; người thừa kế duy nhất dưới 18 tuổi nhận title; giấy phép sử dụng vẫn còn và cùng người được cấp phép; nhiều con cùng hàng, không có người hưởng hoặc khoản vay còn dư đều giữ title trong hồ sơ; thay holding sau khi chuẩn bị làm toàn bộ chuyển tiền/hàng/title bị từ chối trước khi ghi.
- `polyec_realm_tests.exe` pass với **20.636 checks**. Fixture chạy chuỗi thật trong Realm: nhân vật chết, khoản vay 20 được thanh toán từ hồ sơ, tiền và Fiber còn lại được chuyển như trước, title cùng giấy phép sử dụng chuyển cho phối ngẫu, rồi Realm được lưu và tải lại. Fingerprint, title, giấy phép và loan payment khớp sau roundtrip. Runner cũng chạy sẵn workload profile A: 10.000 người, 19.743 quan hệ thưa, 90 ngày; đây không phải profile B.
- Kiểm tra khoảng trắng trên các file thuộc lát này pass. Lệnh `git diff --check` trên toàn working tree báo một dòng trailing whitespace trong file save kiểm thử có sẵn `m5_t1b1_clock_save.bin`, ngoài phạm vi thay đổi; cũng có thông báo chuẩn hóa CRLF ở `core/tests/test_realm.cpp`. File save đó không bị sửa.

## Chưa được kiểm trong lượt này

Không chạy Debug, full CTest, Python binding/replay, GDExtension, thao tác Godot, smoke Realm 64 ngày có model, learned policy, benchmark profile B hoặc Windows package. Không có thay đổi API binding, action, observation, reward, model hoặc save schema trong lát này; adapter được để mốc tích hợp nhóm estate/ownership ổn định như `AGENTS.md` yêu cầu. Kết quả hiện tại chỉ xác nhận core Release và hai test runner nêu trên.

Lát này chưa giải quyết chuyển xưởng, nhiều người đồng thừa kế, không có người hưởng, thiếu tiền trả nợ, quyền giám hộ với tiền/tài sản, hoặc tranh chấp. Do đó đây chưa phải xác nhận M5, core refactor toàn phần hay learned RL đã hoàn thành. Chi tiết phạm vi và lệnh kiểm tra nằm trong [plan](../design/M5-core-estate-land-title-plan.md); file hash theo source hiện tại nằm trong [manifest](../../artifacts/m5-core-estate-land-title-2026-10-22-manifest.json).

## Lệnh đã chạy

```powershell
& 'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe' --build build/m5-core --config Release --target polyec_manager_tests polyec_realm_tests --parallel 4
& 'C:/prc/PolyEc/build/m5-core/core/Release/polyec_manager_tests.exe'
& 'C:/prc/PolyEc/build/m5-core/core/Release/polyec_realm_tests.exe'
```
