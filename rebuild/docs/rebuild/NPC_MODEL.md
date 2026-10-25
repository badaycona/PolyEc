# NPC: dữ liệu, năng lực, xu hướng và mục tiêu

Tài liệu hỗ trợ [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md). Các cơ chế dưới đây là định hướng cho lần xây mới, chưa phải tính năng đã code. Con số thang đo dùng để mô tả rõ dữ liệu; hệ số thử phải ghi trong cấu hình trước triển khai.

## 1. Một người có một danh tính

Mỗi NPC có ID bền, còn sống/đã chết, vị trí thực, tài sản/tài khoản, quan hệ, cơ thể, kinh nghiệm, xu hướng, thông tin được biết, mục tiêu và hoạt động đang thực hiện. Nghề/chức vụ/thành viên tổ chức là quan hệ có thể đổi; không tạo lại người, xóa bộ nhớ hoặc đổi bộ model vì đổi chức.

Nhân vật giữ chức có thể ra lệnh với tư cách cá nhân hoặc đại diện tổ chức. Tư cách phải được ghi trong lệnh; tài sản công không biến thành tiền riêng. Quan hệ hộ không tạo tài khoản dùng chung hoặc quyền tự chi tiền thành viên.

## 2. Các chỉ số cơ thể và đời sống

Đề xuất ban đầu dùng thang0–100, chỉ một dữ liệu chuẩn cho mỗi ý nghĩa. Không lưu đồng thời đói/no/dinh dưỡng như ba bộ đếm trùng nhau nếu chưa có mô hình khác biệt thật.

| Dữ liệu | Ý nghĩa và chiều tốt/xấu | Nguyên nhân thay đổi | Tác động chính cần triển khai |
|---|---|---|---|
| Đói | Cao là thiếu ăn | Thời gian/công sức làm tăng; ăn làm giảm | Thiếu ăn kéo dài làm giảm sức khỏe/khả năng hoạt động; không auto-buy/eat |
| Khát | Cao là thiếu nước | Thời gian/hoạt động; uống từ nguồn có thật | Nhu cầu tiêu dùng cơ bản; không thêm mô hình điện giải hoặc nước trong cơ thể |
| Mệt | Cao là cần nghỉ | Làm việc/di chuyển làm tăng; nghỉ hợp lệ làm giảm | Giảm khả năng duy trì công việc; không nghỉ ở nơi khác khi đang làm việc tại xưởng |
| Sức khỏe | Cao là tốt | Thiếu nhu cầu kéo dài, chấn thương, hồi phục/điều trị đã chọn | Ảnh hưởng khả năng hoạt động và vòng đời; không mô phỏng y khoa chuyên sâu |
| Vui vẻ | Cao là dễ chịu | Bữa ngon, giao tiếp, giải trí, kết quả tốt/xấu có nguyên nhân | Là giá trị NPC có thể coi trọng; có thể ảnh hưởng nhẹ hiệu suất việc cần nỗ lực |

Khát nằm trong nhu cầu cơ bản đã bàn. Trước khi code phải khai báo nguồn nước và cách tiếp cận, chi phí/công sức và lượng tiêu thụ; không thêm một chỉ số chỉ giảm vô hạn mà không có cách đáp ứng. Nước dễ tiếp cận cũng phải có nguồn giải thích được, không là lệnh thành công giả.

Mỗi hoạt động khai báo chỉ số nào ảnh hưởng nó và ở bước nào. Tránh đồng thời giảm khả năng chú ý và nhân thêm cùng một penalty mệt vào sản lượng nếu đó là tính hai lần cùng một tác động. Cùng khoảng hoạt động, chia thành các bước tick khác nhau không được tạo thêm nghỉ/hàng/kinh nghiệm.

### Vui vẻ và hiệu suất — điểm còn cần thử

Người dùng muốn vui vẻ có ảnh hưởng hiệu quả hoạt động và giao agent đề xuất. Hướng ban đầu: tác động nhẹ, có giới hạn, lên công việc cần công sức/tập trung. Không nhân mọi đại lượng kinh tế; vui vẻ không tự tăng giá bán, tiền lãi, quyền hạn hoặc consent người khác.

Ví dụ để thảo luận là biên±5% quanh mức vui vẻ trung tính; đây chưa là hệ số đã duyệt hoặc đã chứng minh. Thử đối chứng cùng kỹ năng/công cụ/cơ thể, chỉ đổi vui vẻ. Nếu một bữa ngon trở thành yêu cầu bắt buộc để mọi việc có lãi, giảm hoặc sửa cơ chế thay vì ép NPC giải trí. Không thêm satisfaction riêng cho từng món và một thanh vui vẻ chung cùng phản ánh một trải nghiệm.

