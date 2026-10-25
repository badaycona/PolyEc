# PolyEc — kế hoạch toàn dự án

Cập nhật đặc tả: 2026-10-22. Đây là nguồn chuẩn về đích sản phẩm, phạm vi và thứ tự triển khai. Đây không phải chứng nhận tính năng đã chạy. Trạng thái và nhóm đang làm nằm trong [CURRENT_TASK](CURRENT_TASK.md); phương pháp kỹ thuật nằm trong [DEVELOPER_GUIDE](DEVELOPER_GUIDE.md).

## 1. Bài toán sản phẩm

### Cách đọc kế hoạch đầy đủ

Tài liệu này trả lời **game cần đạt gì và theo thứ tự nào**. Cách chơi đọc [PLAYER_GUIDE](PLAYER_GUIDE.md); cách phân rã và giải bài toán đọc [TECHNICAL_PROBLEMS](TECHNICAL_PROBLEMS.md) rồi [DEVELOPER_GUIDE](DEVELOPER_GUIDE.md); cách giao việc đọc [DEVELOPMENT_WORKFLOW](DEVELOPMENT_WORKFLOW.md). Một yêu cầu trong plan là đích cần đạt, không phải tính năng đã nghiệm thu.

Phần I (1–10) xác định luật nền, kiến trúc, roadmap và điều kiện kiểm chứng. Phần II (11–21) mô tả nội dung sản phẩm. Phần III (22–30) phân rã yêu cầu thành dữ liệu, quyền, phép chuyển đổi, lưu/tải, tích hợp và phép kiểm đủ cụ thể để triển khai. Các plan nhỏ và report được dẫn là nguồn truy nguyên quyết định hoặc kết quả theo phiên bản; chúng không tạo roadmap song song.

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
| Schema, phiên bản, cấu hình, định nghĩa trước danh mục nội dung và quyền sở hữu dữ liệu | 22 |
| Cá thể, năng lực, tính cách, cơ thể và tri thức | 23 |
| Hộ, đất, lao động, sản xuất, thị trường và vận tải | 24–26 |
| Tổ chức, di sản, quốc gia, quân sự và ngoại thương | 27 |
| Bộ dữ liệu RL, chất lượng policy và model tương thích | 28 |
| Tích hợp, hiệu năng, đóng gói và điều kiện đóng M5 | 29–30 |

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

## 3. Hành vi và hoạt động của NPC — agent-centric

Mỗi NPC là một agent có identity bền, memory/knowledge, presence/attention, capability và policy. Metadata khởi tạo gồm tuổi, hộ và endowment; tên có thể sinh khi cần hiển thị. Knowledge phân biệt facts đã xác nhận, estimates, rumours và secret evidence, có nguồn, thời điểm và độ tin cậy; PlayerKnowledge chỉ cho thấy phần người chơi được biết.

Capability vector mô tả các năng lực chức năng, chẳng hạn hành chính, kỹ thuật, lao động, ngoại giao và quân sự. Không ép mọi cá thể vào đúng sáu stat hoặc lấy một stat làm nguồn duy nhất cho hành vi. Nhóm capability có thể cấu hình theo scenario/experiment nhưng phải có schema, đơn vị, tác động và kiểm chứng tương ứng. Các tên INT/ADM/ECO/DIP/INTG/MIL trong nội dung kế thừa phía dưới là ví dụ hoặc nhãn tổng hợp; không là schema bắt buộc của thiết kế mới.

Learned RL policy chọn hành động từ observation hợp lệ, memory, capability và mục tiêu của cá nhân. Survival, wealth, reputation và mục tiêu vai trò có thể xung đột; policy lựa chọn trong luật. Heuristic/rule-based chỉ là baseline dev/test có nhãn. Không dùng chúng thay learned policy trong sản phẩm hoàn chỉnh. XP là cơ chế tùy chọn: nếu dùng phải định nghĩa nguồn tiến bộ, tác động capability và settlement đúng một lần, không bắt buộc tiến bộ phải đi qua điểm XP.

### M5.0/1 — provenance của knowledge từ investigation, working rule T4.3 (2026-10-22)

Chỉ luồng đã có căn cứ mới được ghi nguồn: khi Investigation hoàn tất, Knowledge của một secret Fact lưu thêm ID Investigation đã cấp thông tin và ngày game nhận được. ID nguồn `0` nghĩa là save cũ không có đủ dữ liệu để truy nguyên; không đoán lại Investigation từ vùng/ngày gần nhau. Save mới dùng Realm schema31 và section `KNOWLEDGE_2`; reader schema7–30 vẫn đọc ba trường cũ và gán nguồn unknown. Save31 chỉ nhận source dương nếu Investigation tồn tại, đã hoàn tất và thuộc cùng observer; Fact tham chiếu phải là secret ở đúng region, không muộn hơn hạn cuộc điều tra, và ngày nhận không trước hạn/không sau ngày Realm. Lý do là source code hiện chỉ tạo Knowledge tại pha hoàn thành Investigation, nên ID này là provenance có thể kiểm được mà không bịa nguồn hoặc confidence. Lát này không phân loại verified/estimate/rumour, không gán confidence, không đổi điều ai biết. Chi tiết/kiểm thử: [plan T4.3](design/M5-T4.3-investigation-knowledge-provenance-plan.md), [report T4.3](reports/M5-T4.3-investigation-knowledge-provenance-evidence-2026-10-22.md).

T4.4 thêm khóa có tên `legacy_capabilities` trong Python/GDExtension và dùng sáu nhãn tiếng Việt trong hồ sơ Godot, vẫn giữ mảng `stats` cùng thứ tự cũ. Luật, save schema31, observation slot10–15, action/reward/model contract và người xem không đổi. Release Realm 18.913, Python mapping 2 checks, Godot display smoke 1/1; GDExtension/Python targets build thành công. Full snapshot→UI smoke còn chờ bundle model realm-v11 tương thích. Đây chỉ là cầu nối tên dữ liệu, không phải registry cấu hình hoặc hoàn thành capability schema. [Plan T4.4](design/M5-T4.4-named-capability-view-plan.md), [report T4.4](reports/M5-T4.4-named-capability-view-evidence-2026-10-22.md).

T4.5 định nghĩa catalog hiện có bằng sáu ID ổn định, điểm nguyên `[0,100]` chưa hiệu chuẩn và tập ID hoạt động có thể chọn theo constructor/scenario C++/Python. Schema mặc định đủ sáu giữ nguyên hành vi, observation và model contract. Scenario bỏ chiều nào thì luật và slot observation tương ứng dùng trung tính 50/0,5; model contract có hậu tố mask riêng để runtime từ chối checkpoint khác schema. Realm schema32 lưu `CAPABILITY_SCHEMA_1`; reader schema7–31 migrate về đủ sáu, giữ nguyên dữ liệu Person. Mask rỗng/ID chưa đăng ký/section thiếu/giá trị ngoài miền bị reject; load lỗi không thay world đang chạy. T4.5 chỉ cho chọn tập con của sáu ID, chưa đăng ký năng lực kỹ thuật/lao động hoặc schema khác nhau giữa người. C++ Release Realm 18.928, court 565, schedule 3.719, transport 91; Python binding 7 checks; runtime guard 1/1; Godot guard 5/5 và profile 1/1. [Plan T4.5](design/M5-T4.5-configurable-legacy-capability-schema-plan.md), [report T4.5](reports/M5-T4.5-configurable-legacy-capability-schema-evidence-2026-10-22.md).

T4.6 phân biệt `SecretEvidence` do Investigation cấp với `UnknownLegacy` từ save không đủ dữ liệu. Investigation mới lưu source ID, loại bằng chứng và confidence `-1` (chưa biết); không gán `Verified` hay phần trăm độ đúng khi chưa có kết quả độc lập để hiệu chuẩn. Realm schema33 ghi `KNOWLEDGE_3`; schema31–32 có source dương migrate thành `SecretEvidence`, schema7–30 migrate thành `UnknownLegacy`. Loại, source và confidence không hợp lệ bị từ chối; tải lỗi giữ nguyên Realm đang chạy. C++ Release Realm 18.951, court 565, schedule 3.719, transport 91. UI/PlayerKnowledge, Estimate/Rumour/Verified, confidence đã đo và RL vẫn mở. [Plan T4.6](design/M5-T4.6-investigation-evidence-knowledge-type-plan.md), [report T4.6](reports/M5-T4.6-investigation-evidence-knowledge-type-evidence-2026-10-22.md).

T4.7 thêm `RealmView.knowledge`, chỉ gồm Knowledge explicit của observer khi Fact tương ứng vẫn hiển thị cho observer. Python và GDExtension cùng xuất observer, Fact, ngày nhận, Investigation nguồn, loại và confidence; Godot nối nhãn “Bằng chứng điều tra” hoặc “save cũ chưa rõ loại và nguồn” vào Fact tương ứng. Không thay quyền xem, observation/action/reward/model hay save. Release Realm 18.954, View Projector 6; Python view smoke đạt; GDExtension build đạt; Godot formatter smoke 2/2. `godot_project/models/realm-v11` thiếu model nên chưa có full session-backed snapshot smoke. [Plan T4.7](design/M5-T4.7-player-knowledge-view-plan.md), [report T4.7](reports/M5-T4.7-player-knowledge-view-evidence-2026-10-22.md).

T4.8 đưa `Person::skill` vào catalog với ID số6/key `labor_skill`, đơn vị điểm kỹ năng lao động thiết kế `[0,100]`. ID đọc chính trường skill từng Person vốn đã quyết định đủ điều kiện việc làm, sản lượng/tiền công hiện hành, tăng từ học/dạy và observation slot4. Không nhân đôi dữ liệu. ID này luôn thuộc schema; mask scenario vẫn chọn sáu ID legacy và neutral 50 khi legacy ID bị tắt. Python/GDExtension thêm `capabilities` và `capability_schema`, còn field/array cũ được giữ. Reader và Realm invariant từ chối skill ngoài miền; save layout, observation/model/action/reward và gameplay không đổi. Release Realm 18.963, manager pass, Python mapping smoke pass, Godot label 3/3, GDExtension/Python targets build. TechnicalSkill chưa có nguồn dữ liệu hay luật Person để sử dụng. [Plan T4.8](design/M5-T4.8-person-labor-capability-catalog-plan.md), [report T4.8](reports/M5-T4.8-person-labor-capability-catalog-evidence-2026-10-22.md).

T4.9 đưa mô tả bảy capability ra một catalog C++ duy nhất và xuất cùng metadata sang Python `CAPABILITY_CATALOG` và GDExtension `capability_catalog()`: ID, key, đơn vị, miền, chủ thể sở hữu, khả năng scenario bật/tắt, luôn hoạt động và giá trị thay thế. Sáu chiều legacy tiếp tục dùng neutral 50; `labor_skill` luôn hoạt động và không có giá trị tắt. Realm Release 18.978; Python kiểm đủ bảy mục và Godot headless catalog/nhãn đạt 5/5; cả hai binding build thành công. Lượt Python build đầu thiếu include `realm.hpp`, đã sửa trước lượt đạt. Không đổi luật, save hay model và không train RL. [Plan T4.9](design/M5-T4.9-capability-catalog-metadata-plan.md), [report T4.9](reports/M5-T4.9-capability-catalog-metadata-evidence-2026-10-22.md).

T5.2 thử thay bảng tổng attention theo actor trong pha cập nhật cơ thể bằng vector theo ID ở workload 100.000 người/512 vai/31 ngày. Realm 18.978 checks pass ở bản thử và sau khi khôi phục. Sáu lượt vector ghi 89–107 ms cho pha body; ba lượt map cũ ghi 84–97 ms. Đồng hồ tổng ngày còn dao động lớn và binary baseline cũ không còn để A/B trực tiếp; không kết luận tăng tốc. Đã gỡ thay đổi source, chỉ giữ report và chín JSON để truy nguyên, nên gameplay/API/save không đổi. [Plan T5.2](design/M5-T5.2-body-phase-sparse-lookup-plan.md), [report T5.2](reports/M5-T5.2-body-phase-sparse-lookup-evidence-2026-10-22.md).

T5.3 thêm năm bộ đếm chỉ trong build profile, tách pha xử lý cư dân thành vòng cập nhật từng người, quyết toán chợ, quét tử vong, ghi hồ sơ tử vong và phân phối di sản/chỉ mục. Hai run Release 100.000 người/512 vai/31 ngày giữ tiền và invariant. Quyết toán chợ là phần lớn nhất: 168–187 ms tích lũy 31 ngày; cập nhật người 97–102 ms; quét tử vong 31–33 ms. Không đổi gameplay hay adapter. [Plan T5.3](design/M5-T5.3-daily-people-phase-diagnostics-plan.md), [report T5.3](reports/M5-T5.3-daily-people-phase-diagnostics-evidence-2026-10-22.md).

T5.4 giữ quét lệnh đến hạn khi `settle_market_day()` được gọi lại trong cùng ngày, nhưng bỏ lượt `any_of` trước vòng quyết toán khi đang sang ngày mới. Không đổi cách phân phối hay hạch toán. Market order **35 checks**, Realm **18.978 checks**; hai benchmark 100.000/512/31 ngày giữ invariant và tiền. Bộ đo thời gian còn dao động và chưa cho thấy tăng tốc rõ, vì vậy report chỉ khẳng định một lượt quét toàn danh sách đã được loại bỏ theo cấu trúc ngày mới. [Plan T5.4](design/M5-T5.4-skip-redundant-market-due-scan-plan.md), [report T5.4](reports/M5-T5.4-market-due-scan-evidence-2026-10-22.md).

T5.5 gỡ vòng quét toàn bộ đơn chợ lồng trong mỗi lần cấp hàng thường. Realm buộc đơn có `settle_day=placed_day+1` và tăng ngày trước settlement, nên đơn ordinary còn mở từ trước đều đến hạn; không có đơn ordinary tương lai giữ slot trong cùng settlement hợp lệ. Command đã giới hạn tổng order pending của người mua ở 16 slot, và settlement vẫn kiểm inventory thực tế cùng fill. Market order **40 checks** (hai lệnh 9+7 nhận đúng 16 món), Realm **18.978 checks**; hai benchmark 100.000/512/31 ngày giữ tiền/invariant nhưng không đo hiệu quả ordinary, nên không tuyên bố tăng tốc tổng thể. [Plan T5.5](design/M5-T5.5-market-open-order-capacity-index-plan.md), [report T5.5](reports/M5-T5.5-market-capacity-scan-evidence-2026-10-22.md).

T5.6 bổ sung bốn biểu đồ lấy nguyên dữ liệu run RL cũ: return huấn luyện của sáu seed v30, utility validation ghép cặp, nợ/mất khả năng thanh toán/số người sống so với baseline, và reward/return của v31 Civilian seed1 update 26–43. Script kiểm phạm vi update, quyết định và 20 validation world/family; JSON, PNG và chín nguồn có SHA-256 trong manifest. Sáu candidate v30 vẫn bị loại; v31 chỉ có 44.032 quyết định, chưa validation. Không chạy lại RL. [Plan T5.6](design/M5-T5.6-rl-run-visual-evidence-plan.md), [report đầy đủ](reports/M5-T5.6-rl-run-visual-evidence-2026-10-22.md).

Đánh giá agency bằng quyết định/ngày, tỷ lệ thực hiện cam kết, attention không sử dụng, kết quả hợp đồng và paired interventions; các metric này bổ sung quality gates phần 8, không thay chúng. UI có thể tổng hợp năng lực thành nhãn dễ hiểu nhưng view không trở thành nguồn state để engine ghi ngược.

Một người có thể làm, học, nghỉ và chăm người khác trong cùng ngày. Tách quan hệ việc làm dài hạn, lịch mong muốn, phiên thực hiện và lượt policy suy luận. Command không liên quan không tự xóa job.

Được nhận cam kết quá tải; world chỉ thực hiện phần thời gian, hiện diện, attention và nguồn lực thực sự có. Phần thiếu có nguyên nhân và hệ quả đã định nghĩa. Không chặn lựa chọn chỉ vì dự báo kém khôn ngoan hoặc không tối ưu. Validation bảo vệ cấu trúc, quyền, consent và bảo toàn; không trở thành người chọn chiến lược hộ NPC.

Attention là tài nguyên chú ý khi thực hiện, khác thiếu ngủ, mệt do gắng sức, sức khỏe và khuynh hướng kiên trì. Attention-time không tự là lương, tiến độ học, dịch vụ chăm sóc hay hồi phục. Tương thích phụ thuộc loại/context; không mặc định mọi hoạt động exclusive hoặc mọi việc ít attention làm được ở hai nơi.

### 3.1 Thời gian làm việc thực tế — contract T2.1 (quyết định làm việc do Codex giao)

Đây là quyết định làm việc được giao cho OpenCode thi hành trong gate T2.1; đây **không phải trích dẫn trực tiếp của người dùng** và không tự thay §2 hay duyệt thêm hệ số cân bằng. Contract này là nguồn chuẩn cho T2.1 cho tới khi Establishment/worksite của M5 xuất hiện.

- Chỉ **Work interval được gõ kiểu rõ ràng** (explicit typed Work interval) mới được tính vào attendance. Khoảng trống không lên lịch (unscheduled gap) và việc chỉ ở trạng thái Employed đều tính **zero**.
- Không suy diễn giờ/ngày cố định (không 8/16 giờ).
- Presence yêu cầu đồng thời: người **còn sống**, **`destination == -1`** (không đang travel/migrate), và **region Realm hiện tại bằng region làm việc tạm ghi trong session**. Đây là so sánh chặt bằng `-1`: destination âm khác `-1` là trạng thái không hợp lệ, không được coi là không travel và không được cộng attendance.
- Overlap của các Work interval vẫn được nhận; attendance **vật lý** được đếm **một lần cho mỗi actor/tick** bằng hợp (union) các interval, không phải tổng. Attendance là khái niệm khác với delivered attention, effort, output và pay.
- Region Realm là **địa điểm lao động tạm** cho tới khi M5 có Establishment/worksite.
- Hoãn (deferred, không làm trong T2.1): quy đổi work-unit, wage, production recipe/output, worksite ID, và tính idempotence của settlement.

#### T2.2 — work settlement bookkeeping (accepted 2026-10-20)

Đây là quyết định làm việc do Codex giao trong gate T2.2; **không phải trích dẫn trực tiếp của người dùng** và không tự duyệt hệ số cân bằng hay economic settlement.

- Hệ số settlement $$T = 16$$ attendance tick cho một cumulative unit là **working assumption do Codex giao**, không phải chỉ đạo trực tiếp hoặc phê duyệt balance. Với 64 tick/ngày, full attendance tối đa là $$64/16 = 4$$ unit/ngày.
- Settlement chạy ở **mỗi day boundary hoàn tất**, sau các phase trong ngày, từ `work_present_ticks` tích lũy duy nhất theo actor. Unit eligible của actor là floor của tổng tick tích lũy chia T. Phần dư tick được giữ tự động vì phép chia lặp lại trên cùng bộ đếm tích lũy, không reset và không backfill. Không double count: chỉ ghi khi eligible lớn hơn unit đã settle trước đó. Watermark `settled_day = floor(cursor/64)`.
- Unit settlement **chỉ là bookkeeping**, không phải effort, output, pay, wage, attention, production hoặc delivered service; settlement không sửa người, tiền, kho, payroll hay RNG.
- Save `POLYEC_REALM_9`/`SCHEDULE_3` thêm `settled_day` cùng map `settled_work_units` (sort theo actor ID). Migration: `POLYEC_REALM_8`/`SCHEDULE_2` giữ nguyên typed session và attendance nhưng settlement bắt đầu rỗng; `POLYEC_REALM_7`/`SCHEDULE_1` mặc định session Generic/`-1`, attendance và settlement rỗng.
- Giới hạn: chưa có production, payroll, worksite/Establishment hoặc `Intent::Work` integration; không phải economic settlement. Contract chi tiết: [M5-T2.2 work settlement](design/M5-T2.2-work-settlement-plan.md); evidence: [M5-T2.2 evidence](reports/M5-T2.2-work-settlement-evidence-2026-10-20.md).

#### T2.4 — Establishment, worksite và sản xuất (working contract delegated by Codex 2026-10-20)

Theo chỉ đạo người dùng tiếp tục M5 không hỏi lại, Codex chốt các working rule dưới đây để OpenCode triển khai theo từng lát có plan/review; các con số vẫn là giả định chưa hiệu chỉnh balance.

- Realm sở hữu Establishment có ID ổn định từ `next_id_`, vùng vận hành, owner reference, industry ID, capacity, cash và kho riêng. Owner ban đầu là Person còn sống (tạo cho chính mình), public authority của Province, hoặc organization account hiện có 1..28 (actor là member); organization site cùng region sở hữu `floor((owner_id-1)/4)`. Public creation đòi Governance + `taxing_authority` + charter + cùng region. Person IDs giữ positional namespace dense riêng; Establishment IDs chia `next_id_` với Loan/Project/Investigation/Proposal IDs, không đòi khác số Person ID và không sửa birth allocator. Các organization account đã được Realm hạch toán, nhưng chưa là organization entity đầy đủ. Tạo/fund phải chuyển ít nhất 1 cash từ chủ thể hợp lệ, atomic và không tạo tiền/hàng. Starter industry IDs: 1 cloth, 2 tools, 3 military supplies. `total_money()` phải tính cash mới; `balances()` append tổng Establishment cash ở slot mới, giữ nghĩa các slot cũ.
- Work interval kiểu `Work` tham chiếu establishment ID; vùng attendance lấy từ establishment. Generic không đổi. Tick trùng tại cùng establishment đếm một lần; Work interval chồng lấn khác establishment bị reject nguyên tử. Giữ $$T=16$$ tick/unit hiện tại. Một unit mới đủ điều kiện tạo tối đa một batch recipe; capacity là số batch nguyên/ngày, mặc định ban đầu 1 nếu scenario không đặt khác.
- Recipe dùng integer `GoodId` trên catalog hiện có **30 sản phẩm thuộc 16 nhóm**, không tự thay thế variant. Danh mục ban đầu có27 món; `atlas-specialty-products-v1` đã thêm ID27–29. Bộ recipe mở đầu: 1 Fiber → 1 Cloth; 1 Iron + 1 Fuel → 1 Tools; 1 Tools + 1 Cloth → 1 MilitarySupply. Mỗi recipe xuất một đơn vị mỗi batch; đây là working assumptions. Các nguồn legacy hiện tại chỉ áp dụng tại pilot; mọi sản phẩm còn thiếu cần hàng source/variant và producer/market contract rõ trước khi gọi là M5-complete. Mùa vụ và khai thác tài nguyên thiên nhiên không được ngầm giả lập trong slice craft; triển khai chúng cần resource/season state theo §15.3.
- Ở day boundary, batch tối đa là min của Work unit mới, capacity còn lại và đầu vào đủ cho batch hoàn chỉnh. Thiếu input làm mất cơ hội sản xuất ngày đó; không carry nợ labor sang ngày sau. Transaction stage/commit nguyên tử; watermark theo establishment/actor ngăn chạy lại sinh output lần hai. T2.3 wage/retry/forfeit behavior vẫn cần conservation khi nguồn trả chuyển sang account của employer.
- Establishment kho dùng 27 canonical product `GoodId` dưới 16 `GoodGroup`. Gate 3 đã mở rộng Person/Province/Campaign holdings và migrate bảy slot tường minh: food→DryFood, fiber→Fiber, cloth→Cloth, iron→Iron, fuel→Fuel, tools→Tools, horses→Horse; sản phẩm mới bắt đầu bằng zero. Save cũ không tự tạo Establishment hoặc retro-assign Work/output. Mọi save mới stage references/invariants rồi commit atomic.
- Command dành cho player và learned policy qua cùng Realm validation/transaction. Không gọi M5 tự chủ hoàn tất cho tới khi policy có establishment IDs/state hợp lệ trong observation/action mask và model compatibility được gate riêng. Khi semantics của feature thay đổi mà observation shape giữ nguyên, phải version `model_contract` và từ chối pilot model cũ ở runtime sản phẩm; chỉ benchmark/dev test có override tường minh, gắn nhãn không tương thích. Không sửa manifest model cũ để tự nhận tương thích. Godot runtime theo đường sản phẩm mặc định và do đó cũng phải từ chối bundle cũ.
- PPO trainer lấy `model_contract` từ cùng C++ Realm qua Python binding và ghi vào config/manifest; exporter chỉ xuất checkpoint đã lưu contract trùng core hiện hành. Runtime manifest gate chứng minh contract/schema/dimensions/lineage declaration, không thay R1–R3 quality gates. Model thiếu R1/R2/R3 vẫn là pilot, dù runtime có thể nạp contract để dev/evaluation.
- Chi tiết/từng gate: [T2.4 Establishment/production contract](design/M5-T2.4-establishment-production-contract-proposal.md). Gate đầu tiên là Establishment identity + funding/creation + invariant + versioned save/migration; OpenCode phải trình plan và Codex review trước code. Recipe catalog đầy đủ, payroll-account integration, bindings, observation/action và Godot sẽ được giao thành các lát kế tiếp theo CURRENT_TASK.

#### T3.3 — quy đổi chú ý được cấp thành công sức tại worksite, trial-1 (working assumption 2026-10-22)

P6.2 còn thiếu cầu nối giữa lượng chú ý ActivityEvaluator thực cấp và cơ hội tạo sản phẩm. Thử nghiệm này dùng đơn vị trừu tượng có thể kiểm lại: ActivityEvaluator có tối đa 1.000 đơn vị chú ý trên mỗi tick; 1.000 đơn vị chú ý được cấp trong lúc người làm có mặt tại đúng worksite tương đương 1 tick công sức; 16 tick công sức tạo 1 work unit, khớp working assumption `T=16` đã dùng cho recipe. Do đó một đơn vị recipe cần 16.000 đơn vị chú ý hợp lệ. Thời gian core là 64 tick/ngày; đây không phải giờ thật.

Chỉ `delivered` của Work session gắn Establishment, khi người còn sống, không di chuyển và đang ở cùng vùng worksite trong chính đoạn tick được xử lý, mới cộng vào công sức. Chú ý chưa được cấp, Generic session, Work ở nơi vắng mặt hoặc thời gian di chuyển không tạo công sức. Attendance vẫn ghi thời gian hiện diện duy nhất và tiếp tục làm đầu vào cho bookkeeping/lương theo contract cũ; nó không tự thành output. Mỗi tick công sức phải đối chiếu với tối đa 1.000 đơn vị chú ý/tick hiện diện. ProductionManager chỉ xem xét batch nguyên theo `floor(công sức chú ý tích lũy / 16.000)`, rồi áp dụng capacity và recipe inputs; thiếu đầu vào làm forfeiture qua watermark đã có, không giữ thêm nợ lao động. Hệ số 1.000/16 là trial có phiên bản, không phải hiệu chuẩn hay mô hình năng lực cá nhân.

Realm cộng dồn lượng chú ý hợp lệ theo `(actor, establishment)` khi evaluator xử lý từng đoạn, giữ trong ScheduleBook và save `SCHEDULE_6`/`POLYEC_REALM_29`. Khi nạp schema 7–28, khởi tạo baseline công sức bằng `processed_work_units × 16.000` của Establishment, để không tạo lại output lịch sử hoặc cộng ngược chú ý cũ. Các fixture schedule schema 1–5 và mọi Realm 7–28 tiếp tục đọc theo grammar cũ. Số action/observation, reward, model contract, quyền/consent, lương, recipe, hệ số capacity và learned policy không tự đổi trong lát này. Người có năng lực/chuyên môn khác nhau chưa được nhân hệ số ở đây; đó là phần P6.2 còn mở cần model/observation và phép kiểm riêng.

#### T3.4 — gắng sức và hồi phục từ công sức/nghỉ thực, trial-1 (working assumption 2026-10-22)

Để tiếp tục P6.3 trên các đơn vị đã có mà không tự nhận là mô hình sinh lý, trial-1 giả định mỗi 16.000 Work effort-attention mới đã lưu ở T3.3 tăng exertion một điểm; mỗi 16 tick Rest thực giảm một điểm. Exertion giữ miền 0–100; lưu riêng phần dư effort `[0,15999]` và phần dư Rest `[0,15]`. Khi chạm cận 0/100, xóa phần dư hướng đi vượt cận. Cập nhật ở ngày game hoàn tất, không dùng attendance, Generic attention, tiền lương hay sản lượng làm đại diện effort. Đây là phép quy đổi để kiểm tính nhất quán; chưa được hiệu chuẩn. Không dùng exertion để chặn hành động, thay output, đổi sức khỏe, model/action/observation/reward hoặc policy trong nhóm này.

Watermark body lưu tổng Work attention đã tính tới để không cộng trùng qua các ngày và save/load. BODY codec nâng lên BODY_3 cùng Realm schema30; khi nạp schema7–29 watermark được khởi tạo theo tổng effort-attention đã có sau migration T3.3, tránh biến công sức lịch sử thành mệt mới. Save giữa ngày phải tiếp tục đúng cả phần dư khi được nạp. Chi tiết source/ownership, case clamp/pause/migration và test nằm trong [plan T3.4](design/M5-T3.4-work-exertion-recovery-plan.md); kết quả chạy Release core và Python ở [report T3.4](reports/M5-T3.4-work-exertion-recovery-evidence-2026-10-22.md). Lát này cập nhật chỉ số exertion, không khẳng định care/lifecycle hoặc P6.3 hoàn tất.

#### T3.9 — skill cá nhân quy đổi công sức sản xuất, working assumption 2026-10-22

