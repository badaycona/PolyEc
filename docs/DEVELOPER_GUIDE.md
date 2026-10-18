# PolyEc — mô hình bài toán, phương pháp và hệ thống thực nghiệm

Cập nhật đối chiếu: 2026-10-18. Tài liệu dành cho developer đọc để hiểu lý do thiết kế, implementation, phép đo và giới hạn. Scope sản phẩm thuộc [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md); source of truth cho tác vụ/trạng thái mới nhất là [CURRENT_TASK](CURRENT_TASK.md). Các công thức mô tả phương pháp; khi khác implementation phải ghi nhận khoảng cách, không giả rằng thiết kế đã chạy.

Baseline dùng để giải thích công thức pilot bên dưới là realm-v6/core repair b8160f8; evaluator attention có contract riêng. Tab chính tiếp tục T1a/T1b với schedule, save-v7 và clock. T1a đã được ghi nghiệm thu core trong HISTORY; T1b vẫn phải đọc trạng thái/gate cụ thể trong CURRENT_TASK. Những đoạn ghi “pilot” mô tả phiên bản thí nghiệm, không khẳng định toàn bộ HEAD còn y hệt; working tree có thể chứa phần đang review.

## Tóm tắt

**Đường đọc:** phần 1–3 đặt bài toán/domain/hạch toán; phần 4–5 giải thời gian, attention, settlement, thông tin và logistics; phần 6 giải mạng và training; phần 7–9 giải chi phí, tương thích và thực nghiệm; phần 10–12 ghi giới hạn, source map/build và nguồn tham khảo. Bảng P1–P16 ở phần 2 là bản đồ bài toán, không phải checklist đã pass.

PolyEc mô phỏng cai trị một quốc gia có nhiều cá nhân và thiết chế tự hành động. Bài toán không chỉ là chọn một hành động có điểm cao: hệ thống phải biểu diễn quyền sở hữu, nguồn lực hữu hạn, thời gian, thông tin riêng và hậu quả truyền giữa các chủ thể. Cùng một quyết định phải được thực hiện nhất quán khi phát từ giao diện, policy native hoặc môi trường huấn luyện.

Phương pháp của dự án là tách một C++20 core làm môi trường chuẩn, Godot làm giao diện, Python/PyTorch làm bộ học và ONNX Runtime CPU làm bộ suy luận. Learned policy chọn ý định; core kiểm quyền, hạch toán và thực hiện. Shared weights giảm chi phí model; ID, memory và sampling cá nhân giữ danh tính. Snapshot, command và save có ranh giới kiểm chứng. Hiện đã có sandbox M3, vertical slice M4, membership hộ và evaluator attention thuần; thế giới xã hội M5 vẫn đang xây, policy pilot chưa đạt toàn bộ quality gates.

**Từ khóa:** agent-based simulation, transaction, partial observability, recurrent PPO, semi-Markov decision process, sparse graph, deterministic replay.

## 1. Đặt vấn đề và tiêu chí thành công

Xét vùng mỏ thiếu lương thực khi tuyến đèo bị tranh quyền và khoản nợ sắp tới hạn. Cấp hàng chỉ có ý nghĩa khi biết hàng thuộc ai, ai được xuất kho, tuyến có đi được không, bao giờ tới và ai biết thông tin đó. Nếu chủ xưởng chết trong lúc chờ hàng, nợ, lương và quyền quản lý cần được giải quyết riêng. Một thanh “ổn định” không đủ biểu diễn chuỗi này.

Đầu vào gồm geography, dân số, tài sản, quyền/nghĩa vụ, luật, seed, command và bước tiến game time. Đầu ra gồm state mới, kết quả hành động, nghĩa vụ còn mở, facts có nguyên nhân, thông tin được quan sát và dữ liệu replay/save.

| Bài toán | Phương pháp | Cách đánh giá |
|---|---|---|
| World có một sự thật hạch toán | Core sở hữu state; quyền và command rõ | Invariant, đối soát tiền/hàng, reject không sửa dở |
| Cá nhân có đời sống/agency | ID bền, trạng thái/knowledge/memory riêng, learned choices | Vòng đời, khác biệt theo hoàn cảnh, phản ứng nhiều bước |
| Hệ quả trồi sinh | Facts nối hành động/world process; nghĩa vụ và thông tin lan có độ trễ | Counterfactual cùng điều kiện đầu, thay can thiệp làm đổi chain |
| Chạy cùng luật khi train/chơi | Binding cùng core, version/model guard | C++/Python/ONNX/native parity |
| Quy mô lớn | Compact storage, sparse index, batching và lịch hữu hạn | Full-cycle workload, backlog, p95/p99, RAM/save/UI |

Phân biệt ba câu hỏi: luật có đúng không, policy có học tốt không, hệ thống có đủ nhanh không. Không phép kiểm nào thay hai phép còn lại. Một world chết hết dân dù tiền bảo toàn vẫn là thất bại về chất lượng mô phỏng.

## 2. Mô hình thế giới và ownership

### 2.1. Phân rã bài toán và ranh giới chứng minh

Mỗi hàng là một bài toán cần giải riêng. Cột kiểm chứng là yêu cầu phương pháp; không có nghĩa tất cả đã pass. Cách đọc hiệu quả: hiểu P1–P4 trước khi đọc mạng neural; hiểu P5–P8 trước khi thiết kế reward; đọc P13–P16 trước khi tuyên bố chất lượng hay quy mô.

| Mã | Bài toán nhỏ | Đầu vào → đầu ra | Phép kiểm quyết định |
|---|---|---|---|
| P1 | Danh tính, ownership và quyền | ID, record, quan hệ → chủ thể/tài khoản có thẩm quyền | Đổi vai/hộ không nhân bản tài sản; reference hợp lệ |
| P2 | Command nguyên tử | State, command, version → reject hoặc state mới | Reject giữ state; stale/duplicate không thực hiện lại |
| P3 | Tiền, hàng, lương và thuế | Số dư, nghĩa vụ, định mức → bút toán đối ứng | Bảo toàn, làm tròn, thiếu ngân sách, overflow |
| P4 | Tiến thời gian | Cursor, delta, deadlines → các quá trình đến hạn | Chia bước không đổi kết quả; pause không tiến lịch |
| P5 | Phân bổ attention | Sessions một actor → delivered/shortfall | Capacity, tie, reorder và split-additivity |
| P6 | Attendance, effort và cơ thể | Attention, vị trí, dụng cụ, body → effort/rest thực | Không có năng suất từ lời hứa; nguồn lực được dùng một lần |
| P7 | Settlement và nghĩa vụ | Effort, hợp đồng, tài khoản → output/lương/nợ/evidence | Không trả hai lần, không trả từ tiền chưa có |
| P8 | Thông tin và hệ quả | Facts, đường truyền, thời gian → knowledge/case | Không biết sớm; chain có căn cứ và đối chứng |
| P9 | Logistics | Lô hàng, graph, slot → manifest/lịch đến | Không nhân công suất, không tạo hàng khi giao |
| P10 | Biểu diễn quan sát và cơ hội | Knowledge/state hợp lệ → features/candidates/mask | Không lộ bí mật; retrieval không âm thầm chọn chiến lược |
| P11 | Policy có memory | Observation, hidden → logits/next hidden/action | Đúng shape/gate/mask; sampling riêng từng ID |
| P12 | Dữ liệu, reward và PPO | Trajectories mới → trọng số cập nhật | Đối soát reward, terminal/bootstrap, recurrent boundaries |
| P13 | Cá nhân hóa và năng lực | Traits, knowledge, body, cơ hội → phân phối hành vi | Paired intervention, ablation, kết quả nhiều bước |
| P14 | Quy mô và song song | Workload, ngân sách CPU/RAM → throughput/latency | Đo cả chu kỳ, backlog, RAM; không đổi luật vì thread order |
| P15 | Save và tương thích | Snapshot/model/schema → world phục hồi | Continuation, malformed load atomic, semantic guard |
| P16 | Đánh giá khoa học | Seeds/partners/metrics khóa trước → CI/kết luận có phạm vi | Holdout, nhiều training seeds, đủ mẫu, đúng chế độ runtime |

P1–P3 có implementation pilot và các test theo báo cáo; P5 có evaluator thuần. P4 đang tích hợp T1; P6–P7 là lát M5 kế tiếp, chưa được chứng minh bởi evaluator. P10–P12 có pipeline pilot, còn quality gates. P13 và quy mô đích của P14 cần thêm thực nghiệm. Phần còn lại có implementation từng phạm vi, không đồng nghĩa hoàn chỉnh toàn thế giới.

### 2.2. Ký hiệu, đơn vị và mô hình chuyển trạng thái

Đặt trạng thái đầy đủ:

$$
S_t=(\mathcal P_t,\mathcal A_t,\mathcal G_t,\mathcal L_t,
\mathcal Q_t,\mathcal F_t,\mathcal K_t,\xi_t).
$$

$\mathcal P$ là cá nhân/vòng đời; $\mathcal A$ là tài khoản và quyền; $\mathcal G$ là hàng hóa; $\mathcal L$ là nghĩa vụ; $\mathcal Q$ là hoạt động/lịch; $\mathcal F$ là facts; $\mathcal K$ là knowledge; $\xi$ là trạng thái RNG. Đây là phân rã để phân tích, không tuyên bố mỗi ký hiệu đã có một class độc lập tương ứng.

Command $c$ và bước thời gian $\Delta\tau$ tạo chuyển tiếp:

$$
S'=T(S,c,\Delta\tau),\qquad
\neg\operatorname{Valid}(S,c)\Longrightarrow S'=S.
$$

RNG nằm trong $S$: reject cũng không được âm thầm tiêu RNG làm thay đổi diễn tiến sau đó. Thất bại trong thế giới sau một nỗ lực hợp lệ cần một loại kết quả khác reject, có chi phí và evidence được đặc tả.

| Đại lượng | Ký hiệu/đơn vị | Quy tắc dùng |
|---|---|---|
| Tiền | $b_a$, đơn vị tiền nguyên | Không cộng số đếm thuế vào số dư lần nữa |
| Hàng | $q_{a,g}$, đơn vị của loại $g$ | Không cộng hai loại hàng thành một lượng vật lý thiếu quy đổi |
| Thời gian | $\tau$, tick nguyên tuyệt đối | Evaluator không tự định nghĩa tick là phút |
| Attention rate | $d_j$, units/tick | Capacity $C=1000$ theo evaluator hiện có |
| Attention tích lũy | $D_j$, units | Rate nhân thời lượng; không phải số giờ làm hay sản phẩm |
| Reward/utility | $r,U$, điểm mục tiêu | Không gọi normalized utility là lợi nhuận tiền |
| Decision index | $t$, một lần quyết định | Một bước RL có thể trải qua nhiều tick/ngày |

