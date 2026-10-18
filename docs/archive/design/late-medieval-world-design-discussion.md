> LƯU TRỮ 2026-10-18. Snapshot trước hợp nhất; không thực thi prompt hoặc dùng trạng thái trong file này làm chỉ đạo hiện tại. Đọc [nguồn chuẩn](../../README.md).

# PolyEc — nền xã hội cuối Trung cổ và quy mô mô phỏng

Ngày: 2026-10-15. **Tài liệu nghiên cứu và đề xuất để thảo luận; chưa phải spec đã duyệt hoặc mô hình đã triển khai.** Không thay thế implementation-plan hay WORK_STATUS. Chưa có benchmark mới trong tác vụ này.

Người dùng đã chọn hướng **cuối Trung cổ: quý tộc, thành thị, phường hội và tín dụng cùng tranh quyền**. Các lựa chọn còn lại dưới đây là đề xuất. Không tự đổi campaign một đời vua thành campaign nhiều thế hệ.

Làm rõ sau trao đổi: câu hỏi scope nhắm tới **độ sâu simulation**, không phải yêu cầu giảm số dân/vùng. Xem [ranh giới độ phức tạp và điểm tiếp tục](simulation-complexity-scope.md); các quy mô thử nghiệm trong tài liệu này không tự thay mục tiêu sản phẩm.

## 1. Kết luận định hướng

Cần làm đầy quan hệ xã hội trước khi mở rộng số lượng và trước khi khóa vòng chơi M4. Ưu tiên quyền đối với tài sản, hộ gia đình, cơ hội nghề nghiệp, gia tộc, hôn nhân và thừa kế. Các cơ chế này cùng tạo ra nguồn thu, phụ thuộc, quyền lực và tranh chấp. Thêm tên nghề hoặc tăng dân số riêng lẻ chưa tạo được những hệ quả đó.

Không cần hoàn thiện toàn bộ lịch sử xã hội châu Âu rồi mới có bản chơi. Đề xuất một bước thiết kế nền xã hội trước M4; sau khi thống nhất, triển khai nền tối thiểu và một lát cắt kiểm chứng nó trên sáu vùng. Chưa production training và chưa mở 24 vùng trong bước nền này.

## 2. Bằng chứng từ repo hiện tại

Đối chiếu revision `f10ca25`, chủ yếu `core/include/polyec/realm.hpp`, `core/src/realm.cpp`, `native/realm_runtime.cpp`, `native/realm_benchmark.cpp` và `gdext/polyec_session.cpp`:

| Hiện có | Giới hạn đối với thế giới mới |
|---|---|
| Person có ID, tuổi, tiền, skill, household, organization, job và family | `family` là họ policy Governance/Economic/Civilian, không phải gia tộc hoặc đẳng cấp |
| Work và teacher payroll có nguồn | `job_kind` mới phân lao động/giáo viên; sản phẩm lao động thường gắn với `region % 7`, chưa gắn cơ sở sản xuất/nghề cụ thể |
| Hộ dân khởi tạo theo cụm bốn ID | Guardian tìm trong khoảng bốn ID; chưa là Household có thành viên và giám hộ thay đổi theo vòng đời |
| Sinh có ID mới, liên hệ tới một parent | Chưa có mô hình cặp đôi, gia phả, hôn ước, cư trú sau cưới và quyền thừa kế |
| Khi chết có xử lý tiền và hàng | Tiền vào quỹ estate chung, hàng về stock vùng; chưa phân phối theo người thừa kế/chủ sở hữu và nghĩa vụ tài sản |
| Organization membership và quỹ | Chưa có đủ nội quy phường hội, chức vụ, cơ chế đại diện và quyền của tổ chức |
| Sparse Edge | Chưa có đầy đủ ngữ nghĩa quan hệ thân tộc, hợp đồng, quyền tài sản và ký ức quan hệ |
| Core nhận population tới một triệu | Benchmark native hiện khởi tạo 10.000; giới hạn constructor không phải bằng chứng một triệu actor chạy đạt |
| Batch suy luận tối đa 128, model shared, GRU riêng | Lựa chọn đúng để tiếp tục; action candidates vẫn cố định 32 slot và nhiều target được lập sẵn |
| Facts có parent và knowledge | Facts cũ bị cắt theo ngưỡng; nếu làm thừa kế/ân huệ cần giữ bằng chứng đang có hiệu lực dù rất cũ |

