# Truy vấn, sắp xếp và dữ liệu tra cứu trong core PolyEc

Tài liệu thiết kế ngày 2026-10-19. Đọc cùng [hồ sơ từng class](core-class-reference.md) và [hướng tổ chức lại core](core-architecture-refactor.md). Công việc được giao và nghiệm thu vẫn theo [CURRENT_TASK](../CURRENT_TASK.md).

Đây là hướng kỹ thuật để phát triển từng bước, không phải tuyên bố mọi thuật toán đã được triển khai. Các mục “đề xuất” cần khảo sát và review trong lát tương ứng trước khi sửa code. Không thay luật chọn việc, mức lương, quyền xem dữ liệu hoặc thứ tự phân phối đã duyệt.

## 1. Nên sửa từng phần hay viết lại core?

**Khuyến nghị: thay từng phần của core, theo một thiết kế chung.** Mỗi phần được thay phải có đầu vào, đầu ra, người giữ dữ liệu và phép kiểm rõ ràng; sau khi nghiệm thu thì bỏ đường xử lý cũ của phần đó.

Lý do là một thế giới mô phỏng có nhiều luật liên quan nhau. Chuyển thời điểm trả lương, đổi thứ tự duyệt người hoặc làm tròn khác một đơn vị cũng có thể thay kết quả. Viết lại toàn bộ cùng lúc sẽ khiến khó biết sai lệch đến từ đâu, đồng thời phải giải quyết lại save, UI, Python và model.

Cách làm cụ thể:

1. Ghi rõ hành vi đang được giữ và hành vi cần sửa. Lỗi cũ không trở thành luật chỉ vì nó đang có trong source.
2. Chọn một công việc trọn vẹn, chẳng hạn trả lương hoặc nhận/thôi việc, thay vì chỉ chuyển vài hàm theo tên file.
3. Tách phần tính kết quả khỏi phần ghi thay đổi, xác định dữ liệu đọc và viết.
4. Đối chiếu với ví dụ nhỏ có kết quả tính tay, trường hợp biên và kiểm tra tiền/hàng/quan hệ.
5. Chuyển caller thật sang đường mới, bỏ logic cũ tương ứng; không duy trì hai bộ luật chạy độc lập.
6. Sau khi đúng mới tối ưu chỗ đo được là chậm.

Vẫn có thể viết lại hoàn toàn một bộ phận nhỏ, ví dụ bảng tra cứu hộ gia đình. Điều kiện là ranh giới đủ rõ, có cách dựng lại dữ liệu và kiểm tra đầu ra. Viết lại toàn core chỉ nên xem xét khi khảo sát chứng minh mô hình dữ liệu hiện tại không biểu diễn nổi yêu cầu đã duyệt, hoặc chi phí chuyển từng phần lớn hơn chi phí thay thế có đo đạc. Hiện chưa có bằng chứng đó.

## 2. Giải thích các từ thường gặp

