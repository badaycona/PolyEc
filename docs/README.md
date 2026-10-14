# Tài liệu PolyEc

README.md ở repo root là điểm bắt đầu. WORK_STATUS.md ghi trạng thái code hiện tại; implementation plan mô tả sản phẩm đích.

## Trạng thái và sản phẩm

- [Trạng thái làm việc](../WORK_STATUS.md): milestone, gate còn mở và bước tiếp theo.
- [Báo cáo M3](M3-evidence.md): kết quả kiểm tra, holdout RL, sandbox dài hạn và giới hạn.
- [Hướng dẫn sandbox Quốc gia](PLAYER-README-REALM.txt): cách chạy và điều khiển bản demo.
- [Hướng dẫn RL lab](PLAYER-README-RL.txt): merchant RL lab.
- [Hướng dẫn prototype map](PLAYER-README.txt): bản prototype cũ.

## Thiết kế và kiến trúc

- [Implementation plan](implementation-plan.md): mục tiêu sản phẩm, roadmap M0–M7 và gates.
- [Realm domain](realm-domain.md): domain model.
- [OpenCode handoff](opencode-handoff.md): workflow lịch sử để giao việc coding agent.

## RL và thí nghiệm

- [Realm v6 protocol](realm-rl-experiment-v6.md): protocol training/evaluation của M3.
- realm-rl-experiment-v1.md đến realm-rl-experiment-v5.md: các lần thử trước; hồ sơ số liệu nằm trong artifacts/models/realm-vN/.
- [Đánh giá runtime v3](realm-runtime-evaluation-v3.md): đánh giá runtime phiên bản cũ.
- [Economic v1 experiment](rl-experiment-v1.md) và [schema](rl-schema-v1.md): merchant RL lab.

## Artifact và runtime assets

- artifacts/models/economic-v1..v2 và realm-v1..v6 giữ checkpoint, config, evaluation, source lock và kết quả. Hồ sơ cũ giữ nguyên để truy nguyên cả thử nghiệm thất bại.
- artifacts/visuals/ giữ chart training, chart evidence và ảnh preview.
- godot_project/models/ giữ bản ONNX và manifest mà game nạp qua res://models. Đây là runtime assets được đóng gói cùng app.
- Khi chạy thí nghiệm mới, tạo thư mục run mới dưới artifacts/models; không ghi đè hồ sơ đã khóa.

## Folder tree

~~~text
docs/
├── implementation-plan.md   Thiết kế đích và roadmap
├── M3-evidence.md           Báo cáo kết quả/gate M3
├── realm-domain.md          Domain model
├── realm-rl-experiment-v1…v6.md
├── realm-runtime-evaluation-v3.md
├── rl-experiment-v1.md      Merchant RL
├── rl-schema-v1.md
├── PLAYER-README*.txt       Hướng dẫn người chơi
├── README.md                Mục lục tài liệu và artifact
└── opencode-handoff.md      Quy trình OpenCode lịch sử
~~~
