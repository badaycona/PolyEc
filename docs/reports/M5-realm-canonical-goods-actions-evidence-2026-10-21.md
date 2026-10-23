# M5 Realm canonical goods action surface

Ngày 2026-10-21. Plan: [canonical goods action surface](../design/M5-realm-canonical-goods-actions-plan.md); checkpoint [manifest](../../artifacts/m5-realm-canonical-goods-actions-2026-10-21-manifest.json). Codex tự review; không có reviewer độc lập, không train, không commit/push.

## Thay đổi

- Realm action interface đổi từ `realm-v6` (32 actions, 512 observation values) sang `realm-v7` (72 actions, 992 observation values). Save schema vẫn là 20.
- Slot 1–14 giữ nguyên vị trí Buy/Sell nhưng `Candidate.good` nay là canonical `GoodId`, gồm mapping cũ qua `kLegacyGoodToCanonical`. Thêm Buy ở slot 32–51 và Sell ở slot 52–71 cho 20 hàng còn lại. Mỗi một trong 27 GoodId có đúng một slot Buy và Sell; slot 15–31 giữ nguyên.
- Realm legal/settlement dùng trực tiếp canonical GoodId và range-check trước khi index. Candidate market features dùng cùng GoodId để đọc price/stock/inventory; feature Good chuẩn hóa theo 27 sản phẩm.
- Native loader chỉ nhận schema/dimensions v7 đúng và contract chứa goods27. Model v6 bị từ chối cả khi bật dev override do tensor shape không tương thích. Bundle lịch sử không bị sửa.
- Thêm `realm_model_v7.py`, `realm_train_v7.py`, `realm_export_v7.py`; trainer/exporter v7 lấy contract từ C++ binding. Godot UI preflight tìm `models/realm-v7`; không tạo model placeholder.
- Thêm `realm_parity_v7.py` và `realm_sampled_v7.py`, kiểm tra manifest dimensions/schema/contract trước khi so PyTorch–ONNX–native, cùng common-seed sampled baseline cho 72 action. Các script v6 còn giữ nguyên làm artifact lịch sử. Agency probe lấy action count từ binding hiện hành.

## Bằng chứng chạy

- Windows VS18 Release full CTest: **16/16 pass** (23,34 giây). Sau khi bổ sung hai assertion cuối về invalid/unstocked GoodId, Release `realm_tests` chạy lại **1/1 pass**.
- Windows VS18 Debug full CTest: **16/16 pass** (304,51 giây). Sau hai assertion cuối, Debug `realm_tests` chạy lại **1/1 pass** (130,39 giây).
- Native runtime contract Release và Debug: **1/1 pass** mỗi cấu hình; kiểm tra v7 metadata hợp lệ, sai family/contract/dimension, contract rỗng, và từ chối v6 ở chế độ mặc định lẫn dev override. Các runtime integration checks về clock, save/restore và role-memory vẫn nằm sau nhánh chạy khi có bundle v7 thật; nhánh đó hiện chưa thực thi vì không có model tương thích.
- Python C++ binding build Release và Debug. `realm_training_checks.py`: **476 checks** mỗi cấu hình, output shape `(batch,time,72)` và observation 992. `realm_binding_checks.py`: **259 checks**/64 ngày mỗi cấu hình, Python/C++ fingerprint và tiền khớp, invalid batch atomic; tốc độ Debug thấp hơn Release và không phải profile B.
- `establishment_transfer_tests` chạy lại Release/Debug **1/1 pass** mỗi cấu hình; canonical GoodId Fiber Buy và Sell đổi đúng inventory, Province stock và cash, giữ nguyên tiền/hàng hóa tổng.
- Bổ sung Realm command regression cho đủ 27 canonical GoodId: mỗi sản phẩm Buy rồi Sell, xác nhận đúng slot inventory/Province stock và giữ `total_money()` + whole-world invariant sau từng lệnh. Release `realm_tests` **1/1 pass** (11,98 giây); Debug **1/1 pass** (124,78 giây).
- Python parity Release/Debug mỗi cấu hình: household fund 18 checks tổng hai case; v16 migration 40 checks/6 saves; gameplay config 49 checks/7 checkpoints; work authorization 7 checkpoints; model contract 13 checks; clock 173 checks/7 checkpoints.
- GDExtension Windows Release build pass. Godot 4.5.1 headless guard **5/5 pass**; main scene khởi động headless 2 frames và exit 0. Guard xác nhận core v7 từ chối bundle lịch sử v6.
- `py_compile` cho ba script training/model/export v7 pass.
- `py_compile` cho parity/sampled v7 và agency probe pass; inference/evaluation v7 chưa chạy vì chưa có bundle v7 đã train.
- `scripts/check-repository-links.py`: **843 local document/resource references pass**, gồm đường dẫn model-v7 có README trạng thái “chưa có bundle”. Scoped `git diff --check` pass; file mới qua trailing whitespace scan.

## Giới hạn còn mở

Chưa có ONNX v7 tương thích; không chạy training. Test 27 giao dịch dựng thêm stock với matching opening goods trong save fixture để kiểm đường transaction, không khẳng định sản xuất/cung ứng tự nhiên hoặc market depth. Province ban đầu và các recipe hiện hành vẫn chỉ cấp một tập hàng lịch sử, giá cho sản phẩm mới chưa được hiệu chỉnh. Vì vậy action surface chưa nghiệm thu sản xuất/retail toàn catalog, learned-policy parity/sampling gates (scripts đã sẵn sàng nhưng chưa thể chạy), quality R1–R3, Godot giao dịch end-to-end hoặc profile B. M5 và core refactor tổng thể vẫn đang mở.
