# PolyEc — kế hoạch toàn dự án

Cập nhật: 2026-10-18. Đây là nguồn chuẩn về đích sản phẩm, phạm vi và thứ tự triển khai. Đây không phải chứng nhận tính năng đã chạy. Trạng thái và lát đang giao nằm trong [CURRENT_TASK](CURRENT_TASK.md); phương pháp kỹ thuật nằm trong [DEVELOPER_GUIDE](DEVELOPER_GUIDE.md).

## 1. Bài toán sản phẩm

### Cách đọc kế hoạch đầy đủ

Phần I (1–10) xác định contract hiện hành, stack, roadmap và gates. Phần II (11–21) chứa đặc tả sản phẩm chi tiết. Người triển khai cần đọc phần tương ứng trước khi viết supporting plan, không chỉ lấy bảng milestone làm spec.

| Cần hiểu | Phần |
|---|---|
| Sản phẩm, luật nền, agency và quốc gia | 1–4 |
| Kiến trúc, thứ tự triển khai, nghiệm thu, RL và hiệu năng | 5–10 |
| Vòng chơi/ending và địa lý/danh mục vùng/tuyến | 11–12 |
| Chính trị, quyền, thông tin và điều tra | 13–14 |
| Sản xuất, thị trường, chính sách, quân sự và ngoại thương | 15–17 |
| UI, khởi đầu và hướng dẫn trong game | 18 |
| Đủ 12 augment, draft, stat, tính cách và tiến bộ | 19–20 |
| Sự kiện, causal chain, đời sống dân cư và thông báo | 21 |

Các bảng nội dung là đặc tả cần đạt, không phải progress report. Đối chiếu trạng thái thực tế trong CURRENT_TASK và bằng chứng có revision.

PolyEc là game desktop native mô phỏng việc cai trị một quốc gia cuối Trung cổ giả tưởng trong một đời vua. Người chơi quan sát thông tin, thương lượng, bổ nhiệm, cấp nguồn lực, giao thực hiện và xử lý hệ quả. Cư dân, hộ, cơ sở và tổ chức có đời sống, nguồn lực và lợi ích riêng; việc thường lệ không chờ người chơi duyệt từng người.

Sự kiện chính trị phải có nguyên nhân trong world: người giữ quyền chết, nợ đến hạn, tuyến bị tắc, lao động rời xưởng, cam kết bị vi phạm. Không rút một thẻ khủng hoảng để tự sinh người, tiền, quân hoặc động cơ. Tutorial và fixture kiểm thử được đặt trước nhưng phải có nhãn.

Mục tiêu là liên kết quyền, nguồn lực, quyết định và hậu quả. Số thuộc tính, số NPC hoặc số câu thông báo không thay cho độ sâu. Giữ quy mô đích; không tự giảm số vùng/dân để đạt gate.

## 2. Các quyết định đã duyệt

Luật nền được duyệt ngày 2026-10-15; phần làm rõ ngày 2026-10-17 thay cách diễn giải cũ mâu thuẫn về busy, cơ thể, hoạt động và agency. Bản này hợp nhất các quyết định đó; không duyệt thêm hệ số cân bằng.

| Phần | Contract phải giữ |
|---|---|
| Người chơi | Là vua; tác động qua tư cách, quyền, tài khoản và người thi hành rõ ràng. Lệnh không đồng nghĩa đã có kết quả. |
| Luật | Một bộ luật chung toàn quốc. Vua thay chính sách có tham số và cấp/thu hồi quyền theo luật; chưa có trình viết lại toàn bộ luật giữa campaign. |
| Cá nhân | ID bền qua đổi hộ/nghề/chức; trạng thái và vòng đời riêng. Giới tính không chặn sở hữu, chức vụ hoặc thừa kế. |
| Địa vị | Tách thân phận, danh phận, nghề, tài sản và chức vụ. Giàu lên không tự thành quý tộc; nợ không biến người thành tài sản. Di chuyển có xử lý cam kết hợp lệ. |
| Hộ | Thành viên, cư trú, người phụ thuộc và giám hộ động; tiền riêng và quỹ chung tách biệt. Vào hộ không tự chuyển toàn bộ tài sản. |
| Đất | Holding/cụm đất và quyền sử dụng, thu tô, chuyển giao, nghĩa vụ cụ thể. Quyền độc quyền không cấp hai lần; sở hữu khác quyền xét xử. |
| Cơ sở/nghề | Có chủ, kho, tài khoản, chỗ làm, công suất, vật tư và lao động thật. Học nghề cần bên nhận, người dạy, chi phí và thời gian. |
| Hôn nhân | Hai người chấp thuận, đủ tuổi/năng lực, không có phối ngẫu còn sống khác; cấm trực hệ/anh chị em. Tuổi thành niên và kết hôn tối thiểu 18 năm game. Giám hộ không thay consent. Cưới không tự nhập lãnh địa hay tạo liên minh. |
| Tài sản sau chết | Bảo vệ tài sản của người sống; trả nghĩa vụ từ di sản trước. Phần còn lại chia đều cho con hợp pháp, rồi phối ngẫu, cha mẹ, anh chị em; không người hưởng thì vào quỹ di sản công có sổ. |
| Tước/chức | Tước theo con hợp pháp lớn tuổi nhất không phân biệt giới; thiếu người kế tiếp mở hồ sơ chờ công nhận. Chức bổ nhiệm trả về cơ quan, không tự truyền theo tước. |
| Người phụ thuộc | Có quyền sở hữu; giám hộ quản lý trong giới hạn, không sở hữu thay. Nợ không tự chuyển vượt di sản trừ nghĩa vụ bảo lãnh riêng. Tranh chấp giữ custody, không xóa/nhân tài sản. |
| Tổ chức | Gia tộc, thành thị, guild, giáo hội có thành viên, quỹ, chức và người đại diện. Quyền đại diện khác quyền sở hữu. Một tôn giáo giả tưởng; luật dân sự và tòa thế tục dùng chung. |
| Thông tin | Sự thật world, lời khai, nghi vấn và điều từng người biết là các lớp khác nhau. Báo cáo có nguồn, ngày và độ trễ. UI vua không đọc trộm bí mật. |

Giới hạn M5 đã duyệt: chưa có nhận con nuôi, ly hôn, di chúc tùy ý hoặc cơ chế con ngoài hôn nhân. Không sinh các trường hợp rồi bỏ quyền của họ. Thay đổi những luật này cần quyết định gameplay riêng.

## 3. Hành vi và hoạt động của NPC

Một người có thể làm, học, nghỉ và chăm người khác trong cùng ngày. Tách quan hệ việc làm dài hạn, lịch mong muốn, phiên thực hiện và lượt policy suy luận. Command không liên quan không tự xóa job.

Được nhận cam kết quá tải; world chỉ thực hiện phần thời gian, hiện diện, attention và nguồn lực thực sự có. Phần thiếu có nguyên nhân và hệ quả đã định nghĩa. Không chặn lựa chọn chỉ vì dự báo kém khôn ngoan hoặc không tối ưu. Validation bảo vệ cấu trúc, quyền, consent và bảo toàn; không trở thành người chọn chiến lược hộ NPC.

Attention là tài nguyên chú ý khi thực hiện, khác thiếu ngủ, mệt do gắng sức, sức khỏe và khuynh hướng kiên trì. Attention-time không tự là lương, tiến độ học, dịch vụ chăm sóc hay hồi phục. Tương thích phụ thuộc loại/context; không mặc định mọi hoạt động exclusive hoặc mọi việc ít attention làm được ở hai nơi.

Tính cách tương đối bền; trạng thái cơ thể, tâm trạng và hiểu biết thay đổi nhanh hơn. Shared learned policy chọn quyết định tự chủ; mỗi người giữ memory và sampling riêng. Routine là kế hoạch sửa được, không phải chiến lược viết tay thay RL. Khác biệt năng lực/tính cách phải được đo, không chứng nhận bằng việc thêm field input.

Tie chưa giải quyết phải lộ ra và có cơ hội quyết định lại, không mặc định ID nhỏ thắng. Quy tắc withholding trong evaluator thuần là contract phát triển hiện tại, chưa là luật phân bổ cuối cho mọi tương tác hoặc tài nguyên giữa nhiều người.

Chưa chốt mới: hệ số ngủ/mệt/phục hồi, tuổi thọ và bệnh/chết, độ phân giải thời gian world mới, cadence inference mới, reward weights và mô hình determination chi tiết. Không tự suy ra giới hạn làm 8/16 giờ. Các ý trao đổi 18/10 về cá tính/RL là định hướng cần contract và kiểm chứng trước khi code, không là thông số cân bằng đã duyệt.

## 4. Nội dung quốc gia phải xây

- 24 lãnh địa chư hầu + 3 khu triều đình; map địa lý 2D có terrain/topology, đường bộ/sông/ven biển, lưu vực, tài nguyên, cửa ải và quyền kiểm soát. Pan/zoom/picking/layers/route hoạt động. Thành thị là đơn vị lồng, không cộng trùng vùng.
- 16 nhóm hàng: lương thực khô, thực phẩm bổ sung, muối, gỗ, nhiên liệu, sắt, kim loại màu/quý, xơ sợi, vải, gia súc/ngựa, công cụ, quân nhu/vũ khí, vật liệu xây dựng, giấy, dược liệu, hàng xa xỉ. Biến thể không tự thay thế ngoài công thức; tàu là tài sản vận tải.
- Sản xuất tại cơ sở dùng vật tư, lao động, công suất, mùa vụ và bảo trì. Không dùng nhãn nghề hoặc `region % 7` làm luật sản xuất cuối. Tiền công, địa tô, thuế, lợi nhuận và tiêu dùng có đối ứng.
- Chợ có tồn kho/giá/cầu thanh toán; tín dụng chuyển tiền thật, có chủ nợ, lãi, hạn, bảo đảm và xử lý vi phạm. Chế độ tiền ổn định; chưa có đúc tiền/phá giá/ngân hàng tạo tiền.
- Logistics có tuyến, thời gian, công suất, phí, hàng đang đi và đặt chỗ. Địa lý ảnh hưởng khả năng thực hiện; không teleport.
- Thành thị/guild có đại diện, quỹ, cơ sở, quyền và cơ hội học/nghề hữu hạn. Giáo hội có tài sản, cứu tế/giáo dục và bổ nhiệm trong quyền.
- Quân sự có nghĩa vụ, chỉ huy, tuyển/huy động, lương và tiếp vận. Người đi lính không đồng thời sản xuất đầy đủ tại nhà. Xung đột theo đơn vị/tuyến, chưa mô phỏng vật lý từng binh sĩ.
- Ngoại thương có đối tác/tài khoản ngoài biên, hàng, đơn, tuyến, thuế và rủi ro; dòng vào/ra được hạch toán, không bù nguồn lực ngầm.
- Khởi tạo có phân bố tuổi/hộ, đất, vốn, kho, kỹ năng và nghĩa vụ tương thích. Endowment là nguồn đầu kỳ khai báo.

Một đời vua kết thúc khi chết, bị phế hoặc mất chính quyền trung ương; không tự kết thúc chỉ vì một vùng ly khai. Dân cư tiếp tục có vòng đời; không đổi thành campaign nhiều đời vua. UX hoàn chỉnh thuộc M6.

Giữ 12 augment khởi đầu và sáu stat INT/ADM/ECO/DIP/INTG/MIL. Augment tác động nguồn lực/quy trình có phạm vi, không tạo tiền sau mở sổ hay ép NPC trung thành. Danh mục vùng/tuyến, toàn bộ 12 thẻ, chỉ số, UI và cơ chế chi tiết nằm ngay trong phần II bên dưới; [bản v3](archive/design/implementation-plan.md) chỉ giữ vai trò truy nguyên. Mâu thuẫn phải giải quyết theo thứ tự ưu tiên mục 10.

## 5. Tech stack và ranh giới kiến trúc