| Từ | Cách hiểu và ví dụ |
|---|---|
| Truy vấn — query | Hỏi dữ liệu một câu: “Những người còn sống trong vùng 2 là ai?” Truy vấn chỉ đọc, không tự chuyển vùng hay tạo việc. |
| Bộ lọc — filter | Điều kiện chọn: còn sống, đúng vùng, có việc còn hiệu lực. Phải ghi rõ các điều kiện được kết hợp thế nào. |
| Sắp xếp — sort | Đặt kết quả theo thứ tự, ví dụ ngày đến tăng dần rồi ID tăng dần. |
| Bảng tra cứu — index | Danh mục giúp tìm nhanh. Giống mục lục sách: chỉ chỗ tới dữ liệu gốc, không phải bản sách thứ hai. |
| Khóa — key | Giá trị dùng để tìm, chẳng hạn Person ID. Một số hộ nối tới nhiều người nên khóa hộ không có nghĩa mỗi hộ chỉ có một người. |
| Khóa tham chiếu | ID ghi trong hồ sơ này để tìm hồ sơ khác. Muốn đúng phải kiểm loại ID, sự tồn tại và hiệu lực khi cần. |
| Cache | Kết quả tính được giữ tạm để dùng lại. Khi dữ liệu liên quan đổi, kết quả có thể hết đúng. |
| Invalidate | Đánh dấu kết quả giữ tạm không còn dùng được; lần đọc sau phải tính lại hoặc cập nhật trước. |
| Invariant | Điều kiện phải giữ đúng ở trạng thái hợp lệ. “Kiểm invariant” là đọc dữ liệu để tìm chỗ vi phạm, không phải tự chữa lỗi. |
| Atomic — nguyên tử | Với phần thay đổi đã cam kết: hoặc thành công đầy đủ, hoặc từ chối và giữ nguyên dữ liệu. Không được trừ tiền rồi báo lỗi trước khi cộng người nhận. |
| Deterministic — tái lập được | Cùng trạng thái, lệnh, seed và quy tắc thứ tự thì có cùng kết quả. Không để thứ tự duyệt bảng băm quyết định ai nhận tiền. |
| Độ phức tạp | Cách lượng việc hoặc bộ nhớ tăng khi dữ liệu nhiều lên. Đây không phải thời gian chạy đã đo. |
| O(1) | Lượng việc xấp xỉ không tăng theo số bản ghi; với bảng băm thường chỉ là trung bình, không phải bảo đảm mọi trường hợp. |
| O(log n) | Tìm bằng cách thu hẹp nhiều lần, như chặt đôi vùng tìm trên danh sách đã sắp. n là số phần tử được tìm. |
| O(n) | Phải xem một lượt n phần tử. Lấy ra k kết quả vẫn cần ít nhất công để xuất k kết quả. |
| O(n log n) | Mức thường gặp khi sắp xếp n phần tử bằng phép so sánh. |
| O(n²) | Khối lượng có thể tăng khoảng bốn lần khi n tăng gấp đôi; thường gặp khi so mọi cặp. |

“Snapshot” là bản chụp của dữ liệu tại một thời điểm. “Revision” là số phiên bản trạng thái. “Read-set” là các giá trị một phép tính đã đọc. Nếu các giá trị ấy đổi trước khi ghi kết quả, cần kiểm lại theo contract; chỉ nhìn ngày game có thể chưa đủ.

## 3. Học từ cơ sở dữ liệu, áp dụng vào lõi C++

Ý tưởng gần cơ sở dữ liệu của người dùng phù hợp ở bốn điểm: có ID rõ, quan hệ rõ, bảng tra cứu và giao dịch giữ tính nhất quán. Điều đó không bắt buộc đưa SQL vào vòng chạy mô phỏng.

State do core sở hữu là dữ liệu chính thức. Các index giữ ID hoặc vị trí tới state; view là bản trình bày chỉ đọc; cache là kết quả có thể bỏ đi và tính lại. Không đặt ba bản tiền trong Person, bảng thống kê và UI rồi cố đồng bộ bằng tay.

Một index mới phải trả lời được:

- Phục vụ câu hỏi nào, trả tất cả hay chỉ một phần, có cần thứ tự không?
- Dữ liệu gốc nằm ở đâu? Khóa có thể trùng không? ID 0 nghĩa gì?
- Những thao tác nào làm nó hết đúng: sinh, chết, chuyển vùng, đổi việc, tải save?
- Ai cập nhật? Nếu cập nhật thất bại, state và index có còn nhất quán không?
- Dựng lại bằng cách nào? So với quét dữ liệu gốc ra sao?
- Tốn thêm bao nhiêu bộ nhớ và công cập nhật? Khi nào lợi hơn quét?

Truy vấn không được bí mật sửa state để “chữa” dữ liệu sai. Một đối tượng query có cache nội bộ phải phân biệt cache với state nghiệp vụ và có cách đồng bộ xác định.

## 4. Tìm một đối tượng bằng ID

**Nhu cầu:** command tìm đúng người, giao dịch tìm người nhận, hồ sơ tìm vùng.

Hiện Person dùng ID liền theo vị trí: người có ID 1 nằm ở phần tử 0. Khi điều kiện này được giữ, kiểm miền ID rồi truy cập mảng là lựa chọn đơn giản, O(1). Phải đối chiếu ID bản ghi khi contract yêu cầu; không chỉ tin vị trí sau khi dữ liệu bị thay hoặc sắp lại.

**Không sắp lại mảng Person gốc để hiển thị người giàu nhất.** Hãy sắp một danh sách ID hoặc vị trí riêng. Việc sắp mảng gốc có thể phá toàn bộ cách tra ID.

