# M5.0/1 — quỹ hộ tách biệt qua kết hôn và thừa kế: kết quả kiểm tra

Ngày 2026-10-22. Đây là kiểm tra nối các luật đã có của quỹ hộ, đồng thuận kết hôn và xử lý di sản. Không thêm luật gameplay mới.

## Người chơi sẽ thấy gì

Khi hai người từ hai hộ khác nhau kết hôn, họ vẫn thuộc hai hộ riêng. Tiền trong quỹ chung của mỗi hộ tiếp tục ở đúng tài khoản đó. Một người phối ngẫu không thể dùng lệnh rút tiền để lấy quỹ của hộ kia. Nếu một người chết, tài sản cá nhân của họ được xử lý theo quy trình di sản đang có; quỹ hộ vẫn đứng riêng và không bị cộng vào di sản cá nhân.

## Chuỗi đã kiểm tra

Fixture C++ tạo hai hộ khác nhau rồi cho thành viên tương lai của mỗi hộ nộp lần lượt 7 và 11 đơn vị tiền cá nhân vào quỹ hộ bằng `HouseholdFundCommand`. Tổng tiền không đổi sau hai lần nộp và Realm vẫn đạt invariant kế toán.

Người thứ nhất cầu hôn người thứ hai qua lệnh Realm. Tiền ký quỹ 50 được giữ trong tài khoản chờ; chưa có quan hệ vợ chồng trước khi người nhận chấp thuận. Thế giới được lưu rồi tải lại khi lời cầu hôn đang chờ. Sau khi chấp thuận, quan hệ vợ chồng được ghi nhận, khoản ký quỹ được thanh toán đúng một lần, nhưng hai mã hộ và hai số dư quỹ 7/11 vẫn tách biệt.

Sau kết hôn, người thứ hai thử rút 1 từ quỹ của hộ thứ nhất. Realm từ chối lệnh và fingerprint, số dư hai quỹ không đổi. Người thứ nhất rút 3 từ quỹ của chính hộ mình: số dư hộ đó còn 4, quỹ hộ kia vẫn là 11, tiền cá nhân người rút tăng đúng 3 và tổng tiền toàn thế giới không đổi.

Tiếp đó người phối ngẫu thứ hai được đánh dấu qua đời trong pha xử lý cư dân của Realm. Quan hệ lịch sử được giữ lại; hộ của người còn sống vẫn có quỹ 4, hộ của người chết vẫn có quỹ 11, và hồ sơ di sản không nhận các khoản quỹ chung này. Các bước lưu/tải tiếp theo giữ fingerprint như trước.

## Thay đổi mã và kiểm tra

Chỉ mở rộng fixture và assertions trong `core/tests/test_realm_court.cpp`; không đổi mã sản phẩm, API, save schema, binding, hành vi Python/native/Godot hoặc model. Kế hoạch giới hạn file và phép kiểm ở [M5-household-funds-marriage-continuity-plan.md](../design/M5-household-funds-marriage-continuity-plan.md).

Windows Release đã build target `polyec_court_tests` thành công và chạy trực tiếp executable: **546 checks passed**. Lần build ban đầu bắt lỗi khai báo trùng tên trong fixture; đã sửa bằng cách dùng lại household ID đã có rồi build và chạy lại thành công. Đây là runner Court có các kiểm tra proposal, escrow và hôn nhân; không chạy full CTest Release/Debug. Không chạy adapter vì production source/API/schema/binding không đổi. Link checker xác nhận tham chiếu local sau khi viết report. Không có reviewer độc lập; đây là tự review.

## Giới hạn còn lại

Kết quả này chứng minh một chuỗi cụ thể về kết hôn, quỹ hai hộ, tử vong và lưu/tải. Nó không thêm quy tắc chuyển thành viên giữa các hộ, không cho phép người phối ngẫu hoặc guardian quản lý tài sản người khác, không giải quyết đồng sở hữu đất/xưởng của nhiều người hưởng và không đóng M5.0/1. Các luật đó vẫn cần contract cụ thể trước khi được triển khai.
