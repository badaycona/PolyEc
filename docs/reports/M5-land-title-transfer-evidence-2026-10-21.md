# M5.0/1 — chuyển title đất theo lệnh holder: evidence (2026-10-21)

Plan: [M5 land-title transfer](../design/M5-land-title-transfer-plan.md). Baseline `dff2781a41ae52a4bd476b4b1460ad0f09ef5dc3`; source ở dirty working tree có nhiều lát khác. Manifest hash nguyên file, không phải patch cô lập: [manifest](../../artifacts/m5-land-title-transfer-2026-10-21-manifest.json). Review là self-review; không commit/push.

## Thay đổi

- Thêm `LandTitleTransferCommand` và Realm transaction: actor phải là title-holder trưởng thành, còn sống; recipient phải là Person còn sống và có thể là minor. Guardian không ký thay dependent. Validation stale sequence, actor/recipient/title, deceased, tuổi, decision/fact overflow xảy ra trước mutation; transfer đổi một holder, ghi decision counters và fact 212.
- `LandHolding` giữ nguyên ID, region, area; không đổi money, schema21/`LAND_1`, action, observation, reward hoặc model. Không có giá, thuế, recipient consent, lien, co-ownership, land-use/lease, guardian administration hay inheritance.
- Gắn command qua native runtime với expected-fingerprint, Python binding, GDExtension và GDScript smoke. Smoke gọi thật qua session → GDExt → C++ command; chưa thêm registry vào `PlayerView` hoặc luồng giao diện người chơi.
- Tách registry/transfer tests thành executable CTest `land_title_tests`. Debug dump trước đó xác nhận exception `0xC00000FD` (stack overflow) khi test title nằm trong harness Realm lớn. Sau khi tách, full Debug suite và targeted tests đều chạy hết.

## Gate đã chạy

| Gate | Kết quả |
|---|---|
| `cmake --build build/m5-runtime --config Release --parallel 8` | Exit 0; tạo `polyec_land_title_tests` Release |
| Full Release CTest | 18/18 pass, 28.10 giây; test title pass |
| Full Debug CTest cuối | 18/18 pass, 280.87 giây; test title pass; Realm test không còn stack overflow |
| Python binding + C++ replay | 263 checks, observation 992/actions 72, replay 64 ngày khớp; exit 0 |
| Python learned-training preflight | 476 checks; exit 0 |
| Native runtime contracts với ba v7 pilot ONNX | 125 checks pass; transfer success, money/invariant và stale expected fingerprint |
| CTest `runtime_contracts` mặc định | 21 checks pass, nhưng dùng bundled v6 và xác nhận bundle cũ bị từ chối; v7 inference không chạy ở gate đó |
| Windows GDExtension Release `polyec` | Build exit 0 |
| Godot 4.5.1 headless Realm smoke | 473/473 checks pass, dùng `--model-root` và replay fixture tường minh |

Godot/native smoke dùng `artifacts/models/realm-v7-pilot-2026-10-21`, chỉ để kiểm tra wiring. Model pilot hiện `full_R1_pass=false`; kết quả không nghiệm thu learned-policy quality hoặc bundle sản phẩm. GDExt build là build native; Godot smoke gọi command qua API nhưng không xác nhận UI người chơi cuối.

## Phạm vi còn mở

Lệnh không định nghĩa giá/thuế, recipient consent, lien, co-ownership, tách thửa, quyền canh tác/thu tô/lease, người đại diện cho dependent, chuyển title khi chủ chết, hoặc inheritance/estate. Những phần này cần contract riêng; không được suy từ command hiện tại. M5.0/1, M5.2–M5.5, learned-policy R1–R5, full-cycle profile B/long-run và Windows package tiếp tục mở. Không có reviewer độc lập.
