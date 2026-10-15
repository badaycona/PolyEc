# PolyEc — ranh giới độ phức tạp simulation

Ngày ghi nhận: 2026-10-15. **Đề xuất thiết kế để tiếp tục thảo luận; chưa phải toàn bộ phạm vi đã được duyệt hoặc đã triển khai.**

## 1. Ngữ cảnh và trạng thái quyết định

Người dùng muốn làm rõ **độ sâu của mô phỏng** trước khi tiếp tục thực hiện: nghề nghiệp và giai cấp, nhiều entity, liên hôn, quan hệ xã hội và thế giới có tính chân thật. Người dùng đã chọn bối cảnh **cuối Trung cổ, quý tộc–thành thị–phường hội–tín dụng cùng tranh quyền**.

Lần trả lời trước đã nghiêng sang giới hạn số vùng, số dân và tính năng bản đầu. Người dùng làm rõ rằng câu hỏi là **mỗi hệ thống cần mô phỏng chi tiết tới mức nào**, không phải yêu cầu giảm số lãnh địa hoặc dân số. Các con số sáu vùng/10.000 người được đề xuất trong trao đổi không trở thành quyết định giảm phạm vi sản phẩm. Implementation plan vẫn giữ đích và các profile cần kiểm chứng của nó.

Đọc cùng [nghiên cứu nền xã hội cuối Trung cổ](late-medieval-world-design-discussion.md), [implementation plan](implementation-plan.md) và [trạng thái mã](../WORK_STATUS.md). Tài liệu này đặt ranh giới độ sâu; tài liệu nghiên cứu giữ nguồn lịch sử, quan sát code và phương án kiến trúc.

## 2. Điểm dừng được đề xuất

**Mô phỏng xã hội–kinh tế ở cấp cá nhân, hộ, cơ sở và thiết chế, với quyền/tài sản/nghĩa vụ có hậu quả thật; diễn giải đời sống bằng tập trạng thái và hành động hữu hạn.**

Một cá nhân cần có xuất thân, nhu cầu, tài sản, kỹ năng, cơ hội, quan hệ, hiểu biết và lựa chọn riêng. Các lựa chọn phải làm thay đổi đời sống của họ và cơ hội của người khác. Không cần mô phỏng toàn bộ tâm trí, cơ thể, sinh hoạt từng phút hoặc mọi chi tiết pháp luật lịch sử để đạt điều đó.

Độ chân thật đến từ tính liên kết và giới hạn: có tiền nhưng thiếu quyền; có kỹ năng nhưng thiếu chỗ làm; muốn liên minh nhưng đối tác có lợi ích riêng; thừa kế hợp pháp nhưng tài sản đang bị tranh chấp. Số biến lớn tự nó không chứng minh đời sống chân thật.

## 3. Ranh giới theo hệ thống

Các mức dưới đây là đề xuất cho độ sâu sản phẩm, không phải danh sách phần code hiện đã có.