Nếu tương lai cần ID thưa hoặc xóa vật lý, phương án là bảng ID → vị trí. Nó tốn thêm bộ nhớ và phải cập nhật khi di chuyển phần tử. Không cần đưa vào trước khi nhu cầu được xác định. Nếu tái sử dụng vị trí, cần cơ chế phân biệt hồ sơ cũ với hồ sơ mới; con trỏ hoặc vị trí cũ không được trở thành người khác.

**Kiểm tra:** ID 0, ID ngoài miền, ID sai loại, bản ghi bị đổi vị trí, thêm người làm vector cấp phát lại, tải save có ID trùng. Không giữ pointer vào vector qua thao tác có thể làm nó cấp phát lại.

## 5. Tìm một nhóm: vùng, hộ, việc làm

### 5.1. Theo vùng

Đề xuất danh sách Person ID theo vùng khi truy vấn lặp lại nhiều. Với số vùng nhỏ, mảng các danh sách thường dễ kiểm soát hơn bảng băm. Trả người còn sống hay mọi hồ sơ phải là hai ý nghĩa được nêu rõ.

Tạo người và chuyển vùng làm đổi membership. Cái chết chỉ làm đổi index nếu index đó quy định chứa riêng người còn sống. Đếm dân không được dùng lẫn hai định nghĩa này; hồ sơ Province hiện đã ghi khác biệt giữa resident count và bản xem Campaign.

Đọc danh sách một vùng cần O(k), với k là số ID của vùng, thay vì quét O(N) người toàn thế giới. Đổi vùng có thể tốn O(k) nếu xóa trong vector giữ thứ tự. Xóa bằng cách đổi chỗ với phần tử cuối nhanh hơn nhưng đổi thứ tự; chỉ dùng khi không ảnh hưởng luật hoặc có bước chuẩn hóa kết quả.

### 5.2. Theo hộ gia đình

Hiện HouseholdIndex dựng bảng nhãn hộ → danh sách ID tăng dần. Một hộ có nhiều thành viên; nhãn hộ không phải ID của chủ hộ. Người chết vẫn có thể nằm trong danh sách; tìm người giám hộ phải kiểm vòng đời và tiêu chí đã duyệt.

Khi dựng lại hàng loạt, có thể thu thập các cặp hộ–người, sắp theo hộ rồi người, kiểm trùng và gom nhóm. Đây là phương án đề xuất để tránh chèn lặp từng phần tử trong đầu vào lộn xộn; không khẳng định nhanh hơn khi chưa benchmark. Bản hiện tại có đường thuận lợi khi ID đã tăng dần, nhưng đầu vào xấu có thể làm chi phí tìm/chèn trong một hộ tăng bậc hai.

Không dùng loại trùng như một cách âm thầm chấp nhận Person ID trùng trong state. Dữ liệu gốc lỗi phải được báo ở lớp kiểm phù hợp.

### 5.3. Theo nơi thuê và việc còn hiệu lực

Đề xuất index bên thuê → các ID người có quan hệ, chỉ khi có truy vấn lặp cần nó. Phải định nghĩa khóa là người thuê, vùng thuê hay hợp đồng; không gộp ba loại số nguyên.

Index theo hợp đồng không thay phép kiểm hợp đồng còn hiệu lực. Deadline có thể qua dù không có command mới, nên tới mốc kết thúc phải cập nhật hoặc truy vấn phải đối chiếu thời gian.

Không lưu thêm một trường “Employed” độc lập mà quên cập nhật cùng hợp đồng. Chọn một nguồn quyết định chính thức, các cách đọc khác phải suy ra hoặc được cập nhật trong cùng thao tác.

**Kiểm tra chung:** chuyển hai lần, thôi việc, bổ nhiệm, người chết, hộ chỉ còn người chưa thành niên, employer mất hiệu lực, reject ở giữa cập nhật. So tập ID từ index với quét state gốc; so thứ tự riêng nếu thứ tự thuộc contract.

## 6. Ghép các hồ sơ mà không so mọi cặp

**Nhu cầu:** ghép người nhận lương với Person; ghép Knowledge với Fact; ghép lịch với việc còn hiệu lực.

Nếu mỗi dòng trong n yêu cầu lại quét m người, lượng việc có thể là O(n × m). Có hai hướng thông dụng:

- Dựng bảng tra khóa cho một phía rồi tra từng dòng phía kia: thường O(n + m), thêm bộ nhớ O(m).
- Nếu hai phía đã sắp theo khóa, đi hai con trỏ: O(n + m); nếu chưa sắp phải tính thêm chi phí sắp.