| Thành phần | Trách nhiệm và nền tảng |
|---|---|
| Core | C++20 OOP/composition, CMake; độc lập Godot; sở hữu world, command, quyền, transaction, invariant và game time. |
| App | Godot 4.5.x native/GDExtension, godot-cpp phù hợp; GDScript UI, terrain 2D. Windows ưu tiên; app/package Linux hoãn. |
| Training | Python/PyTorch gọi cùng core qua pybind11. Giữ core/Python portable để chạy môi trường Linux/Kaggle. Không bộ luật Python thứ hai. |
| Runtime AI | ONNX Runtime CPU, shared weights, per-person memory/sampling, bounded batches. Không Python/Node/API key trên máy chơi. |
| Dữ liệu | Person ID bền; compact storage và sparse relations/index. Không scene node/model riêng cho từng người, không dense N×N. |
| Save | Version/schema/model identity, RNG, lịch, tiến độ, escrow và knowledge; snapshot tại barrier, validate temporary trước thay live world. |
| Kiểm chứng | CTest core; Python/binding/replay; Torch/ONNX/native parity; Godot smoke/UI; benchmark toàn chu kỳ; package theo platform. |

Version đang dùng và lệnh chạy xem [hướng dẫn dev](DEVELOPER_GUIDE.md). Không tự nâng dependency hoặc suy “đã build core” thành “app/model đã đạt”.

Luồng duy nhất: nhận command và tư cách → validate → chuẩn bị thay đổi → kiểm invariant → commit → facts → thông tin tới người biết. Lệnh reject không sửa dở state. Nỗ lực hợp lệ thất bại có thể commit chi phí/hệ quả có căn cứ. Revalidate tại commit khi tài nguyên cạnh tranh; giải quyết cạnh tranh là contract riêng.

Game time điều khiển tuổi, hạn, cooldown và quá trình. Pause không tiến lịch; mất focus/ngủ máy không chạy bù. Đổi tốc độ không đổi luật. Thống nhất tick/ngày/năm trước khi nối evaluator; không suy từ tick trừu tượng sang phút thật.

## 6. Roadmap toàn dự án

| Mốc | Phạm vi phải có | Gate |
|---|---|---|
| M0 | Native core/map/clock; Python binding, ONNX shape và spike dữ liệu thưa | Cùng transition C++/Python, native load, pause/focus; throughput/RAM primitive có số đo. |
| M1 | Kinh tế 6 vùng + đất triều đình, 7 hàng đầu, thị trường/tuyến/nợ/dự án/save | Bảo toàn, reject nguyên tử; shock và sandbox dài nhiều seed. |
| M2 | Quyền/chính trị/thông tin, individual/child/household, sparse relations/vòng đời | Quyền khác nhau có kết quả khác; không lộ bí mật; profile A có tương tác. |
| M3 | Spike RL 3 họ, recurrent PPO, runner/curriculum, 12 augment/sáu stat | Pipeline và chẩn đoán R0–R3; checkpoint pilot không phải release. |
| M4 | Vertical slice native 6 vùng, Hắc Sơn/logistics/xưởng/hồ sơ nhiều phương án | Map/input/save, causal chain, controller có nhãn hoặc policy pilot hợp lệ. |
| M5 | Nền xã hội đủ sâu + quốc gia 27 khu/16 hàng/thành thị/quân sự/ngoại thương + profile B | Luật, liên kết xã hội, invariant, portable core/binding, workload và hiệu năng thật. |
| M6 | Một đời vua/ending, tutorial/F1, thông tin/UI/save/backup/accessibility/map art | Luồng new/load/pause/end và nguyên nhân; freeze feature/action/observation/reward/scenario. |
| M7 | Production training/evaluation, export/parity, native regression và release | Learned policies qua quality gates; package/offline/platform và profile công bố có bằng chứng. |
| S | Một triệu cá thể | Tương tác thật, chất lượng và chi phí full workload; chưa phải cam kết đã đạt. |

M0–M4 có code/evidence nhưng không suy mọi yêu cầu giai đoạn cũ đã đầy đủ hoặc RL đã đạt. Trạng thái hiện tại đọc CURRENT_TASK.

### Lát M5 và thứ tự phụ thuộc

1. Nền attention → nối timeline/command/clock/save → actual presence/progress/settlement → cơ thể/care/lifecycle. Thứ tự kiểm chứng này không thay thế M5.1–M5.5.
2. M5.0/1: registry/schema/quyền; hộ động/quỹ/đất/địa vị; ownership, guardian và vòng đời.
3. M5.2: cơ sở/hợp đồng/học nghề/16 hàng/vận tải/thị trường.
4. M5.3: tổ chức/chức, hôn nhân/escrow, di sản/nợ/claim, giáo hội.
5. M5.4: quốc gia đủ 27 khu, thành thị/quân sự/ngoại thương và UI vua.
6. M5.5: profile B, stress/long-run/native/package và bằng chứng; giữ các gate chưa đạt mở.

## 7. Nghiệm thu nền trồi sinh

Năm giao dịch phải có success/reject và save/replay: hộ trả học phí không tiêu nhầm quỹ; xưởng dùng vật tư/trả lương/tạo hàng có nguồn; bãi nhiệm không tịch thu tiền riêng; cưới có consent/escrow/refund; di sản trả nợ, bảo vệ đồng sở hữu và tài sản trẻ. Không hard-gate nhận cam kết chỉ vì lịch quá tải; settlement phải bảo toàn nguồn thực.

Ba chuỗi phải có khả năng xảy ra và rẽ theo can thiệp: chủ xưởng chết → di sản/nợ → lương/việc → guild/thuế; tắc tuyến → thiếu hàng → giá/sinh kế → kiến nghị; huy động → giảm lao động/tăng chi → nghĩa vụ thiếu → thương lượng. Đây là fixture chứng minh luật; không là kịch bản bắt buộc hoặc chứng nhận policy đã học.

Kiểm tra sai ID/quyền/consent, actor chết, stale/duplicate, overflow, cạnh tranh nguồn lực; replay giữa escrow/travel/học nghề/kế vị; load hỏng không thay phiên. Observation/UI chỉ thấy thông tin được biết. Nhận nhiều cam kết không tạo thêm giờ, tiền, hàng hoặc dịch vụ.

## 8. RL và cá nhân hóa

Giữ learned policy cho lựa chọn tự chủ trong bản hoàn chỉnh; baseline/random/imitation-only phải có nhãn. Train offline trong cùng C++ core; khi chơi chỉ inference. Khởi điểm 3 họ governance/economic/civilian; không model riêng mỗi người. Recurrent memory không phải bằng chứng cá tính hoặc reasoning đạt yêu cầu.

Rollout ghi actor/decision, thông tin/observation/mask, action/log-prob, reward outcome, elapsed game time, memory/reset và terminated/truncated. PPO dùng rollout tươi; log replay dành chẩn đoán. Tách seed/train/validation/test và đối tác; đánh giá đúng sampling runtime. Không chọn lại model bằng test holdout.

Curriculum: cá nhân/hộ/nơi làm → một vùng → hai/sáu vùng → quốc gia → scale holdout. Không thưởng spam nút hay coi vay là thu nhập; invariant lỗi dừng run và lưu reproducer. M3 chỉ pilot; M5–M6 giữ smoke có ngân sách, production train sau freeze. Huấn luyện dài/GPU/Kaggle là tác vụ có ngân sách riêng; không tự mua dịch vụ.

Giữ các quality gates từ plan v3, không hạ khi hợp nhất docs:

| Gate | Yêu cầu |
|---|---|
| R0 — môi trường | C++/Python/app headless cùng transition; không lộ secret, baseline/random test giữ invariant; đo decisions/s, days/s và RAM. |
| R1 — learned policy sáu vùng | Ít nhất 3 training seeds, ≥100 tình huống holdout ghép cặp; so utility/nghĩa vụ/survival/bankruptcy/thương lượng/hold với baseline và initial. Có cải thiện chiến lược thật. |
| R2 — civilian/child agency | Sampling nhiều seed có phân bố hợp lý; đổi tuổi/budget/job/school/nhu cầu làm policy thay đổi đo được. Giữ ID/sampling riêng; không ép mọi trẻ cùng tương lai. |
| R3 — stat/mục tiêu | Giữ tài sản/thông tin/tình huống, thay INT/chuyên môn. Nếu công bố trí lược thì paired 95% CI cải thiện trung bình không chứa 0 trên bài nhiều bước. Goal/personality phân biệt hành vi khi lợi ích xung đột. |
| R4 — quốc gia/transfer | Đủ 24 vùng, 12 augment, stat cực trị/shock/vòng đời/vua khác phong cách; ≥100 holdout ngắn và 30 campaign hoặc sandbox dài tối đa 50 năm. Báo thời gian/count nếu kết thúc sớm, không coi sandbox tiếp tục là vua sống 50 năm. |
| R5 — native ONNX | Logits/mask/memory và action distribution/invalid probability giữa Torch/ONNX C++; tolerance float32 ban đầu abs 1e-5, rel 1e-3, đổi phải giải trình; quantization có gate hành vi riêng. Model lỗi pause, không fallback heuristic. |

Khóa metric theo họ trước chọn checkpoint: noninferiority biên 5% trên thang chuẩn hóa đã định, có ít nhất một cải thiện chiến lược đáng tin và không reward hack. Không suy từ model world nhỏ sang B/C. Gate pilot chưa đạt tiếp tục ghi rõ; protocol/source lock thí nghiệm không được sửa theo test holdout.

## 9. Quy mô, hiệu năng và nền tảng

Profile A: 6 vùng, khoảng 10k người, 128–256 vai phức tạp để thử cơ chế. M5/profile B: 24+3 khu, 100k thường dân/trẻ em, khoảng 512–2048 bên quyết định phức tạp; phân biệt người, vai trò, tổ chức và policy invocation, không cộng trùng. Một triệu ở S. Constructor nhận một triệu không là benchmark.

Máy đích 16 GB RAM, CPU inference. Budget B: resident ≤6 GB, peak ≤8 GB cả save; input p95 <100 ms; ngày thường p95 ≤80 ms, kỳ tháng ≤500 ms. Mục tiêu render 60 FPS/1080p tham chiếu, tối thiểu ổn định 30 FPS; 1×–4× ổn định, 8× báo tốc độ thật theo gate. Đo core+observation+inference+validation/commit+index, p50/p95/p99, backlog, UI/input, RAM/save. Controller-only tách end-to-end inference.

Tối ưu bằng lịch sự kiện/lazy update có last-update và đủ ranh giới, index, bounded batches, shared data, sparse graph trước khi mở đa luồng. Actor không được gọi policy vẫn già, đói và chịu deadline. Snapshot đọc/ý định/commit tránh thứ tự thread/ID quyết định phân bổ. Không bỏ mô phỏng ngoài camera hoặc bơm tài sản để giữ world sống.

Sandbox ổn định tối thiểu 20 năm nhiều seed theo protocol; đo sinh/tử, sinh kế, giá, việc/nợ, nghĩa vụ, queue và kết quả can thiệp. Không chỉ kiểm “còn người sống”. Windows ưu tiên; app/package Linux hoãn, core/Python portable vẫn phải kiểm thử. Không lấy kết quả một platform chứng nhận platform khác.

Giữ yêu cầu stability tổng quát của plan v3: ≥30 seed trong harness, tối đa 50 năm hoặc ending, với metadata riêng nếu sandbox tiếp tục; không gộp nó với gate M5 tối thiểu 20 năm hay coi vài seed invariant là đã hoàn thành. Packaging phải thử offline không Python/Node, close/reopen, focus/sleep/resume, save path/quyền ghi trên platform được công bố. Atlas phải dễ nhận terrain, sông/đường/cầu hợp lý, overlay/route/picking và privacy, có tác dụng trong engine; art-only demo không qua gate.

## 10. Thay đổi và nguồn chuẩn

Ưu tiên: chỉ đạo trực tiếp của người dùng và AGENTS → plan này về scope → CURRENT_TASK về tác vụ/status → DEVELOPER_GUIDE về phương pháp → evidence theo revision. Archive là provenance; nếu thiếu contract phải làm rõ trước triển khai, không chọn prompt cũ có chữ APPROVED để bỏ qua review.