Phân biệt $t$ và $\tau$ là bắt buộc khi cadence không đều. T1a đang dùng 64 subday units/ngày trong contract/source làm việc; đây là quy ước clock, chưa là mô hình sinh lý hay nghiệm thu T1b.

### 2.3. Biểu diễn domain bằng composition

Các trách nhiệm dưới đây mô tả kiến trúc đích và điểm neo hiện có; không phải tất cả tên domain đã có class hoàn chỉnh.

| Domain | State/invariant phải có | Điểm neo và tình trạng |
|---|---|---|
| Person | ID, vòng đời, vai trò, body, memory identity; không nhân bản khi đổi vai | `realm.hpp`, Realm; body mới chưa đủ |
| Household | Membership/guardian, cư trú, quỹ chung và thẩm quyền chi | Membership/index có; quỹ hộ động còn M5 |
| Holding/Right | Chủ thể, đối tượng, loại quyền, nguồn, hiệu lực; độc quyền không cấp trùng | Hướng M5; quyền đèo M4 chỉ một lát |
| Establishment | Chủ, location, account, inventory, capacity, recipe | Xưởng M4 có; cơ sở tổng quát thuộc M5 |
| Employment/Apprenticeship | Các bên, consent, terms, chỗ/người dạy, attendance, nghĩa vụ | Pilot job/school không thay hợp đồng M5 |
| Organization/Office | Membership, quỹ, quyền đại diện, bổ nhiệm/bãi nhiệm | Quỹ/vai trò pilot; tổ chức/chức đầy đủ còn mở |
| Marriage/Estate/Claim | Consent, escrow, tài sản riêng/chung, nợ, custody và tranh chấp | Thiết kế đã chốt luật nền, implementation M5 |
| Ledger/Obligation | Debit/credit, hạn, nguồn, settled ID, phần dư; không chi hai lần | Tiền/nợ/thuế pilot; ledger tổng quát cần từng lát |
| Activity/Schedule | Interval, demand, priority, cursor/counters, retirement | Evaluator + tích hợp timeline T1; đọc task record |
| Geography/Route/Cargo | Topology, quyền, slots, manifest; hàng không teleport | Campaign M4, chưa quốc gia đủ nội dung |
| Fact/Knowledge/Case | Sự thật và thời điểm biết riêng; evidence/provenance | Realm/Campaign có lát cắt, privacy cần audit |
| PlayerView/Command | Projection đọc theo knowledge; mutation qua cùng pipeline | Core/GDExtension/Godot, cần giữ qua mọi lát mới |

**Thuật ngữ vận hành:** payroll là tính và thanh toán lương; settlement là thực hiện nghĩa vụ tới hạn có đối ứng; CTest là công cụ chạy các test đã đăng ký trong build CMake. CTest pass chỉ nói các executable/test đó pass trong cấu hình đã chạy; không tự nói model đã học hay game đủ nhanh. Actor là chủ thể ra quyết định; actor network là nhánh policy, thường dùng chung cho nhiều chủ thể.

Gọi trạng thái chuẩn là S, command là c, bước game time là Δt và trạng thái ngẫu nhiên là ξ. World áp dụng transition T(S,c,Δt,ξ). Với command sai cấu trúc/quyền, yêu cầu S' = S. Một nỗ lực hợp lệ thất bại có thể tạo S' khác S vì có chi phí hoặc hậu quả thực; kết quả phải phân biệt với reject API.

Core dùng composition và các record domain, không yêu cầu mỗi entity là một heap object hay Godot Node. `Person` giữ ID và đời sống; household membership, việc làm, tài sản, tước/chức, nghĩa vụ và quan hệ là những khái niệm riêng. Một người có nhiều vai không được nhân bản cash hoặc tính thêm dân số.

`Realm` sở hữu các container mô phỏng. Membership index `households_` là dữ liệu dẫn xuất từ Person, dựng lại khi load, không là một world hoặc quỹ tiền thứ hai. Các manager finance/household/project hiện là helper không sở hữu world; không public mutable accessor cho UI/trainer sửa container trực tiếp.

`Campaign` sở hữu Realm cùng tuyến, cargo và case của M4. `GameWorld`/`SimulationController` là đường core/clock prototype riêng. Không giả rằng hai clock này đã được nối thành clock timeline mới. Hợp nhất đơn vị và ownership là nhiệm vụ tích hợp sắp tới.

## 3. Command, transaction và bất biến

### 3.1. Validation, chuẩn bị và commit

Luồng xử lý: nhận actor/tư cách/version → validate ID/quyền/consent → kiểm tài nguyên và điều kiện của bước đang thực hiện → chuẩn bị thay đổi → commit → facts → propagation thông tin. Revalidate khi snapshot đã cũ hoặc nhiều actor tranh tài nguyên. Sequence chống lệnh cũ/lặp; ID settlement chống xử lý nghĩa vụ hai lần.

Trong Realm hiện tại, nhiều nhánh validate trước rồi mutate; load dùng world tạm và invariant trước khi move. GameWorld có snapshot/restore cho command. Đây không phải bằng chứng mọi command gameplay mới đã có transaction tổng quát hoặc exception-safety cho input tùy ý. Mỗi lát phải audit phạm vi mutate, overflow và reject của chính nó.

### 3.2. Định luật đối soát và phạm vi tài khoản

Gọi $\mathcal A$ là tập tài khoản thực, gồm cá nhân, cơ sở, ngân khố và escrow nếu đường gameplay sử dụng. Với nguồn/phần tiêu hủy được khai báo $E_t,Z_t$, tổng tiền là:

$$
M_t=\sum_{a\in\mathcal A}b_{a,t},\qquad
M_{t+1}-M_t=E_t-Z_t.
$$

Chuyển nội bộ $x$ từ $u$ sang $v$ phải có $\Delta b_u=-x$, $\Delta b_v=x$. Nợ là quyền/nghĩa vụ, không là một tài khoản tiền mới; escrow là nơi giữ tiền và phải được đếm đúng một lần. Counter `tax_receipts` ghi hoạt động thu, không có mặt thêm trong tổng số dư.

Với một loại hàng $g$, đặt $I_g$ là tổng hàng ở mọi kho/cá nhân/cargo; $P_g,C_g$ là sản xuất và tiêu dùng tích lũy kể từ mốc đầu:

$$
I_g(\tau)+C_g(\tau)=I_g(0)+P_g(\tau).
$$

Đây là hệ kín của phạm vi đang xét. Khi thêm nhập khẩu, hư hỏng hoặc thất thoát phải thêm nguồn/đích tường minh; không sửa số tổng để che thiếu đối ứng.

Diễn giải bằng lời:

```text
Tổng tiền sau = tổng tiền trước + nguồn tiền khai báo − đích tiền khai báo
Kho sau = kho trước + sản xuất + nhập/chuyển vào − tiêu dùng/đầu vào − xuất/chuyển ra
```

Tiền công là chuyển từ tài khoản employer sang người làm; thuế là chuyển tiếp sang treasury. `tax_receipts` là counter, không cộng như tài sản. Hàng trong cargo vẫn có chủ và phải được tính khi đối soát; giao hàng không là sản xuất mới. Tín dụng chuyển tiền thật, vay không là lợi nhuận.

Không hard-gate kế hoạch chỉ vì quá tải hoặc dự báo thất bại. Ngược lại, settlement không được tạo thiếu nguồn để giữ lời hứa. Cạnh tranh tài nguyên phải có cơ chế phân bổ được spec; thứ tự actor/thread không tự thành quyền ưu tiên.

Các bài toán hạch toán cụ thể:

Các công thức dưới đây mô tả pilot hiện có; nguồn đối chiếu là [Realm](../core/src/realm.cpp), [finance manager](../core/src/finance_manager.cpp) và [court](../core/src/realm_court.cpp). Chúng không thay đặc tả settlement M5.

### 3.3. Tiền: chuyển giao thay vì tạo/xóa ngầm

**Đề bài.** Triều đình chi 300 đồng làm đường thì phải biết tài khoản nào trả và tài khoản nào nhận.

Gọi $b_u,b_v$ là số dư hai tài khoản trước giao dịch; $\Delta$ là số chuyển; dấu prime là số dư sau giao dịch:

$$
b'_u=b_u-\Delta,\qquad b'_v=b_v+\Delta.
$$

Nếu ngân khố có 1.000 và bên nhận có 200, chuyển 300 tạo số dư 700 và 500. Tổng trước và sau đều bằng 1.200. Điều kiện cần kiểm trước là $\Delta\ge0$ và bên trả đủ tiền.

Tổng tiền toàn mô hình được đối soát bằng:

$$
M_t=M_0+E_t.
$$

$M_t$ là tổng tiền tại ngày $t$; $M_0$ là tiền đầu kỳ; $E_t$ là tổng khoản endowment bổ sung được khai báo. Nếu có quỹ khởi đầu do augment, nó phải vào vế $E_t$, không xuất hiện như doanh thu vô căn cứ. Đây là invariant của nền kinh tế hiện tại, không phải giả thuyết rằng mọi nền kinh tế lịch sử đều có cung tiền cố định.

### 3.4. Hàng: tính cả lượng đang đi đường

**Đề bài.** Xuất kho 100 đơn vị không làm 100 đơn vị đó lập tức có mặt ở đích hoặc biến mất.

Với một loại hàng $g$, đặt $I_g^0$ là hàng đầu kỳ, $P_g(t)$ là sản xuất tích lũy, $C_g(t)$ là lượng đã tiêu dùng, và $I_g(t)$ là tổng hàng còn trong kho, inventory cá nhân và manifest vận tải:

$$
I_g^0+P_g(t)=I_g(t)+C_g(t).
$$

Ví dụ đầu kỳ 100, sản xuất thêm 40; kho còn 70, cá nhân giữ 10, vận tải giữ 20 và đã tiêu dùng 40. Vế trái bằng 140; vế phải bằng $70+10+20+40=140$. Giao hàng chỉ đổi vị trí trong $I_g(t)$, không được cộng thêm vào sản xuất.

### 3.5. Sản xuất: đầu vào hữu hạn và lao động

**Đề bài.** Xưởng dệt cần người làm, xơ, nhiên liệu, công cụ và tiền. Chờ đủ thời gian không thay được các đầu vào đó.

