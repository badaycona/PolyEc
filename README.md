# PolyEc

PolyEc đang được xây thành game desktop mô phỏng cai trị một quốc gia. Người chơi đọc báo cáo, thương lượng hoặc cấp nguồn lực cho dự án, rồi theo dõi tác động. Các NPC tự chọn hành động bằng policy RL đã huấn luyện; lõi mô phỏng và giao diện chạy native trên Windows/Linux.

## Bản hiện có

Repo hiện ở **M3: sandbox mô phỏng và tích hợp ba họ policy RL**. Mở godot_project bằng bản portable trong gói phát hành, rồi chọn **Quốc gia · Sandbox**. Đây là bản để kiểm tra cơ chế; chưa phải campaign hoàn chỉnh một đời vua.

M3 đã có sáu lãnh địa, đất triều đình, 10.000 người, mô phỏng kinh tế/chính trị cơ bản, draft augment triều đình và suy luận ONNX CPU. Các kiểm tra build, runtime và UI đã chạy trên Windows/Linux. Chất lượng policy còn thiếu: R1/R2/R3 chưa đạt đầy đủ. Xem [trạng thái hiện tại](WORK_STATUS.md) và [báo cáo M3](docs/M3-evidence.md) trước khi diễn giải kết quả.

## Bắt đầu đọc repo

1. [WORK_STATUS.md](WORK_STATUS.md) — repo đang ở đâu, gate nào còn mở, bước tiếp theo.
2. [docs/implementation-plan.md](docs/implementation-plan.md) — thiết kế đích và roadmap M0–M7.
3. [docs/README.md](docs/README.md) — mục lục kiến trúc, hướng dẫn, thí nghiệm và báo cáo.
4. [AGENTS.md](AGENTS.md) — các quy tắc phải giữ khi sửa code.

## Folder tree

~~~text
PolyEc/
├── .github/workflows/       CI build và kiểm tra
├── ai/
│   ├── evaluation/          Báo cáo và evaluator cho policy
│   ├── tests/               Binding, training, replay và ONNX parity
│   └── training/            Mô hình PPO, export và runner huấn luyện
├── artifacts/
│   ├── models/              Checkpoint, config, evaluation và khóa nguồn theo phiên bản
│   └── visuals/             Ảnh chart/preview được sinh từ các đợt kiểm tra
├── bindings/python/         Cầu nối Python ↔ C++ core
├── core/
│   ├── include/polyec/      API và kiểu dữ liệu mô phỏng
│   ├── src/                 Luật thế giới, kinh tế và triều đình
│   └── tests/               Test core và replay
├── docs/                    Plan, kiến trúc, hướng dẫn, báo cáo, mục lục
├── gdext/                   Adapter C++ cho Godot
├── godot_project/
│   ├── fonts/               Font đóng gói trong game
│   ├── models/              ONNX và manifest runtime được app nạp
│   ├── tests/               Godot smoke tests và fixture
│   └── *.gd, *.tscn         Màn hình, map và panel
├── licenses/                Giấy phép third-party và asset
├── native/
│   ├── include/polyec/      API runtime AI native
│   └── *.cpp                ONNX runtime, replay, benchmark, contract tests
├── scripts/                 Build, test, train, package
├── third_party/godot-cpp/   Dependency Godot C++ đã pin
├── CMakeLists.txt           Build core, AI, binding và app
├── AGENTS.md                Quy tắc làm việc trong repo
├── README.md                Trang bắt đầu
└── WORK_STATUS.md           Trạng thái thực tế
~~~

Thư mục build, .godot, binary sinh ra và gói phát hành không được liệt kê; chúng là output/cache. artifacts/ giữ checkpoint, manifest, evaluation và chart theo đợt; godot_project/models chỉ giữ bản ONNX/manifest mà game nạp lúc chạy. Các phiên bản model cũ được giữ để truy nguyên cả kết quả thất bại.

## Cấu trúc code

- core sở hữu world state, luật, invariant và transaction. Thay đổi trạng thái gameplay phải đi qua Action/validator.
- bindings/python cho training/evaluation dùng đúng C++ core.
- native chạy policy ONNX, sampling cá nhân và kiểm tra điều kiện runtime.
- gdext chuyển dữ liệu read-only sang Godot và nhận command từ UI.
- godot_project chứa màn hình, map, panel, model runtime assets và bài kiểm tra giao diện.

## Build và chạy

Xem [hướng dẫn người chơi](docs/PLAYER-README-REALM.txt) để mở gói Windows. Cần toolchain C++/Godot để build từ source; xem [WORK_STATUS.md](WORK_STATUS.md) để biết toolchain đã dùng và [docs/README.md](docs/README.md) để tìm script theo mục đích.

## License

Third-party notices và license font nằm trong licenses/.