Phân vai: OpenCode khảo sát giới hạn/lập plan → Codex review → OpenCode code/test → Codex review diff/evidence. Chỉ hỏi người dùng khi đổi gameplay, phạm vi hoặc ngân sách đã duyệt. Cập nhật CURRENT_TASK và HISTORY khi nghiệm thu; không chép trạng thái vào nhiều tài liệu.


## Phần II — đặc tả sản phẩm chi tiết

Phần 1–10 là contract tổng hợp hiện hành; phần 11–21 giữ lại chi tiết thiết kế v3 còn cần cho toàn project thay vì buộc người đọc tìm archive. Đây là **đích thiết kế**, không phải danh sách tính năng đã có. Luật M5 đã duyệt ở phần 2–3 điều chỉnh nội dung kế thừa: cho phép overcommit, tách cơ thể/attention, ID riêng và vai trò/tài khoản riêng. Cadence/quota, hệ số augment, tuổi vua ban đầu, XP và tham số thị trường dưới đây là baseline thiết kế hoặc giá trị cân bằng thử, không được coi là đã hiệu chuẩn hay tự động tương thích timeline mới. Mọi thay đổi so với chúng phải có giải thích, không âm thầm bỏ đi.

## 11. Vòng lặp chơi và mục tiêu một đời vua

### 11.1 Vòng lặp chính

**Quan sát → xác định vấn đề → thương lượng → ban hành và giao thực hiện → theo dõi → điều chỉnh.**

Ví dụ: giá gạo tăng tại vùng mỏ → xác định mất mùa hay tắc vận tải → thương lượng mở tuyến và mua dự trữ → giao quan phụ trách → theo dõi giá, kho, tiến độ → giải quyết phe mất phí đường hoặc lao động rời ruộng.

Ba khoảng thời gian cùng tồn tại:

- **Ngày–tuần:** tiếp kiến, báo cáo, thời hạn, sự cố và điều tra.
- **Tháng–mùa:** thuế, chi tiêu, sản xuất, buôn bán, đầu tư và thương lượng địa phương.
- **Năm:** dân số, kỹ năng, chuyển dịch ngành, công trình lớn, thể chế và di sản.

### 11.2 Mục tiêu và kết thúc

Không có một chỉ số duy nhất đại diện cho cai trị tốt. Báo cáo cuối đời vua đánh giá riêng:

- Thời gian và mức độ ổn định của triều đại.
- Đời sống dân cư: nhu cầu thiết yếu, tử vong, thu nhập thực, bất bình đẳng giữa vùng.
- Năng lực đất nước: sản xuất, thương mại, công trình và kỹ năng.
- Tài chính: nợ, khả năng trả nghĩa vụ, dự trữ và sức chịu khủng hoảng.
- Thể chế: khả năng thi hành luật, uy tín cam kết và quan hệ trung ương–địa phương.
- Cái giá chính trị: đàn áp, xung đột, mất đất, đặc quyền và lệ thuộc được để lại.

Chiến dịch kết thúc khi vua chết, bị phế thành công hoặc không còn giữ chính quyền trung ương sau xung đột. Khủng hoảng hay một chư hầu ly khai không tự động kết thúc game. Người kế vị có thể tồn tại như một nhân tố chính trị nhưng không mở chiến dịch đời tiếp theo.

Vua bắt đầu ở tuổi 30; tuổi thọ không cố định. Sức khỏe có nguy cơ theo tuổi, bệnh và điều kiện sống; không dùng sự kiện chết ngẫu nhiên không báo hiệu để thay thế gameplay. Bệnh nghiêm trọng có diễn tiến và lựa chọn xử lý. Khi kết thúc, hiển thị chuỗi nguyên nhân cùng dòng thời gian các quyết định quan trọng.

## 12. Bản đồ quốc gia và 24 lãnh địa

### 12.1 Cấu trúc bản đồ

24 lãnh địa chư hầu thuộc 6 cụm địa lý; thêm 3 khu đất triều đình. Thành thị đặc quyền là đơn vị lồng trong một lãnh địa hoặc khu triều đình, không được tính thêm vào tổng số lãnh địa.

Bản đồ kinh tế là đồ thị: nút là thủ phủ, thành phố hoặc điểm giao thương; cạnh là đường bộ, sông hoặc tuyến ven biển. Mỗi cạnh có chiều dài, thời gian đi, công suất, chi phí, an toàn, chủ quyền và phí đường. Không mặc định rằng hai vùng sát nhau có tuyến thương mại tốt.

### 12.2 Danh mục thiết kế ban đầu

**Quy ước dữ liệu:** dân số hàng trăm nghìn dưới đây là số liệu bối cảnh atlas v3, không là số cá thể độc lập đã instantiated hay một thay đổi profile B 100k. Chưa có contract ánh xạ dân số bối cảnh sang số cá thể mô phỏng được nghiệm thu; phải chốt rõ trước sinh quốc gia. Không được đếm dân số bối cảnh như NPC đã chạy. Tên, địa lý và lợi thế được giữ đầy đủ để thiết kế không mất nội dung.

Đơn vị dân số: nghìn người; diện tích: km². Đặc sản là sản phẩm danh tiếng ánh xạ vào nhóm hàng chung, không bảo đảm độc quyền.

| ID | Lãnh địa / cụm | Dân | Diện tích | Đặc sản | Lợi thế và nút thắt |
|---|---|---:|---:|---|---|
| L01 | Thanh Điền / lưu vực | 420 | 8.000 | Gạo nước | Đất màu, sông lớn; lũ và quyền thủy lợi tập trung |
| L02 | Liên Trạch / lưu vực | 210 | 5.000 | Cá khô | Đầm hồ, vận tải thủy; bệnh và thiếu đất khô |
| L03 | Phù Sa / lưu vực | 310 | 7.000 | Dầu hạt | Ruộng luân canh, gần chợ; phụ thuộc đê điều |
| L04 | Đông Viên / lưu vực | 260 | 6.000 | Vải gai | Đông dân, vùng nguyên liệu; năng suất ruộng thấp |
| L05 | Bạch Khâu / cao nguyên | 150 | 14.000 | Len trắng | Đồng cỏ; thiếu gạo và thợ dệt |
| L06 | Hắc Sơn / cao nguyên | 110 | 11.000 | Sắt rèn | Quặng tốt; đường đèo, nhập lương thực |
| L07 | Tùng Lĩnh / cao nguyên | 95 | 13.000 | Gỗ đóng tàu | Rừng, sông thả gỗ; tái sinh rừng chậm |
| L08 | Xích Nham / cao nguyên | 80 | 9.000 | Đồng thỏi | Mỏ đồng; cần nhiên liệu và vốn |
| L09 | Nam Cảng / duyên hải | 280 | 3.000 | Muối biển | Cảng lớn; phụ thuộc gạo nhập và an ninh biển |
| L10 | Hải Châu / duyên hải | 190 | 4.000 | Tàu đi biển | Thợ tàu, bến sâu; cần gỗ, sắt, vải buồm |
| L11 | Bạch Sa / duyên hải | 130 | 4.500 | Thủy tinh | Cát và thương nhân; tiêu hao nhiên liệu |
| L12 | Trầm Hải / duyên hải | 120 | 6.000 | Trầm hương | Rừng đặc hữu, hàng giá trị cao; cầu xa xỉ biến động |
| L13 | Vân Lụa / đông lộ | 240 | 6.000 | Lụa nhuộm | Thợ và mạng khách hàng; thương hội hạn chế gia nhập |
| L14 | Cẩm Giang / đông lộ | 230 | 5.500 | Giấy tốt | Sông, kỹ năng; tranh nước với ruộng và xưởng |
| L15 | Hồng Diệp / đông lộ | 160 | 8.000 | Trà | Khí hậu, thương hiệu; mùa vụ và phí đường |
| L16 | Bích Lò / đông lộ | 200 | 5.000 | Đồ sứ | Đất sét, thợ lò; cần nhiên liệu và vận tải ít hư hao |
| L17 | Bắc Mã / biên bắc | 90 | 20.000 | Ngựa chiến | Đồng cỏ, quân sự; dân thưa, phòng thủ đắt |
| L18 | Phong Ải / biên bắc | 120 | 15.000 | Da thuộc | Đàn gia súc, cửa ải; nguy cơ giao thương bị đóng |
| L19 | Hoàng Nguyên / biên bắc | 170 | 18.000 | Lúa mạch | Ruộng rộng; khô hạn và xa thị trường |
| L20 | Thiết Quan / biên bắc | 140 | 10.000 | Binh khí | Thợ quân giới, vị trí phòng thủ; lệ thuộc sắt và đơn đặt hàng |
| L21 | Dược Cốc / tây lộ | 100 | 9.000 | Dược liệu | Khí hậu đa dạng; khó tiếp cận, chất lượng không đồng đều |
| L22 | Lâm Mật / tây lộ | 115 | 10.000 | Sáp ong | Rừng và sản phẩm phụ; quyền sử dụng rừng tranh chấp |
| L23 | Thạch Kiều / tây lộ | 180 | 7.000 | Đá xây | Mỏ và nút giao; hàng nặng, phụ thuộc công trình |
| L24 | Ngân Khê / tây lộ | 85 | 8.000 | Bạc | Khoáng sản giá trị cao; dễ lệ thuộc thu nhập mỏ |

Ba khu triều đình: **Kinh Kỳ** (hành chính và tiêu dùng), **Hoàng Điền** (ruộng, kho dự trữ), **Trực Lộ** (hậu cần và xưởng nhà nước). Dân số và sản xuất của ba khu nằm trong tổng quốc gia, hạch toán riêng với chư hầu.

### 12.3 Các tuyến bắt buộc trong bản đồ đầu tiên

- Sông lớn: Thanh Điền → Phù Sa → Liên Trạch → Nam Cảng; có giới hạn công suất theo mùa.
- Tuyến đường cần cải thiện: Hắc Sơn → Bạch Khâu → Thanh Điền; nối ra Nam Cảng bằng sông.
- Tuyến ven biển: Nam Cảng ↔ Hải Châu ↔ Bạch Sa ↔ Trầm Hải.
- Tuyến đông: Thanh Điền ↔ Đông Viên ↔ Vân Lụa ↔ Cẩm Giang ↔ Hồng Diệp ↔ Bích Lò ↔ Hải Châu.
- Tuyến bắc: Bạch Khâu ↔ Bắc Mã ↔ Phong Ải ↔ Hoàng Nguyên ↔ Thiết Quan ↔ Hắc Sơn.
- Tuyến tây: Hắc Sơn ↔ Tùng Lĩnh ↔ Xích Nham ↔ Ngân Khê ↔ Thạch Kiều ↔ Dược Cốc ↔ Lâm Mật ↔ Phù Sa.
- Kinh Kỳ nối Thanh Điền và Trực Lộ; Hoàng Điền nối Thanh Điền và Phù Sa; Trực Lộ nối Thiết Quan.

Đây là mạng kết nối tối thiểu; chỉ bổ sung cạnh nếu có vai trò kinh tế hoặc chiến lược rõ ràng. Tọa độ, khoảng cách và sức tải được lưu trong dữ liệu bản đồ, không viết cứng trong luật kinh tế.

### 12.4 Những thuộc tính tạo khác biệt

Mỗi vùng có diện tích sử dụng được, sức chứa canh tác, nước, rừng, mỏ, năng suất nền, cơ cấu dân cư, kỹ năng, tài sản sản xuất, kho, đường, thị trường, nghĩa vụ phong kiến và cơ cấu quyền lực. Diện tích lớn không tự động tạo thu nhập lớn. Lợi thế địa lý chỉ có giá trị khi có nhân lực, vốn và kết nối để khai thác.

### 12.5 Bản đồ có địa lí thật trong thế giới giả tưởng

Phạm vi bản đầu là atlas 2D có địa hình và chiều sâu thị giác, không phải địa cầu 3D hoặc bản đồ Trái Đất chính xác. Mục tiêu là người chơi nhận ra núi, lưu vực, đồng bằng, rừng, đầm, bờ biển, cửa sông và vùng khô khi nhìn bản đồ; các đặc điểm đó có tác dụng trong mô phỏng.

