# Điều đã chốt, mặc định đề xuất và câu hỏi còn mở

Ngày2026-10-25. Tài liệu này giải thích nguồn quyết định để người code không biến lời brainstorm thành luật đã được người dùng duyệt. Scope và thứ tự triển khai nằm tại [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md).

## 1. Người dùng đã quyết định

- Dừng project hiện tại; lấy tài liệu còn dùng được và chỉnh để định hướng code lại. Lần giao này là tài liệu, không tiếp tục code cũ hoặc tự mở repo/code mới.
- Sau khi biên tập bộ định hướng, người dùng yêu cầu xóa mọi file không liên quan/không cần thiết, chỉ giữ docs, rồi xác nhận xóa cả Git. Yêu cầu này thay cho việc bảo toàn source/build/evidence/Git ở lượt trước; bộ docs phải tự đọc được. Các lệnh xóa đang bị công cụ chặn, chưa thực hiện được.
- Người dùng yêu cầu mở một session mới hoàn toàn, bắt đầu code lại từ luật môi trường, mô hình class/object, rồi bản game trực quan một lãnh thổ. Session mới không kế thừa source hoặc hội thoại cũ.
- Trọng tâm là kinh tế chính trị. Sinh hoạt và các nhóm hành động ở mức cơ bản; không làm mỗi hệ thành một trò chơi chuyên ngành.
- Không cần vô số action. Dùng nhóm chung tính chất; mục tiêu/giá/lượng/điều khoản là tham số.
- Bữa ăn khác nhau về rẻ/đắt, nhanh/lâu, ngon/dở và vui vẻ/hài lòng; không cần tên món. Dừng những chi tiết ăn từng món phụ chệch hướng.
- Kỹ năng có thể đơn giản là làm nhiều thì hiệu quả hơn; nói nhiều thì lưu loát hơn. Cần mô tả ảnh hưởng thật, không chỉ tên stat.
- NPC biết qua trải nghiệm trực tiếp, trò chuyện/trao đổi thông tin, thông báo công và học hành.
- NPC khác xu hướng: có người tiết kiệm, chịu thiếu tiện nghi để dành dụm; có người thích đi đây đó. Không phải ai cũng giữ conviction. Xu hướng thay đổi theo biến cố/thời gian là tốt nhưng có thể chưa có.
- Mục tiêu dài hạn phải có chỗ cho chuyến buôn qua nhiều lãnh thổ, nghỉ/đóng trại dọc đường và ở trọ nơi đến; không quy một quyết định dài hạn thành số ngày cố định.
- Hướng xây lại được thảo luận và dùng làm định hướng: một lãnh thổ tương tác kinh tế chính trị → hai lãnh thổ có tuyến giao thương → xung đột cơ bản →27. Đây là cách chứng minh từng quy mô, không tự giảm đích sản phẩm.

Những nguyên tắc còn giữ từ yêu cầu trước: app native C++/Godot/Python cùng core; một learned policy chung; danh tính/memory riêng; NPC đại diện tổ chức; không quỹ hộ; không auto-spend/eat/medicate; khả thi khác hợp pháp; thông tin riêng; tiền/hàng có nguồn và quyền; pause không chạy bù; kiểm theo phạm vi; RL có budget/split/biểu đồ. Source cũ chưa được xem là đã đáp ứng chúng.

## 2. Agent đề xuất làm mặc định ban đầu, chưa phải cân bằng đã duyệt

