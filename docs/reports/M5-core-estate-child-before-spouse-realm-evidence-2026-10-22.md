# Bằng chứng M5 — con hợp pháp nhận di sản trước phối ngẫu trong Realm

Ngày 2026-10-22. Quy tắc tại [IMPLEMENTATION_PLAN.md §2](../IMPLEMENTATION_PLAN.md) chọn hàng thân nhân còn sống đầu tiên theo thứ tự: con hợp pháp, phối ngẫu, cha mẹ, rồi anh chị em. Manager đã kiểm thứ tự; lát này chạy trường hợp có cả một người con hợp pháp chưa thành niên và một phối ngẫu còn sống qua death phase thật của Realm.

## Tình huống và kết quả

Fixture C++ chuẩn bị người chết cùng một con 10 tuổi có cạnh cha-con đã được xác nhận hợp pháp, và một phối ngẫu trưởng thành còn sống. Người chết có 100 tiền cá nhân và 5 Paper trong hồ sơ di sản. Các đơn vị tiền và Paper được chuyển từ tài khoản/kho hiện có trong fixture; không tạo tiền hoặc hàng mới. Trường hợp này không có khoản vay, title đất hoặc xưởng cần chuyển.

Test ghi lại tiền và Paper của con và phối ngẫu trước khi gọi pha cư dân. Sau khi người lớn chết và hồ sơ được phân phối, con nhận đủ 100 tiền và 5 Paper; số tiền và Paper của phối ngẫu giữ nguyên. Tiền/hàng trong hồ sơ về 0, tổng tiền không đổi và Realm invariant vẫn đúng. Save/load giữ fingerprint, hồ sơ di sản và cả số dư của hai người.

Với người chơi, điều này có nghĩa là nếu người chết còn con hợp pháp thì tài sản được chia theo hàng con trước; phối ngẫu còn sống không cùng nhận phần của hàng ưu tiên thấp hơn. Lát này chỉ kiểm việc chuyển tiền và hàng qua luật đã có. Nó không cấp cho người giám hộ quyền chi tiền, quản lý xưởng hoặc ký thay trẻ.

## Thay đổi và kiểm tra

- Mở rộng tier fixture và thêm một assertion trong `core/tests/test_realm.cpp`; không đổi mã gameplay, model, API, save schema hoặc binding.
- Windows Visual Studio Release: build `polyec_realm_tests` thành công; chạy trực tiếp **20.741 checks**. Profile A gồm 10.000 người, 19.743 quan hệ thưa, 90 ngày và mất 5,77 giây ở phần profile.
- Windows Visual Studio Debug: build và chạy cùng target thành công, **20.741 checks**. Profile A cùng quy mô mất 23,02 giây ở phần profile.
- Full Release CTest 19/19 gần nhất chạy trước assertion này; không tính nó là có chứa test mới. Không chạy full CTest hoặc Python/native/GDExtension/Godot vì chỉ thêm fixture Realm và assertion, không đổi production/API/schema/binding/action/observation/reward/model.
- Tự review đối chiếu quan hệ hợp pháp, số dư trước/sau của cả hai người, tiền/hàng trong hồ sơ, tổng tiền, invariant và save/load. Chưa có reviewer độc lập.

Đây là bằng chứng cho đúng một nhánh ưu tiên người hưởng qua Realm, không phải nghiệm thu toàn bộ estate hay M5. [Plan](../design/M5-core-estate-child-before-spouse-realm-plan.md) và [manifest](../../artifacts/m5-core-estate-child-before-spouse-realm-2026-10-22-manifest.json) lưu các file/hash của lát này. Không commit, push hoặc publish.
