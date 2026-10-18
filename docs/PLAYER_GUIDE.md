# PolyEc — hướng dẫn người chơi

Cập nhật: 2026-10-18. Hướng dẫn này giải thích bản chơi hiện có và điều người chơi cần biết. Game đang phát triển: bản M4 là màn chơi thử có mục tiêu; sandbox M3 là phòng thử mô phỏng. Quốc gia và luật xã hội đầy đủ vẫn đang được xây.

## Bạn vào vai ai?

Bạn là vua của một quốc gia giả tưởng lấy cảm hứng cuối Trung cổ. Tiền, lương thực, lao động và quyền lực đều có giới hạn. Bạn đọc báo cáo, giao việc, cấp nguồn lực và theo dõi kết quả. Có quyền ra lệnh chưa có nghĩa cấp dưới có đủ tiền, người, đường đi hoặc quyền địa phương để thi hành.

Trong hướng sản phẩm đầy đủ, dân cư và các tổ chức có lợi ích riêng và tự xử việc thường lệ. Người chơi không phải điều khiển từng người dân. Bản chơi thử hiện tại mới thể hiện một phần hướng đó.

## Mở bản chơi

Nếu đã có gói Windows, giải nén toàn bộ rồi mở `PolyEc.exe`; giữ các file đi kèm. Gói local có thể nằm tại `dist/PolyEc-M4-Windows`. Clone source từ GitHub không tự cung cấp bản game đã build.

Chọn **M4 · Đường đèo & nguồn sống** để chơi lát cắt bản đồ. Chọn **Quốc gia · Sandbox** để quan sát mô phỏng/policy thử nghiệm nếu bản đóng gói có màn này. Các màn phòng thử khác không phải chiến dịch hoàn chỉnh.

App chơi offline, không cần Python, Node hay API key. Gói Windows hiện đã kiểm tra có thể cần Microsoft Visual C++ runtime v14 của hệ thống; xem thông báo đi kèm bản tải nếu máy thiếu. Không tự di chuyển riêng file `.exe` khỏi các thư viện và tài nguyên của gói.

## Điều khiển bản đồ M4

| Thao tác | Kết quả |
|---|---|
| Click vùng | Xem địa thế, kho và hồ sơ của vùng |
| Cuộn chuột | Phóng to/thu nhỏ tại con trỏ |
| Kéo chuột phải hoặc giữa | Di chuyển bản đồ |
| Home | Đưa bản đồ về giữa |
| Space | Chạy/dừng; không kích hoạt khi đang nhập số |
| F1 | Mở hướng dẫn và tạm dừng |
| Esc | Về địa đồ và tạm dừng |

Bản M4 dùng 1× tương ứng một ngày game mỗi giây thực. Khi rời cửa sổ hoặc có khoảng dừng dài, game tạm dừng và không chạy bù. Đóng hướng dẫn không phải tín hiệu tự tiếp tục thời gian. Quan sát trạng thái chạy/dừng trước khi chờ hàng hoặc công trình.

## Bạn có thể làm gì trong M4?

Bản đồ có sáu lãnh địa và đất triều đình, cùng 10.000 người khởi tạo. Bạn có thể xem dự trữ, xử lý quyền đường đèo, xác minh/phân xử, xây đường, cấp lương thực, trả nợ và mở xưởng dệt. Hàng di chuyển theo tuyến và sức tải; công trình có thời gian và có thể mắc điều kiện pháp lý.

Một lượt thử dễ theo dõi là hồ sơ Hắc Sơn:

1. Tiến vài ngày để báo cáo tới nơi, rồi xem thiếu hụt và nghĩa vụ nợ.
2. Giải quyết quyền đường đèo bằng thương lượng hoặc xác minh rồi phân xử. Xem giá, thời gian và hệ quả trước khi chọn.
3. Nếu xây đường, kiểm tra cả tiền, vật tư và quyền. Đường chưa hoàn thành không có tác dụng như đường đã nghiệm thu.
4. Xem tuyến và ngày hàng tới trước khi cấp lương thực. Hàng đang đi chưa nằm trong kho đích; tiền mua và phí vận tải đều tiêu nguồn lực thật.
5. Thanh toán đúng khoản nợ và xem điều kiện đóng hồ sơ. Hắc Sơn được cứu không đồng nghĩa các vùng khác đã đủ dự trữ.
6. Thử xưởng dệt Bạch Khâu để thấy vốn, lao động, nguyên liệu và nhiên liệu liên kết với sản lượng, lương và thu nhập.

