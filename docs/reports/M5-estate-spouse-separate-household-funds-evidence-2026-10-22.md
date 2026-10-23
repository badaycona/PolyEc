# M5.0/1 — di sản của người chết không nhập quỹ hộ của người phối ngẫu

Ngày 2026-10-22. Lát này nối trả khoản vay, chia tiền cá nhân cho người phối ngẫu và giữ riêng hai quỹ hộ qua pha tử vong Realm. Không thêm luật gameplay mới.

## Hành vi đã được đối chiếu

Fixture có một người chết với 1.000 tiền cá nhân, khoản vay 20 và quỹ hộ 37. Người phối ngẫu là người hưởng duy nhất còn sống trong tầng ưu tiên hiện hành; họ ở household label khác và quỹ hộ của họ có 23. Trước pha tử vong, fixture chuyển một DryFood từ kho Province vào kho cá nhân người phối ngẫu. Điều đó ngăn việc mua thức ăn trong ngày làm thay đổi quỹ và giúp đối chiếu chính xác phần di sản.

Khi Realm chạy pha cư dân, 20 tiền được trả cho khoản vay từ hồ sơ của người chết. Người phối ngẫu nhận 980 tiền cá nhân còn lại. Quỹ 37 của household người chết và quỹ 23 của household người hưởng giữ nguyên; cả hai đều không bị đưa vào di sản. Tổng tiền thế giới và invariant kế toán vẫn đúng. Save/load tiếp tục giữ hai quỹ, hồ sơ, tiền người hưởng và fingerprint.

## Thay đổi và phép kiểm

Chỉ thêm helper fixture để tách household label và chuyển một món DryFood từ kho Province; mở rộng `test_estate_keeps_household_fund_outside_personal_claim()` trong `core/tests/test_realm.cpp`. Production code không đổi. Kế hoạch và phạm vi nằm ở [M5-estate-spouse-separate-household-funds-plan.md](../design/M5-estate-spouse-separate-household-funds-plan.md).

Windows Release đã build target `polyec_realm_tests` và chạy trực tiếp: **20.744 checks passed**. Runner đồng thời kiểm profile A gồm 10.000 người, 19.743 quan hệ thưa trong 90 ngày; đây không phải profile B. Lần đầu oracle thất bại vì người phối ngẫu ăn trong ngày và luật hiện có dùng quỹ hộ để mua DryFood. Sau khi fixture cấp món ăn sẵn từ kho Province, kiểm tra pass. Điều này xác nhận chi phí ăn là hoạt động bình thường và không phải lỗi di sản. Không chạy full CTest, Debug hoặc adapter vì lát chỉ thay đổi test/fixture. Tự review; không có reviewer độc lập.

## Giới hạn còn lại

Phép kiểm chứng minh một người phối ngẫu duy nhất nhận tiền cá nhân sau khi trả nợ trong khi hai quỹ hộ tách biệt. Nó không giải quyết các tài sản đất/xưởng khi có nhiều người hưởng, guardian quản lý tài sản, toàn bộ chu trình gameplay M5 hoặc tích hợp Python/native/Godot. M5.0/1 vẫn còn việc mở.