Đây là giới hạn prototype, không phải kết quả test thất bại mới. Chưa sửa gameplay trong tác vụ nghiên cứu.

## 3. Khung lịch sử và cách chuyển thành thiết kế

“Âu phong kiến” không cung cấp một bộ luật thống nhất cho mọi nơi, mọi thế kỷ. Susan Reynolds phê bình cách áp một mô hình fief/vassalage tổng quát lên những quan hệ lịch sử khác nhau [S1]. Vì vậy đề xuất vương quốc giả tưởng lấy cảm hứng Tây/Trung Âu khoảng thế kỷ XIV–XV, với bộ luật được công bố rõ. Khoảng thời gian và vùng cảm hứng này chưa được người dùng chốt riêng.

Nghiên cứu về thừa kế tại Anh cho thấy tập quán có thể khác nhau ngay trong địa bàn gần nhau [S2]. Đề xuất dữ liệu luật theo jurisdiction và loại tài sản; không mặc định mọi ruộng, chức vụ, tước vị và tiền đều theo một luật trưởng nam.

Phường hội có thể vừa truyền nghề vừa bảo vệ đặc quyền. Epstein nhấn mạnh vai trò học nghề và di chuyển lao động có kỹ năng [S3]; nghiên cứu của Hugo Soly cho thấy quan hệ quyền lực giữa thương nhân và thợ trong ngành dệt thay đổi theo nơi/thời kỳ [S4]. Trong game, guild cần có thầy, chỗ học, phí, tiêu chuẩn và quyền thị trường cụ thể; mức độ đóng/mở là luật của từng tổ chức.

Giáo hội có thể nắm tài sản, quyền bảo trợ và ảnh hưởng tới chính quyền địa phương [S5]. Hướng cuối Trung cổ vì vậy nên có thiết chế tôn giáo với tài khoản, đất, chức vụ, trường/cứu tế và thẩm quyền cụ thể. Người đại diện quyết định trong giới hạn chức vụ; tổ chức không mặc nhiên là một bộ óc chung.

## 4. Cá nhân: tách địa vị, nghề, tài sản và chức vụ

Không dùng một enum để xếp mọi người trên thang nông dân → thợ → thương nhân → quý tộc. Đề xuất các trục độc lập:

| Trục | Ví dụ và hệ quả |
|---|---|
| Địa vị pháp lý | Người tự do, người lệ thuộc theo tập quán địa phương, thị dân có quyền; ảnh hưởng cư trú, dịch chuyển, hợp đồng |
| Tư cách đẳng cấp/thiết chế | Quý tộc, giáo sĩ, thường dân; quy định đặc quyền và nghĩa vụ cụ thể |
| Vị thế kinh tế | Thu nhập, nợ, tài sản và quyền sử dụng tài sản; không đồng nhất với huyết thống |
| Nghề/kỹ năng | Canh tác, chăn nuôi, dệt, rèn, vận tải, buôn bán, thư lại, quân vụ; có tiến trình học |
| Quan hệ sản xuất | Chủ đất, tá điền, người làm thuê, học việc, chủ xưởng; xác định ai trả ai và ai chịu rủi ro |
| Chức vụ/tước vị | Quản gia, hội đồng đô thị, lãnh chúa, giám mục; quyền hạn có phạm vi và thời hạn |
| Quan hệ xã hội | Hộ, thân tộc, gia tộc, bảo trợ, guild, đối tác, chủ nợ; tạo cơ hội và ràng buộc |

Một người có thể là quý tộc ít tiền, làm sĩ quan và nợ một thương nhân; một thương nhân giàu có thể thiếu quyền nhận chức; một góa phụ có thể giữ quyền thu nhập từ một phần điền sản theo luật đã chọn. Đây là những trạng thái hợp lệ cần biểu diễn được.

