# M5.0/1 — registry title đất theo cụm vùng

Ngày 2026-10-21. Lát tiếp theo sau household fund và HouseholdIndex. Đây là working assumption được ủy quyền trong `CURRENT_TASK`, không phải xác nhận balance hay luật thuê đất.

## Mục tiêu và working rule

Tạo sổ title đất có ID ổn định, vùng địa lý cố định, diện tích nguyên dương và đúng một chủ sở hữu Person. Title sở hữu không đồng nghĩa quyền xét xử, quyền sử dụng/canh tác, quyền thu tô hay quyền quản trị người thuê. Province tiếp tục giữ quyền xét xử; registry này không ghi quyền Province.

Mỗi hộ hiện hữu khi khởi tạo world mới nhận một holding trừu tượng diện tích 1, tại vùng cư trú của member có ID bằng household label; holder ban đầu là chính Person đó. Đây là endowment ban đầu được khai báo rõ, không tạo tiền/hàng. World cũ khi load không được cấp thêm title. Đơn vị diện tích chưa đổi sang km²/ha cho tới khi map parcel contract được chốt.

Lát này chỉ thêm registry, invariant, stable ID, fingerprint và save/load. Không thêm giao dịch chuyển title, giá đất, thuê/địa tô, canh tác, guardian thay dependent ký deed, quyền sử dụng hay map coordinates. Các mục đó cần command/transaction gate tiếp theo. Không đổi action/observation/reward/model contract hoặc quyết định policy.

## File và ownership

- `core/include/polyec/land_holding.hpp`: record giá trị `LandHolding`.
- `core/include/polyec/land_title_section_codec.hpp`: codec thuần cho `LAND_1`, parse vào state tạm, kiểm bounds trước cấp phát.
- `core/include/polyec/realm.hpp`, `core/src/realm.cpp`: Realm sở hữu records; genesis tạo một record per household; `next_id_` cấp non-Person IDs; serialize/load stage; invariant/fingerprint; schema21. Schema7–20 load đúng dữ liệu cũ với registry rỗng, không retroactively cấp đất.
- `bindings/python/realm_binding.cpp`, `ai/tests/realm_binding_checks.py`: training/test-only, read-only listing of title IDs/regions/area/holders from the same C++ core; no duplicate Python rule engine.
- `core/tests/test_realm.cpp`: genesis oracle, title/region/owner/area invariants, unique stable IDs, schema20→21 migration behavior, roundtrip/corrupt-save atomicity, fingerprint sensitivity.
- `docs/IMPLEMENTATION_PLAN.md`, `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`, evidence report và manifest.

## Invariant và gate

1. Mỗi record: ID dương, duy nhất trong namespace non-Person `next_id_`; region `0..regions`; owner là Person ID hợp lệ; area > 0.
2. Mỗi household label trong world genesis có đúng một initial title; household động không tự tạo title. Khi load save cũ, không tạo title ngoài dữ liệu save.
3. Save output phải có `LAND_1`, schema21; schema7–20 vẫn đọc theo các migration hiện hành và có registry rỗng. Load lỗi không đổi world đích.
4. Realm fingerprint nhạy với record order-independent theo ID, owner, region và area.
5. Release/Debug Realm tests và full CTest khi cấu hình hiện có; rebuild/check Python binding query and replay. GDExtension build catches core ABI integration; do not add raw land titles to Godot PlayerView because current information/visibility contract does not authorize disclosing every holder to the player.

## Self-review trước code

Record không giữ household/guardian pointer: hai khái niệm đó có thể đổi theo lifecycle. Endowment dùng stable Person owner và tọa độ vùng hiện tại tại genesis; sau migrate/di chuyển title vẫn giữ vùng. Không có transfer nên không thể giả lập consent hoặc làm guardian tự định đoạt tài sản phụ thuộc. Python binding chỉ phục vụ test/training; đưa title vào PlayerView cần information and visibility contract trước.

Review là self-review; không có reviewer độc lập. Không commit/push.
