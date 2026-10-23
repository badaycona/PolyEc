# M5.0/1 — quyền sử dụng holding theo ủy quyền của title-holder

Ngày 2026-10-21. Working rule kế tiếp dựa trên lựa chọn của người dùng: đơn vị đất là holding/cụm trừu tượng cố định trong một region. Không có parcel/map geometry.

## Contract

- `LandUseRight` là giấy phép riêng cho một người dùng toàn bộ một holding; không chuyển title, không chia diện tích, không đổi region hoặc jurisdiction. Mỗi holding tối đa một giấy phép đang hiệu lực; title-holder trưởng thành có thể cấp/thay thế và thu hồi bằng Realm command.
- Cấp/thu hồi không thu tiền, không tạo hàng/sản lượng, không ghi lịch lao động, không tạo tenant debt hay quyền với Province. Đây là quyền pháp lý dùng đất; cơ chế canh tác/sản xuất, giá thuê, địa tô, consent, lien, đồng sở hữu và chia holding vẫn nằm ở gate kinh tế/production sau.
- Actor phải là title-holder còn sống và đủ 18 game years. Người nhận phải là Person còn sống, có thể là minor; guardian không ký thay. Title transfer bị từ chối khi holding có active use right cho tới khi holder thu hồi trước, không lặng lẽ chuyển/cancel burden.
- Một right có stable ID, holding ID, grantee ID và grantor ID; danh sách sắp theo right ID, duy nhất mỗi holding. Revoke yêu cầu title-holder hiện tại, right còn active và sequence hiện hành. Hai operation ghi fact riêng, tăng decision/sampling đúng một lần; stale/ID/permission/fact/id-counter overflow reject trước mutation.
- Schema22 thêm `LANDUSE_1` sau `LAND_1`; schema7–21 migrate sang danh sách rỗng. Load stage records, kiểm count/ordering/duplicate/ref/holder ownership trước swap. Save/load/fingerprint và invariant bao gồm quyền.
- Rights chỉ áp dụng hiệu lực ở command registry slice; model v7 không được tự grant/revoke. Action/observation/reward/model không đổi, nên model contract giữ nguyên. C++ core vẫn là nguồn state; Python/GDExt/Godot chỉ gọi lệnh/query đó.

## Files và ownership

- `core/include/polyec/land_holding.hpp`, `realm.hpp`; `core/src/realm_land.cpp`, `realm.cpp`; một stateless `land_use_codec` nếu section mới cần tách parser/serializer.
- `core/tests/test_land_title.cpp` hoặc target riêng; test grant/revoke/replace, authority/dependent denial, adult/minor/dead/bad ID, atomic counters/facts/money, transfer blocked while active, save22 and schema21 migration.
- Python binding expose query/command; native runtime command takes expected fingerprint; GDExt/session forwards; Godot smoke covers grant, revoke, deny stale state and save/load continuation.
- Update `IMPLEMENTATION_PLAN`, `CURRENT_TASK`, History only after gates pass. No schema/model bundle compatibility changes beyond save schema22.

## Gates

1. Realm command success/reject preserves title and ledger, transfer cannot bypass active permission; every reject preserves full fingerprint and counters.
2. Invariant catches unsorted/duplicate/orphaned records and invalid Person references. Save22 round-trip; exact schema21 migration produces zero rights; malformed `LANDUSE_1` fails atomically.
3. Windows Release/Debug full CTest; Python parity/replay and 263 binding checks; GDExt Release build and Godot smoke; repository links/scoped diff/hash manifest.
4. No assertion that agricultural output or rent collection works: neither is implemented by this slice. Review remains self-review unless an independent reviewer is actually used.

## Self-review before implementation

The right has one clear owner, one grantee and no hidden money or output effect. Restricting the first slice to a revocable, unpriced whole-holding license avoids introducing rent accounting or parcel geometry without contract. Blocking title transfer while active prevents an implicit burden migration. All live mutations are Realm commands; a schema bump and all adapters are included. No unresolved gameplay choice blocks this bounded slice under the user's delegated M5 assumptions.