“Tầng lớp kinh tế” có thể là phân loại báo cáo suy ra từ tài sản/thu nhập. Địa vị pháp lý phải là trạng thái có căn cứ. Giàu lên không tự phong quý tộc; được bổ nhiệm không tự đổi huyết thống; bị phá sản không tự mất mọi quyền công dân.

### Chuyển nghề

Chuỗi nghiệp vụ đề xuất: biết cơ hội → ứng tuyển/đề nghị học → bên tiếp nhận quyết định → hợp đồng và chi phí → đào tạo/thử việc → làm việc → kết thúc/thăng bậc/chuyển việc.

Mỗi cơ hội trỏ tới thửa/đơn vị sản xuất/chủ thuê thật, địa điểm, kỹ năng, chỗ ở nếu cần, thời gian và quyền gia nhập. Học nghề cần chỗ và người dạy; mở xưởng cần vốn, cơ sở và quyền hoạt động. Thăng bậc không chỉ cộng một thanh XP.

Ví dụ con một hộ tá điền muốn học dệt ở thành thị: phải biết có chỗ học, giải quyết ràng buộc rời quê, chi phí đi và cư trú, tìm thầy nhận, sắp xếp người thay phần việc của mình ở hộ. Việc thành công làm giảm lao động hộ cũ và tăng nguồn thợ tương lai. Không mặc định mọi tá điền đều bị cấm di chuyển; phụ thuộc luật cụ thể.

Một cá nhân có thể có nghề chính và công việc theo mùa. Ghi phân bổ thời gian hữu hạn để không làm ruộng, dệt và quân dịch toàn thời gian cùng lúc. Đẳng cấp tác động tới cơ hội, chi phí và khả năng tiếp cận, không làm danh sách nghề bất biến suốt đời.

## 5. Các đối tượng miền cần có

| Đối tượng | Sở hữu/trách nhiệm |
|---|---|
| Person | ID, đời sống, tài sản riêng, năng lực, mục tiêu, thông tin và quyết định |
| Household | Thành viên có thời điểm gia nhập/rời, cư trú, người phụ thuộc, ngân sách và nghĩa vụ chung |
| Kinship | Cha/mẹ, con, vợ/chồng, giám hộ; không suy từ ID liên tiếp |
| House/Dynasty | Tư cách dòng họ, người đại diện và tài sản thực sự thuộc gia tộc nếu có; không sở hữu mặc định mọi tài sản thành viên |
| Holding/Tenure | Tài sản/đất và các quyền sử dụng, thu tô, chuyển nhượng; có người giữ quyền và jurisdiction |
| ProductionUnit | Ruộng/xưởng/mỏ/đoàn vận tải, đầu vào, công suất, lao động và tài khoản |
| Employment/Apprenticeship | Hai bên, việc, tiền/công nuôi, đào tạo, thời hạn, điều kiện chấm dứt |
| Title/Office | Người giữ chức, quyền pháp lý, nghĩa vụ, luật chuyển giao; tách khỏi chủ đất |
| Institution | Guild, thành thị, giáo hội, triều đình; nội quy, tài khoản, thành viên và cách ra quyết định |
| Contract/Obligation/Claim | Cam kết có hiệu lực, ai có quyền đòi gì, hạn và tài sản liên quan |
| Jurisdiction/Law | Luật địa phương, cơ quan công nhận và khả năng thi hành |

Quyền đất có thể chồng lớp: hộ canh tác, địa chủ thu tô, người khác có quyền dùng rừng, lãnh chúa giữ quyền xét xử. Không mặc định một owner duy nhất có mọi quyền. Giới hạn số loại quyền ban đầu để kiểm chứng được hạch toán và xung đột.

Hộ, gia tộc và tổ chức không nhân bản con người hoặc tiền. Chuyển nhà/chức vụ giữ nguyên Person ID. Tài khoản tập thể có sổ góp/rút/phân phối, không vừa tính vào tiền riêng vừa vào tiền chung.

## 6. Hôn nhân, liên hôn và thừa kế

