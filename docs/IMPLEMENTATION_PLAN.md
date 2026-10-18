# PolyEc — kế hoạch toàn dự án (Cập nhật: 2026-10-18, sửa đổi theo thiết kế agent-centric)

Cập nhật: 2026-10-18. Đây là nguồn chuẩn về đích sản phẩm, phạm vi và thứ tự triển khai. Đây không phải chứng nhận tính năng đã chạy. Trạng thái và bằng chứng thực tế phải được ghi trong reports/ và CURRENT_TASK.md.

## 1. Bài toán sản phẩm

### Cách đọc kế hoạch đầy đủ

Phần I (1–10) xác định contract hiện hành, stack, roadmap và gates. Phần II (11–21) chứa đặc tả sản phẩm chi tiết. Người triển khai cần đọc phần tương ứng trước khi làm việc.

| Cần hiểu | Phần |
|---|---|
| Sản phẩm, luật nền, agency và quốc gia | 1–4 |
| Kiến trúc, thứ tự triển khai, nghiệm thu, RL và hiệu năng | 5–10 |
| Vòng chơi/ending và địa lý/danh mục vùng/tuyến | 11–12 |
| Chính trị, quyền, thông tin và điều tra | 13–14 |
| Sản xuất, thị trường, chính sách, quân sự và ngoại thương | 15–17 |
| UI, khởi đầu và hướng dẫn trong game | 18 |
| Augment, capability và tiến bộ cá nhân | 19–20 |
| Sự kiện, causal chain, đời sống dân cư và thông báo | 21 |

Các bảng nội dung là đặc tả phải đạt, không phải progress report. Đối chiếu trạng thái thực tế trong CURRENT_TASK và bằng chứng theo reports/.

PolyEc là game desktop native mô phỏng việc cai trị một quốc gia cuối Trung cổ giả tưởng trong một đời vua. Người chơi quan sát thông tin, thương lượng, ban hành chính sách và giao việc; thế giới thực thi theo luật và nguồn lực. Mục tiêu là liên kết quyền, nguồn lực, quyết định và hậu quả. Quy mô, chất lượng causal chain và invariant-safe transactions là tiêu chí thiết kế chính.


## 2. Các quyết định đã duyệt

Luật nền được duyệt ngày 2026-10-15; phần làm rõ ngày 2026-10-17 thay cách diễn giải cũ về busy, cơ thể, hoạt động và agency. Bản này hợp nhất quyết định kỹ thuật và sản phẩm.

| Phần | Contract phải giữ |
|---|---|
| Người chơi | Là vua; tác động qua tư cách, quyền, tài khoản và người thi hành rõ ràng. Lệnh không đồng nghĩa đã có kết quả. |
| Luật | Một bộ luật chung toàn quốc. Vua thay chính sách có tham số và cấp/thu hồi quyền theo luật; chưa có trình viết lại toàn bộ luật giữa campaign. |
| Cá nhân | ID bền qua đổi hộ/nghề/chức; trạng thái và vòng đời riêng. Giới tính không chặn sở hữu, chức vụ hoặc thừa kế. |
| Địa vị | Tách thân phận, danh phận, nghề, tài sản và chức vụ. Giàu lên không tự thành quý tộc; nợ không biến người thành tài sản. |
| Hộ | Thành viên, cư trú, người phụ thuộc và giám hộ động; tiền riêng và quỹ chung tách biệt. |
| Đất | Holding/cụm đất và quyền sử dụng, thu tô, chuyển giao, nghĩa vụ cụ thể. Quyền độc quyền không cấp hai lần; sở hữu khác quyền xét xử. |
| Cơ sở/nghề | Có chủ, kho, tài khoản, chỗ làm, công suất, vật tư và lao động thật. Học nghề cần bên nhận, người dạy, chi phí và thời gian. |
| Hôn nhân | Hai người chấp thuận, đủ tuổi/năng lực, không có phối ngẫu còn sống khác; cấm trực hệ/anh chị em. |
| Tài sản sau chết | Bảo vệ tài sản của người sống; trả nghĩa vụ từ di sản trước. Phần còn lại chia theo quy định di sản hợp pháp. |
| Tước/chức | Tước theo con hợp pháp lớn tuổi nhất không phân biệt giới; thiếu người kế tiếp mở hồ sơ chờ công nhận. |
| Người phụ thuộc | Có quyền sở hữu; giám hộ quản lý trong giới hạn, không sở hữu thay. Nợ không tự chuyển vượt di sản trừ nghĩa vụ bảo lãnh riêng. |
| Tổ chức | Gia tộc, thành thị, guild, giáo hội có thành viên, quỹ, chức và người đại diện. Quyền đại diện khác quyền sở hữu. |
| Thông tin | Sự thật world, lời khai, nghi vấn và điều từng người biết là các lớp khác nhau. Báo cáo có nguồn, ngày và độ trễ. UI vua không đọc trộm bí mật. |

