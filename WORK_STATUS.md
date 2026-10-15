# PolyEc — trạng thái làm việc

Cập nhật: 2026-10-15. Trạng thái này mô tả mã hiện có; [implementation plan](docs/design/implementation-plan.md) mô tả đích sản phẩm.

## Hiện tại

- **M4 đã có lát cắt native và được kiểm chứng Windows local:** bản đồ sáu lãnh địa + đất triều đình/10.000 người, quyền đèo–công trình–vận tải–nợ–hồ sơ và xưởng dệt. M4 dùng controller kiểm thử có nhãn; Linux mới chưa chạy, chưa production RL. Xem [báo cáo M4](docs/reports/M4-evidence.md).

- **M5 đã bắt đầu (mới xong lát cắt hộ gia đình):** `Realm` giờ có membership index thưa theo household (`household_of`, `household_members`, `same_household`, `guardian`, `household_authority`) thay cho quét ID..ID+3, hỗ trợ ID không liền nhau và sinh con. Index là state dẫn xuất, được dựng lại khi load; không thêm tiền hay bản sao world. Save `realm-v6`/magic `POLYEC_REALM_6` giữ nguyên (không đổi schema, không có dynamic household command), nên không có gói model cũ nào bị từ chối vì lý do tương thích giả tạo; trong khi đó household label rỗng hoặc ngoài dải người bị reject như save hỏng. Kết quả Windows: CTest 5/5, `realm_tests` 25.298 check, toolchain Zig 0.13 + Clang 18 C++20 / CMake 3.31.6; xem [M5 progress](docs/reports/M5-progress.md).

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

**Quy trình người dùng yêu cầu (2026-10-15):** OpenCode lập implementation plan ngắn trước mỗi tác vụ code → Codex review → OpenCode triển khai/test → Codex nghiệm thu. Xem [quy trình hiện hành](docs/handoff/opencode-handoff.md) và [M5 progress](docs/reports/M5-progress.md). Áp dụng ở điểm bàn giao an toàn gần nhất của công việc đang chạy, không hủy/viết lại phần đã kiểm chứng; chỉ hỏi người dùng khi cần đổi gameplay, phạm vi hoặc ngân sách đã duyệt.

**Trạng thái duyệt M5 (2026-10-15):** người dùng đã duyệt triển khai gameplay M5; plan đã duyệt này điều hành công việc hiện tại. Ưu tiên Windows trước; việc kiểm chứng app/package Linux bị hoãn trong M5 (theo ưu tiên tài nguyên của người dùng), phần core/Python portable vẫn phải kiểm thử để chuẩn bị Kaggle. Điều này **không** xóa yêu cầu portability/testing của core và **không** cho phép giảm gameplay, bỏ test hoặc bỏ invariant. Bằng chứng M4 cũ (build/test/package Windows/Linux ở revision đã push) vẫn được giữ để truy nguyên.

Mới xong: **lát cắt hộ gia đình đầu tiên** — muộn nhất membership index thưa + giám hộ + reject save hỏng. M5.1–M5.5 còn mở; chưa phải M5 hoàn chỉnh.

**Điểm bàn giao tiếp theo:** đã có [plan quỹ hộ M5.1a](docs/design/M5-1a-household-accounts-plan.md); Codex **chưa duyệt code**, xem [review bắt buộc sửa](docs/design/M5-1a-household-accounts-review.md). Cần giải quyết consent/reservation, observation theo quyền và chặn model cũ tại runtime, rồi review lại plan. Lượt OpenCode plan-only gặp `Endpoint is unavailable` và `Rate limit exceeded` ngày 2026-10-15, đang tự retry; không hủy lượt đang chạy hoặc tự thay OpenCode triển khai. Không có source M5.1a mới. Khi provider hoạt động lại, giao sửa plan theo review trước, không code từ bản plan đầu.

- Kết quả đo được: Windows CTest 5/5 (`realm_tests` 25.298 check), toolchain Zig 0.13 + Clang 18 C++20, CMake 3.31.6, lệnh build/test ghi trong [M5 progress](docs/reports/M5-progress.md). Linux chưa chạy lại cho thay đổi M5 này; không suy từ kết quả Windows. Không có train/GPU mới.
- Chưa build lại Godot app, ONNX Runtime, binding Python, package Linux, profile B; chưa có model đã train.
- [Chuẩn bị dữ liệu RL và Kaggle](docs/experiments/rl-data-kaggle-preparation.md): dữ liệu từ rollout C++, seed train/validation/test rời nhau, controller đối thủ có nhãn; pilot realm-v6 CPU chưa sẵn GPU và chưa tương thích world M5 mới.

1. Xong làn đầu M5 còn lại (M5.1–M5.5: hộ động/thừa kế/hôn nhân, tổ chức, quốc gia–kinh tế, profile B) qua cùng command/transaction pipeline; không báo gate pass nếu chưa chạy.
2. Khi cần kiểm chứng Linux: chạy tay workflow `native-linux.yml` (`workflow_dispatch`, giữ nguyên các bước/check), ghi kết quả riêng; không suy từ kết quả Windows.
3. Nền xã hội theo [ranh giới độ sâu](docs/design/simulation-complexity-scope.md) và [nghiên cứu cuối Trung cổ](docs/design/late-medieval-world-design-discussion.md); người dùng đã chọn bối cảnh cuối Trung cổ. Sáu vùng/10.000 người của M4 không phải quyết định giảm scope sản phẩm.
4. M5–M6 hoàn thiện luật/world rồi đóng băng action/observation/reward; production training và R1–R5 ở M7 sau feature freeze. Các model realm-v1–v6 tiếp tục là pilot.

M4 Windows: CTest 5/5 (campaign 202 checks), Godot native/UI/M4 19/18/39 và shared-core Python 5 checks; hồ sơ Hắc Sơn đóng được ở 10.000 người. [Báo cáo M4](docs/reports/M4-evidence.md) ghi workload, ảnh, số đo và giới hạn. Gói local nằm ở `dist/PolyEc-M4-Windows`; build/dist không được đưa vào Git.

Repo đã phân tài liệu vào design/developer/experiments/guides/handoff/reports và giao diện vào `godot_project/ui/`. [Mục lục docs](docs/README.md) và [quy ước thư mục](docs/developer/repository-layout.md) là điểm tìm đường dẫn mới. OpenCode local/cách sửa 403 và hồ sơ review được giữ ở [handoff](docs/handoff/opencode-handoff.md).

## Các lệnh thường dùng

- Build/test/package: scripts/build-windows.ps1, scripts/test-native.ps1, scripts/package-windows.ps1; Linux package: scripts/package-linux.sh.
- Training: scripts/build-training.ps1, scripts/train-cpu.ps1, scripts/train-realm.ps1.
- CMake options, checks và các giới hạn platform: xem [README](README.md) và [báo cáo M3](docs/reports/M3-evidence.md).

Không tự commit/push khi chưa có yêu cầu cho tác vụ đó. Không báo test mới nếu chưa chạy lại.
