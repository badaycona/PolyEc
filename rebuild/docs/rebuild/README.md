# Định hướng xây lại PolyEc

Ngày 2026-10-25, người dùng quyết định bỏ project cũ, lấy phần tài liệu còn hữu ích để định hướng code lại, rồi xóa các file không cần thiết để chỉ giữ docs. Bộ này là thiết kế cho lần xây mới, chưa phải mô tả một bản game đã được triển khai; không phụ thuộc source, save, model hoặc báo cáo cũ.

## Đọc theo thứ tự

1. [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md): mục tiêu kinh tế chính trị, phạm vi, thứ tự một lãnh thổ → hai lãnh thổ → xung đột → 27 lãnh thổ và điều kiện hoàn thành.
2. [NPC_MODEL](NPC_MODEL.md): cơ thể, kỹ năng từ thực hành, xu hướng, mục tiêu có thể thay đổi, vui vẻ và thông tin riêng.
3. [WORLD_AND_ACTIONS](WORLD_AND_ACTIONS.md): nhóm hành động có tham số, tiền/hàng/quyền, bữa ăn, công việc, nhà ở, giao thương và chính trị.
4. [ARCHITECTURE](ARCHITECTURE.md): C++ core dùng chung, giao dịch, hoạt động theo tick, save/load và ranh giới API/UI/AI.
5. [DEVELOPMENT_AND_RL](DEVELOPMENT_AND_RL.md): cách xây, kiểm theo phạm vi, giải thích dữ liệu RL và đánh giá.
6. [SOURCE_REVIEW](SOURCE_REVIEW.md): tài liệu cũ nào được giữ ý tưởng, sửa hoặc loại; cách dùng dữ liệu Atlas.
7. [DECISIONS](DECISIONS.md): điều người dùng đã chốt, giả định kỹ thuật có thể thử và câu hỏi còn mở.
8. [WORLD_ATLAS_REFERENCE](WORLD_ATLAS_REFERENCE.md): bối cảnh 24 lãnh địa và 3 khu triều đình cho bước mở rộng sau.

[AGENTS_TEMPLATE](AGENTS_TEMPLATE.md) là chỉ dẫn có thể mang sang project mới. [CURRENT_TASK của checkout hiện tại](../CURRENT_TASK.md) chỉ ghi việc chuẩn bị bàn giao tài liệu. Không tiếp tục M5 hoặc core refactor cũ từ một đường dẫn lịch sử.

## Cách sử dụng bộ này

- Copy nguyên thư mục này sang project mới để các liên kết nội bộ vẫn chạy. Có thể đặt nội dung AGENTS_TEMPLATE tại AGENTS.md của project mới và sửa đường dẫn một lần theo vị trí thực tế.
- File IMPLEMENTATION_PLAN duy nhất sở hữu scope, roadmap và tech stack. Các file còn lại giải thích chi tiết; không tự đặt một roadmap thứ hai.
- Những mục ghi **đã chốt** xuất phát từ yêu cầu người dùng. Những mục ghi **đề xuất ban đầu** là phương án để lập kế hoạch, không phải code đã có hoặc cân bằng đã được chứng minh.
- Bắt đầu với một vòng kinh tế chính trị nhỏ hoàn chỉnh. Không copy nguyên engine cũ, danh sách Intent, kích thước model, luật family hoặc hàng chục ngành sản xuất vào nền mới.
- SOURCE_REVIEW ghi kiến thức đã chọn lọc và phần đã loại khỏi nền mới. Không cần khôi phục file cũ để sử dụng bộ này; kết quả thử nghiệm cũ không chứng nhận luật hoặc source mới.

## Tình trạng bàn giao

Người dùng đã giao mở session mới hoàn toàn để viết lại từ luật môi trường, class/object và bản game trực quan một lãnh thổ. Chỉ mang bộ docs này vào workspace sạch; chưa có kết quả triển khai hoặc huấn luyện mới. Tình trạng xóa project cũ bị công cụ chặn được ghi ở CURRENT_TASK, không là lý do dùng tiếp source cũ.
