# PolyEc — trạng thái làm việc

Cập nhật: 2026-10-15. Trạng thái này mô tả mã hiện có; [implementation plan](docs/implementation-plan.md) mô tả đích sản phẩm.

## Hiện tại

- Đã có sandbox native M3: 6 lãnh địa + đất triều đình, 10.000 người, 3 họ policy PPO chạy ONNX CPU, 12 draft augment và bộ kiểm tra core/runtime/UI.
- Windows portable và Linux package đã được dựng; CI core Windows/Linux và Linux app xanh ở revision 73913be.
- Repo GitHub: [badaycona/PolyEc](https://github.com/badaycona/PolyEc).
- Đây chưa phải campaign một đời vua. Trang README mô tả sản phẩm và cấu trúc repo; tài liệu trạng thái này chỉ ghi gate và việc tiếp theo.

## Gate chất lượng còn mở

- **R1:** audit policy theo categorical sampling chưa đạt. Governance không cải thiện so với untrained; cả ba họ chưa qua ngưỡng không kém baseline. Default cao ở governance và economic.
- **R2:** luật tuổi/khả năng được kiểm tra, nhưng chưa chứng minh transfer học từ trẻ sang người lớn.
- **R3:** chưa có benchmark INT nhiều bước; phản ứng policy với mục tiêu còn yếu.
- **Dài hạn:** 8 sandbox 20 năm còn người sống, nhưng một seed giảm còn 7.788 người từ 10.000; chưa chứng minh dân số/xã hội cân bằng.

Training records nằm ở artifacts/models; ảnh evidence nằm ở artifacts/visuals. Số liệu, phương pháp và giới hạn nằm trong [docs/M3-evidence.md](docs/M3-evidence.md). Không dùng kết quả argmax để thay thế audit categorical của app.

## Việc tiếp theo

Theo plan cập nhật, bước tiếp theo là **M4 vertical slice native**: người chơi xử lý trọn một vấn đề phát sinh và thấy hệ quả trên bản đồ/sổ sách. Sau đó M5–M6 hoàn thiện luật và world đích rồi đóng băng action/observation/reward. Các model realm-v1–v6 là pilot để chẩn đoán pipeline; đợt production training và quality gates R1–R5 được đặt sau feature freeze ở M7. Trước đó chỉ giữ smoke training nhỏ cho tích hợp.

## Các lệnh thường dùng

- Build/test/package: scripts/build-windows.ps1, scripts/test-native.ps1, scripts/package-windows.ps1; Linux package: scripts/package-linux.sh.
- Training: scripts/build-training.ps1, scripts/train-cpu.ps1, scripts/train-realm.ps1.
- CMake options, checks và các giới hạn platform: xem [README](README.md) và [báo cáo M3](docs/M3-evidence.md).

Không tự commit/push khi chưa có yêu cầu cho tác vụ đó. Không báo test mới nếu chưa chạy lại.