Điều kiện đầu kỳ của màn thử được đặt để kiểm tra cơ chế. Game không hứa mọi campaign tương lai sẽ có cùng diễn tiến.

## Những điều quan trọng khi ra quyết định

**Tiền riêng khác tiền công.** Ngân khố, tiền xưởng và tài sản cá nhân không tự dùng thay nhau. Việc thiếu tiền không được giải quyết bằng cách cộng tiền từ không khí.

**Kho còn khác hàng đang vận chuyển.** Đường có công suất; chia nhỏ lô không tạo thêm sức tải. Cấp hàng muộn có thể khiến nợ hoặc thiếu hụt tiếp tục xảy ra trong lúc hàng đang tới.

**Báo cáo có độ trễ.** M4 có thông tin tới sau hai ngày. Trong thiết kế đầy đủ, điều chưa biết hoặc nghi vấn sẽ được phân biệt với bằng chứng; không kết luận đối phương vô tội chỉ vì điều tra chưa tìm thấy gì.

**Lệnh có thể bị từ chối.** Đọc lý do về nguồn lực, quyền, điều kiện hoặc dữ liệu đã thay đổi. Lệnh lỗi không được tiêu một nửa tiền rồi để world dở dang.

**Thành công cần theo dõi.** Khoản cấp vốn, lời chấp thuận và công trình hoàn tất là những thời điểm khác nhau. Hãy theo dõi tiến độ, vật tư, ngày đến và nghĩa vụ còn mở.

## Lưu và tải

M4 lưu world, tiến độ, hàng, quyền đường, hồ sơ và trạng thái ngẫu nhiên. Tải mở ở trạng thái tạm dừng; lưu đè giữ bản trước trong `.bak` ở thư mục user-data Godot. Save sai phiên bản hoặc hỏng phải báo lỗi thay vì thay một phần phiên đang chơi.

Không mặc định save hoặc model của các bản thử cũ tương thích phiên bản mới. Giữ bản sao save quan trọng trước khi đổi gói game.

## Hiểu toàn bộ hướng chơi của PolyEc

Phần dưới mô tả **sản phẩm dự kiến**. Các màn thử hiện có chưa cung cấp đầy đủ những hệ thống này. Tên thẻ, vùng và thông số thử chi tiết nằm trong [kế hoạch toàn dự án](IMPLEMENTATION_PLAN.md); ở đây giải thích chúng có nghĩa gì khi chơi.

### Một đời vua và cách đánh giá kết quả

Bạn theo một triều đại trong đời một vị vua. Cái chết, bị phế hoặc mất chính quyền trung ương kết thúc lượt chơi; một vùng ly khai chưa chắc kết thúc tất cả. Báo cáo cuối nhìn riêng đời sống dân cư, sản xuất, tài chính, thể chế và cái giá chính trị. Không có một điểm tiền hay lãnh thổ duy nhất thay cho cai trị tốt.

Việc trước mắt và việc lâu dài cạnh tranh nhau. Cấp cứu lương thực giúp vượt tuần tới; đường, trường nghề và cơ chế thu thuế đáng tin có thể giúp nhiều năm sau. Một dự án tăng thu cho triều đình cũng có thể làm nhóm khác mất lợi ích. Bạn cần nhìn cả ai hưởng, ai trả và ai thực hiện.

### Quốc gia, địa hình và đường vận tải

Quốc gia dự kiến gồm 24 lãnh địa và ba khu triều đình. Thanh Điền có thế mạnh lương thực; Hắc Sơn có sắt nhưng cần đường và thực phẩm; Bạch Khâu có len nhưng cần thợ, vốn và nơi tiêu thụ; vùng cảng cần nguồn hàng và an ninh tuyến. Đây là cơ hội ban đầu, không buộc mỗi vùng giữ một nghề mãi mãi.

Vùng sát nhau trên bản đồ chưa chắc vận chuyển thuận tiện. Đèo, cầu, sông, mùa và quyền thu phí ảnh hưởng thời gian, chi phí và sức tải. Bản đồ cho bạn xem các lớp giá, kho, dòng hàng, quyền và rủi ro. Đường vẽ trên màn hình chỉ có tác dụng khi trong thế giới có tuyến và điều kiện sử dụng tương ứng.

