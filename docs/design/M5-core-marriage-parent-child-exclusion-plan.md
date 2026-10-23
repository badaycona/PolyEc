# Kế hoạch — từ chối kết hôn giữa cha/mẹ và con

Ngày 2026-10-22. `IMPLEMENTATION_PLAN.md` §2 và contract consent/escrow cấm kết hôn giữa hai người có quan hệ cha-con trực hệ. `core/tests/test_realm_court.cpp` đã kiểm cặp anh chị em trưởng thành và trường hợp người chưa thành niên, nhưng chưa có assertion riêng cho cặp cha-con.

## Thay đổi dự kiến

- Chỉ thêm phép kiểm trong khối eligibility của `core/tests/test_realm_court.cpp`.
- Lấy một cạnh `RelationKind::ParentChild` đã có trong fixture Realm, chọn hai đầu còn sống và đều đã đủ 18 tuổi, rồi gửi lệnh kết hôn thử qua action mask. Phép kiểm chỉ dựa vào loại quan hệ typed hiện có; không tuyên bố cạnh khởi tạo đó là parentage hợp pháp.
- Chờ action mask từ chối đề nghị giữa họ. Không đổi production, quy tắc hôn nhân, API, save, binding, model hoặc reward.

## Kiểm tra

- Build và chạy trực tiếp `polyec_court_tests` Windows Release và Debug.
- Không chạy full CTest/adapters vì chỉ thêm assertion Court; không đổi implementation hoặc bề mặt gọi.
- Hoàn tất khi fixture xác nhận có hai người trưởng thành với cạnh ParentChild và action mask không cho gửi đề nghị giữa họ.

Tự review plan; chưa có reviewer độc lập.
