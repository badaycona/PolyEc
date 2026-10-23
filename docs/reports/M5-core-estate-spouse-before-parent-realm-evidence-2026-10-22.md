# Bằng chứng M5 — phối ngẫu nhận di sản trước cha mẹ trong Realm

Ngày 2026-10-22. Luật tại [IMPLEMENTATION_PLAN.md §2](../IMPLEMENTATION_PLAN.md) chia phần di sản còn lại cho hàng thân nhân đầu tiên còn sống theo thứ tự: con hợp pháp, phối ngẫu, cha mẹ, rồi anh chị em. Manager đã có phép kiểm chọn đúng hàng; báo cáo này bổ sung phép kiểm qua chính Realm khi người chết đồng thời còn một phối ngẫu và một cha/mẹ hợp pháp đang sống.

## Tình huống được kiểm

Test dùng Realm command/fixture hiện có để chuẩn bị người chết, phối ngẫu, khoản vay 20 tiền, 1000 tiền cá nhân trước khi cấp vốn cho xưởng, 5 Fiber trong kho cá nhân và một holding đất/xưởng. Tiền và Fiber được chuyển từ các tài khoản/kho có sẵn, không tạo thêm tài sản. Fixture xóa các cạnh quan hệ cũ của người chết để trường hợp này không bị ảnh hưởng bởi thân nhân không liên quan; sau đó tạo cạnh phối ngẫu và một cạnh cha-con hợp pháp, trong đó người chết là con. Không có con hợp pháp còn sống của người chết.

Trước khi chạy pha cư dân, test lưu số dư tiền và Fiber của người cha/mẹ. Realm xử lý tử vong, trả khoản vay từ hồ sơ trước, rồi phân phối. Kết quả xác nhận:

- Người phối ngẫu nhận tiền và Fiber còn lại từ hồ sơ sau khi trả khoản vay và xử lý phí theo lịch của Realm.
- Người cha/mẹ vẫn sống nhưng không nhận thêm tiền hoặc Fiber, vì hàng phối ngẫu còn người đủ điều kiện và đứng trước hàng cha/mẹ.
- Tiền mặt hồ sơ về 0; khoản vay ghi nhận đã trả 20; title đất, quyền sử dụng và xưởng tiếp tục đi theo nhánh người hưởng đã có.
- Tổng tiền, invariant của Realm, fingerprint, hồ sơ, khoản vay, đất và xưởng khớp sau save/load.

Người chơi sẽ thấy di sản đi tới phối ngẫu còn sống thay vì chia đồng thời cho phối ngẫu và cha/mẹ. Phép kiểm này không đưa ra quy tắc mới cho con nuôi, tranh chấp quan hệ, nhiều người cùng một hàng hoặc quyền guardian.

## Thay đổi và phạm vi kiểm tra

- Thay đổi chỉ ở fixture và assertion của `core/tests/test_realm.cpp`; không sửa luật sản phẩm hay mã xử lý Realm.
- Windows Visual Studio Release: build `polyec_realm_tests` thành công; chạy trực tiếp đạt **20.736 checks**. Phần profile A gồm 10.000 người, 19.743 quan hệ thưa và 90 ngày, báo 5,85 giây.
- Windows Visual Studio Debug: build cùng target thành công; chạy trực tiếp đạt **20.736 checks** với cùng profile A, phần profile báo 21,33 giây. Toàn executable mất khoảng bốn phút theo thời gian tiến trình.
- Không chạy full CTest hoặc Python/native/GDExtension/Godot ở lát này: chỉ thay test Realm, không đổi production, API, save schema, binding, action/observation, reward hoặc model. Mốc full Release CTest trước đó là 19/19; kết quả đó không bao gồm assertion mới này.
- Đối chiếu độc lập trong test gồm cạnh cha-con hợp pháp, người nhận đúng, số dư của cha/mẹ không đổi, tiền/hàng trong estate, khoản vay, bảo toàn tổng tiền, invariant và roundtrip save/load. Tự review; chưa có reviewer độc lập.

Đây là phần kiểm chứng thứ tự người hưởng qua death phase, không đóng toàn bộ thừa kế hoặc M5. [Kế hoạch](../design/M5-core-estate-spouse-before-parent-realm-plan.md) và [manifest](../../artifacts/m5-core-estate-spouse-before-parent-realm-2026-10-22-manifest.json) ghi phạm vi, kết quả và hash file. Không commit, push hoặc publish.
