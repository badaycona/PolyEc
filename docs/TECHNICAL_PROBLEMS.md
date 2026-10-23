# PolyEc — cây bài toán kỹ thuật

Tài liệu giúp người đọc hiểu **cần giải bài toán nào, chia nhỏ ra sao và kiểm kết quả thế nào** trước khi đọc mã nguồn. Yêu cầu của game nằm trong [kế hoạch sản phẩm](IMPLEMENTATION_PLAN.md); công thức và thuật toán chi tiết ở [tài liệu kỹ thuật](DEVELOPER_GUIDE.md). Tình trạng công việc nằm trong [CURRENT_TASK](CURRENT_TASK.md). Tên tiếng Anh được giải thích trong [bảng thuật ngữ](TERMINOLOGY.md).

P1–P16 dùng cùng mã với tài liệu kỹ thuật để dễ đối chiếu. Công thức dưới đây mô tả phương pháp và điều cần kiểm tra, không tự xác định tham số chưa được duyệt như tốc độ mệt, điểm thưởng hay cách biểu diễn năng lực. Mỗi mục đi từ vấn đề lớn tới những việc nhỏ có thể kiểm tra riêng.

### Cách đọc mã hạng mục

Mã là địa chỉ để tìm đúng phần trong kế hoạch hoặc cây bài toán; mã không tự cho biết phần đó đã xong hay còn thiếu. Trạng thái phải được ghi bằng lời riêng, chẳng hạn “đã triển khai và kiểm tra”, “đang làm” hoặc “chưa bắt đầu”.

- `P1` là bài toán lớn thứ nhất trong cây: danh tính, quyền sở hữu và quyền hành động. `P1.1` là việc nhỏ thứ nhất thuộc P1; `P1.2` là việc nhỏ thứ hai. Nếu tài liệu chia nhỏ thêm, nó nối thêm một cấp số, chẳng hạn `P1.1.1`.
- `M5` là milestone sản phẩm về mô phỏng xã hội và quốc gia. `M5.3` và `M5.4` là các phần của roadmap: M5.3 bao gồm tổ chức/chức vụ, hôn nhân/escrow, thừa kế/nợ/claim và giáo hội; M5.4 bao gồm quốc gia, thành thị, quân sự, ngoại thương và giao diện vua. Nhãn M5.4 có thể xuất hiện trên nhiều nhóm công việc, nên cần nêu cả tên nhóm cụ thể và trạng thái.
- `R1`–`R4` được dùng trong hai ngữ cảnh khác nhau. Trong tài liệu **core refactor**, R1 là tách tính lương, R2 là chuyển đổi việc làm/chức vụ, R3 là điều phối tiến thời gian, R4 là dữ liệu, save/load và thông tin hiển thị. Trong **quality gates của policy RL**, R1–R5 là điều kiện đánh giá policy đã học. Vì vậy, luôn ghi `core refactor R1` hoặc `RL quality gate R1`; không dùng riêng “R1 đã xong”.

Ví dụ: “P1.1 mô tả yêu cầu giữ nguyên ID của một người khi họ đổi hộ hoặc nghề; mục này đang được kiểm trong nhóm M5.0/1” giải thích lần lượt mã bài toán, hành vi cần làm và vị trí roadmap. Kết luận hoàn thành phải dẫn tới CURRENT_TASK hoặc báo cáo có phép kiểm tương ứng.

## 1. Sáu bài toán lớn và phụ thuộc

| Nhóm | Câu hỏi | Bài toán con | Đọc sâu trong tài liệu kỹ thuật |
|---|---|---|---|
| A. Thế giới đúng luật | Ai có gì, ai được làm gì, có mất/nhân tài sản không? | P1–P3 | Phần 2–3 |
| B. Hoạt động có thời gian | Hứa nhiều việc thì thực sự làm được bao nhiêu? | P4–P7 | Phần 4 |
| C. Hậu quả truyền đi | Hàng và tin đi đâu, đến khi nào, ai biết? | P8–P9 | Phần 5 |
| D. NPC tự quyết định | Thấy gì, chọn gì, học từ kết quả thế nào? | P10–P13 | Phần 6 |
| E. Chạy lớn và tiếp tục được | Có đủ nhanh và tải save đúng không? | P14–P15 | Phần 7–8 |
| F. Chứng minh và đưa tới người chơi | Luật đúng, AI có ích, app dùng được chưa? | P16 và tích hợp app | Phần 9, 11 |

Nhóm A xác định dữ liệu và quyền; B thực hiện công việc theo thời gian; C truyền hàng, thông tin và hậu quả; D chọn hành động tiếp theo và gửi lệnh về A. E bảo đảm cả vòng chạy đủ nhanh và có thể lưu lại; F kiểm kết quả. Thử sớm một ý tưởng không thay việc kiểm các phần mà nó phụ thuộc trước khi nghiệm thu.

