# M5 — Draft nền xã hội, quốc gia và profile B

Ngày: 2026-10-15. **ĐÃ DUYỆT 2026-10-15 — triển khai đã bắt đầu.**

> **Trạng thái: APPROVED 2026-10-15.** Người dùng đã duyệt triển khai gameplay M5. Hướng dẫn nền tảng mới (2026-10-15): ưu tiên Windows trước; việc kiểm chứng app/package Linux bị hoãn trong M5 hiện tại, phần core/Python portable vẫn phải giữ và kiểm thử (chuẩn bị Kaggle). Hướng dẫn này thay thế các gate về app/package Linux, nhưng **không** xóa yêu cầu portability/testing của core và **không** cho phép giảm gameplay, bỏ test hoặc bỏ invariant. Lát cắt đầu tiên (household membership + giám hộ) đã xong; M5.1–M5.5 còn mở — xem [M5 progress](../reports/M5-progress.md).

Draft này cụ thể hóa [implementation plan](implementation-plan.md), tiếp nối M4 ở revision `055f523`. Các quyết định người dùng đã chốt nằm ở mục 1; những quy tắc cụ thể còn lại là đề xuất để duyệt trong lượt này. Đây không phải báo cáo code, benchmark hoặc RL đã đạt.

## 1. Đích và quyết định đã chốt

- Game desktop native; người chơi vào vai vua, cai trị một quốc gia trong một đời vua.
- Bối cảnh cuối Trung cổ giả tưởng, quý tộc–thành thị–phường hội–tín dụng cùng tranh quyền.
- Một bộ luật chung toàn vương quốc; không xây hệ tập quán riêng từng vùng.
- Hôn nhân cần hai người chấp thuận; gia đình có thể thương lượng tài sản và cam kết. Hôn nhân không tự nhập lãnh địa hay tạo liên minh.
- Thừa kế không phân biệt giới tính; tách tài sản, tước và chức bổ nhiệm. Có giám hộ, nợ và tranh chấp.
- Giáo hội giả tưởng có tài sản, chức vụ và thẩm quyền xác định.
- Độ sâu đến từ quyền, nguồn lực và hệ quả liên kết; không tăng chi tiết chỉ để giống đời sống.
- Quyết định tự chủ cuối cùng do learned RL policy; controller phát triển phải có nhãn.

M5 giữ mục tiêu của roadmap: 24 lãnh địa chư hầu + 3 khu triều đình, 16 nhóm hàng, thành thị/quốc phòng/ngoại thương và profile B với 100.000 cá thể thường dân/trẻ em, khoảng 512–2.048 bên quyết định phức tạp. Báo riêng số người, tổ chức và người đồng thời giữ nhiều vai; không cộng trùng thành dân số. Đây là mục tiêu cần đo, không phải khả năng đã đạt. Một triệu vẫn ở S; hoàn thiện UX một đời vua ở M6, production training ở M7.

## 2. Người chơi thực sự là vua

Vua có thể bổ nhiệm/bãi nhiệm chức hoàng gia, đặt thuế và nghĩa vụ trong luật, phân ngân sách, tài trợ dự án, huy động quân, ban đặc quyền, điều tra và xét xử vụ thuộc quyền. Mỗi lệnh ghi rõ vua hành động với tư cách nào, dùng tài khoản nào, tác động tới ai và ai thi hành.

Quyền ra lệnh không đồng nghĩa đã có kết quả. Việc thu thuế, xây đường hay huy động cần nhân sự, vật tư, thời gian và khả năng tiếp cận. Từ chối hoặc cản trở tạo hành vi/hồ sơ cụ thể; không tự trừ điểm trung thành rồi coi là đã mô phỏng xung đột.

Địa phương xử lý việc thường lệ bằng người giữ chức và nguồn lực của mình. Vua nhận báo cáo, kháng nghị, vụ liên vùng hoặc vượt quyền; có thể chủ động yêu cầu điều tra/can thiệp theo luật. UI ưu tiên chính sách, nhân sự, nguồn lực và hồ sơ; không buộc vua duyệt từng hợp đồng lao động hoặc cuộc cưới.

