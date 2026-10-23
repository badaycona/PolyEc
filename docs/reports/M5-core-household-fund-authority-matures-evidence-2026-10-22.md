# Bằng chứng M5 — quỹ hộ mở lại quyền rút khi người sống duy nhất đủ 18 tuổi

Ngày 2026-10-22. Plan: [quyền authority khi người phụ thuộc trưởng thành](../design/M5-core-household-fund-authority-matures-plan.md). Tài khoản chung chỉ cho người lớn hiện là `household_authority` rút. Lát này nối tiếp trường hợp quỹ bị khóa vì hộ chỉ còn trẻ vị thành niên.

## Chuỗi đã kiểm

Fixture chuyển **37 tiền** từ Province firm vào quỹ hộ, chuyển **2 DryFood** từ kho Province sang hành trang người con, rồi đặt người này ở tuổi `18 * 365 - 3` ngày. DryFood được chuyển trước ngày kiểm bằng cách trừ kho nguồn, nên không tạo hàng mới. Fixture chọn ID không đến lượt tiêu thụ thực phẩm trong ba ngày sắp kiểm.

Pha cư dân đầu tiên tăng tuổi người con thêm một ngày, làm người lớn trong hộ chết và để lại người con vẫn dưới 18. Khi đó Realm không chọn ai làm authority; quỹ còn 37. Realm chạy thêm đúng hai ngày game, tuổi thành `18 * 365`; truy vấn authority tự trả ID người con. Hai món DryFood vẫn còn trong hành trang và quỹ hộ vẫn 37. Người vừa trưởng thành gửi `HouseholdFundCommand` rút 5: cash cá nhân tăng 5, quỹ còn 32, tổng tiền và invariant giữ nguyên. Save/load giữ tuổi, authority, 32 tiền chung, cash cá nhân và fingerprint.

Người chơi sẽ thấy tài khoản hộ tạm thời không có ai được rút nếu chỉ còn trẻ vị thành niên; khi thành viên còn sống đủ 18 game years, Realm nhận diện người đó là người lớn đại diện cho hộ và cho phép họ tự gửi lệnh rút theo quyền thông thường. Test không tạo guardian được quyền dùng tiền thay người khác.

## Kiểm tra và giới hạn

- Windows Visual Studio Release: build target `polyec_realm_tests` pass; chạy trực tiếp đạt **20.721 checks**, profile A 10.000 người/19.743 quan hệ thưa/90 ngày.
- Windows Visual Studio Debug: build và chạy trực tiếp cùng target; đạt **20.721 checks**. Toàn executable mất khoảng **216 giây** theo thời gian chờ tiến trình; phần profile 90 ngày báo 22,39 giây.
- Full core CTest Release/Debug 19/19 có trước các bổ sung test-only gần đây. Lát này chỉ thêm fixture/test Realm; không chạy adapters do production, save schema, API, binding và model không đổi.
- Tự review bằng ranh giới tuổi, authority query, lệnh command thật, đối chiếu cash/quỹ/tổng tiền, DryFood có nguồn và save/load. Không có reviewer độc lập.

Đây là kiểm tra Realm cho quy tắc trưởng thành và quyền đại diện; không định nghĩa khoản trợ cấp, chi hộ cho trẻ hoặc quyền guardian đối với tiền riêng. M5 vẫn mở. [Manifest hash](../../artifacts/m5-core-household-fund-authority-matures-2026-10-22-manifest.json) ghi checkpoint mã test và tài liệu. Không commit, push hoặc publish.