Ký hiệu: $$S$$ là toàn trạng thái, gồm cả trạng thái bộ sinh số ngẫu nhiên; $$c$$ là lệnh; $$\tau$$ là thời gian trong game; $$t$$ là số thứ tự quyết định học tăng cường, không nhất thiết một ngày. Tiền, hàng, lượng chú ý và điểm đánh giá kết quả khác đơn vị. Mỗi tác vụ chỉ nên giải một phần nhỏ của các nhóm dưới đây.

## 2. A — biểu diễn thế giới và giữ luật đúng

### P1. Danh tính, quyền sở hữu và quyền hành động

**Đầu vào → đầu ra:** Mã định danh, quan hệ và tư cách → người hoặc tài khoản được phép tác động. Quan bị bãi nhiệm mất quyền ký cho cơ quan nhưng giữ tiền riêng.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P1.1 Giữ danh tính khi đổi hộ, nghề hoặc chức | Mã định danh không đổi; lưu vai trò và thành viên hộ thành quan hệ riêng | Một người không bị tạo lại hoặc nhân đôi tài sản khi đổi vai |
| P1.2 Tách quyền sở hữu và quyền đại diện | Ghi riêng ai sở hữu tài sản và ai được phép quản lý | Người giám hộ không được chi ngoài quyền hạn hoặc nhận tài sản trẻ làm của mình |
| P1.3 Tìm và kiểm tra quan hệ | Chỉ lưu quan hệ cần thiết; dựng chỉ mục tìm kiếm từ dữ liệu gốc | Dựng lại chỉ mục sau khi tải game cho cùng kết quả; xử lý rõ người đã chết hoặc không tồn tại |

Hàm $$\operatorname{CanAct}(\mathrm{actor},\mathrm{capacity},\mathrm{target},\mathrm{operation},\tau)$$ trả lời ai được làm thao tác gì, trong tư cách nào, lên đối tượng nào, vào thời điểm nào. Câu trả lời dựa trên luật, không do mô hình học máy dự đoán. Với $$N$$ người và $$E$$ quan hệ, mục tiêu là lượng dữ liệu tăng theo $$O(N+E)$$, thay vì cấp ô cho mọi cặp trong bảng $$N\times N$$. Mã nguồn liên quan: `core/include/polyec`, `core/src/household_index.cpp`; giải thích sâu ở phần 2.3.

### P2. Thực hiện lệnh trọn vẹn hoặc không thay đổi gì

**Đầu vào → đầu ra:** $$(S,c)$$ → từ chối và giữ nguyên $$S$$, hoặc ghi nhận đầy đủ thay đổi. Nỗ lực hợp lệ nhưng thất bại có thể tốn chi phí theo luật; đó khác với từ chối lệnh.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P2.1 Phát hiện yêu cầu không hợp lệ | Kiểm người, quyền, sự đồng ý, giới hạn tham số và số thứ tự lệnh | Tình huống thử phải chạm đúng điều kiện cần kiểm, không bị một lỗi khác chặn trước |
| P2.2 Cập nhật nhiều tài khoản cùng lúc | Tính trước các thay đổi, kiểm đủ nguồn và giới hạn số nguyên rồi mới ghi nhận | Khi từ chối, tiền, kho, thời gian và dữ liệu điều khiển không đổi ngoài quy định được duyệt |
| P2.3 Xử lý lệnh lặp và cạnh tranh tài sản | Kiểm số thứ tự, phiên bản và nguồn lực tại lúc ghi nhận | Lệnh gửi lại không thực hiện hai lần; hai lệnh không cùng tiêu một khoản |

Quan hệ:

$$
\neg\operatorname{Valid}(S,c)\;\Longrightarrow\;T(S,c)=S.
$$

So toàn bộ dữ liệu liên quan trước và sau lệnh, hoặc dấu kiểm được tính từ dữ liệu đó. Chỉ kiểm một số dư chưa đủ phát hiện thay đổi sai ở nơi khác. Giao diện và AI phải gửi lệnh qua cùng quy trình. Xem phần 3.1 của tài liệu kỹ thuật.

### P3. Hạch toán tiền, hàng, lương và thuế