Đề xuất M5 dùng luật nền cố định, cho vua điều chỉnh các chính sách có tham số và cấp/thu hồi quyền được luật cho phép. Chưa xây trình soạn bộ luật tùy ý; sửa chế độ hôn nhân/thừa kế giữa campaign cần spec riêng nếu muốn đưa vào.

## 3. Luật nền đề xuất

| Phần | Contract đề xuất |
|---|---|
| Cá nhân | ID bền vững qua đổi hộ/nghề/chức; tuổi, sức khỏe, nhu cầu, kỹ năng, tài sản, thông tin và ý định riêng. Giới tính không chặn sở hữu, chức vụ hoặc thừa kế. |
| Địa vị | Tách thân phận pháp lý, danh phận, nghề, của cải và chức vụ. Mặc định dân tự do di chuyển sau khi giải quyết cam kết hợp lệ; nợ không tự biến người thành tài sản hoặc khóa di chuyển vĩnh viễn. |
| Hộ | Thành viên, cư trú, người phụ thuộc và giám hộ là record động. Tài sản riêng và quỹ chung tách biệt; vào hộ không tự chuyển toàn bộ của cải. |
| Đất | Holding/cụm đất có chủ, quyền sử dụng, thu tô và nghĩa vụ cụ thể. Cùng một quyền độc quyền không được cấp hai lần; quyền xét xử/chức vụ tách khỏi sở hữu đất. |
| Nghề/cơ sở | Cơ sở có chủ, kho, tài khoản, công suất, công thức và lao động thực. Học nghề cần chỗ, người dạy, chi phí và thời gian; không sinh thu nhập vì có nhãn nghề. |
| Thời gian lao động | Tổng thời gian làm, học và chăm sóc không vượt ngân sách cá nhân; không nhận lương hai nơi cho cùng thời gian. |
| Tổ chức | Quý tộc/gia tộc, thành thị, guild và giáo hội có thành viên, tài khoản, chức vụ và người đại diện. Quyền đại diện không đồng nghĩa sở hữu tiền tổ chức. |
| Hôn nhân | Đề nghị, hai chấp thuận còn hiệu lực, đủ tuổi/năng lực, không có hôn nhân khác và không thuộc quan hệ cấm. Giám hộ không thay sự chấp thuận của người chưa đủ tuổi. Của hồi môn có thể bằng 0; bên đóng góp phải tự chấp thuận. |
| Thừa kế tài sản | Trả nghĩa vụ bằng di sản trước; bảo vệ phần đồng sở hữu của người sống. Phần còn lại chia đều cho con hợp pháp không phân biệt giới; không có con thì cho phối ngẫu, rồi cha mẹ, rồi anh chị em. Không có người hưởng thì vào quỹ di sản công có sổ. |
| Tước và chức | Tước thế tập theo người con hợp pháp lớn tuổi nhất không phân biệt giới; thiếu người kế tiếp thì mở hồ sơ chờ vua công nhận người giữ tước. Chức bổ nhiệm trả về cơ quan bổ nhiệm; không tự truyền cùng tước. |
| Người phụ thuộc | Người chưa trưởng thành vẫn sở hữu tài sản; giám hộ chỉ quản lý, có tài khoản và giới hạn chi. Có thể bị điều tra/thay thế khi vi phạm. |
| Nợ sau chết | Chủ nợ đòi trên di sản/bảo đảm theo hợp đồng; người hưởng không tự gánh vượt di sản nếu không có nghĩa vụ bảo lãnh riêng. Tranh chấp giữ tài sản trong quản giữ, không xóa hoặc nhân đôi. |
| Giáo hội | Một tôn giáo giả tưởng cho M5; tổ chức sở hữu tài sản, vận hành cứu tế/giáo dục và bổ nhiệm nội bộ. Tòa thế tục thi hành tranh chấp tài sản và cưỡng chế; giáo hội ghi nhận nghi lễ, không có luật dân sự thứ hai. |
| Thông tin | Tách sự thật world, lời khai, nghi vấn và điều actor biết. Báo cáo có nguồn/ngày/độ trễ; UI vua không đọc trộm state bí mật. |

