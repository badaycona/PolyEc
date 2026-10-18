# PolyEc

Game desktop native mô phỏng cai trị một quốc gia cuối Trung cổ giả tưởng trong một đời vua. C++20 giữ luật/world; Godot dựng giao diện; Python train cùng core; ONNX CPU chạy learned policies. Dự án đang xây M5, chưa phải campaign/RL production hoàn chỉnh.

## Chọn đúng tài liệu

| Bạn cần | Nguồn chuẩn |
|---|---|
| Toàn thể sản phẩm, tính năng phải xây, roadmap và tech stack | [IMPLEMENTATION_PLAN](docs/IMPLEMENTATION_PLAN.md) |
| Cách chơi, điều khiển, tính năng bản thử và điều cần biết | [PLAYER_GUIDE](docs/PLAYER_GUIDE.md) |
| Dev: bài toán, tiếp cận, thuật toán, code, kiểm chứng và hạn chế | [DEVELOPER_GUIDE](docs/DEVELOPER_GUIDE.md) |
| Quyết định, thay đổi và bằng chứng theo thời điểm | [DEVELOPMENT_HISTORY](docs/DEVELOPMENT_HISTORY.md) |
| Agent/dev tiếp tục code: trạng thái, contract, scope và gate hiện tại | [CURRENT_TASK](docs/CURRENT_TASK.md) |

Agent đọc [AGENTS](AGENTS.md) rồi CURRENT_TASK trước khi mở source. [Mục lục docs](docs/README.md) giải thích thứ tự ưu tiên; WORK_STATUS chỉ chuyển hướng, không một bảng trạng thái độc lập.

## Chạy và phát triển

Bản chơi M4 có map sáu lãnh địa + đất triều đình/10k người, quyền đèo, logistics, thiếu lương thực, nợ và xưởng dệt. Controller kiểm thử có nhãn; sandbox M3 giữ policy pilot với quality gates chưa đạt đầy đủ. Clone source không tự có executable; nếu có gói Windows thì giải nén toàn bộ và mở PolyEc.exe. Xem PLAYER_GUIDE trước khi chơi; DEVELOPER_GUIDE cho build/test/package.

Source chính: core/ (world), bindings/python/ (cùng core để train), native/ (ONNX), gdext/ (adapter), godot_project/ (UI/assets), ai/ (trainer/eval), scripts/ và .github/workflows/. Không Python/Node/API key trên máy người chơi. Windows ưu tiên; portable core/Python vẫn phải giữ.

Reports là evidence theo revision. [Archive](docs/archive/README.md) giữ docs/prompt cũ; đường dẫn docs cũ chỉ redirect. [Artifacts](artifacts/README.md) là checkpoint/lock/source-snapshot/visual theo run, không spec active. Runtime models trong godot_project/models/ được app nạp theo identity, không tự thay bằng checkpoint mới nhất.

Third-party notices và font licenses: licenses/. Không commit/push/publish khi tác vụ chưa yêu cầu.