## 3. Kỹ năng là hiệu quả qua thực hành

Đã chốt về hướng: làm nhiều thì quen tay; giao tiếp nhiều thì lưu loát. Dùng một cơ chế kinh nghiệm theo số nhóm hoạt động có thật, không kỹ năng cho từng mục tiêu/nút bấm.

Luồng đề xuất: công/thời gian tham gia thật → kinh nghiệm nhóm liên quan → hiệu quả tăng với mức lợi tăng chậm dần. Không tính đủ kinh nghiệm từ lệnh bắt đầu bị hủy, phiên không có mặt, làm không có nguyên liệu hoặc nói chuyện không có người tham gia.

| Nhóm ví dụ | Thực hành được tính | Tác dụng cơ bản | Điều không tự xảy ra |
|---|---|---|---|
| Sản xuất theo nhóm nghề | Phần công thực hiện trong mẻ đủ điều kiện | Hoàn thành cùng lượng công trong ít tick hơn | Không sinh nguyên liệu, thêm đầu ra hoặc tự tăng tiền công |
| Giao tiếp | Thời gian trao đổi thực có các bên tham gia | Truyền cùng lượng nội dung trong ít thời gian chú ý hơn | Không tự tăng sự thật, thiện cảm hay xác suất chấp thuận |

Không dùng một kỹ năng chung làm người làm ruộng tự giỏi rèn. Chỉ tạo nhóm nghề khi có hoạt động cần phân biệt. Chưa mặc định thêm trí tuệ/hùng biện/lãnh đạo với hàng loạt tác dụng ngầm.

Ví dụ minh họa: cùng một mẻ cần32 tick công ở người mới và24 ở người quen việc. Đây là ví dụ hành vi, không khóa số32/24 làm cân bằng. Khi triển khai cần định rõ đơn vị lượng công, hiệu quả, cách tích kinh nghiệm, giới hạn, làm tròn và phần dư để chia nhỏ thời gian không làm tăng lợi ích.

Kỹ năng ảnh hưởng kết quả thực; tiền công vẫn theo thỏa thuận. Người giỏi tìm được việc tốt hơn hay được tin tưởng hơn là kết quả lựa chọn/thông tin của các bên, không tự động cộng tiền hoặc favor.

## 4. Xu hướng, nhu cầu và mục tiêu khác nhau

- Xu hướng: mức coi trọng một loại kết quả tương đối bền, ví dụ an toàn tài chính hoặc hưởng thụ.
- Nhu cầu: hoàn cảnh thay đổi theo thời gian, ví dụ đang đói, mệt hoặc thiếu tiền trả nợ.
- Mục tiêu: điều NPC hiện theo đuổi, ví dụ bán hàng ở vùng khác hoặc dành tiền mua công cụ.

Một người thích hưởng thụ vẫn có thể tiết kiệm khi mắc nợ. Một người vừa thích đi xa vừa tiết kiệm có thể ăn đơn giản và đi tuyến rẻ. Không gắn xu hướng vào một nhãn nghề.

Đề xuất ban đầu gồm ưu tiên an toàn tài chính, hưởng thụ và dịch chuyển; chúng không loại trừ nhau và không bắt buộc cộng thành1. Danh sách/thang đo/phân bố chính thức phải ghi trong plan trước code, không mang sáu tính cách cũ sang chỉ để giữ vector.

Xu hướng phải xuất hiện trong đầu vào của shared policy và cách đánh giá kết quả huấn luyện. Chỉ thêm vào observation mà reward không phân biệt hoặc policy không dùng chưa chứng minh tạo khác biệt. Không cần một bộ loss/model riêng cho mỗi người. Nội dung dự thảo reward/utility ở [DEVELOPMENT_AND_RL](DEVELOPMENT_AND_RL.md).

Thay đổi xu hướng theo tuổi/biến cố là tùy chọn sau; người dùng chấp nhận chưa có ở bản cơ bản. Nhu cầu, thông tin và mục tiêu vẫn thay đổi, nên hành vi không cố định ngay cả khi xu hướng giữ nguyên.

## 5. Mục tiêu và conviction

Người dùng đã nêu có người kiên định, có người không giữ được conviction. Chưa chốt cần một chỉ số conviction độc lập. Không suy ra luật threshold buộc NPC tiếp tục hoặc từ bỏ.