- Dữ liệu nền gồm heightfield, độ dốc, biome, đất/nước, lưu vực, tài nguyên và polygon lãnh địa; tọa độ thế giới thống nhất theo km.
- Bản đồ 24 vùng vẫn được thiết kế thủ công để kiểm soát chất lượng. Seed thay mùa vụ/nhân vật/biến động, không tự đổi địa hình bản đồ này. Procedural map là mở rộng riêng.
- Dãy cao nguyên và biên bắc tạo địa hình cao; lưu vực có đồng bằng phù sa; duyên hải có cảng, vịnh và đầm; tây lộ có rừng/đèo. Không dán icon núi lên một nền phẳng rồi coi là đã mô phỏng địa lí.
- Sông theo lưu vực, đổ từ cao xuống thấp về hồ/cửa biển; trường hợp hồ hoặc đoạn thủy lợi có quy tắc rõ. Đường đi qua núi cần đèo, qua sông cần cầu/bến, không kéo thẳng qua mọi terrain.
- Terrain ảnh hưởng chi phí, thời gian, công suất vận tải, sản lượng phù hợp, dòng nước, bệnh và tiếp vận. Biên giới hành chính không tự chặn dòng sông.
- Tuyến thủy có chiều đi/ngược dòng và biến thiên mùa; cảng phụ thuộc chiều sâu/khả năng cập bến ở mức mô phỏng đơn giản. Không cần mô phỏng thủy động lực đầy đủ.
- Asset terrain, coastline và shader hillshade được đóng gói local, có nguồn/giấy phép rõ; game không tải map từ API lúc chơi.

### 12.6 Render, zoom và tương tác

Godot Node2D/Camera2D với terrain chunks, texture nền, polygon picking và layer đường/sông. Bắt đầu heightfield khoảng 2.048×2.048 samples; độ phân giải này phục vụ render/geography, **không được cộng vào số thực thể xã hội**. Độ phân giải cuối được chọn theo chất lượng và GPU tích hợp.

| Mức zoom | Nội dung chính | Thao tác |
|---|---|---|
| Quốc gia | Địa hình lớn, lãnh địa, thành phố, cảnh báo quan trọng | Chọn vùng, xem tuyến liên vùng, so sánh overlay |
| Vùng | Đèo/sông/đường, khu sản xuất, thành thị, công trình | Chọn facility/project, xem luồng hàng và quyền thu phí |
| Địa phương | Điểm dân cư, cụm hộ, lô hàng và người đang được theo dõi | Inspect/search cá thể, gia đình, hợp đồng, không bắt render mọi dân cư |

Pan/zoom liên tục, zoom giữ điểm dưới con trỏ; label không chồng lên nhau ở mức toàn quốc. Picking dùng world coordinate và spatial index, không thử mọi polygon/entity mỗi frame. Tooltip hiển thị vị trí, địa hình và dữ liệu đã biết với ngày cập nhật.

Overlay: địa hình, độ phì/nước, dân số, hàng hóa/giá, trade capacity, thẩm quyền, quyền lực và rủi ro. Mặc định một heatmap chính với legend + tuyến đang chọn; dùng pattern/nhãn hỗ trợ người khó phân biệt màu. Flow được tổng hợp khi zoom xa, chi tiết hóa khi zoom gần.

Chọn hai điểm để xem tuyến/cost/time/capacity; khi đề xuất đường/cầu, UI preview đoạn địa hình, quyền đất và vật tư trước khi tạo Project. Công cụ này không tự cấp quyền, tạo đường tức thì hoặc ưu tiên tuyến của vua vượt luật.

### 12.7 Mức chi tiết và thông tin bí mật

- Triệu người mô phỏng không đồng nghĩa triệu sprite/scene node. Vẽ density, cụm và icon đại diện ở xa; chỉ dựng visual cho đối tượng cần xem.
- Tên/chi tiết thường dân sinh khi inspect theo ID, không lưu một triệu chuỗi dài. Người được bookmark hoặc liên quan vụ việc giữ tên đã sinh.
- Vị trí và dòng hàng bí mật lọc qua PlayerKnowledge trước khi tạo overlay. Minimap, heatmap và tooltip không được rò plot vì render đọc world thật.
- Simulation fidelity không đổi khi camera rời vùng: người ở ngoài viewport vẫn có trạng thái và lịch quyết định. LOD ở đây là đồ họa, không thay họ bằng AI khác.

### 12.8 Gate bản đồ

Bộ dữ liệu kiểm thử chuẩn (fixture) là một trạng thái nhỏ được chuẩn bị sẵn để kiểm tra một chức năng, không phải diễn tiến định sẵn của campaign. Ví dụ hai vùng nối bằng một cây cầu, có một đoàn hàng đang đi; đóng cầu để kiểm tra đoàn hàng không đi xuyên qua và tuyến thay thế có chi phí đúng. Dùng các bộ tương tự để kiểm tra liên thông, sông/lưu vực, cầu/cảng và tác động của terrain. Native UI kiểm tra chọn đúng vùng sau pan/zoom, tìm cá thể, mở hồ sơ, overlay/legend và route preview. Người thử phải phân biệt được vùng mỏ, lưu vực, cảng và biên ải mà không đọc bảng ID. Bản đồ đẹp nhưng route/địa hình không ảnh hưởng mô phỏng chưa đạt yêu cầu.

## 13. Chính trị nhiều tầng

### 13.1 Các tầng mô phỏng

| Tầng | Chủ thể | Vấn đề tự xử lý |
|---|---|---|
| Trung ương | Vua, hội đồng triều đình, đại thần | Ngân sách, luật, nghĩa vụ, an ninh, dự án liên vùng |
| Lãnh địa | Lãnh chúa và bộ máy địa phương | Thu–chi, duy trì liên minh, thi hành, đầu tư, tranh chấp |
| Quyền lực địa phương | Gia tộc, thương hội, chủ mỏ/xưởng, quân sự, cộng đồng | Lợi ích tài sản, lao động, quyền hạn, hợp tác hoặc phản đối |
| Dân cư | Các nhóm hộ và nghề nghiệp | Tiêu dùng, làm việc, chuyển nghề, di cư, phản ứng tập thể |

Mỗi lãnh địa có 4 khối quyền lực lớn tại khởi đầu, chọn theo kinh tế địa phương; mỗi khối có một đại diện chính để người chơi theo dõi. Đây là lớp tổng hợp UI, không phải giới hạn chỉ 4 AI/vùng. Bên dưới có gia tộc, thương nhân, chủ xưởng, người cho vay, viên chức, chỉ huy và cá thể dân cư tự hành động. Có thể thêm tối đa 2 khối lớn nổi bật khi nền kinh tế đổi; thành viên có vòng đời độc lập. Household/Cohort là đơn vị gia đình hoặc tổng hợp dữ liệu, không thay thế danh tính các cá thể đã được mô phỏng riêng. Quy mô chi tiết ở phần quy mô của kế hoạch này.

### 13.2 Nhân vật và khả năng thi hành

Nhân vật có năng lực, tài sản hoặc quyền tiếp cận nguồn lực, chức vụ, quan hệ, mục tiêu, tính cách, danh tiếng, sức khỏe và hiểu biết riêng. Tách riêng:

- **Thiện chí:** có muốn giúp người ra lệnh không?
- **Lợi ích:** quyết định có lợi hoặc gây hại cho bản thân và liên minh không?
- **Khả năng:** có tiền, người, thẩm quyền và sự hợp tác cần thiết không?
- **Rủi ro:** hậu quả của tuân lệnh, trì hoãn hoặc chống đối là gì?

Không tồn tại trung thành tuyệt đối nhờ quê hương, học bổng hay huyết thống. Quan hệ tốt làm hợp tác dễ hơn nhưng không loại bỏ xung đột lợi ích.

### 13.3 Quyền hạn và nghĩa vụ

Mỗi lãnh địa có một hiệp ước nghĩa vụ xác định thuế/cống nộp, quân dịch, quyền bổ nhiệm, quyền thu phí và quyền kháng nghị. Thành thị đặc quyền có hiến chương phân chia quyền giữa hội đồng, lãnh chúa và vua.

Trên đất triều đình, vua quyết định thuế và nhân sự qua bộ máy của mình. Với chư hầu, vua yêu cầu hoặc thương lượng thay đổi nghĩa vụ; không có lệnh tăng thuế trực tiếp mọi hộ dân mà bỏ qua quyền địa phương. Trong tình trạng khẩn cấp, trưng thu vượt lệ có thể được ban hành, nhưng phải mô phỏng khả năng thi hành, phản đối và tiền lệ tạo ra.

Một vụ can thiệp tạo hồ sơ gồm căn cứ, phạm vi, người thi hành, thời hạn và kết quả. Tiền lệ ảnh hưởng kỳ vọng về những lần sau; không tự biến thành luật toàn quốc nếu chưa được ban hành.

### 13.4 Bộ hành động của NPC

Theo lịch của vai trò và khi có thông tin cần phản ứng, NPC cân nhắc các hành động hợp lệ: thương lượng, góp vốn, chi trả, trì hoãn, kiến nghị, kiện cáo, vận động liên minh, bảo trợ, giảm hợp tác và chống đối. Hành động bí mật gồm gặp gỡ, thỏa thuận kín, tài trợ phe hoặc chuẩn bị nổi loạn. Mỗi loại có nguồn lực và thời gian thi hành, không có nút tự sinh quân hoặc tự đạt đồng thuận.

Trong kiến trúc đích, policy RL đã huấn luyện chọn hành động dựa trên thông tin, ký ức, chỉ số, tính cách và mục tiêu của chính NPC. Engine chỉ sinh hành động khả dụng và thực hiện luật; không viết các nhánh chiến lược như “bất mãn trên 70 thì phản loạn”. Một bộ AI chấm điểm thủ công chỉ dùng làm baseline kiểm thử và đối thủ huấn luyện, không được quảng bá là RL. Mọi hành động phải tiêu tốn nguồn lực hoặc có điều kiện; NPC không được tự sinh tiền, quân hay bằng chứng. Thiết kế kỹ thuật đầy đủ ở DEVELOPER_GUIDE, phần policy và PPO.

Phần lớn việc địa phương tự kết thúc mà không cần vua. Chỉ đưa lên triều đình khi vượt thẩm quyền, cần trọng tài, ảnh hưởng nghĩa vụ hoặc có hậu quả liên vùng. Người chơi có thể mở hồ sơ xem quá trình bất kỳ lúc nào.

### 13.5 Liên minh, cam kết và ân huệ

Liên minh là tập hợp quan hệ và cam kết, không phải một thanh trung thành chung. Ân huệ có bên nợ, bên nhận, nguồn gốc, mức độ và điều kiện trả. Góp tiền tổ chức lễ hội không mặc định tạo một ân huệ nếu chưa có thỏa thuận.

Phản ứng khi thất hứa phụ thuộc thiệt hại, lý do, tiền lệ, bằng chứng và lựa chọn thay thế. Không dùng mức phạt cố định như giảm 40 trung thành. Danh tiếng giữ lời tác động tới chi phí vay, khả năng thương lượng và mức bảo đảm đối tác đòi hỏi.

### 13.6 Xung đột và thay thế quyền lực

Phản đối có nấc: kiến nghị → giảm hợp tác → liên minh gây sức ép → cưỡng chế hoặc nổi loạn. Không bắt buộc đi hết mọi nấc; một âm mưu có thể phát triển bí mật nhưng phải có người, nguồn lực và điều kiện.

Loại bỏ một lãnh chúa không tự chuyển toàn bộ đất, quân và sự ủng hộ cho vua. Phải giải quyết người kế nhiệm, quyền thừa kế, nhóm đang kiểm soát bộ máy và phản ứng của các chư hầu khác. Đảo chính là cuộc tranh quyền với lực lượng và liên minh thực tế, không chỉ một phép thử xác suất từ chỉ số bất mãn.

## 14. Sổ sách, thông tin và điều tra

### 14.1 Quy tắc thông tin

