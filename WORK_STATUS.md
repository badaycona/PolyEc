# PolyEc — trạng thái làm việc

Cập nhật: 2026-10-17. Phiên này được chỉnh để ăn theo tài liệu mới nhất (docs/README.md — Thiết kế mô phỏng đã duyệt 2026-10-17). Tài liệu cũ trong thư mục docs/ và artifacts/ được giữ làm chứng tích lịch sử; các mục có nhãn "artifact" hoặc "source-snapshot" có thể là bản ghi thí nghiệm chứ không phải trạng thái hoạt động hiện hành.

## Hiện tại

- **M4 đã có lát cắt native và được kiểm chứng Windows local:** bản đồ sáu lãnh địa + đất triều đình/10.000 người, quyền đèo–công trình–vận tải–n... (xem docs/reports/M4-evidence.md để biết chi tiết kiểm chứng Windows).

- **M5 đã bắt đầu (mới xong lát cắt hộ gia đình):** `Realm` giờ có membership index thưa theo household (`household_of`, `household_members`, `same_household`, `guardian`, `household_records`) và các sửa liên quan đến save/restore cho fixture hộ gia đình.

- Đã có sandbox native M3: 6 lãnh địa + đất triều đình, 10.000 người, 3 họ policy PPO chạy ONNX CPU, 12 draft augment và bộ kiểm tra core/runtime/UI.
- Lịch sử CI/build: có kết quả Windows native và thử nghiệm M4 như mô tả trong docs/reports/M4-evidence.md; Linux packaging và một số workflow platform-specific có thể chưa được chạy lại cho revision M4 (đừng suy từ kết quả Windows sang Linux mà không chạy `native-linux.yml`).
- Repo GitHub: [badaycona/PolyEc](https://github.com/badaycona/PolyEc).
- Trang README mô tả sản phẩm và cấu trúc repo; tài liệu trạng thái này ghi gate và việc tiếp theo.

## Gate chất lượng còn mở

- **R1:** audit policy theo categorical sampling chưa đạt. Governance không cải thiện so với untrained; cả ba họ chưa qua ngưỡng không kém baseline. Default cao ở governance...
- **R2:** luật tuổi/khả năng được kiểm tra, nhưng chưa chứng minh transfer học từ trẻ sang người lớn.
- **R3:** chưa có benchmark INT nhiều bước; phản ứng policy với mục tiêu còn yếu.
- **Dài hạn:** 8 sandbox 20 năm còn người sống, nhưng một seed giảm còn 7.788 người từ 10.000; chưa chứng minh dân số/xã hội cân bằng.

Training records nằm ở artifacts/models; ảnh evidence nằm ở artifacts/visuals. Số liệu, phương pháp và giới hạn nằm trong [docs/reports/M3-evidence.md](docs/reports/M3-evidence.md)

## Việc tiếp theo

**Quy trình người dùng yêu cầu (2026-10-15):** OpenCode lập implementation plan ngắn trước mỗi tác vụ code → Codex review → OpenCode triển khai/test → Codex nghiệm thu...

**Trạng thái duyệt M5 (2026-10-15):** người dùng đã duyệt triển khai gameplay M5; plan đã duyệt này điều hành công việc hiện tại. Ưu tiên Windows trước; việc ...

Mới xong: **lát cắt hộ gia đình đầu tiên** — membership index thưa + giám hộ + reject save hỏng. M5.1–M5.5 còn mở; chưa phải M5 hoàn chỉnh.

**Điểm bàn giao tiếp theo:** đã có [plan quỹ hộ M5.1a](docs/design/M5-1a-household-accounts-plan.md); Codex **chưa duyệt code**, xem [review bắt buộc sửa](docs/design/M5-1a-handoff.md)

- Kết quả đo được: Windows CTest 5/5 (`realm_tests` 25.298 check), toolchain Zig 0.13 + Clang 18 C++20, CMake 3.31.6, lệnh build/test ghi trong [M5 progress](docs/reports/M5-progress.md).
- Lưu ý về build/rebuild: Godot app và M4 đã được dựng và kiểm chứng trên Windows theo báo cáo M4-evidence; tuy nhiên một số đầu mục chưa được dựng/lặp lại cho các platform khác: ONNX Runtime integration (trên Linux/Windows nếu cần), shared Python binding build & test trên môi trường mục tiêu, Linux packaging và profile B không được chạy lại cho revision này. Nếu cần chứng nhận cross-platform, chạy lại workflow `native-linux.yml` và các bước package tương ứng.
- [Chuẩn bị dữ liệu RL và Kaggle](docs/experiments/rl-data-kaggle-preparation.md): dữ liệu từ rollout C++, seed train/validation/test rời nhau, controller đối thủ có nhãn; ...

1. Xong làn đầu M5 còn lại (M5.1–M5.5: hộ động/thừa kế/hôn nhân, tổ chức, quốc gia–kinh tế, profile B) qua cùng command/transaction pipeline; không báo gate pass nếu chưa chạy đầy đủ regression.
2. Khi cần kiểm chứng Linux: chạy tay workflow `native-linux.yml` (`workflow_dispatch`, giữ nguyên các bước/check), ghi kết quả riêng; không suy từ kết quả Windows.
3. Nền xã hội theo [ranh giới độ sâu](docs/design/simulation-complexity-scope.md) và [nghiên cứu cuối Trung cổ](docs/design/late-medieval-world-design-discussion.md); người d...
4. M5–M6 hoàn thiện luật/world rồi đóng băng action/observation/reward; production training và R1–R5 ở M7 sau feature freeze. Các model realm-v1–v6 tiếp tục là pilot.

M4 Windows: CTest 5/5 (campaign 202 checks), Godot native/UI/M4 19/18/39 và shared-core Python 5 checks; hồ sơ Hắc Sơn đóng được ở 10.000 người. [Báo cáo M4](docs/reports/M4-evidence.md)

Repo đã phân tài liệu vào design/developer/experiments/guides/handoff/reports và giao diện vào `godot_project/ui/`. [Mục lục docs](docs/README.md) và [quy ước thư mục](docs/developer/repository-layout.md)

## Các lệnh thường dùng

- Build/test/package: scripts/build-windows.ps1, scripts/test-native.ps1, scripts/package-windows.ps1; Linux package: scripts/package-linux.sh.
- Training: scripts/build-training.ps1, scripts/train-cpu.ps1, scripts/train-realm.ps1.
- CMake options, checks và các giới hạn platform: xem [README](README.md) và [báo cáo M3](docs/reports/M3-evidence.md).

Không tự commit/push khi chưa có yêu cầu cho tác vụ đó. Không báo test mới nếu chưa chạy lại.
