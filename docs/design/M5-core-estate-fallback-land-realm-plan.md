# Kế hoạch M5 — title đất theo hàng thừa kế dự phòng qua Realm

Ngày 2026-10-22. Contract nằm trong [kế hoạch chuyển title đất trong di sản](M5-core-estate-land-title-plan.md): sau khi nghĩa vụ được trả, title chỉ chuyển khi hàng người hưởng hợp pháp có đúng một người. Nhiều người cùng hàng hoặc không có người hưởng thì holding/quyền sử dụng tiếp tục được tham chiếu trong hồ sơ; không chia holding và không tự chuyển đất vào quỹ công.

Nhóm estate vừa kiểm tra qua Realm ba nhánh tiền/hàng: cha mẹ, anh chị em và không có người hưởng. Lát này nối title và giấy phép dùng đất vào chính các nhánh đó.

## Phạm vi

- Chỉ thêm fixture/test trong `core/tests/test_realm.cpp`, plan, report, CURRENT_TASK, DEVELOPMENT_HISTORY và manifest.
- Dùng ba trường hợp đã có: hai cha/mẹ trưởng thành cùng hàng; một anh/chị/em trưởng thành duy nhất khi cha/mẹ chung đã chết; không còn thân nhân hợp pháp còn sống.
- Mỗi người chết có một holding và một quyền dùng đất do một người còn sống khác nắm. Kiểm title-holder, grantor, grantee và tham chiếu trong EstateCase sau pha Realm.
- Kỳ vọng: title không chia cho hai cha/mẹ và ở lại hồ sơ; title chuyển cho người anh/chị/em duy nhất cùng với grantor của quyền hiện hữu; không có heir thì title và quyền tiếp tục ở hồ sơ, không chuyển thành tài sản công.
- Mỗi nhánh save/load phải giữ đúng holding, quyền sử dụng, hồ sơ, fingerprint và invariant.
- Không sửa production hoặc schema. Không thêm định giá, phí, chia diện tích hay quyền guardian.

## Kiểm tra

- Build và chạy trực tiếp `polyec_realm_tests` Windows Release và Debug; mỗi cấu hình đạt **20.716 checks**, profile A 10.000 người/19.743 quan hệ thưa/90 ngày.
- Không chạy lại full CTest hay adapters vì đây là Realm test-only change. Ghi khoảng cách revision của full CTest gần nhất.
- Tự review bằng đối chiếu theo stable ID và save/load; không có reviewer độc lập.