Giới hạn M5 đã duyệt: chưa có nhận con nuôi, ly hôn, di chúc tùy ý hoặc cơ chế con ngoài hôn nhân. Không sinh các trường hợp rồi bỏ quyền của họ. Thay đổi luật/cơ chế lớn phải qua process in CURRENT_TASK và evidence.


## 3. Thiết kế NPC — mô hình agent-centric (thay cho phần stat-centric cũ)

Mục tiêu: mọi nhân vật (NPC) là một agent độc lập có tư cách hành xử (agency) thể hiện qua memory/knowledge, presence/attention, capability vector và policy.

Đặc tả ngắn:
- Identity: mỗi NPC có ID bền và metadata khởi tạo (age, household ref, endowment snapshot). Tên có thể sinh lazy khi cần.
- Memory/Knowledge: facts (xác nhận), estimates, rumours, secret evidence; mỗi mục ghi nguồn và độ tin cậy. PlayerKnowledge là lớp view của người chơi.
- Presence / Attention / Time budget: agent có một ngân sách thời gian/attention per tick/day; các hành động tiêu tốn attention/time. Nếu thiếu, hành động bị trì hoãn hoặc chỉ thực hiện một phần, tạo ra evidence về partial fulfillment.
- Capability vector (flexible): mô tả năng lực trên các trục chức năng (ví dụ: administrative_capacity, technical_skill, labor_capacity, diplomatic_skill, military_capacity). Không ép thành 6 chỉ số cố định; nhóm capability types được cấu hình bởi scenario/experiment.
- Policy: hàm quyết định (rule-based hoặc learned) chọn action dựa trên observation, memory và capability. Policy có thể là mô hình học (PPO, recurrent) hoặc heuristics; khi dùng học, training phải diễn ra offline trong binding với core.
- Goals & incentives: mỗi agent có tập mục tiêu (survival, wealth, reputation, role-specific objectives) và hệ trọng thưởng nội tại; conflict resolution giữa mục tiêu là một phần của policy.

Nguyên tắc thiết kế:
1. Không dùng một bảng 6-stat cố định làm nguồn duy nhất cho hành vi; thay bằng capability vector + policy.
2. Attention/time là tài nguyên nguyên tử: engine kiểm tra budget trước khi apply action. Thiếu budget tạo partial outcome (ghi rõ nguyên nhân).
3. Memory/knowledge phân lớp: facts (trusted), estimates, public events, secret evidence. Propagation knowledge có latency và nguồn.
4. XP/skill là optional: nếu dùng thì phải có contract rõ ràng (điều kiện tăng XP, ảnh hưởng lên capability) và tests để tránh double-ingest on load.
5. Policy & RL: train offline trong môi trường core (C++/Python binding), export to ONNX cho inference native. Rollout logs phải đầy đủ (actor id, observation, mask, action, log-prob, reward, elapsed time, memory reset, terminated/truncated).
6. Metrics cho agency: dùng behavioral metrics (decisions_per_day, fulfilled_tasks_rate, attention_waste, contract_success_rate) thay vì một stat duy nhất.
7. Backwards compatibility: UI có thể hiển thị summary “stat-like” cho người chơi, nhưng đó là view-derived aggregation từ capability vector và lịch sử, không được dùng làm source of truth cho engine.

Tests bắt buộc:
- Deterministic replay: seed + save -> same outcome trong miền chấp nhận.
- Transaction atomicity: multi-resource commands commit hoặc rollback hoàn toàn.
- Attention enforcement: hành động bị từ chối khi thiếu budget; test partial fulfillment cases.
- ONNX parity: fixture tests Torch->ONNX->native with thresholds and documented tolerances.


