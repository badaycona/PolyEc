# Quy trình xây, kiểm và học từ mô phỏng

Theo [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md). Không tiếp tục training hoặc công việc M5 cũ chỉ vì đọc lại một report. Người dùng đã giao session mới xây luật môi trường, class/object và bản game trực quan một lãnh thổ; training chưa thuộc bước triển khai đầu này.

## 1. Một nhóm công việc đủ cụ thể

Trước code chọn một chuỗi thuộc mốc hiện hành, lập plan ngắn về hành vi, dữ liệu/tài sản, đơn vị, file/target, success/reject/gián đoạn, save/API/model và phép kiểm. Thực hiện nhóm chính tới ranh giới đã nêu; không chuyển sang tạo helper/test dễ hơn khi chức năng đang thiếu mà không ghi phụ thuộc cụ thể.

CURRENT_TASK là bản hiện tại được thay thế sau khi đọc lại, có một nhóm và một bước tiếp. Lịch sử đưa vào HISTORY/report; mỗi nhóm dùng một plan và một report cập nhật rõ ràng. Không tạo snapshot sau mỗi sửa hoặc xóa failure cũ để làm tài liệu gọn. Chỉ commit/push/publish theo yêu cầu; không gom dirty work của người dùng.

## 2. Kiểm theo phạm vi

| Thời điểm | Cần kiểm | Chưa phải kết luận |
|---|---|---|
| Sửa core nhỏ | Build target liên quan; success/reject và biên thật có ảnh hưởng | Chưa chứng nhận app/Python hoặc toàn bộ gameplay |
| Đóng chức năng core | Cùng world: command → giao dịch → tiến thời gian → kết quả → save/load; regression phụ thuộc | Chưa chứng nhận learned policy tự thực hiện chuỗi |
| Nhóm core ổn định | Build consumer bị ảnh hưởng; Python/native/Godot thao tác lệnh thật, so dữ liệu | Build/scan editor không thay việc bấm và thấy hậu quả |
| Mở rộng quy mô/phát hành | Tích hợp các nhóm, cả cấu hình cần thiết, inference thật, hiệu năng và package | Catalog hoặc constructor count không chứng minh workload |

Không ALL_BUILD/full suite Release+Debug/Python/Godot sau mọi chỉnh sửa. Kiểm sớm adapter khi API/ABI/model/observation/action/reward thay, hoặc lỗi riêng ở lớp đó. Save schema đổi phải kiểm roundtrip/reject và migration đã hứa ngay trong C++; consumer replay có thể gom vào mốc ổn định.

Test đối chiếu kết quả bằng số tiền/hàng/tick tính độc lập và invariant, không chỉ gọi cùng helper rồi so với chính nó. Test reject phải chạm đúng điều kiện muốn kiểm. Assertion trong Release phải còn hoạt động. Giữ failure đã biết, không sửa oracle/hash cũ để source mới thành pass.

## 3. Các chuỗi mẫu cần kiểm

- Mốc A: owner giao nguyên liệu → worker nhận việc → thực hiện công → tạo hàng → trả lương/thuế có đối ứng → người mua trả tiền và chuẩn bị bữa → đói/vui vẻ thay đổi sau thời gian → lưu/tải ở giữa và sau cùng.
- Mốc B: người bán/buôn lấy hàng → nhận thông tin → chọn chuyến → vị trí trên tuyến → nghỉ/ở trọ/tiêu dùng → đến nơi/giao dịch → đối soát lợi nhuận sau toàn chi phí; đường đóng giữa chặng và save trên tuyến.
- Mốc C: một chính sách hoặc hành động vi phạm có người thật → đối tượng bị tác động → thông tin tới người biết → phản hồi/đề nghị/xử lý → thay đổi tiền/quyền/quan hệ/nguồn lực; chiến tranh phải đối soát quân nhu và người tham gia.

Lệnh định trước/fixture được ghi nhãn rõ là kiểm luật. Không giả vờ campaign tự phát hoặc learned AI bằng kịch bản đã sắp sẵn. Trước bản phát hành cần kiểm nhiều chuỗi cùng world, không ghép số pass của các world khác nhau rồi gọi là tích hợp.

## 4. Xu hướng, mục tiêu, reward và loss

Thiết kế cần phân biệt:

1. Thế giới: hành động thay tiền/hàng/cơ thể/vui vẻ/quan hệ theo luật chính xác.
2. Giá trị cá nhân: xu hướng và mục tiêu làm người đó coi trọng các kết quả khác nhau.
3. Huấn luyện: reward biểu diễn kết quả được coi trọng; thuật toán dùng trải nghiệm và reward để cập nhật loss/trọng số chung.

Không cần loss/model riêng mỗi NPC. Shared policy nhận quan sát, xu hướng, mục tiêu và memory cá nhân. Nếu dùng value function thì cũng phải thấy ngữ cảnh cần thiết; không yêu cầu nó đoán các trọng số sở thích bị giấu. Reward lấy từ kết quả thật, không thưởng vì tên hành động hoặc vì tự đặt/hủy mục tiêu.

Trước training phải chốt thành phần utility, đơn vị, thang chuẩn hóa, trọng số theo xu hướng và cách trả reward theo thời gian. Các ví dụ đề xuất: đáp ứng nhu cầu; vui vẻ; dự trữ/tài sản ròng sau nợ/nghĩa vụ; kết quả xã hội được biết; tiến độ/hoàn thành mục tiêu thật. Không mặc định tất cả có trọng số dương vô hạn.

Các lỗi phải kiểm: vay được tiền bị tính là giàu lên; mua/bán vòng được thưởng; giữ tiền vô hạn có lợi hơn sống; đi qua đi lại farm reward; nói chuyện/đổi mục tiêu lặp để farm; ngủ/đứng yên làm tăng điểm mãi; người thực hiện trả chi phí còn actor khác được tính nhầm kết quả. Reward không được lộ bí mật của người khác qua evaluator.