Không phải quan hệ nào cũng một–một. Một người có thể có nhiều dòng nhận lương. Không được dùng map chỉ giữ một dòng rồi làm mất các dòng trùng hợp lệ. Trái lại, Person ID trùng là lỗi dữ liệu gốc. Phải xác định rõ trùng khóa nghĩa là gì trước khi chọn container.

R1 PayrollPlan hiện dùng bảng tra người nhận để tránh tìm tuyến tính lặp lại và vẫn giữ thứ tự các claim. Đây là một ví dụ thực tế, không phải lý do thay mọi truy vấn bằng unordered_map.

**Kiểm tra:** phía không có bản ghi tương ứng, nhiều dòng cùng người, khóa 0, tổng vượt int64, người chết hoặc quan hệ hết hạn theo nghiệp vụ, thứ tự đầu vào có ý nghĩa. Không đảo claim nếu thứ tự đó ảnh hưởng phần dư tiền.

## 7. Truy vấn lịch theo khoảng thời gian

**Câu hỏi:** những phiên nào giao với khoảng cần xử lý? Với khoảng [begin, end), một phiên giao khoảng khi nó bắt đầu trước end và kết thúc sau begin. Chạm đúng một đầu mà không có độ dài chung thì không giao.

Với ít phiên mỗi người, quét vector là phương án dễ hiểu và dễ chứng minh. Không cần cây khoảng cho mọi NPC. Nếu truy vấn nhiều và lịch lớn, đề xuất giữ danh sách theo begin, dùng tìm nhị phân để giới hạn phần bắt đầu trước end, rồi kiểm từng end. Chỉ sắp theo begin vẫn có thể phải đọc nhiều phiên cũ kéo dài; không được quảng cáo là O(log n + k) trong mọi trường hợp.

Khi số liệu chứng minh cần thêm, cân nhắc cây khoảng có thông tin end lớn nhất hoặc bucket theo thời gian. Cây phức tạp hơn khi thêm/xóa; bucket phải xử lý phiên vượt nhiều bucket và không trả một phiên nhiều lần.

### Đánh giá phần thời gian chồng nhau

Hiện activity evaluator lấy các mốc đầu/cuối, chia thành đoạn, quét các phiên để tìm phiên hoạt động trong từng đoạn rồi sắp theo priority và ID. Đây là code dễ làm đối chứng, nhưng phần tìm và sắp lặp có thể tới O(s² log s) khi một người có s phiên, chưa phải chi phí nhân trên toàn dân.

Hướng tối ưu tương lai là **quét sự kiện theo thời gian**: sắp mốc bắt đầu/kết thúc, giữ tập phiên đang hoạt động, xử lý từng đoạn có tập không đổi. Cùng một tick cần áp dụng kết thúc/bắt đầu theo cách giữ đúng khoảng nửa mở. Không tính một tick mới chỉ vì hai sự kiện có cùng thời điểm.

Việc sắp sự kiện thường O(s log s); tổng chi phí phân bổ còn phụ thuộc số phiên hoạt động phải xem trên từng đoạn. Không hứa toàn evaluator O(s log s) khi luật phân bổ vẫn cần xem nhiều phiên chồng nhau.

**Không được đổi luật hòa ưu tiên.** ID có thể ổn định thứ tự đầu ra, nhưng không được biến thành quyền được hưởng thời gian trước nếu luật hiện hành chia nhóm hòa như nhau.

**Kiểm tra:** hoán vị đầu vào, phiên chạm đầu, lồng nhau, trùng ID, cùng tick, priority cực trị, chia khoảng xử lý, tràn tích thời lượng × tốc độ, phiên nghỉ hưu, hủy trước bắt đầu. Kết quả phải khớp evaluator đối chứng và quy tắc phân bổ đã duyệt.

## 8. Các việc đến hạn: dùng hàng đợi thời gian đúng chỗ

**Nhu cầu:** tìm khoản vay đến hạn, chuyến hàng tới nơi, hợp đồng hết hạn và công việc hẹn trong tương lai.

Khi số hồ sơ lớn và phần đến hạn mỗi tick ít, đề xuất hàng đợi ưu tiên nhỏ nhất, còn gọi là min-heap. Phần tử gần hạn nhất nằm đầu. Mỗi lần thêm/lấy thường O(log q), q là số phần tử trong heap; không phải quét mọi hồ sơ mỗi tick.