## 4. Nội dung quốc gia phải xây

- 24 lãnh địa chư hầu + 3 khu triều đình; map địa lý 2D có terrain/topology, đường bộ/sông/ven biển, lưu vực, tài nguyên, cửa ải và quyền kiểm soát. Pan/zoom/picking theo spec UI.
- 16 nhóm hàng: lương thực khô, thực phẩm bổ sung, muối, gỗ, nhiên liệu, sắt, kim loại màu/quý, xơ sợi, vải, gia súc/ngựa, công cụ, quân nhu/vũ khí, vật liệu xây, hàng xa xỉ, v.v.
- Sản xuất tại cơ sở dùng vật tư, lao động, công suất, mùa vụ và bảo trì. Không dùng nhãn nghề hoặc region % 7 làm luật sản xuất cuối.
- Chợ có tồn kho/giá/cầu; tín dụng có chủ nợ, lãi, hạn, bảo đảm và xử lý vi phạm.
- Logistics có tuyến, thời gian, công suất, phí, hàng đang đi và đặt chỗ. Không teleport.

(Chi tiết map, tuyến và danh mục vùng giữ như bản gốc; see sections 11–12)


## 5. Tech stack và ranh giới kiến trúc

| Thành phần | Trách nhiệm và nền tảng |
|---|---|
| Core | C++20 (OOP/composition), CMake; độc lập Godot; sở hữu world, command, transaction, invariant và game time. |
| App | Godot 4.5.x native/GDExtension, godot-cpp phù hợp; GDScript UI, terrain 2D. Windows ưu tiên; package Linux theo kế hoạch. |
| Training | Python / PyTorch gọi cùng core qua pybind11. Giữ core/Python portable để chạy môi trường Linux / cloud. |
| Runtime AI | ONNX Runtime CPU, shared weights, per-person memory/sampling, bounded batches. Không Python/Node/API key trên máy chơi. |
| Dữ liệu | Person ID bền; compact storage và sparse relations/index. Không scene node/model riêng cho từng người. |
| Save | Version/schema/model identity, RNG, lịch, tiến độ, escrow và knowledge; snapshot tại barrier, validate temporary trước thay live world. |
| Kiểm chứng | CTest core; Python/binding/replay; Torch/ONNX/native parity; Godot smoke/UI; benchmark toàn chu kỳ; package theo platform. |

Luồng duy nhất: nhận command và tư cách → validate → chuẩn bị thay đổi → kiểm invariant → commit → facts → thông tin tới người biết. Lệnh reject không sửa dữ liệu.


## 6. Roadmap toàn dự án (đã điều chỉnh)

| Mốc | Phạm vi phải có | Gate |
|---|---|---|
| M0 | Native core/map/clock; Python binding, ONNX shape và spike dữ liệu thưa | Transition C++/Python; native load; throughput/RAM primitive có số đo. |
| M1 | Kinh tế 6 vùng + đất triều đình, 7 hàng đầu, thị trường/tuyến/nợ/dự án/save | Bảo toàn, reject nguyên tử; shock và sandbox nhiều seed. |
| M2 | Quyền/chính trị/thông tin, individual/child/household, sparse relations/vòng đời | Quyền khác nhau có kết quả khác; profile A có tương tác. |
| M3 | Spike RL 3 họ, recurrent PPO, runner/curriculum, augment/capability framework | Pipeline và chẩn đoán R0–R3; checkpoint pilot không phải release. |
| M4 | Vertical slice native 6 vùng, Hắc Sơn/logistics/xưởng/hồ sơ nhiều phương án | Map/input/save, causal chain, controller có nhãn hoặc policy pilot hợp lệ. |
| M5 | Nền xã hội đủ sâu + quốc gia 27 khu/16 hàng/thành thị/quân sự/ngoại thương + profile B | Luật, liên kết xã hội, invariant, portable core/binding, workload và evidence. |
| M6 | Một đời vua/ending, tutorial/F1, thông tin/UI/save/backup/accessibility/map art | Luồng new/load/pause/end và freeze feature/action/observation/reward/scenario. |
| M7 | Production training/evaluation, export/parity, native regression và release | Learned policies qua quality gates; package/offline/platform và profile công bố có bằng chứng. |
| S | Scale research to 1M individuals (research target) | Phân tách: nghiên cứu scale + profiling, không phải commitment sản phẩm. |

