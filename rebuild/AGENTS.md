# Chỉ dẫn cho project PolyEc xây lại

Template này dành cho repository mới. Khi dùng làm AGENTS.md tại root, giữ bộ tài liệu ở docs/rebuild hoặc sửa đường dẫn ở đây theo vị trí thực. Không copy các lệnh tác vụ từ repository cũ.

- Đọc docs/CURRENT_TASK.md nếu đã có, sau đó docs/rebuild/IMPLEMENTATION_PLAN.md và tài liệu liên quan. Nếu chưa có CURRENT_TASK, tạo bản hiện tại cho nhóm A được giao; không tự đọc old archive để lấy next action.
- Trọng tâm kinh tế chính trị, đời sống mức cơ bản. Xây theo một lãnh thổ → hai lãnh thổ có tuyến giao thương → xung đột →27 vùng. Lộ trình chi tiết và điều kiện hoàn thành chỉ ở IMPLEMENTATION_PLAN.
- Dùng nhóm hành động chung với mục tiêu/tham số. Không quota18/400 action, file hoặc subsystem. Chỉ tách implementation khi semantics khác; không class rỗng hoặc switch toàn gameplay dồn trong World/manager.
- NPC là actor có danh tính, tài sản, cơ thể, kỹ năng thực hành, xu hướng, thông tin và mục tiêu riêng. Một learned policy chung; memory/sampling riêng, không family lock hoặc reset do đổi nghề. Hộ không có quỹ; tổ chức có tài khoản do NPC đại diện theo quyền.
- Godot chỉ nhận view đọc và gửi command. Python gọi cùng C++20 core. Không web game, luật Python thứ hai, auto-buy/eat/medicate hoặc strategy viết tay để giúp NPC qua kiểm.
- Kiểm khả thi khác pháp lý; hành vi vi phạm có phương tiện/chi phí/tài sản/sự việc và phản ứng. Không tự tạo consent, giả actor hoặc bỏ hạch toán vì nhãn illegal.
- Một bước mô phỏng 10 phút (fixture trial-a1); không kế thừa 64 tick/ngày. Hoạt động/mục tiêu/hành trình lưu được; pause/focus/sleep không chạy bù. Mục tiêu có thể đổi, cam kết có điều khoản; engine không tự chọn việc tiếp theo thay policy.
- Tiền/hàng có nguồn, chủ và đối ứng; nợ không là tiền mới. Command sai reject nguyên tử; nỗ lực hợp lệ thất bại có chi phí phải được đặc tả. Load staging và version rõ; không giả tương thích save/model cũ.
- Trước code lập plan ngắn: hành vi, file/ownership, đơn vị, success/reject/edge/gián đoạn, tác động save/API/model, kiểm ngay/kiểm sau và ranh giới đóng nhóm. Có hệ số thử phải ghi version/lý do/phép kiểm, không gọi là cân bằng đã chứng minh.
- Viết core → build/test target liên quan → chuỗi C++ command/giao dịch/thời gian/save → consumer ở mốc nhóm ổn định. Không ALL_BUILD/full suites/app sau từng sửa nhỏ. Không hoãn kiểm cần cho kết luận sắp báo.
- Code tổ chức đúng từ đầu; không chờ xong gameplay rồi mới sửa nền kiến trúc. Giữ một source writer chính; không mở agent/source writer cạnh tranh trên cùng checkout khi chưa được giao.
- CURRENT_TASK là trạng thái hiện tại, đọc lại rồi thay nội dung; một nhóm và một bước kế. Lịch sử vào HISTORY/report. Báo cáo tiếng Việt giải thích hành vi/kết quả/giới hạn, không thay bằng jargon hoặc số test.
- Training chỉ sau điều kiện trong IMPLEMENTATION_PLAN và có budget riêng. Dataset từ C++ core version hiện hành, seed train/validation/holdout tách, số liệu nguồn và biểu đồ từng run. Giải thích cách tạo dữ liệu; không dùng reward rollout thay đánh giá độc lập.
- Không tự mua cloud/GPU, thay consent/quyền/phạm vi đã chốt, bỏ test hoặc tắt TLS/dependency checks. Không tự commit/push/publish khi chưa được giao; giữ dirty work người dùng.
- Khi quy tắc/tài liệu thiếu: phân biệt yêu cầu đã chốt với đề xuất tại DECISIONS. Tự xử lý kỹ thuật trong phạm vi; không dựng cơ chế chuyên sâu để lấp câu hỏi thiết kế chưa giải đáp.


## Nơi làm việc hiện hành
Đây là source canonical C:\prc\PolyEc\rebuild, thuộc Git checkout C:\prc\PolyEc với origin cũ. Quyền tiếp tục source tại đây đến từ yêu cầu mới của người dùng; không force-push/xóa lịch sử. Luật hiện tại trial-b1, save3; xem TRAVEL_B.md và CLASS_ACTION_MAP.md. Người dùng đã cho phép commit/push theo workflow hiện hành. Snapshot Documents không ghi source nữa.