**Đầu vào → đầu ra:** Tài khoản, kho, nghĩa vụ và định mức → các khoản tăng giảm có nguồn và đối ứng.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P3.1 Chuyển tiền | Ghi khoản trừ và khoản cộng đối ứng; tính cả tiền tạm giữ | Tổng tiền chỉ đổi bởi nguồn tạo hoặc đích tiêu hủy được khai báo |
| P3.2 Sản xuất, tiêu dùng và vận chuyển | Dùng định mức từng loại hàng; tính cả hàng đang đi | Tiêu đúng nguyên liệu; giao hàng không nhân đôi lượng hàng |
| P3.3 Chia khoản chi thiếu tiền và làm tròn | Phân bổ theo đặc tả, giữ phần dư và kiểm giới hạn số nguyên | Đối chiếu với ví dụ tính tay; qua nhiều kỳ không thất thoát tiền thuế |

Với tổng tiền $$M$$, nguồn $$E_t$$, đích $$Z_t$$:

$$
M_{t+1}-M_t=E_t-Z_t.
$$

Chuyển $$x$$:

$$
\Delta b_u=-x,\qquad\Delta b_v=x.
$$

Dư nợ là nghĩa vụ, không phải một khoản tiền mới. Phần 3.2–3.8 giải thích tồn kho, định mức sản xuất, lương, thuế và ví dụ tính tay. Công thức thiết kế chưa chứng nhận hệ thống ghi sổ M5 đã hoàn chỉnh.

## 3. B — thời gian, hoạt động và kết quả thực

### P4. Tiến thời gian đúng một lần

**Đầu vào → đầu ra:** Thời điểm đã xử lý, khoảng cần tiến và lịch việc → thời điểm mới cùng các việc đã làm.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P4.1 Định nghĩa đơn vị thời gian | Dùng số tick nguyên; khoảng $$[b,e)$$ gồm đầu b nhưng không gồm cuối e | Không nhầm tick với phút hoặc một lượt quyết định của mô hình |
| P4.2 Xử lý thời điểm chuyển giai đoạn | Chia bước tại ranh giới ngày, phiên hoạt động hoặc thời điểm ngừng phiên | Không chạy việc hằng ngày hai lần; chia nhỏ khoảng tiến thời gian vẫn cho kết quả tương đương |
| P4.3 Nối thời gian thực với thời gian game | Bộ điều khiển giữ phần thời gian lẻ; lõi quản lý tạm dừng và lịch game | Rời cửa sổ hoặc ngủ máy không làm game chạy bù; tải game bỏ phần dư của phiên cũ |

T1 dùng

$$
\operatorname{day}(\tau)=\left\lfloor\frac{\tau}{64}\right\rfloor.
$$

Đây là quy ước đổi tick sang ngày, không phải mô hình sinh lý. Với cùng đầu vào và thứ tự sự kiện, tiến thời gian qua hai bước phải tương đương một bước bằng tổng của chúng:

$$
\operatorname{Advance}(\operatorname{Advance}(S,a),b)=\operatorname{Advance}(S,a+b).
$$

Đẳng thức áp dụng trong điều kiện của đặc tả. Sự kiện giữa khoảng phải được xử lý đúng lúc; không lấy tốc độ cuối khoảng nhân cho toàn khoảng. Xem phần 4.4 và `schedule.cpp`, `realm_clock.cpp`; kế hoạch áp dụng được dẫn từ CURRENT_TASK.

### P5. Phân bổ khả năng chú ý của một người

**Đầu vào → đầu ra:** Phiên hoạt động có thời gian, mức chú ý yêu cầu và ưu tiên → phần được cấp, phần thiếu và phần chưa phân được.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P5.1 Lấy phần thời gian cần tính | Cắt theo khoảng xét, sắp xếp và loại các mốc trùng | Không ghi công cho thời gian nằm ngoài phiên |
| P5.2 Chia khả năng chú ý trên từng đoạn | Xét các phiên đang diễn ra và mức ưu tiên theo đặc tả | Đổi thứ tự đầu vào không đổi kết quả; không tự cho người có mã nhỏ hơn thắng |
| P5.3 Cộng lượng chú ý đã dùng | Nhân mức chú ý với thời lượng; kiểm phép tính số nguyên 64 bit | Không vượt khả năng sẵn có; chia khoảng tính không mất hoặc cộng thêm kết quả |

Trên đoạn $$\Delta\tau$$, khả năng chú ý yêu cầu $$R_j$$ và khả năng chú ý được cấp $$D_j$$ thỏa mãn:

$$
\begin{aligned}
R_j &= d_j\Delta\tau, & D_j &= x_j\Delta\tau,\\
0 &\le x_j\le d_j, & \sum_j x_j &\le C.
\end{aligned}
$$

