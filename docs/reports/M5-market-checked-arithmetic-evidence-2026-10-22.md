# Báo cáo: phép tính tiền và thuế chợ an toàn khi số lượng lớn

## Thay đổi có ý nghĩa gì

Lệnh mua hàng tính tổng tiền phải trả; lệnh bán tính số tiền vùng trả cho người bán; mua hàng và tiêu dùng cuối ngày còn tính thuế cùng phần lẻ thuế giữ sang giao dịch kế tiếp. Các phép tính này nay phát hiện số liệu không hợp lệ hoặc kết quả không thể biểu diễn trong kiểu số nguyên 64-bit trước khi ghi sổ. Vì vậy một số lượng rất lớn không thể làm phép nhân tràn rồi bị hiểu nhầm thành khoản phải trả nhỏ, và giao dịch thuế không làm tăng sổ biên nhận vượt giới hạn.

Giá và quy tắc gameplay không đổi: người mua trả `(giá niêm yết + 1) × số lượng`; người bán nhận `max(1, giá niêm yết − 1) × số lượng`; thuế tiếp tục lấy phần nguyên của `(tổng mua × thuế suất + phần lẻ trước đó) / 100`, phần lẻ mới là số dư modulo 100. Helper mới chỉ tính an toàn và không chọn người mua/người bán, nguồn hàng hoặc chiến lược.

## Cách giữ nguyên kết quả nhưng tránh tràn

`MarketManager` cung cấp tổng giao dịch có kiểm tra trước khi nhân, báo không có kết quả khi giá/lượng sai miền hoặc tích vượt `INT64_MAX`. Với thuế, thay vì tính trực tiếp tích lớn `gross × tax_rate`, helper tách gross thành phần thương khi chia 100 và phần dư. Vì thuế suất hợp lệ không vượt 100%, phần thương sau khi nhân không lớn hơn gross; phần dư trung gian nhỏ hơn 10.000. Kết quả vẫn đúng bằng công thức thuế nguyên cũ, kể cả khi gross sát `INT64_MAX`.

Realm kiểm tổng tiền mua/bán và phép tính thuế trước khi cho lệnh tiếp tục. Nó cũng từ chối khi sổ thuế tích lũy không còn chỗ cộng biên nhận mới. Lệnh Buy tính kiểm tra trước khi trừ tiền và hàng; hàm thu thuế dùng lại cùng helper. Hai đường mua lương thực trong pha cư dân (tiền riêng hoặc tài khoản tổ chức) cũng kiểm biên nhận trước khi ghi tiền/hàng. Nếu dữ liệu tax/gross/remainder sai, giao dịch đó không trừ tiền hay hàng; trong pha tiêu dùng ngày, người không mua được vẫn đi theo xử lý thiếu lương thực hiện có.

Các file code chính: `core/include/polyec/market_manager.hpp`, `core/src/market_manager.cpp`, `core/src/realm.cpp`, `core/src/realm_lifecycle.cpp`, `core/include/polyec/realm.hpp` và `core/CMakeLists.txt`. Test helper mới là `core/tests/test_market_manager.cpp`.

## Phép kiểm cụ thể

Test helper kiểm tổng `7 × 3 = 21`, tổng bằng 0, giá/lượng âm hoặc lượng 0, tích chính xác bằng `INT64_MAX`, tích vượt giới hạn, quy tắc mua/bán cũ, mức bán tối thiểu 1, thuế 10% với phần lẻ 0 và 95, gross `INT64_MAX` ở thuế suất 100%, cùng gross/thuế suất/phần dư ngoài miền. **19/19 checks** pass.

Realm regression giữ nguyên lệnh thật và pipeline cuối ngày: buy/sell, mua hàng có thuế/phần dư, tiêu dùng lương thực, ledger, save/load, hành động từ chối và invariant. Runner đạt **21.086 checks** trên profile A gồm 10.000 người, 19.743 quan hệ thưa, 90 ngày. Chuỗi command mua Fiber → lập xưởng → sản xuất → bán/mua Cloth sau đó chạy lại với **217 checks**, xác nhận số dư, thuế, phần lẻ và save/load không đổi.