P6.2 dùng trial có phiên bản `m(s)=0,75+0,005s`, với `s=Person::skill` trong `[0,100]`; factor `[0,75;1,25]`. Cài bằng tử số nguyên `150+s`, mẫu `200`. Lượng attention thô tiếp tục quyết định exertion và presence; production đọc một bộ cộng dồn riêng `attention × (150+s)`, tính ở lúc attention được cấp, để thay đổi skill về sau không viết lại công sức lịch sử. Với skill50, quy đổi bằng 1,0 và hành vi cũ giữ nguyên; skill0/100 lần lượt tạo 0,75/1,25 attention sản xuất trên cùng attention hợp lệ. Đây là giả định thử lấy từ hệ số chuyên môn ở §20.2, áp dụng cho sản xuất cá nhân, chưa hiệu chuẩn và không chứng minh cân bằng. Save migration đặt công sức tích lũy cũ ở hệ số trung tính để không tạo thêm/thu hồi sản phẩm. Action, observation, reward, ngưỡng nhận việc, lương, recipe và exertion không đổi; vì kết quả sản xuất đã đổi, timeline model contract tăng v7→v8 và non-timeline giữ v6 để runtime từ chối policy cũ. [Plan T3.9](design/M5-T3.9-personal-work-skill-productivity-plan.md); chưa được nghiệm chứng qua run RL.

T3.10 giữ `health` trong miền 0–100 ở các kết quả ngày hoàn tất và save đã nạp: mọi đường không lấy được khẩu phần trừ 2 nhưng chặn tại 0; cá thể health≤0 chết như trước và được lưu health0; bộ đọc chuẩn hóa giá trị âm của save cũ khi world còn ở trạng thái tạm. C++ Realm Release **18.994 checks** pass, gồm có/không có chợ, ngưỡng health1/2/3, chết/di sản và nạp giá trị âm. Không đổi lượng thức ăn, thời điểm chết, save grammar/model hoặc policy; đây không hoàn thành mô hình dinh dưỡng/tuổi thọ hay P6.3. [Plan T3.10](design/M5-T3.10-starvation-health-bound-plan.md), [report](reports/M5-T3.10-starvation-health-bound-evidence-2026-10-22.md).

T3.11 không đổi hazard sau 75 năm (`1/3650` mỗi ngày) nhưng thêm Fact111 đúng ngày bước sang 75 năm để người chơi được cảnh báo, và Fact112 cho tử vong do tuổi; tử vong health0 vẫn là Fact100. Nếu có shipment đang đi, event vẫn chờ estate settlement và chọn loại từ health đã chốt, không thêm save field. Core Realm Release **19.001 checks** pass, gồm warning/save/load/no-repeat và nhánh tuổi tái hiện bằng RNG xác định. Thêm nhãn Godot 111/112, nhưng runtime Godot không cài nên parser/UI smoke còn chờ mốc tích hợp. Không đổi model, observation/action/reward hoặc hazard. [Plan T3.11](design/M5-T3.11-age-mortality-events-plan.md), [report](reports/M5-T3.11-age-mortality-events-evidence-2026-10-22.md).

T3.12 thay luật health `+15` cho mọi Rest đủ điều kiện bằng `floor(valid_rest_ticks/16)` điểm khi khoảng Rest kết thúc. Điều kiện cá thể còn sống/ở đúng vùng và nutrition≥40 giữ nguyên; cap health100, phần dư tick bị làm tròn xuống riêng mỗi khoảng. Hệ số 16 dùng cùng đơn vị trial T3.4 cho exertion, chưa được hiệu chuẩn. Vì health observation/transition đổi, timeline model contract v8→v9, non-timeline v6→v7; runtime từ chối model cũ. Không đổi save grammar, action, observation shape hoặc reward formula. Realm Release **19.011 checks**, native runtime **1/1**, Python clock **175/7 checkpoint**, model-contract16, config49/7 checkpoint pass. T3.11 Godot label smoke vẫn chờ runtime Godot. [Plan T3.12](design/M5-T3.12-rest-duration-health-recovery-plan.md), [report](reports/M5-T3.12-rest-duration-health-recovery-evidence-2026-10-22.md).

#### T3.13 — cá tính riêng của cá thể trong observation (trial-1, 2026-10-22)

Mỗi cá thể lưu sáu số nguyên 0–100 theo thứ tự tham vọng, lợi ích tài sản, quan tâm cộng đồng, trọng cam kết, chấp nhận rủi ro và kiên nhẫn. Thế giới mới và trẻ sơ sinh khởi tạo mỗi trục độc lập ở [25,75] bằng hàm hash 64-bit ổn định của Realm seed, person ID và trục; không tiêu thụ `Realm::rng_`, nên không đổi chuỗi ngẫu nhiên của luật khác. Đây là trial khởi tạo đa dạng có giới hạn, chưa hiệu chuẩn theo dân số hoặc vai trò và không làm thay luật quyền, tài sản, consent, reward hay quyết định policy.

Policy của người đang ra quyết định nhận sáu giá trị chia cho 100 tại state slots110–115. Đây là quan sát riêng theo actor, không xuất qua `PlayerView`, hồ sơ người chơi hay binding từ điển Person. Save nâng từ Realm34 lên Realm35 và thêm `PERSONALITY_1`, gồm sáu giá trị cho mỗi ID theo thứ tự ổn định. Save schema7–34 khởi tạo các trục thành trung tính50; miền ngoài [0,100] bị từ chối khi còn đang dựng bản nạp tạm. Observation shape vẫn 1.676 nhưng sáu slot trước đây trống nay có nghĩa mới, vì vậy model contract non-timeline v7→v8, timeline v9→v10; runtime từ chối contract cũ kể cả chế độ development override. Không train RL trong lát này; policy hiện có chưa học cách dùng trait. [Plan T3.13](design/M5-T3.13-personality-observation-plan.md), [evidence](reports/M5-T3.13-personality-observation-evidence-2026-10-22.md).

#### T3.14 — cho policy biết loại Knowledge riêng của actor

Knowledge lưu nguồn Investigation và phân biệt `SecretEvidence` với `UnknownLegacy`, nhưng state slot87 chỉ có số đếm tổng quát. Working assumption T3.14 dùng hai vị trí trống: slot88 = số dòng Knowledge loại `SecretEvidence` của actor hiện tại chia10; slot89 = số dòng `UnknownLegacy` của actor chia10. Đơn vị là một dòng Knowledge, không phải xác suất hoặc độ đúng; giá trị có thể vượt1. Không lộ dòng của observer khác, không đổi quyền nhận/xem Knowledge và không coi bằng chứng là sự thật đã xác nhận. Save layout không đổi. Vì hai slot đổi nghĩa, household contract v8→v9 và timeline v10→v11; runtime cần từ chối các contract cũ. Chưa có Estimate/Rumour/Verified hoặc confidence được hiệu chuẩn; chưa train RL. [Plan T3.14](design/M5-T3.14-agent-knowledge-observation-plan.md).

#### M5.2 — nối market-order C++ command vào Python trainer

Realm C++ đã có lệnh đặt/hủy mua có giữ tiền và các test sổ sách. Lát adapter Python chuyển nguyên operation/sequence/actor/region/good/quantity/maximum price vào `MarketOrderCommand`, trả kết quả và đọc lại lệnh mở/escrow từ cùng Realm. Python không tự giữ tiền hoặc tính fill/thuế. Phạm vi của adapter hiện tại chưa gồm native `RealmRuntime` hoặc màn hình Godot; các consumer đó có nhóm riêng sau khi binding parity pass. [Plan](design/M5-market-order-python-binding-plan.md).

#### T3.5 — đưa nợ ngủ và gắng sức vào hồ sơ người chơi, chỉ đọc

Hồ sơ native hiện đã cho xem sức khỏe/dinh dưỡng của cá thể được chọn nhưng chưa hiện hai chỉ số body vừa được Realm lưu. Nhóm này chỉ thêm `sleep_debt` và `exertion` vào snapshot GDExtension rồi ghi nhãn/giải thích bằng tiếng Việt ở hồ sơ Godot và hướng dẫn người chơi. Không thay luật gameplay, quyền xem/ra lệnh, action/observation/reward/model, dữ liệu save hay kết quả learned policy. Plan và phép kiểm tích hợp UI: [T3.5](design/M5-T3.5-body-status-player-profile-plan.md).

#### T3.6 — nguồn hàng có chứng cứ trong Realm đã bật timeline

Khi Realm đã bật lịch làm việc, không phát sản phẩm thụ động theo `region % 7`/nhãn việc làm và không chạy pilot Cloth/Tools cấp tỉnh ngày 30. Trong timeline world, hàng craft phải qua ProductionManager của Establishment và Work hợp lệ; hàng thô giữ nguồn harvest hiện có. Realm chưa bật timeline giữ nguyên hai luật legacy để không đổi replay của thế giới cũ. Unrest và các luật dân cư khác vẫn chạy. Đây là thay đổi nguồn hàng, không phải UI mới hay tuyên bố cân bằng kinh tế; thiếu hàng có thể khiến mua bán/tiêu dùng thất bại cho tới khi nguồn hợp lệ được tạo.

Observation shape giữ nguyên, nhưng policy kinh tế cũ nhìn thấy hàng tồn kho và hệ quả kinh tế khác. T3.6 vì vậy chuyển contract timeline v5→v6, và runtime từ chối policy v5. T3.7 bên dưới tiếp tục đổi ý nghĩa hai state slot để policy nhìn thấy cơ thể, nên contract timeline hiện v7 và non-timeline household hiện v6; runtime từ chối contract trước đó. Không sửa manifest/checkpoint lịch sử. Chưa có bundle realm-v11 tương thích nguồn hàng và body mới; train/evaluate learned policy vẫn là gate riêng. Luật và kiểm tra T3.6: [plan](design/M5-T3.6-timeline-production-source-plan.md), [report](reports/M5-T3.6-timeline-production-source-evidence-2026-10-22.md).

#### T3.7 — đưa body cá nhân vào observation của policy

State vector Realm hiện có 128 vị trí nhưng chỉ 108 vị trí đầu đang dùng. T3.7 gán slot 108 cho `sleep_debt/100` và slot 109 cho `exertion/100`; miền nguồn hai chỉ số là 0–100 nên policy nhận giá trị 0–1. Cả hai chỉ đọc BodyState của actor đang được hỏi; không cho thấy cơ thể cá thể khác hoặc bí mật. Tổng observation 1.676, 129 action, 12 feature/action và action IDs không đổi. Vì tensor giữ shape nhưng ý nghĩa input của model đổi, contract household non-timeline tăng v5→v6, còn timeline worksite tăng v6→v7, gồm hai biến thể có hậu tố `config`. Contract cũ bị runtime từ chối; không sửa checkpoint/manifest, không huấn luyện hoặc đánh giá trong lát này. Mục đích là cung cấp đúng trạng thái cá nhân cho training tương lai để policy có thể học chọn Work/Rest; chỉ có observation mới chưa chứng minh policy đã học hay thay đổi hành vi. [Plan T3.7](design/M5-T3.7-policy-body-observation-plan.md).

### M5.0/1 — working rule cho tài khoản quỹ hộ (Codex delegated 2026-10-21)

- Mỗi household label hiện có có một quỹ riêng, số dư khởi tạo 0. Membership không chuyển hoặc gộp tiền cá nhân. Sổ chỉ chứa số dư dương và được tính vào `total_money()`; `balances()` append tổng quỹ hộ ở slot 10, giữ nguyên slot 0–9.
- Thành viên còn sống và đủ 18 tuổi game time có thể nộp tiền cá nhân vào quỹ hộ của mình. Chỉ người đang là `household_authority` có thể rút quỹ về tài khoản cá nhân. Mọi lệnh dùng sequence/fact và Realm transaction atomic.
- Đây là nền account/transfer, chưa định nghĩa khoản chi nào dùng quỹ. Guardian không tự sở hữu hoặc rút tài sản cá nhân của dependent. Quyền chi cho học, ăn ở, đất, quản lý tài sản dependent, household dissolution/di sản vẫn cần gate riêng; không suy diễn từ tài khoản này.
- Save schema 20 thêm sparse `HOUSEHOLD_1`; schema 7–19 migrate với các quỹ rỗng. Python dùng cùng command và query C++.

### M5.0/1 — working rule cho land-title registry (Codex delegated 2026-10-21)

- Bước nền dùng holding/cụm đất trừu tượng gắn cố định với một region, diện tích nguyên dương, stable ID và một Person title-holder. Mỗi household khi genesis được cấp một holding area 1 cho member có ID household label; đây là endowment khai báo, không tạo tiền/hàng. Save cũ không được retroactively cấp đất.
- Title ownership tách khỏi quyền xét xử của Province và khỏi quyền sử dụng/canh tác, thu tô hay tenancy. Lát đầu chỉ registry + save/invariant/fingerprint, không transfer, định giá, địa tô, canh tác, map coordinates hay quyền guardian ký deed. Các cơ chế đó cần contract/command gate riêng. Diện tích không quy đổi sang km²/ha khi chưa có map parcel contract.
- Plan/evidence: [M5 land-title registry](design/M5-land-title-registry-plan.md). Đây là working assumption được ủy quyền để tiếp tục M5; không phải quyết định balance cuối.

### M5.0/1 — working rule cho holder-authorized title transfer (Codex delegated 2026-10-21)

- Một title-holder Person còn sống và đủ 18 game years có thể tự chuyển toàn title cho Person còn sống qua Realm command có sequence/fact. Recipient có thể là minor; không đổi money/area/region/title ID. Chuyển là không đối giá; không ngầm thêm giá bán, thuế, consent recipient, lien hoặc use-right.
- Guardian không sở hữu thay dependent và không ký deed thay dependent. Minor đang giữ title không tự chuyển được cho tới khi trưởng thành. Người giữ title chết không tự chuyển trong rule này; inheritance/estate là gate riêng.
- Save schema21 giữ `LAND_1`; command không đổi action/observation/reward/model. Evidence: [transfer plan/report](design/M5-land-title-transfer-plan.md) / [transfer evidence](reports/M5-land-title-transfer-evidence-2026-10-21.md). Self-review; chưa có reviewer độc lập.

### M5.0/1 — quyền sử dụng holding theo người dùng (schema22, 2026-10-21)

- Đơn vị là toàn bộ holding/cụm trừu tượng neo vào một region hiện có; không suy ra vị trí parcel, bản đồ, quy đổi diện tích hay quyền Province từ dữ liệu này.
- Quyền sử dụng là record riêng gắn `holding_id` và một người dùng. `LANDUSE_1` nằm sau `LAND_1`, schema22; schema7–21 migrate sang danh sách rỗng. Right không đổi title-holder, area, region, kho, tiền, action, observation, reward hay model contract. Adult holder đang sở hữu có thể revoke; transfer bị chặn trong khi right còn active. Xem [plan/evidence](design/M5-land-use-authorization-plan.md) và [report](reports/M5-land-use-authorization-evidence-2026-10-21.md).
- Đây mới là authorization không thu tiền; chưa có tenancy, rent, yield, đồng sở hữu, chia holding, lien, succession/estate hoặc guardian administration. Các bước đó cần contract riêng theo ownership/lifecycle trước khi triển khai.

### Gameplay configuration snapshot

Gameplay tuning values are a versioned immutable snapshot shared by the C++ core and its callers. The first loader slice preserves current defaults (16 game ticks per work unit, 1 money unit of work wage, Establishment capacity 1 batch/day, and unit input/output quantities for the three starter recipes). It exposes only integer values with explicit units and bounded ranges; ownership, recipe identity, transaction and labor-forfeit rules remain code-owned. A Realm stores the normalized snapshot and fingerprint in schema 15 saves; schema 7–14 load the frozen historical defaults. The initial file format is a strict integer-only TOML subset because the core currently declares no TOML/JSON parser dependency. Python uses the same C++ loader/snapshot. Godot path selection and runtime interaction remain a separate integration gate. Plan: [gameplay configuration loader](design/M5-gameplay-config-loader-plan.md); integration acceptance: [M5 gates](design/M5-integration-and-tuning-gates.md).

Tính cách tương đối bền; trạng thái cơ thể, tâm trạng và hiểu biết thay đổi nhanh hơn. Shared learned policy chọn quyết định tự chủ; mỗi người giữ memory và sampling riêng. Routine là kế hoạch sửa được, không phải chiến lược viết tay thay RL. Khác biệt năng lực/tính cách phải được đo, không chứng nhận bằng việc thêm field input.

Tie chưa giải quyết phải lộ ra và có cơ hội quyết định lại, không mặc định ID nhỏ thắng. Quy tắc withholding trong evaluator thuần là contract phát triển hiện tại, chưa là luật phân bổ cuối cho mọi tương tác hoặc tài nguyên giữa nhiều người.

Các hệ số ngủ, gắng sức, hồi phục và năng suất đã có một số quy ước thử được ghi trong T3.2–T3.12; chúng có đơn vị và phép kiểm nhưng chưa được hiệu chuẩn thành cân bằng sản phẩm. Những phần bệnh/chăm sóc, mô hình tuổi thọ đầy đủ, cadence inference mới, reward weights và determination còn thiếu phải có quyết định riêng trước khi triển khai. Không tự suy ra giới hạn làm 8/16 giờ. Đặc tả chi tiết về dữ liệu và điều kiện xác nhận nằm ở phần III; các ý trao đổi cũ không tự trở thành thông số đã duyệt.

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

Giữ 12 augment khởi đầu; năng lực theo contract agent-centric phần 3. Augment tác động nguồn lực/quy trình có phạm vi, không tạo tiền sau mở sổ hay ép NPC trung thành. Danh mục vùng/tuyến, toàn bộ 12 thẻ, năng lực, UI và cơ chế chi tiết nằm ngay trong phần II bên dưới; [bản v3](archive/design/implementation-plan.md) chỉ giữ vai trò truy nguyên. Mâu thuẫn phải giải quyết theo thứ tự ưu tiên mục 10.

## 5. Tech stack và ranh giới kiến trúc

| Thành phần | Trách nhiệm và nền tảng |
|---|---|
| Core | C++20 OOP/composition, CMake; độc lập Godot; sở hữu world, command, quyền, transaction, invariant và game time. |
| App | Godot 4.5.x native/GDExtension, godot-cpp phù hợp; GDScript UI, terrain 2D. Windows ưu tiên; app/package Linux hoãn. |
| Training | Python/PyTorch gọi cùng core qua pybind11. Không bộ luật Python thứ hai. Phạm vi build/kiểm chứng hiện tại là Windows; Linux/Kaggle runtime và package được hoãn. |
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
| M5 | Nền xã hội đủ sâu + quốc gia 27 khu/16 hàng/thành thị/quân sự/ngoại thương + profile B | Luật, liên kết xã hội, invariant, shared C++/Python binding được kiểm trên Windows, workload và hiệu năng thật. |
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

**Kết quả chuỗi huy động 2026-10-23:** paired Realm seed9201 dùng cùng người và cùng Work. Người phục vụ không tạo hàng trong cửa sổ tám ngày, đối chứng tạo8 món; lương tăng32 đồng; Mobilize chuyển40 từ treasury vùng sang firm. Sovereign accept roster và cam kết DryFood trước khi volunteer tự Migrate. Đóng route sau arrival gây thiếu1; Reject giữ tiền/kho/shipment/hạn; nhánh Accept gia hạn, mở route, mua/gửi shipment công và chỉ hoàn tất sau arrival qua save/load. Đây là một chuỗi fixture C++/Debug, không phải đánh giá nhiều seed hoặc policy. Target mới20 checks và CTest1/1 pass; không đổi production code nên không lặp full Realm hoặc adapter. [Báo cáo chi tiết](reports/M5.4-military-mobilization-intervention-chain-evidence-2026-10-23.md), [plan](design/M5.4-military-mobilization-intervention-chain-plan.md). §7/§27.3 đã có bằng chứng cho chuỗi quân sự này; chuỗi không xác nhận chiến đấu hoặc hoàn tất M5.

Kiểm tra sai ID/quyền/consent, actor chết, stale/duplicate, overflow, cạnh tranh nguồn lực; replay giữa escrow/travel/học nghề/kế vị; load hỏng không thay phiên. Observation/UI chỉ thấy thông tin được biết. Nhận nhiều cam kết không tạo thêm giờ, tiền, hàng hoặc dịch vụ.

## 8. RL và cá nhân hóa

Giữ learned policy cho lựa chọn tự chủ trong bản hoàn chỉnh; baseline/random/imitation-only phải có nhãn. Train offline trong cùng C++ core; khi chơi chỉ inference. Khởi điểm 3 họ governance/economic/civilian; không model riêng mỗi người. Recurrent memory không phải bằng chứng cá tính hoặc reasoning đạt yêu cầu.

Rollout ghi actor/decision, thông tin/observation/mask, action/log-prob, reward outcome, elapsed game time, memory/reset và terminated/truncated. PPO dùng rollout tươi; log replay dành chẩn đoán. Tách seed/train/validation/test và đối tác; đánh giá đúng sampling runtime. Không chọn lại model bằng test holdout.

Curriculum: cá nhân/hộ/nơi làm → một vùng → hai/sáu vùng → quốc gia → scale holdout. Không thưởng spam nút hay coi vay là thu nhập; invariant lỗi dừng run và lưu reproducer. M3 chỉ pilot; M5–M6 giữ smoke có ngân sách, production train sau freeze. Huấn luyện dài/GPU/Kaggle là tác vụ có ngân sách riêng; không tự mua dịch vụ.

**Điều kiện freeze để bắt đầu hoặc tiếp tục training M5:** hoàn tất luật hoạt động T1–T3 và các chu trình gameplay M5.0/1–M5.4 trong C++ core; nối các luật đó vào Python trainer dùng cùng C++ core; cập nhật observation, legal-action mask/action slots và reward theo contract hiện hành; chạy smoke có phạm vi qua C++→Python trước training. Có thể để sau training các phần UI/native presentation và M5.5 (profile B, stress/long-run, hiệu năng toàn chu trình, package Windows). Đây chỉ là điều kiện mở training, không phải nghiệm thu toàn M5; M5.5 cùng các gate sản phẩm vẫn bắt buộc trước khi gọi milestone hoàn tất. Chi tiết kiểm tra: [M5 integration gates](design/M5-integration-and-tuning-gates.md) và trạng thái áp dụng: [CURRENT_TASK](CURRENT_TASK.md).

Giữ các quality gates từ plan v3, không hạ khi hợp nhất docs:

| Gate | Yêu cầu |
|---|---|
| R0 — môi trường | C++/Python/app headless cùng transition; không lộ secret, baseline/random test giữ invariant; đo decisions/s, days/s và RAM. |
| R1 — learned policy sáu vùng | Ít nhất 3 training seeds, ≥100 tình huống holdout ghép cặp; so utility/nghĩa vụ/survival/bankruptcy/thương lượng/hold với baseline và initial. Có cải thiện chiến lược thật. |
| R2 — civilian/child agency | Sampling nhiều seed có phân bố hợp lý; đổi tuổi/budget/job/school/nhu cầu làm policy thay đổi đo được. Giữ ID/sampling riêng; không ép mọi trẻ cùng tương lai. |
| R3 — stat/mục tiêu | Giữ tài sản/thông tin/tình huống, thay INT/chuyên môn. Nếu công bố trí lược thì paired 95% CI cải thiện trung bình không chứa 0 trên bài nhiều bước. Goal/personality phân biệt hành vi khi lợi ích xung đột. |
| R4 — quốc gia/transfer | Đủ 24 vùng, 12 augment, stat cực trị/shock/vòng đời/vua khác phong cách; ≥100 holdout ngắn và 30 campaign hoặc sandbox dài tối đa 50 năm. Báo thời gian/count nếu kết thúc sớm, không coi sandbox tiếp tục là vua sống 50 năm. |
| R5 — native ONNX | Logits/mask/memory và action distribution/invalid probability giữa Torch/ONNX C++; tolerance float32 ban đầu abs 1e-5, rel 1e-3, đổi phải giải trình; quantization có gate hành vi riêng. Model lỗi pause, không fallback heuristic. |

Tên stat/INT trong R3–R4 chỉ các phép can thiệp năng lực kế thừa; schema agent-centric phải có mapping và phép đo tương ứng trước triển khai, không bỏ gate cá nhân hóa vì đổi tên field. R5 phải khai báo tolerance theo metric: logits/memory dùng absolute/relative error phù hợp, phân phối dùng metric thống kê phù hợp (ví dụ KL với cách xử lý xác suất 0 được ghi rõ). Các ngưỡng float32 trên là baseline ban đầu, không một ngưỡng tuyệt đối áp cho mọi metric; thay đổi/ngoại lệ phải có lý do và review, không tự nới gate theo kết quả test.

Khóa metric theo họ trước chọn checkpoint: noninferiority biên 5% trên thang chuẩn hóa đã định, có ít nhất một cải thiện chiến lược đáng tin và không reward hack. Không suy từ model world nhỏ sang B/C. Gate pilot chưa đạt tiếp tục ghi rõ; protocol/source lock thí nghiệm không được sửa theo test holdout.

## 9. Quy mô, hiệu năng và nền tảng

Profile A: 6 vùng, khoảng 10k người, 128–256 vai phức tạp để thử cơ chế. M5/profile B: 24+3 khu, 100k thường dân/trẻ em, khoảng 512–2048 bên quyết định phức tạp; phân biệt người, vai trò, tổ chức và policy invocation, không cộng trùng. Một triệu ở S. Constructor nhận một triệu không là benchmark.

Máy đích 16 GB RAM, CPU inference. Budget B: resident ≤6 GB, peak ≤8 GB cả save; input p95 <100 ms; ngày thường p95 ≤80 ms, kỳ tháng ≤500 ms. Mục tiêu render 60 FPS/1080p tham chiếu, tối thiểu ổn định 30 FPS; 1×–4× ổn định, 8× báo tốc độ thật theo gate. Đo core+observation+inference+validation/commit+index, p50/p95/p99, backlog, UI/input, RAM/save. Controller-only tách end-to-end inference.

Tối ưu bằng lịch sự kiện/lazy update có last-update và đủ ranh giới, index, bounded batches, shared data, sparse graph trước khi mở đa luồng. Actor không được gọi policy vẫn già, đói và chịu deadline. Snapshot đọc/ý định/commit tránh thứ tự thread/ID quyết định phân bổ. Không bỏ mô phỏng ngoài camera hoặc bơm tài sản để giữ world sống.

Sandbox ổn định tối thiểu 20 năm nhiều seed theo protocol; đo sinh/tử, sinh kế, giá, việc/nợ, nghĩa vụ, queue và kết quả can thiệp. Không chỉ kiểm “còn người sống”. Theo quyết định người dùng ngày 2026-10-19, Windows là nền tảng build và nghiệm thu hiện tại; Linux build/runtime parity và package được hoãn, không là gate của M5 hiện tại. Shared C++ core và Python binding vẫn phải qua kiểm tra Windows. Nếu sau này mở lại Linux, cần chạy và nghiệm thu riêng trên Linux.

Giữ yêu cầu stability tổng quát của plan v3: ≥30 seed trong harness, tối đa 50 năm hoặc ending, với metadata riêng nếu sandbox tiếp tục; không gộp nó với gate M5 tối thiểu 20 năm hay coi vài seed invariant là đã hoàn thành. Packaging phải thử offline không Python/Node, close/reopen, focus/sleep/resume, save path/quyền ghi trên platform được công bố. Atlas phải dễ nhận terrain, sông/đường/cầu hợp lý, overlay/route/picking và privacy, có tác dụng trong engine; art-only demo không qua gate.

## 10. Thay đổi và nguồn chuẩn

Ưu tiên: chỉ đạo trực tiếp của người dùng và AGENTS → plan này về scope → CURRENT_TASK về tác vụ/status → DEVELOPER_GUIDE về phương pháp → evidence theo revision. Archive là provenance; nếu thiếu contract phải làm rõ trước triển khai, không chọn prompt cũ có chữ APPROVED để bỏ qua review.

Phân vai hiện hành theo AGENTS: tab chính Codex trực tiếp lập kế hoạch, triển khai và kiểm tra; OpenCode tùy chọn cho tác vụ có phạm vi rõ. Tách lượt triển khai và lượt review; tự review phải được ghi đúng, kèm phép tính tay hoặc bằng chứng độc lập với cách viết code. Chỉ hỏi người dùng khi phải thay luật quyền/consent, phạm vi sản phẩm hoặc ngân sách đã chốt. Cập nhật CURRENT_TASK và HISTORY sau khi xác nhận nhóm chức năng; không chép trạng thái đang làm vào nhiều tài liệu.


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

- Đích địa lý dài hạn gồm heightfield, độ dốc, biome, đất/nước, lưu vực, tài nguyên và polygon lãnh địa. Theo working setting `atlas-cartography-v1` được ủy quyền ngày 2026-10-23, geometry đầu tiên dùng đơn vị bản đồ `map_unit`; chỉ đổi sang km sau khi đã chọn/kiểm một tỷ lệ địa lý chung. Không dùng tọa độ hoặc diện tích polygon của bản đầu để suy dân số hay sản lượng.
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

Phạm vi hiện hành giữ30 hàng thuộc16 nhóm ở §22.5 theo yêu cầu người dùng ngày2026-10-23. Ưu tiên hoàn thiện nguồn sản xuất và công dụng thực tế của30 hàng này; không tiếp tục đề xuất tăng lên54 hàng hoặc dành mã30–53. Nguyên liệu nội bộ của một cơ sở có thể được mô tả bằng resource ledger có nguồn/quyền/lượng rõ ràng; không tự biến chúng thành hàng giao dịch hoặc nguồn miễn phí.

### 15.3 Sản xuất và nhu cầu