Bộ tính hiện tại dùng khả năng chú ý tối đa $$C=1000$$. Nhân vật được nhận cam kết vượt khả năng thực hiện, nhưng phần không làm được phải được ghi nhận. Lượng chú ý chưa tự chuyển thành sản phẩm hoặc tiền lương. Phần 4.1 và 4.3 giải thích xử lý các phiên cùng ưu tiên khi không đủ khả năng; mã ở `core/src/activity.cpp`.

### P6. Có mặt, công sức, nghỉ ngơi và cơ thể

**Đầu vào → đầu ra:** Mức chú ý, vị trí, dụng cụ, năng lực và cơ thể → công sức hoặc nghỉ thực tế và cơ thể tiếp theo. P5 chưa chứng minh phần này; cần đặc tả, hiệu chuẩn riêng.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P6.1 Kiểm tra có mặt và làm đồng thời | Xét vị trí, di chuyển và điều kiện hoạt động | Không làm ở hai nơi không thể đến cùng lúc |
| P6.2 Tính công sức từ mức chú ý | Xét loại việc, năng lực, dụng cụ và nguồn lực thực có | Có chú ý nhưng thiếu dụng cụ không tự tạo đủ sản lượng |
| P6.3 Cập nhật tình trạng cơ thể | Tách nhu cầu ngủ, mệt do gắng sức, sức khỏe và hồi phục | Giữ giới hạn trạng thái; chia thời gian nhất quán; tạm dừng không hồi phục theo giờ thực |

Mô hình phân tích:

$$
F_j=\sum_k v_{j,k}\,x_{j,k}\,\eta_{j,k}\,\Delta\tau_k,
$$

$$
B_i(\tau+\Delta\tau)=f_B\!\left(B_i(\tau),F_i,R_i,\mathcal N_i,\Delta\tau\right).
$$

Ở đây $$v_{j,k}$$ cho biết có mặt hợp lệ hay không; $$x_{j,k}$$ là mức chú ý trên một đơn vị thời gian; $$\eta_{j,k}$$ là hiệu quả làm việc trên đoạn $$k$$. Với người $$i$$, $$F_i$$ là công sức tổng hợp, $$R_i$$ là lượng nghỉ thực tế, $$\mathcal N_i$$ là đầu vào dinh dưỡng. Hàm, hệ số, đơn vị và cách làm tròn vẫn cần đặc tả; công thức chưa xác định tuổi thọ, bệnh, chết hay giới hạn làm 8/16 giờ. Xem phần 4.5.

### P7. Quyết toán và nghĩa vụ

**Đầu vào → đầu ra:** Công sức, thời gian có mặt, hợp đồng, tài khoản và kho → sản lượng, lương, nợ và căn cứ ghi nhận.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P7.1 Xác định phần việc đã làm | Lưu lượng tích lũy, thời điểm đã xử lý và nguồn ghi nhận | Hủy phiên hoặc chết không được tính công cho thời gian tương lai |
| P7.2 Tính sản lượng và khoản phải trả | Áp dụng định mức, nguồn thanh toán, quyền và điều khoản | Không trả từ tiền chưa có; lời hứa không tự thành sản phẩm |
| P7.3 Quyết toán đúng một lần | Gắn mã lần quyết toán và lưu phần dư | Tải game hoặc xử lý lại không trả thêm cho cùng phần việc |

$$
\operatorname{Settle}_k\!\left(\operatorname{Settle}_k(S)\right)=\operatorname{Settle}_k(S).
$$

Xử lý lại cùng lần quyết toán mang mã $$k$$ không được làm trạng thái thay đổi thêm. Kiểm lệnh gửi trùng chưa đủ: phải thử lưu rồi tải để bảo đảm không trả lương lại. Phần 4.6 giải thích phương pháp; T2 phải xác định phần việc nào được tính và khi nào ghi sổ.

## 4. C — truyền hàng, thông tin và hậu quả

### P8. Sự việc, thông tin đã biết và chuỗi nguyên nhân

**Đầu vào → đầu ra:** Thay đổi thực trong thế giới → sự việc có nguồn → người nhận tin đúng lúc → hồ sơ và hành động tiếp theo.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P8.1 Ghi sự việc cùng nguyên nhân | Lưu mã sự việc, người liên quan, nơi, lúc và sự việc gây ra nó | Thông báo phải có sự việc thực làm căn cứ |
| P8.2 Truyền thông tin | Ghi người nhận, nguồn, độ tin cậy và thời điểm nhận | Hai người có thể biết khác nhau; không thấy bí mật trước khi có thông tin |
| P8.3 Tổng hợp hồ sơ và hạn chế quá tải | Gom các sự việc nhưng giữ nguồn và quản lý lượng việc chờ | Gộp báo cáo không xóa nghĩa vụ hoặc tiết lộ người gây việc khi chưa biết |

