# Bằng chứng tích hợp core — di sản, quỹ hộ và chức bổ nhiệm

Ngày 2026-10-22, cập nhật sau khi bổ sung thứ tự người hưởng và vòng đời kết hôn lại sau khi phối ngẫu qua đời. Mốc này gom các kiểm tra test-only quanh chu trình vòng đời: quỹ hộ tách khỏi tài sản cá nhân; người đủ tuổi mới có authority; Realm phân phối di sản theo thứ tự; chức bổ nhiệm không được truyền và chỗ mở lại sau tử vong; từ chức không thu hồi tiền riêng; kết hôn mới vẫn cần escrow và chấp thuận. Quy tắc và giới hạn từng trường hợp nằm trong báo cáo theo lát tại [CURRENT_TASK](../CURRENT_TASK.md).

## Kết quả kiểm tra

- Windows Visual Studio Release full core CTest: **19/19 pass trong 38,54 giây**, bao gồm hai assertion Realm mới về thứ tự con → phối ngẫu → cha mẹ và kiểm tra người góa tái hôn sau death phase. CTest chạy core, Realm, court, land title, household fund, worksite payroll, production, clock, schedule, campaign, manager, configuration và replay. Không chạy ALL_BUILD; CTest dùng các executable đã build.
- Windows Visual Studio Release `polyec_realm_tests`: **20.741 checks**, profile A 10.000 người/19.743 quan hệ thưa/90 ngày, 5,77 giây cho profile.
- Windows Visual Studio Debug `polyec_realm_tests`: **20.741 checks**, cùng profile A, 23,02 giây cho profile.
- Windows Visual Studio Release và Debug `polyec_court_tests`: mỗi cấu hình **537 checks**, bao gồm chuỗi người góa đề nghị và chấp thuận hôn nhân mới sau khi phối ngẫu cũ chết.
- `py -3.12 scripts/check-repository-links.py`: **1.062** tham chiếu tài liệu/tài nguyên nội bộ hợp lệ.
- `git diff --check` trong các file của những lát liên quan không báo lỗi khoảng trắng; Git chỉ cảnh báo quy tắc chuẩn hóa CRLF/LF ở test và tài liệu.

## Giới hạn

Full CTest Debug 19/19 gần nhất chạy trước các fixture mới; sau đó Realm Debug đạt 20.741 checks và Court Debug đạt 537 checks khi chạy trực tiếp. Không chạy Python/native/GDExt/Godot ở mốc này vì chỉ thêm test, không sửa production source, schema, API, binding, observation/action, reward hoặc model. Bằng chứng này xác nhận full core C++ Release cùng hai runner Debug có thay đổi; nó không chứng nhận M5 hoàn chỉnh, learned policy đạt chất lượng, tích hợp UI, profile B, hiệu năng toàn chu trình hoặc package Windows.

Tự review; không có reviewer độc lập. Không commit, push hoặc publish. [Manifest hash](../../artifacts/m5-core-estate-office-lifecycle-integration-2026-10-22-manifest.json) lưu checkpoint của mã test và tài liệu.