Các giao dịch thuộc sổ sách đã xác nhận là đáng tin cậy và không bị NPC tùy ý sửa lại. Báo cáo có ngày cập nhật, phạm vi, nguồn và độ trễ. Bản đầu không triển khai giả mạo kế toán; điều này giữ nền kinh tế có thể kiểm chứng.

Sổ sách công khai cho biết thu–chi công, nghĩa vụ, khoản đã nộp, kho công và tiến độ nghiệm thu. Tài sản tư nhân và hoạt động phi pháp không mặc định nằm trong sổ công. Thống kê toàn vùng như dân số hoặc sản lượng phi chính thức được ghi là ước tính; không trình bày như số kiểm toán chính xác.

Một khoản thất thoát đã được phát hiện vẫn là sự thật; người hưởng lợi hoặc động cơ có thể chưa rõ. Hệ thống bí mật không được thay đổi giao dịch quá khứ để hợp thức hóa tình tiết.

### 14.2 Dữ liệu người chơi được xem

| Loại | Ví dụ | Cách hiển thị |
|---|---|---|
| Đã xác nhận | Khoản nộp thuế, hợp đồng, chức vụ | Giá trị, nguồn và ngày cập nhật |
| Ước tính | Sản lượng chưa báo cáo đủ, lực lượng chưa kiểm kê | Khoảng ước tính và căn cứ |
| Hành vi công khai | Kiến nghị, bỏ phiếu, đóng cửa chợ | Sự kiện đã quan sát |
| Nghi vấn | Gặp đối thủ, mua lương thực bất thường | Dấu hiệu, giả thuyết, chưa kết luận |
| Bằng chứng bí mật | Thư, thỏa thuận, lời khai đã đối chiếu | Nội dung, nguồn, độ tin cậy và giới hạn |

### 14.3 Điều tra

Người chơi chọn đối tượng, vấn đề cần xác minh, người phụ trách và ngân sách. Phương thức gồm thanh tra, đối chiếu hồ sơ, tiếp kiến, thu thập tin từ thương nhân và theo dõi quan hệ.

Điều tra có thời gian, chi phí, khả năng bị phát hiện và xung đột lợi ích của người thực hiện. Kết quả là bằng chứng về một hành động hoặc mối quan hệ; không mở khóa toàn bộ suy nghĩ của nhân vật. Kết quả không tìm thấy bằng chứng phải phân biệt với đã chứng minh vô tội.

NPC có sổ hiểu biết riêng và có thể bị đánh lừa bởi lời cáo buộc. Giao diện chỉ lấy dữ liệu từ lớp thông tin của người chơi; trạng thái bí mật thật chỉ xuất hiện trong công cụ phát triển và kiểm thử.

## 15. Kinh tế, sản xuất và thương mại

### 15.1 Đơn vị và các chủ thể

Mỗi nhóm kinh tế có ngân quỹ, tài sản, lao động, hàng tồn, nợ và quyền sử dụng đất/tài nguyên. Tách riêng tiền triều đình, tiền chính quyền lãnh địa và tài sản tư của nhà vua, lãnh chúa, thương hội. Không coi toàn bộ sản lượng vùng là thu nhập của lãnh chúa.

Dùng đơn vị kế toán **đồng bạc** với số nguyên theo đơn vị nhỏ nhất; hàng hóa có đơn vị chuẩn riêng. Bạc khai mỏ thuộc hàng kim loại, không tự tạo tiền. Bản đầu giả định chế độ tiền ổn định; không triển khai đúc tiền, phá giá hay ngân hàng tạo tiền. Tín dụng là chuyển tiền có thật từ người cho vay sang người vay.

### 15.2 Mười sáu nhóm hàng

1. Lương thực khô; 2. Thực phẩm bổ sung; 3. Muối; 4. Gỗ; 5. Nhiên liệu; 6. Sắt; 7. Kim loại màu/quý; 8. Xơ sợi; 9. Vải; 10. Gia súc/ngựa; 11. Công cụ; 12. Quân nhu/vũ khí; 13. Vật liệu xây dựng; 14. Giấy; 15. Dược liệu; 16. Hàng xa xỉ.

Mỗi nhóm có thể có biến thể chất lượng hoặc công dụng. Ngựa chiến và gia súc thường không thay thế nhau trong quân dịch; sắt và đồng không thay thế tùy ý trong công thức. Các công thức chỉ nhận biến thể hợp lệ. Tàu là tài sản vận tải tạo bằng dự án, không là nhóm hàng tiêu dùng thứ 17.

Đặc sản ánh xạ vào nhóm và biến thể: len → xơ sợi; lụa → vải cao cấp; trà, trầm, thủy tinh trang trí và đồ sứ tốt → hàng xa xỉ. Sáp ong là biến thể nguyên liệu có công dụng xác định. Tên đặc sản giữ bản sắc, nhưng không tạo 24 cơ chế giá riêng không cần thiết.

### 15.3 Sản xuất và nhu cầu

Cơ sở sản xuất thuộc một chủ thể, có ngành, công suất, người làm, kỹ năng, nguyên liệu, tài sản và hao mòn. Sản lượng bị giới hạn bởi yếu tố thiếu nhất:

`sản lượng = công suất hiệu dụng × min(tỷ lệ lao động, tỷ lệ đầu vào, tỷ lệ tài nguyên khả dụng) × hệ số mùa vụ`

Các tỷ lệ bị chặn trong miền hợp lệ; mọi đầu vào được trừ đúng lượng. Nông nghiệp dùng lịch mùa: gieo trồng tạo sản phẩm dở dang, thu hoạch mới nhập kho. Mất mùa tác động đến vụ và kho; không tạo lương thực đều mỗi tháng.

Nhu cầu hộ dân theo quy mô dân số, mức sống và khả năng chi trả. Ưu tiên sinh tồn trước hàng xa xỉ. Tiền công, địa tô, lợi nhuận và thuế xác định phân phối thu nhập. Thiếu lương thực ảnh hưởng sức khỏe và lao động trước khi dẫn tới tử vong hoặc di cư; có cảnh báo và thời gian phản ứng.

### 15.4 Thị trường và giá

Mỗi vùng có chợ, tồn kho và giá theo nhóm hàng. Hộ và tổ chức lập nhu cầu mua trong ngân sách; người bán đưa hàng có thật. Khi thiếu hàng, phân bổ theo các vòng nhu cầu: nghĩa vụ hợp đồng đã bảo đảm bằng hàng, nhu cầu thiết yếu, sau đó nhu cầu còn lại; trong mỗi vòng phân bổ theo lượng đặt mua có khả năng thanh toán. Trưng thu là chính sách riêng, không mặc định mọi nhu cầu triều đình được ưu tiên.

Giá tháng sau điều chỉnh dần theo tỷ lệ cầu chưa đáp ứng và tồn kho so với mục tiêu. Dùng hệ số và biên độ thay đổi cấu hình được; mặc định giới hạn biến động mỗi tháng để tránh vòng lặp số học mất ổn định. Biên độ giá và mọi giới hạn kỹ thuật phải có trong cấu hình cân bằng, không rải trong mã.

Tách giá danh nghĩa và thu nhập thực. Giá tăng không đồng nghĩa quốc gia giàu hơn. Khi hiển thị tăng trưởng, có cả sản lượng thực và khả năng đáp ứng nhu cầu.

### 15.5 Thương mại và vận tải

Thương nhân có thông tin chênh lệch giá, vận tải, phí, hao hụt, thời gian và rủi ro; **policy kinh tế** quyết định có giao dịch, chọn thị trường và mức đặt mua/bán. Engine kiểm tra quyền, tiền, hàng và sức tải rồi thực hiện. Hàng đã mua chuyển sang trạng thái đang vận chuyển; chỉ nhập kho đích khi đến nơi. Giá cuối cùng có thể khác dự báo.

Tuyến có công suất dùng chung; lô hàng không được dùng quá sức tải còn lại. Engine tìm các tuyến khả thi theo mục tiêu dịch vụ mà NPC đã chọn (rẻ/nhanh/an toàn); đây là giải thuật thực hiện lệnh, không thay policy quyết định chiến lược. Đơn đã đặt được phân bổ theo hợp đồng, cơ chế thị trường và khóa phụ ổn định; không xếp ưu tiên bằng một AI lợi nhuận viết tay. Có thể đổi tuyến cho lô chưa khởi hành; lô đang đi chịu quy tắc gián đoạn cụ thể, không dịch chuyển tức thì.

Phí vận tải và cầu đường chuyển cho chủ thể cung cấp dịch vụ/quyền thu phí. Hàng hỏng hoặc bị mất có sự kiện và lượng tổn thất; cướp hàng chuyển sang bên nhận nếu có, không được đồng thời ghi mất và ghi chuyển.

### 15.6 Đầu tư và chuyển đổi đặc sản

Mỗi quý, policy kinh tế chọn mở rộng, lập ngành, thu hẹp, đóng cửa hoặc tiếp tục trên cơ sở lịch sử lợi nhuận, thông tin cầu, đầu vào, nhân lực, vốn, vận tải và chính sách. Mức dự phòng do policy chọn trong miền khả dụng; engine không dùng công thức ROI để tự chọn thay. Dự án chỉ bắt đầu khi có tài trợ thật và điều kiện luật phù hợp.

Mỗi vùng khởi đầu với một đặc sản, 2–3 ngành phụ và 2–3 hướng phát triển có điều kiện. Đây là các cơ hội ưu tiên, không phải cấm phát triển ngành khác nếu đáp ứng điều kiện chung. Công thức dự án quy định đầu vào, thời gian, lao động và năng lực cần thiết.

Ví dụ Bạch Khâu: chăn cừu → xưởng sơ chế len → dệt vải len. Xưởng dệt cần thợ, công cụ, vốn và đường tiêu thụ; có len rẻ là một lợi thế nhưng chưa đủ. Nhà vua có thể hỗ trợ đường, đào tạo, hợp đồng mua hoặc sửa thuế, không bấm một nút đổi đặc sản.

Danh tiếng đặc sản dùng cửa sổ 5 năm về sản lượng bán ra, chất lượng và thị phần; cập nhật hàng năm. Chỉ đổi khi ngành mới dẫn trước bền vững trong 3 lần đánh giá năm. Hai nhãn có thể tồn tại trong giai đoạn chuyển tiếp để tránh thay đổi đột ngột.

### 15.7 Nợ và phá sản

Khoản vay có người cho vay, người vay, gốc, lãi, kỳ hạn, lịch trả và bảo đảm nếu có. Vay mới chuyển tiền; trả gốc giảm cả tiền và nghĩa vụ; lãi là thu nhập của bên cho vay. Vỡ nợ tạo thương lượng gia hạn, chuyển tài sản hoặc phá sản; không xóa nợ miễn phí và không sinh tiền để bù.

Nợ chính quyền có thể tạo sức ép chính trị từ chủ nợ. Nhà vua cứu trợ sẽ chuyển rủi ro sang ngân khố và tạo kỳ vọng cho lần sau. Bản đầu không có thị trường chứng khoán hoặc tài chính phái sinh.

## 16. Phát triển đất nước và chính sách

### 16.1 Dân số, lao động và di cư

**Đối chiếu với hướng hiện hành:** đoạn sau ghi thiết kế v3 về aggregate/cohort. Mỗi NPC đã mô phỏng phải giữ ID/state/lifecycle/agency riêng; cohort chỉ tổng hợp hoặc biểu diễn phần dân số được công bố riêng. Không tự chuyển cá thể thành cohort để đạt benchmark hoặc giảm scope.

Dân cư có hai cách biểu diễn được phân biệt rõ: IndividualCitizen là một người có ID, tuổi, gia đình, nghề, nhu cầu, xu hướng và lịch quyết định riêng; Household/Cohort tổng hợp tiêu dùng, lao động và cư dân chưa được mô phỏng từng người. Những cá thể đã có ID không tiếp tục bị đếm vào dân số cohort nền. Policy thường dân chọn công việc, học tập, kế hoạch chi tiêu, giao tiếp, tham gia nhóm hoặc ý định di cư; quyền, tuổi, tiền và thời gian là ràng buộc thế giới. Trẻ em có tập hành động phù hợp giai đoạn sống và ngân sách gia đình; không sao chép toàn bộ tập hành động tài chính của lãnh chúa.