Đề xuất bản đầu có một mục tiêu chính đang theo đuổi và các cam kết/nghĩa vụ riêng. Hồ sơ mục tiêu ghi loại, đối tượng/đích, điều kiện hoàn thành, lúc bắt đầu và kết quả/cancel. NPC được tiếp tục, điều chỉnh hoặc bỏ mục tiêu khi có cơ hội quyết định. Việc theo đuổi lâu dài phải lưu qua nghỉ/ăn/hành trình/save-load, không yêu cầu một world planner tự chọn chiến lược.

Lợi ích kỳ vọng, nhu cầu, thông tin và xu hướng giúp policy quyết định có tiếp tục hay không. Chỉ tính chi phí/hậu quả đổi ý có thật: phí đã trả, hàng đang giữ, hợp đồng chưa hoàn thành hoặc quan hệ bị ảnh hưởng do bên kia biết. Không thưởng vô hạn vì giữ một mục tiêu, phạt mọi đổi ý hoặc tự tạo tâm lý tiếc tiền đã bỏ. Chỉ thêm tham số gắn bó với mục tiêu nếu thiết kế và đối chứng chứng minh nó cần thiết.

Mục tiêu cá nhân khác cam kết với người khác. Bỏ ý định tự đi buôn không tự thành tội; bỏ giao hàng đã nhận phải xét điều khoản, hàng/tài khoản và thông tin các bên.

## 6. Ví dụ mục tiêu dài hạn: chuyến buôn

NPC muốn bán hàng ở vùng C qua vùng B. Người đó chọn lấy hàng, đi tuyến, nghỉ trên đường, ở trọ khi tới lãnh địa, hỏi tin và bán hàng. Engine tiến đoạn đi/nghỉ/chuẩn bị đã chọn; không tự chọn điểm nghỉ, mua thức ăn hoặc ở trọ thay NPC.

Khi đường đóng hoặc nhận tin giá thay đổi, NPC có thể tiếp tục, đổi tuyến/đích hoặc bỏ chuyến. Cơ hội quyết định xuất hiện theo quy tắc tick/sự kiện/gián đoạn, không đợi tới ngày quyết định theo family. Không bắt mạng suy luận lại mỗi bước chân khi chỉ tiếp tục hoạt động.

Đây là nghĩa của quyết định dài hạn: một mục tiêu còn tồn tại qua nhiều hoạt động; không có thời hạn cố định chung cho mọi mục tiêu. Hạn hợp đồng cụ thể vẫn phải có nếu các bên đã thỏa thuận.

## 7. Thông tin và quan hệ

Đã chốt nguồn thông tin: trải nghiệm trực tiếp; trò chuyện/trao đổi; thông báo công; học hành. Hồ sơ tối thiểu: ai biết, nội dung và đối tượng, nguồn, lúc sự việc và lúc nhận tin. Một giá báo hôm qua không tự thành giá thật hôm nay. Học không tự mở toàn bộ sự thật của thế giới.

Quan hệ thiện cảm có hướng: A quý B khác B quý A. Thay đổi dựa vào tương tác/sự việc mà người đánh giá biết; không đọc trộm ý định. Không tự accept lời nhờ vì điểm cao. Tin tức chỉ truyền cho người đã tiếp cận nguồn, không broadcast mọi event.

Bộ nhớ học của policy không thay dữ liệu giao dịch, mục tiêu, quyền hoặc hồ sơ tin cần chính xác. Dữ liệu đó phải lưu tường minh trong core. Giao diện người chơi đọc thông tin vua được biết; debug/evaluator có kênh toàn tri tách riêng.

## 8. Phép kiểm cần có

Giữ cùng hoàn cảnh, chỉ đổi kỹ năng để đo thời gian/lượng công; đổi vui vẻ để đo hiệu suất; đổi nguồn tin để đo observation. Kiểm thực hành/hủy/thiếu vật tư không farm kinh nghiệm. Chọn một mục tiêu, xen ăn/nghỉ/di chuyển, save/load rồi tiếp tục hoặc tự đổi mục tiêu.

Sau RL mới so các nhóm xu hướng trên nhiều seed ghép cặp: chi tiêu/tích lũy, hưởng thụ và mức di chuyển; kiểm nhu cầu sống và nợ cùng lúc. Không kết luận NPC có tính cách hoặc conviction chỉ bằng việc lưu một field hoặc một ví dụ hành động.