| Hệ thống | Mức đủ sâu được đề xuất | Điểm dừng ban đầu |
|---|---|---|
| Cá nhân | ID và vòng đời; sức khỏe/nhu cầu thiết yếu; kỹ năng theo nhóm; mục tiêu, tính cách hữu hạn; thông tin và ký ức quyết định riêng | Không mô phỏng tâm lý lâm sàng, ngôn ngữ nội tâm, từng cơ quan hoặc sinh hoạt từng phút |
| Hộ gia đình | Thành viên động, cư trú, người phụ thuộc, giám hộ; tài sản riêng/chung phân biệt; phân bổ sinh kế và chăm sóc có nguồn lực | Không điều khiển từng bữa ăn, vật dụng hoặc thao tác chăm trẻ; vẫn hạch toán nhu cầu và thời gian chăm sóc |
| Địa vị/giai cấp | Tách địa vị pháp lý, đẳng cấp, tài sản, nghề và chức vụ; cơ hội thay đổi theo các điều kiện đó | Không có thang cấp duy nhất từ nông dân lên quý tộc; không cố tái tạo mọi hạng người của mọi vùng châu Âu |
| Nghề nghiệp | Nghề chính và việc theo mùa; kỹ năng, hợp đồng, người thuê, cơ sở; học việc, thất nghiệp và chuyển nghề có thời gian/chi phí | Không thao tác từng nhát búa; không cần nghề riêng cho mọi sản phẩm hoặc cấp bậc lịch sử |
| Sản xuất | Cơ sở có chủ/quyền sử dụng, đầu vào, lao động, công suất, sản lượng và bảo trì; người sản xuất và người nhận thu nhập xác định | Công thức sản xuất và năng suất ở cấp cơ sở; không mô phỏng vật lý từng công đoạn hoặc từng hạt giống |
| Tài sản/đất | Các quyền hữu hạn: canh tác, thu tô, chuyển giao, sử dụng tài nguyên, xét xử nếu luật cho phép; nghĩa vụ và tranh chấp gắn với quyền cụ thể | Đất theo holding/cụm thửa có ý nghĩa kinh tế; không cần địa chính từng mét vuông hoặc hệ thống quyền tùy ý vô hạn |
| Thị trường/tín dụng | Kho hữu hạn, nhu cầu, giá, giao dịch; chủ nợ, con nợ, bảo đảm, lãi, hạn và xử lý vi phạm | Không mô phỏng sổ lệnh tần suất cao, mọi đồng tiền vật lý hoặc toàn bộ thị trường tài chính hiện đại |
| Vận tải | Tuyến, thời gian, công suất, chi phí, quyền đi qua, hàng đang vận chuyển và tổn thất | Lô hàng theo tuyến; không pathfinding/animation riêng cho mỗi bao hàng hoặc mọi người mỗi ngày |
| Quan hệ | Thân tộc, bảo trợ, công việc, nợ, tổ chức và cam kết; tin cậy/thiện cảm hai chiều có thể khác; sự kiện có nguồn gốc | Không ma trận mọi người với mọi người, không mô hình riêng cho mọi sắc thái cảm xúc |
| Hôn nhân/gia tộc | Đề nghị và quyền chấp thuận theo luật đã chọn; tài sản, cư trú, người phụ thuộc, con và liên hệ dòng họ; liên minh là cam kết riêng | Không toàn bộ đời sống tình cảm, mô phỏng di truyền hoặc quan hệ hội thoại liên tục; không tự cưới là nhập lãnh địa |
| Thừa kế/kế vị | Người hưởng, tài sản/quyền, nghĩa vụ, claim, custody/nhiếp chính; chức bổ nhiệm khác tước thế tập; thi hành có thể bị chống đối | Một số mẫu luật có tham số; không bộ suy luận pháp lý hiểu mọi di chúc viết tự do hoặc mọi tập quán lịch sử |
| Tổ chức/chính trị | Tài khoản, thành viên, chức vụ, thẩm quyền và cam kết; người đại diện ra quyết định; quý tộc, đô thị, guild và giáo hội có lợi ích riêng | Một số thủ tục hữu hạn như bổ nhiệm, đồng thuận hoặc biểu quyết đã định nghĩa; không hiến pháp tùy ý hoặc mọi thành viên luôn dự mọi việc |
| Thông tin/bí mật | Biết ai, biết gì, nguồn và thời điểm; báo cáo trễ, điều tra, nghi vấn và chứng cứ tách biệt | Facts và tóm tắt ký ức có cấu trúc; không ghi mọi cuộc trò chuyện hoặc suy nghĩ, không LLM làm nguồn chân lý của world |
| Môi trường/sức khỏe | Mùa, thời tiết tương quan theo vùng, khả năng thu hoạch, sức khỏe và một số nhóm bệnh/nguy cơ; hậu quả lên nguồn sống | Mô hình tổng hợp có hiệu chuẩn; không CFD, vi khí hậu từng ô hoặc y sinh đầy đủ |
| Quân sự/xung đột | Lực lượng, chỉ huy, nghĩa vụ, tiền/hàng, huy động, tiếp vận và quyền kiểm soát; kết quả có nguyên nhân | Đội quân/đơn vị và tuyến chiến dịch; không chiến đấu vật lý từng binh sĩ hoặc chiến thuật thời gian thực |