Khóa nên là thời điểm, thứ tự loại sự kiện đã được duyệt, rồi ID ổn định. Không tự đặt thứ tự nghiệp vụ mới cho các việc cùng thời điểm.

Hủy hoặc dời hạn cần số phiên bản của lịch hẹn, hoặc cơ chế xóa/cập nhật có quản lý. Khi lấy một mục cũ khỏi heap phải đối chiếu bản ghi chính thức. Đề xuất bỏ qua mục hết hiệu lực nhưng phải giới hạn và đo lượng mục rác; nếu chỉ thêm mà không dọn thì bộ nhớ vẫn tăng.

Heap giúp tìm việc đến hạn; không quyết định số lần áp dụng. Cần dấu hoàn tất hoặc cơ chế nhận diện để cùng một việc không thanh toán hai lần. Tải save phải dựng lại từ hồ sơ còn hiệu lực hoặc tải hàng đợi được kiểm tra đúng; không tạo hai lịch hẹn cho một nghĩa vụ.

Không đưa mọi bước kinh tế hằng ngày vào heap chỉ vì có heap. Các công thức phụ thuộc toàn dân mỗi ngày vẫn có thể cần một lượt xử lý. Pause và ngủ máy không tạo quyền chạy bù.

## 9. Quan hệ xã hội và kiến thức: dữ liệu thưa

Đề xuất danh sách kề: mỗi người có danh sách ID quan hệ liên quan. Tìm hàng xóm trực tiếp cần đọc theo số quan hệ của người đó, không quét mọi người. Tổng bộ nhớ phụ thuộc số quan hệ E cộng số người N, không phải N².

Cần xác định quan hệ có hướng hay hai chiều, có cho nhiều loại giữa cùng hai người không, có self-edge không. Không loại self-edge hiện có hoặc gộp quan hệ chỉ vì thấy lạ; phải đối chiếu contract. Nếu một quan hệ cần xuất hiện ở hai danh sách kề, chỉ một bản ghi chính thức giữ nội dung, hai danh sách giữ ID.

Knowledge cần trả lời “người này đã biết sự kiện này ở thời điểm game này chưa?”. Index observer → fact → thời điểm đến có thể hỗ trợ; không được bỏ điều kiện thời gian. Fact bị cắt khỏi lịch sử thì chính sách giữ/xóa tham chiếu phải rõ. Không tự coi thiếu sự kiện lịch sử là một sự kiện mới, cũng không yêu cầu mọi parent cũ còn tồn tại khi thiết kế cho phép cắt lịch sử.

Duyệt nhiều bước trong đồ thị cần tập đã thăm để tránh vòng lặp và giới hạn truy vấn theo mục đích. BFS phù hợp tìm số cạnh ít nhất trên đồ thị không trọng số; nó không tự suy ra quan hệ xã hội “tốt nhất” hay quyết định chiến lược cho NPC.

## 10. Đường đi và đặt chỗ vận tải

Với bản đồ Campaign hiện nhỏ, tìm đường bằng cách đơn giản có thể đủ. Chọn thuật toán theo nghĩa của chi phí: BFS khi mọi cạnh có cùng chi phí; Dijkstra khi chi phí cộng dồn không âm. A* chỉ dùng khi có hàm ước lượng không vượt chi phí thực phù hợp với mô hình; không tự dùng khoảng cách hình học khi thời gian đi không theo hình học đó.

Có sức chứa và lịch đặt chỗ thì “đường ngắn nhất theo số ngày mỗi cạnh” chưa đủ. Cần tính thời điểm sớm nhất có thể vào từng chặng, thời gian chờ, lượng hàng và hai chiều dùng chung tuyến. Với thuật toán gán nhãn thời gian đến, phải chứng minh tính chất: xuất phát muộn hơn không thể tới sớm hơn khi được phép chờ. Nếu không có tính chất này, không áp dụng máy móc Dijkstra thông thường.

Đề xuất danh sách đặt chỗ mỗi tuyến được sắp theo đầu khoảng, tìm khe phù hợp. Phải quy định đầu/cuối khoảng được tính thế nào theo contract vận tải riêng; không tự lấy dấu so sánh của activity.

Preview chỉ đọc. Giữa preview và lệnh thật, tuyến có thể đổi hoặc chỗ bị lấy. Lệnh thật phải kiểm lại và đặt toàn bộ các chặng theo cùng nguyên tắc thành công đầy đủ hoặc giữ nguyên, gồm cả tiền và hàng.

