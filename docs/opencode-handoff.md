# PolyEc — Bộ giao việc cho OpenCode

Ngày: 14/10/2026. Công cụ coding agent đã chọn: **OpenCode**.

Đây là tài liệu để bắt đầu triển khai theo từng mốc, chưa phải mã game hoặc báo cáo build thành công. OpenCode chưa được cài/chạy trong môi trường đang dùng để sửa tài liệu. Không có provider, API key hoặc model được cấu hình trong bước này.

## 1. Cách dùng

1. Cài OpenCode trên máy phát triển Windows/Linux theo [tài liệu chính thức](https://opencode.ai/docs/). Có thể dùng WSL cho phần CLI trên Windows; vẫn cần kiểm tra bản app Windows native ở mốc đóng gói.
2. Cấu hình provider/model bằng cơ chế kết nối của OpenCode. Không ghi API key vào prompt, repository hoặc tài liệu.
3. Mở checkout PolyEc. Chép bản kế hoạch v3 vào `docs/implementation-plan.md` và tài liệu này vào `docs/opencode-handoff.md`; việc chép thực hiện khi bắt đầu bước code, không tự ghi đè file đã có.
4. Tạo/ghép quy tắc mục 3 vào `AGENTS.md` của dự án, giữ các quy tắc hiện có nếu có. Không cần tạo Git worktree: cloud task hiện có đã cô lập; chỉ tạo nếu người dùng yêu cầu.
5. Chạy OpenCode tại root repository. Đầu tiên yêu cầu lập phương án cho **M0a** bằng prompt ở mục 5, sau đó giao triển khai theo prompt mục 6.
6. Hoàn tất nghiệm thu một mốc trước khi mở rộng. Nếu phải đổi hợp đồng hoặc phạm vi, cập nhật spec và nêu rõ tác động.

OpenCode là agent điều phối việc đọc/sửa code và chạy công cụ. Khả năng và quota phụ thuộc provider/model đã kết nối; cài OpenCode không đồng nghĩa có inference miễn phí hoặc không giới hạn. Không cần dùng thêm coding agent thứ hai để bắt đầu dự án.

## 2. Tài liệu nền và thứ tự thực hiện

- Kế hoạch nguồn: `PolyEc-implementation-plan-v3-native-rl.md`; dùng bản v3, không dùng plan cũ game web 4–6 vùng.
- Hướng sản phẩm: app desktop Windows/Linux, RAM 16 GB, RL inference bằng CPU.
- Stack đề xuất: Godot native, C++20 OOP, CMake; Python/PyTorch training qua pybind11; ONNX Runtime native.
- Tài liệu giải thích thuật ngữ/gameplay phải viết dễ hiểu và có bảng Hướng dẫn F1 trong game.

M0 của plan được chia thành hai phần để kiểm tra nền trước:

| Phần | Mục tiêu |
|---|---|
| M0a | App native tối thiểu, core C++ OOP, headless test, hai vùng chọn được, clock/pause |
| M0b | Python binding, parity cùng core, ONNX CPU shape/memory test, spike citizen/sparse graph 10k/100k/1m |

M0a chưa đạt toàn bộ M0. M0b không được bỏ đi; chưa có policy đã train trong cả hai phần. Sau M0 mới tiếp tục kinh tế, chính trị/cá thể, RL, vertical slice và quốc gia đầy đủ theo plan.

## 3. Quy tắc dự án để đưa vào AGENTS.md

- Sản phẩm là app game native; không chuyển thành React/Vite/web game hoặc bọc một trang web để thay mục tiêu.
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

## 4. Đặc tả M0a

### Phạm vi

Tạo nền dự án native đủ chứng minh core và app giao tiếp được. Hai vùng thử là Thanh Điền và Hắc Sơn. Map prototype có đường nối và hình vùng dễ chọn; art địa hình đầy đủ, kinh tế và RL thuộc mốc sau.

### Hợp đồng tối thiểu

| Đối tượng | Trách nhiệm |
|---|---|
| EntityId | Định danh ổn định, không dùng tên vùng làm khóa |
| GameDate | Ngày nguyên; lịch giả tưởng 360 ngày/năm |
| Territory | ID, tên, thông tin nền và vị trí; không phụ thuộc Node2D |
| GameWorld | Sở hữu vùng và ngày game; nhận Command và trả Result |
| SimulationController | Sở hữu clock, pause, tốc độ; UI không tự tăng ngày |
| PlayerView | Dữ liệu vùng/ngày đã lọc, chỉ đọc từ phía UI |
| GodotAdapter | Chuyển input thành Command, đưa snapshot lên UI |

Domain advance một ngày phải chạy headless được. Scheduler thực dùng đồng hồ đơn điệu; test dùng fake time. Pause là trạng thái khởi đầu. Resume ở 1× tương đương một ngày game/giây thực; dùng đơn vị ngày nguyên với accumulator, không nhân delta render thẳng vào tuổi hoặc lịch.

### Luồng người thử

Mở app → thấy hai vùng và ngày → click vùng để xem panel → resume cho ngày tăng → pause để ngày đứng → mất focus/thu nhỏ cửa sổ khiến pause → trở lại không chạy bù.

### Tiêu chí nghiệm thu

- C++ core configure/build được bằng CMake và không include Godot trong domain.
- Test headless thực sự kiểm tra advance, pause/resume và ID vùng; không chỉ có test luôn pass.
- Native app nạp được GDExtension và hiện dữ liệu từ core, không phải hai bộ state giả độc lập.
- Picking chọn đúng vùng; UI input không phát một Command hai lần.
- GameDate không thay đổi khi pause; pause/resume không tạo hai scheduler.
- Có README với version/toolchain, lệnh build/test/run đúng với implementation.
- Build Windows/Linux nào chưa chạy được phải báo rõ; không suy rằng pass Linux nghĩa là pass Windows.
- Không có dependency Python/Node trên máy người chơi cho app M0a.

### Chưa làm trong M0a

Kinh tế 16 hàng, 24 vùng, RL training, map art đầy đủ, save campaign dài và một triệu người. Không tạo nút giả cho các chức năng này. Tách rõ prototype hai vùng khỏi cam kết phạm vi bản hoàn chỉnh.

## 5. Prompt giao OpenCode lập phương án M0a

> Đọc AGENTS.md, docs/implementation-plan.md và docs/opencode-handoff.md. Dùng kế hoạch v3 native/OOP/RL, không dùng hướng web cũ. Chỉ chuẩn bị phương án M0a trước khi sửa code: kiểm tra repository và toolchain, đề xuất file/class/interface, Godot/GDExtension version tương thích, CMake build và headless test. Nêu dependency cần cài, điều kiện chưa xác minh và cách nghiệm thu. Không mở rộng sang kinh tế/RL training/triệu người. Không bỏ M0b khỏi roadmap.

## 6. Prompt giao OpenCode triển khai M0a

> Triển khai M0a theo docs/opencode-handoff.md và phương án vừa chốt. Tạo app Godot native với lõi C++20 OOP độc lập Godot, hai vùng Thanh Điền/Hắc Sơn chọn được, panel lấy dữ liệu từ core, đồng hồ ngày, pause/resume và pause khi mất focus. Tạo CMake/headless tests, chạy build/test phù hợp và thử native app khi có khả năng. Sửa lỗi setup/build trong phạm vi M0a, không thay sản phẩm thành web. Không làm RL hoặc kinh tế đầy đủ. Cuối tác vụ báo file thay đổi, command đã chạy, test count/result, hệ điều hành đã kiểm chứng và blocker còn lại. Không tự commit/push/publish.

## 7. Cách giữ quota và tránh agent làm lại

- Mỗi task có spec nhỏ, ranh giới file và gate. Không đưa lại cả plan dài vào mọi prompt; yêu cầu đọc mục cần thiết.
- Dùng model mạnh cho kiến trúc, lỗi khó và review; tác vụ đã rõ có thể dùng model ít tốn hơn nếu chất lượng đáp ứng. Model nào phụ thuộc provider thực tế, chưa khóa trong tài liệu này.
- Giữ WORK_STATUS.md ngắn: mốc đã đạt, commands/results, vấn đề mở, version đã pin và task tiếp theo. Đây là trạng thái làm việc, không thay test report.
- Cho agent tiếp tục từ artifact và code hiện có; không yêu cầu “viết lại toàn bộ” khi chỉ một module lỗi.
- Đánh giá trên build/test và hành vi thực, không dựa vào số dòng code sinh ra.
- Khi bắt đầu RL, giao riêng environment/reward/parity, rồi training experiment, rồi evaluation/export; không gộp “viết mạng + train + thông minh + triệu NPC” vào một task.

## 8. Trạng thái bàn giao hiện tại

Đã chọn OpenCode và chuẩn bị hướng dẫn/prompt/spec M0a. Chưa cài OpenCode, kết nối provider, tạo source, build app hoặc train model. File kế hoạch v3 vẫn là nguồn scope; tài liệu này là hướng dẫn thực hiện.
