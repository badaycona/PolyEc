# PolyEc — quy trình phát triển cho Codex và OpenCode

Tài liệu này giải thích cách biến một phần thiết kế thành thay đổi đã kiểm chứng. [AGENTS](../AGENTS.md) quy định quyền thực thi; [CURRENT_TASK](CURRENT_TASK.md) là nguồn duy nhất về lát đang giao, trạng thái và next gate. Tài liệu này không mở một tác vụ mới hay cấp quyền commit/push/train.

Khi báo tiến độ cho người đọc, không chỉ nêu mã như `R1`, `M5.4` hay `P1.1`. Hãy viết tên/ý nghĩa của nhóm việc, nói nó đang ở trạng thái nào và dẫn tới bằng chứng phù hợp. Ví dụ: “Core refactor R1 (tách tính bảng lương khỏi việc ghi tiền) đã hoàn tất theo báo cáo tích hợp”; “M5.4 (phần quốc gia) chưa hoàn tất; nhóm đang làm là quỹ công trình đô thị thử nghiệm”; “P1.1 là yêu cầu giữ nguyên danh tính khi một người đổi hộ hoặc nghề, không phải milestone tiến độ.” Nếu một mã được dùng ở nhiều hệ thống, ghi rõ ngữ cảnh, ví dụ “core refactor R1” và “RL quality gate R1”. Cách giải mã P1–P16 và M5.0/1–M5.5 nằm trong [cây bài toán](TECHNICAL_PROBLEMS.md) và [kế hoạch sản phẩm](IMPLEMENTATION_PLAN.md).

## 1. Đọc đúng thứ trước khi làm

1. Đọc AGENTS, rồi CURRENT_TASK; kiểm branch/revision và `git status --short`.
2. Xác định file của tác vụ khác phải giữ, supporting plan nào active, gate nào chưa đóng. Không mở lượt code cạnh tranh cùng file.
3. Đọc đúng phần [plan sản phẩm](IMPLEMENTATION_PLAN.md) liên quan. Muốn hiểu phương pháp dùng [cây bài toán](TECHNICAL_PROBLEMS.md) và [paper](DEVELOPER_GUIDE.md).
4. Mở source/tests liên quan để xác nhận behavior thực. Report cũ không chứng nhận HEAD.
5. Chỉ mở archive khi cần truy nguyên một quyết định cụ thể; chữ APPROVED trong archive không là quyền triển khai hiện tại.

### Chuẩn bị danh mục trước code và gộp tài liệu cùng phạm vi — yêu cầu 2026-10-23