Đề xuất tuổi thành niên/kết hôn tối thiểu là 18 năm game; cấm hôn nhân trực hệ và anh chị em, mỗi người tối đa một phối ngẫu còn sống. M5 chưa có nhận con nuôi, ly hôn, di chúc tùy ý hoặc cơ chế con ngoài hôn nhân; không sinh các trường hợp này rồi âm thầm bỏ quyền. Đây là giới hạn đề xuất cần duyệt, không phải quyết định người dùng đã chốt.

Vua xác nhận tước hoặc quyết định vụ tranh chấp qua command có thẩm quyền và căn cứ; không được lấy tiền riêng của NPC bằng thao tác UI bỏ qua luật. Quyền tịch thu cần bản án/quyền pháp lý cụ thể và có hậu quả tới người liên quan.

## 4. Năm ví dụ giao dịch để nghiệm thu nền

Các số dưới đây là fixture kiểm thử, không phải sự kiện campaign bắt buộc. Mỗi ví dụ phải có nhánh thành công và bị từ chối, cùng thông tin đến đúng người.

### A. Một hộ và học nghề

Hộ H có quỹ chung 120; An có riêng 30, Bình có riêng 20; một trẻ phụ thuộc có giám hộ là Bình. An xin học ở xưởng X với học phí 10 từ tiền riêng. Khi hai bên chấp thuận, An còn 20, X nhận 10 và dành chỗ/người dạy; quỹ H vẫn 120. Thời gian học thay thế phần lao động tương ứng, khiến hộ mất thu nhập đó. Thiếu chỗ, thiếu tiền hoặc không đủ ngân sách thời gian thì toàn bộ lệnh bị từ chối. Hộ, xưởng và các bên hợp đồng biết; vua chỉ thấy báo cáo tổng hợp hoặc hồ sơ hợp lệ.

### B. Một xưởng dệt

X có tiền 100, 20 xơ, 5 nhiên liệu và hai người đã nhận hợp đồng. Fixture một kỳ dùng 10 xơ + 2 nhiên liệu, trả tổng lương 12 và tạo 8 vải: tiền X còn 88, người làm nhận tổng 12; kho còn 10 xơ, 3 nhiên liệu, thêm 8 vải. Sổ sản xuất ghi biến đổi vật tư theo công thức. Thiếu vật tư hoặc nguồn trả lương không được tạo đủ sản lượng bằng lao động miễn phí; tiến độ dở dang nếu hỗ trợ phải có record riêng. Thuế phát sinh theo căn cứ công bố, không đánh lặp vô ý.

### C. Một người giữ quyền

Lan có tiền riêng 50, giữ tước một lãnh địa và được vua bổ nhiệm chức chỉ huy. Quỹ lãnh địa 500 và quỹ đơn vị 200 không thuộc tiền riêng của Lan. Vua bãi nhiệm chỉ huy: quyền dùng quỹ đơn vị được bàn giao, tiền riêng và tước giữ nguyên. Lệnh của người đã mất chức bị từ chối cả khi UI còn snapshot cũ. Quân và cơ quan liên quan nhận thông báo theo độ trễ; tòa xử tranh chấp nếu có. Khi Lan chết, chức chỉ huy trống còn tước đi theo hồ sơ kế vị.

### D. Một cuộc cưới

An và Chi đều đủ tuổi, đồng ý kết hôn. Nhà An cam kết 40, nhà Chi cam kết 60 vào quỹ hộ mới. Tiền được escrow khi các chủ tài khoản chấp thuận; khi đủ điều kiện, quỹ mới nhận 100 và escrow về 0. Tài sản ngoài cam kết vẫn của chủ cũ; cư trú và chăm sóc người phụ thuộc phải xác định. Một bên rút trước khi hoàn tất hoặc cam kết hết hạn thì hoàn escrow đúng nguồn. Hai lãnh địa vẫn riêng, không tự sinh hiệp ước. Công nhận hộ/hôn nhân do cơ quan đăng ký; chuyển tiền do transaction, không do nghi lễ.

