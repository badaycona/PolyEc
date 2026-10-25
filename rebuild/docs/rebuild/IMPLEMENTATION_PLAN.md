# PolyEc xây lại — mục tiêu, phạm vi và lộ trình

Phiên bản định hướng: `political-economy-rebuild-v1`, ngày 2026-10-25. Đây là nguồn duy nhất về scope và roadmap cho lần xây mới; kế hoạch M0–M7/M5 và R1–R4 trước đây không còn điều hành việc triển khai.

## 1. Trò chơi cần tạo ra điều gì?

Game native về kinh tế chính trị trong một quốc gia phong kiến giả tưởng. Người chơi là vua, hành động qua quyền hạn, tài sản công, bổ nhiệm, thương lượng, yêu cầu và thông tin nhận được. Công cụ debug điều khiển NPC bất kỳ không phải quyền của người chơi.

Người dân, thương nhân, người làm nghề và người giữ chức đều là NPC có danh tính riêng. Họ khác nhau bởi tài sản, kinh nghiệm, cơ thể, xu hướng, mục tiêu, quan hệ và điều họ biết. Các chức/nghề có thể đổi; không có ba giống người Governance/Economic/Civilian với quyền hoặc bộ não cố định.

Vòng chơi chính: phân bố tài sản và nguồn lực không đều → NPC làm việc, tiêu dùng và giao dịch → giá, thu nhập, nghĩa vụ và quan hệ thay đổi → xuất hiện vấn đề quản trị → người chơi/NPC can thiệp trong quyền của mình → hậu quả kinh tế và chính trị tiếp tục phát sinh.

Ví dụ cần chơi được: đường bị đóng khiến vùng thiếu lương thực; thương nhân cân nhắc đổi tuyến hay bỏ chuyến; công chức nhận yêu cầu nhưng thiếu nguồn lực; thuế và chi công ảnh hưởng dân cư; chiến tranh làm mất người, đứt giao thương và tốn quân nhu. Sự kiện phải có nguyên nhân và nguồn lực thật.

## 2. Ranh giới độ sâu

Đào sâu lựa chọn, đánh đổi và hậu quả giữa các hệ thống. Mỗi hoạt động nội bộ chỉ chi tiết đến mức cần giải thích chi phí, thời gian, kết quả và quyền của các bên.

| Hệ thống | Phạm vi cơ bản | Chưa mở rộng trong bản đầu |
|---|---|---|
| Ăn uống | Lựa chọn trừu tượng theo chi phí, thời gian chuẩn bị và độ ngon; no khác vui vẻ | Tên món, từng gia vị/bước nấu, tiêu hóa hoặc vi chất |
| Cơ thể | Đói, khát, mệt, sức khỏe; hoạt động và nghỉ có hậu quả | Mô phỏng cơ quan, y khoa hoặc hàng chục bệnh |
| Sản xuất | Nguyên liệu, công cụ cần thiết, công thực hiện, đầu ra, chi phí | Kỹ thuật chuyên ngành sâu của từng nghề |
| Đồ dùng | Công dụng thật; hao mòn cho công cụ/phương tiện có ý nghĩa | Theo dõi độ bền từng đơn vị hàng tiêu dùng |
| Nhà/đất | Sở hữu, quyền dùng, người đang ở, phí, nghĩa vụ và yêu cầu xử lý | Tọa độ phòng, nội thất hoặc kỹ thuật xây dựng chi tiết |
| Xã hội | Trò chuyện, trao đổi thông tin, thiện cảm có hướng, lời nhờ và phản hồi | Hội thoại ngôn ngữ tự do hoặc mô phỏng tâm lý toàn diện |
| Vi phạm | Phương tiện và tài sản thật, người bị tác động, người biết, phản ứng và xử lý cơ bản | Danh mục tội/hình phạt/thủ tục tư pháp chuyên sâu |
| Giao thông | Tuyến trừu tượng, tiến độ, công sức, sức tải, dừng/nghỉ và nơi đến | Đếm từng bước chân hoặc giao thông vật lý |
| Chiến tranh | Quyết định chính trị, người tham gia, tiếp vận, kết quả và thiệt hại trừu tượng | Điều khiển đội hình hoặc chiến thuật thời gian thực |