Điểm dừng là giới hạn chi tiết bên trong từng hệ thống, không cho phép thay hệ thống bằng điểm số tùy ý. Ví dụ sản xuất dùng công thức vẫn phải có vật tư và người làm; hôn nhân có ít điều khoản vẫn phải chuyển đúng tài sản; quan hệ tin cậy đơn giản vẫn cần căn cứ và khác nhau theo người quan sát.

## 4. Độ sâu quyết định: agency có giới hạn

Đề xuất mỗi actor chọn **ý định có thời hạn và đối tượng cụ thể**: học nghề ở đâu, làm cho ai, xin vay bên nào, đề nghị điều khoản gì, hỗ trợ ai. Khi kế hoạch còn hiệu lực, luật thực hiện các bước đã cam kết; không gọi lại policy để chọn từng thao tác nhỏ. Khi điều kiện quan trọng đổi, actor có cơ hội xem lại kế hoạch. Việc đánh thức actor không chọn hộ chiến lược.

Policy học từ observation, memory và cơ hội của chính người đó. Tính cách, lợi ích hộ, danh phận, quan hệ và mục tiêu cá nhân có thể xung đột. Người nghèo vẫn có agency; số chức vụ/điều khoản phức tạp hơn xuất phát từ quyền và trách nhiệm thực tế, không vì cá nhân ngoài màn hình bị bỏ mô phỏng.

Không yêu cầu NPC hiểu mọi phương án hoặc lập kế hoạch tối ưu vô hạn. Mức hợp lý là có thể học hậu quả nhiều bước trong các bài toán đã định nghĩa, dùng ký ức thích hợp, phản ứng với điều kiện thay đổi và đôi khi quyết định sai có thể giải thích. INT không tự được tuyên bố là trí lược chỉ vì cộng tốc độ học hoặc tăng kích cỡ mạng.

Engine lập trình luật, quyền và thi hành; learned RL chọn hành động tự chủ. Trong phát triển có thể dùng controller test có nhãn để kiểm tra luật. Không thay lựa chọn bằng luật cứng rồi gọi là hành vi đã học.

Đây là **mục tiêu chất lượng còn phải chứng minh**. Pilot hiện tại chưa qua đầy đủ R1–R3; mở thêm hệ thống làm action space, phối hợp và credit assignment khó hơn. Kiểm chứng một họ quyết định trước khi ghép thêm; production training vẫn sau feature freeze, smoke training nhỏ dùng trong lúc xây.

## 5. Quan hệ giữa độ sâu và quy mô

Không giải bài toán quy mô bằng việc lưu mọi chi tiết cho mọi người. Dùng chung primitives, với dữ liệu bổ sung khi có ý nghĩa: mọi Person giữ danh tính và sinh kế; người có đất mở record quyền đất; người đang tranh chấp mở hồ sơ claim; người giữ chức có context của chức vụ. Bỏ chức không xóa tài sản hoặc lịch sử cá nhân.

Đây là cấp phát dữ liệu theo trạng thái thực, không tự giảm độ trung thực ngoài camera. Cohort nếu dùng phải được báo riêng và không tính thành người có ID/agency riêng. Những người thuộc cùng hộ/guild vẫn có thể chọn khác nhau.

Xây quan hệ thưa; tìm cơ hội qua chỉ mục và mạng thông tin; dàn lịch theo game time; batch inference với memory/sampling cá nhân. Không yêu cầu một triệu người mỗi người biết một triệu người khác. Không cần một node/scene/model riêng cho mỗi người.

Độ phức tạp vận hành cần tính trên:

`actor đến lượt + cơ hội được xét + cạnh/hồ sơ liên quan + giao dịch + cập nhật thế giới + lưu/trình bày`.

Không chỉ tính số Person record. Một biến cố khiến nhiều hộ thiếu ăn, mất việc hoặc đòi nợ cùng lúc phải nằm trong stress test. Cadence và lượng cơ hội hữu hạn cần đo tác động tới agency, công bằng và phản ứng; không tự thay để qua gate.

## 6. Quy tắc quyết định có thêm một chi tiết hay không

Chỉ thêm một chi tiết khi trả lời được các câu sau:

