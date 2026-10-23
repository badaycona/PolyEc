# M5 — nghiệm thu tích hợp và tham số gameplay

Ngày 2026-10-20. Người dùng yêu cầu khắc phục hạn chế quy trình ở tab phụ và muốn chỉnh nhanh các giá trị gameplay qua file văn bản. Tài liệu này bổ sung gate cho tab chính; chưa triển khai bộ đọc cấu hình hoặc nghiệm thu code. CURRENT_TASK vẫn là nguồn trạng thái duy nhất; IMPLEMENTATION_PLAN vẫn sở hữu scope và quyết định gameplay.

## 0. Tần suất kiểm tra — yêu cầu mới 2026-10-21

Áp dụng [AGENTS](../../AGENTS.md), mục “Kiểm tra theo phạm vi thay đổi”, và [workflow mục 4](../DEVELOPMENT_WORKFLOW.md#4-evidence-đủ-để-nghiệm-thu). Các yêu cầu xuyên core–save–Python–Godot bên dưới là điều kiện của mốc tích hợp sau khi nhóm chức năng ổn định, không phải checklist phải lặp sau từng sửa core nhỏ.

Tab chính được hoàn tất một chuỗi chức năng trong core bằng C++ tests trước, rồi gom adapter/replay/Godot vào mốc tích hợp đã ghi rõ. Kiểm adapter sớm khi trực tiếp đổi API/ABI/binding, dữ liệu observation/action/reward/model hoặc có lỗi chỉ xuất hiện ở lớp đó. Schema save đổi phải kiểm migration, roundtrip và reject trong core ngay; không tự buộc chạy toàn Python/Godot nếu bề mặt adapter không đổi. Chỉ mở rộng/full-suite theo mức phụ thuộc, lỗi chưa rõ hoặc mốc ổn định, không tự lặp cả Release và Debug sau mỗi sửa riêng lẻ.

Plan tiếp theo ghi kiểm ngay, kiểm chờ mốc nào và ranh giới đóng nhóm chức năng. Core đã qua có thể được xác nhận riêng; phần Python/Godot chưa chạy trên source mới phải ghi rõ và vẫn bắt buộc trước khi xác nhận tích hợp hoặc toàn M5. Không dừng lệnh đang chạy; áp dụng từ ranh giới an toàn kế tiếp. Không giảm gameplay, learned-policy quality hoặc performance/package scope.

## 0.1. Điều kiện bắt đầu training RL sau khi thế giới gameplay hoàn tất

Đây là mốc riêng để cho phép bắt đầu hoặc resume training; nó không thay thế nghiệm thu toàn M5.

- Trước training, hoàn tất các luật hoạt động T1–T3 và các chu trình gameplay M5.0/1–M5.4 trong C++ core. T3 gồm sleep/exertion/recovery, care/lifecycle và personality/knowledge theo contract. Các luật phải được kiểm bằng command/transition C++ phù hợp, gồm thành công, từ chối nguyên tử và trường hợp biên chịu ảnh hưởng.
- Python trainer phải gọi chính C++ core và thể hiện đúng các luật mà policy cần học: observation chứa dữ liệu cần thiết; legal-action mask/action slots khớp luật; reward lấy từ transition/state utility theo contract đã chốt. Không dùng môi trường Python viết lại luật, adapter/model cũ hoặc hành động chưa được cập nhật để thay thế phần còn thiếu.
- Trước khi train, chạy smoke nhỏ nhưng thật qua C++→Python trên nhóm chức năng vừa ổn định. Kiểm ít nhất việc gọi cùng core, observation, legal action, reward và một chuỗi transition đại diện. Nếu sửa binding/ABI thì build và kiểm target liên quan đủ để smoke Python chạy trên source mới; không cần build toàn bộ app chỉ để mở mốc này.
- Có thể để sau training UI/Godot presentation, full native game/runtime presentation builds và M5.5: profile B, stress/long-run, hiệu năng toàn chu trình và package Windows. Việc hoãn các phần này không cho phép bỏ luật trong core hoặc bỏ khả năng policy quan sát/chọn hành động liên quan.
- Đạt điều kiện bắt đầu training không có nghĩa policy tốt, không thay thế selector/holdout/model quality gate, và không có nghĩa toàn bộ M5 đã hoàn tất. M5.5 và các gate sản phẩm trong IMPLEMENTATION_PLAN vẫn phải qua trước khi xác nhận M5 hoàn thành.

## 1. Theo dõi kết quả người chơi nhận được

Mỗi lát có một kết quả quan sát được, phạm vi file, điều kiện hoàn thành và bằng chứng. Phân biệt bốn mức: đã thiết kế, đã code, đã kiểm từng phần, đã nghiệm thu tích hợp. Không quy số test hoặc số codec đã tách thành phần trăm hoàn thành M5.

Tab chính ghi ngay đầu CURRENT_TASK: lát hiện tại, kết quả gần nhất, gate còn thiếu và bước tiếp theo. Các entry cũ giữ vai trò lịch sử. Với core refactor, ghi riêng các lát R1–R4 đã accepted và gate tích hợp còn thiếu để đóng toàn bộ; không vừa gọi cùng một phạm vi complete vừa open.

## 2. Một chu trình sản xuất xuyên các lớp

Chọn một recipe đã có contract, với tồn kho/tiền ban đầu nhỏ và kết quả tính tay độc lập. Thực hiện theo các lát code có giới hạn, sau đó nghiệm thu chu trình:

1. Qua command thật, tạo/fund cơ sở và chuyển nguyên liệu từ nguồn có quyền. Ghi số dư và tồn kho ban đầu.
2. Giao Work có worksite, cho actor đến đúng nơi và tiến đúng game time.
3. Core tính công, capacity, đầu vào/đầu ra và lương theo contract. UI không ghi trực tiếp state.
4. Lưu giữa quá trình; tải vào world mới và tiếp tục. So sánh với lần chạy không ngắt bằng state chuẩn hóa và ledger.
5. Python gọi cùng C++ core, cùng seed/cấu hình/command và đối chiếu checkpoint với C++. Không triển khai lại luật kinh tế trong Python.
6. Trong Godot thực sự mở phiên chơi, thực hiện command tương ứng và kiểm PlayerView phản ánh đúng kho, tiền, tiến độ và lỗi. Build extension chỉ là điều kiện trước bước này.
7. Kiểm pause, thiếu đầu vào, hết capacity, command lặp/stale và từ chối không sửa dở. Kiểm ranh giới ngay trước/đúng/sau một công; tiếp tục sau save không trả lương hoặc sinh hàng lần hai.

Chu trình có thể dùng driver deterministic có nhãn test để kiểm cơ chế. Nó không chứng nhận learned RL policy đã chọn được worksite. Gate policy/action/observation/model riêng vẫn bắt buộc theo scope M5.

Không yêu cầu tổng số món hàng bất biến qua recipe. Ledger phải đối chiếu đúng lượng tiêu thụ/sản xuất được recipe cho phép; chuyển hàng và chuyển tiền nội bộ phải bảo toàn.

## 3. Cấu hình gameplay bằng file văn bản

Hướng được người dùng yêu cầu: đưa các giá trị cân bằng vào một bộ cấu hình có phiên bản, dễ sửa, được core dùng chung cho native/Godot/Python. Đây là tham số mô phỏng, khác hyperparameter huấn luyện như learning rate.

Đề xuất định dạng TOML hoặc JSON theo dependency sẵn có; OpenCode khảo sát có giới hạn và trình plan trước khi chọn parser. Không thêm file mẫu khiến người dùng tưởng runtime đã đọc được khi chưa có loader.

Phạm vi đầu tiên: ticks cho một work unit, mức lương cơ sở, capacity mặc định, và lượng nguyên liệu/sản phẩm nguyên theo recipe. Các tham số tiếp theo được bổ sung khi hệ thống tương ứng triển khai. Giữ giá trị mặc định hiện hành để tránh đổi balance ngầm.

Yêu cầu thiết kế trước code:

- Mỗi trường có tên, kiểu, đơn vị, giá trị mặc định, miền hợp lệ và quy tắc rounding. Không nhập floating point tùy tiện vào ledger nguyên.
- Parse và validate toàn bộ cấu hình vào đối tượng tạm trước khi tạo/thay world. Reject khóa không nhận diện, ID recipe/good sai, số âm, divisor bằng không, giá trị vượt miền an toàn; báo đúng đường dẫn trường.
- Một world dùng snapshot cấu hình bất biến. Sửa file áp dụng cho world mới; không tự thay luật giữa một ca làm đang chạy. Hot reload cần contract riêng nếu triển khai sau này.
- Save lưu snapshot đã chuẩn hóa cùng phiên bản và fingerprint. Load tiếp tục bằng cấu hình của save dù file bên ngoài đã đổi. Save cũ dùng đúng bộ mặc định lịch sử được version hóa.
- Replay, report, benchmark và model metadata ghi fingerprint cấu hình. Khác fingerprint không tự kết luận model tương thích; gate model phải phân loại thay đổi, hỗ trợ hay reject rõ ràng.
- Quyền sở hữu, nguyên tử giao dịch, stable IDs và invariant không trở thành công tắc cấu hình. Thay đổi luật như retry/forfeit cần contract rõ, không lén đổi bằng một con số.

Gate cho loader: default giữ hành vi đã chốt; một cấu hình thay đổi cho kết quả tính tay dự kiến; cấu hình sai bị reject trước mutation; save tiếp tục đúng sau khi file bị sửa; C++/Python dùng cùng snapshot. Test đúng hệ thống bị tác động, không chạy lại toàn bộ chỉ vì sửa tài liệu.

## 4. Review, bằng chứng và fallback

Theo yêu cầu áp dụng thử của người dùng ngày 2026-10-20, Codex trực tiếp triển khai mốc chu trình xưởng dệt và cấu hình; OpenCode tùy chọn cho tác vụ rõ phạm vi. Ngoại lệ phân vai ở đầu AGENTS có ưu tiên trong mốc này. Mỗi tác vụ vẫn có plan, file boundary và lệnh nghiệm thu. Sau tối đa hai lần gọi OpenCode không tạo plan/diff hữu ích cho cùng tác vụ, Codex tiếp tục; không gọi exit 0 không có output là hoàn thành.

Khi Codex thực hiện fallback, tách lượt viết và lượt review theo checklist contract; không mô tả đó là review độc lập bởi người khác. Dùng oracle tính tay, fixture lịch sử và kiểm chuỗi hành động để bù rủi ro tự xác nhận.

Bằng chứng gắn với trạng thái source đã kiểm: HEAD cộng manifest/hash các file liên quan, bao gồm file mới chưa tracked; lệnh, kết quả, cấu hình và giới hạn. Thay code sau test phải đánh giá lại phần chịu ảnh hưởng và chạy kiểm tương ứng trước khi xác nhận phạm vi đó. Xác nhận riêng core không bắt buộc đóng mốc Python/Godot ngay; ghi rõ phần chờ theo mục 0. Trong mốc thử nghiệm, được commit cục bộ sau nghiệm thu nếu tách chính xác phần thay đổi thuộc tác vụ; nếu không, giữ checkpoint manifest/hash. Không push/publish.

Không coi nhiều lần full suite xanh là thay thế coverage: ưu tiên kiểm đường chạy thật, thiếu nguồn lực, ranh giới và atomicity. Đo hiệu năng khi đổi đường chạy thường xuyên bằng workload/seed/cấu hình xác định; build benchmark không có nghĩa đã đo.

## 5. Thứ tự áp dụng ở safe boundary

Tab chính hoàn tất hoặc chốt rõ trạng thái lát đang chạy trước khi đổi contract. Đọc tài liệu này ở lần plan kế tiếp; đưa quyết định cấu hình vào IMPLEMENTATION_PLAN, review tác động save/model rồi giao loader theo lát nhỏ. Tiếp tục recipe/production và wiring còn thiếu để đạt chu trình mục 2.

Sau chu trình đầu tiên, mở rộng từng nhóm tính năng còn thiếu theo backlog M5; không giảm scope để gọi chu trình mẫu là hoàn thành milestone. Bàn giao toàn bộ cần core refactor tích hợp, toàn bộ gate M5, runtime Windows, model và performance đúng phạm vi đã chốt.

Đánh giá thử nghiệm sau chu trình đầu: ghi thời gian chờ công cụ nếu có số đo, số vòng sửa sau review, các gate thực sự chạy và lỗi còn mở. Không tự tạo số liệu so sánh với quy trình cũ khi thiếu log. Báo cáo ngắn bằng tiếng Việt: tính năng chạy được, bằng chứng, hạn chế, bước tiếp theo.