Trước khi thêm một loại hàng, nghề, cơ sở, hoạt động, dịch vụ hoặc tổ chức, đối chiếu [IMPLEMENTATION_PLAN §22.4–22.6](IMPLEMENTATION_PLAN.md#content-definitions-v1). Điền trước mã, đơn vị, nguồn, quyền, đầu vào/đầu ra, nơi sử dụng, vòng đời và liên kết. Không coi việc có enum/class/tên hiển thị là hoàn thành hành vi. Cấu trúc dữ liệu chung nhận các định nghĩa có cùng cơ chế; chỉ thêm class/thành phần xử lý cho trách nhiệm và hành vi thực sự khác nhau.

Một chủ đề có một nơi sở hữu nội dung: phạm vi/luật/danh mục đích trong IMPLEMENTATION_PLAN; phương pháp chung trong DEVELOPER_GUIDE hoặc TECHNICAL_PROBLEMS theo vai trò đã quy định; trạng thái hiện tại trong CURRENT_TASK; lệnh/kết quả theo phiên bản trong report. Các file còn lại dẫn tới nơi đó thay vì sao chép bảng hiện hành. Bảng danh mục mới không được khiến báo cáo kiểm kê cũ trông như đã kiểm source mới.

Khi hai file cùng mô tả một nhóm đã kết thúc và có thể đọc hợp lý trong một file hiện có, gộp nội dung đầy đủ vào file đó thay vì tạo thêm file tổng hợp. Kế hoạch và báo cáo của một cuộc kiểm kê chỉ đọc có thể cùng nằm trong report, chia rõ phương pháp trước kiểm kê và kết quả theo ngày. Trước khi bỏ file cũ, chuyển đủ quyết định, giả định, danh sách source, phép kiểm, giới hạn và bằng chứng, ghi đường dẫn gốc/ngày gộp rồi sửa đường dẫn trong các tài liệu liên quan. Giữ nguyên kết quả, raw artifact và hash lịch sử; không chạy lại hoặc sửa hash để làm việc gộp có vẻ là một lần kiểm mới.

Không gộp chỉ vì tên gần nhau khi các file thuộc phiên bản/run khác nhau, có người đang sửa hoặc có mục đích riêng cần truy nguyên. Với file đang được nhóm khác sửa, chọn phần khác không chồng lấn hoặc chờ ranh giới an toàn; đọc lại nội dung mới nhất ngay trước khi chỉnh. Không mở đợt dọn toàn kho archive. Sau sửa docs, kiểm liên kết, mục dẫn, bảng/ID và nội dung đã chuyển; không build game hay chạy lại RL cho thay đổi chỉ là tài liệu.

## 2. Phân vai và vòng công việc

**Áp dụng hiện tại (2026-10-22):** theo AGENTS, tab chính Codex trực tiếp lập kế hoạch, triển khai và kiểm tra core/M5; OpenCode là tùy chọn. Các vai OpenCode bắt buộc trong bảng và đoạn bên dưới là quy trình cũ, không chặn đợt này. Vẫn tách lượt triển khai và lượt rà soát kết quả; tự review phải ghi đúng giới hạn.

| Bước | Người thực hiện | Đầu ra/gate |
|---|---|---|
| Chọn lát | Codex phối hợp người dùng | Before → after hẹp, phạm vi file và điều kiện nghiệm thu |
| Khảo sát/plan-only | OpenCode | File/class/functions, ownership/invariants, flows, edge cases, compatibility và test commands; chưa sửa code |
| Review plan | Codex | Approved có điều kiện rõ hoặc yêu cầu sửa plan; không suy im lặng là duyệt |
| Triển khai/test | OpenCode | Diff trong phạm vi, test thực và evidence; thay contract lớn phải quay lại review |
| Review kết quả | Codex | Đối chiếu spec/diff/oracle, kiểm bằng chứng, chạy kiểm liên quan khi cần |
| Bàn giao | Codex/OpenCode theo phần được giao | CURRENT_TASK cập nhật; history ghi quyết định, report giữ số liệu; next action duy nhất |

Codex không tự thay OpenCode viết gameplay trong tác vụ đã giao. Công việc biên tập docs theo yêu cầu người dùng không là một lượt coding gameplay. Chỉ hỏi người dùng khi thay gameplay, scope hoặc ngân sách; các quyết định kỹ thuật đã được contract duyệt có thể xử lý trong lát.

## 3. Một lát đủ nhỏ trông như thế nào?

Ví dụ về ranh giới: nối pause/game time vào controller và chứng minh không catch-up; không đồng thời đổi reward, body và payroll. Một prompt “hoàn thành M5” không có ranh giới review hữu ích.

Kế hoạch ngắn của nhóm chức năng nằm trong supporting plan được CURRENT_TASK dẫn tới; không lưu mẫu và kế hoạch lịch sử trong CURRENT_TASK. Khi điền phải trả lời:

- Ai sở hữu state; index nào dẫn xuất; đơn vị là gì?
- Success, reject không đổi state và nỗ lực hợp lệ thất bại khác nhau thế nào?
- ID/quyền/consent, chết, shortage, stale/duplicate, concurrency và overflow xử lý ra sao?
- Save/Campaign/Python/native/Godot/action/observation/reward/model bị ảnh hưởng gì?
- Test nào quyết định đúng/sai, lệnh nào chạy ở platform nào, phần nào chưa kiểm?

## 4. Evidence đủ để nghiệm thu

CURRENT_TASK là bản hiện tại được thay thế khi cập nhật, không phải nhật ký nối thêm. Đọc lại ngay trước ghi để giữ thay đổi mới của tác vụ khác. Giữ một mục tiêu thực thi và một bước kế tiếp; chuyển lịch sử sang HISTORY/report. Gom các bước nhỏ cùng chức năng vào một báo cáo; mỗi lần chạy có bằng chứng riêng cố định. Không rehash hồ sơ cũ do tài liệu chung hoặc binary trong build directory đã đổi; đính chính sai lệch bằng một mục có xuất xứ.

Report theo revision/run cần ghi baseline và dirty scope, supporting plan, diff liên quan, lệnh thực thi, exit code, test counts hoặc workload, platform/toolchain, kết quả và giới hạn. Không ghi một lệnh dự định chạy như đã pass.

### Kiểm tra theo mức ảnh hưởng — người dùng yêu cầu 2026-10-21

Ưu tiên vòng phát triển: **viết core → test core → kiểm chuỗi xử lý core bằng C++ → kiểm tích hợp theo nhóm chức năng ổn định**. Quy định này thay yêu cầu full suite sau từng sửa nhỏ trong các plan cũ, trừ khi có lý do kỹ thuật cụ thể cần mở rộng. Các kiểm tra toàn sản phẩm vẫn phải hoàn tất trước khi công bố phạm vi đó hoàn thành.

| Thời điểm | Việc cần chạy | Điều kiện để mở rộng |
|---|---|---|
| Một sửa core nhỏ | Build target bị ảnh hưởng; test hành vi vừa đổi, từ chối không sửa dở và biên liên quan | Lỗi chưa khoanh vùng hoặc nhiều hệ thống cùng phụ thuộc |
| Hoàn tất chức năng core | Chuỗi command, giao dịch, diễn biến ngày/thời gian và save/load trong C++; regression các thành phần dùng chung | Đổi điều phối chung, hạch toán/invariant toàn thế giới, persistence rộng hoặc tới mốc core ổn định trong plan |
| Nhóm chức năng ổn định | Build consumer bị ảnh hưởng; Python/native/Godot tests tương ứng, đối chiếu dữ liệu qua adapter | Bề mặt gọi hoặc hành vi xuyên lớp thuộc phạm vi được xác nhận |
| Xác nhận bản game/M5 hoàn thành | Toàn bộ tích hợp, model quality, hiệu năng và đóng gói theo IMPLEMENTATION_PLAN | Không dùng kiểm core riêng để thay kết quả bản game |

Không mặc định lặp ALL_BUILD, full Release/Debug CTest, Python replay và Godot sau mỗi thay đổi riêng lẻ. Test đã qua chỉ cần chạy lại khi source/phụ thuộc chịu ảnh hưởng đổi, có lỗi mới hoặc có nghi vấn cụ thể. Chọn một cấu hình cho vòng sửa nhanh; kiểm cấu hình còn lại tại mốc ổn định hoặc sớm hơn nếu có rủi ro về bộ nhớ, số học, tối ưu hóa hoặc khác biệt cấu hình.

Python và Godot vẫn dùng cùng core, nhưng không phải mọi sửa core đều cần kiểm adapter ngay. Kiểm sớm đúng lớp khi đổi API, kiểu public/ABI, binding/UI, observation/action/reward/model hoặc khi lỗi chỉ tái hiện qua lớp đó. Với reward thay đổi, chạy kiểm môi trường huấn luyện nhận đúng reward mới; không mặc định kiểm toàn UI. Với schema save mới, core phải kiểm save/load, migration và dữ liệu sai ngay; replay xuyên Python/Godot có thể chờ mốc tích hợp nếu chữ ký/bề mặt adapter không đổi và không có lỗi liên quan chưa giải quyết.

Ví dụ di sản: có thể triển khai và kiểm lần lượt hồ sơ tài sản → trả nghĩa vụ → chia cho người hưởng, bằng C++ và oracle tiền/hàng. Sau khi chuỗi đủ ổn định mới chạy Python/save replay và Godot cho nhóm này. Nếu một bước đổi API public được adapter dùng, build/kiểm adapter đó sớm để phát hiện không tương thích. Không cần lặp toàn bộ các lớp sau từng helper hoặc refactor giữ hành vi.

Trong plan và CURRENT_TASK, ghi rõ: **kiểm ngay / kiểm tại mốc nào / lý do / điều kiện đóng nhóm chức năng**. Phần tích hợp chờ phải gắn với source/schema/model version đã đổi và mốc cụ thể, không để “chờ” vô thời hạn hoặc đưa kết quả cũ thành kết quả mới. Báo “core đã kiểm, Python/Godot chờ mốc tích hợp X” là hợp lệ và không chặn việc tiếp tục core đã được giao. Giữ nguyên failure đang có; không trì hoãn phép kiểm cần thiết cho kết luận sắp đưa ra.

| Loại thay đổi | Kiểm tối thiểu theo tác động |
|---|---|
| Chỉ docs | Link/path, headings/fences/công thức, coverage và consistency; không chạy CTest gameplay chỉ để tăng số check |
| Core transition | Success/reject/edge cases có oracle, invariant, save/replay và regression liên quan |
| Clock/adapter/binding | Units/pause/focus/restore, cùng transition qua bề mặt bị tác động |
| Action/obs/reward/model | Schema/semantic guards, masks/memory/reset, parity; quality test đúng phạm vi |
| UI/package | Hành vi trên app, quyền biết, input/focus/save và offline package theo platform |
| Performance | Workload/counts, full-cycle latency/throughput/RAM/backlog; không dùng constructor count làm scale evidence |

Test Release phải còn hoạt động; không dùng `assert` bị NDEBUG loại bỏ. Reject test phải chạm validator định kiểm. Kết quả build, model train, policy quality và profile benchmark là bốn kết luận riêng.

### AI/RL: biểu đồ để chẩn đoán, đánh giá độc lập để kết luận

Mỗi lượt huấn luyện hoặc đánh giá AI/RL cần có biểu đồ liên kết từ số liệu thật của đúng run/model/version và đường dẫn tới dữ liệu nguồn. Vẽ reward/return huấn luyện theo update hoặc số quyết định; nếu làm mượt thì ghi cách làm và giữ lại các điểm gốc. Khi có nhiều seed, biểu đồ phải cho thấy độ phân tán giữa chúng.

Tách đồ thị rollout dùng khi train khỏi đồ thị đánh giá policy trên world seed riêng. Khi kiểm tra policy có khá lên theo số update không, đánh giá checkpoint ở các mốc đã định trên cùng bộ seed độc lập, ghép cặp với baseline và policy khởi đầu; vẽ utility cùng các chỉ số phù hợp như survival, nợ/mất khả năng thanh toán và hành vi theo scenario/action. Nêu khoảng tin cậy hoặc độ phân tán tương ứng. Không dùng reward training thay cho điểm đánh giá, và không xem đường cong đẹp tự nó là bằng chứng policy đạt chuẩn.

Trong report, giải thích biểu đồ cho thấy điểm yếu nào, ví dụ hành động quá thường xuyên, tình huống policy bỏ sót, reward không tăng, hoặc khác biệt lớn giữa seed; sau đó nêu phép kiểm tiếp theo dựa trên phát hiện đó. Không đổi reward, luật hay thuật toán chỉ để làm đường cong đẹp hơn, và không dùng validation/holdout để điều chỉnh policy rồi tiếp tục gọi cùng dữ liệu đó là đánh giá độc lập.

## 5. Ghi nhận quyết định và sửa sai

Thay scope/gameplay ghi vào IMPLEMENTATION_PLAN sau khi được chốt. Phương pháp/công thức ghi vào tài liệu kỹ thuật với đơn vị, giả thiết và giới hạn. Một supporting contract trong `docs/design/` chỉ active khi CURRENT_TASK dẫn đúng phiên bản và review status.

Lỗi phát hiện trong review cần ghi điều kiện gây lỗi → invariant/behavior bị vi phạm → cách sửa → test chặn tái diễn → phần còn mở. Nếu bằng chứng cũ sai, thêm đính chính có nguồn; không sửa checkpoint/source lock để giả tương thích, không xóa failure khỏi lịch sử.

## 6. Safe handoff

Kiểm lại dirty state trước bàn giao, giữ thay đổi tác vụ khác. CURRENT_TASK ghi acceptance thực, gate còn mở và next action; DEVELOPMENT_HISTORY ghi mốc/bug/giải pháp; `docs/reports/` giữ số liệu chi tiết. README chỉ tóm tắt giai đoạn lớn, không copy bảng trạng thái từng lát.

Không commit/push/publish, huấn luyện dài hoặc chi tiền cloud khi chưa được yêu cầu tương ứng. Không bỏ test, giảm scope hoặc tắt dependency/TLS checks để qua gate. Lệnh build phải dùng cấu hình phù hợp lát; core-only flags không chứng nhận app/native/Python.

## 7. Báo cáo dễ hiểu với người dùng

Quy ước theo yêu cầu người dùng ngày 2026-10-21, áp dụng cho cập nhật tiến độ, câu trả lời cuối và phần tóm tắt báo cáo gửi người dùng. Viết bằng tiếng Việt thông thường, bắt đầu bằng kết quả cụ thể. Giải thích chức năng nào thay đổi, nhân vật hoặc người chơi sẽ thấy gì, đã kiểm tra được điều gì và còn thiếu gì. Nếu chỉ sửa cấu trúc code, nêu phần xử lý được tách và mục đích của việc tách; không gọi đó là chức năng game mới.

Không dùng thuật ngữ quy trình thay cho nội dung. Những từ như “lát”, “gate”, “state”, “contract”, “nghiệm thu”, “chốt evidence” chỉ được dùng khi cần và đã giải thích ngay. Tránh các cụm mơ hồ như “mất trạng thái” hoặc “kiểm tra tích hợp chung”: phải chỉ rõ dữ liệu hoặc chức năng có nguy cơ bị ảnh hưởng. Với quyền trong game, nói rõ ai được thực hiện thao tác nào lên tài sản nào.

Số nhóm kiểm tra đã qua, thời gian chạy, chế độ Debug/Release, hash và tên class là thông tin bổ sung. Chúng không tự chứng minh một chức năng hoàn tất. Giữ các lệnh và log chi tiết trong báo cáo kỹ thuật; lời cập nhật gửi người dùng cần nói được ý nghĩa của kết quả. Chỉ khẳng định điều đã có bằng chứng thực chạy.

Ví dụ không nên viết:

> Debug CTest đã kết thúc 19/19; mình chốt report và hash cho EstateManager, rồi kiểm tra các thay đổi tích hợp chung để không làm mất trạng thái của lát quyền đất vừa nghiệm thu.

Viết rõ hơn:

> Tôi đã tách đoạn xử lý tài sản khi nhân vật chết thành một phần code riêng để dễ kiểm tra và bổ sung luật thừa kế sau này. Các kiểm tra tự động hiện có đều qua ở cả hai chế độ build. Tôi đang kiểm tra xem thay đổi này có làm hỏng chức năng cấp và thu hồi quyền sử dụng đất vừa thêm hay không. Việc chia tài sản cho người thừa kế vẫn chưa được làm.

Ví dụ này minh họa cách diễn đạt, không xác nhận tiến độ hiện tại của dự án. Trạng thái công việc vẫn được ghi tại CURRENT_TASK.
