# Tài liệu PolyEc

[WORK_STATUS.md](../WORK_STATUS.md) ghi trạng thái mã và gate còn mở. [Implementation plan](design/implementation-plan.md) mô tả đích sản phẩm. M4 hiện được kiểm chứng Windows với controller kiểm thử có nhãn; chưa chứng nhận RL production hoặc Linux của thay đổi M4.

## Đọc theo mục đích

| Mục đích | Tài liệu |
|---|---|
| Chơi thử M4 | [Hướng dẫn M4](guides/PLAYER-README-M4.txt) |
| Hiểu kiến trúc và toán học | [Bài thuyết minh kỹ thuật](developer/polyec-technical-paper.md) |
| Build/test/package và kết quả M4 | [Báo cáo M4](reports/M4-evidence.md) |
| Chất lượng policy pilot | [Báo cáo M3](reports/M3-evidence.md) |
| Tiếp tục thiết kế xã hội | [Ranh giới độ sâu](design/simulation-complexity-scope.md), [nghiên cứu cuối Trung cổ](design/late-medieval-world-design-discussion.md) |
| Duyệt phạm vi trước khi code M5 | [Draft M5: luật, giao dịch, quốc gia và gates](design/M5-draft.md) |
| Tiến độ M5 và lệnh đã chạy | [Báo cáo M5-progress](reports/M5-progress.md) |
| Chuẩn bị dữ liệu RL và GPU | [RL data & Kaggle preparation](experiments/rl-data-kaggle-preparation.md) |
| Tìm trách nhiệm domain | [Realm domain](design/realm-domain.md) |
| Tiếp tục với OpenCode | [Handoff](handoff/opencode-handoff.md), [review thiết kế](handoff/opencode-simulation-review-2026-10-15.md) |

## Folder tree

~~~text
docs/
├── README.md
├── design/                 Đích sản phẩm, domain và đề xuất độ sâu
├── developer/              Bài thuyết minh và quy ước cấu trúc source
├── experiments/            Protocol, schema và thí nghiệm RL
├── guides/                 PLAYER-README theo bản build
├── handoff/                OpenCode và hồ sơ review
└── reports/                M3/M4 evidence và giới hạn kiểm chứng
~~~

## Hướng dẫn và thí nghiệm

- [Sandbox quốc gia M3](guides/PLAYER-README-REALM.txt), [RL lab](guides/PLAYER-README-RL.txt), [prototype map](guides/PLAYER-README.txt).
- [Realm v6 protocol](experiments/realm-rl-experiment-v6.md) là thí nghiệm pilot M3; các protocol v1–v5 được giữ để truy nguyên.
- [RL data & Kaggle preparation](experiments/rl-data-kaggle-preparation.md) là kế hoạch dữ liệu/GPU cho M7; chưa có bằng chứng train.
- [Runtime evaluation v3](experiments/realm-runtime-evaluation-v3.md), [economic v1](experiments/rl-experiment-v1.md), [economic schema](experiments/rl-schema-v1.md).

## Source, bằng chứng và output

[Quy ước thư mục](developer/repository-layout.md) chỉ nơi đặt source/test/tài liệu mới. Checkpoint, config, source lock và evaluation nằm ở `artifacts/models/`; ảnh/profile nằm ở `artifacts/visuals/`, gồm [profile M4](../artifacts/visuals/m4/Windows-profile.json). Bundle ONNX app nạp nằm ở `godot_project/models/`.

Giữ nguyên manifest/checkpoint/source lock đã khóa: đường dẫn source trong record cũ thuộc revision thí nghiệm tương ứng. `build/`, `dist/`, `third_party/` và `.godot/` là output/dependency local được Git ignore.
