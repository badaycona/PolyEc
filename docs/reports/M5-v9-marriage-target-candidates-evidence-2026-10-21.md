# M5 — nhiều lựa chọn người nhận hôn nhân trong policy contract

Ngày 2026-10-21. Lát này bổ sung lựa chọn target hôn nhân cho learned policy và khóa tương thích theo schema. Đây chưa phải model đã huấn luyện hoặc nghiệm thu R1.

## Thay đổi

- Policy contract hiện là `realm-v9`: 1.088 observation và 80 action. Tám slot 72–79 đưa ra tám người nhận khác nhau, không phải actor; danh sách được dựng xác định theo actor và bộ đếm sampling, không sửa world state.
- Core tiếp tục tạo legal mask và revalidate target khi command chạy. Observation đưa tuổi và vùng công khai của target; không đưa cash, hộ hay quan hệ riêng.
- Manifest native chỉ nhận v9/1088/80; checkpoint v8 bị từ chối. Trainer/evaluator v8 cũng fail closed nếu binding đã là v9. Chưa có trainer/export v9 hoặc checkpoint v9 được huấn luyện.
- `Realm::deserialize` tạo Realm tạm trên heap trước khi publish save đã validate. Debug suite tổng hợp vượt stack Windows mặc định do khung test lớn; cấu hình stack 4 MiB chỉ áp dụng cho executable `polyec_realm_tests` trên MSVC.
- Đồng bộ hai Python assertion config-v3 và một fixture Buy dùng nhầm GoodId cũ với canonical `GoodId::Fiber=9`; sau khi sửa, các phép đối chiếu liên quan qua.

## Đã kiểm tra

- Build core toàn bộ và CTest Release: **18/18**; Debug: **18/18**. `realm_tests` Debug chạy 146,70 giây và pass sau khi cấu hình stack của executable test lên 4 MiB.
- Python 3.12 Release/Debug: model-contract 14 checks; trainer/core preflight 477 checks; clock binding 173 checks và 7/7 checkpoint C++–Python.
- Release Python: household fund 11 checks; household study 7; migration save v16 40; gameplay config 49 checks/7 checkpoint; work authorization 7 checkpoint; establishment transfer/production 32 checks/7 checkpoint.
- Native `runtime_contracts` Release: 1/1 pass, kiểm schema/shape model và từ chối manifest v8.
- GDExtension Windows Release build thành công. Godot 4.5.1 headless chạy `realm_worksite_smoke.gd`: **5/5** kiểm tra; schema v9 được nhận và bundle v6 bị từ chối, Realm không load bundle cũ.
- Mã chẩn đoán tạm đã được gỡ; `git diff --check` được chạy trước khi hoàn tất report.

## Giới hạn và việc còn mở

Self-review; không có reviewer độc lập. Godot smoke chỉ kiểm fail-closed với bundle cũ; chưa có ba model v9 để chơi và chưa chứng minh marriage gameplay qua Godot. RealmBatch vẫn thiếu learned workshop cycle, đối tác giản lược và cross-play. Chưa huấn luyện policy v9, chưa đạt R1. M5 và core refactor tiếp tục mở; lát kế là đưa create/fund/supply/worksite selection vào cùng command pipeline và adapter training trước khi huấn luyện lại.

Không commit/push: checkout đang có thay đổi M5/core chưa thể tách chính xác khỏi các lát khác.

SHA-256 manifest: [m5-v9-marriage-target-candidates-2026-10-21-manifest.json](../../artifacts/m5-v9-marriage-target-candidates-2026-10-21-manifest.json).