### Bạn có quyền đến đâu?

Quỹ triều đình, tài sản riêng của vua, quỹ lãnh địa và tiền của từng người là những khoản khác nhau. Trên đất triều đình bạn quyết định qua bộ máy của mình; với chư hầu, nhiều việc cần căn cứ quyền hạn, nghĩa vụ hoặc thương lượng. Bãi nhiệm một người không tự biến toàn bộ tài sản, đất và quân của họ thành của bạn.

Quan hệ tốt giúp mở cơ hội hợp tác nhưng không bảo đảm trung thành tuyệt đối. Lãnh chúa, thương nhân, guild, gia tộc và cơ quan có lợi ích và cam kết riêng. Một lời hứa có giá trị vì các bên có khả năng và lý do thực hiện, cùng hậu quả nếu vi phạm.

### Người dân sống và làm việc thế nào?

Mỗi người được mô phỏng có đời sống riêng: gia đình, tài sản, công việc, kỹ năng, sức khỏe, hiểu biết và lựa chọn. Hộ có thể chia sẻ quỹ nhưng vào hộ không tự giao hết tiền riêng. Trẻ em có quyền sở hữu và người giám hộ phù hợp; chúng không phải phần kho của người lớn.

Người dân có thể học, làm, nghỉ, chăm người khác và thay đổi kế hoạch. Nhận hai việc không tạo thêm thời gian hay sức khỏe. Có người kiên trì hơn, có người coi trọng gia đình hay ngại rủi ro; giỏi chuyên môn cũng khác giỏi dự báo. Đây là hành vi cần xây dựng và kiểm chứng, chưa phải lời hứa rằng model thử hiện tại đã thể hiện đủ.

Bạn không giải quyết từng đơn xin học hay từng ca làm. Các bên tự xử lý việc trong quyền; vấn đề vượt quyền, ảnh hưởng nghĩa vụ hoặc liên vùng mới cần lên triều đình. Bạn vẫn có thể theo dõi một người hay một hồ sơ để hiểu điều gì đã xảy ra.

### Sản xuất, chợ và nợ

Xưởng cần vốn, người, vật tư, dụng cụ và nơi bán; ruộng cần đất, mùa và lao động. Có tiền mà thiếu thợ hoặc đường thì chưa chắc mở rộng được. Hệ thống đích có 16 nhóm hàng; các loại có công dụng khác nhau, không tự thay thế nhau chỉ vì cùng giá.

Mua hàng phụ thuộc khả năng chi trả và hàng thật. Thương nhân tính chuyện mua bán khi thông tin giá có thể đến muộn; hàng tới nơi chưa chắc còn bán được đúng giá dự kiến. Vay cho bạn tiền hôm nay và nghĩa vụ sau này. Cứu trợ có thể giúp một bên sống sót nhưng làm ngân khố chịu rủi ro và tạo kỳ vọng về lần sau.

### Gia đình, hôn nhân và thừa kế

Luật dự kiến yêu cầu hai người đủ 18 tuổi và chấp thuận khi kết hôn. Giám hộ không đồng ý thay họ. Cưới không tự sáp nhập lãnh địa hay tạo liên minh; tiền góp cho hộ mới phải được chủ tài khoản đồng ý.

Khi một người chết, tài sản riêng, tài sản họ chỉ quản lý hộ, nợ, tước và chức bổ nhiệm được xử lý riêng. Di sản trả nghĩa vụ trước khi chia; tài sản người còn sống không bị gom vào. Tranh chấp có thể giữ lại phần liên quan để xét xử. Người kế vị tước chưa chắc được bổ nhiệm vào chức cũ của người chết.

### Báo cáo, nghi vấn và điều tra

Thông tin có nguồn và ngày cập nhật. Sổ công được xác nhận khác tài sản tư; ước tính khác số đã kiểm kê; nghi vấn khác bằng chứng. Chưa tìm ra bằng chứng không đồng nghĩa chứng minh vô tội. Điều tra cần người, ngân sách và thời gian, có thể chịu xung đột lợi ích.

