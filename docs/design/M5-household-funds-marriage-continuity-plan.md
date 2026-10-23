# M5.0/1 — quỹ của hai hộ vẫn tách biệt sau khi kết hôn

## Mục tiêu và căn cứ

Nối hai luật đã có thành một kiểm tra qua Realm thật. Contract hôn nhân trong `M5-core-marriage-consent-plan.md` quy định kết hôn không nhập hộ hoặc chuyển tài sản. Contract quỹ hộ trong `M5-T1-household-fund-account-plan.md` quy định mỗi hộ có một tài khoản chung riêng. Vì vậy, việc hai người chấp thuận kết hôn không được nhập hai số dư quỹ hoặc làm một người rút được tiền của hộ kia.

## Phạm vi

- Chỉ sửa fixture và assertions trong `core/tests/test_realm_court.cpp`.
- Dùng `HouseholdFundCommand` để nộp tiền thật vào quỹ của mỗi người trước lời cầu hôn.
- Chạy lời cầu hôn, save/load khi đang chờ, chấp thuận, rồi dùng lệnh Realm rút một phần từ đúng quỹ của người rút.
- Kiểm tra số dư hai quỹ, số dư cá nhân, escrow, tổng tiền, quan hệ vợ chồng, invariant và save/load.
- Không sửa production, luật hôn nhân, quyền hộ, action/observation/model, save schema hoặc Python/native/Godot.

## Kiểm tra ngay

Build target `polyec_court_tests` Release và chạy trực tiếp runner. Cùng target là nơi đang kiểm lời cầu hôn, ký quỹ, chấp thuận và tính nguyên vẹn của quan hệ. Vì lát này chỉ mở rộng fixture test, không cần chạy toàn bộ CTest hoặc adapters. Sau mốc ổn định M5.0/1, chu trình hộ–hôn nhân–di sản sẽ được nối tiếp trong kiểm tra Realm rộng hơn.

## Giới hạn báo cáo

Kết quả này chỉ chứng minh hai tài khoản hộ còn riêng qua một chuỗi hôn nhân cụ thể; không chứng minh đã có thủ tục đổi thành viên hộ, chuyển tài sản giữa hai hộ, hoặc toàn bộ M5.0/1 đã hoàn tất. Tự review trong cùng lượt; không có reviewer độc lập.