$$
\tau_{\mathrm{received}}\ge\tau_{\mathrm{occurred}}.
$$

Thông tin không đến trước sự việc, nhưng có độ trễ không nghĩa mọi người đều nhận tin. Chạy từ cùng điều kiện đầu rồi thay một can thiệp để xem chuỗi hậu quả thay đổi thế nào. Chuỗi chủ xưởng chết → nợ → thiếu lương là tình huống thử cơ chế, không phải diễn biến bắt buộc của chiến dịch. Xem phần 5 của tài liệu kỹ thuật và phần 14, 21 của kế hoạch sản phẩm.

### P9. Vận tải hữu hạn

**Đầu vào → đầu ra:** Lô hàng, mạng đường, thời gian, sức tải → vận đơn và lịch giao có nguồn hàng thật.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P9.1 Biểu diễn mạng đường | Mỗi nút là địa điểm; mỗi cạnh có thời gian, quyền đi và sức tải | Gần nhau trên bản đồ chưa có nghĩa có đường vận chuyển |
| P9.2 Chọn tuyến và đặt sức tải | Tìm thời điểm đến sớm nhất, tính cả chờ và sức tải đã đặt | Xử lý đúng tuyến đóng, tuyến đầy hoặc thiếu quyền đi |
| P9.3 Giao hàng và đối soát | Theo dõi kho xuất, hàng đang đi và kho đích trong vận đơn được lưu | Chia lô không tăng sức tải; tải game không giao hai lần |

Với lượng hàng $$q$$ và tải mỗi lượt $$c>0$$, cần ít nhất $$\left\lceil q/c\right\rceil$$ lượt. Con số này chưa cho biết ngày đến nếu qua nhiều tuyến hoặc phải chờ. Phần 5.1 giải thích thuật toán. Khi đổi luật chờ, phải kiểm điều kiện đi trước không bị đến sau do hàm thời gian trên cạnh; không mặc định mọi biến thể đều dùng được thuật toán Dijkstra.

## 5. D — học quyết định cá thể

### P10. Thông tin quan sát, các hành động ứng viên và dấu cho biết lựa chọn hợp lệ

**Đầu vào → đầu ra:** Điều nhân vật được biết và cơ hội → dữ liệu đầu vào, danh sách lựa chọn và dấu hợp lệ có phiên bản rõ.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P10.1 Lấy thông tin được phép biết | Dựng đầu vào từ hiểu biết cá nhân và dữ liệu công khai | Đổi một bí mật chưa biết không làm đầu vào nhân vật thay đổi |
| P10.2 Tìm các lựa chọn có thể xét | Dùng chỉ mục để tìm cơ hội và giới hạn số lựa chọn xử lý | Việc cắt danh sách không được ngầm áp một chiến lược viết tay |
| P10.3 Đánh dấu lựa chọn hợp lệ | Kiểm quyền, điều kiện và phiên bản cấu trúc dữ liệu | Không cấm lựa chọn chỉ vì bất lợi; cùng kích thước dữ liệu chưa đủ tương thích |

$$
o_i=\operatorname{Observe}\!\left(K_i,S_{\mathrm{public}}\right).
$$

Mỗi đặc trưng cần đơn vị và cách đưa về thang số phù hợp cho mạng. Bản thử có 128 giá trị trạng thái và 32 hành động ứng viên, mỗi hành động có 12 đặc trưng; đây không phải cấu trúc bắt buộc của M5. Gọi từ Python hay C++ phải hiểu cùng một trường theo cùng ý nghĩa. Xem phần 6.1.3.

### P11. Chọn hành động với bộ nhớ và quá trình lấy mẫu riêng

**Đầu vào → đầu ra:** $$(o_i,h_i)$$ gồm quan sát và bộ nhớ hiện tại → điểm hành động, bộ nhớ mới và lựa chọn hợp lệ.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P11.1 Xử lý đầu vào và bộ nhớ | Mã hóa đầu vào, cập nhật bộ nhớ bằng GRU, chấm điểm từng hành động | Bản PyTorch và ONNX cho điểm và bộ nhớ tương đương trên cùng dữ liệu thử |
| P11.2 Lấy mẫu hành động | Lấy mẫu theo xác suất trong các lựa chọn hợp lệ; mỗi người có bộ đếm riêng | Đổi thứ tự xử lý nhóm không trộn bộ nhớ giữa người |
| P11.3 Quản lý bộ nhớ qua vòng đời | Dùng chung trọng số; đặt lại bộ nhớ theo quy định khi kết thúc lượt hoặc đổi vai | Chết, đổi vai hoặc tải game không dùng nhầm bộ nhớ hay mô hình |

