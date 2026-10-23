# M5.0/1 — registry title đất: evidence (2026-10-21)

Plan: [M5 land-title registry](../design/M5-land-title-registry-plan.md). Đây là working assumption được ủy quyền trong `CURRENT_TASK`; không phải quyết định cuối về diện tích/map hoặc cân bằng. Review: tự review, chưa có reviewer độc lập. Baseline Git `dff2781a41ae52a4bd476b4b1460ad0f09ef5dc3`; không commit/push. Source nằm trong dirty working tree có các lát khác; manifest ghi hash nguyên file, không phải patch cô lập.

## Đã triển khai

- Realm sở hữu `LandHolding` có stable ID, region cố định, diện tích nguyên dương và Person title-holder. Genesis cấp một holding area 1 cho mỗi household label, do founding Person nắm; không tạo tiền/hàng.
- `LAND_1` ghi/đọc theo ID có thứ tự, đọc staged và kiểm tra lỗi trước khi commit. Save hiện là schema21. Schema7–20 load không được cấp title hồi tố.
- Invariant/fingerprint bao gồm holding; ID dùng namespace non-Person và cùng kiểm tra collision với record Realm.
- Python binding chỉ cung cấp truy vấn read-only để test/training từ C++ core. Không đưa tất cả chủ đất vào Godot `PlayerView`, vì contract visibility hiện tại không cho phép tiết lộ toàn bộ title.
- Không có title transfer/sale, định giá, thuê/địa tô, canh tác, guardian deed, geometry hoặc thay đổi action/observation/reward/model.

## Gates đã chạy

| Gate | Kết quả |
|---|---|
| Release build `build/m5-runtime` | Exit 0 |
| Release CTest | 17/17 pass, 24.97 giây |
| Debug build `build/m5-runtime` | Exit 0 |
| Debug CTest | 17/17 pass, 329.08 giây |
| Python binding replay | 261 checks, 64 ngày C++ replay khớp; observation 992, actions 72; exit 0 |
| Python v7 training preflight | 476 checks; exit 0 |
| GDExtension Windows Release `polyec` | Build exit 0 |
| Godot runtime | Không chạy trong lát này; không tìm thấy `godot`/`godot4` trên PATH. Lát không sửa `PlayerView`/UI. |

Lệnh CTest: `ctest --test-dir build/m5-runtime -C Release --output-on-failure` và cấu hình tương ứng `Debug`. Python: `python ai/tests/realm_binding_checks.py --replay build/m5-python-ai/core/Release/polyec_realm_transition_replay.exe`; `python ai/tests/realm_training_checks.py`.

## Giới hạn và next gate

Đây là registry/title ownership, chưa phải quyền sử dụng đất, guardian administration, đất canh tác hoặc inheritance. Cần contract/command transaction riêng cho quản trị tài sản dependent; title phải giữ nguyên owner khi chưa có deed hợp lệ. M5.0/1, M5.2–M5.5, R1–R5 policy quality, profile B end-to-end, long-run và package tiếp tục mở. Schema21 hiện tại không làm model `realm-v7` thành learned-policy đạt chuẩn; pilot R1 vẫn fail theo report policy.
