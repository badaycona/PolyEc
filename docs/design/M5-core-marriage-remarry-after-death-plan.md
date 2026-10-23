# Kế hoạch — người góa có thể kết hôn lại sau khi phối ngẫu qua đời

Ngày 2026-10-22. Contract trong [IMPLEMENTATION_PLAN.md §2](../IMPLEMENTATION_PLAN.md) yêu cầu không có phối ngẫu còn sống khác; plan hôn nhân consent ghi rõ hôn nhân độc quyền khi cả hai còn sống. Hôn nhân đã có test offer/accept, escrow, reject/expiry, schema và chặn hôn nhân thứ hai khi người phối ngẫu hiện tại vẫn sống. Còn thiếu xác nhận rằng death phase thật giải phóng điều kiện này.

## Thay đổi dự kiến

- Chỉ thêm test và friend test helper trong `core/tests/test_realm_court.cpp`.
- Sau khi Realm ghi nhận hôn nhân thứ nhất qua offer, chờ và acceptance, đặt người phối ngẫu kia vào trạng thái chết và chạy chính `Realm::day_people` death phase.
- Người còn sống phải nhận candidate mới, mở đề nghị thứ hai bằng escrow hiện có, lưu/tải đề nghị, rồi người nhận mới phải tự chấp thuận. Quan hệ với người trước được giữ làm lịch sử; sau chấp thuận chỉ còn một phối ngẫu còn sống. Tổng tiền, invariant và hộ không bị nhập phải đúng.
- Không sửa production, luật, API/action/model contract hoặc schema. Không thêm luật ly hôn, chia hộ hay kết hôn khi có phối ngẫu còn sống.

## Kiểm tra

- Build và chạy trực tiếp `polyec_court_tests` Windows Release và Debug.
- Không chạy full CTest/adapters vì chỉ thêm test Court và không đổi production/API/schema/binding; nhóm M5 adapters vẫn cần nghiệm thu trước khi tuyên bố hoàn tất.
- Hoàn tất lát này khi cái chết gỡ điều kiện “có phối ngẫu còn sống”, nhưng hôn nhân mới vẫn đòi offer/escrow/chấp thuận và save/load chính xác.

Tự review plan; chưa có reviewer độc lập.