Về mặt phân tích, mỗi quy trình có định mức đầu vào/đầu ra. Số mẻ thực hiện bị giới hạn bởi điều kiện thiếu nhất: đủ nguyên liệu cho 10 mẻ nhưng lao động chỉ đủ 4 mẻ thì không được làm quá 4. Đây là dạng production có ràng buộc đầu vào, gần mô hình Leontief ở mức khái niệm; code hiện dùng công thức cụ thể, không giải tối ưu input–output tổng quát.

M4 có quy trình xưởng dệt Bạch Khâu với tối đa sáu lao động trưởng thành, vốn và vật tư hữu hạn. Quy trình kiểm tra tiền trả lương/đầu vào trước khi biến đổi kho; bán hàng nhận tiền từ thị trường có số dư. Các khoản này cùng tham gia đối soát. Nó chưa phải bộ mô phỏng mọi loại doanh nghiệp, công nghệ và thị trường lao động.

### 3.6. Giá: phản ứng đơn giản với mức tồn kho

**Đề bài.** Thiếu hàng cần gây hậu quả tới chi phí sinh sống và cơ hội mua bán. Prototype chưa cần giải một hệ cân bằng kinh tế tổng quát.

Giá hiện tại trong `Realm` là:

$$
p=\operatorname{clip}\!\left(
\operatorname{int}\!\left[p_{\mathrm{ref}}\left(1.5-\frac{s}{800}\right)\right],
2,5p_{\mathrm{ref}}\right).
$$

Ở đây $p$ là giá, $p_{\mathrm{ref}}$ là giá tham chiếu, $s$ là tồn kho; `int` chuyển về số nguyên; `clip(x,a,b)` giữ giá trị trong khoảng $[a,b]$. Với giá tham chiếu 20, kho 400 tạo giá 20; kho 0 tạo giá 30. Ý nghĩa là khan hiếm tương đối làm giá tăng, trong giới hạn quy định.

Đây là heuristic định giá từ tồn kho. Hệ số 800 và 1,5 là tham số implementation, không phải hằng số kinh tế học. Chưa có auction, order book hoặc thuật toán tìm giá cân bằng cung–cầu.

### 3.7. Lương thiếu ngân sách và thuế làm tròn

**Đề bài lương.** Nhiều người được trả lương từ cùng tài khoản nhưng tiền không đủ. Trả hết cho ID nhỏ trước sẽ tạo ưu tiên lâu dài do thứ tự duyệt.

Gọi $d_i$ là lương phải trả cho người $i$, $D=\sum_i d_i$ là tổng nghĩa vụ, và $B\le D$ là ngân sách được dùng, lấy bằng min(số dư khả dụng, tổng nghĩa vụ). Phần chi trả ban đầu:

$$
w_i=\left\lfloor B\frac{d_i}{D}\right\rfloor.
$$

$\lfloor x\rfloor$ là làm tròn xuống. Ví dụ hai người cùng phải nhận 3 đồng, tài khoản chỉ có 5: mỗi người nhận trước 2, còn dư 1. Code phân phần dư theo thứ tự luân phiên. Kỹ thuật này giảm thiên lệch làm tròn, không chứng minh công bằng xã hội toàn mô hình.

**Đề bài thuế.** Nếu mọi lần thu nhỏ đều làm tròn xuống độc lập, tổng thuế có thể bị hụt đáng kể.

Gọi $W_t$ là khoản thu nhập chịu thuế lần $t$, $\tau$ là thuế suất phần trăm và $r_{t-1}$ là phần dư trước đó. Thuế và dư mới là:

$$
T_t=\left\lfloor\frac{\tau W_t+r_{t-1}}{100}\right\rfloor,
\qquad r_t=(\tau W_t+r_{t-1})\bmod100.
$$

Ký hiệu `mod` lấy phần dư phép chia. Với thu nhập 1 đồng mỗi lần, thuế 10%, phần dư tăng 10 sau mỗi lần; lần thứ mười thu được 1 đồng thuế. Không cần dùng tiền dạng float để giữ phần nhỏ.

### 3.8. Nợ và escrow

**Đề bài nợ.** Người vay nhận tiền nhưng đồng thời phát sinh nghĩa vụ, vì vậy `cash` tăng không đồng nghĩa giàu lên tương ứng.

Gọi $P$ là principal, $I$ là interest theo điều khoản, $R$ là tổng tiền đã trả. Dư nợ:

$$
L=P+I-R.
$$

Vay 200, lãi 8, đã trả 50 thì còn 158. Tại ngày đến hạn mà $L>0$, khoản vay bị đánh dấu quá hạn. Interest hiện là nghĩa vụ được xác định theo điều khoản, không phải mô hình lãi kép hay lãi suất nội sinh.

**Đề bài escrow.** Một đề nghị có tiền đặt trước phải giữ tiền ở nơi trung gian, để bên gửi không tiếp tục tiêu nó. Escrow là khoản giữ chờ điều kiện. Khi đề nghị được nhận hoặc hết hạn, tiền được chuyển/refund có đối ứng. `Proposal` hiện hỗ trợ những quy trình hữu hạn; chưa phải engine hợp đồng hiểu mọi điều khoản pháp lý.

## 4. Thời gian, hoạt động và cơ thể

Đề bài là phân biệt “có hợp đồng”, “muốn làm”, “đã dành attention”, “đã hiện diện” và “đã tạo kết quả”. Thuật ngữ busy duy nhất không đủ vì một người có thể nhận việc mới, gián đoạn việc cũ hoặc làm hai việc tương thích trong ngày.

### 4.1 Evaluator thuần đã có

`polyec::activity::evaluateActivity(actor_id, Horizon{begin,end}, sessions)` nhận immutable views của **một actor**, với half-open intervals tuyệt đối. Tick trừu tượng, không có ngầm định phút/ngày. Capacity là rate cố định 1000 attention units/tick, mỗi session có demand_rate 1..1000 và priority do caller cung cấp.

Thuật toán clip horizon, tách tại endpoints và xét nhóm priority giảm dần. Nhóm vừa capacity nhận đủ; singleton vượt phần còn lại nhận attention còn lại. Tie vượt capacity dương chưa phân thì không chọn theo ID: giữ unresolved cho nhóm và demand bị chặn phía dưới. Capacity đã tiêu hết bởi delivery cao hơn thì phần thiếu là capacity shortfall, kể cả nhóm thấp có tie.

```text
requested = delivered + unmet
unmet = capacity_unmet + unresolved_unmet
units = rate × độ dài interval
```

Số học int64 có kiểm overflow; reject trả results rỗng, input không đổi. Với S sessions, implementation hiện chia O(S) segments, quét và sort active set từng đoạn: worst-case O(S² log S) time, O(S) memory. Đây là phân tích thuật toán, không là benchmark dân số. Không copy toàn dân để tính một actor.

Phép kiểm chính là reorder invariance và additivity: evaluate [a,c) phải bằng tổng [a,b) và [b,c) trên các counters khi input không đổi, kể cả b không là endpoint. Tính chất này bảo vệ việc phân đoạn thời gian và cách chia workload.

**Giới hạn:** chưa nối Realm command/clock/save; attention delivered chưa là attendance, output, wage, learning hoặc care. Tie withholding hiện là contract phát triển tạm; khi tích hợp cần cơ hội actor giải quyết để tránh world đứng yên vì xung đột lịch.

### 4.2 Phần world hiện tại chưa đáp ứng thiết kế mới

`Realm::daily()` còn cập nhật theo ngày; `busy_until` chặn nhiều quyết định, job/school/body còn theo cơ chế pilot. Một số command không liên quan còn có đường xóa job. `Rest` pilot tăng health ngay khi command; đó chưa phải mô hình phục hồi từ nghỉ thực.

Thiết kế đích tách sleep pressure/debt, exertion và health; actual rest/effort mới thay đổi body. Determination/kiên trì là khuynh hướng lựa chọn, không đồng nghĩa sức bền hoặc attention vô hạn. Công thức, đơn vị và tham số cơ thể chưa được xác nhận bởi evaluator. Không tự áp ngưỡng 8/16 giờ hay cấp quyền pháp lý từ một chỉ số sức khỏe.

Game time tiến thì aging, nhu cầu, deadline và tiến độ phải được cập nhật dù policy không được gọi. Lazy update chỉ đúng khi xử lý đủ các ranh giới; pause/mất focus/sleep không tích lũy để chạy bù.

### 4.3. P5 — mô hình toán của evaluator attention

Nguồn hiện thực: [activity.hpp](../core/include/polyec/activity.hpp), [activity.cpp](../core/src/activity.cpp); tính chất kiểm thử trong [test_activity.cpp](../core/tests/test_activity.cpp). Mỗi session $j$ có interval $[b_j,e_j)$, demand rate $d_j$, priority $p_j$ và ID duy nhất; horizon là $[a,b)$. Độ dài overlap và attention yêu cầu là:

$$
\ell_j=\max\{0,\min(e_j,b)-\max(b_j,a)\},\qquad R_j=d_j\ell_j.
$$

Session kết thúc tại $\tau$ không trùng session bắt đầu tại $\tau$, nhờ interval nửa kín. Sau clip, lấy tập endpoints đã sort/unique $u_0<\cdots<u_m$. Trên mỗi đoạn $I_k=[u_k,u_{k+1})$, active set không đổi nên allocation rate tính một lần; counters nhận rate nhân $\Delta_k=u_{k+1}-u_k$.

Với một nhóm cùng priority, đặt $G=\sum_{j\in g}d_j$, $c$ là capacity còn lại. Xét nhóm giảm dần priority, trước khi có tie chưa giải quyết:

$$
x_j=\begin{cases}
d_j,&G\le c,\\
c,&G>c,\ |g|=1,\\
0,&G>c>0,\ |g|>1,\\
0,&c=0.
\end{cases}
$$

$x_j$ là rate thực cấp. Nhóm vừa cập nhật $c\leftarrow c-G$; singleton vượt phần còn lại dùng hết $c$. Với tie vượt capacity dương, cả demand của nhóm và các nhóm thấp hơn được ghi unresolved; phần capacity còn lại không cấp ngầm cho ID nhỏ hay priority thấp hơn. Nếu $c=0$ do delivery trước đó, thiếu hụt là capacity shortfall. Thứ tự kiểm hai trường hợp này là một phần contract, không chỉ cách đặt tên counter.

Tích lũy trên các đoạn:

$$
D_j=\sum_k x_{j,k}\Delta_k,\qquad
R_j=D_j+C_j+U_j,\qquad M_j=C_j+U_j.
$$

$D,C,U,M$ lần lượt là delivered, capacity-unmet, unresolved-unmet và unmet. Capacity còn có bất biến theo đoạn:

$$
\sum_j x_{j,k}\le1000,\qquad x_{j,k}\ge0.
$$

