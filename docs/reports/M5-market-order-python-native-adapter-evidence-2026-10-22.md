# M5.2 — nối lệnh đặt mua vào Python và native runtime

Ngày 2026-10-22. Phạm vi của plan [M5 market-order adapter](../design/M5-market-order-python-binding-plan.md). Đây là một phần tiếp theo của lệnh mua có giữ tiền đã có trong C++ Realm; báo cáo không xác nhận toàn bộ thị trường, cả 16 nhóm hàng hoặc M5 đã hoàn tất.

## Người chơi và trainer nhận được gì

Lệnh đặt mua đã có trong C++ Realm từ trước. Người mua chọn vùng, một trong 27 mã hàng, số lượng và giá tối đa. Khi Realm nhận lệnh, tiền được giữ từ đúng tài khoản; lệnh đến hạn ở ngày game tiếp theo. Nếu mua được hàng, giao dịch dùng kho, giá và luật chợ của Realm. Phần tiền không dùng được hoàn về nguồn ban đầu. Lệnh còn mở có thể bị chủ lệnh hủy; khi hủy, tiền chưa dùng cũng trở lại đúng tài khoản.

Lần thay đổi này giúp Python gọi chính lệnh C++ đó thay vì phải viết một quy tắc mua thứ hai. Python có thể đọc mã lệnh, chủ thể, vùng, sản phẩm, thời hạn, lượng, giá trần và tiền đang giữ. Lời gọi native nhận actor, thao tác đặt/hủy, ID lệnh, mặt hàng, lượng, giá trần và fingerprint màn hình; fingerprint cũ bị từ chối trước khi Realm thay đổi. GDExtension cung cấp phương thức tương ứng và thêm danh sách lệnh đang mở cùng tổng tiền giữ vào snapshot để Godot có thể trình bày sau này.

Đây mới là đường gọi lệnh và đọc sổ lệnh. Chưa thêm nút/chọn hàng vào giao diện Godot, chưa có bundle model tương thích đang được cài để người chơi nạp Realm v11, và chưa có bằng chứng rằng learned policy đã học khi nào nên đặt mua.

## Thay đổi source

- `bindings/python/realm_binding.cpp` xuất enum `MarketOrderOperation` (`Place`, `Cancel`), hàm `market_order_command`, danh sách `market_orders()` và tổng `market_escrow`. Hàm gọi `Realm::execute(MarketOrderCommand{...})` rồi trả kết quả C++ và ID ổn định; Python không tính giá, giữ tiền, cấp hàng hoặc hoàn tiền.
- `ai/tests/market_order_binding_checks.py` tạo Realm C++ qua extension, đặt một lệnh dùng tiền cá nhân, đọc sổ lệnh, kiểm tiền giữ, thử hủy bằng sequence cũ, lưu/nạp lệnh và hủy đúng sequence để nhận tiền lại. Một lượng bằng không phải bị core từ chối mà không sửa fingerprint Realm.
- `native/include/polyec/realm_runtime.hpp` và `native/realm_runtime.cpp` thêm API đặt/hủy. Runtime tự lấy sequence kế tiếp từ quyết định hiện tại của actor; kiểm tra Realm đã nạp, ID actor, fingerprint tùy chọn và tràn sequence; sau đó chỉ gửi `MarketOrderCommand` vào C++ Realm. Hàm trả ID lệnh qua tham số đầu ra và đưa lý do từ chối của C++ qua `error()`.
- `native/realm_contracts.cpp` có kiểm tra Realm chưa nạp và fixture thành công cho trường hợp đặt, giữ tiền, so fingerprint cũ, hủy, hoàn tiền và invariant. Fixture thành công được biên dịch nhưng chưa được thực thi trong môi trường hiện tại vì kiểm tra nạp model dừng trước fixture.
- `gdext/polyec_session.cpp` xuất `realm_market_order(...)` dạng Dictionary (`success`, `error`, `order_id`, fingerprint khi thành công), kiểm tra miền số ở ranh giới Godot và thêm `market_orders`/`market_escrow` vào snapshot. Giao diện GDScript chưa gọi method mới.

## Kiểm tra đã chạy

1. **Core C++:** executable Release `polyec_market_order_tests` báo `market order checks=40`. Các kiểm tra chạm lệnh C++ không đổi trong lượt adapter này; chúng là oracle cho luật được Python/native gọi.
2. **Python:** build `polyec_env` Release đã hoàn tất trong lượt thay binding. Chạy `ai/tests/market_order_binding_checks.py` cho kết quả `market_order_binding_checks=13 order_id=164 held=16`. Các phép kiểm gồm tìm người mua đủ tiền, nhận lệnh với ID, trường chủ/vùng/mặt hàng/định lượng/giá, tiền cá nhân ký quỹ, bảo toàn tổng tiền, từ chối hủy cũ mà không đổi fingerprint, round-trip save/load, hủy từ đúng chủ và hoàn tiền, cùng từ chối số lượng 0.
3. **Native runtime:** build target Release `polyec_realm_contracts` thành công; CTest chạy đúng `runtime_contracts`, **1/1 test pass**. Log đầy đủ là `RUNTIME CONTRACTS: 48 checks passed; historical bundle rejected; v10 inference not run because no compatible trained bundle is installed`. Hai kiểm tra mới ở đầu chương trình xác nhận runtime chưa nạp Realm phải từ chối lệnh và đặt ID trả về về 0. Do đó test báo pass không có nghĩa là native đã đặt/hủy thành công: model v6 có trong checkout bị từ chối theo contract tensor hiện hành trước khi nhánh runtime thành công chạy. Fixture success/cancel/stale phía dưới đã được biên dịch nhưng không được thực thi.
4. **GDExtension:** build riêng target Windows Release `polyec` thành công và tạo DLL. Điều này kiểm tra mã C++ của GDExtension được biên dịch/liên kết; không phải Godot đã nạp extension hoặc giao diện chạy thật. `Get-Command godot*` không tìm thấy Godot runtime, vì vậy không chạy parser/headless smoke.

Không chạy `ALL_BUILD`, full CTest, Debug, replay Python/Godot, benchmark hoặc train RL; các phép đó không cần thiết để kiểm phần API mới. Chưa chạy Python kiểm giá dưới tiền, actor ngoài miền hay mã operation lạ trong lượt này; điều kiện core tương ứng vẫn thuộc C++ test và wrapper chuyển từ chối sang core.

## Phạm vi và việc còn thiếu

Python gọi thành công qua transaction C++ và kiểm chứng được lệnh/tiền trước-sau save/load. GDExtension compile với API mới. Native runtime có guard được chạy; native success/cancel vẫn chưa có kết quả runtime vì không có model bundle đúng observation/model contract. Godot chưa có màn hình tạo/hủy đơn; model policy tương thích, tích hợp UI và đánh giá policy là các việc riêng. Các sản phẩm có mã catalog nhưng chưa có nguồn hàng vẫn không tự sinh hàng, và không có quy tắc nào trong lượt này thay đổi hành vi sản xuất.

Đây là self-review: không có reviewer độc lập. Không commit, push hoặc publish.

SHA-256 của source, bài kiểm, extension đã build và hai native test executable được cố định trong [manifest](../../artifacts/m5-market-order-python-native-adapter-2026-10-22-manifest.json). Manifest gắn với đúng các file tại lần ghi bằng chứng này; không được dùng để chứng nhận các thay đổi source về sau.