### E. Một vụ thừa kế

Người chết có riêng 100 và nghĩa vụ nợ 30; ngoài ra quản lý 40 của người phối ngẫu. Hai con hợp pháp, một người chưa thành niên. Trả nợ 30, hai con mỗi người hưởng 35, 40 của phối ngẫu không bị đưa vào di sản. Phần của trẻ được quản giữ dưới tên trẻ. Tước đến người con lớn tuổi hơn, chức bổ nhiệm trả về cơ quan; quyền chỉ huy cần người được bổ nhiệm mới. Có tranh chấp thì phần liên quan bị phong tỏa, phần không tranh chấp xử lý theo luật. Tòa địa phương xử trong quyền, vua nhận kháng nghị/vụ thuộc hoàng gia. Không được chi hai lần trước và sau save/load.

## 5. Quốc gia và kinh tế đầy đủ

- Map 24 lãnh địa + 3 khu triều đình dùng dữ liệu địa lý/topology theo plan; xác định rõ vùng hành chính, node vận tải và quyền kiểm soát, không mặc định mọi vùng có đường trực tiếp.
- 16 nhóm hàng theo plan: lương thực khô, thực phẩm bổ sung, muối, gỗ, nhiên liệu, sắt, kim loại màu/quý, xơ sợi, vải, gia súc/ngựa, công cụ, quân nhu/vũ khí, vật liệu xây dựng, giấy, dược liệu, hàng xa xỉ.
- Production theo cơ sở và lao động; mùa, tài nguyên, đầu vào, bảo trì và vận tải cùng ảnh hưởng khả năng cung ứng. Không dùng `region % 7` làm nghề/nguồn sản phẩm cuối cùng.
- Thành thị có quỹ, cơ sở, chỗ ở/cơ hội việc, đặc quyền và đại diện. Guild có thành viên, học nghề và điều kiện gia nhập hữu hạn; không một điểm guild thay toàn hệ thống.
- Quân sự có nghĩa vụ, đơn vị, chỉ huy, tuyển/huy động, lương, quân nhu và tiếp vận. Người đi lính không đồng thời sản xuất đủ tại nhà. Xung đột giải theo đơn vị/tuyến, không mô phỏng vật lý từng binh sĩ.
- Ngoại thương có đối tác/tài khoản ngoài biên, đơn hàng, tồn kho, tuyến/cửa khẩu, giá, thuế và rủi ro. Tiền/hàng qua biên được ghi nhập/xuất, không giả vờ là hệ kín rồi bù sai lệch.
- Khởi tạo phải có đất, kho, vốn, tuổi/hộ, kỹ năng, vacancy và nghĩa vụ tương thích. Endowment ghi rõ nguồn đầu kỳ; không bơm tiền/hàng ngầm để giữ xã hội sống.

## 6. Sự kiện chính trị trồi sinh

Engine tạo facts từ hành động và world process; hồ sơ gom facts có liên quan, người biết, quyền xử lý và yêu cầu đang tồn tại. Policy chọn đòi hỏi, thương lượng, hỗ trợ, cản trở hoặc hành động đối kháng trong luật. Engine thực hiện, ghi hệ quả và phát thông tin; không dùng ngưỡng bất mãn để tự chọn phản loạn thay policy.

Ba chuỗi cần kiểm chứng: chủ xưởng chết → di sản/nợ → trả lương/việc làm → guild/thuế; tắc tuyến → thiếu nguyên liệu/lương thực → giá/thu nhập → kiến nghị/can thiệp; huy động quân → giảm lao động/tăng chi → nghĩa vụ thiếu → thương lượng giữa vua và lãnh chúa. Đây là khả năng nhân quả cần hỗ trợ, không bảo đảm mọi lần chạy đều sinh cùng câu chuyện.