Hôn nhân là quan hệ giữa người và một tập điều khoản được luật công nhận. Liên minh quân sự/chính trị là hợp đồng khác; có thể được thương lượng cùng hôn nhân nhưng không tự phát sinh từ một cạnh married.

Vòng đời đề xuất: tìm đối tác qua mạng quen biết/sứ giả → đề nghị → thương lượng điều khoản → các bên có quyền chấp thuận/từ chối theo luật đã chọn → đăng ký/thực hiện chuyển giao → cư trú và nghĩa vụ → góa bụa/chấm dứt/tranh chấp nếu đủ căn cứ.

Điều khoản có thể gồm của hồi môn, phần tài sản/thu nhập bảo đảm cho người góa, nơi cư trú, tư cách dòng họ của con và hiệp ước riêng. Không tạo tiền khi cưới. Khoản hứa giao sau trở thành nghĩa vụ có hạn. Phân biệt tài sản chuyển quyền với quyền hưởng thu nhập.

Nghiên cứu về kinh tế quý tộc nhấn mạnh vai trò của phụ nữ trong bảo tồn và chuyển giao của cải qua hôn nhân, quà tặng và thừa kế [S6]. Không thiết kế họ chỉ như một modifier quan hệ. Luật tài sản, quyền tham gia quyết định và quyền từ chối của các bên cần ghi rõ trong spec; đây là lựa chọn thiết kế, không tự suy từ nhãn lịch sử.

Hôn nhân có thể tạo lợi ích chung nhưng cũng tạo xung đột: của hồi môn chưa trả, hai nhánh cùng đòi một tước vị, người nhiếp chính kiểm soát nguồn thu của trẻ thừa kế, hoặc hai nhà có cam kết quân sự đối nghịch. Thân thuộc không ép policy luôn hợp tác.

Khi chết: ghi nhận di sản và các quyền liên quan → xác định nghĩa vụ/chủ nợ và quyền người phụ thuộc theo luật → xác định người có quyền hưởng → chuyển giao/custody/nhiếp chính → mở claim nếu có tranh chấp. Thứ tự thanh toán cụ thể phụ thuộc luật tài sản, không mặc định một luật cho mọi nơi. Chức được bổ nhiệm có thể khuyết; tước thế tập và đất có thể đi theo các quy tắc khác nhau.

Claim là quyền yêu cầu được ghi nhận, khác quyền kiểm soát thực tế và khác quyền sở hữu đã giải quyết. Người thừa kế không tự nhận mọi quân đội, bí mật, lòng tin và nghĩa vụ cá nhân của người chết. Tài sản đang tranh chấp vẫn cần người quản giữ, không biến mất khỏi hạch toán.

Chỉ theo dõi tổ tiên/nhánh đã mất có liên quan tới thân thuộc, quyền hoặc hồ sơ đang hiệu lực; không sinh gia phả vô hạn để trang trí. Thế giới mở đầu cần các gia đình và người thừa kế có sẵn hợp lý, vì một đời vua không đủ thời gian chờ mọi thiết chế hình thành từ đầu.

## 7. Quan hệ tạo đời sống xã hội

Ưu tiên sáu mạng liên kết: thân tộc/giám hộ; chủ đất–người sử dụng đất; chủ thuê–người làm/thầy–học việc; chủ nợ–con nợ/người bảo lãnh; bảo trợ–người được bảo trợ; thành viên–tổ chức/chức vụ.

Tách facts pháp lý khỏi nhận thức chủ quan. Hợp đồng ghi chính xác khoản nợ; bên A có thể tin B sắp thất hứa dựa trên thông tin mình biết. Trust của A đối với B có thể khác chiều ngược lại. Tranh chấp cần một đối tượng/hành động có thật, không chỉ điểm ghét.

Không tạo clique mọi thành viên trong một guild hoặc mọi cặp họ hàng. Membership nối người tới tổ chức; kinship lưu các cạnh nền và tra quan hệ khi cần. Chỉ lưu quan hệ cá nhân phát sinh có ý nghĩa. Thông tin đi qua mạng người, chuyến đi và báo cáo với độ trễ/chi phí, không lan tức thời toàn quốc.

