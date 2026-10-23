# Kế hoạch — Realm ưu tiên phối ngẫu trước cha mẹ khi không có con hợp pháp

Ngày 2026-10-22. `IMPLEMENTATION_PLAN.md` §2 và hợp đồng người hưởng di sản quy định hàng còn sống đầu tiên theo thứ tự con hợp pháp → phối ngẫu → cha mẹ → anh chị em nhận toàn bộ phần chia. Các manager test đã kiểm thứ tự; test Realm hiện có phối ngẫu nhưng không đặt người ở hàng cha mẹ để chứng minh hàng cha mẹ bị bỏ qua.

## Thay đổi dự kiến

- Chỉ mở rộng fixture `prepare_estate_claim_death` và test Realm death đang dùng fixture đó trong `core/tests/test_realm.cpp`.
- Fixture giữ một phối ngẫu còn sống và thêm một cha/mẹ còn sống với cạnh cha-con hợp pháp. Không thêm con, khoản nợ, tài sản hoặc thay đổi luật.
- Trước pha cư dân ghi số dư tiền/hàng của cha/mẹ. Sau khi người chết được quyết toán, phối ngẫu phải nhận phần còn lại theo đường hiện có; cha/mẹ không nhận tiền/hàng. Tổng tiền, hồ sơ, khoản vay, title/xưởng, invariant và save/load tiếp tục được kiểm như trước.
- Không đổi production, API, save schema, binding, action, observation, reward, model hoặc dữ liệu huấn luyện.

## Kiểm tra

- Build và chạy trực tiếp `polyec_realm_tests` Windows Release và Debug vì chỉ Realm fixture thay đổi.
- Không chạy ALL_BUILD, full CTest hoặc Python/native/Godot: không đổi production hay bề mặt gọi. Sau khi nhóm estate core ổn định, tích hợp adapters vẫn là gate riêng trước khi gọi M5 hoàn tất.
- Kết thúc lát này khi cùng Realm death phase chứng minh phối ngẫu nhận và cha/mẹ còn sống không nhận, với bảo toàn tiền và save/load.

Tự review plan; chưa có reviewer độc lập.
