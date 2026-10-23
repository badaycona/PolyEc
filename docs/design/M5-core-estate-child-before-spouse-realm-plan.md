# Kế hoạch — con hợp pháp nhận di sản trước phối ngẫu trong Realm

Ngày 2026-10-22. `IMPLEMENTATION_PLAN.md` §2 quy định hàng con hợp pháp đứng trước phối ngẫu. Manager test đã kiểm thứ tự, nhưng Realm death fixtures hiện kiểm con và phối ngẫu riêng rẽ. Lát này nối đúng trường hợp có đồng thời con hợp pháp và phối ngẫu còn sống.

## Thay đổi dự kiến

- Chỉ mở rộng fixture `prepare_estate_fallback_death` và thêm test trong `core/tests/test_realm.cpp`.
- Thêm tier fixture riêng: người chết có đúng một con hợp pháp còn sống (đặt tuổi 10 năm) và một phối ngẫu trưởng thành. Không thêm cha mẹ, khoản vay, title hoặc xưởng.
- Chuẩn bị 100 tiền và 5 Paper bằng chuyển nguồn hiện hữu. Sau death phase, con nhận toàn bộ tiền/hàng; tiền và Paper của phối ngẫu giữ nguyên. Kiểm tra tổng tiền, hồ sơ, invariant và save/load.
- Không thêm luật giám hộ, không cấp quyền cho guardian và không phân chia tài sản khác. Không sửa production, API, schema, binding, model hoặc reward.

## Kiểm tra

- Build và chạy trực tiếp `polyec_realm_tests` Windows Release và Debug.
- Không chạy ALL_BUILD/full CTest/adapters vì chỉ thêm test Realm; full CTest và adapters vẫn là nhóm tích hợp riêng khi có thay đổi production hoặc đạt mốc ổn định tương ứng.
- Kết thúc lát khi cùng death pipeline trao di sản cho con hợp pháp trước phối ngẫu, giữ nguyên tài sản của phối ngẫu và bảo toàn Realm qua save/load.

Tự review plan; chưa có reviewer độc lập.