Một chuỗi kiểm chứng đề xuất: hộ mất mùa → xin vay → người bảo trợ bảo lãnh → con hộ đó đi học nghề ở đô thị → guild đòi hạn chế thợ mới → hội đồng đô thị và chủ đất có lợi ích khác nhau. Cũng có thể không vay, không được nhận học, hoặc có trợ giúp khác. Đây là tình huống kiểm thử có nhánh, không phải lịch truyện campaign.

## 8. Mở rộng số entity mà giữ agency

### Phân biệt số lượng

Báo cáo riêng dân số thế giới, Person ID còn sống, hồ sơ người đã chết, actor gọi policy trong kỳ, hộ, cơ sở, hợp đồng và quan hệ. Một tổ chức hoặc hợp đồng là entity miền nhưng không phải một NPC mới. Không dùng cohort multiplier để gọi 100k người có ID là một triệu cá nhân.

### Dữ liệu và lịch xử lý

- Giữ C++ OOP ở API/invariant/ownership; lưu dữ liệu nóng trong mảng/chunk compact, dùng handle ID ổn định. Không một heap object, Godot node hay model riêng cho mỗi người.
- Tách dữ liệu thường dùng khỏi genealogy, văn bản, hồ sơ cũ; không cấp toàn bộ cấu trúc quý tộc cho mọi trẻ sơ sinh.
- Lưu ngày sinh và thời điểm cập nhật; không phải quét toàn dân chỉ để cộng tuổi. Nhu cầu vật chất vẫn được tính theo thời gian và tham gia hạch toán đúng kỳ.
- Dùng queue/bucket cho hết hợp đồng, lương, deadline, trưởng thành và cơ hội ra quyết định. Sốc có liên quan kích hoạt đánh giá lại; “đánh thức” không quyết định thay hành động. MASON là nguồn tham khảo cho lịch sự kiện [S7].
- Index theo settlement, cơ sở, nghề/kỹ năng, hộ, tổ chức, chủ nợ; truy vấn cơ hội trong tập có liên quan thay vì mọi người so với mọi người.
- Cặp đôi gặp nhau qua địa bàn/mạng quen biết/kênh ngoại giao; một số lượng cơ hội hữu hạn, có mở rộng tìm kiếm khi cần. Nghiên cứu marriage markets chỉ ra chi phí và giả định thiếu thực tế của việc mọi người biết/đánh giá mọi người [S8]. Không sao chép quota kết hôn hoặc bộ chấm điểm của nghiên cứu đó để thay RL.
- Thực hiện đọc observation/inference theo batch, sau đó validate và commit có thứ tự tái hiện; kiểm tra công bằng thứ tự khi tranh chỗ học/việc làm. Parallel mutation vào cùng kho/tài khoản cần thiết kế riêng.
- UI lấy view đã lọc/phân trang; không copy mọi Person/edge hoặc quét invariant toàn world mỗi lần refresh. Invariant đầy đủ vẫn giữ ở gate kiểm chứng; cơ chế runtime phải giữ kiểm tra transaction và đối soát, không tắt kiểm tra để lấy số đẹp.
- Giữ nghĩa vụ, căn cứ quyền đất/hôn nhân và bằng chứng tranh chấp đang hiệu lực khi compact lịch sử. Queue đầy không được xóa khoản chi/cam kết.

BioDynaMo cung cấp bằng chứng rằng index lân cận và locality dữ liệu là các nút thắt quan trọng trong mô phỏng lớn [S9]. Kết quả máy chủ của họ không chứng nhận game RL kinh tế này trên desktop 16 GB, và không phải đề xuất đổi engine.

### Ngân sách định lượng ban đầu

Các số sau là phép tính, **không phải benchmark**:

