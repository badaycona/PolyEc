# Bằng chứng M5 — người từ chức giữ nguyên tiền cá nhân

Ngày 2026-10-22. Plan: [từ chức tự nguyện sau thời hạn báo trước](../design/M5-core-office-resignation-private-cash-plan.md). Đây là kiểm tra đường `Resign` đã có; không phải bãi nhiệm. Realm chưa có một lệnh bãi nhiệm riêng trong lát này.

## Chuỗi và kết quả

Court fixture chọn augment mở ứng viên hiện có. Sovereign dùng command Realm bổ nhiệm một người trưởng thành, trả lệ phí 20 từ Province Treasury vào cash riêng của người được bổ nhiệm. Lệnh từ chức trong thời hạn báo trước bị reject nguyên tử và giữ fingerprint. Realm chạy 30 ngày tới mốc notice; kỳ lương đến hạn được trả trước khi test chụp cash cá nhân ngay trước lệnh từ chức.

Người được bổ nhiệm tự gửi command `Resign`. Sau command, `family` trở lại Civilian, `office` thành false, `hired_by` thành 0. Cash ngay trước/sau lệnh giống hệt nhau và total money của Realm không đổi. Như vậy chi phí đã trả trước đó không bị thu hồi và Realm không tịch thu phần tiền riêng người này đã giữ. Save/load và continuation thêm 7 ngày vẫn so khớp trong court runner.

## Kiểm tra đã chạy

- Lệnh build đầu nhắm nhầm tên `polyec_realm_court_tests`; CMake không có target đó. Đã tra `core/CMakeLists.txt` và đổi sang target thực tế `polyec_court_tests`.
- Windows Visual Studio Release: build target `polyec_court_tests` pass; executable trực tiếp **522 checks passed**.
- Windows Visual Studio Debug: build cùng target pass; executable trực tiếp **522 checks passed**.
- Chỉ sửa test `core/tests/test_realm_court.cpp`; không sửa production, API, save, binding hoặc model. Không chạy lại full CTest/Python/native/GDExt/Godot vì các lớp khác không chịu tác động.
- Tự review bằng fingerprint của lệnh từ chối, role fields, cash chụp đúng thời điểm trước/sau lệnh hợp lệ và total money. Không có reviewer độc lập.

Đây chỉ là bằng chứng từ chức tự nguyện không thu hồi tiền cá nhân sau notice. Nó không chứng minh cơ chế bãi nhiệm hay các quyền/chức vụ M5 đã hoàn tất. [Manifest hash](../../artifacts/m5-core-office-resignation-private-cash-2026-10-22-manifest.json) ghi hash file thuộc lát này. Không commit, push hoặc publish.
