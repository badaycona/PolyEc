# PolyEc — checkout chính của rewrite

Code đang hoạt động duy nhất ở rebuild/. Đọc rebuild/AGENTS.md và rebuild/docs/CURRENT_TASK.md. Root CMake chỉ build rewrite; roadmap nằm ở rebuild/docs/rebuild/IMPLEMENTATION_PLAN.md.

Yêu cầu hiện hành cho phép commit/push để thay nội dung repo https://github.com/badaycona/PolyEc.git bằng rewrite rồi triển khai hai lãnh thổ. Giữ lịch sử, push thường; không force-push. Không làm gì với worktree ngoài.

Legacy local chưa xóa do thao tác trước bị chặn. Không retry recursive delete/move hoặc né chặn; chỉ tree source mới được publish. Không dùng source/build/model cũ; không đọc archive để tiếp tục M5/R1–R4. Không sửa source tại snapshot Documents.