Ví dụ A demand 700 ưu tiên cao; B demand 600 ưu tiên thấp; cùng chạy 10 tick. A nhận 7000, B nhận 3000 và thiếu capacity 3000. Nếu A/B cùng priority, tổng 1300 vượt 1000: cả hai unresolved, không chia đều hay chọn A trước. Đó là kết quả của contract hiện tại, chưa phải cơ chế cuối để NPC thương lượng lịch.

Hai tính chất cần chứng minh bằng test:

$$
E(S,[a,c))=E(S,[a,b))+E(S,[b,c)),\qquad a<b<c,
$$

$$
E(\sigma(S),H)\equiv E(S,H).
$$

$E$ trả vector counters theo session ID; dấu cộng là cộng từng counter cùng ID, không cộng status/reason. $\sigma$ là hoán vị input, không thay priority. Additivity chỉ áp khi lịch/rate/priority không đổi; có cancel/reprioritize tại $b$ phải dùng snapshot thích hợp hai phía. Code kiểm cộng/nhân int64, reject trả kết quả rỗng; test cần chạm đúng overflow thay vì bị một validator sớm hơn chặn.

### 4.4. P4 — tiến thời gian và chia bước

Đặt $\Phi_{a,b}$ là tiến world từ cursor $a$ tới $b$. Mục tiêu tích hợp là:

$$
\Phi_{b,c}(\Phi_{a,b}(S))=\Phi_{a,c}(S).
$$

Điều kiện là cùng command/event tại cùng thời điểm, cùng thứ tự phase và RNG, cùng semantics số học. Không được suy tính chất của toàn world chỉ từ additivity evaluator. Nếu daily chạy tại mỗi bội $q=64$ trong T1a, số boundary đi qua khi advance $(a,b]$ là:

$$
n_{\mathrm{day}}=\left\lfloor\frac bq\right\rfloor-
\left\lfloor\frac aq\right\rfloor.
$$

Ví dụ từ cursor 63 tới 65 chỉ chạy daily một lần tại 64; từ 64 tới 65 không chạy lại boundary 64. Integration cần phân đoạn tại boundary/ngày/session/retirement liên quan, tích lũy đúng phần chưa xử lý rồi chạy phase đến hạn theo contract. Lưu counter tích lũy mà không lưu cursor tương ứng có thể khiến load trả credit lần nữa.

T1a source làm việc đã có [schedule.hpp](../core/include/polyec/schedule.hpp) và tích hợp trong Realm. Phân tích ở đây giải thích contract; acceptance vẫn theo CURRENT_TASK và report của tab chính. Nó không chứng nhận clock UI, nghỉ, lương hay học đã nối xong.

### 4.5. P6 — từ attention sang attendance, effort và cơ thể

**Trạng thái: phân rã thiết kế, chưa là công thức balance được duyệt.** Delivered attention chỉ trả lời người đó đã phân bổ bao nhiêu khả năng thực hiện. Attendance còn cần vị trí và khung giờ; effort cần dụng cụ/vật tư và khả năng cơ thể; output cần recipe và các đầu vào khác.

Một mô hình để viết contract, chưa phải code đang chạy:

$$
A_j=\int_{b_j}^{e_j}v_j(\tau)x_j(\tau)\,d\tau,
\qquad 0\le v_j(\tau)\le1,
$$

$$
F_j=\int_{b_j}^{e_j}v_j(\tau)x_j(\tau)
\eta_j(B_i(\tau),s_i,\mathcal R_j(\tau))\,d\tau.
$$

$A_j$ là attention có hiện diện; $v_j$ là điều kiện hiện diện hợp lệ; $F_j$ là effort hiệu dụng; $B_i$ là body, $s_i$ kỹ năng, $\mathcal R_j$ tài nguyên/công cụ. Tích phân diễn đạt ý tưởng tích lũy; implementation có thể là tổng fixed point theo đoạn. Hàm $\eta$, đơn vị và cách giữ phần dư phải được spec riêng. Không nhân một hệ số health vào mọi sản lượng để thay toàn bộ mô hình.

Body cần state transition theo thời gian và hoạt động thực:

$$
B_i(\tau+\Delta\tau)=f_B(B_i(\tau),\mathrm{rest}_i,
\mathrm{effort}_i,\mathrm{nutrition}_i,\Delta\tau).
$$

Chưa chốt $f_B$ hay hệ số của nó. Determination là khuynh hướng lựa chọn, không tăng capacity vô hạn và không miễn hậu quả thiếu ngủ. Test phải tách nghỉ được thực hiện với việc gửi command Rest; một người chết/đi đường không được ghi thêm attendance tại xưởng.

### 4.6. P7 — sản xuất và settlement đúng một lần

**Trạng thái: yêu cầu phương pháp cho M5.** Với recipe theo mẻ cần $a_g$ đơn vị mỗi nguyên liệu $g$, effort $a_F$ mỗi mẻ và công suất $K$, giới hạn phân tích là:

$$
n\le\min\left(K,\left\lfloor\frac F{a_F}\right\rfloor,
\min_{g:a_g>0}\left\lfloor\frac{q_g}{a_g}\right\rfloor\right).
$$

Đây là giới hạn khả thi, không phải policy chọn sản xuất bao nhiêu hoặc bằng chứng recipe tổng quát đã triển khai. Sau khi chọn $n$, kho tiêu $na_g$ và nhận $nb_g$ theo recipe; hàng dở dang phải có record nếu hỗ trợ. Tiền lương theo hợp đồng/attendance phải settle từ tài khoản thật, có nhánh thiếu ngân sách và khoản còn nợ theo luật được duyệt.

Mỗi lần settlement cần identity ổn định $k$ (ví dụ contract, kỳ, installment). Tính chất idempotence mong muốn:

$$
\operatorname{Settle}_k(\operatorname{Settle}_k(S))
=\operatorname{Settle}_k(S).
$$

Phải kiểm cả trước/sau save/load, session bị cancel, người chết giữa kỳ, chủ đổi hoặc nghĩa vụ tranh chấp. Counter chống lặp, debit/credit, output và evidence phải cùng transaction; không đánh dấu đã settle rồi mới tìm tiền.

## 5. Từ facts đến thông tin và trồi sinh

World tạo fact có ID, participants, location/time và parent cause. KnowledgeRecord xác định ai nhận được fact khi nào. Case/obligation tổng hợp các facts đang liên quan, không tự bịa thủ phạm hoặc nguồn lực để khớp câu chuyện.

Một chuỗi thử: chủ xưởng chết → di sản còn nợ → thiếu nguồn trả lương → người làm có thể tìm việc/đòi quyền → guild hoặc chính quyền nhận hồ sơ. Test luật có thể phát command định sẵn để chứng minh chain; test policy phải đánh giá riêng các quyết định đã học. Không đòi mỗi seed lặp đúng một câu chuyện.

Đối chứng cần giữ điều kiện đầu/shock và thay một can thiệp, rồi so dòng tiền/hàng, việc/quan hệ và thông tin. Causal links hỗ trợ audit nhưng tự có log chưa chứng minh nguyên nhân; cần branch replay/counterfactual phù hợp. Actor chỉ dùng điều biết; critic train có thông tin thêm không được làm lộ secret qua actor input hoặc reward proxy.

Quan hệ lưu sparse, directed/typed và có provenance. Membership người→tổ chức không biến thành clique mọi cặp thành viên. Đổi hộ không tự xóa họ hàng, nợ hoặc quyền. History thường lệ có thể tổng hợp; evidence của nghĩa vụ/claim đang hiệu lực phải được giữ.

### 5.1. Logistics: đưa hệ quả qua không gian và thời gian

Đây là luật của lát cắt M4 trong [campaign.cpp](../core/src/campaign.cpp), chưa là solver kinh tế toàn quốc.

#### 5.1.1. Đề bài và biểu diễn

Cần chuyển $Q$ đơn vị từ nguồn tới đích. Mỗi cạnh đường có thời gian, công suất và trạng thái. Lô khác có thể đã đặt chỗ. Kết quả cần gồm cả tuyến và lịch đi từng chặng, không chỉ khoảng cách trên màn hình.

Biểu diễn bằng graph $G=(V,E)$: $V$ là tập đầu mối/lãnh địa, $E$ là tập tuyến nối chúng. “Graph” ở đây là cấu trúc đỉnh–cạnh, không phải biểu đồ số liệu.

#### 5.1.2. Số lượt tải và thời điểm đến

Trên cạnh $e$, gọi $C_e$ là công suất mỗi ngày. Số lượt tải cho lô $Q$:

$$
B_e=\left\lceil\frac{Q}{C_e}\right\rceil.
$$

Dấu $\lceil x\rceil$ là làm tròn lên. Hàng 200, công suất 80 cần 3 lượt, vì 2 lượt chỉ tải được 160.

Gọi $s_e$ là ngày bắt đầu dùng slot, $d_e$ là thời gian di chuyển và $\delta_e$ là độ trễ do trạng thái tuyến. Thời điểm đến theo quy tắc M4 có dạng:

$$
a_e=s_e+(B_e-1)+d_e+\delta_e.
$$

Ví dụ bắt đầu ngày 10, cần 3 lượt, đi 5 ngày và không có độ trễ bổ sung thì đến ngày 17. Trong quy ước này lượt đầu xảy ra ngay ngày bắt đầu. Nếu ngày 10 đã có đặt chỗ, $s_e$ phải dời sang khoảng khả dụng trước khi tính.

#### 5.1.3. Thuật toán tìm đường

M4 dùng tìm đường kiểu Dijkstra theo thời điểm đến sớm nhất:

1. Gán thời điểm tại nguồn bằng ngày hiện tại, các đỉnh còn lại là chưa biết.
2. Chọn đỉnh chưa xử lý có thời điểm tới sớm nhất.
3. Với mỗi cạnh mở đi tiếp, tìm khoảng slot không giao nhau với đặt chỗ trước.
4. Tính thời điểm đến đỉnh kế tiếp; cập nhật nếu sớm hơn phương án đã biết.
5. Lưu đỉnh trước và lịch chặng để dựng lại tuyến.

Các slot là công suất dùng chung hai chiều. Chia một lô lớn thành nhiều lô không được tự nhân công suất. Hàng đã dispatch mang lịch riêng; nâng cấp tuyến không tự tính lại và “dịch chuyển” hàng đang đi nếu luật chưa quy định việc đó.

Ở graph nhỏ hiện tại, implementation chọn đỉnh bằng quét mảng và xét interval đặt chỗ; chưa dùng priority queue cho toàn bộ bài toán. Cách này không phải solver tối ưu luồng đa hàng hóa toàn quốc. Cần kiểm FIFO/tính đơn điệu của thời điểm tới nếu mở rộng quy tắc chờ để giữ điều kiện cho thuật toán tìm đường.

