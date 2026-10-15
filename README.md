# PolyEc

PolyEc đang được xây thành game desktop mô phỏng cai trị một quốc gia. Người chơi đọc báo cáo, thương lượng hoặc cấp nguồn lực cho dự án, rồi theo dõi tác động. Đích sản phẩm dùng learned RL cho quyết định tự chủ; lõi mô phỏng và giao diện chạy native trên Windows/Linux.

## Bản hiện có

Đã bổ sung **M4 · Đường đèo & nguồn sống**, lát cắt native với bản đồ và 10.000 người: xử lý quyền đèo/công trình, vận tải hữu hạn, thiếu lương thực, nợ và xưởng dệt. Mở `PolyEc.exe` trong `dist/PolyEc-M4-Windows`, chọn nút M4. Việc thường lệ của lát cắt dùng controller kiểm thử có nhãn; chưa phải campaign RL hoàn chỉnh. Xem [hướng dẫn M4](docs/guides/PLAYER-README-M4.txt) và [bằng chứng Windows/giới hạn](docs/reports/M4-evidence.md). Linux của thay đổi này chưa chạy.

Phần **M3: sandbox mô phỏng và tích hợp ba họ policy RL** vẫn được giữ. Khi build có ONNX Runtime, chọn **Quốc gia · Sandbox**. Đây là bản để kiểm tra cơ chế; chưa phải campaign hoàn chỉnh một đời vua.

M3 đã có sáu lãnh địa, đất triều đình, 10.000 người, mô phỏng kinh tế/chính trị cơ bản, draft augment triều đình và suy luận ONNX CPU. Các model hiện tại là pilot; quality gates sẽ được chạy lại sau khi luật và feature world hoàn tất ở M6. Xem [trạng thái hiện tại](WORK_STATUS.md) và [báo cáo M3](docs/reports/M3-evidence.md) để phân biệt kết quả pilot với model production.

## Bắt đầu đọc repo

1. [WORK_STATUS.md](WORK_STATUS.md) — repo đang ở đâu, gate nào còn mở, bước tiếp theo.
2. [docs/design/implementation-plan.md](docs/design/implementation-plan.md) — thiết kế đích và roadmap M0–M7.
3. [docs/README.md](docs/README.md) — mục lục kiến trúc, hướng dẫn, thí nghiệm và báo cáo.
4. [AGENTS.md](AGENTS.md) — các quy tắc phải giữ khi sửa code.
5. [Bài thuyết minh kỹ thuật](docs/developer/polyec-technical-paper.md) — bài toán, kiến trúc và nền tảng toán học dành cho developer.

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
├── docs/
│   ├── design/             Plan, domain và đề xuất độ sâu simulation
│   ├── developer/          Kiến trúc và tài liệu kỹ thuật
│   ├── experiments/        Protocol, schema và thí nghiệm RL
│   ├── guides/             Hướng dẫn người chơi theo bản build
│   ├── handoff/            OpenCode, review và bàn giao
│   └── reports/            Bằng chứng M3/M4 và giới hạn kiểm chứng
├── gdext/                   Adapter C++ cho Godot
├── godot_project/
│   ├── fonts/               Font đóng gói trong game
│   ├── models/              ONNX và manifest runtime được app nạp
│   ├── tests/               Godot smoke tests và fixture
│   ├── ui/
│   │   ├── main.gd         Điều hướng màn hình
│   │   ├── campaign/       Bản đồ và tương tác M4
│   │   ├── realm/          Sandbox quốc gia M3
│   │   ├── labs/           Phòng thử policy kinh tế
│   │   └── prototype/      Bản đồ prototype
│   └── main.tscn           Scene khởi động
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

Xem [hướng dẫn M4](docs/guides/PLAYER-README-M4.txt) để mở gói Windows; [báo cáo M4](docs/reports/M4-evidence.md) có lệnh build/test/package từ source. Gói local trong `dist/` không được đưa vào Git; clone repo không tự có executable. Xem [mục lục tài liệu](docs/README.md) và [WORK_STATUS.md](WORK_STATUS.md) để chọn đúng hướng dẫn và biết gate còn mở.

## License

Third-party notices và license font nằm trong licenses/.
