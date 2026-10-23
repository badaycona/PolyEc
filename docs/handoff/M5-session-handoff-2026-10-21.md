# Bàn giao phiên tiếp theo — PolyEc M5

Ngày 2026-10-21. Đây là ghi chú tiếp nối cho [CURRENT_TASK](../CURRENT_TASK.md), nơi giữ trạng thái chính thức. Người dùng đã yêu cầu hoàn tất phần đang làm dở, chuẩn bị bàn giao và dừng phần triển khai mới trong phiên này; không cần hoàn thành toàn bộ M5.

## Phần vừa hoàn tất

Đã thêm provenance cha–con để truy vấn người hưởng không coi cạnh tổng hợp lúc khởi tạo hộ là căn cứ thừa kế. Cạnh sinh thật được lưu trong schema25. Migration save schema7–24 chỉ khôi phục cờ khi childbirth fact khớp đúng child → parent. Query chỉ dùng cha–con có provenance; anh chị em phải có cha/mẹ hợp pháp chung. Chưa chuyển tiền hay tài sản.

- Plan: [M5-core-legal-parentage-provenance-plan](../design/M5-core-legal-parentage-provenance-plan.md)
- Kết quả kiểm tra: [báo cáo](../reports/M5-core-legal-parentage-provenance-evidence-2026-10-21.md)
- Lệnh đã chạy: Release manager tests pass; Realm tests pass 20.634 checks, gồm schema24 migration/roundtrip và malformed-index atomic rejection.
- Chưa kiểm lại full CTest, Python, GDExt hoặc Godot trên schema25. Chưa có reviewer độc lập. `realm_smoke` còn thiếu bundle `realm-v11`.

## Việc tiếp theo nên làm

1. Đọc lại `AGENTS.md`, `docs/CURRENT_TASK.md`, `docs/IMPLEMENTATION_PLAN.md` và phần liên quan trong workflow trước khi sửa code.
2. Giữ schema25 và migration parentage. Đọc plan hiện hành cho bước kế tiếp; nối beneficiary query vào phân phối tiền theo hồ sơ, có hạch toán rõ người chết/quỹ công và bảo đảm reject không đổi một phần dữ liệu.
3. Chưa chuyển hàng, land title, quyền sử dụng đất hoặc xưởng cho người thừa kế trong cùng lát nhỏ này; làm plan riêng sau khi tiền đã có fixture và báo cáo.
4. Sau khi nhóm phân phối tiền ổn định, chạy lại kiểm tra lưu/tải, migration và C++ chain; gom Python/GDExt/Godot theo hướng dẫn CURRENT_TASK. Không đánh dấu M5/policy hoàn tất khi các điều kiện còn mở.

## Working tree và cách tiếp quản

Checkout: `C:\prc\PolyEc`. Có rất nhiều file modified/untracked từ các phần M5 trước đó, trong đó `core/include/polyec/realm.hpp`, `core/src/realm.cpp`, `core/include/polyec/managers/estate_manager.hpp`, `core/tests/test_realm.cpp` và một số docs chứa thay đổi ngoài riêng phần parentage. Không reset, dọn, stage toàn bộ hay tạo commit/push gộp. So sánh `git status --short` với trạng thái đầu phiên trong log chat, rồi chỉ tiếp tục trên file liên quan. Không còn tiến trình kiểm tra nào cần chờ.

Build Release đã dùng: `build/m5-core`. Realm profile A hoàn tất thành công. Các số liệu kiểm tra và giới hạn được ghi ở report; không cần chạy lại hai executable này trừ khi sửa tiếp phần parentage hoặc core save.

M5 vẫn mở: phân phối estate, liquidate/transfer tài sản, guardian, các hệ thống gameplay còn thiếu, policy learned đạt chuẩn/R1, profile B và package Windows. Không tạo thêm backlog ở ghi chú này; dùng CURRENT_TASK để xác định ưu tiên tiếp theo.