$$
\pi_j=\frac{m_j\exp(\ell_j)}{\sum_k m_k\exp(\ell_k)},
$$

Giá trị $$m_j\in\{0,1\}$$ cho biết hành động hợp lệ hay không. Đặc tả phải quy định trường hợp không có lựa chọn hợp lệ. Nếu mô hình lỗi, không tự dùng chiến lược viết tay để tiếp tục. Phần 6.1.4–6.1.5 giải thích bộ nhớ GRU và cách lấy mẫu hành động theo xác suất.

### P12. Ghi trải nghiệm và học từ kết quả bằng PPO

**Đầu vào → đầu ra:** Chuỗi trải nghiệm mới → trọng số mô hình được cập nhật, có thể đánh giá độc lập.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P12.1 Định nghĩa mục tiêu học | Tính điểm từ kết quả thực theo mục tiêu từng người; ghi phiên bản công thức | Vay tiền không bị coi là kiếm thêm thu nhập; lặp nút không tự tạo thưởng |
| P12.2 Ghi chuỗi trải nghiệm | Lưu người, quan sát, lựa chọn, xác suất, kết quả, thời gian và dấu kết thúc | Không nối hai lượt mô phỏng; hết thời gian lấy mẫu khác với nhân vật chết |
| P12.3 Cập nhật mô hình | Ước lượng lợi ích từ trải nghiệm rồi cập nhật bằng PPO; công thức ở tài liệu kỹ thuật phần 6.3 | Dữ liệu đúng phiên bản mô hình; xử lý giá trị tương lai và ranh giới chuỗi đúng quy định |

Sai số dự đoán giá trị của một bước, gọi là TD, được tính như sau khi các bước có thời lượng đều:

$$
\delta_t=r_t+\gamma(1-d_t)V(s_{t+1})-V(s_t),
$$

Trong đó $$d_t=1$$ khi lượt mô phỏng kết thúc thật và $$d_t=0$$ khi còn tiếp tục. GAE kết hợp sai số TD qua nhiều bước với trọng số $$(\gamma\lambda)^l$$; cách ngắt chuỗi khi kết thúc được giải thích ở phần 6.3. Nếu khoảng cách giữa quyết định không đều, cần quy định cách giảm trọng số kết quả tương lai theo thời gian game. Phần 6.3 cũng giải thích chuẩn hóa điểm thưởng, cập nhật PPO, học qua bộ nhớ và khác biệt đối tác lúc học/lúc chơi. Huấn luyện dài cần ngân sách riêng.

### P13. Năng lực, cá tính và khác biệt hành vi

**Đầu vào → đầu ra:** Yếu tố cá nhân thay đổi có kiểm soát → chênh lệch hành vi và kết quả đo được.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P13.1 Tách nguyên nhân hành vi | Biểu diễn riêng năng lực, tính cách, cơ thể, hiểu biết và mục tiêu | Không dùng một chỉ số thay cho mọi yếu tố |
| P13.2 Đo tác động của thay đổi | So hai điều kiện ghép cặp, giữ các yếu tố cần kiểm soát giống nhau | Báo độ bất định của chênh lệch, không chỉ chọn một ví dụ thuận lợi |
| P13.3 Kiểm thành phần có thực sự được dùng | Bỏ thử đầu vào hoặc bộ nhớ rồi đo trong nhiều tình huống | Thêm một trường dữ liệu mà mô hình bỏ qua chưa tạo khác biệt cá nhân |

$$
\Delta_i=Y_i(\mathrm{changed})-Y_i(\mathrm{base}).
$$

Báo chênh lệch trung bình, khoảng tin cậy và tác dụng ngoài dự kiến theo phương pháp đã chọn trước. Sáu chỉ số và điểm kinh nghiệm cũ là phương án tham khảo. Muốn chứng minh khả năng lập kế hoạch cần tình huống nhiều bước và đối chứng, không chỉ công thức. Xem phần 6.2 và điều kiện R2–R4 của kế hoạch sản phẩm.

## 6. E — quy mô, lưu game và tương thích

### P14. Hiệu năng và song song

