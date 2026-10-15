# PolyEc — AGENTS.md

Sản phẩm là app game native; không chuyển thành React/Vite/web game hoặc bọc một trang web để thay mục tiêu.

- Lõi mô phỏng dùng C++20 OOP, độc lập Godot. Domain có ownership và invariant; ưu tiên composition, không tạo cây kế thừa cho mọi tổ hợp nhân vật.
- Godot nhận PlayerView chỉ đọc và gửi Command. Không sửa tiền, kho, đồng hồ hoặc citizen state trực tiếp từ UI.
- Training Python phải gọi cùng C++ core; không viết một bản luật kinh tế khác để train cho tiện.
- Quyết định tự chủ trong bản hoàn chỉnh phải đến từ learned RL policy. Baseline chỉ dành cho dev/test và phải có nhãn; random weights, hardcoded strategy hoặc imitation-only không được gọi là RL đã hoàn thành.
- Luật thế giới, quyền hạn, Action validation, hạch toán và pathfinding vẫn được lập trình. Chúng thực hiện quyết định, không bí mật chọn chiến lược thay NPC.
- Mỗi cá thể có ID, trạng thái và vòng đời. Trọng số model shared; memory/sampling cá nhân. Không cấp một scene node hoặc model riêng cho mỗi người.
- Quan hệ thưa; không dùng ma trận dense N×N cho hàng triệu người. Không đếm pixel, log hoặc duplicate view như NPC.
- Sự kiện campaign phát sinh từ world/action/causal chain. Tutorial/test có thể dùng tình huống định sẵn nhưng không tráo thành cơ chế campaign.
- Mọi thay đổi gameplay phải đi qua cùng command/transaction pipeline cho UI và AI; Action reject không thay đổi một phần state.
- Dùng game time cho deadline/cooldown; pause không tiến lịch. Không chạy bù sau mất focus hoặc máy ngủ.
- Không tự giảm scope, bỏ test, tắt kiểm tra dependency/TLS hoặc dùng placeholder thành công để vượt gate.
- Không ghi credential vào code/log; không tự mua tài nguyên GPU hoặc dịch vụ cloud. Huấn luyện dài là tác vụ riêng có ngân sách đo được.
- Không tự commit/push/publish nếu tác vụ chưa yêu cầu. Giữ các thay đổi người dùng đã có.
- Chỉ báo check pass khi đã thực sự chạy và có kết quả; phân biệt app đã build, model đã train và profile đã benchmark.
- **Phân vai thực thi:** OpenCode là bên làm code chính. Codex chia tác vụ có ranh giới file/gate, ra chỉ dẫn, review plan/diff và bằng chứng, cùng phối hợp nghiệm thu. Codex không tự ý thay OpenCode viết code mới trong tác vụ đã giao; nếu phải sửa thì phải nêu rõ và review lại. Codex có thể tìm lỗi, chặn test sai và chỉ ra operator-precedence/logic bug, nhưng phần sửa gameplay chính thuộc OpenCode.

## Quy trình triển khai với OpenCode — cập nhật 2026-10-15

- OpenCode thực hiện phần triển khai chính; Codex điều hành, review phương án/diff và kiểm chứng kết quả. Giao từng tác vụ nhỏ có ranh giới file và gate, không giao toàn milestone trong một prompt triển khai.
- Trước mỗi tác vụ sửa code, OpenCode khảo sát có giới hạn và trình implementation plan ngắn, chưa sửa code: file/class/hàm cần đổi; ownership và invariant; luồng thành công, reject và trường hợp biên; tác động save/binding/action/observation/reward/model; test và lệnh nghiệm thu. Không rà toàn repo/toolchain khi thông tin liên quan đã có.
- Codex review plan trước khi giao triển khai. Các quyết định kỹ thuật trong spec đã duyệt được tự xử lý; chỉ hỏi người dùng khi phải đổi gameplay, phạm vi hoặc ngân sách. Nếu cần đổi kiến trúc/contract đáng kể trong lúc làm, cập nhật và review lại plan trước khi tiếp tục.
- OpenCode triển khai theo lát cắt, chạy test liên quan và regression; Codex review diff và bằng chứng trước tác vụ kế tiếp. Plan không thay test. Chi tiết và mẫu giao việc: docs/handoff/opencode-handoff.md, mục quy trình hiện hành.