Ghi chú: milestone S được giữ là mục tiêu nghiên cứu; trước khi cam kết cần harness profiling và memory/throughput evidence.


## 7. Nghiệm thu nền tảng

(giữ các yêu cầu nghiệm thu như bản gốc: five transactions atomic, three causal chains, checks for stale/duplicate/overflow, replay determinism)


## 8. RL và cá nhân hóa (tinh chỉnh)

- Train offline trong core via Python bindings; inference in-app via ONNX runtime.
- Rollout must log full trace (actor, observation, mask, action/log-prob, reward, elapsed time, memory reset). Export manifests required for artifacts/.
- Parity: build Torch→ONNX→native parity harness with fixtures. Use per-metric tolerances (KL divergence for distributions; relative/absolute tolerances for logits with documented reasoning). Avoid single hard abs threshold for all metrics.

Giữ quality gates R0–R5 nhưng cập nhật R5 parity requirement: parity checks plus statistical checks and documented justification for exceptions.


## 9. Quy mô, hiệu năng và nền tảng

(Profile A/B and performance targets as before; keep budgets and profiling guidance. Emphasize sparse graph, bounded batches, last-update lazy scheduling, and single-threaded determinism for critical sections.)


## 10. Thay đổi và nguồn chuẩn

Ưu tiên: chỉ đạo trực tiếp của người dùng và AGENTS → plan này về scope → CURRENT_TASK về tác vụ/status → DEVELOPER_GUIDE về phương pháp → evidence theo revision.

Phân vai: OpenCode khảo sát giới hạn/lập plan → Codex review → OpenCode code/test → Codex review diff/evidence. Chỉ hỏi người dùng khi đổi gameplay, phạm vi hoặc ngân sách.


## Phần II — đặc tả sản phẩm chi tiết (tóm tắt và điều chỉnh)

Phần II chứa các mục 11–21; phần lớn nội dung giữ nguyên nhưng có những điều chỉnh quan trọng dưới đây:

### Tham khảo và vị trí của các nguồn cảm hứng
Trong quá trình thiết kế, team đã tham khảo các bài viết/thiết kế sau:
- Factorio — học được các nguyên tắc tối ưu hoá bottleneck, batching và tránh over-update; áp dụng ý tưởng về xử lý bottleneck trước khi parallel hóa. Không áp nguyên xi entity-as-factory cho NPC.
- Victoria 3 — cơ chế aggregate economic feedback, các nhóm xã hội và nhu cầu; tham khảo cách biểu diễn aggregate nhưng không dùng nguyên mẫu cho per-agent decisions.
- ODD protocol (Grimm et al.) — framework mô tả purpose/patterns/state/scales/process/scheduling.
- Neural MMO / related — tham khảo scaling lessons nhưng không dùng kiến trúc server-client web để triển khai native.

Những tham khảo trên được lưu trong docs/archive/ như ghi chú lịch sử; IMPLEMENTATION_PLAN trích xuất bài học có thể áp dụng, đồng thời nêu rõ mọi khác biệt thiết kế cốt lõi (per-agent agency, invariant-safe transactions, save/replay determinism).


(Phần 11–21 chi tiết về loop chơi, bản đồ, chính trị, kinh tế, dự án, sự kiện giữ hầu hết nội dung cũ, nhưng mọi chỗ có nhắc "6 chỉ số" hoặc "XP bắt buộc" sẽ được hiểu theo contract agent-centric nói trên và được chuyển sang phần capability/policy trong DEVELOPER_GUIDE.)

---

Ghi chú cuối: file này là source-of-truth cho scope và milestone; mọi thay đổi cần cập nhật CURRENT_TASK.md và DEVELOPMENT_HISTORY.md kèm evidence trong reports/. Nếu bạn muốn, tôi sẽ mở PR draft với thay đổi này và bổ sung một checklist CURRENT_TASK cho các việc còn thiếu (atlas→individual mapping, parity harness, capability calibration).