**Đầu vào → đầu ra:** Khối lượng công việc cụ thể → tốc độ, độ trễ, bộ nhớ và công đoạn gây chậm.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P14.1 Ước lượng tài nguyên | Tính dân số, tần suất quyết định, vùng đệm và quan hệ cần lưu | Phân biệt số bản ghi với số cá thể có tương tác thật |
| P14.2 Đo toàn vòng xử lý | Đo tìm việc đến hạn, dựng đầu vào, chạy mạng, ghi nhận và cập nhật giao diện | Đo cả lần chậm, việc còn chờ và bộ nhớ lúc lưu; không chỉ thời gian chạy mạng |
| P14.3 Tối ưu mà giữ đúng luật | Cải thiện tìm kiếm, xử lý theo nhóm và cập nhật đúng mốc trước khi chia luồng | Chạy lại vẫn đúng; nhân vật ngoài màn hình vẫn già đi và chịu nghĩa vụ |
| P14.4 Chọn truy vấn và bảng tra cứu | Chỉ thêm index/cache khi một câu hỏi lặp lại đủ nhiều; state gốc vẫn là nguồn chính thức | So với quét state; kiểm sau sinh/chết/chuyển vùng/đổi quan hệ/reject/load; đo cả chi phí dựng và cập nhật |
| P14.5 Chọn thuật toán theo ý nghĩa dữ liệu | Quét, sắp, heap đến hạn, BFS/Dijkstra hay top-k tùy số phần tử và loại chi phí | Cùng seed cho kết quả tái lập; không để container hoặc thread quyết định quyền ưu tiên; test dữ liệu lệch và trường hợp hòa |
| P14.6 Giữ cache và phân trang đúng | Gắn kết quả tạm với revision, thời điểm và quyền xem; dùng snapshot hoặc khóa tiếp tục khi danh sách đổi | Không lộ thông tin giữa observer; không bỏ/lặp bản ghi khi trang kế tiếp được lấy sau một thay đổi |

Nếu mỗi phần tử bộ nhớ mạng dùng số thực 32 bit, tức bốn byte, dung lượng cho bộ nhớ riêng của các nhân vật là:

$$
M_h=4NH\;\mathrm{bytes}.
$$

Với một triệu người và $$H=64$$ phần tử mỗi người, riêng bộ nhớ mạng cần 256 MB, chưa tính thế giới. Số quyết định cần xử lý trong một giây thực là:

$$
\Lambda=\frac{Nv}{c}.
$$

Trong đó $$N$$ là số người, $$v$$ là số ngày game trong một giây thực, $$c$$ là số ngày giữa hai quyết định của một người. Phần 7 phân tích bộ nhớ tạm và công đoạn gây chậm toàn vòng; giới hạn tài nguyên ở phần 9 của kế hoạch sản phẩm. Không tự giảm phạm vi để đạt số đo mong muốn.

