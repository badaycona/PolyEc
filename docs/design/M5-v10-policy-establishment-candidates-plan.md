# M5 — policy tạo và cấp vốn cơ sở sản xuất

## Kết quả người chơi cần thấy

Người chơi/agent có thể chọn tạo một xưởng dệt, xưởng công cụ hoặc xưởng quân nhu trong vùng của mình, đồng thời chọn mức vốn 25, 50 hoặc 100. Core tạo cơ sở qua `EstablishmentCommand` đang dùng cho UI/test; người không đủ quyền hoặc tiền thấy action bị khóa và gửi lệnh trực tiếp vẫn bị từ chối nguyên tử.

## Phạm vi source

- `core/include/polyec/realm.hpp`: `realm-v10`, 89 actions, 1.196 observations; slots 80–88 là tích của ba ngành và ba mức vốn.
- `core/src/realm.cpp`: đưa 9 candidates vào snapshot, legality, observation và cùng typed command hiện hành; actor Governance tạo sở hữu Public bằng treasury địa phương, actor khác tạo sở hữu Person bằng cash cá nhân.
- `native/include/polyec/model_manifest.hpp`, `native/realm_contracts.cpp`, Python contract checks và model file: v10/1196/89; từ chối v9/1088/80.
- `godot_project/ui/realm/realm_panel.gd`, Godot compatibility smoke: chỉ tải đúng v10; bundle v6/v9 cũ fail closed.
- `core/tests/test_realm.cpp` và test model/runtime: mask, debit đúng nguồn, ngành/vốn, quyền public, thiếu tiền, stale snapshot/command và atomic rejection.

## Invariants

- Candidate chỉ mô tả vùng actor hiện đứng; không chọn nơi xa hoặc tự dịch chuyển.
- Legal mask phản ánh đúng owner/funding source và tiền; phần thực thi gọi cùng `EstablishmentCommand` thay vì sao chép luật giao dịch.
- Public site cần taxing authority/charter và trả từ treasury; Person site thuộc actor và trả từ cash. Giao dịch tăng đúng một establishment ID/fact, không nhân bản khi bị replay.
- Save schema giữ nguyên: action/policy surface không đổi state codec. Model contract đổi v9→v10; checkpoint cũ không được nạp.
- Chưa có action nạp/rút kho, chuyển Goods, cấp quyền Work hoặc gửi Schedule Work; không chạy PPO v10 và không gọi chu trình xưởng đã hoàn thành.

## Kiểm tra trước khi chuyển sang phần tiếp

1. Core Release/Debug tests cho candidate, legality, command atomicity và save/load.
2. Python binding shape/preflight cùng native manifest contract.
3. GDExt build và Godot kiểm schema mới, từ chối bundle cũ.
4. Self-review diff/evidence; sau đó thêm chuyển vật tư và ca Work ở các lát có bounded command coverage.
