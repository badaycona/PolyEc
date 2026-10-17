# Copilot core refactor — sửa và review 2026-10-17

Người dùng xác nhận refactor core vừa kéo trong lịch sử tới a5647a5 do GitHub Copilot thực hiện. OpenCode thực hiện sửa source; Codex review plan/diff và chạy kiểm chứng độc lập. Không hoàn tác toàn commit; giữ WorldState và trích xuất FinanceManager/HouseholdIndex/ProjectManager, Realm vẫn sở hữu dữ liệu. [Plan đã review](../handoff/copilot-core-repair-plan.md).

Đã khôi phục các file đầy đủ từ d5751bb làm tham chiếu, áp lại extraction có chọn lọc; loại patch markers, khôi phục header command và implementation sim đúng vị trí, không mở public mutable accessors. Payroll giữ chia dư theo ngày, chuyển thuế từ cash sang treasury và cập nhật tax_receipts (counter, không phải tài sản). Header manager dùng đúng domain types và dependencies.

Windows: configure mới build/copilot-core-repair bằng Ninja, Zig 0.13/Clang 18.1.6, Release; APP/AI/PYTHON OFF. Build hoàn tất 25/25. Lệnh thực tế dùng đầy đủ đường dẫn CMake trong plan; lệnh cmake ngắn ban đầu thất bại vì không có trong PATH. Configure ban đầu thiếu nmake được thay bằng generator/toolchain rõ ràng, không tắt kiểm tra dependency.

CTest đầu: 5/6 pass, manager_tests fail do test kỳ vọng cash trước thuế sau khi settlement đã hoàn tất. OpenCode sửa kỳ vọng net 90, giữ assertion thuế và bảo toàn; build/test lại 6/6 pass. Codex chạy độc lập:

```powershell
& build/tooling/cmake/data/bin/ctest.exe --test-dir build/copilot-core-repair --output-on-failure
```

Kết quả exit 0, 6/6 pass, 8,36 giây: core, trade, realm, court, campaign và manager. Test manager dùng kiểm tra runtime/exit code, không bị NDEBUG vô hiệu hóa. git diff --check pass. Diff so d5751bb cho Realm header không đổi; command header chỉ khác BOM; Realm implementation chỉ khác các delegate/includes đã review. Save/action/observation/reward/model contract không đổi trong lát này.

Nghiệm thu lát sửa core Windows; không chứng nhận app/Godot, Python parity, ONNX/runtime, Linux, profile B hoặc chất lượng RL. Không train, commit hoặc push. Arithmetic overflow tổng quát không được mở rộng trong lát parity này; transfer giữ precondition/semantics cũ. Không coi mô tả helper là chứng nhận transaction hoàn chỉnh cho mọi input tùy ý. Bước sau: OpenCode plan-only cho timeline/clock/command/save theo thiết kế 17/10, review trước code.

Codex chạy thêm build/copilot-core-repair/core/polyec_realm_transition_replay.exe: exit 0, 64 ngày/256 người, invariant giữ và tổng tiền 308400 suốt trace; đây là smoke trên bản sửa, chưa đối chiếu fingerprint với executable trước refactor. Đã xóa file tạm tên `-` do compiler probe tạo ở root sau khi xác nhận nội dung là output CXX-DetectStdlib.h.