Danh mục 18 nhóm cũ là nguồn rà ý tưởng, không phải quota hành động, lớp, file hay bài test. Dùng các nhóm hành động chung có mục tiêu và tham số; chỉ tách cách thực hiện khi điều kiện, tiến trình hoặc hậu quả thực sự khác.

Hình dạng lệnh chung gồm actor, action/verb, target, goal khi cần, bộ tham số có tên và version/context cần để kiểm tra stale input. Ví dụ lệnh mua nêu người mua, nơi, hàng, lượng và nguồn thanh toán. Không tạo một loại class/file cho từng lượng, giá, hàng hoặc target; cũng không đưa luật của mọi domain vào một switch lớn. Giữ definition/registry gọn, còn xử lý theo domain khi cần quy tắc khác nhau.

## 3. Những nguyên tắc phải giữ

- NPC là người quyết định. Hộ là quan hệ gia đình/cư trú; tổ chức có thể có tài sản và tài khoản, NPC đại diện theo tư cách cụ thể. Không có policy của hộ/tổ chức thay thành viên.
- Tiền thuộc từng người hoặc tổ chức; không có quỹ hộ và không tự chi tiền người khác. Tặng/trả hộ/vay có người ra lệnh, nguồn tiền và consent cần thiết.
- Một learned RL policy dùng chung cho NPC trong sản phẩm hoàn chỉnh; bộ nhớ, thông tin và lấy mẫu riêng từng người. Không biến baseline hoặc scripted scenario thành AI đã học.
- Khả thi khác hợp pháp. Một hành vi vi phạm được mô phỏng phải có phương tiện, chi phí, tài sản, sự việc và phản ứng thật; không dùng cờ illegal để bỏ kiểm tra actor hoặc tự tạo consent.
- NPC chọn ăn, mua, nghỉ, làm, đi, nhận lời và đổi mục tiêu. Engine chỉ chạy cơ thể, hậu quả và tiến trình hoạt động đã chọn; không auto-buy/eat/medicate để giúp policy sống.
- Độ phân giải thời gian và nhịp quyết định cần được thiết kế lại cho vòng chơi mới. Không mặc định dùng cadence 7/30 ngày theo family hoặc 64 tick/ngày của codebase cũ. Hoạt động đang diễn ra phải tiếp tục có thể tạm dừng/gián đoạn theo luật được chọn; mất focus/pause không tiến hoặc chạy bù game time.
- C++ là một nguồn luật; Python dùng cùng core, Godot nhận dữ liệu chỉ đọc và gửi lệnh. Pause/mất focus/ngủ máy không tiến hoặc chạy bù game time.
- Hết hợp đồng thuê không tự làm người đang ở biến mất. Chức vụ không đồng nghĩa sở hữu tài sản công. Thông tin của NPC khác sự thật toàn tri của engine.

## 4. Lộ trình theo kết quả

Không đặt lịch theo giờ. Mỗi bước chỉ mở phạm vi tiếp theo khi chuỗi chính đã được kiểm; một chức năng không bị xem là hoàn thành chỉ vì đã tạo class hoặc build được.

### A. Một lãnh thổ có vòng kinh tế chính trị

Tạo một cấu hình nhỏ, có seed và danh sách người/tài sản/nguồn rõ ràng. Số NPC của fixture được ghi trong plan trước code; fixture nhỏ phục vụ kiểm luật, không thay mục tiêu quy mô quốc gia.

Phạm vi: NPC có nhu cầu và tiền riêng; chủ tài sản và người lao động; nguồn thực phẩm và một chuỗi sản xuất phi thực phẩm tối thiểu; giao dịch; bữa ăn theo giá/thời gian/độ ngon; nghỉ và chỗ ở; công việc có điều khoản; học qua thực hành; trò chuyện/thông tin/lời nhờ; người quản trị địa phương đại diện tổ chức có tài khoản, một nguồn thu và một khoản chi công có đối tượng thật. Khởi đầu phải có bất bình đẳng có thể giải thích, không mọi hộ cùng đất/cash.