Nhóm kinh tế thuê lao động từ nguồn lao động có thật; quân dịch và công trường giảm lao động cho sản xuất. Một người không đồng thời được tính đầy đủ ở nhiều ngành. Tăng dân số cập nhật chậm theo sức khỏe và sinh–tử; đầu tư y tế không lập tức tạo lao động trưởng thành.

### 16.2 Các lĩnh vực đầu tư công

| Lĩnh vực | Lợi ích | Ràng buộc và hệ quả chính trị |
|---|---|---|
| Đường, cầu, cảng | Giảm chi phí, tăng sức tải | Mua đất, bảo trì, tranh quyền thu phí |
| Thủy lợi, đê | Tăng ổn định mùa vụ | Quyền dùng nước, bảo dưỡng, lợi ích thượng/hạ lưu |
| Kho dự trữ | Giảm mức độ thiếu hàng khi khủng hoảng | Chi phí tồn kho, hao hụt, tranh quyền phân phối |
| Giáo dục, học nghề | Tăng kỹ năng và năng lực hành chính | Mất thời gian, thay đổi quyền lực người giữ nghề |
| Y tế, nước sạch | Giảm bệnh và gián đoạn lao động | Cần nhân lực, vật tư và ngân sách thường xuyên |
| Tư pháp, hành chính | Hợp đồng đáng tin, thi hành tốt hơn | Đụng đặc quyền, tăng chi bộ máy |

Phát triển công nghệ qua năng lực và dự án tiếp thu: điều kiện kiến thức, nhân lực, vật liệu, thử nghiệm và áp dụng. Tránh cây nâng cấp chỉ cộng phần trăm toàn quốc. Công nghệ đã biết chưa đồng nghĩa mọi vùng đủ sức áp dụng.

### 16.3 Dự án và chính sách là đối tượng có vòng đời

Dự án: đề xuất → thỏa thuận/tài trợ → chuẩn bị → thi công → nghiệm thu → vận hành/bảo trì; có thể đình trệ, sửa phương án hoặc hủy. Chính sách: đề xuất → đạt thẩm quyền/cam kết → ban hành → thực hiện → đánh giá/sửa đổi.

Mỗi đối tượng ghi bên chịu chi phí, bên hưởng quyền, người thi hành, mục tiêu, hạn và nguồn tài trợ. Dự án được cấp đủ tiền nhưng thiếu nhân lực hay quyền sử dụng đất vẫn có thể chậm. Khoản chi đã thực hiện phải có đối ứng; số dư được hoàn lại khi hủy, chi phí chìm không hoàn lại.

### 16.4 Phát triển và quyền lực

Tài sản, thu nhập, việc làm phụ thuộc, chức vụ, tổ chức và quân lực đóng góp các loại sức mạnh khác nhau. Tiền có thể tài trợ vận động nhưng không trực tiếp chuyển thành quyền chỉ huy quân đội. Mỗi loại hành động kiểm tra nguồn lực liên quan.

Cảng mới có thể tăng quyền mặc cả của thương nhân; đường mới giúp triều đình thi hành nhanh hơn; trường học tạo nguồn quan lại ngoài các gia tộc cũ. Không quy định mọi sự phát triển đều có cùng một kết quả chính trị.

## 17. Quốc phòng, ngoại giao và khủng hoảng

Chiến tranh phục vụ mô phỏng nhà nước; bản đầu không có điều khiển chiến thuật từng đơn vị. Quân lực có chủ chỉ huy, nghĩa vụ, quân số, trang bị, tiền lương, lương thực và tuyến tiếp vận. Chư hầu có thể thiếu khả năng huy động dù muốn tuân lệnh; huy động làm giảm lao động và thu nhập địa phương.

Ba thế lực bên ngoài tạo quan hệ giao thương, đe dọa và điều ước. Trong bản đầu, họ là tác nhân kinh tế–ngoại giao tổng hợp có ngân sách, cung/cầu hữu hạn và trạng thái quan hệ; không mô phỏng đầy đủ lãnh địa nội bộ của họ. Ngoại thương phải hạch toán với khu vực bên ngoài, không trở thành nguồn mua vô hạn.

Thời tiết, bệnh, tranh chấp, tắc vận tải và xung đột hình thành từ trạng thái thế giới, hành động NPC và quá trình ngẫu nhiên theo không gian/thời gian. Không có lịch “năm thứ ba bắt buộc nổi loạn” hoặc một quota khủng hoảng phải sinh. Nội dung diễn giải sự kiện đã phát sinh và chuỗi nguyên nhân; không tạo nguyên nhân hồi tố. Khủng hoảng nghiêm trọng có thể tự pause theo thiết lập của người chơi. Kiến trúc sự kiện phát sinh và tiêu chí đa dạng ở phần 21.

Các hành động cưỡng chế như trưng thu, bắt giữ hoặc tước chức cần quyền thực hiện, người thi hành và chi phí. Không có tùy chọn ám sát miễn phí, phát hiện mọi âm mưu hoặc miễn nhiễm đảo chính. Hệ thống ám sát chủ động và chiến tranh chiến thuật nằm ngoài phạm vi bản đầu.

## 18. Giao diện và trải nghiệm chơi

### 18.1 Những màn hình chính

1. **Tổng quan quốc gia:** ngân sách, cung ứng thiết yếu, dự án, nghĩa vụ và tối đa 5 việc cần chú ý nhất.
2. **Bản đồ:** lớp địa lý, sản xuất, giá, thương mại, quyền lực và rủi ro; chọn vùng để mở chi tiết.
3. **Lãnh địa:** sổ sách, kinh tế, người nắm quyền, công việc đang xử lý, thông tin và điều tra.
4. **Triều đình và quan hệ:** chức vụ, cam kết, liên minh, tiếp kiến và vụ phân xử.
5. **Kinh tế quốc gia:** hàng hóa, dòng hàng, vốn, lao động, chênh lệch giá và nợ.
6. **Dự án/chính sách:** phương án, bên liên quan, tài trợ, tiến độ và hậu quả đã quan sát.
7. **Hồ sơ và lịch sử:** báo cáo có nguồn, bằng chứng, giao dịch và chuỗi nguyên nhân.

### 18.2 Trình bày quyết định

Mỗi quyết định cho biết mục tiêu, chi phí biết chắc, điều kiện còn thiếu, bên chịu tác động, thời gian và dự báo có bất định. Cho phép mở chi tiết để hiểu vì sao một dự báo được đưa ra; không hiển thị ý định bí mật làm dữ liệu chắc chắn.

Ví dụ thông báo: “Đường Hắc Sơn chậm hai tháng. Ngân sách đủ; quyền sử dụng đất chưa giải quyết. Lãnh chúa đang thương lượng với gia tộc giữ đèo.” Nút thao tác: xem hồ sơ, tiếp kiến, phân xử, theo dõi. Điều tra chỉ cần thiết khi có vấn đề chưa rõ.

Luồng tin gộp việc thường lệ thành báo cáo tháng. Có bộ lọc quan tâm và đánh dấu lãnh địa theo dõi. Người chơi không phải xử lý mọi tranh chấp; mặc định người có thẩm quyền tiếp tục tự xử lý nếu vua không can thiệp.

### 18.3 Điều khiển và phong cách

- Bản đồ 2D có địa hình và đổ bóng tạo cảm giác chiều sâu; bảng điều hành theo phong cách atlas/lịch sử. CRT xanh/hổ phách/cyan chỉ là theme tùy chọn cho panel/console.
- Pan bằng kéo chuột/nút giữa hoặc WASD; zoom quanh con trỏ bằng wheel; click chọn vùng/đối tượng; ESC bỏ chọn/đóng panel. Có legend, tìm kiếm và nút về thủ đô.
- Chuột và bàn phím thực hiện cùng hành động; CLI là đường thao tác nhanh.
- Phím Space tạm dừng; tốc độ 1×/2×/4×/8×. Mặc định 1× = 1 ngày game/giây thực, điều chỉnh sau kiểm thử.
- Game khởi đầu tạm dừng; tự dừng khi cửa sổ mất focus/thu nhỏ hoặc mở quyết định quan trọng theo thiết lập. Không chạy bù thời gian khi trở lại hoặc máy thức dậy.
- Âm thanh tùy chọn và tắt nhanh; hỗ trợ giảm chuyển động, tương phản và cỡ chữ.

RL chạy bằng CPU không đồng nghĩa đồ họa không dùng GPU tích hợp. Mục tiêu desktop dùng renderer Compatibility của Godot và iGPU hỗ trợ OpenGL phù hợp; không yêu cầu GPU rời, CUDA hay Vulkan làm đường chạy bắt buộc. Cấu hình tham chiếu thực tế phải được ghi trước benchmark.

Lệnh mẫu: `map`, `inspect L06`, `ledger L06`, `market iron`, `petition <id>`, `project <id>`, `investigate <case_id>`, `speed 4`, `pause`, `help`. Mọi lệnh thay đổi trạng thái phải đi qua cùng bộ kiểm tra quyền và điều kiện như giao diện.

### 18.4 Khởi đầu chiến dịch

Hướng dẫn trong game dùng một vụ nộp thuế chậm và một dự án nhỏ để dạy sổ sách, quyền hạn và giao việc. Không bắt đầu bằng nhiều tối hậu thư. Có ba mức hỗ trợ về thông tin/gợi ý và độ khắc nghiệt của điều kiện ban đầu; không cho NPC gian lận tài nguyên ở độ khó cao.

Sau khi xem quốc gia và hồ sơ vua, người chơi draft một augment khởi đầu trong ba lựa chọn cùng cấp sức mạnh. Augment tạo lợi thế, khả năng riêng và các hướng phối hợp với nhân sự/chính sách; không miễn nhiễm phản loạn hoặc tạo trung thành tuyệt đối. Luật draft và danh mục ở phần 19; hồ sơ nhân vật ở phần 20.

### 18.5 Bảng hướng dẫn mở trong lúc chơi — yêu cầu bổ sung của người dùng

Luôn có nút **Hướng dẫn** trên thanh điều khiển và phím **F1** để mở bảng. Người chơi không phải thoát chiến dịch hoặc đọc tài liệu bên ngoài. Bảng có tìm kiếm, mục lục, thuật ngữ dễ hiểu và ví dụ thao tác; hỗ trợ cả chuột lẫn bàn phím. Khi mở toàn bộ bảng, game tự pause; đóng bảng giữ nguyên pause để người chơi chủ động tiếp tục. Trợ giúp nhỏ trong tooltip không tự pause.

| Mục trong bảng | Nội dung và cách dùng |
|---|---|
| Bắt đầu | Mục tiêu đời vua, đọc tổng quan, những việc nên kiểm tra đầu tiên; cho chạy lại tutorial riêng mà không sửa campaign |
| Điều khiển | Pan/zoom/chọn vùng, đổi lớp bản đồ, pause/tốc độ, mở hồ sơ, console và phím tắt |
| Hiểu bản đồ | Địa hình, tuyến, công suất, lớp giá/dân số/quyền lực; legend và ngày cập nhật |
| Kinh tế và sổ sách | Tiền của ai, thu/chi, kho, sản xuất, giá, vận tải, nợ; ví dụ lần theo một khoản thu |
| Cai trị và giao việc | Thẩm quyền, nghĩa vụ, tiếp kiến, thương lượng, dự án và người thi hành |
| Nhân vật và thông tin | Năng lực, tính cách, gia đình, quan hệ; phân biệt sự thật, ước tính, nghi vấn và bằng chứng |
| Xử lý vấn đề | Cách tìm nguyên nhân thiếu hàng, chậm dự án, thiếu tiền, tranh chấp; liệt kê hướng xử lý có điều kiện, không hứa một đáp án luôn đúng |
| Lệnh nhanh | Command có cú pháp, tham số, ví dụ, chi phí/điều kiện và nút mở màn hình liên quan |
| Lưu và kết thúc | Save/load, autosave, tiếp tục, kết thúc chiến dịch và cách đọc di sản |
| Thuật ngữ | Giải thích các từ trong game bằng tiếng Việt; không đưa thuật ngữ phát triển như fixture vào UI người chơi |

