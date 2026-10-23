# Bằng chứng M5 — cạnh cha-con chưa hợp pháp không được nhận di sản

Ngày 2026-10-22. Kế hoạch: [parentage không hợp pháp trong Realm death phase](../design/M5-core-estate-unverified-parentage-realm-plan.md). Contract trong IMPLEMENTATION_PLAN §2 cho hàng con **hợp pháp** được hưởng trước; chỉ có cạnh `RelationKind::ParentChild` chưa đủ, phải có cờ provenance hợp pháp.

## Tình huống và kết quả

Realm fixture có người chết với 100 tiền cá nhân và 5 Paper. Năm Paper được ghi nhận ở kho Province ban đầu rồi chuyển sang người chết; cash được chuyển từ Province firm sang cá nhân. Một người trưởng thành khác còn sống có cạnh ParentChild trỏ về người chết, nhưng `legal_parentage=false`. Không có khoản vay hoặc người hưởng hợp pháp nào khác.

Sau Realm death phase, người có cạnh không hợp pháp không nhận tiền hoặc Paper. Hồ sơ người chết đã phân phối tiền/hàng nên hai phần này bằng 0; balance di sản tổng hợp giữ thêm 100 tiền trong quỹ di sản công; kho Province của vùng người chết tăng 5 Paper. Tổng tiền và invariant được bảo toàn. Save/load giữ fingerprint, hồ sơ và trạng thái.

Điều này có nghĩa một quan hệ gia đình được ghi nhận ở mức thông tin nhưng chưa được xác lập hợp pháp không tự biến thành quyền nhận tài sản. Test không thêm quy tắc về nhận con nuôi, con ngoài hôn nhân, giám hộ hay tranh chấp parentage.

## Kiểm tra

- Windows Visual Studio Release: build `polyec_realm_tests` pass; chạy trực tiếp đạt **20.735 checks**, profile A 10.000 người/19.743 quan hệ thưa/90 ngày.
- Windows Visual Studio Debug: build và chạy cùng target; đạt **20.735 checks**. Toàn executable mất khoảng **270 giây** theo thời gian chờ tiến trình; phần profile 90 ngày báo 22,49 giây.
- Full core CTest Release **19/19** ở mốc tích hợp estate/office ngay trước test-only fixture này. Sau đó chỉ chạy trực tiếp Realm target; không đổi production/API/schema/binding/model, nên Python/native/GDExt/Godot không chạy lại.
- Tự review đối chiếu edge legal bit, cash/hàng người hưởng, balance quỹ công, kho vùng, tổng tiền, invariant và save/load. Không có reviewer độc lập.

Đây là một phép kiểm nối vào Realm cho ranh giới parentage; không đóng toàn bộ thừa kế hoặc M5. [Manifest](../../artifacts/m5-core-estate-unverified-parentage-realm-2026-10-22-manifest.json) ghi hash file của lát này. Không commit, push hoặc publish.