1. Nó thay đổi lựa chọn, quyền, nguồn lực hoặc hệ quả nào? Ví dụ quyền dùng rừng thay đổi sinh kế và lợi ích khi mở mỏ.
2. Ai có thể biết hoặc kiểm chứng nó? Nó hiện qua sổ, báo cáo, chứng cứ hay quan sát nào?
3. Nó có giao dịch/quá trình rõ để thực thi, lưu và tái hiện không?
4. Chi phí dữ liệu, truy vấn, policy và kiểm thử của nó là gì? Có thể biểu diễn bằng primitives sẵn có không?

Nếu chỉ làm tăng số thuộc tính mà chưa tạo quyết định hoặc hệ quả phân biệt được, để sau. Nếu bỏ chi tiết sẽ làm sai quyền sở hữu hoặc nguồn lực, cần mô phỏng dù ít xuất hiện trên UI.

## 7. Gate chứng minh mô phỏng đủ sâu

Đề xuất dùng các tình huống dưới đây làm nghiệm thu liên kết, không lập lịch câu chuyện bắt buộc trong campaign:

- **Đổi nghề:** người rời hộ đi học có chỗ/thầy và chi phí; hộ mất phần lao động, cơ sở nhận lao động; không nhân đôi thời gian hoặc tiền lương.
- **Liên hôn:** chuyển giao của hồi môn/nghĩa vụ có sổ; tạo quan hệ và quyền hưởng có căn cứ; hai nhà vẫn có thể bất đồng hoặc từ chối một hiệp ước khác.
- **Người giữ quyền chết:** tài sản, nợ, chức vụ và quyền chỉ huy được giải quyết riêng; người thừa kế còn nhỏ có quản giữ; tranh chấp không làm tài sản biến mất.
- **Biến cố kinh tế:** mất mùa/tắc tuyến lan qua kho, giá, việc làm, hộ và nghĩa vụ; can thiệp khác nhau của vua có thể đổi diễn tiến.
- **Tự vận hành:** việc thường lệ được địa phương, hộ và tổ chức xử lý khi vua không can thiệp; có cả tình huống không giải quyết thành công.
- **Thông tin:** người chơi/NPC hành động theo dữ liệu họ biết, phân biệt nghi vấn với sự thật; mở hồ sơ có thể truy được căn cứ còn hiệu lực.

Test luật và transaction kiểm tra tính đúng; đánh giá policy kiểm tra học/động cơ/thích nghi; benchmark kiểm tra chi phí toàn chu kỳ. Ba loại bằng chứng này không thay thế nhau. Vượt invariant mà xã hội chết đói hoặc mọi người chọn vô nghĩa thì chưa đạt mô phỏng tốt.

## 8. Điểm tiếp tục cho phiên làm việc sau

**Tiếp tục làm rõ thiết kế trước khi triển khai thêm gameplay.** Không suy ra người dùng đã duyệt mọi hàng trong bảng chỉ vì yêu cầu ghi docs.

Đề xuất trình tự thảo luận tiếp:

1. Rà bảng độ sâu ở mục 3, điều chỉnh các điểm người dùng muốn sâu hơn hoặc đơn giản hơn.
2. Chốt nền xã hội: địa vị và quyền di chuyển, tài sản/đất, hộ, cơ sở/nghề, tổ chức và bộ luật hữu hạn. Chốt riêng quyền hôn nhân/thừa kế, quyền của phụ nữ và các tập quán địa phương; chưa có quyết định cuối cho các mục này.
3. Viết ví dụ có sổ và quyền: một hộ, một xưởng, một lãnh chúa, một cuộc cưới và một vụ thừa kế. Kiểm tra các khái niệm có đủ để biểu diễn hệ quả mà không thêm ngoại lệ chắp vá.
4. Khi nền đã rõ, tách thành spec/domain contracts và lát cắt kiểm chứng; đo chi phí sớm trên workload thực. Mở rộng số lượng theo gate riêng, không dùng số vùng/dân thay cho độ sâu.

Các việc tác vụ này **chưa làm**: sửa luật C++, thay schema save/model, implement xã hội mới, retrain, benchmark, commit hoặc push. Đây là hồ sơ quyết định để lần sau không mất ngữ cảnh.