**Kiểm tra:** không có đường, cùng nguồn–đích, tuyến đóng, đủ đúng sức chứa, hai chiều, nhiều chặng, dời ngày, tràn giờ đến, hòa đường đi, preview cũ, reject sau chặng đầu. Phép kiểm bảo toàn phải tính cả hàng đang vận chuyển.

## 11. Sắp xếp bảng, lấy nhóm đứng đầu và phân trang

**Nhu cầu:** hiển thị danh sách người hoặc hồ sơ có thứ tự. Không dùng xếp hạng UI để tự quyết định người được thuê, được học hoặc nhận hàng.

Sắp toàn bộ n kết quả thường O(n log n). Nếu chỉ cần k người đứng đầu và k nhỏ, có thể dùng heap kích thước k, thường O(n log k), rồi sắp k kết quả để hiển thị. Dùng nth_element cũng cần xử lý hòa và sắp phần kết quả; thứ tự phần còn lại không có nghĩa.

Mọi thứ tự cần khóa đầy đủ, ví dụ tiền giảm dần rồi ID tăng dần. So sánh trực tiếp, không lấy hiệu hai số để quyết định thứ tự vì có thể tràn số. stable_sort chỉ giữ thứ tự hòa của đầu vào; nếu đầu vào lấy từ bảng băm thì chưa đủ bảo đảm tái lập.

Phân trang theo vị trí có thể bỏ sót hoặc lặp người khi dữ liệu đổi giữa hai lần xem. Nếu cần duyệt nhất quán, dùng snapshot, hoặc con trỏ trang gồm revision và khóa cuối cùng; khi revision hết hợp lệ phải có cách báo/tải lại rõ ràng. Snapshot lớn cũng tốn bộ nhớ, nên không chụp cả thế giới cho mọi bảng.

Với chỉ số thực như tọa độ hoặc score, phải xác định cách xử lý NaN và giá trị vô hạn trước khi sắp. Tiền trong core vẫn là số nguyên theo đơn vị đã chốt, không đổi sang float cho tiện sắp.

## 12. Tổng hợp số liệu và giữ kết quả đã tính

**Nhu cầu:** tổng dân sống, tiền theo vùng, số việc còn hiệu lực, tồn kho và tổng khoản phải trả.

Một lượt quét O(N) thường phù hợp để dựng hoặc kiểm tra. Nếu phải hỏi cùng tổng nhiều lần mỗi khung hình, có thể giữ bộ đếm dẫn xuất và cập nhật theo thay đổi đã chốt. Mọi đường sinh, chết, chuyển vùng, tải save và rollback liên quan phải được liệt kê.

Tổng tiền cần số nguyên đủ rộng và phép cộng có kiểm tràn. Tổng thuế đã thu là một bộ đếm lịch sử, không phải ví chứa thêm tiền. Không cộng nó lần nữa vào tài sản.

Cache phải có khóa đủ nghĩa: vùng, thời điểm, người xem/quyền xem và phiên bản dữ liệu liên quan. Hai người chơi có quyền biết khác nhau không được dùng chung kết quả chứa thông tin bị giấu. Ngày không đổi nhưng vừa chuyển tiền thì cache chỉ theo ngày đã cũ.

Chưa có cơ chế phiên bản chung cho mọi phần thì không giả vờ rằng cache tự hợp lệ. Có thể bắt đầu bằng tính lại ở điểm công bố hoặc đánh dấu bẩn trong các đường cập nhật đã kiểm soát; sau đó mới chia phiên bản theo vùng/domain để giảm việc tính lại.

## 13. Tạo, cập nhật và kiểm chứng index

Đây là yêu cầu chung cho mọi bảng tra cứu đề xuất:

1. Đọc state chính thức tại một mốc nhất quán, dựng index tạm.
2. Kiểm ID, tập kết quả, thứ tự và quan hệ theo contract; chỉ thay index đang dùng sau khi dựng thành công.
3. Với cập nhật tăng dần, xác định trước mọi thao tác có thể lỗi, kể cả cấp phát container. Không sửa state xong rồi phát hiện index không thêm được mà để hai bên lệch.
4. Khi transaction reject, cả state và dữ liệu tra cứu phải giữ nhất quán. Đánh dấu index không dùng được là một phương án chỉ khi mọi reader đều thực sự bị chặn hoặc chuyển sang đường đọc đúng.
5. Sau tải save, dựng lại các index dẫn xuất và đối chiếu trước khi công bố thế giới. Không cần lưu một bản sao index chỉ vì nó tồn tại trong RAM.
6. Có đường kiểm tra bằng quét state và đường dựng lại phục vụ phát hiện lỗi. Không âm thầm tự chữa lỗi để bài kiểm thử thành xanh.

