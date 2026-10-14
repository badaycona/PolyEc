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
