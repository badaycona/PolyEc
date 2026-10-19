# PolyEc

Game desktop native mô phỏng cai trị một quốc gia cuối Trung cổ giả tưởng trong một đời vua. C++20 giữ luật/world; Godot dựng giao diện; Python train cùng core; ONNX CPU chạy learned policies. Dự án đang xây M5, chưa phải campaign/RL production hoàn chỉnh.

## Mục đích của game

Người chơi là vua: quan sát, thương lượng, bổ nhiệm, cấp nguồn lực và theo dõi việc thi hành. Mục tiêu là một thế giới có hậu quả hiểu được: đường tắc làm hàng tới chậm; xưởng thiếu vật tư có thể thiếu lương; người lao động đổi việc; vấn đề kinh tế trở thành vấn đề chính trị. Thành công được nhìn qua đời sống dân cư, tài chính, thể chế và cái giá của quyền lực, không chỉ tiền hay diện tích lãnh thổ.

## Giai đoạn hiện tại

Dự án đang ở **M5 — nền xã hội và mở rộng quốc gia**. Bản M4 là màn chơi thử; M3 là thử nghiệm AI. Campaign một đời vua và learned policy đạt chất lượng sản phẩm vẫn chưa hoàn chỉnh. Phần đã nghiệm thu, phần đang review và việc cần làm ngay chỉ cập nhật tại [CURRENT_TASK](docs/CURRENT_TASK.md); README chỉ định vị giai đoạn chung.

## Chọn đúng tài liệu

| Bạn cần | Nguồn chuẩn |
|---|---|
| Toàn thể sản phẩm, tính năng phải xây, roadmap và tech stack | [IMPLEMENTATION_PLAN](docs/IMPLEMENTATION_PLAN.md) |
| Cách chơi, điều khiển, tính năng bản thử và điều cần biết | [PLAYER_GUIDE](docs/PLAYER_GUIDE.md) |
| Dev: từ bài toán lớn tới bài toán nhỏ, phương pháp và phép kiểm | [TECHNICAL_PROBLEMS](docs/TECHNICAL_PROBLEMS.md), rồi [DEVELOPER_GUIDE](docs/DEVELOPER_GUIDE.md) để đọc công thức/thuật toán sâu |
| Codex/OpenCode: cách giao, review, triển khai và nghiệm thu từng lát | [DEVELOPMENT_WORKFLOW](docs/DEVELOPMENT_WORKFLOW.md), sau AGENTS và CURRENT_TASK |
| Quyết định, thay đổi và bằng chứng theo thời điểm | [DEVELOPMENT_HISTORY](docs/DEVELOPMENT_HISTORY.md) |
| Agent/dev tiếp tục code: trạng thái, contract, scope và gate hiện tại | [CURRENT_TASK](docs/CURRENT_TASK.md) |

Agent đọc [AGENTS](AGENTS.md) rồi CURRENT_TASK trước khi mở source. [Mục lục docs](docs/README.md) giải thích thứ tự ưu tiên; WORK_STATUS chỉ chuyển hướng, không một bảng trạng thái độc lập.

## Chạy và phát triển

Bản chơi M4 có map sáu lãnh địa + đất triều đình/10k người, quyền đèo, logistics, thiếu lương thực, nợ và xưởng dệt. Controller kiểm thử có nhãn; sandbox M3 giữ policy pilot với quality gates chưa đạt đầy đủ. Clone source không tự có executable; nếu có gói Windows thì giải nén toàn bộ và mở PolyEc.exe. Xem PLAYER_GUIDE trước khi chơi; DEVELOPER_GUIDE cho build/test/package.

Source chính: core/ (world), bindings/python/ (cùng core để train), native/ (ONNX), gdext/ (adapter), godot_project/ (UI/assets), ai/ (trainer/eval), scripts/ và .github/workflows/. Không Python/Node/API key trên máy người chơi. Windows ưu tiên; portable core/Python vẫn phải giữ.

Reports là evidence theo revision. [Archive](docs/archive/README.md) giữ docs/prompt cũ; đường dẫn docs cũ chỉ redirect. [Artifacts](artifacts/README.md) là checkpoint/lock/source-snapshot/visual theo run, không spec active. Runtime models trong godot_project/models/ được app nạp theo identity, không tự thay bằng checkpoint mới nhất.

Third-party notices và font licenses: licenses/. Không commit/push/publish khi tác vụ chưa yêu cầu.

## Tech stack và trách nhiệm

| Lớp | Công nghệ | Trách nhiệm |
|---|---|---|
| Mô phỏng | C++20, OOP/composition, CMake | World, quyền, command, transaction, game time và invariants; độc lập Godot |
| App native | Godot 4.5.x, GDExtension/godot-cpp, GDScript | Bản đồ/UI; gửi command, nhận view chỉ đọc |
| Huấn luyện | Python, PyTorch, pybind11 | Gọi cùng C++ core; không viết bộ luật Python thứ hai |
| AI khi chơi | ONNX Runtime CPU | Shared learned weights, memory/sampling từng người, bounded batches |
| Kiểm chứng | CTest, Python/parity, Godot smoke, CI | Kiểm luật, tương thích, hành vi và hiệu năng riêng biệt |

Luồng chính: UI hoặc policy → command → core validate/commit → facts → thông tin được phép biết → view. Core thực thi luật; learned RL chọn quyết định tự chủ của NPC trong sản phẩm hoàn chỉnh. Baseline viết tay chỉ dùng dev/test có nhãn.

Stack chuẩn ở [plan phần 5](docs/IMPLEMENTATION_PLAN.md); phiên bản đã kiểm chứng, pin và lệnh build ở [Developer Guide phần 11](docs/DEVELOPER_GUIDE.md). Bảng này không là yêu cầu tự nâng dependency.

## Folder tree

Cây định hướng các thư mục quan trọng, không liệt kê mọi file hoặc output local:

```text
PolyEc/
├── AGENTS.md                 Quy tắc cho agent
├── CMakeLists.txt            Cấu hình build
├── core/
│   ├── include/polyec/       API và domain C++
│   ├── src/                  Luật và chuyển trạng thái
│   └── tests/                Core tests, replay, invariants
├── bindings/python/          Python gọi cùng core
├── native/                   ONNX runtime, replay, benchmark
├── gdext/                    Cầu Godot ↔ core/runtime
├── godot_project/
│   ├── ui/                   Giao diện và bản đồ
│   ├── tests/                Smoke/UI harness
│   └── models/               Model runtime khóa identity
├── ai/
│   ├── training/             Model, trainer, export
│   ├── evaluation/           Đánh giá hành vi/profile
│   └── tests/                Binding/parity fixtures
├── scripts/                  Build, test, package, training
├── .github/workflows/        CI
├── docs/                     Sản phẩm, kỹ thuật, tác vụ, lịch sử
│   ├── design/               Supporting contract theo lát
│   ├── reports/              Evidence theo revision/run
│   ├── guides/               Hướng dẫn theo gói
│   └── archive/              Lịch sử, không là lệnh active
├── artifacts/                Checkpoint, manifest, source lock
├── third_party/              Dependency bên thứ ba
├── licenses/                 Notices và giấy phép
└── build/                    Output/toolchain local, không là source
```

Điểm vào class/file cụ thể nằm trong [Developer Guide phần 11](docs/DEVELOPER_GUIDE.md). Đường đọc và nơi cập nhật từng loại thông tin nằm trong [bản đồ tài liệu](docs/README.md).
