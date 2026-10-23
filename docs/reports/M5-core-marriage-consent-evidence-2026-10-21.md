# M5.3 — cưới có đồng thuận và ký quỹ

Ngày 2026-10-21. [Kế hoạch](../design/M5-core-marriage-consent-plan.md), [checkpoint SHA-256](../../artifacts/m5-core-marriage-consent-2026-10-21-manifest.json). Đây là self-review; chưa có reviewer độc lập. Hash trong checkpoint là hash toàn file tại working tree chung, không tách riêng được các chỉnh sửa hôn nhân khỏi những phần M5/core đã có trong cùng file.

## Hành vi đã triển khai

Hai người trưởng thành còn sống có thể gửi đề nghị kết hôn và người nhận phải tự chấp thuận sau thời gian chờ. Đề nghị giữ 50 tiền trong escrow; chấp thuận chuyển đúng số đó cho người nhận, còn từ chối hoặc hết hạn hoàn lại người gửi. Cả lúc gửi lẫn lúc nhận đều kiểm tra tuổi, quan hệ cha/mẹ–con hoặc anh/chị/em, hôn nhân đang tồn tại và đề nghị cạnh tranh. Sau chấp thuận, Realm ghi quan hệ Spouse đối xứng. Không gộp hộ, chuyển title, tạo liên minh hoặc chia tài sản.

Hành động mới dùng slot 72, vì vậy số hành động tăng 72→73, observation 992→1004, model-contract chuyển realm-v7→realm-v8 và save schema 22→23. Save schema 7–22 không có spouse relation tiếp tục đọc được. Model v7 bị từ chối; chưa có bundle v8 được huấn luyện nên đây chưa phải tính năng chơi được qua runtime Godot sản phẩm.

## Bằng chứng chạy

- Windows Release và Debug full build + CTest: **19/19 mỗi cấu hình**; kiểm tra tập trung `court_tests`: **1/1 mỗi cấu hình**.
- Python binding v8 preflight: **476 checks mỗi cấu hình**; replay core–Python: **273 checks/64 ngày mỗi cấu hình**; model contract: **13 checks mỗi cấu hình**.
- Python gameplay-config parity: **49 checks, 7 mốc mỗi cấu hình**; clock binding: **173 checks và 7 mốc replay mỗi cấu hình**. Kiểm thử binding hôn nhân tạm thời ở Release/Debug xác nhận escrow, save/load pending, đồng thuận, settlement 50, bảo toàn tổng tiền và chặn đề nghị thứ hai.
- Native `runtime_contracts` Release: **1/1**; GDExtension Release build; Godot 4.5.1 model-compatibility smoke: **5/5**; main scene headless chạy hai frame và thoát thành công.
- `realm_smoke.gd` đầy đủ chưa chạy được: project không có `models/realm-v8/*/manifest.json`; script báo không đọc được manifest rồi dừng. Không tạo model placeholder. Chưa huấn luyện hoặc xác nhận R1–R5.

## Còn mở

Chưa có bundle realm-v8 đủ ba nhóm. Đã có model thử cho quản trị và kinh tế; nhóm dân sự mới có checkpoint một seed bị dừng giữa chừng, chưa có đánh giá hoặc ONNX. Vì vậy Godot chỉ xác nhận model cũ bị từ chối, chưa chạy được màn chơi v8 hoặc thao tác hôn nhân qua UI. Action 72 hiện cung cấp một đối tượng đích xác định sẵn theo ID, không phải màn chọn đối tượng của người chơi. Cần hoàn tất đường chọn/ngắm người nhận và kiểm tra hành vi qua Godot khi có policy bundle thật. M5.3 vẫn còn tổ chức, chức vụ, estate thừa kế/claim/nợ và giáo hội; M5.0–M5.5, core refactor, learned-policy quality và profile B vẫn mở.

Không commit/push: working tree có nhiều thay đổi M5/core của các phần khác cùng nằm trong các file nguồn này, nên chưa thể tách chính xác phạm vi để xuất bản an toàn.