Sau khi nhóm core này ổn định, tôi build lại binding Python `polyec_env` từ C++ source mới. Script `ai/tests/market_transaction_binding_checks.py` dùng cùng seed và cùng command: mua Fiber, tạo xưởng, chuyển hàng, gửi Work, tiến 64 tick, rút/bán Cloth và cho cư dân khác mua. Nó so fingerprint toàn Realm giữa C++ và Python, đồng thời so số dư người mua/bán, quỹ xưởng, hàng trong xưởng/chợ, doanh nghiệp, ngân khố, tổng tiền, invariant và save/load cuối. Mỗi lần chạy đạt **23 checks**; chạy ba lần, gồm một lần truyền đường dẫn runner tương đối và hai lần tuyệt đối, đều khớp. Lần gọi ban đầu trước khi chuẩn hóa đường dẫn runner gặp lệch dữ liệu; script được sửa để phân giải đường dẫn thành tuyệt đối trước khi gọi C++ child process, không thay source gameplay, và ba lượt sau đều pass.

## Kiểm tra đã chạy

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/windows --config Release --target polyec_market_manager_tests polyec_realm_tests polyec_establishment_transfer_tests
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-python-ai --config Release --target polyec_env
& .\build\windows\core\Release\polyec_market_manager_tests.exe
& .\build\windows\core\Release\polyec_realm_tests.exe
& .\build\windows\core\Release\polyec_establishment_transfer_tests.exe
$env:PYTHONPATH = (Resolve-Path build/m5-python-ai/Release).Path
python ai/tests/market_transaction_binding_checks.py --replay build/windows/core/Release/polyec_establishment_transfer_tests.exe
```

Build core/target và Python extension hoàn tất; các runner C++ lần cuối thoát mã `0`; binding smoke đạt 23 checks ở cả ba lượt lặp. Trong lần biên dịch đầu, C++ phát hiện hai call site thu thuế pha cư dân vẫn gọi hàm cũ đã bị gỡ; tôi khôi phục hàm đó dưới dạng dùng `MarketManager` và thêm preflight cho pha cư dân. Khi review sau lần chạy đầu, tôi cũng thấy điều kiện tuổi trong nhánh Buy mới viết bị đảo cho người không phải Civilian; đã sửa để đúng với biểu thức cũ và chạy lại cả ba target. Kết quả 21.086 checks trong báo cáo là lần chạy sau sửa này.

Không chạy full CTest, Debug, native hoặc Godot. Python binding được build lại và smoke qua đúng C++ core cho chuỗi chợ nêu trên; chưa chạy toàn bộ Realm replay/adapter M5, observation/action/reward parity trên mọi luật mới, hoặc inference. Không đổi API/binding code, schema save, action, observation hay reward; chỉ thêm helper C++, kiểm tra C++/Python và chuỗi xưởng/chợ. Tự review, chưa có reviewer độc lập; không commit/push.

## Phần còn thiếu

Đây là sửa độ bền số học của thị trường đang có và một smoke chia sẻ core cho đúng chuỗi này. Nó không bổ sung điều chỉnh giá theo cầu/tồn kho, ngân sách nhu cầu nhiều chủ thể, ưu tiên hợp đồng, nguồn cung cho hàng deferred, logistics giữa vùng, UI thị trường, policy đã học hoặc đánh giá RL. Vì vậy không đóng M5.2 hay M5; smoke riêng lẻ này cũng không chứng nhận toàn adapter M5, native hoặc Godot trên source hiện tại.

Kế hoạch: [M5-market-checked-arithmetic-plan](../design/M5-market-checked-arithmetic-plan.md). Hash nguồn, tài liệu và binary đã kiểm ở [manifest](../../artifacts/m5-market-checked-arithmetic-2026-10-22-manifest.json).