Một bảng tra cứu chỉ đáng giữ nếu chi phí cập nhật và bộ nhớ nhỏ hơn phần việc truy vấn lặp lại mà nó tiết kiệm. Đặt $$N$$ là số bản ghi gốc, $$E$$ là số quan hệ thật và $$k$$ là số kết quả trả ra: quét thường tốn O(N), còn index theo khóa có thể đưa lần đọc về O(k) sau khi đã dựng. Đây là phân tích độ phức tạp, không phải benchmark. Index phải được dựng lại từ dữ liệu chính thức và cho cùng kết quả với phép quét; hướng dẫn chọn index, heap, thuật toán đường đi, thứ tự ổn định và oracle nằm ở [DEVELOPER_GUIDE mục 7.3](DEVELOPER_GUIDE.md#73-chon-cach-truy-van-bang-tra-cuu-va-sap-xep).

### P15. Lưu, tải và giữ đúng ý nghĩa dữ liệu

**Đầu vào → đầu ra:** Bản lưu, phiên bản dữ liệu và nhận diện mô hình → tiếp tục đúng hoặc từ chối tải, giữ phiên hiện tại.

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| P15.1 Chụp đủ trạng thái | Lưu tại thời điểm nhất quán, gồm lịch, ngẫu nhiên, nghĩa vụ, mã quyết toán và bộ nhớ | Lưu giữa một hoạt động rồi tải phải tiếp tục như chưa gián đoạn |
| P15.2 Đọc và kiểm tra bản lưu | Đọc vào vùng tạm, kiểm số lượng, giới hạn, liên kết và điều kiện dữ liệu | Tệp hỏng không thay một phần thế giới đang chạy |
| P15.3 Kiểm mọi đường tải dữ liệu | Áp dụng kiểm tra tương thích qua Realm, Campaign, Python, C++ và Godot | Không có đường tải bỏ qua kiểm tra; kích thước dữ liệu giống không bảo đảm cùng ý nghĩa |

Với chuỗi lệnh $$\mathcal C$$, yêu cầu khi lưu rồi tải là:

$$
\operatorname{Run}\!\left(\operatorname{Load}(\operatorname{Save}(S)),\mathcal C\right)
\equiv \operatorname{Run}(S,\mathcal C).
$$

Quan hệ tương đương chỉ áp dụng trong điều kiện tái hiện đã công bố. Bộ điều khiển thời gian thực phải bỏ phần dư của phiên trước khi tải. Phần 8 giải thích tương thích; chuyển đổi bản lưu cần kế hoạch riêng. Không sửa thông tin nhận diện mô hình cũ để làm như nó phù hợp với luật mới.

## 7. F — kiểm chứng và hoàn thiện ứng dụng game

### P16. Đánh giá có thể kiểm tra lại

| Bài toán nhỏ | Phương pháp | Đầu ra cần có |
|---|---|---|
| P16.1 Kiểm luật mô phỏng | Thử từng thành phần và khi ghép chúng; đối chiếu ví dụ tính tay và chạy lại lệnh | Ghi phiên bản mã, nền tảng, lệnh chạy, kết quả và cách tái hiện lỗi |
| P16.2 Đánh giá hành vi đã học | So với quy tắc đối chứng và mô hình chưa học trên nhiều tình huống độc lập | Báo chỉ số, khoảng tin cậy, nhiều lần huấn luyện độc lập và trường hợp thất bại |
| P16.3 Đánh giá tốc độ và độ ổn định | Đo cả vòng xử lý và chạy mô phỏng dài với khối lượng công việc đã nêu | Ghi dân số, thời gian thực/game, độ trễ, bộ nhớ, việc còn chờ và giới hạn kết luận |

Cho cùng đầu vào vào bản PyTorch và bản ONNX rồi thu được kết quả gần nhau chỉ chứng minh việc xuất mô hình giữ được cách tính. Muốn biết nhân vật quyết định tốt phải kiểm kết quả chơi riêng. Kiểm thử lõi đạt chưa chứng minh ứng dụng chạy tốt; mô phỏng còn người sống chưa chứng minh xã hội ổn định.

Trong phát triển, dùng tập tình huống chọn mô hình (validation) để so các bản lưu sau huấn luyện. Chọn xong mới đánh giá trên tập kiểm tra độc lập dành riêng (test holdout). Không dùng kết quả tập độc lập để chọn lại mô hình rồi vẫn gọi đó là đánh giá chưa thiên lệch. Phần 9 giải thích phương pháp; mỗi báo cáo phải ghi rõ kết quả áp dụng cho phiên bản, môi trường và tình huống nào.

### Đưa P1–P16 tới người chơi

| Bài toán nhỏ | Phương pháp | Phép kiểm |
|---|---|---|
| UI.1 Quan sát và ra lệnh | Giao diện nhận dữ liệu được phép biết; gửi lệnh qua cầu nối Godot–C++ | Không sửa tiền/kho từ giao diện; thông báo từ chối phải dễ hiểu |
| UI.2 Điều khiển phiên chơi | Thống nhất mở mới, tải, tạm dừng, rời cửa sổ và kết thúc | Đóng hướng dẫn không tự chạy tiếp; ngủ máy không chạy bù |
| UI.3 Giải thích hậu quả | Hiển thị hồ sơ, nguồn/ngày báo cáo, hàng đang đi và tiến độ | Người chơi phân biệt đã ra lệnh với đã hoàn thành |
| UI.4 Đóng gói và khả năng tiếp cận | Có đủ tài nguyên, mô hình, giấy phép; thực hiện yêu cầu về điều khiển và hiển thị | Thử chơi không mạng, lưu/khôi phục dự phòng và đóng/mở lại trên nền tảng công bố |

Điểm vào mã: `gdext/polyec_session.cpp`, `godot_project/ui`, `godot_project/tests`, `scripts/`. Phần 18 của kế hoạch sản phẩm quy định trải nghiệm; PLAYER_GUIDE giải thích cách chơi. Nghiệm thu giao diện cần thử ứng dụng, không chỉ xem bản vẽ hoặc biên dịch lõi.

## 8. Chọn lát triển khai từ cây bài toán

Chọn chức năng có đầu vào, cách thay đổi trạng thái và kết quả rõ ràng; nêu điều kiện phải giữ và phép thử quyết định được đúng/sai. P4.3 có thể chỉ nối bộ điều khiển thời gian, chưa làm thêm cơ thể P6 hay lương P7. Ghi phần phụ thuộc, tệp được sửa, ảnh hưởng tương thích và điều chưa có; dùng [quy trình phát triển](DEVELOPMENT_WORKFLOW.md) và mẫu trong [CURRENT_TASK](CURRENT_TASK.md).

Cách biểu diễn năng lực, quy luật cơ thể, điểm thưởng, nhịp quyết định, dân số từ bản đồ và các khoản tranh cùng nguồn cần đặc tả được duyệt. Cây bài toán chỉ ra câu hỏi; không tự lấp bằng tham số hoặc nội dung tạm chưa xem xét.
