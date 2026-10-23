# M5.0/1 — quyền sử dụng holding theo ủy quyền của title-holder

Ngày 2026-10-21. Plan: [M5 land-use authorization](../design/M5-land-use-authorization-plan.md). Manifest SHA-256: [land-use checkpoint](../../artifacts/m5-land-use-authorization-2026-10-21-manifest.json).

## Kết quả

Triển khai `LandUseRight` độc lập cho toàn holding/cụm trừu tượng neo vào region; không có parcel geometry. Mỗi holding có tối đa một quyền đang hiệu lực. Adult title-holder còn sống có thể cấp, thay thế hoặc thu hồi quyền cho một Person còn sống; minor được nhận nhưng guardian không ký thay. Right không thu tiền, không tạo sản lượng, không thay title, kho, diện tích, region hoặc jurisdiction. Title transfer bị reject khi còn quyền active cho tới khi holder thu hồi trước.

Lệnh đi qua Realm transaction với kiểm tra expected fingerprint, authority, ID và counter trước mutation. Ghi fact/sequence đúng một lần; reject giữ fingerprint và counters. `LANDUSE_1` được thêm sau `LAND_1`, save schema22; schema7–21 nạp danh sách rỗng. Parser load stage records và kiểm count, sort/uniqueness, references, title grantor và giới hạn ID trước swap. Invariant/fingerprint/save-load bao gồm registry. Python, native runtime, GDExt và Godot chỉ gọi/query cùng C++ core. Action/observation/reward/model contract không đổi.

## Verification

- Windows CMake `ALL_BUILD`: Release và Debug thành công.
- Full CTest: Release 19/19 và Debug 19/19. Land title target riêng: 81 checks mỗi cấu hình; schedule target: 3,717 checks.
- Native `polyec_realm_contracts` với pilot v7: 129 checks ở Release và Debug, gồm cấp quyền, query, chặn transfer, revoke, transfer sau revoke và invariant.
- Python binding/replay: Release và Debug 269 checks, 64 ngày C++ replay; training preflight 476 checks mỗi cấu hình. Python–ONNX Runtime–C++ v7 parity: 507 checks (Release).
- GDExt Release build; Godot headless smoke với ba model v7 pilot và fixture interactive 64 ngày: 477/477 checks, gồm command/query/revoke qua extension.
- Repository links: 881 local references passed. Scoped `git diff --check` cho source/test/adapter files: exit 0. Unscoped check còn báo whitespace trong `m5_t1b1_clock_save.bin`, file test artifact đã dirty trước lượt này; không chỉnh file đó.

Fixture tại `artifacts/m5-land-title-transfer-realm-expected-2026-10-21.json` được tái sinh bằng native replay interactive sau schema bump để fingerprint Realm schema22 và Godot load-paused lifecycle khớp nhau. Hash đầy đủ của checkpoint hiện tại được ghi trong manifest phía trên; manifest transfer cũ vẫn là snapshot của lát trước schema22.

## Giới hạn và review

Đây chỉ là authorization registry. Chưa có tenant production, canh tác, rent, yield, lien, consent người nhận, co-ownership, chia holding, kế thừa/estate hoặc guardian administration. Pilot v7 dùng làm fixture tích hợp; manifest báo `R1=false`, nên không phải learned-policy acceptance. Profile B vẫn vượt ngưỡng daily target đã ghi ở checkpoint trước; lát này không đo lại hiệu năng. Review là self-review, không có reviewer độc lập. Không commit hoặc push.