Điều kiện đóng: cùng một world chạy được lao động → tạo hàng → nhận tiền theo điều khoản → mua/tiêu dùng → thay đổi cơ thể/vui vẻ → thực hành tăng hiệu quả → thu/chi công → sự việc tới người biết. Kiểm đủ nguồn thiếu, từ chối nguyên tử, nhiều người tranh tài nguyên, cam kết chưa hoàn thành, save/load giữa hoạt động và UI gửi lệnh thật. Nếu học/đối thoại chưa có kết quả thật thì không đánh dấu hoàn thành bằng hồ sơ rỗng.

### B. Hai lãnh thổ và tuyến giao thương

Tạo hai vùng có lợi thế nguồn hàng khác nhau; cùng luật từ A. Thêm tuyến có thời gian/sức tải, vị trí hành trình, hàng đang chở, phí có người nhận, nghỉ dọc đường và ở trọ tại vùng đến. NPC nhận tin về cơ hội, chọn chuyến đi, có thể tiếp tục, điều chỉnh hoặc bỏ mục tiêu; không teleport, tự mua đồ ăn hay tự chọn trọ thay họ.

Điều kiện đóng: một người mang hàng thật qua tuyến, tự chọn các bước chuẩn bị/nghỉ/ở trọ/giao dịch, chịu chi phí và giữ mục tiêu qua save/load. Đóng đường hoặc thay thông tin dẫn tới tập lựa chọn mới; engine không viết sẵn chiến lược tối ưu. Kiểm toàn bộ tiền/hàng xuất phát–đang đi–đã đến và nghĩa vụ giao hàng. Các chuỗi test có lệnh định trước chỉ chứng minh luật, chưa chứng minh learned policy tự làm được.

### C. Xung đột và chính trị giữa hai lãnh thổ

Thêm thương lượng, quyết định ngăn/mở giao thương, huy động theo quyền/consent đã đặc tả, tài khoản quân nhu, lực lượng là NPC thật và chiến đấu trừu tượng. Bao phủ cơ bản hành vi chiếm đoạt/lừa dối/lạm quyền, phản ứng người bị tác động và xử lý thông qua cùng các nhóm hành động; không mở một simulator riêng cho mỗi biến thể. Các cách cưỡng ép/hình phạt chưa được chốt phải đặc tả trước, không suy ra từ vai vua.

Điều kiện đóng: can thiệp chính trị thay đổi nguồn lực hoặc quyền thực; chiến tranh tiêu vật tư, thời gian và có hậu quả người/tài sản/thương mại; đàm phán/vi phạm/phản ứng có actor và nguồn thông tin. Không sinh quân/hàng ẩn hoặc trừ hao theo một nhóm cướp không tồn tại. Một tình huống không có xung đột cũng phải tiếp tục vận hành bình thường.

### D. Mở rộng cùng luật lên 27 vùng

Đưa bối cảnh Atlas vào cấu hình, không làm 27 hệ riêng. Kiểm mạng nhiều chặng, quyền/phạm vi tổ chức, nguồn hàng và causal chain xuyên vùng. Catalog có27 tên không chứng minh mô phỏng27 vùng. Dân số bối cảnh không tự là số NPC đang chạy.

Điều kiện đóng: generator có seed và đối soát; replay/save ổn định; giao diện giải thích được hậu quả nhiều vùng; đo toàn chu kỳ quyết định/hoạt động/lưu game trên workload có số cá thể thực. Chốt ngân sách dân số, RAM và độ trễ trước benchmark phát hành, không hạ người hoặc đếm bản sao hiển thị để báo đạt.

### E. Learned policy và bản phát hành

Chỉ mở training sau khi gameplay của A–D cần cho bản đầu đã hoạt động và được kiểm qua core/UI; tách giai đoạn kiểm luật bằng command khỏi kết luận AI tự chơi. Huấn luyện có ngân sách riêng, dùng một policy chung và dữ liệu chính C++ hiện hành. Không mang ngân sách/seed ba family/model cũ sang.

#### Dữ liệu huấn luyện phải tạo từ đâu và gồm gì

Trainer gọi đúng C++ core đang dùng trong game. Với mỗi world seed/config, core tạo trạng thái ban đầu; mỗi lượt lấy observation riêng của actor, danh sách hành động cùng target/parameter và điều kiện hiện tại; policy chọn một lệnh; core kiểm tra và commit/từ chối; sau đó thời gian tiến, sự kiện và kết quả được ghi. Python không cài lại luật sản xuất, tiền, quyền, đường đi hay đời sống để tạo “dữ liệu tương tự”.

