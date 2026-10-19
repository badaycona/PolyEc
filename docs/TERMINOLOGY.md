# PolyEc — giải thích thuật ngữ kỹ thuật

Tài liệu tra cứu dành cho người đọc thiết kế và kỹ thuật. Trong nội dung chính, ưu tiên diễn đạt tiếng Việt đầy đủ; giữ tên tiếng Anh trong ngoặc khi cần đối chiếu với mã nguồn hoặc tài liệu chuyên ngành. Bảng này giúp tra cứu, không thay phần giải thích tại nơi thuật ngữ được sử dụng.

## Mô phỏng và dữ liệu

| Thuật ngữ | Nghĩa trong dự án và ví dụ |
|---|---|
| Lõi mô phỏng (core) | Phần C++ thực hiện luật của thế giới. Giao diện và chương trình huấn luyện đều yêu cầu phần này xử lý hành động. |
| Trạng thái (state) | Dữ liệu mô tả thế giới tại một thời điểm: người, tiền, hàng, thời gian, nghĩa vụ và trạng thái bộ sinh số ngẫu nhiên. |
| Khả năng tự lựa chọn (agency) | Nhân vật chọn hành động dựa trên hoàn cảnh và mục tiêu riêng. Có mã định danh hay nhiều chỉ số chưa đủ chứng minh điều này. |
| Lệnh (command), hành động (action) | Hành động là lựa chọn như mua hàng; lệnh là yêu cầu cụ thể gửi tới lõi, có người thực hiện, đối tượng và tham số. |
| Giao dịch nguyên tử (atomic transaction) | Các thay đổi liên quan được ghi nhận trọn vẹn hoặc không ghi nhận gì. Không trừ tiền người mua rồi để người bán không nhận được tiền vì một lỗi giữa chừng. |
| Điều kiện luôn phải đúng (invariant) | Quy tắc cần giữ sau mọi thay đổi, chẳng hạn tổng tiền chỉ đổi khi có nguồn tạo hoặc đích tiêu hủy được khai báo. |
| Từ chối (reject) và ghi nhận (commit) | Từ chối nghĩa yêu cầu không hợp lệ và không đổi trạng thái. Ghi nhận nghĩa thay đổi đã được áp dụng. Một nỗ lực hợp lệ có thể thất bại nhưng vẫn tốn chi phí nếu luật đã quy định; đó không phải từ chối lệnh. |
| Khả năng chú ý (attention) | Nguồn lực giới hạn khi thực hiện hoạt động. Chú ý đã dành cho một việc chưa tự là thời gian có mặt, công sức, sản lượng hay tiền lương. |
| Quyết toán (settlement) | Chốt sản lượng, khoản phải trả và khoản đã trả dựa trên phần việc thực hiện cùng điều khoản hợp đồng. Gọi lại cùng lần quyết toán không được trả tiền hai lần. |
| Tài sản tạm giữ (escrow) | Tiền hoặc hàng đã được dành cho giao dịch đang chờ hoàn tất; chủ tài sản không thể đồng thời chi dùng khoản đó. |
| Sự việc và thông tin đã biết (fact/knowledge) | Một việc đã xảy ra trong thế giới khác với việc ai đó đã biết nó. Báo cáo có thể đến muộn hoặc chỉ phản ánh một phần sự thật. |
| Quan hệ thưa (sparse relations) | Chỉ lưu quan hệ có thật hoặc cần theo dõi, thay vì cấp một ô dữ liệu cho mọi cặp người. |
| Chỉ mục (index) | Dữ liệu phụ giúp tìm nhanh, ví dụ tìm thành viên của một hộ. Nếu là chỉ mục dẫn xuất, phải dựng lại được từ dữ liệu gốc. |
| Tick, nhịp quyết định (cadence) | Tick là đơn vị thời gian nguyên của mô phỏng. Nhịp quyết định nói mỗi người được chọn lại hành động thường xuyên thế nào; một quyết định không nhất thiết tương ứng một tick hay một ngày. |
| Chạy lại (replay) | Đưa cùng trạng thái ban đầu và chuỗi đầu vào vào hệ thống để kiểm tra kết quả có tái hiện được trong phạm vi đã công bố hay không. |

## Học máy và đánh giá