| Chủ đề | Đề xuất trong bộ docs | Khi nào cần xác định cụ thể |
|---|---|---|
| Bộ chỉ số | Đói/khát/mệt/sức khỏe/vui vẻ, thang0–100; không duplicate satiety/nutrition | Plan mô hình cơ thể mốc A: đơn vị/tốc độ/biên và cách đáp ứng |
| Vui vẻ → hiệu suất | Tác động nhỏ có giới hạn lên việc cần nỗ lực; ví dụ±5% chỉ để thử | Trước code hệ số và đối chứng; người dùng giao nghĩ giúp nhưng chưa khóa con số |
| Kỹ năng | Kinh nghiệm theo nhóm nghề/hoạt động, lợi tăng chậm; một tác dụng chính | Trước production/communication code: đơn vị thực hành, cap/rounding |
| Xu hướng | An toàn tài chính, hưởng thụ, dịch chuyển; có thể đồng thời cao | Trước scenario/training: danh sách, phân bố và utility |
| Mục tiêu | Một mục tiêu chính đang theo đuổi, nhiều nghĩa vụ riêng | Trước chuyến đi/lời nhờ: hoàn thành, hủy và đổi ý |
| Conviction | Chưa thêm stat riêng; bắt đầu bằng mục tiêu được nhớ và hành vi tiếp tục/đổi | Chỉ thêm khi tình huống và đối chứng cho thấy cần |
| Đa nhiệm | Một hoạt động chính chiếm công sức, các phản ứng ngắn có quy tắc | Trước activity scheduler; chưa cần bộ tối ưu lịch phức tạp |
| Model/reward | Policy/value thấy ngữ cảnh xu hướng và mục tiêu; reward từ kết quả thật | Trước training có protocol và budget |

Các giá trị này được phép hoàn thiện trong plan có version và kiểm thử theo scope, không tự tuyên bố là yêu cầu nguyên văn. Thay quyền/consent/phạm vi hoặc ngân sách phải có quyết định riêng.

## 3. Câu hỏi cần xử lý trước phần code tương ứng

1. **Địa điểm code mới và mức tương thích:** thư mục/repository chưa được chọn. Không có yêu cầu nạp save/model cũ; nếu cần importer, tách tác vụ riêng có mapping quyền tài sản.
2. **Tài chính cá nhân/mục tiêu:** khoản dự trữ để đáp ứng bao lâu, nghĩa vụ nào và đầu tư nào; có giá trị sở hữu tiền tự thân không? Chưa chốt. Không thưởng tiền vô hạn trước khi có câu trả lời thiết kế.
3. **Cơ thể/vui vẻ:** các tốc độ đói/khát/mệt, hồi phục và hệ số hiệu suất; tránh nhiều penalty tính trùng. Chốt trong cấu hình thử và kiểm tay, không mở simulator sinh lý.
4. **Nguồn lực/kinh tế mốc A:** nhóm hàng/nghề tối thiểu, nguồn tự nhiên, tài sản/tiền mở đầu, giá/cách khớp giao dịch, thuế một loại giao dịch và chi công nào. Các thiết lập cũ không tự được giữ.
5. **Quyền/chính trị/xung đột:** phạm vi chức, cam kết, quân sự/consent, hậu quả vi phạm và ranh giới cưỡng chế. Phải định nghĩa trước khi code cách thực hiện đó, không hỏi lại mọi chi tiết kỹ thuật đã nằm trong luật.
6. **Thời gian/nhịp chơi:** chưa chốt độ chia tick trong ngày, tốc độ ngày theo thời gian thực hoặc độ dài chiến dịch; quyết định lại theo hoạt động và vòng chơi của bản mới. Không mang 64 tick/ngày, tuần/tháng theo family hoặc nhịp cũ sang làm mặc định.
7. **Ngân sách quy mô và RL:** số NPC thật trong bản27 vùng, RAM/latency mục tiêu, ngân sách train và tiêu chí đủ chất lượng. Không suy ra từ dân số bối cảnh Atlas hoặc các run cũ.

Các câu hỏi không chặn việc bàn giao docs. Khi được giao code, giải quyết đúng phần cần cho nhóm đang làm; không tự mở toàn bộ backlog hoặc bắt người dùng trả lời mọi mục cùng lúc.

## 4. Những đề xuất cũ bị thay thế

Không tiếp tục quota400 action/file, ba family policy, quỹ hộ fallback, cadence7/30 ngày, auto-consumer, thao tác Salt/Pottery riêng sau bữa, Pottery cộng nutrition, schema/tensor/save compatibility cũ, hoặc thứ tự hoàn thành gameplay trên kiến trúc cũ rồi mới sửa nền.

Theo yêu cầu dọn mới nhất, các báo cáo/code/model cũ phải được loại bỏ; thao tác này chưa hoàn thành vì công cụ chặn xóa. Kiến thức còn dùng được đã biên tập vào bộ docs này; không dùng số pass/checkpoint lịch sử để chứng nhận lần xây mới, và không để việc dọn cũ bị chặn buộc code mới tiếp tục dùng nền cũ.