Cơ sở sản xuất thuộc một chủ thể, có ngành, công suất, người làm, kỹ năng, nguyên liệu, tài sản và hao mòn. Sản lượng bị giới hạn bởi yếu tố thiếu nhất:

$$
Q=C_{\mathrm{eff}}\,\min\!\left(\rho_{\mathrm{labor}},\rho_{\mathrm{input}},\rho_{\mathrm{resource}}\right)\,\mu_{\mathrm{season}}.
$$

Ở đây $$Q$$ là sản lượng, $$C_{\mathrm{eff}}$$ là công suất hiệu dụng; các tỷ lệ $$\rho$$ lần lượt biểu diễn lao động, đầu vào và tài nguyên khả dụng; $$\mu_{\mathrm{season}}$$ là hệ số mùa vụ.

Các tỷ lệ bị chặn trong miền hợp lệ; mọi đầu vào được trừ đúng lượng. Nông nghiệp dùng lịch mùa: gieo trồng tạo sản phẩm dở dang, thu hoạch mới nhập kho. Mất mùa tác động đến vụ và kho; không tạo lương thực đều mỗi tháng.

Nhu cầu hộ dân theo quy mô dân số, mức sống và khả năng chi trả. Ưu tiên sinh tồn trước hàng xa xỉ. Tiền công, địa tô, lợi nhuận và thuế xác định phân phối thu nhập. Thiếu lương thực ảnh hưởng sức khỏe và lao động trước khi dẫn tới tử vong hoặc di cư; có cảnh báo và thời gian phản ứng.

### 15.4 Thị trường và giá

Mỗi vùng có chợ, tồn kho và giá theo nhóm hàng. Hộ và tổ chức lập nhu cầu mua trong ngân sách; người bán đưa hàng có thật. Khi thiếu hàng, phân bổ theo các vòng nhu cầu: nghĩa vụ hợp đồng đã bảo đảm bằng hàng, nhu cầu thiết yếu, sau đó nhu cầu còn lại; trong mỗi vòng phân bổ theo lượng đặt mua có khả năng thanh toán. Trưng thu là chính sách riêng, không mặc định mọi nhu cầu triều đình được ưu tiên.

Giá tháng sau điều chỉnh dần theo tỷ lệ cầu chưa đáp ứng và tồn kho so với mục tiêu. Dùng hệ số và biên độ thay đổi cấu hình được; mặc định giới hạn biến động mỗi tháng để tránh vòng lặp số học mất ổn định. Biên độ giá và mọi giới hạn kỹ thuật phải có trong cấu hình cân bằng, không rải trong mã.

Tách giá danh nghĩa và thu nhập thực. Giá tăng không đồng nghĩa quốc gia giàu hơn. Khi hiển thị tăng trưởng, có cả sản lượng thực và khả năng đáp ứng nhu cầu.

### 15.5 Thương mại và vận tải

#### Quyết định triển khai thị trường và dữ liệu tuyến — 2026-10-22

Người dùng giao Codex tự giải quyết các quyết định phù hợp đang chặn triển khai. Các lựa chọn dưới đây là cấu hình làm việc ban đầu để hoàn thiện luật đã nêu ở 15.4–15.5, chưa phải cân bằng đã hiệu chỉnh. Không đổi các ưu tiên phân phối, quyền sở hữu hay vai trò policy. Nhóm chức năng kế tiếp là thị trường theo nhu cầu và giá tháng; triển khai tuyến quốc gia sau khi cơ chế đặt mua/quyết toán đã ổn định.

**Đơn mua và nguồn cầu:** giữ Buy/Sell tức thời tương thích cho consumer hiện tại; bổ sung đường lệnh đặt mua hữu hạn với ID, chủ thể, vùng, hàng, lượng và giá tối đa. Đơn nhận hợp lệ giữ tiền từ tài khoản người đặt trong tài khoản ký quỹ; không tạo tiền. Chỉ đơn hợp lệ có tiền bảo đảm tạo cầu. Lệnh bị từ chối không đổi sổ cầu hoặc bất kỳ dữ liệu nào. Không suy cầu từ số lần policy bấm Buy lỗi. Không tự tạo đơn chiến lược cho NPC; policy/player gửi lệnh, nhu cầu sinh tồn tự động hiện hữu phải được ghi nhận đúng một lần qua cùng cơ chế giao dịch.

**Phân phối:** dùng ba vòng ở 15.4: hợp đồng đã bảo đảm bằng hàng, nhu cầu thiết yếu, còn lại. Đơn thông thường không được tự nhận nhãn thiết yếu; engine xác định từ nhu cầu sinh tồn thực tế và catalog. Trong mỗi vòng, phân bổ theo lượng đã có tiền bảo đảm, làm tròn xuống rồi chia phần dư theo phần lẻ lớn nhất, khóa phụ ID đơn tăng dần. Tổng cấp không vượt tồn kho. Đơn mua ngày hết hạn ở cuối kỳ xử lý ngày đó; phần chưa mua được hoàn tiền ký quỹ, lưu lượng thiếu vào tổng hợp tháng đúng một lần. Hủy trước quyết toán hoàn tiền và không tính là cầu thiếu. Chết hoặc chủ thể không còn hợp lệ xử lý tiền ký quỹ như tài sản của chủ thể/di sản, không ghi mất hoặc trả hai lần. Khớp giá không vượt giá tối đa; thuế và tiền bán ghi cùng giao dịch. Những hợp đồng dài hạn phải có thời hạn riêng, không dùng mặc định đơn ngày để thay luật hợp đồng.

**Giá tháng:** cho mỗi vùng/hàng, ghi D là tổng lượng cầu hợp lệ đến kỳ quyết toán, U là phần không đáp ứng và S là kho có thể bán tại cuối tháng. Cả ba là số lượng hàng thực, không phải giá trị tiền. Tồn mục tiêu T=max(1,D) (một tháng cầu vừa quan sát). Nếu D=0 thì giữ giá, không tự suy nhu cầu. Nếu D>0, áp suất q=clamp(U/D + (T-S)/T, -1, 1); tỷ lệ thay đổi r=clamp(0.10*q,-0.10,0.10); giá mới clamp(round_half_up(giá_cũ*(1+r)),giá_sàn,giá_trần). Dùng số nguyên tỷ lệ với phép nhân/chia kiểm tràn và quy tắc làm tròn xác định, không phụ thuộc float của platform. Sàn mặc định 1 đơn vị tiền, trần mặc định 1.000.000 đơn vị tiền; miền cấu hình phải kiểm sàn<=trần và mọi sản phẩm tiền×lượng trước commit. Giá nguyên nhỏ có thể không đổi sau làm tròn; phải báo rõ hiện tượng này khi đánh giá cân bằng. Không đổi số ngày/tháng hiện hành. Cập nhật giá một lần khi qua ranh giới tháng rồi mở tổng hợp tháng mới; pause/save/load/chia bước thời gian không chạy lặp.

**Cấu hình và lưu/tải:** hệ số 0.10, biên độ 0.10, sàn/trần và mục tiêu một tháng nằm trong GameplayConfig có phiên bản, được chụp vào thế giới/save; không đọc lại file cấu hình để thay luật giữa campaign. Save cũ dùng luật/giá lịch sử theo migration tường minh, không âm thầm bật giá động; thế giới mới bật bộ luật mới khi đã qua kiểm tra. Lưu đơn, ký quỹ, tổng hợp tháng và kỳ đã xử lý. Thiếu hoặc sai tham chiếu phải từ chối tải trước khi công bố thế giới.

**Tuyến quốc gia:** không chờ tọa độ hoàn chỉnh để triển khai vận tải. Dùng dữ liệu cấu hình từng cạnh atlas gồm hướng đã duyệt, thời gian nguyên tick, sức tải theo ngày, phí trên đơn vị hàng và ID bên nhận phí. Mặc định thử cho cạnh chưa có đo đạc: đường bộ 2 ngày, sông xuôi dòng 1 ngày, sức tải 64 đơn vị/ngày, phí 1 đơn vị tiền/đơn vị hàng/cạnh; thời gian quy đổi bằng lịch core hiện hành. Đây là thông số game trừu tượng, không suy km hoặc tốc độ thực, không thêm cạnh ngược sông. Cạnh thiếu bên nhận phí hợp lệ bị từ chối cấu hình, không tự đốt phí hoặc cấp tiền. Ghi đè theo từng cạnh và lưu snapshot cấu hình. Kiểm nhạy cảm ở 0.5×/1×/2× sức tải và thời gian khi hoàn tất nhóm vận tải; không gọi việc có tuyến mẫu là hoàn tất quốc gia 27 vùng.

#### Quyết định triển khai vận tải quốc gia — 2026-10-22

- **Ánh xạ atlas sang tuyến chạy được:** giữ ID vùng theo atlas 0–26 và không đổi vị trí cạnh. Cạnh `GreatRiver` chỉ chạy `from → to` như atlas đã ghi; không tạo chiều ngược. `RoadImprovement` chạy hai chiều, `Coastal` chạy hai chiều, còn các hành lang `East`, `North`, `West` đang có chiều hai chiều nhưng chưa có loại tuyến thì dùng mặc định thử đường bộ. Năm liên kết `RoyalDomains` hiện chưa định hướng/loại tuyến; cấu hình thử bật hai chiều như đường bộ để 3 khu triều đình nối được vào mạng quốc gia. Đây là giả định gameplay tạm có phiên bản, không phải kết luận địa lý; dữ liệu theo cạnh có thể tắt hoặc đổi thời gian/sức tải/phí trong config trước khi tạo Realm. Không tự sinh liên kết ngoài atlas.
- **Giá trị và thời gian:** sông xuôi dòng 1 ngày; đường bộ và ven biển 2 ngày; sức tải 64 đơn vị hàng mỗi cạnh mỗi ngày; phí 1 đồng bạc trên mỗi đơn vị hàng qua mỗi cạnh. Đây là giá trị game thử, không quy đổi sang km. Kiểm tra cấu hình và phép nhân phí×lượng bằng số nguyên có kiểm tràn. Mỗi cấu hình cạnh lưu hướng chạy, trạng thái mở, số ngày, sức tải, phí và bên nhận phí vào Realm/save để load không phụ thuộc file cấu hình hiện tại.
- **Ngày xếp hàng và dự báo tuyến:** lệnh nhận sau khi ngày D kết thúc bắt đầu tải sớm nhất ngày D+1. Nếu tải ngày L qua cạnh có thời gian N ngày, lô đến cuối ngày L+N; có thể tải chặng sau trong cùng ngày nhận hàng theo thứ tự pipeline. Sức tải là chung hai chiều của liên kết atlas. T3.8 đưa queue cargo lot hiện đang chờ trên từng link trong đường ứng viên vào phép tính; đối với cạnh sau, trừ sức tải có thể phục vụ queue trong lúc shipment mới di chuyển qua các cạnh trước. Lô đang đi và chuyến đến sau thời điểm ước tính chưa được tính. Lịch tải hằng ngày vẫn quyết định giao hàng thật, nên forecast không là cam kết ngày tới hay dự báo congestion đầy đủ. [Plan T3.8](design/M5-T3.8-visible-transport-queue-plan.md), [report kết quả](reports/M5-T3.8-visible-transport-queue-evidence-2026-10-22.md).
- **Bên thu phí:** mỗi chiều có một người nhận phí là ngân khố vùng khởi hành cho chuyến đi qua cạnh đó. ID người nhận chính là ID vùng 0–26; tài khoản loại `ProvinceTreasury` được xác định tường minh, phải tồn tại trong Realm. Khi cấu hình không có người nhận hợp lệ thì Realm mới hoặc save bị từ chối; không bỏ phí, không đưa phí vào `firm` thay cho ngân khố, không tạo khoản tiền mới.
- **Chủ hàng và sức tải:** lệnh vận tải do cá thể sở hữu hàng gửi. Khi lệnh được nhận, lượng hàng yêu cầu rời kho cá thể và được cộng đúng một lần vào sổ hàng đang được Realm giữ; lệnh giữ phí theo đường đã chọn. Hàng đang đi không nằm trong kho vùng hoặc kho cá thể. Mỗi ngày, từng liên kết atlas dùng chung một giới hạn lượng hàng tổng cho cả hai chiều; các lô theo ID shipment rồi ID lô tăng dần, tải tối đa tới giới hạn còn trống, phần còn lại đợi ngày sau. Phí từ ký quỹ chuyển sang ngân khố vùng khởi hành đúng lúc lô tải lên cạnh; nếu ngân khố không còn đủ chỗ nhận phí hoặc ký quỹ không đủ, lượng tải được giới hạn an toàn. Đến điểm cuối, hàng được cộng vào inventory người gửi còn sống hoặc hồ sơ di sản của người đã mất. Khi hoàn tất, không còn phí dự kiến chưa chi.
- **Tuyến, gián đoạn và hủy:** lệnh mang đích và lựa chọn dịch vụ `Fastest` hoặc `LowestFee`; policy chọn lựa chọn dịch vụ, planner tìm đường có hướng/đang mở rồi tie-break ổn định. `Fastest` tính queue snapshot hiện chờ trên các link của candidate path và giảm queue các chặng sau theo thời gian shipment đi qua tuyến trước. Queue/lô xuất hiện sau snapshot chưa được dự báo. `LowestFee` giữ tổng phí làm tiêu chí chính. `Safest` chỉ được bật khi có dữ liệu rủi ro; không giả lập an toàn bằng hằng số. Cạnh đóng không nhận lô mới; phần đã trên cạnh vẫn tới nút kế tiếp rồi chờ nếu chặng sau đóng. Chủ gửi chỉ có thể lấy lại hàng chưa tải đang ở cùng vùng; người ở vùng khác không được kéo hàng đang chờ về từ xa. Hủy phần chờ không dừng lô đang đi: lô đó tới nút kế tiếp trước, rồi chủ có thể hủy khi ở cùng vùng. Phí của chặng chưa tải được hoàn theo lượng hủy; phí đã trả cho chặng bắt đầu đi không hoàn lại. Đóng cạnh không xóa lô.
- **Trạng thái Realm và vận tải sau triển khai:** giới hạn profile 1/2/6 vùng nêu trong kế hoạch ban đầu đã được xử lý; Realm hiện tạo và lưu được profile atlas 27 vùng. C++ có tuyến, lệnh chuyến, ký quỹ phí, tải hàng theo sức chở chung hai chiều, giao hàng, hủy hợp lệ, xử lý di sản, migration save và queue snapshot dọc candidate path; Python binding đã được đối chiếu với C++ trong một chuỗi vận tải. Kết quả tại [report vận tải](reports/M5-national-transport-evidence-2026-10-22.md) và [report T3.8](reports/M5-T3.8-visible-transport-queue-evidence-2026-10-22.md). Còn thiếu forecast lô đến sau snapshot, giao diện và policy/model tương thích Atlas; không dùng API quyết định v11 của Realm 7 vùng để điều khiển Atlas.
- **Phép thử độ nhạy vận tải, phiên bản trial-1 — 2026-10-22:** giữ mặc định tham chiếu là đường bộ 2 ngày/chặng và 64 đơn vị hàng/liên kết/ngày. Dùng một fixture C++ cố định với cùng world seed, cùng một liên kết đường bộ hai chiều, chỉ mở liên kết đó, gửi 80 đơn vị cùng loại hàng một lần, giữ phí 1 đồng/đơn vị/chặng, và thay đúng hai thông số cấu hình. Chạy đủ chín tổ hợp: thời gian `1/2/4 ngày` (0,5×/1×/2×) và sức tải `32/64/128 đơn vị/ngày` (0,5×/1×/2×). Hai mức thời gian nhân nguyên không tạo phần lẻ; miền hợp lệ theo manager là 1–365.000 ngày, sức tải 1–1.000.000 đơn vị/ngày. Ghi dữ liệu gốc mỗi tổ hợp: thời gian, sức tải, số ngày thực sự tải, ngày giao toàn bộ quan sát được sau xử lý đầu ngày, lượng giao và kết quả invariant. Mục đích là xác nhận luật lịch tải/thời gian phản ứng đúng và làm rõ độ trễ trong một tình huống có hàng đợi; một seed/fixture không chứng minh cân bằng, không đại diện cho mạng quốc gia và không được gọi là thông số đã hiệu chuẩn. Không thay các giá trị mặc định bằng kết quả của phép thử này.
- **Lưu và tính toàn vẹn:** schema 27 hiện đã ghi snapshot cấu hình tuyến. Thêm shipment mà không đổi nghĩa save cũ, vì vậy phần shipment thuộc schema 28 với section riêng cho lệnh/chuyến, lô theo chặng, ký quỹ phí và hàng Realm đang giữ; sức tải đã dùng trong ngày được khôi phục từ các lô đã tải và thời gian cạnh. Schema 7–27 migrate sang ledger shipment trống, đồng thời giữ luật snapshot tuyến của schema 27. Tải schema 28 kiểm ID lệnh/lô, người hưởng lợi còn sống hoặc có di sản, hàng, đường, ngày đi/đến, số dư ký quỹ, hàng giữ và sức tải trước khi thay Realm đích. Save/load giữa các chặng phải cho cùng kết quả với chạy liền.
- **Phân kỳ kiểm tra:** trước tiên bổ sung hướng có hướng vào planner dùng chung mà không đổi mặc định Campaign; chạy test LogisticsManager Release và một Campaign pipeline. Tiếp theo nối cấu hình atlas/validate và công cụ chọn đường, kiểm từng loại cạnh, river reverse, đóng tuyến, tie và phí/overflow trong manager. Chỉ sau khi API thuần có oracle mới tích hợp command/escrow/hàng theo lô; kiểm trực tiếp thành công, reject nguyên tử, sức tải hai chiều và người nhận phí. Cuối nhóm kiểm schema migration/roundtrip/reject và chuỗi mua → vận tải → thời gian → tới nơi → save/load bằng C++; khi API ổn định mới kiểm các consumer gọi trực tiếp. Kiểm độ nhạy 0.5×/1×/2× ở cuối nhóm, đồng thời giữ nguyên dữ liệu nguồn và biểu đồ nếu có lượt AI/RL đánh giá.
**Điều kiện kết thúc nhóm thị trường:** lệnh đặt mua → ký quỹ → phân phối hàng/thuế/hoàn tiền → qua tháng đổi giá → lưu/tải tiếp tục cho kết quả tương đương chạy liền; kiểm thiếu hàng, thiếu tiền, tràn số, đơn trùng/hủy/hết hạn, chủ thể chết và các ranh giới giá. Oracle tính tay trên thế giới nhỏ độc lập với helper sản phẩm. C++ trước; binding mới phải có parity theo cùng chuỗi trước khi công bố dùng được từ Python. Learned policy, vận tải quốc gia, toàn bộ 16 nhóm nguồn hàng và M5 vẫn là các điều kiện riêng còn phải hoàn tất.

**Nối lệnh đặt mua vào consumer — 2026-10-22:** Python binding gọi `MarketOrderCommand` qua `Realm::execute`, đọc lệnh đang mở và tiền giữ; native `RealmRuntime` cùng GDExtension nhận lệnh đặt/hủy, fingerprint để từ chối snapshot cũ và trả ID lệnh. Windows Release Python binding kiểm 13 hành vi; C++ order test 40; runtime contract 48 kiểm tra nhưng chỉ hai guard Realm chưa nạp liên quan lệnh mới thực chạy; bundle v6 hiện có bị từ chối nên success/cancel native chưa được chạy. GDExtension `polyec` build thành công, chưa có Godot runtime để smoke. [Báo cáo và giới hạn](reports/M5-market-order-python-native-adapter-evidence-2026-10-22.md). Màn hình GDScript chưa gọi API; consumer UI và policy/model tương thích vẫn mở. Không dùng chức năng này để tuyên bố learned policy hoặc M5 hoàn tất.

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

Chấp thuận phục vụ quân ngũ không đồng nghĩa chấp thuận tham chiến. Mỗi volunteer phải tự đồng ý riêng sau khi đợt phục vụ bắt đầu; chỉ chính họ được rút lại lựa chọn đó. Việc gọi một người vào trận, cách xử lý khi họ rút lại trong lúc giao chiến, và quyền chỉ huy vẫn phải được định nghĩa trước khi có battle-resolution. Đây là điều kiện bổ sung để bảo vệ consent hiện có, không phải luật đã hoàn tất về chiến tranh.

Ba thế lực bên ngoài tạo quan hệ giao thương, đe dọa và điều ước. Trong bản đầu, họ là tác nhân kinh tế–ngoại giao tổng hợp có ngân sách, cung/cầu hữu hạn và trạng thái quan hệ; không mô phỏng đầy đủ lãnh địa nội bộ của họ. Ngoại thương phải hạch toán với khu vực bên ngoài, không trở thành nguồn mua vô hạn.

Lát `foreign-trade-suspension-v1` bổ sung một phần quyền chính sách của triều đình: Sovereign còn sống có thể tạm đình chỉ hoặc mở lại lệnh ngoại thương mới với riêng từng đối tác A/B/C. Lệnh đình chỉ tắt cả nhập và xuất mới với đối tác đó; đơn đã được chấp nhận vẫn đến/quyết toán, và tuyến vận tải vật lý không bị đổi. Trạng thái nằm trong section `FOREIGN_TRADE_4`, save57; save56 trở về trước nạp giao dịch mở. Đây chưa phải phản ứng của đối tác, cấm vận do chiến tranh hay treaty song phương; tiền/hàng/sức chứa vẫn là sổ hữu hạn hiện có. Luật, migration, phép kiểm và giới hạn: [plan](design/M5.4-foreign-trade-suspension-plan.md), [báo cáo](reports/M5.4-foreign-trade-suspension-evidence-2026-10-23.md).

Lát `bilateral-trade-history-v1` đã thêm sổ lịch sử theo từng đối tác: số đơn nhập/xuất đã hoàn tất, lượng hàng nhận/giao và gross tiền hàng theo chiều. Chỉ tăng sau khi quyết toán; gross không bao gồm thuế/phí và không đồng nghĩa lợi nhuận. Save58 ghi section `FOREIGN_TRADE_5`; save57 trở về trước khởi tạo lịch sử bằng0. Đây là dữ liệu lịch sử cho người chơi, chưa phải quan hệ ngoại giao hoặc lựa chọn tự chủ của đối tác. Phạm vi, migration, phép kiểm và giới hạn: [plan](design/M5.4-bilateral-trade-history-plan.md), [báo cáo](reports/M5.4-bilateral-trade-history-evidence-2026-10-23.md).

Lát `atlas-multi-region-raid-fronts-v1` mở rộng vụ đột kích từ Nam Cảng tới mọi vùng Atlas có tuyến nội địa đang mở và còn sức tải. Mỗi đối tác ngoại thương hiện có chỉ mở một vụ đang chờ; ba đối tác có thể tạo tối đa ba mặt trận ở ba vùng khác nhau. Đối tác trả phí lực lượng, DryFood và phí vận tải vào ngân khố từng vùng nhận phí; chiến lợi phẩm Salt lấy từ đúng vùng đích. Consent, quân tình nguyện, quyền phản hồi Sovereign, điểm chiến đấu và thương vong giữ nguyên. Đây chưa phải quyền chiếm đóng hoặc chiến dịch: chưa có controller theo vùng, lãnh thổ đối địch, tù binh, đầu hàng hay đình chiến, và UI chưa nối mặt trận. [Plan luật và giới hạn](design/M5.4-multi-region-fronts-plan.md); [báo cáo kiểm chứng core](reports/M5.4-multi-region-raid-fronts-evidence-2026-10-25.md).

Lát tiếp theo `military-occupation-v1` đã thay đổi phần “chưa có controller” ở trên: thắng raid nay tạo quyền kiểm soát quân sự của đối tác tài trợ tại vùng đích, không chuyển chủ quyền pháp lý hoặc quyền sở hữu đất. Save schema76 lưu partner và quân đồn trú; save cũ nạp với toàn bộ vùng do Realm kiểm soát. Governance địa phương không thể đổi thuế, chi ngân khố, điều hành cơ sở/quỹ công, huy động hoặc chuyển khẩu phần khi vùng bị giữ; tiền thuế tự động vẫn vào sổ vùng và đối phương không được tiêu khoản đó. Private work và hành động cá nhân không bị khóa. Mỗi quân đồn trú cần1 DryFood mỗi ngày từ tồn kho hữu hạn của chính đối tác; thiếu hụt làm quân rút theo phân bổ tỷ lệ và phần dư theo ID vùng tăng dần. Sovereign được phản công bằng volunteer đang sống, khỏe, mobilized, có consent tham chiến và hiện diện tại vùng; luật hòa và tính điểm được version hóa trong [plan](design/M5.4-military-occupation-and-recapture-plan.md). C++ kiểm tra lệnh địa phương bị chặn, khẩu phần, capture/recapture và save/load qua [báo cáo](reports/M5.4-military-occupation-and-recapture-evidence-2026-10-25.md). Mốc này chưa có UI mặt trận, tù binh, đầu hàng, đình chiến hay quyết định tự chủ của đối tác; các phần đó vẫn thuộc M5.4.

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

## 20. Năng lực và cá tính nhân vật chủ chốt

**Phạm vi áp dụng:** phần 20 giữ chi tiết thiết kế v3 để không mất yêu cầu về chuyên môn, tính cách, quyền biết và tiến bộ. Schema sáu stat, phân bố 0–100, giới hạn trait và công thức XP dưới đây là phương án kế thừa để đối chiếu, không phải yêu cầu triển khai mới. Contract agent-centric phần 3 thay yêu cầu cố định sáu stat/XP bằng capability vector, memory/knowledge và learned policy. Trước khi chuyển các thẻ hoặc gate còn dùng tên stat sang schema mới, supporting plan phải ghi mapping, đơn vị, observation/reward/save/model impact và test; chưa được tự áp hệ số cũ cho capability mới.

### 20.1 Sáu nhãn năng lực của thiết kế v3, thang tham chiếu 0–100

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

### 20.4 Kinh nghiệm và nhiệm vụ — phương án XP kế thừa

Nhân vật tích lũy XP theo công việc thực chất hoàn thành, độ khó và mức tham gia. Một nhiệm vụ không phát XP lặp lại khi mở/đóng hoặc đổi người phụ trách. Mỗi stat tăng tối đa 3 điểm/năm từ kinh nghiệm; có lợi suất giảm, đánh giá cân bằng sau chơi thử. INT chỉ tăng tối đa 1 điểm/năm từ dự án học tập dài hạn; không tăng vì spam thương lượng.

Chuyên môn nghề mở đặc tính theo mốc 40/60/80, ví dụ xử lý thêm một hồ sơ hoặc tổ chức khảo sát chi tiết; không biến số cao thành quyền pháp lý mới. Sức khỏe và stress có modifier tạm thời, UI tách chỉ số nền/hiệu dụng. Không tăng XP cho policy model: XP thay đổi input và khả năng trong môi trường, không huấn luyện lại mạng ngay trong save.

Mức năng lực bộ máy phụ thuộc người chủ trì, nhân sự và tải. Dùng hệ số chuyên môn thử đầu $$m(s)=0{,}75+0{,}005s$$, với $$s\in[0,100]$$ và $$m(s)\in[0{,}75,1{,}25]$$, cho tốc độ phần công việc hành chính tương ứng; không nhân toàn bộ sản lượng vùng. DIP và INTG dùng trong thời gian/chất lượng quy trình, không cộng thẳng vào xác suất “đồng ý” hoặc “phát hiện mọi âm mưu”.

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

Khi dùng hazard, xác suất theo bước thời gian có thể tính $$p=1-\exp(-\lambda\Delta\tau)$$, lambda phụ thuộc trạng thái hợp lệ. Cần hiệu chuẩn và test theo cadence; không lấy “mỗi tháng 20% crisis” làm mô hình cho mọi thiên tai. Ngẫu nhiên có seed giúp tái hiện, không tạo lịch tình tiết bắt buộc. Từ cùng seed, hành động khác phải thay đổi diễn tiến.

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

## Phần III — đặc tả triển khai, dữ liệu và điều kiện xác nhận M5

Phần này mở rộng các yêu cầu ở phần I–II để người triển khai không phải tự suy luật từ tên milestone hoặc từ code đang có. Nó mô tả sản phẩm phải làm gì, dữ liệu nào cần giữ và bằng cách nào kiểm chứng. Các ví dụ đã có code được dẫn tới report theo phiên bản; chúng không chứng nhận source mới chưa được kiểm. Trạng thái đang làm tiếp tục chỉ nằm trong CURRENT_TASK.

Phân biệt ba loại quyết định: **luật sản phẩm đã chốt** phải được giữ; **quy ước thử có phiên bản** được triển khai để đo nhưng chưa phải cân bằng tốt; **điểm chưa xác định** phải được ghi rõ trước khi viết chức năng phụ thuộc. Không điền một hằng số tùy ý, thêm field rỗng hoặc dùng baseline để làm cho một yêu cầu có vẻ đã hoàn tất.

## 22. Schema, phiên bản và cấu hình của một thế giới

### 22.1 Schema nghĩa là gì trong PolyEc?

Schema là quy định về cấu trúc và ý nghĩa của dữ liệu. Ví dụ một bản ghi bằng chứng điều tra phải nói ai biết, biết Fact nào, nhận lúc nào, từ Investigation nào, loại thông tin và mức tin cậy. Chỉ ghi sáu số mà không nói số nào là ngày, ID hoặc dấu “chưa biết” thì chưa đủ schema.

Mỗi nhóm dữ liệu phải định nghĩa các mục sau trước khi có writer/reader hoặc binding:

| Thành phần đặc tả | Nội dung phải ghi |
|---|---|
| Định danh | Tên trường/ID ổn định; phạm vi duy nhất; có được tái sử dụng sau khi đối tượng mất hay không. |
| Chủ sở hữu | Realm, Person, hộ, xưởng, tổ chức hay hồ sơ di sản sở hữu dữ liệu; bên nào chỉ được đọc bản sao. |
| Kiểu và đơn vị | Số nguyên/số thực/enum/danh sách; đồng tiền, số món hàng, tick game, ngày, điểm 0–100 hay xác suất. |
| Miền hợp lệ | Hai đầu miền, trường tùy chọn, giá trị “không rõ”, tham chiếu bắt buộc và cách báo lỗi. |
| Quy tắc cập nhật | Command hoặc mốc game time nào được đổi trường; điều kiện trước và sau; xử lý tràn số. |
| Tầm nhìn | Cá thể nào được biết, người chơi được xem gì, policy của ai nhận trường đó. |
| Lưu và chuyển phiên bản | Trường được ghi ở section nào; cách đọc dữ liệu cũ; dữ liệu thiếu có thể bổ sung chắc chắn hay phải để unknown. |
| Kiểm chứng | Ví dụ tính tay, thành công, từ chối, biên, save/load và consumer thực sự sử dụng dữ liệu. |

Có ba loại phiên bản khác nhau. **Save schema** quy định cách cất thế giới. **Model contract** quy định ý nghĩa dữ liệu vào/ra và luật mà policy được huấn luyện để sử dụng. **Gameplay configuration** là bộ tham số của một thế giới. Đọc được save không chứng minh model cũ dùng đúng luật; hai tensor cùng kích thước vẫn có thể mang nghĩa khác.

### 22.2 Sổ phiên bản và migration

Các mốc sau giải thích vì sao định dạng tăng phiên bản; không phải yêu cầu đưa writer hiện hành trở về phiên bản cũ:

| Mốc Realm | Dữ liệu được bổ sung hoặc đổi nghĩa | Cách xử lý dữ liệu trước mốc |
|---|---|---|
| 26 | Đơn mua có tiền giữ và tổng hợp cầu/giá tháng | Giữ luật/giá lịch sử theo migration thị trường đã quy định. |
| 27 | Snapshot cấu hình tuyến quốc gia | Tạo cấu hình mặc định đúng profile đối với save cũ không có tuyến. |
| 28 | Shipment, lô hàng, hàng đang giữ và phí chưa chi | Tạo danh sách shipment rỗng; không tự phát chuyến hoặc hàng. |
| 29 / SCHEDULE_6 | Chú ý Work thực theo người/xưởng | Dựng mức nền đã chốt từ công đã xử lý; không sinh lại hàng cũ. |
| 30 / BODY_3 | Mốc công sức đã tính vào gắng sức và phần dư Work/Rest | Đặt mốc theo công sức sau migration; không làm nhân vật mệt lần hai vì lịch sử. |
| 31 / KNOWLEDGE_2 | Investigation nguồn của Knowledge | Thiếu nguồn để 0=unknown; không đoán bằng vùng/ngày gần nhau. |
| 32 / CAPABILITY_SCHEMA_1 | Tập năng lực legacy được bật trong Realm | Mặc định đủ sáu ID, giữ nguyên giá trị từng Person. |
| 33 / KNOWLEDGE_3 | Loại thông tin và confidence | Nguồn điều tra hợp lệ thành SecretEvidence; thiếu nguồn thành UnknownLegacy; confidence chưa đo là -1. |
| 34 / SCHEDULE_7 | Công sức sản xuất quy đổi theo skill | Công sức lịch sử nhận hệ số trung tính; output cũ không được tính lại. |
| 35 | Sáu giá trị tính cách của từng người | Save7–34 nhận trung tính50, không đoán lại lịch sử; [report T3.13](reports/M5-T3.13-personality-observation-evidence-2026-10-22.md) ghi phạm vi đã kiểm. |

Reader phải chọn đúng grammar theo số phiên bản của từng section. Không nâng số phiên bản Realm rồi giả định mọi section đều có định dạng mới. Migration phải đi qua world tạm, kiểm tham chiếu, cận số, tiền/hàng và các mốc đã xử lý trước khi thay thế phiên chơi. Save hỏng không được làm mất thế giới đang mở. Fixture save cũ phải thật sự dùng grammar cũ; chuyển fixture chỉ xóa/thêm đúng trường, giữ byte lịch sử ở nơi test đang đối chiếu byte.

Fingerprint là dấu kiểm tra tính từ dữ liệu đã chọn. Nó hữu ích để so hai thế giới hoặc phát hiện một lệnh bị từ chối vẫn sửa dữ liệu, nhưng không chỉ ra trường nào sai và không thay phép đối chiếu tiền/hàng/ngày cụ thể. Report phải nêu fingerprint bao phủ gì; hash source/binary dùng để nhận diện bản chạy là một loại bằng chứng khác.

### 22.3 Cấu hình bất biến trong một campaign

Mỗi tham số có tên, phiên bản, đơn vị, mặc định lịch sử, miền an toàn, cách làm tròn và chức năng tiêu thụ. Parse toàn bộ vào cấu hình tạm; khóa lạ, ID hàng/recipe sai, số âm trái luật, mẫu số bằng 0 hoặc phép nhân có thể tràn phải bị từ chối kèm tên trường. Một Realm giữ snapshot cấu hình, tức bản chụp các tham số dùng cho thế giới đó; sửa file ngoài chỉ áp dụng khi tạo thế giới mới. Load phải dùng snapshot trong save dù file ngoài đã đổi.

Các tham số sản xuất, lương, capacity, giá và vận tải không được biến quyền sở hữu/consent hay bảo toàn thành công tắc tắt được. Thêm tham số mới cần nêu ảnh hưởng tới save, transition, reward và model compatibility. Report replay/benchmark/train phải ghi cùng fingerprint cấu hình; khác cấu hình không được ghép làm một phép so sánh cùng điều kiện.

<a id="content-definitions-v1"></a>

### 22.4 Định nghĩa trước nội dung game và cách triển khai — yêu cầu 2026-10-23

Trước khi viết một nhóm chức năng, chuẩn bị đầy đủ danh mục đối tượng, thuộc tính, quan hệ và quy tắc mà nhóm đó cần trong tài liệu này. Mục đích là để triển khai theo thiết kế đã đối chiếu, không vừa thêm mã hàng vừa phát hiện chưa có nguồn, người sử dụng hoặc quyền thực hiện. Yêu cầu này áp dụng cho hàng hóa, tài nguyên, công thức, cơ sở, nghề, hoạt động, dịch vụ, tổ chức, chức vụ, vùng/địa điểm và quân sự. Định nghĩa trước không có nghĩa tạo hàng loạt class rỗng hoặc công bố tính năng chưa chạy.

Phân biệt **định nghĩa dùng chung** với **đối tượng đang tồn tại trong thế giới**. Ví dụ định nghĩa Paper nói giấy là gì, đơn vị và các công thức sử dụng; kho của mỗi xưởng mới ghi số Paper thật mà chủ xưởng sở hữu. Định nghĩa bệnh hoặc dịch vụ điều trị không tự tạo người bệnh, thuốc, người chăm sóc hay hiệu quả chữa trị. Dữ liệu chung được đọc nhất quán bởi C++, Python và native/Godot; số dư, tiến độ, vị trí và vòng đời thuộc đối tượng runtime trong Realm.

Mỗi định nghĩa phải có các nội dung sau; trường không áp dụng phải ghi rõ lý do, còn trường chưa quyết định phải chỉ rõ quyết định còn thiếu thay vì điền số giả:

| Nội dung | Phải xác định trước khi triển khai hành vi phụ thuộc |
|---|---|
| Mã và tên | Mã ổn định, khóa chữ duy nhất, tên/giải thích tiếng Việt, nhóm, phiên bản và tình trạng còn sử dụng hay đã ngừng. |
| Kiểu, đơn vị và giới hạn | Đơn vị kế toán/vật lý, lượng nhỏ nhất, kiểu số, cận an toàn, làm tròn, giá trị chưa biết và cách xử lý dữ liệu sai. |
| Thuộc tính và vòng đời | Trường bắt buộc/tùy chọn; giá trị khởi tạo có nguồn; điều kiện tạo, thay đổi, hết hạn, chết, đóng hoặc loại bỏ. |
| Chủ sở hữu và quyền | Ai sở hữu đối tượng, tài sản hoặc nghĩa vụ; ai được xem, đề nghị, đồng ý, thực hiện, sửa và hủy; không suy quyền từ việc cùng vùng/cùng nhóm. |
| Đầu vào và kết quả | Người, thời gian, tiền, hàng, tài nguyên, địa điểm, năng lực cần có; lượng tiêu/sinh, người nhận và hệ quả khi không đủ nguồn. |
| Quan hệ | Mã của các đối tượng được tham chiếu, quan hệ một/nhiều, bên chịu trách nhiệm, điều kiện tham chiếu còn hợp lệ và xử lý khi bên liên quan mất. |
| Tác động tới quyết định | Command thực hiện, lựa chọn có thể xuất hiện, điều kiện hợp lệ, thông tin actor biết, kết quả quan sát và tác động tới reward nếu có. |
| Lưu và tương thích | Phần dữ liệu chung và dữ liệu từng thế giới, snapshot cấu hình, save/migration, ảnh hưởng ý nghĩa model và các lớp gọi core. |
| Bằng chứng và mức hoàn thiện | Tách rõ đã định nghĩa, đã code, đã kiểm core, đã kiểm Python/native/Godot; dẫn báo cáo và ghi từng phần còn thiếu. |

**Mã ổn định:** giữ nguyên GoodId0–29 và các ID đã xuất hiện trong save; thêm mới ở cuối hoặc trong miền đã được đặc tả, không đánh số lại/xài lại ID đã bỏ. Khóa chữ là tên định nghĩa, không phải số slot hành động UI/policy. Mã recipe, nghề hoặc dịch vụ mới chỉ được cấp sau khi đối chiếu registry hiện hành; tên đề xuất trong thiết kế chưa được coi là ID số đã lưu. Tham chiếu sai phải được báo lỗi; không tự đổi sang DryFood, vùng0 hoặc một đối tượng mặc định để tiếp tục.

**Cấu trúc code:** tiếp tục C++20 OOP và composition. Dùng đối tượng dữ liệu dùng chung cho các loại hàng/công thức; thêm class hoặc thành phần xử lý khi có hành vi, trách nhiệm và dữ liệu vòng đời riêng. Không bắt buộc mỗi GoodId có một subclass. Muối và giấy có thể dùng chung cơ chế kho/giao dịch; khác biệt nằm trong nguồn và recipe. Điều trị hoặc chăn nuôi cần cơ chế riêng nếu có quá trình, điều kiện và hậu quả riêng. Không viết lại cùng luật ở Python hay Godot.

### 22.5 Danh mục hàng hóa chuẩn bị trước khi mở rộng code

Nguồn định danh đang có là [`good_catalog.hpp`](../core/include/polyec/good_catalog.hpp): 30 GoodId, 16 GoodGroup, tên và giá tham chiếu. Bảng dưới đối chiếu đủ ID0–29 với nội dung cần hoàn thiện. **Nguồn/nơi dùng trong hai cột cuối là thiết kế đích, không phải xác nhận code hiện tại đã có tất cả các luật đó.** Độ phủ triển khai lấy từ báo cáo theo phiên bản; không lấy bảng đích làm bằng chứng đã chạy.

Đơn vị hạch toán hiện hành là một đơn vị nguyên của đúng sản phẩm (trong code thường gọi là món); giá là đồng bạc/đơn vị đó, chỉ là giá tham chiếu thử, không phải giá thị trường cố định hoặc giá đã cân bằng. Chưa có quy đổi được chốt thì không tự gọi một món là kg, mét, một liều thuốc hay một con vật. Quy đổi vật lý phải có phiên bản và ảnh hưởng tới recipe/kho/save được kiểm riêng. Các tài nguyên như silica, đất sét hoặc phụ phẩm trong source ledger không tự trở thành hàng có thể mua/bán; nếu cần giao dịch chúng, phải định nghĩa sản phẩm và cấp mã riêng trước.

| ID / khóa hiện có | Tên dùng trong thiết kế | Nhóm hiện có | Giá tham chiếu | Nguồn cần có | Nơi sử dụng cần xác định và triển khai |
|---|---|---|---:|---|---|
| 0 `dry_food` | Lương thực khô | Lương thực khô | 20 | Canh tác/thu hoạch có mùa, tồn đầu kỳ hoặc nhập hữu hạn có nguồn. | Bữa ăn hộ/cá nhân, kho lương và tiếp tế; lượng ăn và hậu quả thiếu phải dùng cùng luật. |
| 1 `supplementary_food` | Thực phẩm bổ sung | Thực phẩm bổ sung | 28 | Seed Press Works có nguồn giới hạn/tái sinh theo luật hiện hành. | Volunteer có depot nhận sau DryFood; Civilian ngoài quân vụ có thể dùng1 món cá nhân sau bữa DryFood thành công để tăng10 nutrition, không tăng health. Trial chưa cân bằng nguồn và giá. |
| 2 `salt` | Muối | Muối | 35 | Làm muối tại nguồn hợp lệ hoặc nhập hữu hạn. | Chế biến/bảo quản thực phẩm và ngoại thương; bán ra không thay cho việc có nhu cầu trong nước. |
| 3 `timber` | Gỗ | Gỗ | 45 | Rừng có lượng khai thác và quy tắc tái sinh. | Giấy, xây dựng và các công thức gỗ được định nghĩa; phân biệt gỗ với nhiên liệu. |
| 4 `fuel` | Nhiên liệu | Nhiên liệu | 18 | Nguồn hoặc chế biến nhiên liệu có tài nguyên và công sức thật. | Luyện/chế tác, lò thủy tinh và lò gốm; công thức phải trừ đúng Fuel. |
| 5 `iron` | Sắt | Sắt | 45 | Khai thác/chế luyện hoặc nhập hữu hạn; chỉ rõ sắt trong catalog là đầu ra ở công đoạn nào. | Công cụ, vũ khí và các dự án nhận đúng vật liệu. |
| 6 `copper` | Đồng | Kim loại màu/quý | 65 | Khai thác/chế luyện tại nguồn đồng hoặc nhập hữu hạn. | Chế tác/vật dụng có công thức nhận Copper; không mặc nhiên thay Iron. |
| 7 `silver` | Bạc hàng hóa | Kim loại màu/quý | 100 | Khai thác/chế luyện tại nguồn bạc hoặc nhập hữu hạn. | Chế tác và trao đổi kim loại; không tự cộng tiền đồng bạc từ hàng Silver. |
| 8 `gold` | Vàng hàng hóa | Kim loại màu/quý | 120 | Nguồn khai thác/chế luyện hoặc đối tác có hàng hữu hạn được định nghĩa. | Chế tác hàng quý hoặc nhu cầu tổ chức đã xác định; không tự tạo tiền. |
| 9 `fiber` | Xơ sợi | Xơ sợi | 14 | Cây/nguồn sợi hoặc chăn nuôi phù hợp, mùa và công đoạn tạo sợi. | Dệt Cloth và công thức nhận đúng Fiber. |
| 10 `beeswax` | Sáp ong | Xơ sợi, phân nhóm kế thừa | 40 | Nuôi/thu hoạch có địa điểm và quyền hợp lệ; rừng tranh chấp không tự được cấp quyền. | Nến, vật dụng hoặc dịch vụ dùng sáp theo recipe riêng; cùng nhóm không cho phép thay Fiber. |
| 11 `cloth` | Vải thường | Vải | 35 | Dệt từ Fiber và công sức hợp lệ. | Nhu cầu trang phục và recipe quân nhu; xác định mức dùng, không coi chuyển kho là tiêu dùng. |
| 12 `silk` | Lụa | Vải | 120 | Chuỗi tơ/dệt hoặc nhập hữu hạn, kèm quyền cơ sở/địa điểm. | Trang phục chất lượng cao hoặc nhu cầu tổ chức/hộ có ngân sách; không alias Cloth. |
| 13 `livestock` | Gia súc | Gia súc/ngựa | 50 | Chăn nuôi, thức ăn, chỗ nuôi và vòng đời; đơn vị/con phải được quy định. | Thực phẩm/phụ phẩm hoặc sức kéo theo luật cụ thể; không dùng thay Horse. |
| 14 `horse` | Ngựa thường | Gia súc/ngựa | 90 | Chăn nuôi hoặc nhập hữu hạn, với thức ăn/chỗ nuôi. | Đầu vào huấn luyện WarHorse; công dụng vận tải/sức kéo cần luật riêng. |
| 15 `war_horse` | Ngựa chiến | Gia súc/ngựa | 200 | WarhorseStable huấn luyện từ Horse và Work. | Inventory cá nhân rút ngắn Travel/Migrate Atlas một ngày, giảm2 đồng khi đường ban đầu dài hơn một ngày; ngựa bền, route đóng vẫn chặn. Chưa có campaign/chiến mã đơn vị. |
| 16 `tools` | Công cụ | Công cụ | 65 | Chế tạo từ Iron và Fuel theo recipe đã duyệt. | Sản xuất/quân nhu/dự án; hao mòn chỉ áp dụng khi có quy tắc, không tự trừ mỗi tick. |
| 17 `military_supply` | Quân nhu chế tạo | Quân nhu/vũ khí | 90 | Chế tạo từ Tools và Cloth theo recipe đã duyệt. | Cấp phát/tiêu dùng quân vụ theo chức năng riêng; không thay DryFood hoặc Weapon. |
| 18 `weapon` | Vũ khí | Quân nhu/vũ khí | 110 | Xưởng rèn dùng Iron, Tools và công sức thật. | Trang bị, bảo trì/tổn thất và giao chiến theo luật quân sự; không tự tạo người tham chiến. |
| 19 `construction_material` | Vật liệu xây dựng | Vật liệu xây dựng | 55 | Chọn rõ loại vật liệu/tổ hợp và công đoạn tạo sản phẩm. | Dự án xây dựng nhận đúng mã và lượng; không bao gồm mọi vật liệu trong nhóm một cách ngầm định. |
| 20 `paper` | Giấy | Giấy | 60 | Paper Mill dùng Timber và công sức theo recipe. | Học tập, hành chính hoặc sao chép thông tin có định mức; không coi test di sản là nhu cầu giấy. |
| 21 `medicine` | Dược liệu/thuốc | Dược liệu | 130 | Xác định sản phẩm, thu hái/trồng/chế biến hoặc nhập hữu hạn. | Dịch vụ điều trị có người thực hiện, đối tượng, lượng dùng, thời gian và giới hạn hiệu quả; không cộng health chỉ vì đang sở hữu Medicine. |
| 22 `tea` | Trà | Hàng xa xỉ | 100 | Tea Garden có mùa và nguồn hữu hạn/tái sinh được quy định. | Nhu cầu có khả năng chi trả, định mức và tác dụng đã xác định; tồn kho không tự tạo cầu. |
| 23 `incense` | Hương/trầm | Hàng xa xỉ | 160 | Khai thác/chế biến tại cơ sở có nguồn và quyền. | Nhu cầu cá nhân hoặc nghi lễ/tổ chức, với lượng tiêu và chi phí thực. |
| 24 `decorative_glass` | Thủy tinh trang trí | Hàng xa xỉ | 250 | Chế tác có đầu vào và chất lượng riêng; nếu dùng PlainGlass phải ghi rõ recipe. | Nhu cầu trang trí có người mua và ngân sách; không thay kính thường trong xây dựng. |
| 25 `fine_porcelain` | Đồ sứ tinh xảo | Hàng xa xỉ | 300 | Chuỗi nguyên liệu/lò/chế tác có công đoạn và chất lượng riêng. | Nhu cầu đồ dùng cao cấp có người mua và ngân sách; không alias Pottery. |
| 26 `general_luxury` | Hàng xa xỉ tổng hợp | Hàng xa xỉ | 150 | Định nghĩa sản phẩm đại diện hoặc giỏ hàng, nguồn và cách định giá. | Nhu cầu tổng hợp được mô tả rõ; không cộng trùng với Tea/Incense/đồ quý và không dùng làm hàng thay thế tùy ý. |
| 27 `plain_glass` | Thủy tinh thường | Vật liệu xây dựng | 90 | Glassworks Atlas10: nguồn silica hữu hạn, Fuel và Work theo phiên bản hiện có. | PrivateResidenceBuild tiêu đúng1 PlainGlass cá nhân cùng hai ConstructionMaterial và một Tools; luật được đưa vào contract v44. [Evidence](reports/M5.2-plain-glass-housing-use-evidence-2026-10-25.md). |
| 28 `pottery` | Đồ gốm thường | Vật liệu xây dựng | 55 | PotteryKiln Atlas15: nguồn đất sét hữu hạn, Fuel và Work theo phiên bản hiện có. | Pottery cá nhân bền: sau bữa DryFood thành công, Civilian ngoài quân vụ nhận +5 nutrition, tối đa100; không tiêu đồ gốm. [Evidence](reports/M5.2-pottery-meal-vessel-evidence-2026-10-25.md). |
| 29 `leather` | Da thuộc | Gia súc/ngựa, phân nhóm nguồn gốc | 80 | Tannery Atlas17 tạo từ ledger phụ phẩm hữu hạn; chuỗi chăn nuôi/phụ phẩm mở rộng vẫn thiếu. | Một Leather cá nhân cho defender Nam Cảng cộng1 defense bền; không tiêu hoặc mượn. Chưa có áo giáp campaign. |

Các hướng dùng chưa có luật lượng/thời gian/hiệu quả trong bảng là phần thiết kế phải hoàn thiện, không được dùng như command đã chấp thuận hoặc nguồn reward mới. Không tự bổ sung bệnh, nến, trang phục hay công dụng quân sự vào runtime chỉ từ một dòng bảng. Trước khi code cơ chế tương ứng, điền đầy đủ hồ sơ ở §22.4, quyết định thử có phiên bản nếu cần và các phép kiểm; giữ phạm vi/quyền/consent đã chốt. Các con số giá ở đây được đối chiếu với catalog khi bổ sung tài liệu; thay giá sau này phải cập nhật nguồn chuẩn và ghi phiên bản, không sửa báo cáo run cũ.

### 22.6 Danh mục đối tượng và liên kết phải chuẩn bị cùng hàng hóa

Không chỉ lập danh sách tên. Mỗi nhóm sau phải có bản ghi định nghĩa và tham chiếu tới nhóm còn lại; sử dụng các ID/type hiện có khi đã có, không tạo một hệ ID song song trong tài liệu:

| Nhóm định nghĩa | Chi tiết phải chuẩn bị | Đối tượng thực trong thế giới và liên kết |
|---|---|---|
| Tài nguyên và địa điểm | Loại nguồn, đơn vị, trữ lượng/khả năng hồi, mùa, vùng/site, điều kiện tiếp cận và giới hạn khai thác. | Nguồn tại địa điểm cụ thể; lượng đã khai thác, phần còn lại, người có quyền. |
| Công thức và ngành cơ sở | ID recipe/ngành, GoodId đầu vào/đầu ra, công sức, kỹ năng, capacity, hao hụt, thứ tự quyết toán và trường hợp thiếu. | Establishment có chủ, kho, nguồn vốn và tiến độ; recipe tham chiếu catalog hàng thay vì chép tên tùy ý. |
| Nghề và hoạt động | Điều kiện làm nghề, năng lực cần, nơi làm, lịch, công việc đủ điều kiện, tiền công và ảnh hưởng cơ thể. | Person/hợp đồng/phiên hoạt động; tách tên nghề khỏi ca Work thực được cấp chú ý. |
| Nhu cầu và dịch vụ | Ai có nhu cầu, lượng/cadence, ngân sách, nhà cung cấp, đầu vào hàng/thời gian, kết quả và hậu quả không được phục vụ. | Nhu cầu hoặc hồ sơ dịch vụ gắn đúng người/hộ/tổ chức; giữ tiền và tiêu vật tư đúng một lần. |
| Cơ thể, bệnh và chăm sóc | Chỉ số/đơn vị, nguyên nhân, tiến triển, điều kiện nhận chăm sóc, nguồn thuốc/công và giới hạn hồi phục. | Body/hồ sơ riêng từng người; nối dịch vụ, hoạt động và Fact, không chép chỉ số giữa cá thể. |
| Hộ, đất và tài sản | Loại chủ thể, sở hữu/sử dụng/giám hộ, đồng sở hữu, quyền chuyển giao, nghĩa vụ và di sản. | Các tài khoản/title/holding thực; liên kết bằng ID, không nhân tiền hay lấy quỹ hộ làm tài sản riêng. |
| Tổ chức và chức vụ | Loại tổ chức, điều kiện thành viên, chức/quyền đại diện, chọn/bãi nhiệm, quỹ và nghĩa vụ. | Tổ chức/thành viên/người giữ chức thực; tài sản tổ chức tiếp tục thuộc tổ chức khi người đại diện đổi. |
| Đô thị và tuyến | Loại site, vùng cha, hình học/đơn vị, cư trú thật, authority; tuyến có hướng, thời gian, capacity, phí và điều kiện đi qua. | Census đếm Person sống đúng một lần; route runtime dùng cấu hình thế giới, không suy từ một nét vẽ bản đồ. |
| Quân sự và ngoại giao | Loại lực lượng, consent, chỉ huy, trang bị, lương, tiếp tế, điều kiện khởi xướng, đối thủ và các kết quả hợp lệ. | Roster, order, cam kết và hồ sơ trận/đối tác; người/hàng/tiền có nguồn, thương vong nối vòng đời và di sản. |
| Năng lực, tính cách và tri thức | Mã/ý nghĩa/thang đo, nguồn khởi tạo, thông tin biết được và cơ chế sử dụng; tách dữ liệu cá nhân khỏi catalog chung. | Giá trị riêng của từng Person, Knowledge có nguồn, memory riêng; policy chỉ nhận dữ liệu được phép. |
| Command, sự kiện và UI | Actor, tư cách, tham số, điều kiện hợp lệ, kết quả thành công/từ chối, Fact và quyền xem/độ trễ. | Command đi qua C++ transaction; UI/Python dùng cùng định nghĩa và không trực tiếp viết số dư/cơ thể. |

Mọi recipe, nhu cầu, dịch vụ và hoạt động phải đối chiếu thành một chuỗi có thể giải thích: **nguồn → người có quyền → công việc/chi phí → sản phẩm/dịch vụ → người sử dụng → hệ quả**. Thiếu một mắt xích thì ghi đúng phần thiếu; không giải quyết bằng hàng miễn phí, consumer giả hoặc một class không có hành vi. Catalog trong source là nơi định danh dùng khi chạy; tài liệu này quy định nội dung đích và quyết định; report giữ bằng chứng của phiên bản đã kiểm. Không dùng bảng lịch sử để chứng nhận source mới.

Khi bổ sung một đối tượng thông thường đã nằm trong cơ chế chung, ưu tiên thêm dữ liệu và phép kiểm có ý nghĩa. Khi cần hành vi mới, định nghĩa trách nhiệm class/thành phần, quyền sở hữu dữ liệu và điểm nối vào command/time/save trước khi viết code. Chỉ gọi nhóm hoàn tất khi cả định nghĩa lẫn hành vi liên quan đã được kiểm đúng phạm vi; lập sẵn tên/class không hoàn tất M5 và không tự mở lại RL.

## 23. Cá thể, cơ thể, năng lực, tính cách và điều được biết

### 23.1 Dữ liệu cá thể và quyền quyết định

Person giữ ID bền, tuổi/sống-chết, vị trí/đích đến, hộ/nghề/chức, tài sản và nghĩa vụ, năng lực, body, tính cách và các tham chiếu liên quan. Giá trị của hai người phải độc lập dù dùng chung trọng số model. Memory và bộ lấy mẫu quyết định thuộc từng agent; không sao chép memory người khác khi đổi vai trò hoặc tải save. Luật khởi tạo phải ghi endowment và phân bố có seed, không dùng sự thiếu hụt về sau để tự bơm tài sản.

Command ghi rõ actor, tư cách hành động, đối tượng, tham số và sequence. Được nhìn một hồ sơ không tự có quyền dùng tiền hay giao việc cho người đó. Person chết phải ngừng nhận quyết định tự chủ mới; các nghĩa vụ, hàng đang đi và tài sản của người đó được chuyển sang quy trình phù hợp thay vì bị xóa.

### 23.2 Năng lực: dữ liệu cá nhân khác danh mục chung

Danh mục capability định nghĩa ý nghĩa ID; giá trị capability thuộc từng Person; scenario chọn schema nào được sử dụng. Một dimension là một chiều năng lực cụ thể, ví dụ ngoại giao. Scenario là cấu hình tình huống/thế giới khởi đầu. Cùng một danh mục không có nghĩa mọi người có cùng giá trị. Mỗi chiều mới phải có nguồn khởi tạo, đơn vị, miền, luật gameplay tiêu thụ và phép đo ảnh hưởng; thêm tên trong catalog không chứng minh năng lực đã có tác dụng.

| ID đã có trong catalog T4.9 | Giá trị và cách sử dụng đã được mô tả |
|---|---|
| strategy | Điểm kế thừa từ 0 đến 100; ảnh hưởng mức học kỹ năng sau đợt học hoàn tất. |
| administration | Điểm kế thừa từ 0 đến 100; dùng trong thời gian báo cáo và thực hiện dự án theo luật hiện hành. |
| economic_planning | Điểm kế thừa từ 0 đến 100; ảnh hưởng thời gian chuẩn bị dự án, không tạo tiền/vật tư. |
| diplomacy | Điểm kế thừa từ 0 đến 100; ảnh hưởng chuẩn bị thỏa thuận/đề nghị, không ép người khác đồng ý. |
| investigation | Điểm kế thừa từ 0 đến 100; ảnh hưởng hạn điều tra, không cấp mọi bí mật. |
| military | Điểm kế thừa từ 0 đến 100; tiêu thụ bởi luật huy động, không thay tiếp vận thực. |
| labor_skill | Đọc Person::skill trong miền 0–100; luôn hoạt động trong catalog, dùng cho quy đổi công sức xưởng ở T3.9. |

