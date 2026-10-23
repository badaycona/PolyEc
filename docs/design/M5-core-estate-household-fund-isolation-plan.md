# Kế hoạch M5 — giữ quỹ hộ ngoài hồ sơ di sản

Ngày 2026-10-22. Contract đã ghi trong [plan ledger di sản](M5-core-estate-ledger-plan.md) và `IMPLEMENTATION_PLAN.md` §2: tiền quỹ hộ là tài khoản chung, tách khỏi tiền riêng và không chuyển vào hồ sơ của một người chết. Lát này chỉ bổ sung Realm integration test cho điều kiện đó.

## Hành vi cần chứng minh

- Một Realm hợp lệ có household fund dương và một thành viên trong hộ chết khi còn cash riêng, hàng, khoản vay, đất và xưởng.
- Khi pha tử vong gom cash/hàng cá nhân vào estate, khoản vay được quyết toán và tiền còn lại chia cho người hưởng hiện hành, số dư quỹ hộ vẫn giữ nguyên.
- Số tiền người hưởng nhận chỉ bằng phần thuộc tài sản cá nhân của người chết; household fund không làm tăng khoản thừa kế.
- Realm giữ invariant và tổng tiền. Save/load giữ đúng số dư quỹ hộ và hồ sơ di sản.

Không thêm luật về ai được rút quỹ hộ, không thay đổi household membership, beneficiary priority, schema, binding hoặc source gameplay. Quỹ được dựng trong fixture bằng cách chuyển một khoản có thật từ Province firm sang sparse household account để tổng tiền không đổi.

## File và kiểm tra

- `core/tests/test_realm.cpp`: fixture nhỏ và một test tử vong → claim → phân phối → save/load.
- Build/chạy Release `polyec_realm_tests.exe`. Test runner cũng chạy workload profile A 10.000 người/19.743 quan hệ thưa/90 ngày, không phải profile B.
- Full core Release CTest 19/19 và Debug CTest 19/19 vừa qua trước test-only addition; không lặp cả suite. Không đổi production source/schema/API/binding nên Python/native/GDExt/Godot không chạy lại.
- So oracle tiền trước/sau độc lập; `invariant()` và fingerprint roundtrip là kiểm tra bổ sung.
