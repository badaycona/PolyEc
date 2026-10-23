# Kế hoạch M5 — chức bổ nhiệm trở về cơ quan khi người giữ chức qua đời

Ngày 2026-10-22. Contract nguồn: [IMPLEMENTATION_PLAN.md §2](../IMPLEMENTATION_PLAN.md), hàng “Tước/chức”: chức được bổ nhiệm quay về cơ quan, không tự truyền theo tước/người thừa kế. Realm hiện đếm số chức theo các appointee còn sống và có `hired_by` trỏ đến người bổ nhiệm; cần kiểm chứng qua death phase rằng vị trí chết không còn chiếm một chỗ và có thể bổ nhiệm người khác.

## Hành vi và phép kiểm

1. Dùng Realm command thật để một sovereign bổ nhiệm ba người trưởng thành, lấp đủ ba chỗ hiện có.
2. Lệnh bổ nhiệm người thứ tư bị từ chối và để nguyên fingerprint/cash/appointments.
3. Một appointee chết trong pha Realm. Hồ sơ `office_appointment` của người chết chỉ còn ở chính hồ sơ cá nhân; người đó bị loại khỏi số chức đang hoạt động.
4. Dùng lại cùng command để bổ nhiệm ứng viên thứ tư. Lệnh thành công, ứng viên mới có `hired_by` là sovereign; người chết không được tính là người giữ chức sống.
5. Save/load giữ appointments và slot đang dùng; một ngày tiếp theo chạy giống nhau giữa Realm gốc và bản load.

Không thay cơ chế lương, thông báo từ chức, số chỗ, quyền appointment hoặc cách xử lý người kế vị. Không chuyển appointment field sang beneficiary.

## Phạm vi và kiểm tra

- Chỉ thêm test fixture/Realm integration trong `core/tests/test_realm.cpp` và plan/report/CURRENT_TASK/HISTORY/manifest.
- Build và chạy trực tiếp `polyec_realm_tests` Windows Release rồi Debug. Không chạy full CTest/adapters: không đổi production, save schema, API, binding hoặc model; ghi rõ mốc CTest core trước các test-only additions.
- Tự review bằng state trước/sau reject, số người giữ chức sống, sequence dùng lại sau reject, ledger và save continuation; không có reviewer độc lập.