| Thuật ngữ | Nghĩa trong dự án và ví dụ |
|---|---|
| Học tăng cường (reinforcement learning, RL) | Cách huấn luyện mô hình bằng tương tác với môi trường và tín hiệu đánh giá kết quả. Luật thế giới vẫn do lõi mô phỏng thực hiện. |
| Chính sách chọn hành động (policy) | Hàm nhận thông tin nhân vật có và đưa ra cách chọn hành động. Trong sản phẩm hoàn chỉnh, quyết định tự chủ dùng chính sách đã học; quy tắc viết tay chỉ dùng làm đối chứng hoặc kiểm thử có nhãn. |
| Quan sát, đặc trưng đầu vào (observation/features) | Các thông tin được mã hóa thành đầu vào cho mô hình, chẳng hạn số dư và giá mà nhân vật được biết. Không được đưa bí mật chưa biết vào chỉ vì lõi đang giữ dữ liệu đó. |
| Hành động ứng viên và mặt nạ hợp lệ (candidates/mask) | Danh sách lựa chọn cụ thể cùng dấu cho biết lựa chọn nào hợp lệ. Mặt nạ chặn vi phạm luật, không tự chặn lựa chọn chỉ vì dự đoán nó không có lợi. |
| Điểm đánh giá kết quả (reward) | Tín hiệu dùng khi học, được tính từ hậu quả của hành động. Điểm này không phải tiền và không tự đại diện cho mọi tiêu chí chất lượng game. |
| Bộ nhớ của mạng (hidden state) | Dữ liệu mạng duy trì qua các quyết định của một nhân vật. Khác với trọng số dùng chung giữa nhiều người; không phải bằng chứng rằng mạng nhớ chính xác mọi sự kiện. |
| Chuỗi trải nghiệm (trajectory/rollout) | Bản ghi các quan sát, lựa chọn và kết quả khi mô hình tương tác với thế giới, dùng để huấn luyện hoặc phân tích. |
| Bản lưu mô hình (checkpoint) | Trọng số mô hình được lưu tại một thời điểm huấn luyện. Có nhiều bản lưu không có nghĩa bản cuối cùng là bản tốt nhất. |
| Tập chọn mô hình (validation set) | Những tình huống dùng để so các bản lưu và chọn mô hình hoặc điều chỉnh cách huấn luyện. |
| Tập kiểm tra độc lập (test holdout) | Những tình huống dành riêng để đánh giá sau khi đã chọn mô hình. Nếu dùng kết quả này để chỉnh mô hình, cần một tập độc lập khác để đánh giá cuối; không tiếp tục gọi tập cũ là chưa từng được dùng để lựa chọn. |
| Phương án đối chứng (baseline) | Phương án dùng để so sánh, ví dụ quy tắc viết tay hoặc mô hình chưa huấn luyện. Cần ghi rõ đang so với loại nào. |
| Kiểm tra tương đương đầu ra ONNX (ONNX parity) | Cho bản PyTorch và bản đã xuất sang ONNX nhận cùng đầu vào, bộ nhớ và điều kiện hợp lệ; so điểm hành động, bộ nhớ tiếp theo và phân phối xác suất theo sai số cho phép. Qua kiểm tra này chỉ chứng minh việc chuyển mô hình giữ được cách tính, chưa chứng minh quyết định trong game có chất lượng. |
| Seed | Giá trị khởi tạo bộ sinh số ngẫu nhiên. Giữ cùng seed giúp tái hiện, nhưng sau khi đổi hành động, số lần lấy mẫu có thể khác nên không bảo đảm mọi sự kiện về sau vẫn khớp. |
| So sánh ghép cặp (paired comparison) | So hai phương án trên các tình huống tương ứng, giữ những điều kiện cần kiểm soát như nhau để đo ảnh hưởng của thay đổi. |
| Thử loại bỏ thành phần (ablation) | Bỏ một đầu vào hoặc cơ chế rồi đo kết quả, nhằm kiểm tra thành phần đó thực sự đóng góp gì. |
| Khoảng tin cậy (confidence interval, CI) | Khoảng ước lượng từ mẫu theo một phương pháp thống kê được nêu rõ. Nó thể hiện độ bất định của phép đo, không phải lời bảo đảm kết quả trên mọi tình huống. |

## Hiệu năng và phát triển

| Thuật ngữ | Nghĩa trong dự án và ví dụ |
|---|---|
| Khối lượng công việc đo thử (workload) | Dân số, số hành động, loại tương tác và thời gian mô phỏng cụ thể dùng để đo. Không so hai kết quả tốc độ nếu khối lượng công việc khác nhau mà không nói rõ. |
| Đo toàn chu kỳ (full-cycle/end-to-end) | Tính cả tìm việc đến hạn, dựng đầu vào, chạy mạng, kiểm lệnh, cập nhật thế giới và đồng bộ giao diện. Chỉ đo chạy mạng không đại diện cho cả game. |
| p95/p99 | Mốc thời gian mà 95%/99% số lần đo không vượt quá. Ví dụ p95 của thao tác là 100 ms nghĩa 95% mẫu mất không quá 100 ms; vẫn có những mẫu chậm hơn. |
| Việc còn chờ (backlog) | Số công việc đã đến hạn nhưng chưa xử lý. Tốc độ hiển thị cao trong khi hàng chờ tăng mãi vẫn là vấn đề. |
| Đặc tả cần tuân theo (contract), điều kiện nghiệm thu (gate) | Đặc tả quy định hành vi, quyền, dữ liệu và trường hợp biên. Điều kiện nghiệm thu quy định cần kiểm gì trước khi kết luận phần việc đạt yêu cầu. |
| Bằng chứng kiểm chứng (evidence/report) | Ghi nhận phiên bản mã, lệnh đã chạy, môi trường, kết quả và giới hạn. Một báo cáo cũ không tự chứng nhận phiên bản mã hiện tại. |

Tên công nghệ như C++, Python, PyTorch, ONNX, Godot và tên API giữ nguyên để tra cứu. Các thuật toán PPO, GAE, GRU và BPTT được giải thích cùng công thức ở phần 6 của [tài liệu kỹ thuật](DEVELOPER_GUIDE.md), không dùng chữ viết tắt để thay cho lời giải thích.