Index không được chứa con trỏ bị mất hiệu lực khi vector nở. Giữ ID và tra lại thường dễ quản lý hơn; nếu dùng vị trí phải có quy tắc ổn định và cập nhật rõ ràng.

## 14. Kiểm tra đúng và đo hiệu quả

Mỗi thuật toán tối ưu cần một cách đối chứng đơn giản: quét state, sắp toàn bộ hoặc tìm đường trên đồ thị nhỏ. Cách đối chứng phải có cả ví dụ tính tay, vì hai hàm sao chép cùng một lỗi vẫn có thể cho kết quả giống nhau.

Các nhóm kiểm cần thiết:

- Đầu vào rỗng, một phần tử, trùng khóa, khóa không tồn tại, giá trị cực trị và khoảng chạm đầu.
- Thêm, đổi, xóa hoặc kết thúc quan hệ; reject; lưu/tải rồi hỏi lại.
- Truy vấn qua index cho cùng tập như quét state. Khi contract có thứ tự, phải so cả thứ tự.
- Cùng seed và chuỗi command cho cùng kết quả, dù container được cấp dung lượng khác. Không hoán vị dữ liệu mà thứ tự có ý nghĩa nghiệp vụ để đòi đầu ra bất biến.
- Sau từng chuyển đổi mẫu, dữ liệu gốc, index, tổng tiền/hàng và hồ sơ liên quan đều giữ điều kiện đã chốt.

Đo riêng công dựng index, thời gian hỏi, thời gian cập nhật, bộ nhớ và độ trễ xấu. Dữ liệu cần có hộ rất đông, nhiều phiên chồng nhau, người tập trung một vùng và nhiều mục hết hiệu lực, không chỉ phân bố đều đẹp mắt.

Các mốc 10 nghìn, 100 nghìn, một triệu bản ghi là nhóm thử hiệu năng đề xuất, không phải tuyên bố benchmark đã pass hoặc số NPC đầy đủ đã chạy được. Đo trên Windows theo phạm vi hiện hành, ghi máy, cấu hình build, seed, loại bản ghi và số lần lặp. Không dùng spike Citizen đơn giản làm bằng chứng triệu Person với đủ gameplay.

## 15. Thứ tự đưa vào project

Đây là thứ tự phụ thuộc kỹ thuật để tham khảo khi lập lát, không thay roadmap hay tự giao việc cho OpenCode.

| Ưu tiên | Việc thiết kế/triển khai khi được giao | Lý do |
|---|---|---|
| Trước hết | Nguồn dữ liệu chính thức, ID, quan hệ và phép kiểm | Tìm nhanh dữ liệu sai vẫn cho kết quả sai. |
| Kế tiếp | Tra ID, nhóm theo hộ/vùng và quan hệ việc làm đang cần | Phục vụ trực tiếp các domain hiện tại; có quét đơn giản để đối chứng. |
| Theo nhu cầu thật | Ghép lương, lịch theo người, tổng hợp cho view | Tránh quét lặp, giữ nguyên luật phân bổ. |
| Khi số liệu cho thấy cần | Quét sự kiện lịch, heap đến hạn, đặt chỗ tuyến, top-k | Cần benchmark và kiểm trường hợp biên trước khi tăng độ phức tạp. |
| Về sau, nếu có yêu cầu | Cây khoảng, phiên bản cache chi tiết, xử lý song song | Có chi phí bộ nhớ/đồng bộ; chưa xây trước khi biết tải thực. |

Thiết kế sẵn cho tương lai nghĩa là chốt ý nghĩa đầu vào/đầu ra, ownership, thứ tự, điều kiện hiệu lực và cách kiểm tra. Nó không có nghĩa xây sẵn mọi cấu trúc dữ liệu có thể cần trong tương lai. Giữ API đủ rõ để có thể thay thuật toán bên trong mà không thay luật chơi là điều quan trọng nhất.