Các panel kinh tế, bản đồ, nhân vật và dự án có nút **?** mở đúng mục hướng dẫn; command sai dẫn tới phần cú pháp tương ứng. Bảng giải thích cơ chế công khai nhưng không lộ ý định NPC, bí mật hoặc tương lai chưa xảy ra. Ví dụ giả định được đánh dấu rõ, không giả làm dữ liệu campaign hiện tại.

Nội dung hướng dẫn phải cập nhật cùng phiên bản luật/command. Kiểm thử native: F1/nút mở được trong campaign, focus bàn phím đúng, tìm kiếm và link theo ngữ cảnh hoạt động, clock không tiến khi bảng mở, và đóng bảng không tự resume ngoài ý muốn. Đây là yêu cầu của bản hoàn chỉnh, không chỉ tutorial một lần lúc đầu.

## 19. Augment khởi đầu và các yếu tố tạo hướng chơi

### 19.1 Mục tiêu

Mỗi chiến dịch có một lựa chọn tạo khác biệt rõ ngay đầu game: người chơi nhìn ba thẻ, chọn một, rồi xây đội ngũ và chính sách tận dụng nó. Lấy cảm hứng từ trải nghiệm chọn augment, không sao chép tên, hình ảnh hoặc bộ thông số của TFT. Bản đầu chỉ có draft khởi đầu; không tự thêm các đợt nhận augment giữa đời vua.

Augment là quy tắc gameplay có giải thích trong thế giới, không bắt buộc tất cả đều phải có bất lợi tương xứng. Chi phí cơ hội là bỏ hai thẻ còn lại; những thẻ liên quan liên minh hoặc tín dụng còn có nghĩa vụ thực tế. Hiệu ứng mạnh tạo cơ hội, không bảo đảm thành công hoặc buộc NPC đồng ý.

### 19.2 Luật draft

1. Khởi tạo bản đồ, vua, nhân vật và sổ sách nền; game tạm dừng. Cho xem tổng quan và tài nguyên trước khi chọn.
2. Chọn cấp draft theo seed: Bạc 50%, Vàng 35%, Kim cương 15%; ba thẻ cùng cấp. Chế độ thử nghiệm cho chọn cấp, được ghi vào metadata chiến dịch.
3. Lấy ngẫu nhiên ba augment khác nhau đủ điều kiện, thuộc ba hướng khác nhau. Pool bản đầu có bốn thẻ mỗi cấp, đều có thể dùng trên bản đồ cố định.
4. Người chơi được đổi **một thẻ đúng một lần**. Thay bằng thẻ hợp lệ chưa thấy cùng cấp; giữ hai thẻ còn lại. Reroll không dùng tiền trong game.
5. Chọn một thẻ, hiển thị đầy đủ nguồn lực/cam kết được tạo rồi áp dụng nguyên tử. Thẻ và seed draft lưu ngay; tải lại save không sinh bộ mới.
6. Các tỷ lệ xuất hiện và hệ số là thông số cân bằng, chưa được chứng minh tương đương sức mạnh. So sánh chiến dịch theo cấp draft và độ khó; không trộn tất cả vào một bảng xếp hạng công bằng giả định.

Augment là của triều đại người chơi. NPC có trait, chỉ số, xuất thân và lợi thế tài sản; không tự được nhận bản sao thẻ của vua. Policy phải được huấn luyện với các biến thể augment mà nó có thể quan sát qua hiệu ứng công khai.

### 19.3 Bộ 12 augment ban đầu

`R0` là tổng thu ngân khố triều đình trong một năm cơ sở, tính từ dữ liệu khởi đầu trước augment và đóng băng làm đơn vị cân bằng. Các phần trăm dưới đây là giá trị thử đầu tiên.

| ID / cấp | Tên | Hiệu ứng triển khai | Hướng phối hợp và giới hạn |
|---|---|---|---|
| A01 / Bạc | Sổ Bộ Chuẩn Mực | Giảm 20% thời gian tổng hợp báo cáo công; +10 Administration hiệu dụng cho bộ máy kiểm kê triều đình, chặn ở 100 | Thông tin đến nhanh, không nhìn được sổ tư hoặc bí mật |
| A02 / Bạc | Lò Đào Tạo Hoàng Gia | Giảm 15% thời gian đào tạo nghề tại dự án triều đình tài trợ | Vẫn cần giáo viên, lương thực và trả chi phí; không tạo lao động mới |
| A03 / Bạc | Người Đứng Giữa | Mỗi 180 ngày mở một phiên hòa giải bổ sung ngoài công suất tiếp kiến thường lệ; giảm 20% thời gian chuẩn bị hồ sơ phiên này | Tăng cơ hội thương lượng; bên tranh chấp có thể từ chối |
| A04 / Bạc | Kho Thóc Tiền Triều | Hoàng Điền bắt đầu có thêm kho đủ chứa 3 tháng nhu cầu thiết yếu của dân đất triều đình, chứa đầy một lần | Nguồn là tài sản khởi đầu ghi sổ; vẫn hao hụt và cần bảo trì |
| A05 / Vàng | Liên Minh Công Thương | Tạo đề nghị tín dụng từ liên minh thương hội tới 0,5×R0, trả trong 5 năm, lãi bằng 75% lãi chuẩn khởi đầu | Tiền nằm trong tài khoản quỹ khởi đầu đã hạch toán; vay tùy chọn, có chủ nợ và nghĩa vụ |
| A06 / Vàng | Đoàn Công Trình Hoàng Gia | Bắt đầu có một đoàn kỹ sư; +20% năng suất lao động xây đường/cầu khi được phân công | Chỉ một dự án cùng lúc; không giảm yêu cầu vật tư, quyền sử dụng đất và lương |
| A07 / Vàng | Môn Sinh Khắp Chốn | Ba ứng viên quan chức có chuyên môn khác nhau xuất hiện trong danh sách tuyển; mỗi người có một skill nghề 70–80 | Phải trả lương, có tính cách/gia tộc riêng; không trung thành tuyệt đối |
| A08 / Vàng | Mạng Lưới Thư Trạm | Giảm 25% thời gian chuyển tin trên đường do triều đình kiểm soát; thêm một điều tra đồng thời | Điều tra vẫn cần nhân lực/ngân sách; không tự có bằng chứng |
| A09 / Kim cương | Ngân Quỹ Kiến Quốc | Khởi đầu thêm quỹ công hạn chế mục đích bằng 1×R0 để đồng tài trợ hạ tầng tối đa 50% mỗi dự án | Là tài sản ban đầu khai báo; không dùng trả nợ hay mua chuộc; hết quỹ không tái tạo |
| A10 / Kim cương | Hiến Ước Liên Lãnh | Ba chư hầu liền kề đã ký khung thương lượng chung và mở chức năng dự án liên lãnh chung một hồ sơ | Chọn bộ ba trong dữ liệu; mỗi dự án vẫn cần đóng góp và chấp thuận; các bên có quyền kháng nghị |
| A11 / Kim cương | Học Viện Bách Nghệ | Bắt đầu có học viện cấp nền; hai dự án chuyển giao nghề cùng lúc, tăng 25% tốc độ huấn luyện giảng viên | Cần ngân sách vận hành, điều kiện đầu vào và người học; không nhân đôi mọi công nghệ |
| A12 / Kim cương | Hội Đồng Khai Quốc | Có hai chức vụ cố vấn đặc biệt với hai ứng viên chuyên môn cao; mỗi cố vấn có thể chịu trách nhiệm một vụ việc bổ sung | Chiếm ngân sách lương và tạo hai nhóm bảo trợ có lợi ích riêng |

Mọi tài sản “được thêm lúc bắt đầu” được ghi là endowment của kịch bản trước bước mô phỏng đầu tiên; hiển thị trong bảng tài sản mở sổ. Không cộng tiền/hàng âm thầm vào nền kinh tế sau khi game đã chạy. Quỹ A05 cần có số dư thật trước khi giải ngân.

### 19.4 Kết hợp và kỹ năng chủ động

Tổ hợp hình thành giữa augment, nhân sự, vùng và chính sách. Ví dụ: Đoàn Công Trình + quan giỏi hành chính + tuyến vùng mỏ giúp triển khai tốt hơn, nhưng vẫn cần lãnh chúa giải quyết quyền thu phí. Môn Sinh + học nghề ở Bạch Khâu có thể đẩy nhanh ngành dệt; nguồn cầu và vốn vẫn quyết định có nên đầu tư.

Mặc định chỉ một augment nên không cần cộng dồn vô hạn. Modifier chỉ áp dụng cho trường và phạm vi đã khai báo. Với nhiều nguồn cùng loại, cộng phần trăm rồi chặn: giảm thời gian tối đa 40%, tăng năng suất do trait/augment tối đa 50%; chất lượng tài sản và lao động vẫn tính riêng. Giới hạn tối thiểu một ngày là tham số của thiết kế v3 cho các quy trình này; không áp nó lên mọi session subday của M5. Khi chuyển augment sang timeline mới phải định nghĩa lại đơn vị và rounding trong contract rồi review.

Thẻ có năng lực chủ động hiển thị mục tiêu, cooldown, người phụ trách và chi phí. Cooldown tính theo ngày game; pause không làm nó hồi. UI cho xem so sánh trước–sau, không giấu hệ số quan trọng sau lời mô tả mơ hồ.

### 19.5 Dữ liệu augment

`AugmentDefinition` gồm ID, cấp, nhóm hướng chơi, điều kiện, mô tả, modifier, tài sản khởi đầu, quyền/hợp đồng được tạo, năng lực chủ động, tag loại trừ và phiên bản. Dùng danh sách kiểu hiệu ứng được engine hỗ trợ; không cho thẻ chạy mã tùy ý. `DraftState` ghi pool, ba lựa chọn, lịch sử reroll, lựa chọn cuối và hash cấu hình.

Kiểm thử từng thẻ độc lập và mọi cặp thẻ có thể xuất hiện trong cùng bộ; endowment chỉ áp dụng một lần, load không nhân đôi. Tỷ lệ chọn thẻ là tín hiệu cân bằng, không phải bằng chứng sức mạnh ngang nhau.

## 20. Chỉ số và cá tính nhân vật chủ chốt

### 20.1 Sáu chỉ số năng lực, thang 0–100

| Stat | Tên hiển thị | Ảnh hưởng gameplay | Không quyết định |
|---|---|---|---|
| INT | Trí lược | Năng lực dự báo có điều kiện, khai thác ký ức và ngân sách cân nhắc; chi tiết DEVELOPER_GUIDE, phần policy và PPO | Lòng tốt, tham vọng hoặc quyền biết bí mật |
| ADM | Hành chính | Điều phối công việc, mức tải hồ sơ và chậm trễ trong phạm vi được giao | Tự có nhân sự/vật tư hoặc quyền vượt hiến chương |
| ECO | Kinh tài | Độ chính xác dự toán ngành, khả năng tổ chức mua sắm và tài chính | Tiền miễn phí, giá mua luôn thấp hơn thị trường |
| DIP | Ngoại giao | Chuẩn bị thương lượng, độ rõ của đề nghị và giảm ma sát giao tiếp | Ép đối phương chấp nhận thỏa thuận bất lợi |
| INTG | Mưu lược | Chất lượng tổ chức điều tra/bảo mật, xử lý nguồn tin và che giấu hành động | Đọc suy nghĩ hay sửa sổ sách đã xác nhận |
| MIL | Quân sự | Chuẩn bị huy động, điều phối tiếp vận và thực hiện kế hoạch quân sự | Đổi lính thiếu đói thành quân vô địch |

INTG được dùng làm mã riêng cho Intrigue để không nhầm với INT. Các chỉ số vật chất như tài sản, uy tín, quyền lực, quân số và sức khỏe được hiển thị riêng, không cộng vào “tổng sức mạnh” của nhân vật.

