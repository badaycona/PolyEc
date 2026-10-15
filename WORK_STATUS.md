# PolyEc — trạng thái làm việc

Cập nhật: 2026-10-15. Trạng thái này mô tả mã hiện có; [implementation plan](docs/design/implementation-plan.md) mô tả đích sản phẩm.

## Hiện tại

- **M4 đã có lát cắt native và được kiểm chứng Windows local:** bản đồ sáu lãnh địa + đất triều đình/10.000 người, quyền đèo–công trình–vận tải–nợ–hồ sơ và xưởng dệt. M4 dùng controller kiểm thử có nhãn; Linux mới chưa chạy, chưa production RL. Xem [báo cáo M4](docs/reports/M4-evidence.md).

- Đã có sandbox native M3: 6 lãnh địa + đất triều đình, 10.000 người, 3 họ policy PPO chạy ONNX CPU, 12 draft augment và bộ kiểm tra core/runtime/UI.
- Ở M3, Windows portable và Linux package đã được dựng; CI core Windows/Linux và Linux app xanh ở revision 73913be. Kết quả lịch sử này không chứng nhận thay đổi M4.
- Repo GitHub: [badaycona/PolyEc](https://github.com/badaycona/PolyEc).
- Đây chưa phải campaign một đời vua. Trang README mô tả sản phẩm và cấu trúc repo; tài liệu trạng thái này chỉ ghi gate và việc tiếp theo.

## Gate chất lượng còn mở

- **R1:** audit policy theo categorical sampling chưa đạt. Governance không cải thiện so với untrained; cả ba họ chưa qua ngưỡng không kém baseline. Default cao ở governance và economic.
- **R2:** luật tuổi/khả năng được kiểm tra, nhưng chưa chứng minh transfer học từ trẻ sang người lớn.
- **R3:** chưa có benchmark INT nhiều bước; phản ứng policy với mục tiêu còn yếu.
- **Dài hạn:** 8 sandbox 20 năm còn người sống, nhưng một seed giảm còn 7.788 người từ 10.000; chưa chứng minh dân số/xã hội cân bằng.

Training records nằm ở artifacts/models; ảnh evidence nằm ở artifacts/visuals. Số liệu, phương pháp và giới hạn nằm trong [docs/reports/M3-evidence.md](docs/reports/M3-evidence.md). Không dùng kết quả argmax để thay thế audit categorical của app.

## Việc tiếp theo

1. Kiểm chứng Linux của M4 tại commit được push; không suy từ kết quả Windows.
2. Trước M5, chốt nền xã hội theo [ranh giới độ sâu](docs/design/simulation-complexity-scope.md) và [nghiên cứu cuối Trung cổ](docs/design/late-medieval-world-design-discussion.md). Người dùng đã chọn bối cảnh cuối Trung cổ; các luật xã hội đầy đủ vẫn là đề xuất. Sáu vùng/10.000 người của M4 không phải quyết định giảm scope sản phẩm.
3. M5–M6 hoàn thiện luật/world rồi đóng băng action/observation/reward; production training và R1–R5 ở M7 sau feature freeze. Các model realm-v1–v6 tiếp tục là pilot.

M4 Windows: CTest 5/5 (campaign 202 checks), Godot native/UI/M4 19/18/39 và shared-core Python 5 checks; hồ sơ Hắc Sơn đóng được ở 10.000 người. [Báo cáo M4](docs/reports/M4-evidence.md) ghi workload, ảnh, số đo và giới hạn. Gói local nằm ở `dist/PolyEc-M4-Windows`; build/dist không được đưa vào Git.

Repo đã phân tài liệu vào design/developer/experiments/guides/handoff/reports và giao diện vào `godot_project/ui/`. [Mục lục docs](docs/README.md) và [quy ước thư mục](docs/developer/repository-layout.md) là điểm tìm đường dẫn mới. OpenCode local/cách sửa 403 và hồ sơ review được giữ ở [handoff](docs/handoff/opencode-handoff.md).

## Các lệnh thường dùng

- Build/test/package: scripts/build-windows.ps1, scripts/test-native.ps1, scripts/package-windows.ps1; Linux package: scripts/package-linux.sh.
- Training: scripts/build-training.ps1, scripts/train-cpu.ps1, scripts/train-realm.ps1.
- CMake options, checks và các giới hạn platform: xem [README](README.md) và [báo cáo M3](docs/reports/M3-evidence.md).

Không tự commit/push khi chưa có yêu cầu cho tác vụ đó. Không báo test mới nếu chưa chạy lại.