T4.5 cho scenario chọn tập không rỗng của sáu dimension legacy. Dimension bị bỏ vẫn được cất giá trị cũ nhưng luật và observation nhận trung tính50/0,5. Model contract có hậu tố mask tương ứng, model mask khác bị từ chối dù shape không đổi. labor_skill không nằm trong sáu bit bật/tắt đó. Đây là bước chuyển tiếp, chưa phải schema cho phép thêm tùy ý mọi nghề hoặc TechnicalSkill. [Report T4.5](reports/M5-T4.5-configurable-legacy-capability-schema-evidence-2026-10-22.md), [catalog T4.9](reports/M5-T4.9-capability-catalog-metadata-evidence-2026-10-22.md).

Trước khi mở rộng schema tiếp, phải xác định dimension nào dùng chung và dimension nào theo nghề, quy tắc scenario thêm/bỏ ID, ánh xạ save/model và cách xử lý dimension chưa biết. Không mặc định một điểm kỹ năng xưởng đại diện cho y học, quân sự hoặc kỹ thuật mọi ngành. Nếu thêm học tập/kinh nghiệm, chỉ cấp tiến bộ từ công việc đủ điều kiện và chỉ một lần; phương án XP ở phần20 không tự trở thành luật bắt buộc.

### 23.3 Tính cách và phép chứng minh cá nhân hóa

Sáu trục sản phẩm ở §20.2 là tham vọng, vụ lợi tài sản, quan tâm cộng đồng, trọng cam kết, chấp nhận rủi ro và kiên nhẫn. T3.13 dùng số nguyên0–100 trong save và chia100 khi đưa vào observation, nên không mâu thuẫn với thang0–1 của thiết kế. Khởi tạo thử25–75 bằng hash seed/person/trait ổn định, không tiêu RNG đang dùng cho các quá trình khác; save cũ về50 thay vì đoán lại. Trait nội bộ của NPC không được chiếu nguyên vẹn cho vua chỉ vì policy của NPC cần biết bản thân. [Plan](design/M5-T3.13-personality-observation-plan.md), [report T3.13](reports/M5-T3.13-personality-observation-evidence-2026-10-22.md).

Đưa sáu số vào observation chỉ chứng minh đường dữ liệu. Để xác nhận tính cách ảnh hưởng lựa chọn, đánh giá ghép cặp phải giữ cùng tài sản, thông tin, năng lực và tình huống, chỉ thay một trục; đo thay đổi quyết định và kết quả trên nhiều seed. Không sửa engine để ép hành vi mong muốn thay policy. Mục tiêu vai trò, quan hệ từng cặp, cảm xúc ngắn hạn và tính cách bền phải có dữ liệu riêng; không suy trung thành cố định từ một trait.

### 23.4 Chú ý, hiện diện, công sức và body

Tick là một bước thời gian mô phỏng; profile timeline hiện dùng 64 tick cho một ngày game. Nó không có nghĩa là một giờ ngoài đời. Body là nhóm dữ liệu cơ thể của từng người, gồm nợ ngủ, gắng sức, sức khỏe và các mốc/phần dư giúp tính tiếp đúng sau khi tải save.

| Đại lượng | Ý nghĩa, đơn vị và ranh giới |
|---|---|
| Presence/attendance | Tick người còn sống, không đang di chuyển và ở đúng vùng/xưởng của Work. Tick trùng không cộng hai lần. Dùng để xác định có mặt/lương theo luật, không tự chứng minh có sản phẩm. |
| Delivered attention | Lượng chú ý ActivityEvaluator thực cấp, tối đa1.000 đơn vị/tick trong trial hiện hành. Phiên đã đặt nhưng chưa chạy không có lượng được cấp. |
| Công sức Work thô | Tổng delivered attention hợp lệ theo người/xưởng. Là nguồn cho gắng sức; Generic không được coi là lao động xưởng. |
| Công sức sản xuất quy đổi | Tổng attention×(150+skill), mẫu200. Chia tiếp theo ticks_per_unit×1.000 để xác định work unit; giữ số lẻ bằng tổng tích lũy. |
| Nợ ngủ | Số ngày quy đổi nguyên, kèm sleep balance/phần dư đã quy định ở T3.2; không phải đồng hồ số giờ thiếu ngủ ngoài đời. |
| Gắng sức | Điểm thử0–100: 16.000 attention Work thô tăng1; 16 Rest tick thực giảm1. Lưu mốc đã xử lý và phần dư; bão hòa xóa phần dư hướng vượt cận. |
| Sức khỏe | Miền0–100; các nguồn làm giảm/tăng phải có nguyên nhân. T3.10 chặn số âm cả khi tiến ngày và migration. |

Ví dụ T3.9: cùng16.000 attention thô, skill0/50/100 tạo lần lượt12.000/16.000/20.000 đơn vị attention sản xuất quy đổi. Cả ba vẫn có cùng công sức thô để cập nhật gắng sức. Khi đổi skill giữa hai phiên, mỗi phần được tính bằng skill lúc attention được cấp; không nhân lại toàn bộ lịch sử bằng skill mới. Schema34 migrate lịch sử ở hệ số trung tính1 để giữ output cũ. [Report T3.9](reports/M5-T3.9-personal-work-skill-productivity-evidence-2026-10-22.md).

Nghỉ phải thực sự trôi qua trong game, đúng vùng và đúng khoảng được chấp nhận. T3.12 hồi floor(valid_rest_ticks/16) điểm sức khỏe khi khoảng Rest hoàn tất, nếu còn sống, ở đúng vùng và nutrition≥40; chặn100. Phần lẻ hồi sức khỏe bị làm tròn xuống theo từng khoảng Rest, khác phần dư Rest phục hồi gắng sức được giữ trong BODY_3. Ví dụ hai khoảng8tick có thể giảm1 điểm gắng sức theo phần dư nhưng không tự tạo1 điểm hồi health theo luật từng khoảng hiện hành. Phải giữ sự khác nhau này rõ trong test và hướng dẫn. [Report T3.12](reports/M5-T3.12-rest-duration-health-recovery-evidence-2026-10-22.md).

Chăm sóc bởi người khác, bệnh, chấn thương, tác động dài hạn của nợ ngủ/gắng sức và tuổi thọ đầy đủ còn cần luật cụ thể. Trước khi triển khai phải nêu ai cung cấp dịch vụ, ở đâu, tốn thời gian/vật tư/tiền gì, ai được chấp nhận, cơ thể thay đổi khi nào, thiếu dịch vụ gây hệ quả gì và nguồn Fact. Không gọi việc có trường health/exertion hoặc nhãn tử vong là hoàn tất care/lifecycle.

### 23.5 Tri thức, nguồn và độ tin cậy

Fact mô tả điều đã xảy ra trong thế giới; Knowledge mô tả một người đã nhận thông tin về Fact bằng đường nào. PlayerKnowledge là bản chiếu chỉ đọc sau lọc quyền xem. Danh sách Knowledge không thay danh sách mọi Fact công khai mà observer có thể thấy. ID observer sai phải bị từ chối trước khi lọc, không chuyển thành góc nhìn toàn tri.

Bản ghi điều tra gồm observer ID, Fact ID, ngày nhận, Investigation nguồn, loại và confidence basis points. SecretEvidence là bằng chứng điều tra, chưa phải sự thật được xác minh độc lập. UnknownLegacy là dữ liệu cũ thiếu xuất xứ. Confidence là mức tin cậy biểu diễn bằng phần mười nghìn: 10.000 tương ứng 100%, 100 tương ứng 1%. Giá trị -1 nghĩa chưa đo, không phải 0%; miền biểu diễn hợp lệ là -1 hoặc 0–10.000. Nguồn dương phải thuộc đúng observer, đã hoàn tất và khớp Fact/vùng/mốc thời gian. UI phải diễn giải unknown thay vì tự gán độ tin cậy 100%.

Verified, Estimate và Rumour cần đường tạo dữ liệu riêng: nguồn nào đưa tin, đối tượng/thời điểm được nói tới, phương pháp ước lượng hoặc xác minh, cách mâu thuẫn giữa nguồn được giữ lại và điều kiện cập nhật. Không thêm enum rồi dùng mọi loại như nhau. Nếu dùng confidence như xác suất đúng, cần kết quả đối chiếu độc lập để hiệu chuẩn; thiếu dữ liệu phải để chưa biết. [Report loại thông tin](reports/M5-T4.6-investigation-evidence-knowledge-type-evidence-2026-10-22.md), [bản xem theo người quan sát](reports/M5-T4.7-player-knowledge-view-evidence-2026-10-22.md).

## 24. Hộ, đất, quyền và nghĩa vụ cá nhân

### 24.1 Hộ và tài khoản tách biệt

Hộ có ID, thành viên, nơi cư trú, người phụ thuộc, người có quyền đại diện và quỹ chung. Quan hệ cha/mẹ, hôn nhân, giám hộ và cùng hộ là các quan hệ khác nhau; chuyển hộ không tự viết lại huyết thống hoặc chủ tài sản. Nguồn pháp lý của quan hệ cha/mẹ phải đủ để dùng trong hôn nhân/thừa kế; không đoán từ họ tên hoặc cùng địa chỉ.

Mọi khoản chi phải ghi chủ tài khoản, người ra lệnh, căn cứ quyền, người nhận, số tiền và mục đích. Nếu lệnh được phép dùng cả tiền riêng và quỹ hộ, phải ghi thứ tự/giới hạn chia nguồn đã chốt; thiếu tiền thì từ chối hoặc chi một phần đúng luật của lệnh đó. Hộ không được tiêu tiền của trẻ chỉ vì trẻ là thành viên; người đại diện cũng không biến quỹ hộ thành tiền riêng. Mốc đủ18 năm game phải cập nhật quyền của người từng là trẻ mà không chuyển tài sản hai lần.

Các phép kiểm bắt buộc gồm học phí từ đúng quỹ, thay người đại diện, người đại diện chết/chuyển hộ, thành viên trưởng thành, thiếu tiền, nguồn về đúng0 và save/load giữa nghĩa vụ. Kiểm bằng số dư cụ thể từng tài khoản trước/sau, không chỉ tổng tiền toàn thế giới.

### 24.2 Đất và quyền sử dụng

Holding có ID và vùng; bản ghi quyền phải tách người giữ tài sản, người được sử dụng, người nhận tô, người có quyền chuyển giao và cơ quan có quyền xét xử/thu nghĩa vụ. Giám hộ chỉ quản lý trong phạm vi luật, không thay tên chủ sở hữu của trẻ. Không gán cùng một quyền độc quyền cho hai người trên cùng holding/thời kỳ; quyền không độc quyền cần loại và phạm vi tường minh.

Chuyển title hoặc cấp quyền sử dụng phải đi qua command: kiểm đúng người có quyền, đối tượng còn hợp lệ, phần được chuyển, nghĩa vụ liên quan và consent nếu luật yêu cầu. Lưu/tải phải dựng lại chỉ mục từ registry chính và phát hiện tham chiếu người/holding sai; không dùng index đã lưu làm nguồn sự thật thứ hai. Di sản chứa title nào và quyền nào của người sống vẫn còn phải được ghi riêng. Đối chiếu luật chi tiết tại các working rule ở phần3.

### 24.3 Chức vụ, nghề và cam kết

Tư cách quý tộc, tước kế vị, chức bổ nhiệm, nghề và tài sản không thay thế nhau. Bãi nhiệm trả quyền công và ngân sách được giao về cơ quan theo luật, không lấy tiền riêng của người bị bãi nhiệm. Chức đang trống hoặc chờ công nhận phải hiện là hồ sơ có thể xử lý; không lén bổ nhiệm một NPC bằng chiến lược viết tay.

Hợp đồng việc/học/nghĩa vụ cần bên tham gia, quyền nhận/giao, địa điểm, thời hạn game, nguồn trả, yêu cầu hiện diện/chú ý/vật tư, cách quyết toán và quy tắc thiếu hụt. Cho phép người nhận nhiều cam kết theo luật nhưng không tự tạo thêm thời gian. Khi cùng một tick không thể đáp ứng tất cả, ghi lượng thực hiện và nghĩa vụ chưa đủ; không sửa lịch sử thành đã làm đủ để trả lương hoặc cấp chứng chỉ.

## 25. Nền sản xuất và thị trường có nguồn lực thật

### 25.1 Cơ sở, kho, lao động và sản lượng

Establishment là cơ sở có ID ổn định, vùng, ngành, chủ sở hữu, tiền, kho, capacity và quyền Work. Chủ có thể là cá nhân/cơ quan/tổ chức đúng phạm vi đã có luật. Tạo/fund/chuyển hàng/chuyển tiền phải có nguồn đối ứng; không tạo xưởng hoạt động với vật tư hoặc tiền ngầm. Người đang xem xưởng không tự có quyền rút kho hoặc ngân quỹ.

Chuỗi phải kiểm được: tạo/fund → cấp nguyên liệu → cho phép Work → tạo lịch đúng worksite → hiện diện/chú ý thực → quy đổi công sức → tiêu nguyên liệu/tạo sản phẩm → trả lương/thuế → bán/mua hoặc gửi đi → lưu/tải và tiếp tục. Thời gian có mặt dùng cho luật lương đã chốt; công sức quy đổi dùng cho production; capacity/nguyên liệu vẫn giới hạn lượng sản xuất. Không tự giữ nợ lao động để ngày sau sản xuất bù nếu contract quy định cơ hội cũ bị mất.

Recipe phải có ID, sản phẩm cụ thể, lượng nguyên đầu vào/đầu ra, đơn vị work/capacity, điều kiện và quy tắc watermark đã xử lý. Ba recipe mở đầu là Fiber→Cloth, Iron+Fuel→Tools, Tools+Cloth→MilitarySupply theo lượng ở phần3. Không lấy Cloth thay mọi variant vải hoặc Fuel thay mọi nhiên liệu nếu catalog không có luật đó. Tổng số món có thể đổi qua recipe; phép đối chiếu phải dùng đúng lượng tiêu/sinh, không đòi tổng số món luôn bất biến.

Lỗi cần kiểm gồm sai chủ/quyền, người chết, đang đi đường/khác vùng, Work không được cấp chú ý, thiếu nguyên liệu, capacity đã hết, overflow, trùng/stale sequence và save sau khi đã tạo output nhưng trước mốc tiếp theo. Output và lương không được ghi lần hai sau load. Thế giới bật timeline dùng nguồn craft qua xưởng; tương thích pilot cũ được ghi riêng, không dùng sản lượng nghề/region%7 để chứng nhận nguồn M5.

### 25.2 Đủ nguồn cho 16 nhóm hàng

