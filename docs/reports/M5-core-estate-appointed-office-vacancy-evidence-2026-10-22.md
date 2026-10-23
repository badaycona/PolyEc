# Bằng chứng M5 — chỗ chức bổ nhiệm được mở lại sau khi người giữ chức qua đời

Ngày 2026-10-22. Kế hoạch: [chức bổ nhiệm trở về cơ quan](../design/M5-core-estate-appointed-office-vacancy-plan.md). [IMPLEMENTATION_PLAN.md §2](../IMPLEMENTATION_PLAN.md) quy định chức được bổ nhiệm không tự truyền cho người hưởng di sản; vị trí quay về cơ quan đã bổ nhiệm để cơ quan tiếp tục chọn người.

## Chuỗi Realm đã kiểm

Fixture dùng sovereign hiện có làm cơ quan bổ nhiệm, bốn ứng viên trưởng thành đang sống và ít nhất 80 tiền thật trong kho Treasury sau khi chuyển từ Province firm nếu cần. Không tạo tiền. Realm command bổ nhiệm ba ứng viên đầu; số người sống có `office=true` và `hired_by` trỏ sovereign là ba. Command thứ tư bị từ chối; fingerprint không đổi.

Ứng viên đầu được đặt sức khỏe về −1 qua fixture test rồi Realm chạy một game day. ID người này chết trong pha cư dân. Trên hồ sơ cá nhân của người chết, các trường appointment cũ vẫn còn như lịch sử; phép đếm vị trí đang hoạt động bỏ người chết nên số chức từ ba giảm về hai. Cùng command tiếp theo được dùng lại vì lệnh từ chối trước đó không tiêu thụ sequence; ứng viên thứ tư được bổ nhiệm thành công, trở lại ba vị trí có người sống. Vì beneficiary không được sao chép dữ liệu appointment và chỉ người đang sống được tính vào office capacity, chức không đi theo tài sản/thân nhân.

Save/load giữ fingerprint, ba appointment đang hoạt động và trường lịch sử của người chết. Sau khi tải, Realm gốc và bản tải chạy thêm một ngày; fingerprint, số appointment và tổng tiền tiếp tục khớp. Lương kỳ tiếp theo chưa tới hạn nên lần chạy này không trả lặp lương.

## Lượt sửa test

Lần compile đầu dừng ở test fixture vì thiếu include khai báo `EmploymentManager::kAppointmentFee`. Đã thêm include đúng header vào file test và build lại; không đổi production source. Realm Release cuối cùng đạt toàn bộ checks.

## Kiểm tra và giới hạn

- Windows Visual Studio Release: build target `polyec_realm_tests` pass; chạy trực tiếp đạt **20.730 checks**. Runner kèm profile A 10.000 người/19.743 quan hệ thưa/90 ngày; đây không phải profile B.
- Windows Visual Studio Debug: build và chạy trực tiếp cùng target; đạt **20.730 checks**. Toàn executable mất khoảng **233 giây** theo thời gian chờ tiến trình; phần profile 90 ngày báo 22,61 giây.
- Full core CTest Release/Debug 19/19 có trước các bổ sung test-only gần đây. Không chạy lại full suite hoặc Python/native/GDExt/Godot vì lát này chỉ đổi Realm tests; production/API/schema/binding/model không đổi.
- Tự review bằng sequence/fingerprint trước-sau lệnh bị từ chối, đếm appointment của người còn sống, giao dịch bổ nhiệm thật, total money, save/load và tiếp tục một ngày. Không có reviewer độc lập.

Test này xác nhận số chỗ chức được bổ nhiệm được trả về sau khi người giữ chức chết; nó không triển khai toàn bộ hệ thống gia tộc/chức vụ kế vị, không làm chức vụ kế vị tước và không đóng M5. [Manifest hash](../../artifacts/m5-core-estate-appointed-office-vacancy-2026-10-22-manifest.json) ghi hash của mã test và tài liệu. Không commit, push hoặc publish.
