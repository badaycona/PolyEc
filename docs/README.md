# PolyEc — Tài liệu hiện hành (index, rút gọn)

Cập nhật: 2026-10-18 (đồng bộ và rút gọn). Đây là điểm vào tài liệu chính.

Bộ docs tối thiểu hiện giữ những nguồn sau (nguồn chính, required):

- IMPLEMENTATION_PLAN.md — scope, roadmap, gates và contract sản phẩm (đã cập nhật theo thiết kế agent-centric).
- DEVELOPER_GUIDE.md — paper kỹ thuật, công thức, API, tests và build (đã hợp nhất các công thức quan trọng từ archive).
- PLAYER_GUIDE.md — hướng dẫn người chơi, UI và tương tác (giữ nguyên, cập nhật từ archive nếu cần).
- DEVELOPMENT_HISTORY.md — lịch sử phát triển rút gọn: quyết định, bug chính và cách khắc phục.
- CURRENT_TASK.md — task đang làm, tests để chạy, files/ownership.
- artifacts/README.md — giữ như hướng dẫn artifacts (không phải docs điều hành).

Mọi nội dung khác trong docs/archive/ đã được di chuyển vào `docs/archive_snapshots/2026-10-18/` để giữ lịch sử. Trước khi di chuyển, các file có thông tin quan trọng (ledger/accounting, scaling/complexity, simulation design decisions) đã được trích và hợp nhất vào DEVELOPER_GUIDE.md hoặc IMPLEMENTATION_PLAN.md.

Nếu bạn muốn truy nguyên lại nội dung cũ, xem `docs/archive_snapshots/2026-10-18/`.

---

Hướng dẫn đọc nhanh:
- Người phát triển mới: đọc IMPLEMENTATION_PLAN → DEVELOPER_GUIDE → CURRENT_TASK.
- Người chơi: PLAYER_GUIDE → QUICKSTART trong godot_project.
- Reviewer/agent: check CURRENT_TASK và DEVELOPMENT_HISTORY để biết những thay đổi gần đây và evidence trong reports/.

---

Lưu ý: các thay đổi này được thực hiện tự động theo yêu cầu đóng gói tài liệu; nếu bạn muốn giữ một file archive ở vị trí cũ, thông báo để rollback.