Test luật dùng command định sẵn để chứng minh chuỗi có thể xảy ra và can thiệp làm đổi kết quả. Test policy đánh giá riêng liệu NPC học chọn hành động hợp lý; không lấy controller test làm bằng chứng trồi sinh bằng RL.

## 7. Kiến trúc và tương thích

C++20 core sở hữu Person, Household, Holding/Right, Establishment, Employment/Apprenticeship, Organization/Office, Marriage, Estate/Claim, Ledger, Knowledge và lịch. Đây là trách nhiệm domain đề xuất; có thể composition/compact storage, không bắt buộc mỗi record là một object heap hay cây kế thừa.

Mọi Command từ vua/UI/AI qua cùng chuỗi: nhận diện actor và tư cách → validate quyền/tham chiếu/phiên bản/nguồn lực → lập thay đổi và kiểm tra invariant → commit nguyên tử → ghi facts/biên nhận → lịch báo cáo. Revalidate lúc commit để xử lý hai người tranh cùng tài nguyên; reject giữ nguyên state gameplay. Tác vụ dài tạo nghĩa vụ đã cam kết và các bước hoàn tất có ID chống thực hiện lặp.

Godot chỉ nhận PlayerView đọc được theo thông tin của vua và gửi command. Python gọi đúng core qua binding. Shared model weights, memory/sampling riêng; sparse relations và chỉ mục cơ hội. Không một node/model cho mỗi người, không dense N×N, không copy cả dân số mỗi frame.

Version hóa save, action, observation, reward và model binding khi thay contract. Không tự coi model realm-v6 tương thích với world mới. Đề xuất giữ bộ fixture và artifact M4 để regression; save cũ không chuyển được phải bị từ chối rõ ràng trước khi thay phiên. Migration chỉ được công bố sau khi có kiểm thử, không tự hứa hỗ trợ mọi save.

Game time chi phối deadline/cooldown; pause không tiến lịch, không chạy bù sau sleep/mất focus. Save ở phase barrier giữ ownership, escrow, việc đang làm, information delay, RNG và quyết định đang chờ.

## 8. Thứ tự triển khai và gate

| Bước | Kết quả review được | Gate trước bước tiếp |
|---|---|---|
| M5.0 | Chốt draft, bảng quyền, registry hàng và contract transaction/schema | Không còn lựa chọn gameplay nền bị bỏ ngỏ; xác định smoke/regression và công cụ build hiện có, kiểm chứng Linux M4 |
| M5.1 | Hộ/đất/quyền/tài khoản/địa vị và vòng đời | Ownership, bảo toàn sổ, giám hộ động, reject nguyên tử, save/load; đo chi phí sớm |
| M5.2 | Cơ sở, hợp đồng, học nghề, 16 hàng, vận tải/thị trường | Ví dụ A/B và shock liên vùng; thời gian lao động và vật tư có đối ứng |
| M5.3 | Tổ chức/chức, cưới/di sản/claim, giáo hội | Ví dụ C/D/E; quyền vua/đại diện, bảo mật thông tin và tranh chấp |
| M5.4 | Quốc gia 27 khu, thành thị, quân sự, ngoại thương và UI vua | Lát cắt nhân quả có nhiều cách can thiệp; toàn bộ thao tác qua command |
| M5.5 | Profile B, stress, báo cáo Windows/Linux | Workload đầy đủ, số đo và giới hạn công khai; lỗi gate còn mở không được đổi thành pass |

Các fixture nhỏ/sáu vùng phục vụ kiểm tra từng bước; không thay mục tiêu quốc gia. Nếu phát hiện contract không biểu diễn được quyền/nguồn lực, sửa spec có giải thích trước khi chắp ngoại lệ. Chỉ hỏi lại khi phải đổi lựa chọn gameplay đã duyệt hoặc ngân sách/phạm vi; các quyết định kỹ thuật trong contract được tự xử lý.