#### 5.1.4. Manifest và đối soát

Manifest là bản ghi đoàn hàng: nguồn, đích, hàng, lượng, ngày đi/đến, tuyến và lịch các chặng. Hàng rời kho được tính vào manifest cho tới khi giao. Sau giao, kho đích tăng đúng lượng và manifest được đánh dấu hoàn tất.

## 6. Bài toán học hành vi

### 6.1 Policy, actor, critic và GRU

Trong world, actor là chủ thể ra quyết định. Trong thuật toán actor–critic, **actor network** chọn action; **critic** ước lượng lợi ích tương lai để hỗ trợ học. Một actor network được nhiều NPC dùng chung, không tương đương một model riêng cho mỗi NPC.

GRU (Gated Recurrent Unit) là layer recurrent giữ vector memory. Gate quyết định phần thông tin cũ giữ lại và phần mới cập nhật. Nó giúp khai thác lịch sử nhưng không tự là ký ức đầy đủ của đời người, database quan hệ hoặc bằng chứng về reasoning. Dự án còn phải giữ facts/knowledge rõ ở domain.

```text
(logits_i, h_i_next) = actor_theta(observation_i, h_i)
action_i ~ Categorical(masked_logits_i)
```

Mạng pilot hiện có state encoder → GRU64 → candidate scorer; mỗi observation gồm 128 state values và 32×12 candidate features. Ba họ model governance/economic/civilian chia sẻ weights trong họ. Memory/sampling riêng từng ID; đổi họ reset memory vai trò nhưng không reset cá thể/tài sản.

#### 6.1.1. Đề bài học quyết định

Actor cần chọn làm gì dựa trên trạng thái của mình, cơ hội tiếp cận và mục tiêu. Hai actor ở cùng vùng không nhất thiết quyết định giống nhau. Engine phải kiểm luật, nhưng không được dùng một quy tắc chiến lược cứng cho NPC rồi gọi đó là hành vi đã học.

Một cách hình thức hóa là trò chơi Markov đa tác nhân có quan sát từng phần. “Markov” nghĩa trạng thái đầy đủ chứa dữ liệu cần cho bước tiếp theo; “quan sát từng phần” nghĩa mỗi actor chỉ được nhận một phần dữ liệu đó. Các actor còn làm thay đổi hoàn cảnh của nhau. Đây là cách phân tích bài toán, không có nghĩa implementation đã giải chính xác bài toán suy luận niềm tin hay tìm cân bằng trò chơi.

Đặt $O_i$ là hàm dựng quan sát cho actor $i$, và $K_t^i$ là thông tin actor đó biết:

$$
o_t^i=O_i(S_t,K_t^i).
$$

Ví dụ cùng có 100 đồng, người biết có chỗ học nghề và người chưa nhận tin đó có thể nhận observation khác nhau. Chất lượng che thông tin phải được audit trên từng feature; công thức không tự chứng minh code không lộ dữ liệu.

#### 6.1.2. Ba họ policy và chia sẻ trọng số

Runtime sandbox có ba họ: Governance cho quyết định cai trị, Economic cho hoạt động kinh tế và Civilian cho cư dân. Đây là nhóm model theo loại quyết định, không phải ba đẳng cấp xã hội. Trường `Person.family` hiện có nghĩa họ policy, không phải dòng họ.

Mỗi họ dùng một bộ trọng số chung. Cá nhân giữ hidden state và sampling counter riêng. Trọng số là tham số dài hạn của mạng; hidden state là bộ nhớ tạm cập nhật sau mỗi quyết định. Người A học cùng “bộ quy tắc neural” với B nhưng có lịch sử đầu vào và chuỗi lấy mẫu khác nhau.

#### 6.1.3. Observation và candidate

Model `realm_model.py` nhận 512 float:

$$
512=128+32\times12.
$$

128 phần tử đầu là context; phần sau là 32 candidate, mỗi candidate có 12 feature. Candidate là một hành động cụ thể kèm target/điều khoản, không chỉ động từ. “Mua xơ ở vùng hiện tại” và “Mua thực phẩm ở vùng hiện tại” có thể là hai candidate dù cùng loại Buy.

Context gồm biến cá nhân, goal, vai trò, giá công khai, kho cục bộ và một số tín hiệu khác. Feature candidate gồm loại, target, amount, term và mask. Input được chuẩn hóa bằng hệ số cố định rồi clamp trong mạng; clamp giới hạn mỗi phần tử vào [-10,10], không phải thuật toán chuẩn hóa thống kê học từ toàn bộ dataset.

Danh sách hiện cố định 32 slot và được core dựng theo luật. Đó không phải bộ tìm mọi đối tác/nghề/hôn nhân có thể có. Việc mở action space xã hội sẽ đòi hỏi thiết kế candidate retrieval và observation mới.

#### 6.1.4. Encoder, GRU và scorer

Kiến trúc hiện tại:

```text
Context 128 → Linear(128,64) → tanh → GRU(64,64)
                                      ├→ Linear → value
                                      └→ ghép với 12 feature của từng candidate
                                           → Linear(76,64) → tanh → Linear(64,1)
                                           → 32 logits
```

Linear là phép biến đổi có trọng số và bias. `tanh` nén giá trị về khoảng $(-1,1)$. GRU, viết đầy đủ Gated Recurrent Unit, là mạng hồi tiếp dùng gate để quyết định mức giữ bộ nhớ cũ và mức nhận thông tin mới (Cho et al., tài liệu tham khảo). Scorer là mạng chấm điểm cùng một công thức cho từng candidate.

Gọi $x_t$ là context 128 chiều; $W_e,b_e$ là trọng số/bias của encoder:

$$
e_t=\tanh(W_ex_t+b_e).
$$

$e_t$ là vector 64 chiều được học, không phải 64 khái niệm được gán tên thủ công. Một thay đổi về tiền hoặc sức khỏe có thể ảnh hưởng nhiều phần tử của vector này.

Để giải thích GRU, gọi $h$ là memory cũ, $r,z$ là hai vector gate có phần tử từ 0 đến 1, và $n$ là nội dung memory đề xuất. Các phép biến đổi affine $A_r,B_r,A_z,B_z,A_n,B_n$ dưới đây là các hàm tuyến tính có bias được học:

$$
r=\sigma(A_r(e_t)+B_r(h)),\qquad
z=\sigma(A_z(e_t)+B_z(h)),
$$

$$
n=\tanh(A_n(e_t)+r\odot B_n(h)),\qquad
h'=z\odot h+(1-z)\odot n.
$$

Hàm sigmoid $\sigma$ tạo giá trị gate trong $(0,1)$; $\odot$ là nhân từng phần tử; $h'$ là memory mới. Gate $r$ điều chỉnh ảnh hưởng của memory cũ khi tạo nội dung đề xuất. Gate $z$ quyết định giữ cũ hay nhận mới. Ví dụ ở một chiều, $h=0.8,n=0.2,z=0.75$ thì $h'=0.65$: giữ phần lớn giá trị cũ. Cách đặt reset gate phải khớp GRU PyTorch đang dùng, không tự đổi sang một biến thể đại số khác (quy ước GRU PyTorch).

Sau GRU, mỗi candidate $j$ có vector feature $c_j$. Ghép hai vector, ký hiệu bằng dấu chấm phẩy:

$$
\ell_j=w_2^\top\tanh(W_1[h';c_j]+b_1)+b_2.
$$

$\ell_j$ là logit, tức điểm thực chưa chuẩn hóa; $W_1,b_1,w_2,b_2$ là trọng số/bias scorer. Vector ghép dài $64+12=76$. Cùng một memory có thể cho điểm cao hơn cho Work so với Study tùy feature cơ hội; logit chưa phải xác suất hoặc tiền lời.

Critic là nhánh dự đoán value dùng trong training. Value là dự đoán reward tương lai còn lại, không phải hàm luật quyết định tiền thật trong world. Mạng có khoảng 38 nghìn tham số mỗi họ; đây là phép đếm theo kích thước layer, không phải benchmark năng lực.

#### 6.1.5. Mask và categorical sampling

**Đề bài.** Policy không được mua hàng khi thiếu tiền hoặc gửi trẻ chưa đủ tuổi vào hành động bị luật cấm.

Core tạo mask $m_j$ bằng 1 nếu candidate hợp lệ, 0 nếu không. Với logits $\ell_j$, xác suất chọn:

$$
\pi_j=\frac{m_j\exp(\ell_j)}{\sum_k m_k\exp(\ell_k)}.
$$

$\exp$ là hàm mũ; mẫu số cộng trọng số của mọi candidate hợp lệ để tổng xác suất bằng 1. Ví dụ trọng số mũ là 2,3,5 nhưng candidate thứ hai bị cấm, xác suất trở thành 2/7,0,5/7. Mask là luật được core áp dụng, không chứng minh policy đã tự học mọi giới hạn pháp lý.

Categorical sampling là rút một hành động theo các xác suất này. Argmax là luôn chọn điểm cao nhất. Hai chế độ có thể tạo chất lượng khác nhau; runtime dùng sampling và có audit tương ứng. RNG cá nhân được xác định từ seed, ID và sampling counter đã lưu.

### 6.2 Cá nhân hóa và năng lực: mục tiêu, chưa chứng nhận

Hướng thiết kế là condition policy theo hoàn cảnh, knowledge, tính cách/mục tiêu, năng lực và trạng thái cá nhân. Tính cách bền khác mood/body nhanh; khác mục tiêu không đồng nghĩa kém thông minh. Người thích thời gian gia đình có thể từ chối tăng ca một cách hợp lý; người ít kinh nghiệm có thể dự báo sai dù rất chăm chỉ.

Không đủ chỉ thêm vector traits. Phải có bài đo ghép cặp: giữ hoàn cảnh, thay trait/năng lực có chủ đích, đo hành vi và kết quả qua nhiều seed; ablation bỏ input để kiểm tác động. Sampling quá nhiễu không thay cho cá tính; reward chung GDP không phù hợp mọi actor. Cụ thể hóa determination, mô hình competence và weights là công việc spec/evaluation tiếp theo, chưa có implementation hoàn chỉnh.

#### 6.2.1. Kiểm chứng cá nhân hóa trước khi mở training lớn

Một cách phân rã policy đích là:

$$
a_t^i\sim\pi_{\theta_f}(\cdot\mid o_t^i,h_t^i,
\underbrace{g_i,p_i}_{\text{mục tiêu/tính cách}},
\underbrace{s_i}_{\text{năng lực}},
\underbrace{B_i(\tau_t)}_{\text{cơ thể}}).
$$

Đây là phân rã khái niệm; pilot đang gói các feature trong observation, chưa có bằng chứng đủ cho từng thành phần hay một vector determination đã hoàn thiện. $\theta_f$ shared theo họ; $h^i$ và RNG thuộc cá nhân. Tăng sampling temperature chỉ thay độ phân tán, không chứng minh actor kém chuyên môn hay coi trọng gia đình hơn.

Để đo ảnh hưởng một trait $p$, giữ initial condition và tình huống cơ hội $x_k$, chạy hai điều kiện rồi đo:

$$
\Delta_p=\frac1K\sum_{k=1}^K
\left[Y(x_k,p_{\rm high})-Y(x_k,p_{\rm low})\right].
$$

$Y$ phải là metric định trước như tỷ lệ nhận ca thêm, giờ nghỉ thực, tỷ lệ vi phạm cam kết hoặc chất lượng kết quả nhiều bước; không mặc định mọi metric đều cần tăng. Báo CI và kết quả theo bối cảnh. Ablation bỏ trait input kiểm mạng có dùng thông tin đó không; kiểm cơ học riêng để phân biệt thay hành vi với việc core tự tăng năng suất.

Chưa có tỷ lệ “bao nhiêu NPC quyết định tệ” được duyệt. Cần đánh giá viability, survival/default, phân phối lựa chọn và sự khác biệt theo mục tiêu/cơ hội. Một lựa chọn ít tiền hơn có thể hợp mục tiêu khác; lỗi chất lượng là reward hack, phi lý với thông tin đã biết, hoặc xã hội sụp vì luật/learning sai chứ không phải mọi hành động khác tối ưu tiền.

### 6.3 Dữ liệu và PPO

Nguồn hiện thực: [realm_binding.cpp](../bindings/python/realm_binding.cpp), [realm_model.py](../ai/training/realm_model.py), [realm_train_v6.py](../ai/training/realm_train_v6.py). Tham số dưới đây thuộc pilot v6; chưa chuyển thành trainer cho world M5.

#### 6.3.1. Reward phải đo điều gì?

**Đề bài.** Game không có sẵn đáp án tốt nhất cho mọi quyết định. Cần một mục tiêu số để phân biệt kết quả tốt hơn/kém hơn, nhưng mục tiêu không được thưởng sai như chỉ tăng cash bằng cách vay vô hạn.

Utility $U_i(S)$ là hàm đánh giá state đối với actor $i$. Actor kinh tế quan tâm tài sản ròng, sức khỏe và dinh dưỡng; cư dân có thêm kỹ năng/xã hội; actor cai trị có dự trữ ngân khố, đời sống dân cư, bất mãn và hạ tầng. Đây là lựa chọn thiết kế, không phải định luật tự nhiên về con người.

Tài sản ròng được dùng trong một số utility:

$$
W_i=\operatorname{cash}_i+\sum_g p_g^{\mathrm{ref}}q_{i,g}-\operatorname{debt}_i.
$$

$q_{i,g}$ là lượng hàng cá nhân giữ; $p_g^{\mathrm{ref}}$ là giá dùng định giá; tổng chạy qua các loại hàng. Nếu có 100 tiền, hàng trị giá 50 và nợ 80 thì $W_i=70$. Vay 100 với tổng nghĩa vụ 104 làm cash tăng 100 nhưng tài sản ròng giảm 4, không tạo 100 utility tài sản miễn phí.

Ví dụ utility kinh tế hiện có:

$$
U_{\mathrm{economic}}=W_i/100+0.05H_i+0.01N_i-20\mathbf1_{\mathrm{dead}}.
$$

$H_i$ là health, $N_i$ là nutrition; $\mathbf1_{\mathrm{dead}}$ bằng 1 khi đã chết, 0 khi sống. Với $W=100,H=N=100$ và còn sống, utility bằng 7. Đây không phải 7 đồng; nó là điểm tổng hợp phục vụ learning.

Reward v6 là thay đổi utility chuẩn hóa:

$$
r_t=\frac{U(S_{t+1})-U(S_t)}{\max(1,|U(S_0)|)}.
$$

$S_0$ là state đầu episode; dấu trị tuyệt đối tránh chuẩn hóa bằng số âm; `max` tránh chia cho 0. Utility đầu kỳ 100 và tăng từ 120 lên 125 tạo reward 0,05. Vì mẫu số cố định, cộng reward hết episode sẽ bằng thay đổi utility cuối–đầu chia cùng mẫu số. Code dùng tính chất này để đối soát.

Đây là hiệu utility ở discount 1, không tự chứng minh mọi dạng potential-based reward shaping đều giữ nguyên policy tối ưu khi đổi luật, terminal condition hoặc mục tiêu.

#### 6.3.2. Actor–critic và sai số TD

Actor tạo phân phối action. Critic dự đoán return còn lại. Gọi $V_t$ là dự đoán trước bước, $r_t$ là reward quan sát, $d_t$ bằng 1 nếu episode kết thúc, và $\gamma$ là hệ số discount:

$$
\delta_t=r_t+\gamma(1-d_t)V_{t+1}-V_t.
$$

$\delta_t$ là sai số temporal difference, tức kết quả tốt hơn/kém hơn dự đoán. Ví dụ chưa kết thúc, reward 2, value tiếp theo 8, value cũ 9 và gamma 1 thì delta bằng 1. Khi đã kết thúc, $1-d_t=0$, nên không nhận value tương lai từ episode kế tiếp.

#### 6.3.3. GAE: phối hợp bằng chứng qua nhiều bước

GAE, Generalized Advantage Estimation, ước lượng advantage: một hành động tốt hơn mức kỳ vọng của policy bao nhiêu. Nó gom các sai số TD gần và xa bằng trọng số giảm dần (Schulman et al., GAE). Công thức truy hồi:

$$
\widehat A_t=\delta_t+\gamma\lambda(1-d_t)\widehat A_{t+1}.
$$

$\widehat A_t$ là advantage ước lượng; $\lambda$ điều khiển đóng góp từ các bước sau. V6 dùng gamma 1 và lambda 0,95. Nếu delta hiện tại là 1 và advantage bước sau là 2, chưa terminal, kết quả hiện tại là $1+0.95\times2=2.9$. Tính ngược từ cuối rollout để có các giá trị này.

GAE hỗ trợ cân bằng độ nhiễu và độ lệch của ước lượng. Nó không lập cây kế hoạch tương lai cho NPC; nó là thuật toán xử lý trải nghiệm để cập nhật trọng số.

#### 6.3.4. PPO: cập nhật có giới hạn động lực thay đổi quá mạnh

PPO, Proximal Policy Optimization, là thuật toán policy gradient dùng rollout từ policy cũ rồi tối ưu một mục tiêu surrogate qua nhiều minibatch (Schulman et al., PPO).

Gọi $\theta$ là trọng số đang cập nhật, $\theta_{old}$ là trọng số đã tạo rollout; $a_t$ là action đã lấy mẫu; và $z_t=(o_t,h_t)$ là context quan sát–memory. Tỷ lệ xác suất:

$$
\rho_t(\theta)=\frac{\pi_\theta(a_t\mid z_t)}{\pi_{\theta_{old}}(a_t\mid z_t)}.
$$

Xác suất cũ 0,2 và mới 0,3 cho ratio 1,5. Tỷ lệ này cho biết action trong dữ liệu đang được policy mới ưa hơn bao nhiêu, không phải lợi nhuận của action.

Mục tiêu actor được tối đa hóa:

$$
L^{clip}=\mathbb E_t\left[\min\left(\rho_t\widehat A_t,
\operatorname{clip}(\rho_t,1-\epsilon,1+\epsilon)\widehat A_t\right)\right].
$$

$\mathbb E_t$ là trung bình trên mẫu; $\epsilon=0.2$ trong v6; `min` lấy biểu thức nhỏ hơn. Với advantage dương bằng 1 và ratio 1,5, nhánh unclipped là 1,5 còn nhánh clipped là 1,2, nên chỉ nhận 1,2. Điều đó giảm phần lợi ích tối ưu nhận thêm từ việc tiếp tục đẩy action tốt lên quá mạnh. Advantage âm cần đọc cùng hai nhánh, không áp dụng lập luận chỉ cho advantage dương vào mọi trường hợp.

Clipping không phải ràng buộc cứng rằng mọi xác suất mới chỉ được thay đổi 20%; các tham số chung vẫn có thể tác động đến nhiều action. Nó cũng không bảo đảm hội tụ tới chiến lược tốt toàn xã hội.

#### 6.3.5. Value loss, entropy và optimizer

Training đồng thời dạy critic và giữ mức khám phá. Mục tiêu tối thiểu hóa hiện tại:

$$
\mathcal L=-L^{clip}+0.5\operatorname{MSE}(V,\widehat R)-0.01\mathcal H(\pi).
$$

Ở đây MSE là trung bình bình phương sai lệch giữa value và target return $\widehat R$; target return trong code bằng advantage chưa chuẩn hóa cộng value cũ. Entropy $\mathcal H$ đo độ phân tán phân phối action. Dấu trừ entropy khuyến khích policy tránh trở nên tất định quá sớm. Các hệ số 0,5 và 0,01 là trọng số implementation.

Ví dụ critic dự đoán 3 còn target 5 thì sai số bình phương của mẫu là 4, đóng góp có hệ số là 2. Nếu entropy là 0,8 thì term entropy đóng góp -0,008. Các con số minh họa cách cộng loss, không phải một kết quả training.

Adam cập nhật trọng số bằng gradient có ước lượng moment bậc một và hai. Learning rate v6 là 0,0003; gradient norm bị clip ở 0,5 để hạn chế bước gradient quá lớn. Khởi tạo orthogonal áp dụng cho Linear trong model, không nên diễn giải thành mọi ma trận GRU đều đã được custom-initialize orthogonal.

#### 6.3.6. Recurrent training và truncated BPTT

BPTT là Backpropagation Through Time: truyền gradient qua chuỗi tính toán hồi tiếp. Rollout 64 quyết định được chia thành đoạn 16 quyết định để giảm chi phí bộ nhớ. Hidden state khởi đầu đoạn được lấy từ rollout, và reset mask ngăn gradient/memory nối nhầm hai episode.

Giới hạn BPTT 16 là giới hạn chuỗi truyền gradient trong mỗi đoạn, không khẳng định NPC chỉ nhớ đúng 16 quyết định. Thông tin có thể đi qua hidden state lâu hơn, nhưng khả năng học phụ thuộc xa vẫn phải kiểm chứng.

| Tham số v6 | Giá trị đọc từ code/manifest |
|---|---:|
| Parallel worlds | 32 theo cấu hình phổ biến |
| Rollout | 64 quyết định |
| BPTT | 16 quyết định |
| Epoch trên rollout | 3 |
| PPO clip | 0,2 |
| Gamma / GAE lambda | 1 / 0,95 |
| Optimizer | Adam, learning rate 0,0003 |
| Value / entropy coefficient | 0,5 / 0,01 |
| Gradient norm clip | 0,5 |
| Họ policy | Governance, Economic, Civilian |
| Training seeds | 1, 2, 3 trong protocol v6 |

#### 6.3.7. Partner và giới hạn multi-agent learning

`RealmBatch` tạo world nhỏ và chọn focal actor để train. Các actor khác dùng partner controller/sampling đã ghi nhãn trong training: civilian có Work/Study, các họ khác sample legal action theo protocol. Đây không phải bằng chứng mọi NPC đã cùng tự học hoặc đã đạt cân bằng game-theoretic. Khi runtime ghép cả ba learned policy, môi trường hành vi khác partner training; cần đánh giá co-play riêng.

#### 6.3.8. Chuyển từ pilot sang world M5


Training gọi cùng C++ core qua pybind11. World khởi tạo hợp lệ, policy chọn action, core thực hiện/tiến thời gian, trajectory ghi outcome rồi learner cập nhật. PPO dùng rollout mới của policy cùng iteration; trace cũ dùng replay/chẩn đoán, không tự coi là tập on-policy hiện tại.

Trajectory cần actor/decision, observed data/mask, action/log-prob/value, reward, elapsed game time, memory/reset, terminated/truncated, env/reward/model/partner versions và seed. Hành động dài và lịch không đều yêu cầu discount theo thời gian đã qua, ví dụ γ^(Δt/t_ref); đây là contract cần test khi chuyển trainer mới, không mặc định trainer pilot đã xử lý đầy đủ.

PPO tối ưu surrogate clipped để tránh bước đổi phân bố quá mạnh:

```text
ratio = pi_theta(action|obs,memory) / pi_old(action|obs,memory)
L_clip = E[min(ratio × A, clip(ratio,1-epsilon,1+epsilon) × A)]
```

Advantage A ước lượng hành động tốt hơn kỳ vọng bao nhiêu; critic/value loss hỗ trợ ước lượng, entropy hỗ trợ exploration. Truncated BPTT huấn luyện chuỗi memory theo đoạn và phải reset đúng tại episode boundary. Phân biệt chết/kết thúc thật với cắt rollout, tránh bootstrap sai. Các tham số pilot xem source/manifest phiên bản, không sao chép mù sang world M5.

Hiện `RealmBatch` train một focal actor trong world nhỏ, đối tác còn là controller test hoặc random legal tùy họ. Runtime lại chạy nhiều learned policies. Đây là distribution gap cần xử lý bằng partner populations/checkpoints/co-play. Centralized training/decentralized execution có thể được thử, nhưng xã hội có mixed motives nên không mặc định dùng team reward hoặc áp thành công cooperative MAPPO cho toàn quốc.

Curriculum từ cá nhân/hộ/cơ sở → vùng → liên vùng → quốc gia, giữ train/validation/test rời nhau theo seed, đối tác và bối cảnh. Đánh giá đúng categorical sampling mà app dùng. Reward dựa kết quả, tài sản sau nợ và nghĩa vụ thật; không thưởng nhấn Work, spam hợp đồng hoặc chuyển tiền vòng. Invariant lỗi dừng run và giữ reproducer, không loại mẫu rồi train tiếp.

GPU tăng tốc learner chưa chắc tăng tốc rollout. Đo CPU simulation và learner riêng trước khi chọn tài nguyên; core/Python portable để chuẩn bị Kaggle nhưng chưa là bằng chứng đã chạy notebook/GPU. Production train sau freeze M6; mọi run dài có ngân sách riêng.

## 7. Runtime, đa luồng và giảm tải

### 7.1. Mô hình chi phí và bộ nhớ

Với $N$ cá nhân, hidden size $H=64$ và float32 bốn byte, riêng memory policy cần:

$$
M_h=4NH\ \text{byte}.
$$

Một triệu cá nhân tương ứng 256 MB thập phân chỉ cho hidden state, chưa tính world/graph/history/allocator. Giữ toàn bộ observations 512 chiều đồng thời sẽ cần $4N\times512$ byte, tức 2,048 GB thập phân. Batch bounded giảm phần buffer này xuống theo $B$, không làm biến mất tổng chi phí dựng observation qua cả chu kỳ.

Nếu mỗi actor có một quyết định mỗi $c$ ngày, chạy $v$ ngày game/giây thực đòi trung bình:

$$
\Lambda=\frac{Nv}{c}\quad\text{decisions/giây thực}.
$$

Một triệu người, $c=30$, $v=1$ cần khoảng 33.333 decisions/s. Nhiều cadence dùng tổng $\sum_r N_rv/c_r$; shock làm arrival rate tăng đột ngột. Đây là yêu cầu tính từ giả định, không phải throughput đã đo.

Chi phí một chu kỳ cần tách:

$$
T_{\rm cycle}=T_{\rm due}+T_{\rm retrieve}+T_{\rm obs}
+T_{\rm infer}+T_{\rm validate/commit}+T_{\rm processes}
+T_{\rm view}+T_{\rm sync}.
$$

Không cộng các thời gian wall-clock bị chồng lấp như thể tuần tự; khi có overlap phải đo critical path và work/CPU time riêng. Công thức trên là phân rã đường tuần tự ban đầu. Tối ưu inference không đủ nếu due scan, truy vấn khoản vay hoặc view là phần lớn chi phí.

Quan hệ sparse cần $O(N+E)$ storage thay vì dense $O(N^2)$. Nếu một event queue heap lưu $Q$ việc, push/pop thường $O(\log Q)$; đây là lựa chọn thiết kế, không khẳng định Realm hiện dùng queue cho mọi quá trình. Lazy update phải giữ last-update và đi qua mọi ranh giới có ý nghĩa, không chỉ nhân một rate cuối kỳ cho toàn interval khi state đã đổi giữa chừng.

### 7.2. Thread ownership và giới hạn tăng tốc

Runtime hiện tập hợp actor đến lượt, dựng observation/candidates, infer batch tối đa 128, sampling cá nhân và revalidate trước commit. `due()` còn quét people, economic cadence 7 ngày, các họ khác 30 ngày và dàn theo ID; đây là implementation pilot, không phải nhịp quyết định đã chốt cho timeline mới.

`native/policy.cpp` cấu hình ORT 2 intra-op threads, 1 inter-op và sequential graph. Binding training giải phóng Python GIL và dùng `std::jthread` để chạy các world độc lập, mặc định 4 workers. Đó không phải một world đã chia mọi mutate ra nhiều thread; đường Realm/Campaign/adapter chính còn thực hiện phần lớn tuần tự.

Ưu tiên tối ưu: đo bottleneck → bỏ truy vấn/quét dư → index/time queue → compact hot/cold data → bounded batch → song song phần độc lập. Thiết kế đích đọc snapshot nhất quán, tính ý định song song, resolve tranh chấp theo contract rồi commit có kiểm soát. Không dùng thứ tự thread hoàn thành làm quy tắc giành vốn/công cụ. Tổng worker game và ORT cùng nằm trong ngân sách CPU; thêm thread có thể tăng contention.

Lưu state O(N) và relations O(E) khác chi phí quyết định, dựng candidates, validation và history. Một triệu record đứng yên không là benchmark agency. Không giảm fidelity ngoài camera; không bỏ aging/deadline để đạt tốc độ. Cache observation chỉ hợp lệ khi chứng minh điều kiện tương đương và không gộp danh tính/sampling.

## 8. Save, schema, replay và tương thích

### 8.1. Những version khác nhau giải quyết những vấn đề khác nhau

| Contract | Khi phải xem lại | Lỗi nếu chỉ kiểm shape |
|---|---|---|
| Save | Đổi layout, cursor, references, settlement hoặc lịch | Load mất nghĩa vụ hoặc phát output lần nữa |
| Action | Đổi loại/target/đơn vị/terms/sequence | Cùng chỉ số action lại mang nghĩa khác |
| Observation | Đổi feature, normalization, knowledge/retrieval | Policy diễn giải một giá trị thành biến khác |
| Reward | Đổi objective, normalization, horizon | So checkpoint trên hai mục tiêu khác nhau |
| Model/bundle | Đổi kiến trúc, hidden, weights, preprocessing | Memory và logits không còn hợp contract |
| Environment/partner | Đổi world process, đối tác và cadence | Training distribution khác runtime dù tensor vẫn vừa |

Những contract này cần manifest/source lock và guard cụ thể. Một số phiên bản có thể được đóng chung trong bundle, nhưng không được suy save-v7 đồng nghĩa policy đã thành realm-v7.

### 8.2. Snapshot, load nguyên tử và replay

Snapshot pilot v6 dùng text scalar `POLYEC_REALM_6`, court tail và bundle; Campaign bọc `POLYEC_M4_1`, runtime lưu memory và identity model. T1a thêm save `POLYEC_REALM_7` cho timeline; các thay đổi clock/save kế tiếp đọc contract active. Định dạng binary/chunk trong roadmap vẫn là đích, chưa coi đã chuyển đổi.

Ghi file tạm/checksum/backup theo platform, load vào temporary, kiểm count/range/reference/invariant/bundle trước thay live state. Save tiến độ mới cần game time, pending commands, obligations, escrow, RNG, memory và resolved IDs để không settlement lại. Model hash/version có ý nghĩa semantics, không chỉ shape.

Thay save/action/observation/reward phải kiểm kê Campaign wrapper, Python binding, native/model loader, GDExtension và fixtures. Không sửa manifest/checkpoint cũ để giả tương thích; migration phải có test, không hứa mọi save cũ đều load được. Replay bit-identical chỉ công bố trong phạm vi runtime/platform đã kiểm chứng.

## 9. Phương pháp thực nghiệm và bằng chứng

### 9.0. Từ câu hỏi đến kết luận có thể kiểm tra

Một thí nghiệm cần ghi hypothesis, biến can thiệp, biến giữ cố định, metric/đơn vị, seeds, đối tác, horizon và cách xử lý ending trước khi chạy. Dùng paired seeds chỉ hữu ích khi các cặp thực sự cùng điều kiện đầu; RNG có thể tiêu thụ khác nhau sau can thiệp nên không hứa mọi shock tương lai tự khớp chỉ nhờ cùng seed.

Với kết quả policy và baseline trên tình huống $k$:

$$
d_k=J_k^{P}-J_k^{B},\qquad \bar d=\frac1K\sum_kd_k.
$$

Paired bootstrap lấy lại các chỉ số $k$ có hoàn lại và tính $\bar d$ nhiều lần để ước lượng CI. Nếu metric được định hướng “cao hơn tốt hơn”, cận dưới CI lớn hơn 0 hỗ trợ kết luận cải thiện trên phân bố đã thử. Noninferiority margin $\delta$ kiểm cận dưới không nhỏ hơn $-\delta$; nó khác kết luận cải thiện. Không gán cùng margin 5% cho mọi metric khác đơn vị.

Tách số training seeds và số evaluation episodes: 100 episode từ một checkpoint không thay được ba lần train độc lập. Validation chọn checkpoint; holdout không dùng chọn lại reward/seed/model. Những run đã xem kết quả để sửa thiết kế trở thành dữ liệu phát triển, không tiếp tục gọi là holdout chưa thấy.

Với cadence không đều, contract trainer đích cần ghi thời gian đã qua. Một lựa chọn discount cần review là:

$$
\Gamma_t=\gamma_{\rm ref}^{\Delta\tau_t/\tau_{\rm ref}},\qquad
\delta_t=r_t+\Gamma_t b_tV_{t+1}-V_t.
$$

$b_t$ là bootstrap mask: terminal thật ngắt value; cắt rollout do giới hạn lấy mẫu thường cần value của observation cuối trước reset nếu quá trình còn tiếp tục. Việc ngắt GAE qua episode/reset là mask khác và phải định nghĩa riêng. Pilot v6 dùng gamma 1 và một `done` gộp chết/horizon; công thức tổng quát này chưa chứng nhận pilot đã xử lý terminated/truncated tách biệt. Chọn horizon là terminal mục tiêu hay chỉ truncation cũng là contract phải công bố.

### 9.1 Thiết kế đánh giá

- Unit/integration: invariant, numerical fixtures, wrong ID/authority/consent, stale/duplicate, shortage, boundary và atomic reject.
- Replay/parity: cùng command/seed/inputs qua C++/Python; Torch/ORT/native logits/memory trong tolerance công bố; load giữa tiến độ và malformed input.
- Policy: baseline/initial/learned có nhãn, paired seeds khi phù hợp; validation chọn checkpoint, test holdout để đánh giá; nhiều training seeds và khoảng tin cậy. So cả distribution hành vi, default/survival và outcome, không chỉ mean reward.
- Scale: cấu hình CPU/RAM/OS/runtime/hash, workload normal/stress, actor đến lượt/thực xử lý, p50/p95/p99/backlog, render/input/RAM/save. Microbenchmark, controller-only và end-to-end không tráo nhau.

### 9.2 Kết quả đã ghi, không phải phép đo mới của tài liệu này

| Evidence | Kết quả ghi nhận | Giới hạn |
|---|---|---|
| [M3](reports/M3-evidence.md) | Pipeline recurrent PPO/ONNX, replay/parity; v6 khoảng 2,63 triệu decisions; profile A 10k p95 ngày 5,89 ms, peak native 44,6 MiB trên máy tham chiếu | Chưa gồm render/input/save trong phép đo đó; categorical audit R1 chưa đạt, default cao, R2/R3 chưa chứng minh đầy đủ. |
| [M4](reports/M4-evidence.md) | Native/UI/core/binding và package Windows; Hắc Sơn/xưởng/logistics, UI+day p95 4,910 ms; click p95 47,446 ms ở workload ghi trong report | Controller có nhãn, không ONNX trong phép đo, không profile B/C hoặc máy sạch. |
| [M5 membership](reports/M5-progress.md) | Index/guardian/load/reject, Windows CTest 5/5 ở revision đó | Không hộ động/tài khoản/thừa kế hoàn chỉnh. |
| [Core repair](reports/copilot-core-repair-2026-10-17.md) | Giữ refactor manager/WorldState, sửa payroll và source integrity; 6/6 Windows | Không build app/binding/ONNX mới cho lát đó. |
| [Activity 18/10](reports/M5-activity-evaluator-2026-10-18.md) | Core suite 7/7 và targeted activity 1/1 sau test-gate fix được ghi trong report | Pure evaluator; chưa world integration hoặc profile mới. |

Các con số là lịch sử theo workload và revision trong report, không bảo đảm HEAD hiện tại. Trạng thái chấp nhận/dirty mới nhất đọc CURRENT_TASK và kiểm git khi tiếp tục.

## 10. Hạn chế và câu hỏi mở

M5 chưa có đầy đủ dynamic household/accounts/rights, cơ sở/hợp đồng, marriage/estate/claim/institutions và quốc gia đủ scope. Pilot production còn gắn nghề vào vùng; attention chưa nối attendance/body/ledger. Quality gates RL còn mở và việc thêm hệ thống tăng khó khăn exploration/credit assignment. Memory và conditioning chưa chứng minh cá tính hoặc trí lược tương ứng stat.

Event/history cần ngân sách; cá nhân hóa không cho phép lưu mọi cặp quan hệ hoặc mọi suy nghĩ. Những tối ưu tăng throughput phải test không đổi quyền và kết quả. T1 cần thống nhất clock/units trước; công thức cơ thể và reward mới cần plan review. Đây là hạn chế thật để định hướng thí nghiệm, không phải lý do thay learned policy bằng chiến lược viết tay.

## 11. Tổ chức mã và môi trường làm việc

| Đường dẫn | Điểm vào |
|---|---|
| `core/include/polyec`, `core/src`, `core/tests` | Realm, Campaign, activity, sim, helpers, invariants/tests |
| `bindings/python/realm_binding.cpp` | Core cho train/eval, batched worlds/workers |
| `ai/training`, `ai/evaluation`, `ai/tests` | Policy/trainer/export, quality evaluation, replay/parity |
| `native/realm_runtime.cpp`, `native/policy.cpp` | Shared ONNX models, batch/memory/sampling, guards |
| `gdext/polyec_session.cpp` | Adapter native snapshot/command cho Godot |
| `godot_project/ui`, `godot_project/tests` | UI/map/labs và smoke harness; scene khởi động `main.tscn` |
| `godot_project/models`, `fonts`, `licenses` | Runtime assets và license cần đóng gói |
| `scripts`, `.github/workflows` | Build/test/train/package/CI; `core.yml`, `native-linux.yml` |
| `docs/` | Năm nguồn chuẩn + supporting task records/evidence, archive có nhãn |
| `artifacts/` | Checkpoint/manifest/source-lock/evaluation/visual theo run; không phải docs điều hành |

Godot đã kiểm chứng M4: 4.5.1, godot-cpp commit trong report M4. Core local gần nhất: Zig 0.13/Clang 18.1.6, CMake 3.31.6/Ninja. Không suy máy khác đã có cùng toolchain. Dependency Python pin thực tế nằm trong [ai/requirements.txt](../ai/requirements.txt): PyTorch CPU, pybind11, numpy, ONNX/ORT… đọc file trước cài; manifest của run lịch sử là pin riêng.

### Build core local Windows đã biết

```powershell
& build/tooling/cmake/data/bin/cmake.exe -S . -B build/m5-activity-evaluator -G Ninja -DCMAKE_MAKE_PROGRAM=D:/Code/PolyEc/build/tooling/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=D:/Code/PolyEc/build/tooling/zig-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=OFF -DPOLYEC_BUILD_PYTHON=OFF
& build/tooling/cmake/data/bin/cmake.exe --build build/m5-activity-evaluator --parallel 4
& build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-activity-evaluator --output-on-failure
```

Đường dẫn toolchain trên đặc thù checkout này; máy khác cấu hình C++20/Ninja/toolchain phù hợp, không scan/cài lại toàn máy khi thông tin đã có. Core-only OFF flags không chứng nhận app, binding hoặc AI. Không sửa dependency/TLS/checks để vượt gate.

App/build/test/package: `scripts/build-windows.ps1`, `test-native.ps1`, `package-windows.ps1`; tham số và lệnh M4 đã chạy nằm trong report M4. Binding: `scripts/build-training.ps1`; AI cần ORT_ROOT, Python cần pybind11/Python development. Linux app/package chạy tay theo ưu tiên hiện tại; portable core/Python vẫn phải test. Chỉ mở training script khi tác vụ có run/budget rõ; không tự dùng rerun script làm nghiệm thu gameplay.

### Quy trình OpenCode và quản lý docs

Đọc AGENTS → CURRENT_TASK → đúng phần source/spec được giao. OpenCode trình plan có file/hàm, ownership/invariants, success/reject/accepted-failed, edge cases, units, impact save/binding/action/observation/reward/model và commands. Codex review rồi giao code; OpenCode test; Codex review diff/evidence. Thay contract lớn phải review lại. Tác vụ thông thường không hỏi lại gameplay đã duyệt.

Nghiệm thu cập nhật CURRENT_TASK; lịch sử vào DEVELOPMENT_HISTORY; số đo chi tiết vào reports. Không chép bảng trạng thái vào nhiều nơi; tài liệu archive chỉ đọc khi có câu hỏi provenance cụ thể. Giữ model/source locks bất biến, không sửa để hợp thức hóa luật mới. Không commit/push/train dài/chi tiền khi chưa được yêu cầu.

## 12. Tài liệu tham khảo và cách đọc

- [Plan toàn dự án](IMPLEMENTATION_PLAN.md): các lựa chọn sản phẩm đã chốt và gates.
- [Tác vụ hiện tại](CURRENT_TASK.md): phần đang làm, bằng chứng và bước kế tiếp.
- [Paper 15/10 được lưu](archive/developer/polyec-technical-paper.md): truy nguyên cách giải thích tại snapshot cũ; các công thức và phương pháp còn dùng đã được đưa vào guide chuẩn này.
- [Cho et al., 2014 — GRU](https://aclanthology.org/D14-1179/): nền tảng gated recurrent unit.
- [Schulman et al. — GAE](https://arxiv.org/abs/1506.02438), [PPO](https://arxiv.org/abs/1707.06347): cơ sở ước lượng/cập nhật policy; không chứng nhận chất lượng PolyEc.
- [Yu et al. — MAPPO](https://arxiv.org/abs/2103.01955): bằng chứng cooperative MARL, không trực tiếp bảo đảm mixed-motive society.
- [Leibo et al. — Melting Pot](https://arxiv.org/abs/2107.06857): đánh giá tổng quát hóa sang đối tác/tình huống mới.
- [ONNX Runtime threading](https://onnxruntime.ai/docs/performance/tune-performance/threading.html): phân biệt intra/inter-op và contention.

Các nguồn nghiên cứu là giải thích phương pháp, không thay code, approved contract hoặc evidence theo revision.
