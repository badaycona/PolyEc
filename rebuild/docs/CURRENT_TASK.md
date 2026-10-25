# Nhóm hiện hành — đưa rewrite lên repo chính

Người dùng đã yêu cầu đưa code về workspace PolyEc, overwrite nội dung GitHub trước, rồi triển khai mốc B hai lãnh thổ. Quyền commit/push hiện đã có; không cần force-push/xóa lịch sử. Source canonical ở C:\prc\PolyEc\rebuild, root build chỉ rewrite.

Bước kế: tạo tree publish chỉ gồm rewrite/entrypoints/CI mới, kiểm baseline rồi commit và push main thông thường. Legacy local được giữ trên đĩa do xóa trước bị chặn; không có trong tree publish, không dùng index tạm để xóa file trên máy. Giữ worktree ngoài.

Sau khi xác nhận main remote đúng commit mới, thay CURRENT_TASK sang nhóm B với luật/version, class/action, kiểm core rồi Godot. Không RL/chiến tranh trong lần này.