| Ví dụ ở một triệu người | Bộ nhớ/công việc |
|---|---|
| 256 byte state/người giả định | 256 MB, chưa tính tài sản/hộ/index/allocator |
| GRU 64 float32/người như runtime hiện tại | 256 MB riêng recurrent memory |
| 20 triệu cạnh 24 byte/cạnh giả định | 480 MB trước index và lịch sử; phải nói rõ đây là cạnh có hướng hay một record cho cặp |
| Observation hiện tại: 512 float32 cho mọi người cùng lúc | 2.048 MB (khoảng 2,05 GB), đơn vị thập phân; hiện runtime đã dựng batch bounded, cần giữ nguyên tắc này |
| Một quyết định/người/30 ngày, chạy 1× = một ngày/giây | Khoảng 33.333 quyết định/giây thực |
| Cùng cadence ở 8× | Khoảng 266.667 quyết định/giây thực, trước các lần phản ứng do sốc |

RAM chứa record có thể vừa nhưng suy luận, tìm cơ hội, transaction, graph và save mới quyết định throughput. Thay cadence cần đánh giá độ trễ phản ứng và chất lượng, không tự giảm agency ngoài camera. Đo spike ở 10k → 100k → 300k → 1m trên cùng workload thực; 300k chỉ là điểm đo trung gian đề xuất, không tự đổi mục tiêu phát hành.

Code hiện còn quét dân số nhiều lần khi tìm actor đến lượt/sync role, quét các khoản vay/đề nghị khi dựng candidates và mask, quét world khi tạo một số view/invariant. Save hiện là text với nhiều chuỗi trung gian; restore có giới hạn 256 MiB cho file runtime. Cần profile từng đường; chưa kết luận đường nào là bottleneck lớn nhất hoặc giới hạn đó chắc chắn bị vượt ở một dân số cụ thể.

## 9. Hệ quả với RL

Ba họ policy hiện tại là tổ chức tính toán, không phải ba giai cấp xã hội. Giữ shared weights và sampling/memory riêng. Không tạo một model cho từng nghề/gia tộc.

Người có thể đồng thời giữ nhiều vai trò. Có thể thử policy điều kiện hóa theo vai trò đang hành động, hoặc router lựa chọn tác vụ đã học; phải phân định ngân sách, thời gian và người chịu trách nhiệm. Chưa khóa kiến trúc trước spike.

Action cần mang đối tượng thật: ứng tuyển vị trí nào, học với thầy nào, đề nghị hôn nhân với ai, điều khoản nào, khởi kiện claim nào. Mảng 32 candidates có target cố định hiện tại sẽ cần thiết kế lại. Đề xuất thử policy đánh giá slate candidates có feature đối tượng và option tìm thêm; retrieval chỉ dùng khả năng tiếp cận, luật và nguồn thông tin. Audit recall/độ đa dạng để tầng tìm kiếm không bí mật chọn sẵn “nghề tốt nhất” hoặc “vợ/chồng tốt nhất”. Nghiên cứu action representations là hướng tham khảo, không là thuật toán đã chọn [S10].

Luật kiểm tra tuổi, quyền, số chỗ, số dư và tính hiệu lực của hợp đồng; learned policy chọn mục tiêu và điều khoản trong khả năng của mình. Không dùng if quý tộc thì luôn liên hôn, hoặc mất việc thì luôn chuyển nghề để thay policy.

Reward phải cân bằng đời sống, hộ, cam kết và mục tiêu cá nhân qua thời gian; không thưởng theo số lần cưới, số con hoặc số lần đổi nghề. Tránh khiến mọi người tối ưu cùng một ý chí gia tộc: thành viên có lợi ích riêng. Critic có thông tin training rộng hơn nếu thiết kế cho phép, nhưng observation của actor chỉ chứa điều người đó biết.

Thay luật nghề, tài sản, hộ, thừa kế đồng nghĩa version hóa transition/action/observation/reward, save và model bundle. Pilot hiện tại không tự hiểu các hệ thống mới. Trong giai đoạn xây dùng controller test có nhãn và smoke training nhỏ; quyết định tự chủ của bản hoàn chỉnh vẫn phải qua learned RL quality gates.

## 10. Tiêu chuẩn kiểm chứng trước khi tăng quy mô