Mỗi lượt dữ liệu phải nối lại được với world seed, phiên bản luật/cấu hình/model contract, actor ID, tick, quan sát, action definition và tham số, kết quả validator, thay đổi/chi phí, reward và outcome, trạng thái kết thúc/tạm dừng, policy version và nguồn quyết định (policy, baseline dev có nhãn, hoặc scenario điều khiển). Giữ raw data đủ tái lập; không chỉ giữ trung bình reward. Lệnh bị từ chối và hoạt động thất bại theo luật phải phân biệt.

Chia world seed độc lập thành train, validation và holdout trước khi học/chọn model; không để cùng seed, cùng world state hoặc biến thể bắt nguồn từ một world xuất hiện ở nhiều tập. Train dùng để cập nhật trọng số; validation chỉ để chọn giữa các ứng viên; holdout chỉ chạy sau khi đã khóa ứng viên và không được dùng chỉnh lại. Nếu dùng baseline viết tay để khởi tạo/đối chứng thì ghi rõ đó là baseline, không gọi là policy đã học.

Mỗi run ghi cấu hình, seeds, lượng quyết định, thời gian/RAM, kết quả và raw metrics; lưu biểu đồ được vẽ từ đúng run/version. Training chart đặt reward/return theo update hoặc quyết định, giữ dữ liệu gốc và thể hiện độ phân tán seed. Evaluation chart riêng dùng world seeds độc lập ghép cặp giữa policy mới, baseline và policy ban đầu; thể hiện độ bất định cùng survival, khả năng thanh toán, hành động và các tình huống liên quan. Reward của rollout train không phải điểm đánh giá độc lập.

Kiểm khả năng tự vận hành các tình huống A–D trên seed đánh giá độc lập; khác biệt xu hướng và mục tiêu phải đo được, không chứng minh bằng một đoạn trình diễn thuận lợi. Sau đó kiểm ONNX/native, hiệu năng có inference thật, Windows package/offline/save và trải nghiệm người chơi. Không tuyên bố bản mới hoàn chỉnh bằng số test hoặc reward train.

## 5. Công nghệ và cách khởi đầu

Giữ hướng C++20 OOP/composition cho mô phỏng độc lập engine; Godot cho app native; Python gọi core để học/đánh giá; ONNX CPU là hướng runtime cần kiểm tương thích. Không tự đổi thành web game. Phiên bản thư viện, kiến trúc mạng, thuật toán tối ưu cuối cùng và số chiều đầu vào chưa bị khóa bởi bản cũ.

Chỉ đạo mới nhất đã giao mở session sạch để viết lại luật môi trường, mô hình class/object và bản game trực quan một lãnh thổ. Session đó chỉ mang bộ docs sang workspace mới, tạo manifest nhóm A, cấu hình nguồn lực/đơn vị/phép kiểm, rồi triển khai một chuỗi nhỏ hoàn chỉnh trên nền tổ chức code mới. Không tiếp tục vá Realm cũ hoặc chờ cuối gameplay mới sửa kiến trúc. Lệnh xóa project cũ đang bị công cụ chặn; không dùng việc đó làm lý do tiếp tục source cũ hoặc né cơ chế chặn xóa.

## 6. Kiểm soát scope

Trước khi thêm một chỉ số, nhóm hàng hoặc cách hành động, phải trả lời: nó tạo lựa chọn kinh tế/chính trị khác ở đâu; dữ liệu nào là thật; tác động chính là gì; kiểm được bằng chuỗi nào; có thể biểu diễn bằng tham số của cơ chế hiện có không? Nếu chưa trả lời, ghi câu hỏi thay vì tự mở feature.

A–D là các quy mô kiểm chứng, không là các game khác nhau. Không copy tất cả ngành nghề, đồ dùng hay con số cũ chỉ để đạt đủ catalog. Các hạn chế chưa xử lý phải ghi rõ trong CURRENT_TASK; mở rộng độ sâu hoặc ngân sách sản phẩm cần quyết định mới.

## Triển khai nhóm A trong session mới
Bản trial-a2 bổ sung class/action một lãnh thổ theo [luật và catalog](../ACTIONS_A2.md), thay các hạn chế a1 được ghi rõ. Các mốc B–E không đổi.