Bạn thấy điều triều đình có căn cứ biết, không thấy thẳng suy nghĩ và bí mật của mọi NPC. Tin đến trễ có thể làm một quyết định vốn hợp lý trở nên không phù hợp lúc thực hiện. Theo dõi thời điểm báo cáo là một phần của cách chơi.

### Phát triển, quân sự và đối tác ngoài biên

Đường, thủy lợi, kho, giáo dục, y tế và tư pháp tạo năng lực lâu dài, đồng thời cần bảo trì và người vận hành. Phát triển có thể thay cán cân quyền lực: cảng giúp thương nhân mạnh hơn, trường mở cơ hội cho người ngoài gia tộc cũ.

Huy động quân cần người chỉ huy, nghĩa vụ, lương, trang bị và tiếp vận. Người đi lính không vẫn làm đủ việc ở nhà. Ngoại thương có đối tác hữu hạn, tuyến và thuế; không phải cửa hàng mua bán vô tận. Bản đầu hướng tới quyết định nhà nước và tiếp vận, chưa có điều khiển chiến thuật từng binh sĩ.

### Chọn lợi thế đầu triều đại

Thiết kế có 12 augment khởi đầu theo ba cấp. Bạn xem ba thẻ, có một lần đổi một thẻ rồi chọn một. Các hướng gồm kiểm kê/báo cáo, đào tạo, hòa giải, dự trữ, tín dụng, công trình, nhân sự, thư trạm, quỹ hạ tầng, liên lãnh, học viện và cố vấn.

Thẻ cho lợi thế có nguồn gốc rõ: kho đầu kỳ, nhân sự, quy trình hoặc cam kết. Nó không làm đối phương tự đồng ý, không tái tạo quỹ vô hạn và không miễn chi phí vận hành. Các thông số còn cần cân bằng; sandbox pilot có draft không có nghĩa cả bộ hiệu ứng sản phẩm đã hoàn chỉnh.

### Sự kiện phát sinh từ đâu?

Một mùa kém làm kho giảm; thương nhân chuyển hàng khiến tuyến đầy; xưởng thiếu vật tư, trả thiếu lương; người lao động tìm việc, chủ xưởng xin vay, nhóm mất quyền lợi kiến nghị. Chuỗi có thể dừng hoặc rẽ khi các bên quyết định khác. Game hướng tới những chuyện phát sinh như vậy, không bắt buộc cứ đến năm nào đó phải có đảo chính.

Hướng dẫn và màn thử có tình huống đặt trước để dạy/kiểm tra cơ chế. Chúng không đại diện cho việc campaign luôn diễn lại cùng câu chuyện.

## Trạng thái thực tế so với hướng sản phẩm

M4 dùng bộ điều khiển kiểm thử được ghi nhãn để chạy việc thường lệ. Sandbox M3 có mạng đã huấn luyện nhưng vẫn là model thử nghiệm; chưa chứng minh đạt toàn bộ chất lượng quyết định.

Hộ động/quỹ chung đầy đủ, cơ sở/nghề, hôn nhân, thừa kế, giáo hội, quốc gia 27 khu/16 nhóm hàng và hành vi học đủ sâu đang thuộc công việc M5 trở đi. Bộ tính attention hiện tại là nền kỹ thuật; người chơi chưa nên hiểu nó như hệ làm/nghỉ/chăm sóc đã hoàn chỉnh.

Muốn theo dõi tiến độ đọc [lịch sử phát triển](DEVELOPMENT_HISTORY.md). Người muốn build/mod/debug đọc [tài liệu dev](DEVELOPER_GUIDE.md). Những README theo bản đóng gói trong `guides/` là bản hướng dẫn cố định cho gói tương ứng; không phải một bộ yêu cầu mới.

| Bạn tìm điều gì? | Đọc/xem ở đâu? |
|---|---|
| Chơi ngay bản M4 | Các mục mở bản chơi, điều khiển, Hắc Sơn và save ở đầu guide này |
| Tính năng nào đã có bằng chứng? | Reports M3/M4 và lịch sử; phân biệt sandbox với campaign hoàn chỉnh |
| Toàn bộ thiết kế vùng, luật, thẻ và roadmap | IMPLEMENTATION_PLAN, gồm phần đặc tả chi tiết |
| Điều đang triển khai và chưa xong | CURRENT_TASK; không coi một file mới xuất hiện là tính năng đã nghiệm thu |