1. Một người đổi nghề, cư trú, hộ hoặc chức vụ vẫn giữ ID và quyền/tài sản hợp lệ; không nhân đôi nguồn thu.
2. Trả lương, của hồi môn, hoàn escrow, thừa kế và settlement nợ có đối ứng; command lỗi không thay đổi một phần state.
3. Hộ có thể tách/nhập, nhận giám hộ mới và mất thành viên mà không dựa vào ID liên tiếp; không có hai người cùng giữ một quyền độc quyền trái luật.
4. Kết hôn không tự nhập lãnh địa hoặc tự có hiệp ước; chết không tự giải quyết mọi claim; quyền lợi đang tranh chấp có người quản giữ.
5. Thiếu chỗ học/nhà/vốn hoặc bị từ chối làm kế hoạch phải đổi; cùng xuất thân không ép mọi người cùng nghề. Đo kết quả có điều kiện theo luật/cơ hội, không đặt quota di động xã hội phải đạt.
6. Gián đoạn tuyến, mất mùa và chết người giữ chức lan qua hộ, hợp đồng, sản xuất và thu nhập; có thể giải thích bằng facts được biết.
7. Không lộ của cải/ý định bí mật qua candidate retrieval; không có toàn tri về mọi đối tác hôn nhân/việc làm.
8. Stress gồm sinh–tử, học việc, di cư, thu nợ, liên hôn, tranh chấp và shock tập trung. Đo core + observation + inference + validation/commit + graph/index + render/input + save, p95/p99, backlog và peak RAM trên máy ghi rõ cấu hình.
9. Profile lớn phải so cả chất lượng: cơ hội bị bỏ sót, độ trễ phản ứng, công bằng thứ tự và phân bố hành vi. Không chỉ kiểm invariant hoặc số record.

Khởi tạo world cần dân số theo tuổi/hộ, đất/nguồn sống, kỹ năng, nghề, vacancy, nợ và nghĩa vụ cùng tương thích; không dùng dân số ngẫu nhiên với tài sản sinh kế thiếu rồi diễn giải sụp đổ thành “AI chọn sai”. Mọi endowment đầu kỳ phải hữu hạn và ghi sổ. Hiệu chuẩn là công việc tương lai; nghiên cứu này chưa xác nhận cân bằng.

## 11. Thứ tự đề xuất và những điểm cần bàn

Thứ tự: (A) thống nhất quy tắc xã hội → (B) hộ/kinship/tài sản/địa vị → (C) cơ sở sản xuất/hợp đồng/học nghề → (D) gia tộc/hôn nhân/thừa kế/claim → (E) lát cắt M4 thể hiện các hệ quả → (F) mở quốc gia và profile lớn. Đo hiệu năng ngay từ B, không đợi mọi feature xong. Trong mỗi phần giữ phạm vi sáu vùng để có thể kiểm tra liên hệ; không cần trì hoãn toàn bộ UI đến F.

Các câu cần thống nhất trước khi biến thành spec:

- Mức độ lệ thuộc nông dân và quyền thành thị: luật chung với ngoại lệ hay các vùng có tập quán khác nhau?
- Giáo hội có luật/tổ chức độc lập tới đâu; dùng tôn giáo giả tưởng nào và quyền tài sản/chức vụ nào?
- Luật kế thừa đất, tước và chức vụ; quyền của phụ nữ, con ngoài hôn nhân, góa phụ và người giám hộ; ai công nhận/thực thi?
- Ai có quyền đề nghị/chấp thuận hôn nhân và nghĩa vụ đi kèm? Quan hệ tình cảm cá nhân ảnh hưởng quyết định tới mức nào?
- Người chơi chủ yếu can thiệp ở cấp vua; tranh chấp thường dân do cơ quan địa phương tự xử, trừ vụ vượt quyền hoặc liên vùng?
- Số cá thể mong muốn là mục tiêu nghiên cứu hay số tối thiểu phát hành? Hiện plan ghi 100k đề xuất và 1m mở rộng, chưa được tác vụ này thay đổi.