## 9. Bằng chứng nghiệm thu

- **Luật/transaction:** năm ví dụ, thiếu tiền/vật tư, sai quyền, target chết, stale command, cạnh tranh tài nguyên, xử lý lại cùng ID; không chuyển một phần state hoặc tạo quyền độc quyền trùng.
- **Tái hiện/lưu:** save/load giữa escrow, vận tải, học nghề, kế vị, điều tra và kỳ clearing; load hỏng không thay phiên; replay trên runtime tham chiếu đã kiểm chứng.
- **Thông tin/UI:** vua và NPC chỉ thấy dữ liệu được biết; báo cáo trễ và chứng cứ có nguồn; pan/zoom/picking/layers, điều hành và pause không sửa state trực tiếp.
- **Ổn định xã hội:** nhiều seed, tối thiểu sandbox 20 năm theo protocol phải ghi trước khi chạy; ghi sinh/tử, hộ thiếu sinh kế, việc làm, nợ, giá, quân số, hàng đợi và kết quả can thiệp. Chưa đặt một tỷ lệ sống tùy ý làm bằng chứng cân bằng; không chỉ kiểm invariant.
- **RL:** binding/parity/schema checks và smoke training có ngân sách riêng được ghi trước khi chạy. Không production training M5; R1–R5 còn phải đánh giá sau freeze. Đo chi phí inference bằng model tương thích có nhãn trạng thái; thiếu model thì báo phần chưa đo, không thay bằng no-op rồi chứng nhận toàn chu kỳ.
- **Scale:** 100k cá thể thường dân/trẻ em trên map đủ, báo riêng người có vai phức tạp và tổ chức; tải bình thường và stress đồng thời thiếu ăn, di cư, thu nợ, đề nghị, kế vị. Ghi cấu hình CPU/RAM/OS, revision, seed, ngày game, model/hash, số actor đến lượt và thực xử lý.
- **Hiệu năng:** đo core + observation + inference + validation/commit + graph/index, p50/p95/p99, backlog/tốc độ thực, FPS/input, RAM resident/peak, save size/time. Budget B từ plan: resident ≤6 GB, peak ≤8 GB cả save; input p95 <100 ms; ngày thường p95 ≤80 ms, kỳ tháng p95 ≤500 ms. Giữ các yêu cầu render còn lại của plan; 8× báo tốc độ thực, không skip NPC. Controller-only và end-to-end inference phải báo riêng.
- **Bàn giao:** build/test/package Windows và Linux ghi kết quả riêng; evidence chứa lệnh, workload, counts, số đo, ảnh và giới hạn. Không dùng kết quả Windows hoặc M4 để chứng nhận Linux/M5.

## 10. Những phần người dùng cần duyệt trong draft này

**Đã duyệt 2026-10-15.** Các mục dưới đây là các luật nền đã được người dùng chốt trong lượt duyệt này và được dùng để triển khai; nếu sau này muốn đổi luật giữa campaign cần spec riêng.

1. Luật nền mục 3, đặc biệt quyền di chuyển, tuổi 18, thứ tự hưởng di sản, tước theo con lớn nhất và giới hạn hôn nhân/thừa kế M5.
2. Quyền vua mục 2: chính sách có tham số và quyền can thiệp rõ, chưa có trình thay toàn bộ luật giữa campaign.
3. Giáo hội một tôn giáo giả tưởng, quyền dân sự dùng chung luật và tòa thế tục.
4. Phạm vi quốc gia/quân sự/ngoại thương và trình tự M5.0–M5.5; mọi hệ thống vẫn phải có nguồn lực/hệ quả thật.

Sau khi duyệt: đã cập nhật tài liệu quyết định, bắt đầu M5.0 và ghi kết quả các lát cắt trong [M5 progress](../reports/M5-progress.md). Tác vụ draft này không sửa gameplay thêm, không đổi schema thực tế, không train, không benchmark, không commit và không push.