Nếu thời gian giữa quyết định không đều, lượng thưởng và giảm giá trị tương lai phải theo elapsed game ticks; không nhân thưởng/discount dựa trên số lần bấm lệnh. PPO là phương án có thể tái sử dụng từ kinh nghiệm cũ, chưa tự khóa kiến trúc/siêu tham số trước khi đo workload mới. Không bắt gameplay gánh luật phức tạp chỉ để dễ train.

## 5. Dữ liệu RL được tạo thế nào?

Dữ liệu là trải nghiệm sinh từ C++ core hiện hành, không phải bộ truyện hành động viết tay hoặc một bản luật kinh tế Python khác.

Mỗi episode bắt đầu từ generator có world seed, phân bố tài sản/nguồn/cơ thể/kinh nghiệm/xu hướng và version cấu hình. Mỗi lần NPC có cơ hội quyết định ghi: episode và NPC ID, tick, dữ liệu được biết, lựa chọn/mask khả thi, mục tiêu/xu hướng liên quan, memory đầu bước, action được lấy mẫu, xác suất/value cần cho thuật toán, elapsed ticks, reward thành phần và cuối bước/kết thúc. Ghi rõ version core/model/observation/reward. Không nối đoạn của người hoặc world khác như cùng một chuỗi.

Giữ dữ liệu domain cần chính xác trong core; memory model chỉ là bộ nhớ học. Đổi nghề không reset memory. Nhân vật chết, episode hết thật và rollout hết độ dài là các ranh giới khác nhau; xử lý bootstrap và reset theo từng trường hợp.

Tách world seed train/validation/holdout trước khi chạy. Train sinh trải nghiệm cập nhật; validation chọn model theo tiêu chí đã chốt; holdout chỉ đánh giá model đã khóa. Biến thể cùng world ban đầu không được len vào nhiều tập dưới tên khác mà không kiểm rò rỉ. Protocol ba family/split cũ không áp cho bản mới.

Report training phải giải thích generator tạo tình huống nào, phân bố xu hướng/mục tiêu, số world/người/quyết định/game ticks, điều kiện reset, bạn đồng hành/đối tác ra quyết định thế nào, cách lấy mẫu/loại dữ liệu, split seed và các tình huống còn thiếu. Không gọi một checkpoint cũ là dữ liệu hoặc kết quả của luật mới.

## 6. Bằng chứng policy và ngân sách

Chỉ chạy huấn luyện sau gameplay theo mốc E của kế hoạch và có ngân sách đo được: phần cứng, giới hạn run/decisions/updates/thời gian hoặc tài nguyên, điểm dừng, lưu checkpoint và tiêu chí đánh giá. Không tự mua GPU/cloud hoặc coi yêu cầu chuẩn bị docs là cho phép train.

Đánh giá ghép cặp learned policy, policy ban đầu và baseline có nhãn trên cùng world seeds độc lập. Baseline chỉ dùng kiểm/đối chứng, không thay quyết định NPC trong sản phẩm. Không ưu tiên model chỉ có reward đẹp nếu nợ, survival, hàng tồn, hoạt động hoặc quyền có lỗi.

Mỗi lượt train/đánh giá lưu số liệu nguồn và ít nhất một biểu đồ đúng run/version. Train vẽ reward/return theo update hoặc số quyết định; làm mượt phải ghi cửa sổ và giữ dữ liệu gốc. Evaluation vẽ riêng kết quả seed độc lập, độ phân tán/khoảng tin cậy và kết quả tiền/nợ/sống/nhu cầu/hành động phù hợp. Không gọi rollout reward là điểm đánh giá độc lập.

Để chứng minh khác biệt cá nhân, chỉ đổi xu hướng/kỹ năng/thông tin/mục tiêu trên các world ghép cặp; đo chi tiêu, tích lũy, di chuyển, thời gian/công và hành vi. Kiểm nhiều seed và nhu cầu sống; một người nghèo không mua giải trí chưa chứng minh policy hiểu tiết kiệm. Conviction chỉ được tuyên bố khi có phép kiểm persistence/đổi mục tiêu, không vì có field.

## 7. Hiệu năng và ứng dụng

Đo theo dân số thực, quan hệ thưa, số người có cơ hội quyết định, hoạt động đang chạy, candidate retrieval, inference, tick, save và lượng việc chờ. Không benchmark chỉ constructor hoặc chỉ world đứng yên. Mốc quy mô nhỏ giúp tìm lỗi, không thay benchmark quốc gia.

PyTorch/ONNX/native phải đối chiếu điểm hành động và memory đúng model; kiểm mask/lấy mẫu/reset và lỗi inference. Package cần app Windows native mở mới, pause/focus, save/load, trình bày nguồn thông tin, đủ model/assets/license và chạy offline đúng phạm vi. Không chứng nhận UI từ build thư viện.

## 8. Cách báo cáo

Viết tiếng Việt thông thường: người chơi/NPC thấy gì, nguồn lực nào thay đổi, đã kiểm được gì và còn thiếu gì. Với refactor chỉ nói phần code được tách và lý do; không gọi thành tính năng game. Số checks/hash/thời gian là bằng chứng phụ, không thay giải thích hành vi.

Một report ghi version/dirty scope, plan, lệnh thực thi/exit code, nguồn log/số liệu, kết quả, giới hạn và bước kiểm tiếp. Bằng chứng của run cũ cố định; đính chính có xuất xứ thay vì sửa cho khớp workspace. Quyết định về scope chưa có thì ghi rõ là đề xuất, không dùng chữ “đã chốt”.