Đề xuất ban đầu: vương quốc giả tưởng cuối Trung cổ, nhiều tập quán nhưng dùng cùng bộ primitives; hộ và cơ sở có kinh tế thật; quý tộc/guild/thành thị/giáo hội có quyền cụ thể; hôn nhân–thừa kế có hậu quả; vua cai trị qua thiết chế. Chưa tự đổi tên vùng hoặc artwork trong nghiên cứu này.

## 12. Nguồn đã tham khảo và giới hạn

Các kết luận về code dựa vào source local. Đề xuất thiết kế là suy luận cho PolyEc, không phải những điều nguồn bên ngoài khẳng định game phải làm. Nguồn lịch sử dưới đây thiên về Anh/Tây Âu, không đại diện thống nhất toàn châu Âu. Một số trang chỉ cung cấp abstract/extract; không tuyên bố đã đọc toàn văn sách hoặc bài bị paywall.

- **S1 — Susan Reynolds, Fiefs and Vassals (1994), abstract OUP:** https://academic.oup.com/book/47420 — phê bình mô hình phong kiến tổng quát.
- **S2 — David Roden, Inheritance Customs and Succession to Land in the Chiltern Hills, extract:** https://www.cambridge.org/core/journals/journal-of-british-studies/article/abs/inheritance-customs-and-succession-to-land-in-the-chiltern-hills-in-the-thirteenth-and-early-fourteenth-centuries/48D0E3E859B349D7CC47607618F6D42F — đa dạng tập quán thừa kế.
- **S3 — S. R. Epstein, Craft Guilds, Apprenticeship, and Technological Change in Preindustrial Europe (1998), abstract:** https://doi.org/10.1017/S0022050700021124 — học nghề, kỹ năng và di chuyển thợ.
- **S4 — Hugo Soly, The Political Economy of European Craft Guilds, Cambridge:** https://www.cambridge.org/core/journals/international-review-of-social-history/article/political-economy-of-european-craft-guilds-power-relations-and-economic-strategies-of-merchants-and-master-artisans-in-the-medieval-and-early-modern-textile-industries/CB6B4FD5B10DF57521C5B44F2BBF2A55 — tương quan quyền thương nhân/thợ, trích đoạn kết quả tìm kiếm.
- **S5 — Ecclesiastical Patronage Rights in the Thirteenth Century, Cambridge:** https://www.cambridge.org/core/books/abs/nobility-and-ecclesiastical-patronage-in-thirteenthcentury-england/ecclesiastical-patronage-rights-in-the-thirteenth-century/05D4C7DE2BD4CEC24D2531040F3B1A62 — quyền bảo trợ và điền trang, trích đoạn kết quả tìm kiếm.
- **S6 — Aristocratic Economies: Women and Family, Oxford Handbook:** https://academic.oup.com/edited-volume/34370/chapter-abstract/291509368 — phụ nữ, gia đình, hôn nhân và tài sản, abstract qua kết quả tìm kiếm.
- **S7 — MASON Schedule, tài liệu chính thức:** https://people.cs.gmu.edu/~eclab/projects/mason/classdocs.22/sim/engine/Schedule.html — lịch sự kiện; kết quả tìm kiếm đọc được, mở trực tiếp lỗi fetch trong lần nghiên cứu này.
- **S8 — Walker & Davis (2013), Modelling Marriage Markets:** https://www.jasss.org/16/1/6.html — lựa chọn hai phía, thông tin hạn chế và chi phí đối sánh dân số; dân số nghiên cứu hiện đại, không dùng làm chuẩn hành vi Trung cổ.
- **S9 — Breitwieser et al. (2023), High-Performance and Scalable Agent-Based Simulation with BioDynaMo:** https://arxiv.org/abs/2301.06984 — neighbor search và memory locality; benchmark khác workload/máy PolyEc.
- **S10 — Chandak et al. (2019), Learning Action Representations for Reinforcement Learning:** https://proceedings.mlr.press/v97/chandak19a.html — hướng nghiên cứu biểu diễn action; không chứng minh lựa chọn slate cụ thể của đề xuất này.

National Archives có kết quả tìm kiếm liên quan wardship/inheritance, nhưng mở trực tiếp bị 403; không dùng các chi tiết tuổi hoặc quy tắc của trang đó làm luật game.