Danh mục hiện có30 sản phẩm/16 nhóm là dữ liệu nhận diện, không tự tạo30 dây chuyền. [§22.4–22.6](#content-definitions-v1) tập trung hồ sơ cần định nghĩa trước, bảng đủ30 mã hàng và các đối tượng liên quan; không tạo bảng danh mục đích cạnh tranh ở từng plan nhỏ. Với mỗi sản phẩm trong §15.2, bảng triển khai trước code phải chỉ rõ: nguồn đầu kỳ; harvest/khai thác/recipe/nhập khẩu nào tạo mới; địa điểm và quyền tiếp cận; lịch/mùa; lao động, dụng cụ, vật tư; capacity; hao hụt/tiêu dùng; nơi lưu kho và thị trường tiêu thụ. Sản phẩm chưa có nguồn hoặc consumer phải được đánh dấu thiếu; không bù hàng khi kho cạn.

Tiến độ theo sản phẩm: slice Saltworks tại atlas region8 thêm nguồn Salt có đầu vào Fuel, Work, capacity và giao dịch chợ; kết quả: [report M5.2 Saltworks](reports/M5.2-saltworks-core-integration-evidence-2026-10-23.md). Slice Forestry tại atlas region6 thêm nguồn Timber có resource site-held, Work, giới hạn, phục hồi và giao dịch chợ; kết quả: [report M5.2 Timber Forestry](reports/M5.2-timber-forestry-core-integration-evidence-2026-10-23.md). Paper Mill tại region13 nay là consumer gameplay thật của Timber; report toàn chuỗi tại [M5.2 Paper Mill/Timber/Travel](reports/M5.2-paper-mill-timber-travel-evidence-2026-10-23.md). Ba đặc sản PlainGlass/Pottery/Leather đã có producer và đường thị trường generic; report chi tiết tại [M5.2 Atlas specialty](reports/M5.2-atlas-unresolved-specialty-production-evidence-2026-10-23.md). Các lát này chỉ xác nhận từng sản phẩm/chuỗi có nêu; không suy ra đủ30 GoodId đều có nguồn, consumer gameplay và mùa vụ hoặc cả16 nhóm đã hoàn chỉnh.

Đã triển khai ba món Atlas còn thiếu chính xác trong `atlas-specialty-products-v1`: PlainGlass ID27 ở vùng10, Pottery ID28 ở vùng15, Leather ID29 ở vùng17. Glassworks/PotteryKiln tiêu một Fuel thật và một source unit cho mỗi mẻ; Tannery dùng một source unit phụ phẩm hữu hạn, không giết/xóa cá thể. Mỗi mẻ cần16 tick Work; mỗi địa điểm có100 source unit ban đầu, không tự hồi trong phiên bản này. Giá tham chiếu lần lượt90/55/80 đồng bạc một món là giả định thử, chưa hiệu chuẩn. Ba GoodId không thay thế nhau và không ánh xạ sang thủy tinh trang trí/đồ sứ tinh xảo. Người sở hữu có thể rút sản phẩm bằng lệnh hàng hóa hiện hành rồi bán trong chợ; tên, kho, nhiên liệu, source ledger và giá trial được UI hiển thị. Realm lên save55/model-v31, actions282/observation3766, household-v33/timeline-v35; save54 đọc tiếp với ba slot hàng mới bằng0. C++ Release CTest phụ thuộc15/15, Python specialty49/model contract20/config49 (7 checkpoint), native runtime CTest1/1, GDExtension build, Godot editor scan và UI/native/specialty smoke đều đã qua theo report nhóm. Không chạy RL. [Plan đầy đủ](design/M5.2-atlas-unresolved-specialty-production-plan.md); [report đầy đủ](reports/M5.2-atlas-unresolved-specialty-production-evidence-2026-10-23.md).

Đã triển khai và kiểm Copper (`GoodId::Copper`, ID6; region7) cùng core refactor ledger từ `forest_units` sang `source_good`/`source_units`. Trial đang chạy: 100 ore unit, trần100, không hồi; 1 Copper sau16 tick Work và trừ1 ore. Schema38/`ESTABLISH_8` migrate save schema37 Forestry sang Timber source không đổi lượng. Contract realm-v14, household v12, timeline v14; action138/observation1784. C++ production61, Forestry28, Mining26, Realm18.993, Schedule3.720, payroll138+5 migration, gameplay config47 qua; Python model20, Saltworks23, Forestry21, Mining20 và workshop policy qua; native48 qua và từ chối model v6; GDExt target build qua. Report đầy đủ: [M5.2 Copper source ledger](reports/M5.2-copper-mining-shared-source-ledger-evidence-2026-10-23.md), plan đã duyệt và kết quả: [Copper plan](design/M5.2-copper-mine-source-plan.md). Không có consumer riêng cho Copper.

Đã triển khai Silver (`GoodId::Silver`, ID7) tại atlas region23 bằng Mining/source ledger chung. Trial100 ore unit, không hồi, Work16 ticks đổi1 Silver; save Realm38 giữ nguyên, contract realm-v15/household v13/timeline v15, action138/observation1784 không đổi. Core Release: Silver19, Production61, Copper26, Forestry28, Realm18.993, Schedule3.720, payroll138+5 migration, GameplayConfig47, transfer217; Python Silver22, model20, Saltworks23, Forestry21, Copper20, GameplayConfig49/7 parity, Clock175/7, transfer32/7, market23; native48 từ chối bundle v6; GDExt build. [Silver plan](design/M5.2-silver-mining-source-plan.md), [report](reports/M5.2-silver-mining-region23-evidence-2026-10-23.md). Cập nhật 2026-10-25: Silver nay có consumer gameplay được nối ở `silver-social-gift-trial-v1`; Civilian sở hữu Silver tiêu1 món và2 đồng để tăng tối đa10 điểm giao lưu, bận7 ngày. Candidate26, contract v43; chuỗi mining/Work/market/order/consumer/save-load và adapter mới đã qua theo [report consumer](reports/M5.2-silver-social-consumer-evidence-2026-10-25.md). Không coi Silver là tiền hoặc mức giá trị đã cân bằng.

Paper Mill dùng Timber ở region13: Forestry region6 tạo Timber; chuỗi đã dùng rút hàng, thị trường, shipment, Travel đến region13, tạo Paper Mill, cấp Work, tạo Paper rồi bán Paper. Hành vi, quyết định thử và phạm vi kiểm được ghi tại [report M5.2 Paper Mill/Timber/Travel](reports/M5.2-paper-mill-timber-travel-evidence-2026-10-23.md) và [plan](design/M5.2-paper-mill-timber-consumer-plan.md). Travel/Migrate giữ action0–140, thêm vùng7–26 tại141–160; tổng161 hành động/2060 observation; Realm27 vùng đếm chặng trên route quốc gia đang mở và từ chối khi không có đường. Model realm-v17, household-v15, timeline-v17; save schema39 giữ nguyên. Phí `4 + 2 × số chặng`, thời gian `số chặng + 1` cùng các hệ số Forestry/Paper vẫn là working assumptions có phiên bản, chưa được hiệu chuẩn.

Atlas region21 định danh Beeswax (`GoodId::Beeswax`, ID10) nhưng mô tả quyền sử dụng rừng tranh chấp. Do luật quyền/consent không được đổi để cấp quyền khai thác mặc định, chưa chọn region21 làm nguồn trong lát kế tiếp; khoảng trống này được giữ nguyên trong audit. Lát kế tiếp được chọn là Tea (`GoodId::Tea`, ID22) tại atlas region14, nơi atlas nêu sản phẩm cụ thể cùng mùa vụ và phí đường. Plan trước code phải định nghĩa rõ mùa, đơn vị/tái tạo, quyền, Work/capacity, đường giao dịch, migration và consumer. Tea chỉ được tạo trong nguồn được kiểm soát, không tự sinh từ nhãn atlas. Trạng thái hiện tại nằm ở CURRENT_TASK.

`tea-garden-trial-1` chỉ cho tạo một Tea Garden tại region14. `tea-harvest-unit-trial-1` dùng source ledger đơn vị trừu tượng, bắt đầu25, trần100; mỗi kỳ 90 ngày hoàn tất cộng25 tới trần. `tea-season-cycle-trial-1` định nghĩa mốc day90/180/270/360 cho bốn kỳ vụ trial, không sửa quy ước 365 ngày tuổi/thân thể và không gọi đó là lịch khí hậu đã được chứng minh. `tea-recipe-trial-1` dùng một source unit +16 Work tick hợp lệ → một Tea, capacity hiện tại tối đa một batch/site/ngày, không thêm Fuel/Tools/Water hay hàng thay thế. Đây là hệ số để kiểm nguồn, mùa, save và market; chưa phải cân bằng năng suất. Tea Garden dùng loại chủ sở hữu Establishment, funding, quyền quản lý cơ sở, Work authorization, stock và market hiện hữu; không thêm LandTitle/LandUseRight/consent mới hay quyền đất từ atlas label. GoodId::Tea có thể được agent mua qua lệnh market cụ thể; chưa có tiêu dùng xa xỉ tự động. Vì ID ngành mới là8, save cần nâng39/`ESTABLISH_9`→40/`ESTABLISH_10`; giữ action0–160, thêm candidate161–163, tổng164 action và2096 observation; model realm-v17→v18, household v15→v16, timeline v17→v18. Chi tiết trước code: [Tea Garden plan](design/M5.2-tea-garden-source-plan.md).

Tea Garden hoàn tất: source theo chu kỳ, production qua Work, migration/save, market command và các target phụ thuộc được kiểm trên source hiện tại. Report ghi đầy đủ 28 Tea Garden checks, Production66, Realm18.993, các regression C++/Python, native48, GDExt build và giới hạn chưa có Godot runtime/RL/holdout/performance/package: [Tea Garden region14](reports/M5.2-tea-garden-region14-evidence-2026-10-23.md). Đây là một nguồn Tea thử, chưa chứng minh mùa/năng suất ngoài đời và chưa thêm nhu cầu xa xỉ tự động.

Incense Works tại region11 đã hoàn tất: site dùng nguồn100 unit trừu tượng không hồi; một unit +16 Work tick tạo một Incense; không thêm luật quyền rừng hoặc nhu cầu xa xỉ. Save40/ESTABLISH_10→41/ESTABLISH_11; action164–166, tổng167/2132; contract realm-v19, household-v17, timeline-v19. C++ Incense21, Tea28, Production66, Realm18.993 và các regression nêu trong report pass; Python binding/replay các producer và runtime contract48 pass; GDExtension build pass. Report nêu mọi target/count, lỗi ban đầu và giới hạn Godot/RL/holdout/performance/package: [Incense Works report](reports/M5.2-incense-works-region11-evidence-2026-10-23.md); [plan](design/M5.2-incense-source-plan.md).

Mùa vụ và tài nguyên phải có dữ liệu vùng, kỳ đã xử lý và lượng còn lại/tái tạo khi luật cần. “Đặc sản vùng” là lợi thế có cơ chế nguồn/chi phí/chất lượng cụ thể; tên đặc sản trong atlas không phải sản lượng đã mô phỏng. Tàu là tài sản vận tải, không tự gán vào một nhóm hàng để vượt kiểm đếm. Công thức mới cần ví dụ tính tay và phép kiểm không thay thế variant.

Học nghề cần người học, bên nhận, người dạy, nơi học, năng lực dạy hữu hạn, chi phí và tiến độ thực. Setting thử ban đầu `formal-apprenticeship-v1` được điều chỉnh riêng học phí trong `formal-apprenticeship-v2`; giữ bằng chứng của v1 và dùng v2 làm setting hiện hành. Không suy hiệu quả hay học phí tối ưu từ lương công nhân hoặc XP cũ.

### 25.3 Học nghề theo hợp đồng tại xưởng

Setting thử hiện hành `formal-apprenticeship-v2` đóng khoảng trống học nghề M5.2 bằng hợp đồng hai bên nối người học, người dạy và xưởng. Chủ xưởng tư từ18 tuổi với kỹ năng lao động ít nhất40/100 có thể mời một Civilian từ18 đến dưới31 tuổi, cùng vùng, kỹ năng dưới80/100. Người học tự nhận hoặc từ chối trong7 ngày game. Khi nhận,10 đồng bạc chuyển từ khoản tiền được phép dùng của người học vào tài khoản xưởng và người học được cấp quyền Work trong30 ngày. Mỗi xưởng, người dạy và người học chỉ có thể có một đề nghị/khóa học đang mở. V1 đã thử mức20; sau30 ngày game candidate bị từ chối vì người học cùng vùng có16 đồng. V2 dùng10 để lời mời có thể thực hiện trong ca này. Đây là setting gameplay thử; chưa được xem là học phí hay giới hạn tối ưu.

Một ngày thực hành chỉ được tính nếu cả hai người hiện diện đủ64 tick Work ở đúng ID xưởng trong cùng ngày game. Đạt10 ngày chung trong thời hạn sẽ tăng10 điểm kỹ năng lao động, tối đa100. Nếu hết30 ngày chưa đạt thì khóa kết thúc không cấp kỹ năng. Một bên có thể kết thúc sớm; người chết, chuyển vùng hoặc mất quyền sở hữu xưởng sẽ làm khóa bị gián đoạn. Học phí không hoàn sau khi bắt đầu; lương Work tiếp tục theo sổ lương đã có. Quyền làm việc được mở đúng theo hợp đồng và không thể dùng vùng khác/xưởng khác làm bằng chứng thực hành.

Kích thước tensor giữ382 state/400 action/5.182 observation/reward14, nhưng candidate slot32–70 của Civilian và44–70 của Economic được dùng cho đề nghị/kết thúc học nghề; slot71 vẫn là lệnh hội đồng. V1 đổi contract `realm-v41`→`realm-v42`, config `v52`→`v53`; v2 đổi tiếp `realm-v43`/config `v54` vì giá và legal mask của offer đổi. Save nâng74→75 và lưu ID hai người, ID xưởng, thời hạn, trạng thái, số ngày cùng tick hiện diện đã quan sát để load giữa khóa không cấp lại tiền hay tính lại ngày. Học trường công `Study` tiếp tục chạy độc lập. [Plan v1](design/M5.2-formal-apprenticeship-plan.md), [quyết định v2](design/M5.2-formal-apprenticeship-tuition-v2-plan.md).

#### Kết quả v1 và điều chỉnh v2

V1: C++79/79, full Release CTest65/66 (lỗi hash save v14 ở check434), Python contract/smoke và Godot snapshot smoke qua; Godot lúc đó chưa chạy offer/accept. Chi tiết giữ nguyên trong [evidence report v1](reports/M5.2-formal-apprenticeship-evidence-2026-10-25.md).

V2 giảm học phí20→10 vì seed444438 cho thấy sau30 ngày Study người học còn16, khiến candidate v1 không hợp lệ. C++ test chuyên biệt82/82; người có đúng10 available funds nhận lời, sổ xưởng tăng đúng10, thiếu10 bị từ chối. Gameplay config51, Python model contract21; Release GDExtension/Python binding build; Godot action-flow smoke19/19 và worksite/UI smoke32/32. Smoke Godot gửi candidate và Accept qua `realm_command` với fingerprint thật rồi xác nhận active course/snapshot/sổ xưởng. Không thao tác chuột cửa sổ; chưa phải usability test. Save vẫn75, contract realm-v43/config-v54; tensor không đổi. `polyec_realm_tests` vẫn dừng check434 ở cùng hash lịch sử v14; không sửa oracle. V2 chưa qua full CTest lại; không có RL/performance/package. Kế hoạch trước code và report v2 ghi rõ quyết định cùng giới hạn: [plan](design/M5.2-formal-apprenticeship-tuition-v2-plan.md), [report](reports/M5.2-formal-apprenticeship-tuition-v2-evidence-2026-10-25.md).

### 25.4 Đơn mua, giá và sổ cầu

Đơn mua phải chỉ rõ người mua/nguồn tiền, vùng, GoodId, số lượng, giá tối đa, hạn, loại nhu cầu hợp lệ và tiền giữ. Chỉ nhu cầu có khả năng thanh toán được đưa vào lượng cầu thanh toán; tự ghi “thiết yếu” không tạo ưu tiên. Hủy trước quyết toán hoàn phần tiền còn giữ đúng nguồn; khi hết hạn, phần chưa mua ghi thiếu cầu đúng một lần theo quy tắc đã duyệt.

Phân bổ tuân theo các vòng ở §15.4 và cách làm tròn §15.5; tổng giao không vượt tồn kho, tổng chi/thuế/hoàn không vượt tiền đã giữ. Hai đơn cùng chủ vẫn là hai nghĩa vụ có ID; không bỏ một đơn khi giới hạn sức mua hoặc kho. Khi chủ chết, tiền giữ theo hồ sơ di sản; không mất hoặc hoàn cho chủ thể khác.

Sổ tháng giữ D=cầu hợp lệ, U=thiếu cầu, S=kho cuối tháng; cập nhật giá một lần theo công thức và cận ở §15.5. Test cần D=0, hàng không đủ, giá trần/sàn, giá nguyên nhỏ không đổi do làm tròn, đúng ranh giới tháng, pause và save/load. Ba kịch bản bắt buộc: đủ hàng/đủ tiền; thiếu hàng dù có tiền; có hàng nhưng người mua không đủ tiền. Phải phân biệt nguyên nhân thay vì gọi chung là “thị trường thất bại”.

## 26. Vận tải: sở hữu hàng, hàng chờ và thời gian thực hiện

### 26.1 Tuyến, phí và một lô hàng

Atlas giữ31 liên kết vật lý và27 ID vùng; cấu hình vận tải ghi từng hướng được phép. Sông chỉ có chiều đã chốt; hai hướng của một liên kết dùng chung capacity ngày. Thời gian, phí và capacity thử có đơn vị/miền ở §15.5; không chuyển số km giả định thành luật mới. Mỗi hướng có ngân khố nhận phí tồn tại và hợp lệ.

Shipment phải giữ ID, chủ/người hưởng lợi, loại/lượng hàng ban đầu, nguồn/đích, tuyến đã chọn, phí còn giữ, lượng đã giao/hủy và danh sách lô. Mỗi lô có lượng dương, vị trí đang chờ hoặc cạnh đang đi, chặng kế tiếp và mốc ngày hợp lệ. Hàng đang đi không đồng thời còn trong kho người gửi hoặc kho vùng. Tổng hàng chờ+đang đi+đã giao+đã hủy phải đối chiếu với lượng gốc và giao dịch liên quan.

Tạo lệnh trừ hàng/giữ phí cùng một lần. Tải lên cạnh mới trả phí từ phần giữ tới ngân khố; đến nơi mới nhập kho đúng người hoặc di sản. Hủy chỉ lấy phần chưa tải đang ở cùng vùng với người gửi và hoàn phần phí chưa chi của lượng đó; lô đang đi tiếp tục tới nút kế. Đóng tuyến không xóa lô; mở lại không thu lặp phí. Người chết không làm hàng teleport: di sản chờ các chuyến liên quan hoàn tất/hủy hợp lệ trước khi phân phối phần phụ thuộc.

Test nhiều chặng phải ghi riêng ngày nhận lệnh, ngày tải, arrival_day lưu, ngày pipeline nhận lô và ngày lô có thể tải chặng tiếp. Dùng đúng thứ tự pipeline ở §15.5; không đổi cách gọi “đầu/cuối ngày” trong report để che lệch một ngày. Save giữa chặng phải cho cùng lịch tải, phí và hàng giao với chạy liền; ngày đến/reference sai phải bị từ chối trước khi thay Realm đích.

### 26.2 Ước tính đường đi khác lịch giao thật

T3.8 cho Fastest tính lượng hàng đang chờ tại các liên kết khi ra lệnh, chung cả hai chiều. Ở cạnh sau, phép ước tính trừ lượng queue có thể được phục vụ trong thời gian chuyến mới đi qua các cạnh trước. LowestFee ưu tiên tổng phí, dùng thời gian theo quy tắc phụ đã chốt. Cùng lượng hàng mới phải được giữ cố định khi so trường hợp có/không có queue; khác lượng là hai bài toán khác.

Ước tính hiện không biết lô đang đi sẽ đến queue sau đó, chuyến tương lai, route đổi sau snapshot hoặc hạn chế tài chính làm hàng cũ không tải được. Vì vậy phải ghi phạm vi dự báo và không hứa ngày giao chắc chắn. Predictor đầy đủ nếu triển khai cần nguồn reservation theo thời gian, cách xử lý cập nhật/hủy và phép kiểm sai số; không chỉ đổi nhãn Fastest. [Bằng chứng T3.8](reports/M5-T3.8-visible-transport-queue-evidence-2026-10-22.md).

Chuỗi tích hợp phải có mua→gửi→tiến ngày→phí→giao→save/load, hai chiều tranh capacity, nhiều chặng, đóng/mở tuyến, hủy sau tải một phần và chủ chết. Python gọi cùng command C++, không có luật routing/trừ tiền riêng. UI và policy Atlas cần view/action hợp lệ cho27 vùng; API quyết định7 vùng không được âm thầm nhận profile Atlas.

## 27. Tổ chức, di sản và quốc gia vận hành

### 27.1 Tổ chức và tài sản của người sống/người chết

Gia tộc, guild, thành thị và giáo hội cần ID, thành viên, quỹ/tài sản, chức, quyền đại diện và nguồn nghĩa vụ cụ thể. Không đồng nhất một tài khoản organization hiện có với một tổ chức xã hội đầy đủ. Thay người đại diện không chuyển tài sản tổ chức vào tiền riêng. Consent của thành viên/đối tác phải được kiểm tại lệnh cần consent; quyền đại diện không cho phép vượt luật đó.

**Kết quả `municipal-council-representation-v1`, 2026-10-24:** Atlas27 có 27 record hội đồng theo site ID. Cư dân Urban đủ18 tuổi game tự nộp/rút hồ sơ miễn phí; Sovereign bổ nhiệm hoặc từ chối riêng từng settlement; nhiệm kỳ thử là1.460 ngày game. Đại diện chỉ được ra lệnh xây nhà công bằng quỹ/vật liệu của site đó qua pipeline housing hiện có. Từ chức, bắt đầu di cư, chết hoặc hết hạn dừng quyền chi; save65 migration tạo council trống cho save64 trở về trước mà vẫn giữ municipal fund và108 organization balances. Policy contract là realm-v33/model household-v34, 382 state,336 actions,4.414 observations,reward v14. C++/Python/native/GDExtension/Godot smoke theo phạm vi đã chạy; chưa có live Realm click-through do không có bundle model realm-v33 được huấn luyện, chưa có membership/election hoặc nhà tư. [Plan và lý do chọn luật](design/M5.4-municipal-council-representation-plan.md); [báo cáo kết quả và phép kiểm đầy đủ](reports/M5.4-municipal-council-representation-evidence-2026-10-24.md).

Di sản phải gom đúng tài sản riêng của người mất, khoản phải thu/phải trả, title/xưởng và tài sản đang giữ ở escrow hoặc shipment. Tài sản của phối ngẫu, đồng sở hữu hoặc quỹ hộ không tự nhập toàn bộ vào di sản. Trả nợ từ nguồn hợp lệ trước chia; thiếu di sản không chuyển nợ vô hạn sang con. Tước kế vị theo luật riêng, chức bổ nhiệm trả cơ quan. Trẻ nhận sở hữu nhưng quyền quản lý theo giám hộ, không cấp quyền rút tiền/chuyển đất tự do.

Các ca bắt buộc gồm nhiều người hưởng, con trước phối ngẫu, người hưởng chưa thành niên, nguồn cha/mẹ chưa xác minh, tài sản chung, khoản nợ chưa đủ trả, chủ xưởng chết khi còn công/lương/hàng đi, chức trống và không còn người hưởng. Save giữa các bước tiếp tục đúng, không chia lại hoặc bỏ mất claim. Quy tắc vận hành xưởng khi có nhiều chủ/trẻ và thẩm quyền quản lý phải được xác định rõ trước khi gọi chuỗi đó hoàn tất.

### 27.2 Quốc gia27 khu và địa lý có tác dụng

Danh mục24 lãnh địa+3 khu triều đình, dân số bối cảnh và topology là đầu vào nội dung. Chúng không thay việc khởi tạo Province, Person, hộ, holding, kho, cơ sở, quan hệ/quyền và tuyến vận hành. Mỗi lãnh địa cần chỉ ra nguồn nước/tài nguyên, nguồn hàng, nơi cư trú/thành thị, chủ thể có quyền, nghĩa vụ và kết nối; tổng dân số từ bảng atlas không tự được tính thành số NPC đang mô phỏng.

**Working setting `atlas-cartography-v1` (người dùng ủy quyền 2026-10-23):** Atlas là thế giới giả tưởng do dự án thiết kế, nên có thể tự tạo geometry cố định cho đủ27 ID hành chính; dữ liệu mới phải ghi rõ là thiết kế PolyEc, không trình bày như lịch sử hoặc đo đạc. Tọa độ ban đầu dùng `map_unit`, chưa gọi là km và không dùng diện tích polygon để suy dân số/sản lượng. Geometry giữ nguyên giữa campaign seed. Điểm/thành thị nằm trong một lãnh địa, không là vùng28; thành phố không cộng người lần hai. Dân số/diện tích đang có tiếp tục là ngữ cảnh cho24 lãnh địa; ba khu triều đình chưa có con số. Vẽ sông/đèo/cầu chỉ định vị; route/gameplay chỉ đổi khi đã có luật hướng, thời gian, capacity, phí, mùa và passability riêng. Realm vẫn sở hữu người/hàng/tiền/quyền, UI chỉ đọc projection đã lọc theo observer. Phạm vi, phép kiểm và thứ tự triển khai: [plan Atlas27 geometry v1](design/M5-atlas-cartography-v1-plan.md); audit xuất xứ và phần chưa hoàn tất: [báo cáo mức hoàn thiện thế giới](reports/M5-world-completeness-audit-and-next-work-2026-10-23.md).

Địa hình/sông/đường/cầu/bến phải thay đổi đường đi, khả năng thực hiện hoặc chi phí đúng luật. Map phải có ánh xạ picking từ vùng hiển thị sang ID thật, overlay hàng/tuyến/quyền/thông tin với cùng bộ lọc observer. Pan/zoom/layer không được làm thay đổi mô phỏng hoặc lộ bí mật. Art đẹp không thay phép kiểm đường đi và giao dịch27 vùng. Thành thị nằm trong lãnh địa, không cộng dân/tài sản lần hai.

Quy tắc triển khai đô thị hiện hành là thử nghiệm `urban-civic-fund-v1`, không phải tuyên bố về dữ kiện địa lý/lịch sử: vì Atlas chỉ có ngữ cảnh dân số và topology, còn `Person` mới có vùng chứ chưa có địa chỉ con, mỗi vùng trước mắt có một quỹ công trình đô thị gắn với Province và không có dân số riêng. Governance có thẩm quyền tại vùng được chuyển tối đa 5% số dư treasury vào quỹ một lần mỗi 30 ngày; policy có bốn mức cấp1%,2%,3%,5% và các lựa chọn tạo xưởng đô thị từ chính quỹ qua candidate đang trống, không đổi kích thước tensor. Không tạo dân, hàng, đất, council/election hoặc quyền mới. Lát thử nghiệm đã qua core/save migration, Python binding, native contract, GDExtension build và Godot model guard/editor scan; kích thước giữ220 actions/2935 observations, reward không đổi. Chưa có policy v26 để chạy inference/training, chưa có thao tác UI thật và chưa có nguồn dữ liệu địa lý/dân số thành phố, do đó chưa thể gọi toàn bộ thành thị hoàn tất. Tham số, migration, lệnh kiểm, lỗi ban đầu và giới hạn ghi tại [kế hoạch](design/M5.4-urban-civic-fund-plan.md) và [báo cáo đầy đủ](reports/M5.4-urban-civic-fund-evidence-2026-10-23.md).

Lát `municipal-relief-from-civic-fund-v1` nối quỹ Municipal với Aid vùng: candidate Governance action69 tiêu tối đa10% số dư mỗi lệnh (ít nhất1 đồng, không quá1.000.000), chỉ trong Province có charter và cùng actor có taxing authority. Người nhận là mọi Civilian còn sống trong Province, chia theo Aid hiện có; không phân loại “dân thành thị” và không chọn ReliefCase cụ thể. Fact232 nhận diện nguồn, save50 không đổi; action220/observation2935/reward giữ nguyên, household model contract v27 và timeline v29 do action/candidate meaning đổi. Release establishment runner262, CTest Realm và establishment transfer1/1 mỗi nhóm, Python model contract20/urban27, native contract1/1, GDExtension build và Godot editor scan pass. Không có policy v27/v29 hoặc UI interaction thật. [Plan](design/M5.4-municipal-relief-plan.md) và [báo cáo đầy đủ](reports/M5.4-municipal-relief-from-civic-fund-evidence-2026-10-23.md) giải thích phép tính và giới hạn. Thành thị M5.4 vẫn thiếu tọa độ/ranh giới/dân số thành thị và một tổ chức đô thị tách khỏi Province; không tuyên bố hoàn tất từ sổ quỹ này.

Lát đầu ngoại thương `finite-port-trade-v1` tại Nam Cảng (ID8) đã qua kiểm core và các consumer. Đối tác tổng hợp bắt đầu với5.000 đồng,32 DryFood và sức chứa32 Salt; không tự bổ sung. Giá thử DryFood24/đv, Salt28/đv, duty5%, phí2/đv; tối đa một lệnh8 đơn vị/ngày, đặt ngày D và đến cuối D+4. Candidate Economic vùng8 dùng action32–37; observation117–119; tensor giữ220/2935. Do đổi nghĩa action/observation, Realm giữ schema v27 nhưng contract household lên v26 và timeline worksite lên v28; model cũ không tương thích. Save49→50 ghi tài khoản/lệnh; migration Atlas27 bổ sung endowment một lần. Release ngoại thương30 kiểm tra, establishment247; CTest phụ thuộc10/10; Python foreign21/model contract20/urban24/config49 (7 checkpoint)/Realm273 (64 ngày)/clock175 (7 checkpoint); native contract1/1, GDExtension build và Godot editor scan pass. Đây chưa phải ngoại thương M5.4 đầy đủ: tuyến luôn mở, không có mất hàng ngẫu nhiên, một cảng và hai hàng, không có model v26/v28 đã huấn luyện và chưa có thao tác UI thật. Các giả định, kết quả, lỗi sửa và giới hạn: [plan](design/M5.4-finite-foreign-account-plan.md), [báo cáo đầy đủ](reports/M5.4-finite-foreign-account-evidence-2026-10-23.md).

Phần bổ sung `finite-port-trade-coastal-v2` nối lệnh với route Atlas tàu ven biển Nam Cảng8↔Hải Châu9. Nhập dùng chiều9→8; xuất dùng8→9; đóng đúng chiều tắt hành động và reject lệnh nguyên tử, mở lại phục hồi. Route hiện dùng2 ngày đi, công suất64 món/ngày, phí1 đồng/món; service thử giữ một đơn tối đa8 món mỗi ngày và phí cảng2 đồng/món. Ngày mới là D+3; chuyến save cũ vẫn đến D+4. GDExtension/Godot hiển thị trạng thái và thời gian/phí hai chiều. Action mask đổi nghĩa nên household contract lên v28, timeline lên v30; schema save50, tensor220/2935, reward giữ nguyên. Windows Debug: C++ foreign43, Realm/transport CTest2/2, Python model20/foreign22/urban27/config49/7 checkpoint/clock175/7 checkpoint, native runtime1/1, GDExtension build và Godot model guard5/5/editor scan pass. Ngoại thương vẫn dùng đối tác tổng hợp, không có xác suất thất thoát; tuyến chưa chia capacity chung với cargo nội địa, chưa có policy tương thích, chưa smoke UI với model live. [Kế hoạch](design/M5.4-foreign-trade-coastal-route-plan.md), [báo cáo đầy đủ](reports/M5.4-foreign-trade-coastal-route-evidence-2026-10-23.md). Không kết luận M5.4/M5 hoàn tất.

Ngoại thương và cargo nội địa nay cùng trừ vào công suất của một Atlas link theo ngày. Foreign order chờ nguyên lô, giữ ngày tải/đến; shipment nội địa tiếp tục chia cargo theo công suất dư. Hai chiều chung link như transport hiện tại. Không đổi save schema51 vì arrival đã được lưu; household contract v30, timeline v32, tensor220/2935 và reward giữ nguyên. C++ CTest nhóm route/Realm/native4/4 và nhóm establishment/household2/2; Python model20, foreign22, transport28, gameplay config49/7 checkpoint, clock175/7 checkpoint, Realm273/64 ngày, urban27; GDExtension, Godot model guard5/5 và editor scan pass. Chưa có policy v30/v32, nhiều seed, benchmark, live UI, RL, Profile B hoặc package. Chi tiết phép xếp hàng, bằng chứng save/load, lần chỉnh nhịp ngày trong test và giới hạn: [shared capacity report](reports/M5.4-foreign-trade-shared-capacity-evidence-2026-10-23.md), [plan](design/M5.4-foreign-trade-shared-capacity-plan.md).

Quân sự phải ghi nguồn quân, chỉ huy, nghĩa vụ, nơi tập kết, thời gian, tiền công, lương thực/quân nhu, route và cách giải ngũ. Huy động làm giảm thời gian lao động nơi xuất phát, tăng chi và có thể làm thiếu nghĩa vụ khác. Không cấp lính/sản lượng trùng, không cho quân thiếu tiếp vận miễn hệ quả. Cách tính chiến đấu/rủi ro chưa có contract phải được chốt trước code, không lấy thắng/thua định sẵn làm luật campaign.

Đã triển khai lát `volunteer-muster-obligation-v1`: Governance đề nghị danh sách volunteer đã đồng ý và được huy động; người đứng đầu quốc gia xem đúng danh sách, Accept/Reject; người thật di chuyển bằng Migrate; core ghi số đã tới, thiếu hụt theo hạn và một lần xin gia hạn có consent. Nghĩa vụ không ép volunteer, không thêm quân số, payroll, hàng ăn hay combat. Save51 thêm section nghĩa vụ, schema50 trở về trước nạp danh sách rỗng; household/timeline contract v29/v31; action/observation giữ220/2935, reward không đổi. C++ runner25 checks, CTest liên quan3/3, Python model20/foreign22/config49 trên7 checkpoint/clock175 trên7 checkpoint/Realm replay273 trên64 ngày, native contract, GDExtension build, Godot UI smoke13/13, model guard5/5 và editor scan đều qua. Chưa có policy v29/v31, benchmark/Profile B/package hoặc thao tác Godot bằng policy live. Thiết kế và kiểm tra được ghi đầy đủ trong [plan](design/M5.4-volunteer-muster-obligation-plan.md) và [báo cáo](reports/M5.4-volunteer-muster-obligation-evidence-2026-10-23.md). Những yêu cầu còn lại về chỉ huy/quân nhu/đơn vị chiến đấu không được xem là đã giải quyết.

`muster-destination-ration-v1` nối roster được Sovereign chấp thuận tới kho DryFood của vùng nhận: thành viên thật đã Migrate tới đó, còn phục vụ và còn thời hạn huy động dùng đúng một món kho công vào ngày ăn hiện hữu. Hàng trong kho bị trừ một lần trước inventory cá nhân; khi hết kho hoặc người giải ngũ thì fallback/behavior cũ giữ nguyên. Điều kiện được suy ra từ service, roster, vị trí và depot đã save; schema51, household v30/timeline v32, action220/observation2935 và reward không đổi. Windows Debug obligation runner32 checks, CTest obligation1/1, Realm trực tiếp19.045 checks trên Profile A10.000 người. Kế hoạch [M5.4 muster destination rations](design/M5.4-muster-destination-rations-plan.md), bằng chứng chi tiết [report](reports/M5.4-muster-destination-rations-evidence-2026-10-23.md). Thành thị: audit xác nhận chỉ có hai nhãn của prototype hai vùng “Bến Lúa”/“Ải Đá”, không có ánh xạ tới Atlas27; không được dùng chúng làm dữ liệu thật. Report ghi chính xác các trường địa lý/thành thị còn thiếu. Không có policy mới, RL, package hoặc kiểm consumer trong nhóm này.

Audit sẵn sàng RL hiện tại xác nhận Realm model schema v27 phát state295,220 action,2.935 observation; trainer/model v11 chỉ định schema v11, state128,129 action,1.676 observation; một test preflight cũ còn hardcode observation1.736. Run Governance/Civilian v30 dùng contract household v5 và cả sáu candidate trượt selector; Civilian v31 dừng update43 chưa validation. Những run cũ tạo rollout online từ C++ nhưng không phải policy hiện hành và không resume/đổi nhãn. Bước code kế là architecture/wrapper mới cho contract C++ đang dùng, smoke nhỏ có nhãn test-only; full train vẫn chờ gameplay M5.0–M5.4 ổn định và budget local đo được. [Audit plan](design/M5-current-contract-rl-readiness-audit-plan.md), [report đầy đủ](reports/M5-current-contract-rl-readiness-audit-evidence-2026-10-23.md), [plan adapter tiếp theo](design/M5-current-contract-rl-adapter-plan.md).

Ngoại thương cần chủ thể/tài khoản ngoài biên, đơn, goods, route/capacity, thuế và rủi ro có nguồn. Sổ đối ứng phải giải thích hàng/tiền nào ra vào biên; không dùng thị trường ngoài biên như nguồn vô hạn chữa thiếu hàng. Giáo hội/guild/thành thị phải có quyết định và nguồn lực thật trong chuỗi cứu tế, học nghề, thuế hoặc huy động, không chỉ là nhãn trên UI.

### 27.3 Chuỗi nguyên nhân và can thiệp

Ba chuỗi ở phần7 phải có Fact đầu vào, đối tượng bị tác động, độ trễ, lựa chọn can thiệp hợp lệ và kết quả đối chiếu. Fixture đặt trước được dùng để kiểm nhưng report phải ghi là fixture. Trong campaign, cùng luật phải cho phép phát sinh chuỗi từ world/action mà không gọi hàm riêng để dựng kết quả câu chuyện.

Mỗi chuỗi kiểm ít nhất nhánh không can thiệp và một nhánh có can thiệp cùng điều kiện đầu. Ví dụ cứu trợ phải thật sự lấy hàng/tiền ở nguồn, chịu route và đến đúng người; khoản thiếu không tự biến mất chỉ vì vua bấm cứu trợ. Quân huy động phải làm thay đổi việc và chi phí ở bên liên quan. Fact/thông báo giữ liên kết nguyên nhân để người chơi đọc được vì sao tình hình đổi; việc tổng hợp thông báo không được bỏ giao dịch hay sự kiện pháp lý.

Phần consent tham chiến được thêm riêng khỏi hợp đồng phục vụ quân ngũ: từ save56, volunteer có thể tự bật/tắt lựa chọn trong kỳ phục vụ; save55 và cũ hơn đọc thành chưa đồng ý. Command C++, lưu/tải, trường hiển thị qua GDExtension và nút Godot đã có kiểm build/smoke. Chưa có tương tác trên session Realm đang chạy vì không có bundle policy `realm-v31` hiện hành được phép dùng trong mốc này; consent cũng chưa là hành động của trainer. Chưa có đối thủ hay luật giải quyết trận. Xem [kế hoạch](design/M5.4-military-combat-consent-plan.md) và [bằng chứng](reports/M5.4-military-combat-consent-evidence-2026-10-23.md). Điều khiển từng đơn vị, casualty, tù binh và lãnh thổ không được suy ra đã có.

**Cập nhật `military-battle-readiness-v1`, 2026-10-23:** roster có thêm danh sách người đã consent và danh sách có thể được gọi vào trận trong ngày. Danh sách thứ hai còn yêu cầu đang trong kỳ phục vụ và hạn Mobilize chưa hết; vùng trận cụ thể phải được kiểm bằng vị trí Person lúc thực hiện command, vì volunteer có thể đã Migrate khỏi vùng nguồn theo một đề nghị tập kết được chấp thuận. Đây là phép chiếu từ service record, không ghi state trùng. Core runner chuyên biệt16/16, CTest1/1, Python snapshot smoke, Windows Release `polyec_env` và GDExtension `polyec` đều pass. Realm runner đầy đủ dừng sau434 checks ở checksum save lịch sử v14; giữ nguyên checksum và ghi giới hạn. Lát này không thêm trận hoặc đối thủ. Nguồn xung đột và quyền khởi xướng cần được quyết định trước khi casualty, rút lui và kiểm soát lãnh thổ được code. [Kế hoạch](design/M5.4-volunteer-battle-resolution-plan.md), [bằng chứng](reports/M5.4-volunteer-battle-resolution-evidence-2026-10-23.md).

## 28. Bộ dữ liệu RL, policy và tương thích model

### 28.1 Điều kiện được bắt đầu training

Điều kiện mở training ở phần8 là hoàn tất luật T1–T3 và gameplay M5.0/1–M5.4 trong core, nối vào trainer dùng cùng C++, cập nhật observation/action/mask/reward và kiểm chuỗi C++→Python liên quan. UI presentation và M5.5 có thể theo sau; chúng vẫn bắt buộc trước khi gọi toàn M5 hoàn tất. Không nhầm điều kiện mở training với điều kiện chọn policy hoặc phát hành. Không resume checkpoint cũ nếu luật/model contract đã đổi nghĩa.

**Kết quả adapter Realm v12, 2026-10-23:** model nhận state295/action220/observation2935; timeline v32, fingerprint, reward và fixture bật timeline được khóa trong metadata. Smoke PPO Economic một update từ 16 world ×64 bước tạo1.024 quyết định focal và28.042 quyết định peer; rollout69,05 giây, update71,27 giây, RAM đỉnh551.616.512 byte. Kiểm tensor/mask, transition C++, exact environment/checkpoint restore và reject bundle v11 đều pass. Một chart cùng JSON được sinh từ đúng log update. Đây chỉ là phép nối đường C++→Python và chẩn đoán một rollout; không có train/validation/holdout, selector hay policy đạt chất lượng. Giữ lại dataset đúng nghĩa: 1.024 transition chỉ nằm trong RAM khi update; file lưu history tổng hợp, manifest và checkpoint, không lưu trajectory từng bước. [Report đầy đủ cách dựng rollout, metrics, chart/hash và giới hạn](reports/M5-current-contract-rl-adapter-evidence-2026-10-23.md); [plan/kết quả](design/M5-current-contract-rl-adapter-plan.md).

### 28.2 “Xây bộ dữ liệu” cụ thể là làm gì?

PPO là phương pháp cập nhật policy bằng các lượt tương tác mới với môi trường. Policy là mô hình chọn hành động; nó không bắt đầu bằng bảng câu trả lời đúng viết tay. Trainer chạy nhiều thế giới C++ từ seed/cấu hình đã ghi; policy nhận dữ liệu quan sát được phép, chọn hành động, core kiểm và thực hiện, rồi trả hậu quả và reward. Reward là điểm số theo mục tiêu huấn luyện đã chốt; nó cần được đối chiếu với kết quả kinh tế/xã hội thật. Chuỗi tương tác mới đó được gọi là rollout và được dùng cho một lượt cập nhật model. Dữ liệu cũ dùng chẩn đoán không tự trở thành rollout tươi cho PPO.

Trong các trường dữ liệu bên dưới, observation là dữ liệu một actor được phép nhận; mask đánh dấu hành động nào hợp lệ; log-prob ghi xác suất của hành động lúc thu thập; value là dự đoán giá trị tương lai của model; recurrent memory là bộ nhớ riêng giữa các quyết định của cùng actor. Seed là số khởi tạo giúp tái lập world hoặc quá trình lấy mẫu. Những trường phục vụ thuật toán không được thay thế sổ tiền/hàng và Fact để đánh giá hành vi.

Mỗi run phải mô tả đầy đủ quy trình sau:

1. **Tạo world:** tập seed, scenario, phân bố người/hộ/tài sản/việc/nhu cầu, thời hạn game và các điều kiện lấy mẫu; ghi cấu hình/nguồn code/model contract. Không loại âm thầm thế giới khó hoặc tự bơm tài nguyên khi chạy.
2. **Chọn actor/đối tác:** family nào học, family nào frozen hoặc baseline, số người/vai trò thực, lịch gọi quyết định và sampling. Nhiều vai trò của cùng người không tự được tính thành nhiều NPC.
3. **Ghi transition:** actor ID, run/world/decision ID, observation/mask, action và tham số được chọn, log-prob/value của policy thu thập, recurrent memory/reset, game time trước/sau, reward và các thành phần, kết quả lệnh, terminated/truncated cùng nguyên nhân.
4. **Tạo update:** số world, số bước/world, tổng quyết định, cách ghép mini-batch và xử lý chuỗi recurrent. Phân biệt chết/kết thúc thật với hết ngân sách rollout; nêu cách bootstrap và reset memory để không học sai bước kết thúc.
5. **Lưu dữ liệu:** chỉ rõ giữ toàn trajectory hay chỉ thống kê update/checkpoint. Nếu rollout chỉ nằm trong RAM rồi bỏ, phải nói rõ; history.jsonl không được gọi là dataset trajectory đầy đủ có thể phát lại.
6. **Kiểm chất lượng dữ liệu:** không lộ secret, action/mask nhất quán, không invalid action bị đổi lén, reward không đếm vay như thu nhập, invariant lỗi dừng run và lưu seed/commands đủ tái hiện. Các trường hợp dữ liệu bị loại phải có số lượng và lý do.
7. **Tách tập:** train tạo update; validation chọn checkpoint/siêu tham số; holdout chỉ đánh giá cuối. Ghi danh sách/range seed và đối tác từng tập, ngăn trùng hoặc điều chỉnh theo holdout. Báo đủ horizon, số quyết định, số world và trường hợp chết sớm.

Nếu dùng curriculum hoặc oversample tình huống xưởng/nợ/khủng hoảng, phải ghi tỷ trọng và mục đích; bộ đánh giá cần thế giới độc lập để phát hiện policy chỉ giỏi tình huống được luyện quá nhiều. Không gọi imitation-only, random weights hoặc dev baseline là learned RL hoàn tất.

### 28.3 Đánh giá tiến bộ và phân tích điểm yếu

So learned candidate với cả policy ban đầu và baseline trên cùng world seed độc lập, cùng cách sampling runtime. Báo utility theo family và các hậu quả thực như survival, nợ/mất khả năng thanh toán, nghĩa vụ, công việc, tồn kho và phân bố hành động theo câu hỏi đang kiểm. Chọn metric/biên không kém hơn trước khi xem kết quả. Các quality gates R0–R5 tại phần8 giữ nguyên; mã R1–R4 của quality RL khác với tên R1–R4 của kế hoạch core refactor.

Mỗi run training/evaluation phải có dữ liệu nguồn và hình đúng run/version. Hình train đặt return/reward theo update hoặc số quyết định; nếu làm mượt ghi cửa sổ và giữ raw series. Nhiều seed phải thể hiện độ phân tán. Hình validation/holdout tách khỏi train, dùng so sánh ghép cặp và độ bất định phù hợp; không gọi rollout return là điểm đánh giá độc lập. Phân tích ít nhất hành động bị lạm dụng/bỏ quên, tình huống thiếu dữ liệu và dấu chững lại nếu dữ liệu cho phép.

Ví dụ về giới hạn chứng cứ: [report T5.6](reports/M5-T5.6-rl-run-visual-evidence-2026-10-22.md) vẽ từ history v30/v31 đã lưu; sáu candidate v30 bị loại, v31 chưa có validation. Việc có biểu đồ không biến run thất bại thành policy đạt chất lượng và không cấp phép tiếp tục training khi các điều kiện gameplay còn thiếu.

### 28.4 Export, native inference và lỗi model

Checkpoint phải ghi family, source/config identity, observation/action/memory schema, model contract và seed/update. Export giữ xuất xứ, không sửa manifest model cũ để nhận luật mới. Kiểm Torch/ONNX/native về logits, mask, memory và phân bố hành động theo tolerance phần8; shape đúng một mình chưa đủ.

Runtime dùng shared weights, memory/sampling riêng và bounded batches; model lỗi/không tương thích phải dừng hoặc pause theo luồng sản phẩm có thông báo hiểu được. Thiếu model không được thay bằng heuristic rồi ghi learned-policy pass. Smoke model guard, smoke formatter UI, session game có inference và evaluation policy là bốn bằng chứng khác nhau; phải ghi đúng loại đã chạy.

### 28.3 Contract Realm v13 cho ba sổ đối tác, 2026-10-23

Realm hiện dùng `realm-v28`, household v31 và timeline v33; state301/action232/observation3085, reward không đổi. Đối tác A/B/C là sổ thử hữu hạn, mỗi sổ có5.000 đồng,32 DryFood và sức chứa32 Salt; cùng dùng cảng Nam Cảng và capacity route nội địa. Save52 giữ ledger/order cũ trong A, gắn order lịch sử với A và cộng endowment B/C đúng một lần. Python v13, native manifest và GDExtension đã được cập nhật; Godot panel source đã đổi nhưng UI smoke chưa chạy vì checkout không có Godot engine executable.

PPO smoke Atlas Economic seed20261025 dùng16 world×64 bước =1.024 quyết định; peer Governance/Civilian là dev baseline có nhãn, tổng28.624 quyết định peer. Reward rollout mean−0,010518; chart/raw JSON, manifest, history và checkpoint đúng run được giữ trong artifact. 51 cơ hội ForeignTrade hợp lệ xuất hiện nhưng policy smoke chưa chọn action ngoại thương. Đây không phải evaluation, không có train/validation/holdout split, paired baseline hay selector, và checkpoint không phải ứng viên phát hành. [Báo cáo đầy đủ cách tạo dữ liệu, kết quả, hashes, các checks và giới hạn](reports/M5.4-multiple-foreign-counterparties-evidence-2026-10-23.md).

### 28.4 Contract Realm v14 cho hai loại kho khẩu phần

Quyết định trial `volunteer-supplementary-ration-trial-1`: volunteer đang phục vụ ở vùng nguồn hoặc đã tập kết theo cam kết còn hiệu lực chỉ được nhận thêm SupplementaryFood sau khi lần kiểm tra bảy ngày đã thực sự cấp bữa DryFood. Mỗi món tăng10 nutrition, không tăng health, không thay DryFood và không tạo nhu cầu mới. Hệ số10 là giả định cân bằng có phiên bản, chưa được chứng minh; nó giữ mức bổ sung thấp hơn hiệu ứng bữa chính và được kiểm bằng so sánh trước/sau.

Để lệnh mua có nguồn hàng thật, Atlas mở màn với32 SupplementaryFood tại chợ vùng2, nơi World Atlas đã đánh dấu specialty SupplementaryFood. Hàng thuộc tồn kho chợ, không thuộc ngân khố hay kho quân nhu; mua vẫn trừ đúng giá và thuế thị trường. Profile nhỏ không nhận lượng mở màn. Migration schema52 và cũ hơn giữ DryFood, bắt đầu ngăn SupplementaryFood bằng0 và không tạo hàng.

Schema tăng Realm v28→v29, save52→53, state301→328, action232→263, observation3085→3484; household/timeline/reward giữ nguyên. Hai GoodId có ngăn kho, candidate mua/gửi, số tồn quan sát riêng và shipment đi/về cùng route/công suất. Checkpoint v13 giữ nguyên; model, co-play và trainer mới v14 từ chối shape/schema cũ. Core phải kiểm purchase/reject, shipment/capacity/roundtrip/migration và lịch bữa; Python/GDExtension/Godot smoke được gom ở mốc adapter. Nhóm này không thêm combat, không đổi consent hay tạo người.

## 29. Tích hợp, hiệu năng và gói Windows

### 29.1 Một lõi, nhiều lớp gọi

C++ sở hữu luật; Python, native runtime và GDExtension gửi command/đọc bản chiếu. GDScript không trừ tiền, sửa kho, tiến tuổi hoặc ghi body trực tiếp. Snapshot có thể sao chép dữ liệu đã lọc; không chuyển con trỏ mutable cho UI/trainer. Thay field public phải ghi consumer nào đọc và mốc nào sẽ kiểm bản source mới.

| Thay đổi | Kiểm ngay | Bằng chứng cần ở mốc tích hợp |
|---|---|---|
| Luật core riêng | Build target bị ảnh hưởng; thành công/từ chối/biên và một pipeline C++ | Regression theo phần dùng chung khi nhóm ổn định. |
| Save/codec | Roundtrip, migration phiên bản thực, hỏng/reference/tràn và không thay world đích | Replay consumer dùng save ở mốc liên quan; không lấy log source cũ chứng nhận source mới. |
| Binding hoặc observation/action/reward | Consumer trực tiếp, model guard và chuỗi C++→Python cùng seed/config | Transition/checkpoint/reward/mask khớp; model đúng nghĩa dữ liệu mới. |
| GDExtension/UI | Build extension và chạy Godot trên đường dữ liệu/hiển thị vừa đổi | Session thật có dữ liệu từ core, quyền observer, command và pause/save/load theo phạm vi UI. |
| Tối ưu hiệu năng | Chức năng không đổi, workload/seed/config cụ thể và đối chiếu trước/sau | Nhiều lượt cùng điều kiện; ghi nhiễu, p50/p95/p99 và chi phí/RAM liên quan. |

Build GDExtension tạo thư viện native để Godot nạp; chạy Godot smoke là chạy một đường kiểm tra trong engine; build/package toàn app là mốc khác. Chỉ sửa core không mặc định lặp cả ba. Nếu máy thiếu Godot hoặc model bundle, report giữ rõ phần bị chặn; kiểm nhãn từ dữ liệu dựng sẵn không thay kiểm snapshot từ session thật.

### 29.2 Đo đủ chi phí sản phẩm

Profile B phải có27 khu,100.000 thường dân/trẻ và512–2048 vai phức tạp theo §9. Ghi riêng số Person, số vai/tổ chức, số lời gọi policy và số actor thật; số bản ghi render/log không phải dân số. Workload cần đường kinh tế/xã hội thực, không chỉ constructor hoặc dân đứng yên không ra quyết định.

Đo riêng core day, observation/mask, inference, validate/commit, index, invariant chẩn đoán, snapshot/UI, save/load và tổng chu trình. Ghi p50/p95/p99, số mẫu, CPU/RAM resident/peak, backlog, input và FPS theo budget §9. Core-only không có inference phải gắn nhãn; một benchmark31 ngày không thay mô phỏng20 năm hoặc quality policy. Giữ binary/source/config của hai phía A/B; không so source mới với con số cũ trên máy khác rồi kết luận tăng tốc.

Khi thử tối ưu không chứng minh được lợi ích, có thể bỏ thay đổi và giữ dữ liệu thất bại như T5.2. Chỉ giữ tối ưu khi bảo toàn hành vi và có bằng chứng phù hợp; không dùng số run tốt nhất hoặc giảm phạm vi mô phỏng để đạt budget. Tối ưu trước hết vào dữ liệu thưa, lookup/chỉ mục, bounded batches và thứ tự cập nhật có ranh giới đúng.

### 29.3 Chạy dài và đóng gói

Phép chạy dài theo §9 phải theo dõi tuổi/sinh/tử, lương thực/sức khỏe, việc/thu nhập, giá/thiếu hàng, nợ/nghĩa vụ, queue và phản ứng trước can thiệp. Báo seed thất bại và nguyên nhân; một thế giới còn người sống không chứng minh xã hội ổn định. Giữ phân biệt protocol M5 tối thiểu20 năm và stability tổng quát30seed/tối đa50năm hoặc ending; không gộp số ngày để thay số seed.

Package Windows phải chứa đủ binary/runtime/model/dữ liệu và hướng dẫn đúng bản; chạy offline không cần Python/Node/API key. Kiểm new/load, pause/tốc độ, mất focus/ngủ máy không chạy bù, đóng/mở lại, đường dẫn save/quyền ghi và lỗi model/dữ liệu thiếu. Đo RAM peak khi save và khả năng tiếp tục sau load. Linux đang hoãn theo quyết định nền tảng, không tự xuất hiện như điều kiện chặn Windows M5.

M6 vẫn sở hữu hoàn thiện UX/tutorial/accessibility và M7 sở hữu production training/release theo roadmap. Những tích hợp/UI/model/package đã được yêu cầu để đóng M5 vẫn phải làm ở M5; không chuyển chúng sang mốc sau để giảm điều kiện đã chốt.

## 30. Bao phủ yêu cầu, phụ thuộc và điều kiện đóng M5

### 30.1 Mỗi nhóm phải kết thúc bằng hành vi có thể đối chiếu

| Nhóm yêu cầu | Điều kiện cần chứng minh trước khi đóng phạm vi tương ứng |
|---|---|
| Core refactor | Tách trách nhiệm nhưng giữ thứ tự giao dịch/RNG/ngày và hành vi được duyệt; R1–R4 có report tích hợp riêng. Không nhầm với các quality gate RL cùng tên. |
| M5.0/1 | Registry/ID/schema/quyền; cá thể/năng lực/tri thức; hộ/giám hộ/đất/tài khoản có vòng đời và save/load đúng; thiếu quyền không tiêu nhầm tài sản. |
| T1–T3/P6 | Lịch→hiện diện/chú ý→công sức/sản phẩm→body/care/lifecycle có nguồn thời gian/dịch vụ thực; cá nhân hóa có dữ liệu và phép đo phù hợp. |
| M5.2 | Cơ sở/hợp đồng/học nghề, nguồn/consumer đủ16 nhóm hàng, thị trường/giá, vận tải và các tình huống thiếu nguồn lực; toàn chuỗi qua command. |
| M5.3 | Tổ chức/đại diện/quỹ, hôn nhân có consent, di sản/nợ/claim/đồng sở hữu/trẻ, chức/tước và giáo hội có tác dụng trong chuỗi xã hội. |
| M5.4 | Quốc gia27 khu vận hành, thành thị/quân sự/ngoại thương, quyền/thông tin UI vua và ba chuỗi nguyên nhân có nhánh can thiệp. |
| RL và tích hợp | Dữ liệu/model đúng luật, C++→Python parity, policy đạt tiêu chí đã chốt, ONNX/native parity và session thật tương ứng; thiếu model ghi đúng phần chờ. |
| M5.5 | Profile B, stress/chạy dài/hiệu năng toàn chu trình và package Windows theo budget/protocol; không dùng các phép chạy riêng thay thế. |

### 30.2 Cách chọn việc kế tiếp

Chọn theo điều kiện còn thiếu của một chuỗi M5. Khi nhóm chính chưa đóng, chỉ chuyển sang helper/benchmark/biểu đồ nếu có phụ thuộc cụ thể hoặc nhóm chính bị chặn; ghi rõ nguyên nhân và việc mở khóa. Danh mục capability, codec hay formatter là bước hỗ trợ; số lượng các bước đó không đại diện phần trăm milestone. Hoàn tất nhóm thì chọn nhóm phụ thuộc tiếp theo theo roadmap, không lên lịch theo giờ. Theo chỉ đạo người dùng ngày 2026-10-23, hoàn thiện các chức năng gameplay của bản game trước; không bắt đầu hoặc tiếp tục train/selection/validation/holdout RL cho tới mốc đó. Các cổng policy, hiệu năng và package vẫn giữ nguyên, chỉ thay đổi thứ tự để gameplay đi trước RL.

Trước code, plan ngắn phải nêu hành vi người chơi/agent, dữ liệu và chủ sở hữu, success/reject/biên, save/binding/action/observation/reward/model impact, file/target bị ảnh hưởng, kiểm ngay/kiểm sau và điều kiện đóng nhóm. Tham số chưa có căn cứ phải ghi working assumption có phiên bản, đơn vị, miền, lý do và phép kiểm; không đổi luật quyền/consent, scope hay ngân sách bằng một giả định kỹ thuật.

### 30.3 Báo cáo đủ để người đọc tự đối chiếu

Report của một nhóm phải giải thích: trước đây xảy ra gì; nay thay đổi gì với người/tiền/hàng/thời gian/thông tin; phép tính hoặc tình huống cụ thể; code/phiên bản/config nào đã chạy; lớp nào đã kiểm; lỗi tìm thấy và cách sửa; giới hạn và điều kiện còn thiếu. Lệnh/log/hash là bằng chứng bổ sung, không thay kết luận chức năng. Với core refactor, mô tả trách nhiệm code được tách và hành vi được giữ, không gọi thành tính năng game mới.

Bằng chứng theo run phải cố định. Source đổi về sau không được dẫn tới việc rehash manifest cũ để khớp workspace mới; nếu report sai, thêm đính chính có nguồn và giữ kết quả ban đầu. Một nhóm dùng một plan/report được cập nhật rõ ràng, không cần bộ tài liệu mới cho mỗi assertion. CURRENT_TASK chỉ giữ trạng thái hiện tại, còn HISTORY/report giữ các kết quả đã đóng. Đây là cơ sở đối chiếu đầy đủ trước khi tuyên bố M5 hoàn tất.

WarHorse Stable region16 đã hoàn tất. Cơ sở tiêu đúng Horse có thật từ stock region, sau Work tạo WarHorse; không thêm breeding, quân sự hay quyền chuồng. Save41/`ESTABLISH_11`→42/`ESTABLISH_12`; actions0–166 giữ nguyên, thêm167–169, tổng170/2168; model realm-v20, household-v18, timeline-v20. C++ Stable27, Production68, Realm18.993 và regression C++/Python/native/GDExt đều pass như [WarHorse report](reports/M5.2-warhorse-training-region16-evidence-2026-10-23.md); recipe, lệnh, test ban đầu và giới hạn được ghi đầy đủ. Plan: [WarHorse training](design/M5.2-warhorse-training-plan.md).

Weapon Forge tại region19 đã hoàn tất: một Iron + một Tools +16 Work tick hợp lệ tạo đúng một Weapon; thiếu đầu vào và hàng thay thế không tạo sản phẩm. Schema42/`ESTABLISH_12`→43/`ESTABLISH_13`; giữ actions0–169, candidates170–172, tổng173/2204; contract realm-v21, household-v19, timeline-v21. Các target C++/Python/native/GDExtension liên quan pass; chi tiết, số đếm và giới hạn Godot/RL/Profile B/package trong [Weapon Forge evidence](reports/M5.2-weapon-forge-region19-evidence-2026-10-23.md), plan/result tại [Weapon Forge plan](design/M5.2-weapon-forge-plan.md). Lát này chưa thêm consumer quân sự.

Nền tổ chức trên đủ27 địa bàn giữ nguyên quyền tham gia thành viên và không tự cấp quyền sử dụng quỹ. Những phần đã qua phép kiểm ghép cặp gồm tuyến→thị trường→giá, tuyến→lương thực/sinh kế, ghi nhận thiếu bữa, kho lương công, hợp đồng tình nguyện, roster và hạn `Mobilize`; xem các [báo cáo theo nhóm](reports/). Test mới nối thêm kho lương công→tuyến→volunteer tự chấp thuận→`Mobilize`→bữa ăn→save/load, nhưng chỉ chứng minh đường thực thi có sẵn, không tạo một khoản nghĩa vụ quân nhu còn thiếu. Release runner vận tải172 checks và CTest lọc1/1 pass; chi tiết và giới hạn trong [plan](design/M5.4-volunteer-food-route-integration-plan.md) và [report](reports/M5.4-volunteer-food-route-integration-evidence-2026-10-23.md). Ngoại thương đã có sổ đối tác hữu hạn ở Nam Cảng nhưng chưa có nhiều tuyến/rủi ro; đô thị mới có sổ quỹ thử nghiệm gắn Province, chưa có dữ liệu và cơ quan thành phố thật. Audit quân sự xác nhận chưa có hồ sơ nghĩa vụ kèm số phải giao, hạn, thiếu hụt tồn và thương lượng khắc phục. Vì vậy ba phần này vẫn là backlog M5.4; không xem roster, tài khoản thử nghiệm hoặc một fixture nối được là hoàn thành M5.

### Quyết định kỹ thuật: dải ID/quỹ tổ chức cho 27 địa bàn — `organization-ledger-territory-v1`

Giữ nguyên công thức Join đã có `ID = 1 + 4 × region + group`, group0–3, nhưng cho phép toàn dải region0–26, tức organization ID1–108. Đây chỉ mở sức chứa của mô hình hiện hành; không tự tạo tài khoản có số dư, tổ chức, thành viên, quyền đi lại hay quyền khai thác. Join, phí, thời hạn, một membership mỗi người và kiểm tra member giữ nguyên. Chủ thể không phải member tiếp tục bị từ chối khi dùng quỹ.

Save schema44 ghi108 số dư; schema43 trở xuống vẫn đọc28 số dư đã tồn tại và khởi tạo phần mới bằng0. Khi kiểm toàn bản đồ, phát hiện giá vùng10 trở đi đang ghi đè observation membership/quỹ và các ô sau đó. Do đó plan đã được sửa trước khi tiếp tục: giữ128 state cũ, thêm140 ô giá cho region7–26, chuyển candidate features tới index268, tổng observation2344; action173 và offsets action giữ nguyên. Field ID tổ chức chuẩn hóa từ `/28` sang `/108`, luôn trong [0,1]. Model contract tăng realm-v21→v22, household-v19→v20, timeline-v21→v22; không gắn nhãn lại bundle cũ, reward không đổi. Bài kiểm riêng phải assert tất cả region price offsets và các state field không bị ghi đè. [Plan và quyết định đã điều chỉnh sau khi test bắt được lỗi](design/M5.3-organization-ledger-territories-plan.md).

### Kết quả quyết định `organization-ledger-territory-v1`

Đã mở ID1–108, migration save43→44 và observation2344; Join/member/non-member/sai vùng giữ hành vi cũ. C++ regression liên quan, Python binding/replay, native runtime contract và GDExtension Release đều pass; test dedicated hiện có215 assertion, gồm189 giá trị quan sát (27 vùng × 7 hàng), ID108 và lọc ID/quỹ khỏi observation người ngoài. Không chạy Godot session, learned RL, Profile B hoặc package; đây không đóng M5. [Report đầy đủ](reports/M5.3-organization-ledger-territories-evidence-2026-10-23.md).

### M5.4 route → stock thị trường → giá cuối tháng

Test Release seed811 trên cùng hai Realm cho kết quả tuyến đóng: shipment10 không tới vùng đích, bốn đơn mua Tools đều thiếu và giá78→86; mở lại tuyến: người bán nhận hàng, đi tới vùng4, bán vào stock, bốn đơn được đáp ứng và giá78→70. Đây chứng minh hành vi C++ ghép cặp của phần route/market, không chứng minh sinh kế/dinh dưỡng, kiến nghị, ngoại thương hoặc policy đã học. [Report và kế hoạch](reports/M5.4-route-shortage-market-chain-evidence-2026-10-23.md).

### M5.4 route → lương thực → dinh dưỡng/sức khỏe

Release seed812: 200 DryFood rời stock vùng5; trong nhánh được mở tuyến hàng giao ngày8, người gửi đi tới vùng4 ngày10 và bán hàng; trên đường chính người gửi tiêu thụ một món. Sau30 ngày, sáu cư dân đích đều còn sống nhưng nutrition/health tổng lần lượt là80/548 khi không mở tuyến và600/596 khi mở tuyến; food stock0 so với179. Đây là hai Realm ghép cặp trong một fixture, không phải đánh giá nhiều seed hoặc chất lượng policy. [Báo cáo và plan](reports/M5.4-transport-food-livelihood-evidence-2026-10-23.md).

### M5.4 ghi nhận nguyên nhân dinh dưỡng giảm do bỏ bữa

Quyết định thông tin `food-shortage-report-trial-1` phát Fact113 đúng khi nutrition trước đó trên60 và sau một lần bỏ bữa xuống60 hoặc thấp hơn. Mức nutrition vẫn giảm20 và health giảm2 như luật hiện hành; Fact không tự cấp cứu tế hay thay đổi nguồn lực. Fact dùng visibility địa phương, không lộ cho cư dân thường ngoài vùng, và sovereign nhận theo độ trễ báo cáo sẵn có. Release transport target đạt130 checks, gồm đường chợ và cấu hình lịch sử không chợ; CTest chọn `realm_transport_tests` 1/1. Fixture không-market ban đầu dùng nhầm vùng4 thay vùng0; phép kiểm riêng tư ban đầu chọn sovereign, vốn được nhận báo cáo toàn quốc theo luật; sau khi sửa fixture/điều kiện, chạy tuần tự trên binary mới pass. Nhãn GDScript đã được thêm nhưng chưa chạy Godot do máy không có executable; do đó chỉ core/data/visibility được xác nhận và UI còn chờ. [Plan và report đầy đủ](design/M5.4-food-shortage-observation-plan.md).

### M5.4 audit baseline lệnh Mobilize

Lệnh hiện có chỉ chuyển40 tiền từ treasury vùng sang firm vùng và hạ bất ổn bằng `floor(military capability / 10)`; candidate term30 ngày bị validator giới hạn1–30 nhưng thử cặp term1/30 cho fingerprint giống nhau, nên chưa có thời hạn phục vụ thật. Kiểm riêng C++ cho phép đúng actor/vùng/quỹ, từ chối sai actor/vùng, thiếu tiền và term ngoài miền; các reject giữ fingerprint và tổng tiền khi thành công được bảo toàn. Realm Release runner19005 checks và CTest `realm_tests` 1/1 pass. Đây chỉ đóng audit baseline, không đóng yêu cầu quân lực. Quyền/consent và nguồn người, chỉ huy/nghĩa vụ, sổ lương, food/supply, tuyến, giải ngũ và save/load vẫn phải được thiết kế trong contract quân sự trước khi sửa gameplay. [Plan và report đầy đủ](design/M5.4-mobilization-baseline-audit-plan.md).

### Quyết định thử trước code `military-volunteer-service-trial-1`

Để có nguồn người thật mà không cưỡng bức Person, lát đầu chỉ cho Governance cùng vùng gửi proposal phục vụ tới Civilian từ18 tuổi, cùng vùng, đang rảnh và chưa có hồ sơ active. Người nhận có thể Accept/Reject và tự Resign sớm; không sửa consent ở loại proposal khác. Mỗi Person có tối đa một offer đang chờ/service active. Service term dùng ngày game `[1,30]`, candidate ban đầu30; offer mở7 ngày sau thời điểm sẵn sàng rồi expiry hoàn escrow. Lương thử `military-service-wage-trial-1` là4 đồng/ngày, tức tối đa ngày công hiện tại `64 tick / 16 tick mỗi unit × 1 đồng = 4 đồng`; con số dùng để nối đúng đơn vị lương đã có, chưa phải mức lương quân sự đã hiệu chuẩn. Treasury escrow toàn bộ `4 × term`, chi một phần mỗi ngày và hoàn phần dư khi expiry, resignation hoặc chết; không tạo tiền. Person có hồ sơ active không được nhận Work mới nhưng vẫn dùng action sinh hoạt khác; quân số đếm đúng người Person, không tạo NPC ảo. Không có chiến đấu, cưỡng chế, food/supply hay route trong lát này.

Lát này phải thêm16 candidate Governance (action173–188), tổng189 action và observation2536 (`state268 + 189×12`). Slot0–172 giữ nguyên; reward formula không đổi. Vì quyết định/action/state và model input thay đổi, contract tăng realm-v22→v23, household-v20→v21, timeline-v22→v23; save45 ghi service state, save44 trở về trước không tự sinh proposal/quân/tiền. Đây là trial để kiểm consent/escrow/payday/end/save, không phải bằng chứng policy đã học hay balance. Plan đầy đủ và các phép kiểm: [M5.4 volunteer service](design/M5.4-voluntary-military-service-plan.md).

**Kết quả 2026-10-23:** triển khai xong lời mời cùng vùng, đồng ý/từ chối, lương4 đồng/ngày từ ngân khố giữ trước, thời hạn1–30 ngày, giải ngũ/tử vong/hết hạn và hoàn phần chưa trả; hồ sơ trên Person cấm Work mới trong lúc phục vụ. Schema45 ghi hồ sơ; schema44 khởi tạo rỗng. Kiểm Windows Release gồm CTest chọn9/9, replay C++/Python273 checks trên64 ngày, contract binding20, native48, GDExtension build, Godot headless5/5 và service text smoke9/9. Không có model v23 hoặc run RL mới; đây không phải bằng chứng cân bằng/policy. Phần còn thiếu: cấp food/supply qua logistics và đo lao động mất cơ hội. Báo cáo kết quả, tham số, toàn bộ phép kiểm, sửa lượt đầu và giới hạn: [M5.4 voluntary service evidence](reports/M5.4-voluntary-military-service-evidence-2026-10-23.md).

### Quyết định thử trước code `volunteer-personal-food-route-trial-1`

Trước khi thiết kế kho/quân nhu chung, kiểm nối đúng những luật đã chạy: một volunteer là chủ lô DryFood, tự vận chuyển10 đơn vị trên một tuyến quốc gia hiện có, đi tới vùng đích bằng Travel, rồi tiêu thụ theo lịch nhu cầu food hằng tuần hiện hành. Hai Realm ghép cặp cùng seed và state; chỉ một nhánh mở tuyến. Quy tắc ăn giữ nguyên một DryFood mỗi7 ngày theo lịch ID, +20 nutrition khi ăn, −20 nutrition và −2 health khi bỏ bữa. 10 đơn vị là lượng fixture để quan sát đầy đủ chuyển hàng, không phải định mức khẩu phần hoặc cấp phát quân đội. Không thêm consumption `MilitarySupply`, Weapon hay WarHorse.

`volunteer-labor-opportunity-proxy-trial-1` đo số volunteer-person-days trong kỳ và kiểm Work bị từ chối; không giả định sản lượng dân sự bị mất vì ứng viên được chọn khi đang idle. Không thêm consumer runtime, kho/quyền quân nhu, model/action/observation/save hoặc thay đổi UI. Plan và điều kiện đóng trước test: [volunteer food route integration](design/M5.4-volunteer-food-route-integration-plan.md). Báo cáo phải phân biệt đây là hàng cá nhân tự vận chuyển, chưa phải tiếp tế đơn vị quân.

**Kết quả 2026-10-23:** transport test-only ghép cặp seed814: cùng volunteer #75 ở vùng5, shipment10 DryFood tới vùng4 ngày14 khi route mở; tuyến đóng không giao. Sau tám ngày, nutrition100 so với60; hai nhánh đều giữ hồ sơ active, cùng payroll và Work bị từ chối. Save/load giữa shipment và Migrate giữ fingerprint; direct runner153 checks, CTest transport1/1 pass. Đây là đường hàng cá nhân; chưa có kho/cấp phát hoặc consumer quân nhu dùng chung. Điều chỉnh Travel→Migrate cho Civilian và thứ tự mở/đóng tuyến được ghi chi tiết tại [evidence report](reports/M5.4-volunteer-food-route-integration-evidence-2026-10-23.md).

### Quyết định thử trước code `volunteer-work-opportunity-paired-trial-1`

Hai Realm cùng seed dùng cùng offer/người nhận. Nhánh service nhận offer; nhánh control từ chối, được hoàn escrow rồi gửi cùng command Work hợp lệ ở vùng của người đó. So sánh C++ `produced_` sau tám ngày; công thức đang có mỗi ngày là `1 + floor(skill/40)` đơn vị legacy-good của vùng. Active service Work phải reject, control Work phải tạo sản lượng. Đây đo tác động trực tiếp của cùng một người, không ngoại suy GDP hay sản lượng nhiều seed. Plan trước code: [M5.4 volunteer labor cost](design/M5.4-volunteer-labor-cost-plan.md). Kiểm chỉ Realm Release target, direct runner và CTest `realm_tests`.

**Kết quả 2026-10-23:** paired seed9201, vùng0, person15, skill11. Nhánh phục vụ không tạo DryFood từ Work; nhánh control tạo8 DryFood trong8 ngày, đúng một mỗi ngày theo luật hiện tại. Payroll phục vụ32 đồng, nhánh từ chối nhận escrow lại. Realm Release runner18,995 checks và CTest `realm_tests`1/1 pass. Đo này chỉ là production trực tiếp một người trong một fixture; không phải GDP hay ước lượng đa seed. Chi tiết: [M5.4 volunteer labor cost evidence](reports/M5.4-volunteer-labor-cost-evidence-2026-10-23.md).

### Quyết định thử trước code `military-ration-depot-trial-1`

Lát quân nhu tiếp theo tạo kho DryFood công mỗi vùng. Governance có taxing authority/charter mua hàng từ stock cùng vùng bằng treasury theo giá vùng, thuế theo `MarketManager`; từ chối nguyên tử nếu thiếu tiền/hàng. Chuyến công chuyển hàng giữa kho qua route/capacity hiện có, phí từ ngân khố vùng gửi; hàng công không được ghi vào di sản của người phát lệnh chết. Volunteer dùng tối đa một món từ kho cùng vùng trong đúng bữa ăn theo lịch hiện có (mỗi7 ngày theo ID), trước inventory riêng; nếu hết kho thì fallback đúng luật cá nhân hiện hành. Không tăng nhu cầu và không tạo chiến đấu/định mức vũ khí.

Để policy có thể chọn mua/gửi: thêm4 candidate mua1/4/16/64 món và27 candidate đích gửi; thêm27 quan sát số DryFood ở các kho. Kết quả là state295, actions220 và observation2935; contract realm-v24/household-v22/timeline-v24. Save46 bổ sung account kho; save47 lưu owner kind/vùng nguồn cho chuyến công trong khi vẫn đọc shipment cá nhân lịch sử. Reward không đổi. C++ paired runner170, Python ration binding16, Python model contract20, native runtime contract1/1, Godot guard5/5, service UI smoke9 và Godot headless editor scan pass. Không có model v24 nên chưa có gameplay AI live hoặc run RL mới. Toàn bộ tham số, phép kiểm, lỗi ban đầu và giới hạn: [regional ration depot plan](design/M5.4-regional-ration-depot-plan.md), [adapter evidence](reports/M5.4-regional-ration-depot-adapter-evidence-2026-10-23.md).


**Kết quả 2026-10-23 — thời hạn tập kết:** save48 ghi section `MILITARY_SERVICE_2` cùng `mobilized_until_day`; schema45–47 vẫn nạp được và khởi tạo cờ tập kết bằng0. Term1–30 chỉ đánh dấu hồ sơ volunteer active cùng Governance chỉ huy và vùng nguồn, không vượt ngày hết thỏa thuận đã nhận. Lệnh gọi lại ngắn giữ nguyên hạn; lệnh dài chỉ gia hạn tới giới hạn đó. Roster và feature10 của candidate cho biết thành viên/hạn; Fact227/228 nối lúc bắt đầu/hết hạn vào lịch sử có thể đọc. Realm Release runner **19.075 checks**; CTest Realm và mười consumer kiểm serialization **11/11**; Python model contract20/ration17; mười binding check xưởng/tổ chức215; gameplay-config replay49, Realm Clock175 với bảy checkpoint khớp; CTest gameplay-config/clock replay2/2; native contract1/1; GDExtension build; Godot muster UI13, model guard cũ5/5 và editor scan pass. Kích thước giữ state295/actions220/observation2935, reward giữ nguyên; contract realm/household/timeline là v25/v23/v25. Chưa có model v25 nên chưa có kết quả policy live hoặc training. Công thức, các lỗi đã sửa và giới hạn đầy đủ: [report thời hạn tập kết](reports/M5.4-mobilize-volunteer-muster-window-evidence-2026-10-23.md), [plan/kết quả](design/M5.4-mobilize-volunteer-muster-window-plan.md).

### Quyết định thử trước code `volunteer-supplementary-ration-trial-1`

M5.4 còn thiếu một loại quân nhu ngoài DryFood. Dùng đúng GoodId hiện có `SupplementaryFood` làm khẩu phần tăng cường, trong ngăn kho riêng; không thay DryFood và không thêm thức ăn cho dân thường. Một volunteer đang phục vụ hoặc đã tập kết đúng nơi được chấp thuận chỉ nhận thêm món bổ sung sau khi đã nhận bữa DryFood theo lịch bảy ngày hiện tại. Trial cộng10 nutrition, không tăng health, không tạo thêm bữa hay thay nhu cầu. Đây là hệ số thử có tên phiên bản, chưa được chứng minh cân bằng; quyết định đầy đủ và phép kiểm: [plan khẩu phần bổ sung](design/M5.4-supplementary-military-rations-plan.md).

Phạm vi kỹ thuật dự kiến: hai ngăn GoodId tách biệt, mua từ kho vùng bằng treasury/MarketManager, gửi theo route chung, consume chỉ cho volunteer đủ điều kiện. Thêm purchase/action slots và observation cho tồn kho SupplementaryFood; tăng Realm model schema và tạo model/trainer v14, không gắn nhãn lại v13. Save53 giữ nguyên DryFood/shipment cũ và khởi tạo ngăn bổ sung bằng0. Không thêm combat, weapon, consent hoặc production miễn phí.

**Kết quả 2026-10-23:** Core chuyển `DryFood` và `SupplementaryFood` trong các kho riêng, mua bằng ngân khố địa phương, gửi theo tuyến/công suất cũ và chỉ cộng10 nutrition cho volunteer đủ điều kiện sau bữa DryFood thật; health không đổi. Region2 có32 SupplementaryFood trong stock chợ ban đầu vì đó là specialty Atlas, không phải hàng công/miễn phí; profile nhỏ và save cũ không nhận endowment. Save53, Realm model v29, state328/action263/observation3484 và model/trainer v14. C++ `polyec_realm_transport_tests`190 checks, test công thức tập trung2/2 và Realm Release runner19.045 checks pass. Hai lượt Realm cũ dừng tại18.901 và18.922 do tham chiếu tới vector tạm trong test; sau khi đổi sang bản sao ổn định, full Realm pass. Python binding rations, model v14, co-play/resume/reject-old bundles và model contract20 pass; native runtime contract1/1, GDExtension build pass. Godot engine không có trong môi trường nên không có smoke UI thật. PPO v14 smoke 16×64=1.024 quyết định Economic chỉ để kiểm adapter; rollout buffer trong RAM, file chỉ lưu metrics tổng hợp/checkpoint/manifest và chart/raw JSON, không có held-out evaluation hoặc policy candidate. Báo cáo đầy đủ gồm tác động người chơi, phép kiểm, nguồn tạo rollout, giới hạn, chart và SHA-256: [SupplementaryFood evidence](reports/M5.4-supplementary-military-rations-evidence-2026-10-23.md); [plan](design/M5.4-supplementary-military-rations-plan.md). Đây chưa đóng M5.4/M5; nhóm kế tiếp đã chọn là nghĩa vụ cung cấp DryFood cho nghĩa vụ tập kết đã được hai bên chấp thuận.

### M5.4 — Cam kết DryFood cho nghĩa vụ tập kết

`volunteer-dry-ration-obligation-trial-1` nối roster tập kết đã được Sovereign chấp thuận tới kho quân nhu công của đúng vùng nhận. Governance mua DryFood bằng treasury vùng; chỉ shipment công được tạo từ lúc cam kết được accept, đúng người sở hữu/vùng nguồn/vùng nhận/GoodId và đã tới kho mới được tính. Mỗi roster member tương ứng một phần DryFood; hạn đầu14 ngày; một gia hạn7 ngày cần Sovereign accept. Shortage được ghi, không phạt và không cưỡng ép. Proposal đang chờ tự đóng nếu volunteer hoàn tất nghĩa vụ gốc hoặc hàng tới đủ trước khi Sovereign phản hồi. Fact234 shortage, Fact235 accept cam kết, Fact236 accept gia hạn; dải riêng không va Fact232 cứu trợ.

Save54; Realm v30; household/timeline v32/v34; state382, actions264, observation3550; reward giữ nguyên. Model/trainer/co-play v15 mới, v14 không đổi. Windows Debug: supply obligation36, military obligation31, transport190, Realm19.045 checks pass. Python binding/model/co-play v15/model contract20 pass; native runtime contract1/1; GDExtension Release build pass. Godot runtime chưa test vì không có executable; không tạo PPO run/dataset/biểu đồ và không chọn policy. Report chứa thông số, lệnh test, ca wrong destination/cancel/personal/supplementary, hai proposal tự đóng, migration, giới hạn và SHA-256: [evidence đầy đủ](reports/M5.4-volunteer-dry-ration-obligation-evidence-2026-10-23.md), [plan](design/M5.4-volunteer-dry-ration-obligation-plan.md). M5.4/M5 chưa xong; nhóm kế tiếp là kiểm chứng chuỗi nguyên nhân quân sự có hai nhánh can thiệp trước khi mở thêm luật chiến đấu (chưa có contract).

**RL được hoãn theo chỉ đạo người dùng ngày 2026-10-23:** selector/validation, holdout, model mới và các bước RL khác vẫn là cổng bắt buộc ở giai đoạn sau nhưng chưa được làm trước khi các chức năng gameplay cần thiết của game hoàn tất. Bước gameplay hiện tại nằm trong [CURRENT_TASK](CURRENT_TASK.md): hoàn thiện tích hợp battle đã qua kiểm core/Python/GDExtension/UI smoke và lập plan cho registry địa điểm đô thị Atlas27. Đây là thứ tự ưu tiên hiện tại, không hạ tiêu chí chất lượng policy, hiệu năng hoặc package.

**Cập nhật trạng thái theo phiên bản ngày 2026-10-24:** trạng thái tác vụ mới nhất nằm ở [CURRENT_TASK](CURRENT_TASK.md), không dùng câu “bước hiện tại” ở các đoạn lịch sử phía trên để chọn việc. Save hiện `POLYEC_REALM_66`, schema `realm-v34`, household/config v40 và timeline/config v41, state382/actions336/observations4414/reward v14. Consumer hiện có gồm MilitarySupply trong raid, Paper trong Investigation, Tea và Incense trong giao lưu tùy chọn, Copper trong recipe Tools, WarHorse trong Travel/Migrate, Leather trong phòng thủ Nam Cảng và SupplementaryFood cho Civilian sau DryFood; nguồn, luật và kết quả từng nhóm có báo cáo riêng. [CURRENT_TASK](CURRENT_TASK.md) ghi các khoảng trống cần làm tiếp, gồm quy mô dân số/profile B, nguồn hàng còn thiếu, hội đồng chưa bầu cử, nhà tư/thuê, campaign quân sự, địa hình/passability và kiểm tra phát hành. Realm runner rộng từng dừng ở assertion byte-for-byte save lịch sử v14 sau434 checks và chưa chạy lại trên toàn source v34 hiện tại. Không có policy được train trên household v40/timeline v41; RL và dataset vẫn hoãn cho tới khi gameplay cần thiết hoàn tất.

**RL v15 smoke sau khi ghép gameplay M5.4, 2026-10-23:** một update Economic qua binding C++ hiện hành,16 world sáu vùng×64 bước focal=1.024 quyết định;29.842 peer decisions dùng dev baseline có nhãn. Thời gian18,60 giây, peak RSS629.510.144 byte. Realm schema v30, observation3550, actions264, model/timeline v34. Reward rollout mean−0,008177, return target mean−0,302205; đây là tín hiệu trong cùng một update, không phải đánh giá policy. Chart reward/return và biểu đồ đủ33 Intent cùng JSON nguồn đúng run được lưu;18 Intent không có cơ hội hợp lệ trong fixture Economic ngắn, nên không kết luận policy bỏ quên chúng. Manifest smoke_only, không candidate/selection/runtime; rollout đầy đủ chỉ ở RAM, files giữ aggregate/checkpoint/manifest/plot data. Checkpoint khôi phục metadata/16 world states; script vẽ chỉ nhận đúng contract và1024 decisions. [Báo cáo đầy đủ về cách tạo dữ liệu, metrics, bảng Intent và hash](reports/M5-current-contract-rl-v15-smoke-evidence-2026-10-23.md), [plan/kết quả](design/M5-current-contract-rl-v15-smoke-plan.md). Đây là điều kiện đo tốc độ ban đầu, không mở cổng kết luận policy; việc kế tiếp là evaluator/selector validation v15, sau đó training candidate theo budget riêng và holdout tách biệt.

**Quyết định protocol R1 v15 trước code:** khóa split `realm-v15-r1-seed-split-1`: train `[20261101,20261102,20261103]` cho từng family; validation20 world seeds `303000–303019`; holdout100 world seeds `404000–404099`. R1 dùng Realm6 vùng/full focal horizon, mỗi run25 update×16 environments×64 bước=25.600 focal decisions. Economic curriculum là `economic-worksite-reset-v1`; Governance/Civilian `none`; peer, reward, config, schema/model/timeline và hyperparameters phải khớp trong ba run cùng family. Paired CI95 theo world seed khóa ngưỡng validation/holdout: utility learned−baseline lower bound≥−0,05; utility learned−initial lower bound>0; alive delta lower≥0; insolvent/defaults/obligations-due upper≤0. Chỉ validation chọn run có paired mean baseline cao nhất khi đủ điều kiện; không qua thì không bundle. Holdout chỉ kiểm run đã khóa, không chọn lại; `peer_eligible` chỉ sau khi R1 holdout đạt, `release_eligible` vẫn phụ thuộc app/performance/package. Đây là protocol thí nghiệm đã version hóa, không sửa reward, luật consent hoặc quality gate. Chi tiết runner, seed, lượng dữ liệu và kiểm tra: [plan selector/holdout v15](design/M5-v15-validation-and-holdout-plan.md).

### Trạng thái cập nhật M5.4: đề nghị giao dịch ngoại thương (2026-10-23)

Một thương nhân ở Nam Cảng có thể gửi đề nghị một đơn nhập DryFood hoặc xuất Salt tới partner A/B/C. Lượng hợp lệ1/4/8; bảng giá, duty5%, phí cảng2 đồng/món và route giữ theo cài đặt đang có. Offer không phải order cho tới khi partner actor gửi explicit accept với sequence riêng. Tài sản đầu bên nào thì được giữ đúng sổ đầu bên đó; reject/expiry hoàn lại, còn accept đưa vào escrow và tuyến giao hàng ngoại thương hiện có. Hạn phản hồi là hết ngày gửi cộng7; pause không tăng ngày. Save59/`FOREIGN_TRADE_6` lưu cả offer, escrow, sequence và trạng thái.

Core behavior cùng runtime/GDExtension snapshot và giao diện gửi/đọc offer đã có; runner ngoại thương82/82, ba CTest liên quan3/3, Windows Release `polyec`/`polyec_env` build, Python save roundtrip pass. **UI vẫn chưa được xác minh**: Godot4.5.1 executable không sẵn trên môi trường; editor scan và smoke còn là bước bắt buộc trước khi đóng nhóm. Không được coi C++ GDExtension build là bằng chứng parser/UI chạy. Chưa có partner learned policy; không được thay bằng hardcoded consent. Offer không được actor trả lời sẽ hết hạn; direct trade tiếp tục hoạt động. Model v31/state382/actions282/observations3766/reward giữ nguyên. [Kế hoạch](design/M5.4-foreign-trade-offer-plan.md), [báo cáo đầy đủ](reports/M5.4-foreign-trade-offer-evidence-2026-10-23.md). RL vẫn đợi tới khi gameplay M5 còn thiếu hoàn tất.

**Đính chính tích hợp UI ngày 2026-10-23:** Sau report ban đầu, Godot4.5.1 official portable được tải vào TEMP và xác minh SHA-256, editor scan pass; `military_service_ui_smoke.gd` 33/33, app `ui_smoke.gd` 49/49 và `native_smoke.gd` 19/19 pass. Điều này xác nhận GDScript parse, formatter và app/native UI hiện có. Chưa có live Realm UI click-through cho offer vì checkout không chứa bundle learned policy `realm-v31` tương thích; không dùng model cũ hoặc baseline giả làm bằng chứng feature đó. [Evidence report đã đính chính](reports/M5.4-foreign-trade-offer-evidence-2026-10-23.md).

## Cập nhật `atlas-urban-sites-v1` — 2026-10-23

Atlas27 có một địa điểm trung tâm hư cấu, có ID/tên/loại/tọa độ `map_unit`, cho từng territory ID0–26. Nam Cảng/Hải Châu là cảng; Kinh Kỳ/Hoàng Điền/Trực Lộ là trung tâm/kho/xưởng triều đình. C++ constexpr kiểm ID, tên duy nhất, foreign key và point-in-polygon. GDExtension đưa registry qua `atlas_map_data`; map Godot vẽ marker, label khi hover/chọn và giữ nguyên picking lãnh địa. Đây là nội dung bản đồ; chưa có census, Person residence, ranh giới đô thị, quỹ/authority riêng hoặc hiệu ứng kinh tế/route. Không thêm dân/hàng/tiền/quyền.

Release world-atlas target + CTest pass1/1; Windows Release `polyec` build pass; Godot4.5.1 editor scan pass; `ui_smoke.gd` 133/133 và `native_smoke.gd` 19/19. Chi tiết đủ 27 tên, kind, tọa độ, luật, file, command, kết quả và giới hạn ở [báo cáo địa điểm trung tâm](reports/M5.4-atlas-urban-sites-evidence-2026-10-23.md), triển khai theo [plan](design/M5.4-atlas-urban-sites-plan.md). Bước kế tiếp cùng vấn đề đô thị là xác định người mô phỏng có ở trong địa điểm nào, không lấy dân số Atlas làm census; contract phải xử lý migration, save và phân tách city/rural trước khi code.

## Cập nhật `urban-residence-census-v1` — 2026-10-23

Atlas Realm hiện gán nơi cư trú xác định bằng hash(seed, Person ID), với tỷ lệ đô thị kỳ vọng20% theo cấu hình thử `urban-residence-share-trial-1`. Không tiêu thụ RNG gameplay; 20% chưa hiệu chuẩn nhân khẩu học. Mỗi census đếm Person còn sống thành đô thị, nông thôn hoặc chưa rõ; các nhóm phải cộng đúng population sống. Di cư giữ site nguồn trong thời gian đi và đổi site khi arrival; newborn kế thừa loại/site của parent; death rời tổng đang sống. Python/GDExtension chỉ trả số tổng hợp theo quyền địa phương/toàn quốc, không cung cấp danh tính hoặc thêm observation cho policy.

Save tăng60→61 với section `URBAN_RESIDENCE_1`; schema60 trở về trước đọc thành Unknown thay vì tự nhận là nông thôn. Core CTest chuyên biệt xác nhận seed, tổng census, query không sửa fingerprint, roundtrip, migration save cũ, reject sai site nguyên tử, profile nhỏ, birth và arrival. Python smoke: seed20261023, 512 người → 132 đô thị/380 nông thôn; Sovereign nhận27 vùng, observer thường1 vùng. Windows Release `polyec_env`/`polyec` build, Godot editor scan và UI formatter47 checks pass. Foreign-trade CTest1/1 pass. Realm test runner dừng ở assertion historical v14 byte-for-byte sau434 checks như đã ghi ở CURRENT_TASK; checksum lịch sử không bị thay. Chưa làm live Realm UI click-through do thiếu policy bundle realm-v31 phù hợp; không huấn luyện RL. Bảng luật, file, kiểm tra, giới hạn và giá trị mẫu ở [báo cáo đầy đủ](reports/M5.4-urban-residence-census-evidence-2026-10-23.md), contract tại [plan](design/M5.4-urban-residence-census-plan.md).

Nhóm tiếp theo sau census được chọn từ khoảng thiếu địa lý/di chuyển: dùng thời gian route đang lưu để Travel/Migrate cộng ngày từng cạnh, phí thử `4 + 2 đồng/ngày`; không suy từ map_unit hoặc gọi là km. C++ shortest-path/command/save-load và route regression đã pass; Python/GDExtension build cùng Godot UI/native smoke đạt kết quả trong [report route-time](reports/M5.4-atlas-route-travel-time-evidence-2026-10-23.md), theo [plan](design/M5.4-atlas-route-travel-time-plan.md). `realm_tests` vẫn dừng ở historical v14 byte checksum sau434 checks, không sửa dữ liệu lịch sử.

### Cập nhật `medicine-source-and-treatment-v1` — 2026-10-23

GoodId Medicine21 hiện có nguồn hữu hạn trên MedicinalGarden tại Atlas20 Dược Cốc và consumer tự điều trị trong vòng đời Person. Vườn bắt đầu100 abstract source units; mỗi batch Work16 tick tiêu1 và tạo1 Medicine. Vốn mở cơ sở25/50/100 đồng là cash cơ sở và không đổi nguồn. Person sống với health1–60 chỉ dùng medicine từ inventory của chính mình: trừ1, tăng10 tới giới hạn hiệu quả70, ghi consumed ledger và Fact237; health>60 không dùng, health0/dead không được hồi sinh. Đây là luật game thử, không phải khuyến nghị y tế.

Save tăng61→62; không thêm field, `ESTABLISH_14` giữ shape. Realm v31, action282, observation3766 và reward giữ nguyên; model cũ không chứng minh chất lượng dưới gameplay mới. Core test runner28 checks; CTest Medicine, Atlas specialty và urban residence3/3; Windows Release `polyec_env`/`polyec` build; Python binding14 checks; Godot editor scan, medicine UI50, native19 và app UI133 pass. Chuỗi C++ kiểm được producer→save/load→rút→mua→treatment→save/load. Chưa chạy đủ100 mẻ đến cạn nguồn hoặc live Realm policy session; không có RL/dataset/holdout/package. Report chi tiết, các tham số, file, phép kiểm và giới hạn: [Medicine evidence](reports/M5.2-medicine-production-treatment-evidence-2026-10-23.md); plan trước code: [Medicine plan](design/M5.2-medicine-production-treatment-plan.md).

**Kết quả `supplementary-food-source-v1` — 2026-10-24:** Phù Sa region2 có SeedPressWorks16. Một source unit và16 Work tick tạo đúng một SupplementaryFood1; source mở đầu100, tăng25 mỗi90 ngày game và dừng ở100. Tồn mở đầu32 món, stock xưởng, stock market và depot quân nhu là các sổ riêng. Governance mua sản phẩm xưởng, gửi qua route Atlas, vùng nhận giao một món cho volunteer sau bữa DryFood. Save lên63, `ESTABLISH_14` không đổi; schema62 tiếp tục nạp Medicine garden. Realm v31/actions282/observation3766/reward không đổi. C++ runner28 checks, CTest liên quan5/5, Release `polyec_env`/`polyec` build, Python producer19 assertions và ration binding22 checks pass. Godot editor/UI/native smoke đang chờ vì không tìm thấy executable; không dùng smoke cũ làm bằng chứng UI của source mới. Bảng luật, từng ca kiểm và giới hạn đầy đủ: [report](reports/M5.2-supplementary-food-source-evidence-2026-10-24.md); [plan](design/M5.2-supplementary-food-source-plan.md).

**Nhóm kế tiếp được chọn: nhà ở và sức chứa của settlement Atlas27.** Census hiện biết Person thuộc Urban/Rural/Unknown nhưng không thể trả lời hộ nào có chỗ ở, settlement có bao nhiêu chỗ, một hộ di cư đến nơi hết chỗ thì chuyện gì xảy ra, hoặc ai có quyền xây. Viết và review plan trước code cho `urban-housing-capacity-v1`, gồm vật liệu ConstructionMaterial có nguồn thật, quyền trên land holding/use-right, công sức/tiền xây dựng, gắn căn nhà và sức chứa với settlement/person/household, hành vi khi sinh/migrate/death, census không đếm trùng, save migration và Python/GDExtension/Godot impact. Chưa đặt thông số sản xuất/giá trước khi plan đối chiếu source và quyền hiện hành. RL vẫn dừng cho tới khi gameplay cần thiết hoàn tất.

### Kết quả mới nhất: `military-occupation-ui-v1` (2026-10-25)

Đóng phần đưa chiếm đóng/đồn trú/phản công từ core sang người chơi: GDExtension snapshot lọc quyền; raid chỉ vùng đúng; người chơi Sovereign có phản công/rút lui theo từng vùng với volunteer đã consent và đang hiện diện; RealmRuntime dùng cùng command C++ và từ chối fingerprint cũ. Không đổi luật core, save76 hay tensor Realm v44. Release GDExtension build, Godot editor scan, occupation smoke40/40, military/service/supply/trade/census/medicine smoke51, apprenticeship action smoke19/19. Test occupation dùng service/consent/mobilization thật và signal nút RealmPanel; chưa phải click-through bằng chuột. Chi tiết luật hiển thị, lọc thông tin, các lỗi lượt đầu đã sửa và giới hạn tại [plan](design/M5.4-military-occupation-ui-v1-plan.md) và [report](reports/M5.4-military-occupation-ui-v1-evidence-2026-10-25.md).

**Kết quả `atlas-terrain-travel-resistance-v1` (2026-10-25):** route mặc định mới áp dụng ETA theo Atlas corridor: GreatRiver1 ngày; North3; West4; RoadImprovement/Coastal/East/RoyalDomains2. Đây là setting gameplay giả tưởng, không phải số đo địa lý; không đổi capacity, phí hoặc topology. Cùng route planner truyền ETA tới Travel/Migrate, shipment, ngoại thương và raid; save cũ/chuyến giữa đường giữ time đã lưu. Save76 không đổi; model contract lên v45 với tensor382/400/5182, reward14 như cũ. C++ CTest transport group6/6; Python model contract21, transport cross-check28; GDExtension Release, Godot editor scan, bốn UI smoke40/51/19/32 pass. Ban đầu ngoại thương fixture không hạ schema76 về lịch sử do điều kiện helper thiếu76; đã sửa helper và test chạy lại pass. Lượt replay Python đầu dùng nhầm `polyec_rl_replay`, thay bằng replay target C++ chuyên dùng và pass. [Setting/plan](design/M5.4-atlas-terrain-travel-resistance-v1-plan.md), [bằng chứng đầy đủ](reports/M5.4-atlas-terrain-travel-resistance-evidence-2026-10-25.md). Terrain raster, seasonal passability, Profile B, stress, perf, package và learned policy v45 chưa có. Atlas đã có hex polygon/picking, nhưng UI route không phản ánh terrain từ raster địa lý.

**Nhóm kế tiếp được chọn sau kiểm tra source:** route-map inspection và seasonal passability. Geometry Atlas27 không phải backlog mới: `atlas-cartography-v1` đã cung cấp27 polygon hex cố định đơn vị `map_unit`, core project GDExtension trả polygon/site, Godot `map_canvas.gd` vẽ chúng và dùng point-in-polygon picking. Cần tránh làm lại geometry. Bản đồ hiện là bản đồ topology giả tưởng, chưa có terrain raster; route ETA mới chưa được giải thích trên mọi hành lang và chưa có passability theo mùa. Kế hoạch kế tiếp phải nối inspection với route planner thật, đặt mùa/ngày game có phiên bản và đơn vị, chỉ thay passability/travel thuộc setting fiction đã ghi; không dùng tọa độ map_unit để suy kilômét, dân số hay sản lượng.

Đình chiến/đầu hàng/tù binh vẫn là M5 còn mở nhưng chưa code: external partners hiện là ba ledger không có cá thể quân đội hay bộ quyết định chiến lược. Không tạo accept/reject hardcode thay policy tự chủ. Đưa lại sau khi có mô hình actor/decision ngoại giao; nếu có tù binh thì phải là Person thật, không giả lập một garrison thành cá thể.
