# Tài liệu PolyEc

**Thiết kế mô phỏng đã duyệt 2026-10-17:** [Quyết định và nguồn tham khảo](design/M5-simulation-design-decisions.md), [bảng phần cần giữ/sửa/build mới](handoff/M5-s[...]

[WORK_STATUS.md](../WORK_STATUS.md) ghi trạng thái mã và gate còn mở. [Implementation plan](design/implementation-plan.md) mô tả đích sản phẩm. M4 hiện được kiểm chứng Wi[...]

## Tệp refactor M5

Đã thêm: docs/developer/M5-refactor-notes.md — ghi lại các thay đổi refactor ban đầu (WorldState, manager helpers, snapshot/restore) để chuẩn bị cho M5.

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
| Refactor M5 notes | [M5 refactor notes](developer/M5-refactor-notes.md) |

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