Mặc định tạo nhân vật chủ chốt với trung bình 45–60, một chuyên môn mạnh 65–85, ít nhất một điểm yếu dưới 45. Dùng ngân sách chỉ số cùng seed và vai trò; không để mọi lãnh chúa đều có bộ stat cân bằng giống nhau. Nhân vật xuất chúng hiếm được kiểm soát bằng nội dung kịch bản, không gán một mô hình mạng lớn riêng cho họ.

### 20.2 Tách năng lực, tính cách và quan hệ

Tính cách dùng sáu trục 0–1: tham vọng quyền lực, vụ lợi tài sản, quan tâm cộng đồng, trọng cam kết, mức chấp nhận rủi ro và kiên nhẫn. Đây là đầu vào mục tiêu và hành vi, không là phần thưởng/phạt đạo đức dành cho người chơi. Một người INT cao có thể mưu lợi ngắn hạn vì hoàn cảnh; người INT thấp vẫn có thể giữ lời và làm tốt công việc quen thuộc.

Quan hệ là từng cặp: tin cậy, ân nghĩa, xung đột, ràng buộc và lịch sử. Không dùng stat trung thành cố định. Trait nội dung giới hạn hai mỗi người, ví dụ “Lão luyện thủy lợi”, “Có mạng lưới thương hội”, “Trọng danh dự”; trait chỉnh khả năng/mục tiêu cụ thể và đi vào quan sát của policy khi nhân vật đó biết.

### 20.3 Ai được biết chỉ số?

Vua thấy chính xác chỉ số của bản thân và các đánh giá nghề nghiệp đã kiểm chứng. Nhân vật mới thường có khoảng ước tính kèm nguồn; làm việc, thẩm định và điều tra thu hẹp khoảng. Không cần tình báo để biết một vị quan đã quản lý tốt nhiều công trình công khai.

Stat trí lược hoặc ngoại giao cao không tiết lộ đối địch. Tham vọng, âm mưu và quan hệ kín không hiện chính xác khi chưa có bằng chứng. Giao diện chân dung tách “năng lực đã biết” và “nhận định tính cách”, luôn có ngày đánh giá.

### 20.4 Kinh nghiệm và nhiệm vụ

Nhân vật tích lũy XP theo công việc thực chất hoàn thành, độ khó và mức tham gia. Một nhiệm vụ không phát XP lặp lại khi mở/đóng hoặc đổi người phụ trách. Mỗi stat tăng tối đa 3 điểm/năm từ kinh nghiệm; có lợi suất giảm, đánh giá cân bằng sau chơi thử. INT chỉ tăng tối đa 1 điểm/năm từ dự án học tập dài hạn; không tăng vì spam thương lượng.

Chuyên môn nghề mở đặc tính theo mốc 40/60/80, ví dụ xử lý thêm một hồ sơ hoặc tổ chức khảo sát chi tiết; không biến số cao thành quyền pháp lý mới. Sức khỏe và stress có modifier tạm thời, UI tách chỉ số nền/hiệu dụng. Không tăng XP cho policy model: XP thay đổi input và khả năng trong môi trường, không huấn luyện lại mạng ngay trong save.

Mức năng lực bộ máy phụ thuộc người chủ trì, nhân sự và tải. Dùng hệ số chuyên môn thử đầu `0,75 + 0,005 × stat`, miền 0,75–1,25, cho tốc độ phần công việc hành chính tương ứng; không nhân toàn bộ sản lượng vùng. DIP và INTG dùng trong thời gian/chất lượng quy trình, không cộng thẳng vào xác suất “đồng ý” hoặc “phát hiện mọi âm mưu”.

### 20.5 Ví dụ hai lãnh chúa

**Lãnh chúa Hắc Sơn:** INT 78, ADM 48, ECO 62, DIP 35, INTG 67, MIL 55; tham vọng cao, kiên nhẫn cao. Có thể chọn liên minh có lợi lâu dài nhưng cần quan giỏi hành chính thực hiện, đồng thời dễ gây căng thẳng khi tiếp kiến.

**Lãnh chúa Thanh Điền:** INT 56, ADM 82, ECO 66, DIP 74, INTG 32, MIL 28; trọng cam kết và quan tâm địa phương. Có thể điều hành cứu đói hiệu quả nhưng cần người điều tra khi đối thủ vận động kín. Đây là hành vi mong muốn để đánh giá policy, không kịch bản bắt buộc cho mọi tình huống.

## 21. Sự kiện phát sinh và tương tác đa đối tượng

### 21.1 Nguồn phát sinh

| Nguồn | Cơ chế | Ví dụ |
|---|---|---|
| Quá trình thế giới | Thời tiết theo lưu vực, chu kỳ mùa, dịch theo tiếp xúc, tuổi/sức khỏe | Thiếu nước hạ lưu, dịch đi theo tuyến thương mại |
| Hành động learned policy | Đề nghị, đầu tư, chuyển việc, vận động, rút hợp tác, quyết định logistics | Thương nhân đổi nơi mua; lãnh chúa tìm liên minh |
| Hệ quả kinh tế/thể chế | Thiếu đầu vào, nghĩa vụ đến hạn, congestion, tranh quyền hoặc mất khả năng thi hành | Xưởng ngừng, nợ quá hạn, dự án đình trệ |
| Tương tác cá thể | Gia đình, cơ hội học/nghề, cùng tổ chức, tiếp xúc và migration | Một nhóm thợ rời xưởng, người mới tham gia phe |

Không có deck khủng hoảng rút theo thời gian để bù đủ quota. Template chỉ trình bày facts hoặc khung hồ sơ, không tự tạo người, quân, tiền hay động cơ chưa có. Tutorial có tình huống đặt trước riêng để dạy, được gắn nhãn và không nằm trong campaign emergent.

### 21.2 Exogenous process có ngẫu nhiên nhưng liên kết địa lí

WeatherProcess dùng trường khí hậu/mùa và nhiễu có tương quan theo không gian/thời gian; không roll hạn/lũ độc lập hoàn toàn từng ngày từng vùng. HydrologyProcess cập nhật ảnh hưởng mưa, hồ/đê/thủy lợi và lưu vực ở mức đơn giản; không cần CFD. DiseaseProcess dùng exposure, môi trường, di chuyển, y tế và incubation.

Khi dùng hazard, xác suất theo bước thời gian có thể tính $p=1-\exp(-\lambda\Delta\tau)$, lambda phụ thuộc trạng thái hợp lệ. Cần hiệu chuẩn và test theo cadence; không lấy “mỗi tháng 20% crisis” làm mô hình cho mọi thiên tai. Ngẫu nhiên có seed giúp tái hiện, không tạo lịch tình tiết bắt buộc. Từ cùng seed, hành động khác phải thay đổi diễn tiến.

Nhà vua bệnh nghiêm trọng có dấu hiệu/diễn tiến và lựa chọn can thiệp. Luật không phát một event chết tức thì chỉ để làm lượt chơi khó; sinh–tử của dân cư vẫn có xác suất theo điều kiện và tuổi trong world process.

### 21.3 Facts trước, thông báo sau

1. World/Action tạo thay đổi thực và CausalEvent có ID, participants, location, tick/day và parent causes.
2. Observer cập nhật vụ việc, obligation, tổn thất hoặc nhu cầu điều tra dựa trên thay đổi đó.
3. Knowledge propagation xác định ai biết khi nào; cùng một vụ không mặc định hiện ngay trên HUD vua.
4. Presentation composer chọn cách diễn đạt từ facts đã biết; cho xem hồ sơ và hành động hiện đang khả dụng.
5. Người chơi hoặc NPC tiếp tục tương tác; hậu quả tạo facts mới, không “nhận option xong là đóng câu chuyện” khi nghĩa vụ còn tồn tại.

Một hồ sơ biết chủ đề nhưng chưa biết thủ phạm phải hiển thị như vậy. Không dùng logits của NPC hay world secret làm lời tường thuật chắc chắn.

### 21.4 Ví dụ chuỗi phát sinh, không phải script campaign

Mưa ít tại lưu vực → thu hoạch giảm → giá thực phẩm tại Hắc Sơn tăng → một thương nhân chọn chuyển hàng từ vùng khác → tuyến đèo đầy → xưởng thiếu nhiên liệu và không trả đủ lương → vài người lao động chọn tìm việc → chủ xưởng xin vay → chủ nợ yêu cầu bảo đảm → lãnh chúa thương lượng quyền thu phí → một nhóm bị mất lợi ích vận động phản đối.

Chuỗi này là **kịch bản kiểm thử mong đợi có thể xuất hiện**, không đoạn diễn tiến phải chạy mỗi lượt. Có kho dự trữ, tuyến khác, policy ít tham vọng hoặc quyết định vua can thiệp sớm thì chuỗi có thể dừng hoặc rẽ. Test tạo điều kiện và kiểm tra khả năng/hạch toán; test stochastic đo phân bố trên nhiều seed, không ép policy phải chọn đúng một câu chuyện.

### 21.5 Vai trò của thường dân và trẻ em

- Mỗi cá thể có cơ hội chọn các ý định như học, tìm việc, giúp gia đình, mở rộng quan hệ, tham gia cộng đồng hoặc đề nghị di cư trong khả năng hiện có.
- Quyết định có duration và cost; đổi nghề cần đào tạo, xin trường cần chỗ, di cư cần chỗ ở/đi lại. Không dịch chuyển dân số ngay khi một mẫu action xuất hiện.
- Các cá thể có thể chọn khác dù chung vùng; trait, household constraints, lịch sử và sampling riêng tạo khác biệt.
- Nhiều lựa chọn nhỏ được thực hiện theo batch nhưng hậu quả phân lại đúng người. Việc tăng tổng nhu cầu, thiếu nhân lực hoặc tham gia tổ chức làm thay đổi cơ hội của các bên khác.
- Người chơi không xử lý hàng nghìn đơn xin học. Nhà trường, chủ xưởng và gia đình tự điều phối; chỉ vấn đề vượt quyền hoặc ảnh hưởng nghĩa vụ mới lên triều đình.

Không cần mỗi người có cuộc hội thoại sinh bằng LLM hoặc animation riêng để có agency. Tên, hồ sơ và timeline cá nhân được inspect khi cần; cơ chế cho họ tự chọn phải tồn tại dù chưa ai mở hồ sơ.

### 21.6 Đa dạng và kiểm chứng

Đa dạng được đo bằng họ nguyên nhân, ngành/vùng/actor liên quan, chuỗi hậu quả, hình thức thương lượng và kết quả; không chỉ đếm số đoạn văn. Theo dõi action entropy trong bối cảnh phù hợp, mức tập trung chính sách, tỉ lệ hold và tương tác liên nhóm; entropy cao nhưng vô nghĩa không phải chất lượng.

Các thử nghiệm bắt buộc:

- Không can thiệp: các bên vẫn giao dịch và xử lý việc thường lệ, không chờ vua/UI.
- Cùng world/shock, vua chọn khác: dòng hàng/quyền lực/kết quả khác theo causal chain.
- Cùng objective nhưng tuổi/nguồn lực khác: Action phù hợp khả năng; key character đưa quyết định rõ.
- Cùng state, nhiều sampling seed: có biến thiên hợp lí; chưa cần tất cả hành vi đều khác.
- Đóng tuyến/đổi phí: tác động lan qua inventory, hợp đồng, lao động và quan hệ; không chỉ trừ điểm một vùng.
- Không có yêu cầu “mỗi năm phải đảo chính”; nếu không ai tổ chức và không đủ lực lượng, không có coup tự sinh.
- Kiểm tra propagation dài có backpressure để không tràn queue hoặc gộp mất khoản chi.

### 21.7 Causal graph và giảm tải thông báo

Causal graph là graph thưa của facts có liên quan, không một ma trận mọi event × mọi event. Giữ chain/evidence đang cần xử lí; tổng hợp facts thường lệ thành báo cáo ngày/tháng và giữ provenance của tổng hợp.

UI mặc định tối đa năm việc nổi bật, filter theo vùng/chủ đề và bookmark. Auto-pause chỉ áp cho mức độ người chơi chọn. Việc không hiện trên màn hình vẫn được mô phỏng/ghi vào hồ sơ thích hợp; backpressure không được xóa payment, bỏ Action hay làm cohort mất quyền sở hữu.
