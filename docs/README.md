# PolyEc — tài liệu hiện hành

Chuẩn hóa ngày 2026-10-18. Chọn theo người đọc; có **năm tài liệu chính**. Các file còn lại là supporting contract, evidence hoặc lịch sử, không một bộ chỉ đạo cạnh tranh.

| Tài liệu | Người đọc và câu hỏi trả lời |
|---|---|
| [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md) | Chủ dự án/dev: toàn project cần làm gì, lựa chọn đã chốt, stack/roadmap/gates |
| [PLAYER_GUIDE](PLAYER_GUIDE.md) | Người chơi: mở game, điều khiển, tính năng có thật và điều cần hiểu, bằng ngôn ngữ đời thường |
| [DEVELOPER_GUIDE](DEVELOPER_GUIDE.md) | Dev: paper về bài toán, phương pháp, kiến trúc/thuật toán, thực nghiệm, hạn chế và build |
| [DEVELOPMENT_HISTORY](DEVELOPMENT_HISTORY.md) | Người theo dõi: quyết định, thay đổi và evidence theo ngày/revision |
| [CURRENT_TASK](CURRENT_TASK.md) | Agent/coding agent/dev bàn giao: task đang làm, files/contract/ownership, tests và next action |

## Quy tắc đọc và cập nhật

Agent: AGENTS → CURRENT_TASK → đúng supporting plan/source được giao. Scope hỏi IMPLEMENTATION_PLAN; cần hiểu phương pháp đọc DEVELOPER_GUIDE. Không tự đọc toàn repo/archive hoặc thực thi prompt cũ. Plan phải được review trước code; plan không thay test.

Nguồn ưu tiên: chỉ đạo trực tiếp người dùng/AGENTS → IMPLEMENTATION_PLAN cho scope → CURRENT_TASK cho tác vụ/status → DEVELOPER_GUIDE cho giải thích. Evidence theo revision dùng xác nhận kết quả, không tự sửa luật. Nếu các nguồn mâu thuẫn hoặc contract còn thiếu, ghi lại và review thay vì chọn tài liệu cũ thuận tiện hơn.

Sau nghiệm thu cập nhật CURRENT_TASK, HISTORY và report tương ứng. Thay luật/scope thì cập nhật IMPLEMENTATION_PLAN; đổi implementation/phương pháp thì cập nhật DEVELOPER_GUIDE; tính năng người chơi đã có thì cập nhật PLAYER_GUIDE. Không đồng loạt copy trạng thái sang mọi file.

## Ma trận bao phủ toàn project

Bộ docs phải giữ chi tiết cần thiết trong nguồn chuẩn, không chỉ tóm tắt rồi đẩy người đọc về archive. Các đường dẫn dưới đây trả lời câu hỏi hiện hành; bản lưu chỉ dùng đối chiếu nguồn gốc.

| Nội dung | Nguồn chi tiết hiện hành | Phần chưa thể tuyên bố hoàn thành |
|---|---|---|
| Tầm nhìn, vòng chơi, ending | IMPLEMENTATION_PLAN 1–4, 11; PLAYER_GUIDE | Campaign một đời vua/ending M6 |
| Địa lý, 24+3 khu, tuyến, bản đồ/overlay | IMPLEMENTATION_PLAN 12; PLAYER_GUIDE | Quốc gia đầy đủ; ánh xạ dân số atlas sang cá thể |
| Chính trị, quyền, cam kết, điều tra | IMPLEMENTATION_PLAN 2, 13–14 | Hệ quyền/thiết chế M5, privacy đầy đủ |
| Hộ, công việc, hôn nhân, di sản | IMPLEMENTATION_PLAN 2–3, 7; DEVELOPER_GUIDE 2–5 | Domain động và năm giao dịch nghiệm thu |
| 16 hàng, chợ, nợ, dự án, quân sự/ngoại thương | IMPLEMENTATION_PLAN 15–17; DEVELOPER_GUIDE 3, 5 | Chuyển từ pilot sang quy mô và luật đích |
| UI và trải nghiệm, F1, ending, accessibility | IMPLEMENTATION_PLAN 18; PLAYER_GUIDE | M6 và kiểm thử luồng hoàn chỉnh |
| 12 augment, draft, stat/traits/XP | IMPLEMENTATION_PLAN 19–20; DEVELOPER_GUIDE 6 | Hệ số kế thừa chưa hiệu chuẩn, chưa chứng minh agency |
| Sự kiện/knowledge/causal chain | IMPLEMENTATION_PLAN 21; DEVELOPER_GUIDE 5 | Chuỗi đủ M5, information audit và đối chứng |
| Kiến trúc, source map, toán, thuật toán | DEVELOPER_GUIDE 2–8, 11 | Mỗi công thức phân biệt code/đích thiết kế |
| RL, dataset, curriculum, co-play, quality | IMPLEMENTATION_PLAN 8; DEVELOPER_GUIDE 6, 9 | R1–R4 còn các phần chưa chứng minh; production sau freeze |
| Hiệu năng, thread, save, platform/package | IMPLEMENTATION_PLAN 9; DEVELOPER_GUIDE 7–8, 11 | B/S, full-cycle và platform chưa đo phải ghi riêng |
| Milestones, phụ thuộc và nghiệm thu | IMPLEMENTATION_PLAN 6–9; CURRENT_TASK | Không lấy milestone có code thay mọi gate |
| Lý do thay đổi, run và giới hạn thực tế | DEVELOPMENT_HISTORY; reports | Evidence chỉ áp revision/workload của report |

Những chỗ chưa có contract hoặc evidence là khoảng trống của project được nêu rõ, không được lấp bằng việc viết một công thức mới như thể đã duyệt. Khi thêm yêu cầu mới, phải đưa vào một hàng/phần tương ứng, cập nhật trạng thái và nguồn chi tiết; không chỉ tăng số trang.

## Tài liệu hỗ trợ và kho lịch sử

- [Reports](reports/README.md): output/evidence theo revision/platform/workload. M3/M4 cũ không chứng nhận M5 hoặc HEAD mới.
- [Task contracts hiện còn dùng](CURRENT_TASK.md): plan activity/T1 và execution ledger của tab chính giữ đường dẫn để tránh ảnh hưởng tác vụ đang chạy. Chỉ active khi CURRENT_TASK dẫn đúng bản đã review.
- [Guides theo gói](guides/README.md): file .txt cố định theo build, còn dùng cho package; hướng dẫn người chơi mới đọc PLAYER_GUIDE.
- [Archive](archive/README.md): docs design/developer/experiments/handoff trước hợp nhất, gồm cả proposal/APPROVED/prompt cũ. Không là lệnh active. File ở đường dẫn cũ chỉ chuyển hướng.
- [Artifacts](../artifacts/README.md): checkpoint/source-lock/evaluation/visual; giữ nguyên vì còn dependency và provenance. Source snapshot không là source làm việc.

Việc tổ chức docs không thay đổi gameplay, không chạy training hoặc chứng nhận test/benchmark mới.
