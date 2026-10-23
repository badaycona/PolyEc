# PolyEc — blueprint bài toán theo milestone M0–M7

Cập nhật 2026-10-23. Tài liệu này làm rõ **toàn bộ roadmap sản phẩm**, kể cả M0–M4 đã có code/evidence và các giai đoạn M5–M7 chưa hoàn tất. Nó nối hai cách nhìn:

- `M0–M7`: kết quả sản phẩm mà một giai đoạn cần tạo.
- `P1–P16`: bài toán kỹ thuật dùng xuyên nhiều giai đoạn.

Một milestone không tương ứng với một module hay một commit. Mỗi hàng bên dưới phải được tiếp tục phân rã thành contract nhỏ có thể viết code và quyết định pass/fail. Các blueprint không tự đặt luật gameplay mới: [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md) vẫn sở hữu phạm vi/luật, [TECHNICAL_PROBLEMS](TECHNICAL_PROBLEMS.md) sở hữu cây vấn đề, [CURRENT_TASK](CURRENT_TASK.md) sở hữu trạng thái hiện tại và report sở hữu bằng chứng của một revision/run.

## 1. Hợp đồng tối thiểu cho mọi nhánh

Mỗi milestone con và mỗi bài toán con mới phải có cùng một bộ câu trả lời. Một nhánh chưa có đủ thông tin thì được ghi rõ là OPEN và phân rã tiếp; không ghi tên class/hàm tưởng tượng như thể đã có source.

| Trường bắt buộc | Cần ghi gì |
|---|---|
| Mục tiêu và phạm vi | Ai sử dụng, hành vi trước/sau, M cha và P phụ thuộc; phạm vi ngoài nhánh |
| Input | Loại dữ liệu, nguồn, chủ sở hữu, ID, thời điểm, đơn vị và version |
| Constraints | Miền hợp lệ, kích thước tối đa từ schema/config thật, invariant, quyền/consent, giới hạn số học và độ phức tạp |
| Quy tắc/thuật toán | Các bước theo thứ tự; success, reject không đổi thế giới, thất bại hợp lệ theo gameplay, trường hợp biên |
| Output | Trạng thái/sổ/view/event tạo ra, ai được thấy, cách ghi lý do không tạo kết quả |
| Save/tiếp tục | Schema, migration cũ, dữ liệu hỏng, save giữa hoạt động, replay/idempotence |
| API/code owner | Source hiện tại, module chịu trách nhiệm, API caller, cache/index dẫn xuất; tên đề xuất phải được ghi rõ |
| Oracle | Ví dụ tính tay hoặc fixture; expected values/invariant; test target và lệnh chạy ngay/mốc tích hợp |
| Bằng chứng | Revision, platform/config/seed/workload, lệnh đã chạy, raw data/chart RL nếu có, giới hạn kết luận |

Một bài toán tương tự đề competitive programming cần có input/output, constraints và cách tính đủ tất định để hai người độc lập cho cùng kết quả. Gameplay có thêm quyền, thời gian, save/load, tính nhìn thấy và hậu quả kinh tế; các yếu tố đó cũng phải được ghi thành điều kiện và output thay vì được lược bỏ.

## 2. M0 — dựng lõi native, bản đồ, đồng hồ và các cầu gọi ban đầu

**Kết quả sản phẩm.** Có core C++ native biểu diễn vùng/thế giới và thời gian; Python gọi cùng chuyển đổi; native nhận dạng được hình dạng dữ liệu ONNX đầu tiên; một workload thưa đủ để đo tốc độ và RAM ban đầu.

| Bài toán con | Input và constraints | Output đúng | Phép kiểm / phụ thuộc |
|---|---|---|---|
| M0.1 Khởi tạo world/map | Cấu hình vùng/đồ thị từ catalog; ID duy nhất và dữ liệu nằm trong miền schema | Realm khởi tạo tất định, truy vấn vùng/đường cho cùng dữ liệu | Khởi tạo cùng seed/config phải cho cùng fingerprint; sai ID/config bị từ chối; P1/P9/P15 |
| M0.2 Tiến đồng hồ game | `Realm state`, số tick nguyên, pause/focus state; ngày là 64 tick theo quy ước plan | Cursor tăng đúng; pause không tiến; không catch-up sau focus/máy ngủ | Một khoảng chia đoạn tương đương khoảng gộp khi không có sự kiện khác; tick ngày biên đúng một lần; P4 |
| M0.3 Một transition C++ có thể gọi | State hợp lệ + command cụ thể | Kết quả command và view; reject giữ nguyên state | So transition success/reject/fingerprint trong C++; P2/P3/P16 |
| M0.4 Python gọi đúng core | Cùng state/command được biểu diễn qua binding, không có luật kinh tế Python song song | Python nhận cùng kết quả và dữ liệu đọc | Ghép cặp C++/Python trên cùng fixtures; khác biệt kiểu số/null/order phải có oracle; P16 |
| M0.5 Native ONNX shape smoke | Model thử, manifest/dimensions, input/action mẫu | Runtime kiểm được input/output shape và báo lỗi model không tương thích | Shape/load smoke không được gọi là policy học được hoặc gameplay thành công; P10/P11/P16 |
| M0.6 Spike dữ liệu thưa | Số lượng người/quan hệ/workload được ghi; memory/index policy | Số đo throughput và RAM cùng script/input tái chạy | Không cấp ma trận N×N; báo kích thước thực và số đo, không suy khả năng một triệu người từ spike nhỏ; P14/P16 |

**Điều kiện phụ thuộc/đóng.** C++ là nguồn luật; Python và native là caller. Pause/focus, một transition qua binding, native model shape, primitive throughput/RAM đều phải có evidence riêng. Việc có thể load một ONNX chưa chứng minh inference đúng hoặc quality policy. Mã thuật toán cụ thể lấy từ source hiện hành, không được suy rằng ONNX đầu tiên là PPO production.

## 3. M1 — kinh tế căn bản sáu vùng và có thể tiếp tục qua save/load

**Kết quả sản phẩm.** Thế giới có sáu vùng, đất triều đình, bảy hàng đầu, chợ, tuyến, nợ, dự án và save/load. Các lệnh thay đổi tiền/hàng phải có nguồn đối ứng hoặc nguồn tạo/tiêu được khai báo.

| Bài toán con | Input và constraints | Output đúng | Phép kiểm / phụ thuộc |
|---|---|---|---|
| M1.1 Thế giới và quyền sơ khởi | Realm sáu vùng; holding/actor/account với ID hợp lệ | Sở hữu/quyền và view theo đúng chủ thể | ID không nhân đôi; người không có quyền không chi/rút; P1/P2 |
| M1.2 Hạch toán tiền và hàng | Account, GoodId, quantity nguyên, transaction source/destination | Ledger/debit/credit hoặc tiêu thụ có nguồn được ghi | Sum trước/sau; checked arithmetic; reject không sửa sổ một phần; P2/P3 |
| M1.3 Thị trường cơ bản | Buyer/seller, order, giá, quantity, stock/cash, thời điểm mở/đến hạn | Fill/cancel/expire, tiền giữ/hoàn, hàng chuyển và cầu chưa thỏa | Không bán vượt stock; không mua vượt cash; duplicate settle không đổi thêm; P3/P7 |
| M1.4 Tuyến và vận chuyển ban đầu | Hai vùng có đường, lô hàng sở hữu hợp lệ, sức tải/chi phí theo luật | Cargo được gửi, đến hoặc giữ trạng thái chờ rõ ràng | Đường không tồn tại/đóng hoặc thiếu tải không làm hàng biến mất; tổng hàng xuất/đang đi/đến đối chiếu; P9 |
| M1.5 Nợ và nghĩa vụ | Creditor/debtor, principal, kỳ hạn, payment source, tài khoản | Claim/debt được ghi, trả hoặc còn nợ | Nợ không tạo tiền; trả thiếu cập nhật đúng phần; không chuyển nghĩa vụ sang người sai; P1/P3/P7 |
| M1.6 Dự án/hoạt động có đầu vào | Chủ thể, thời gian, tiền/vật tư, mục tiêu và điều kiện hoàn tất | Tiến độ và kết quả chỉ sau khi đạt input thực | Hứa hẹn không thành output; pause/save không chạy bù; P4/P6/P7 |
| M1.7 Save/load nền kinh tế | Realm, ledger, market orders/cargo/debt/project và version | Tiếp tục đúng, migration cũ hoặc từ chối dữ liệu hỏng mà không phá world live | Roundtrip fingerprint; save giữa settle/travel; schema cũ không tạo thêm tiền/hàng; P15 |
| M1.8 Shock/sandbox qua nhiều seed | Scenario/config/seed và biến cố; luật bị khóa trước run | Raw metrics về giá, thiếu hàng, debt, survival và hậu quả | Invariant dừng run khi lỗi; báo mọi seed và phân tán; không chọn một seed thuận lợi làm kết luận; P16 |

**Constraints chưa được phép bịa.** Mỗi loại tiền/hàng và phép làm tròn phải lấy giới hạn từ schema/config thật. “Bảy hàng đầu” là phạm vi catalog ở milestone, không phải giới hạn vĩnh viễn cho M5. Thuế, giá, thời gian vận chuyển và khả năng thanh toán chỉ dùng thông số được duyệt tại plan/contract active.

## 4. M2 — quyền, chính trị, thông tin và vòng đời cá nhân/hộ

**Kết quả sản phẩm.** Danh tính và quan hệ xã hội động; nhiều vai trò/quyền lực có kết quả khác nhau; cá nhân, trẻ em, hộ và vòng đời được duy trì thưa, không làm lộ bí mật.

| Bài toán con | Input và constraints | Output đúng | Phép kiểm / phụ thuộc |
|---|---|---|---|
| M2.1 Danh tính và quan hệ | Person ID ổn định; parent/household/role/office links có provenance | Một cá nhân có thể đổi hộ/nghề/chức mà không bị clone | Kiểm unique ID, relation acyclic/valid theo loại, rebuild sparse index; P1/P14/P15 |
| M2.2 Quyền và tư cách chính trị | Actor, capacity, target, operation, time, consent khi áp dụng | Command hợp lệ thực hiện; lệnh sai tư cách bị từ chối không đổi state | Cặp fixture actor có/không có authority phải cho kết quả khác; không gán chiến lược vào validator; P1/P2 |
| M2.3 Sự kiện và điều mỗi người biết | Fact, source, occurred/received time, observer permissions | Knowledge riêng theo observer, có nguồn/thời điểm/độ trễ | Người chưa nhận tin không biết secret; không nhận trước khi sự kiện xảy ra; P8 |
| M2.4 Hộ, trẻ em và giám hộ | Person/guardian/fund/member data; quyền sở hữu và đại diện riêng | Trẻ giữ tài sản của mình; guardian chỉ quản lý trong phạm vi được phép | Boundary tuổi/năng lực; guardian không tự chuyển tiền riêng; death/transfer/save; P1/P3/P15 |
| M2.5 Hôn nhân, thừa kế và vòng đời pilot | Hai consent, tuổi/năng lực, quan hệ huyết thống/phối ngẫu, nghĩa vụ và người hưởng | Hôn nhân hoặc estate settlement theo luật; người sống và tài sản đồng sở hữu được bảo vệ | Sai consent/kinship/duplicate; nhiều người hưởng; người chưa thành niên; claim thiếu tiền; replay; P1/P2/P3/P7/P15 |
| M2.6 Dữ liệu quan hệ thưa và workload | N người, E relation, scenario/seed | Lưu/search theo dữ liệu thực, không ma trận N×N | Profile A đo thao tác và RAM; kỳ vọng storage theo `O(N+E)` với workload nêu rõ; P14/P16 |

**Ranh giới.** Đây là bước nền, không chứng minh toàn bộ M5. Các luật quyền, consent, tuổi, thừa kế và privacy chỉ thay khi plan có quyết định cụ thể. Feature test phải kiểm song song quyền đúng/sai và view của observer khác nhau.

## 5. M3 — thử nghiệm học quyết định, không phải bản AI phát hành

**Kết quả sản phẩm.** Ba họ policy thử nghiệm, recurrent PPO, runner/curriculum, 12 augment và sáu stat; các run có thể lặp lại và chẩn đoán được. M3 chỉ tạo pilot; learned policy chưa tự được coi là đạt chất lượng sản phẩm.

| Bài toán con | Input và constraints | Output đúng | Phép kiểm / phụ thuộc |
|---|---|---|---|
| M3.1 Định nghĩa observation/action | Snapshot từ core, danh sách candidate, mask, version/contract | Tensor và lựa chọn map về actor/command đúng | Shape không đủ: kiểm nghĩa từng slot, mask, reset/memory và invalid action; P10 |
| M3.2 Ba họ policy và recurrent memory | Cùng environment/data, architecture config/version, per-person memory | PolicyOutput/action và memory tiếp tục đúng cá nhân | Batch reorder không trộn actor; reset ở episode/end theo rule; deterministic sampling fixture; P11/P13 |
| M3.3 PPO runner và rollout | Transition từ C++ core qua binding; seed, model version, log-prob/reward/outcome/time/reset | Dataset rollout có lineage để tái tạo update | Không dùng luật kinh tế Python; rollout phải ghi trạng thái kết thúc/reset; P12/P16 |
| M3.4 Train/validation/holdout | World seed/scenario family được tách trước khi chọn model | Checkpoint chọn bằng validation; holdout độc lập để báo cáo | Không train/điều chỉnh bằng holdout; paired comparison và số run đủ theo gate; P12 |
| M3.5 Curriculum | Tình huống cá nhân/hộ/nơi làm → vùng → nhiều vùng; luật/control giữ nhất quán | Khó khăn tăng theo phạm vi, không tăng bằng cách tạo shortcut reward | Invariant fail dừng run; metric không thưởng spam hoặc vay làm income; P12/P13 |
| M3.6 Augment/stat và cá nhân hóa thử | 12 augment/sáu stat trong schema rõ; actor/goal/context | Thay đổi đầu vào chỉ ảnh hưởng đúng nhánh đã định nghĩa | Paired ablation, permutation/identity test; shape change tạo model contract mới; P10/P13 |
| M3.7 Chẩn đoán run | Raw transitions, update IDs, seed, policy/model version | Biểu đồ reward/return theo update/decision và metrics theo seed | Plot đúng dữ liệu của run; training return tách khỏi evaluation; báo độ phân tán; P12/P16 |

**Gate.** Chỉ kết luận “pilot đã chạy” khi runner hoàn tất. Chỉ kết luận quality theo các gate RL R0–R5 của IMPLEMENTATION_PLAN khi mọi phép đánh giá tương ứng qua; model export hoặc đường cong reward không thay điều kiện đó.

## 6. M4 — vertical slice native có logistics, xưởng và hồ sơ nhiều phương án

**Kết quả sản phẩm.** Một đoạn chơi native trên sáu vùng, gồm Hắc Sơn, logistics, xưởng, hồ sơ vấn đề và nhiều phương án can thiệp. Người chơi có thể mở bản đồ, gửi lệnh, xem hậu quả có nguyên nhân và lưu/tải. Controller có nhãn rõ; nếu dùng policy pilot thì không gọi là policy production.

| Bài toán con | Input và constraints | Output đúng | Phép kiểm / phụ thuộc |
|---|---|---|---|
| M4.1 Màn chơi, bản đồ và input | Native session, map 6 vùng, actor/view permissions, user command | Bản đồ đúng Realm và command đi qua C++ core | Input focus/pause/selection; UI không ghi trực tiếp tiền/kho/clock/person; P2/P16 |
| M4.2 Sự kiện/Hắc Sơn và hồ sơ vấn đề | Fact có nguyên nhân, tình huống và option phù hợp với state | Case file trình bày thông tin đã biết, hậu quả và options có thể chọn | Không scripted outcome giả làm campaign; secret chỉ hiện khi player biết; P8/P10 |
| M4.3 Logistics qua nhiều vùng | Cargo/owner, route graph, schedule/capacity/fee theo contract | Shipment tiến theo game-time; giao/thiếu/đứt tuyến có kết quả giải thích được | Không teleport; route đóng/thiếu capacity; save giữa chuyến; hàng xuất/đang đi/đến đối chiếu; P9/P15 |
| M4.4 Xưởng và Work chain | Establishment/site, quyền Work, recipe/input, attention/presence | Work có thể tạo output chỉ khi nguồn và điều kiện thật đủ | Reject quyền; vắng mặt; thiếu input; capacity; duplicate/save; core C++ transaction→time→save; P3/P6/P7/P15 |
| M4.5 Nhiều lựa chọn và hậu quả so sánh | Một case cùng state ban đầu, các command hợp lệ khác nhau | Mỗi nhánh có state/result/fact riêng, giải thích trade-off | Paired intervention từ snapshot giống nhau; không nhầm narrative text với sim effect; P2/P8/P16 |
| M4.6 Native view và session save | RealmView/PlayerView, controller/model label, session serialization | UI chỉ đọc view được phép; resume hiển thị đúng world và model status | Native binding, save/load, focus; thiếu model hoặc incompatibility phải hiện lỗi; P11/P15/P16 |
| M4.7 Vertical acceptance chain | Seed/config khóa, một case → lệnh → route/work → consequence → save/load | Cùng một chuỗi có thể lặp và đối chiếu từ đầu tới cuối | Causal chain phải có thể rẽ theo intervention; core test trước, native/Godot kiểm khi bề mặt đổi; P2/P4/P8/P9/P15/P16 |

**Còn phải tra trước khi code/rewrite.** Tên scene, widget, controller/API native, scenario seed và tiêu chuẩn native UI cụ thể phải lấy từ plan/report/source của M4; không phát minh tên class. Bằng chứng M4 cũ không tự xác nhận code HEAD hiện tại.

## 7. M5 — nền xã hội và quốc gia đủ sâu

M5 được chia M5.0/1–M5.5; roadmap, luật, phụ thuộc và điều kiện đóng nằm ở [IMPLEMENTATION_PLAN §§6, 30](IMPLEMENTATION_PLAN.md). Bản đồ kỹ thuật nối M5 với P1–P16 tại [PROJECT_MAP §4–5](PROJECT_MAP.md). Các luồng sản xuất và vấn đề đang OPEN có đặc tả sâu trong [PRODUCTION_BLUEPRINT](PRODUCTION_BLUEPRINT.md). Đây không phải status: trạng thái lấy từ CURRENT_TASK.

| Phần | Kết quả bắt buộc | Các nhánh phải tiếp tục phân rã |
|---|---|---|
| M5.0/1 | Registry/schema/quyền; hộ động, guardian, đất/tài khoản, provenance knowledge, vòng đời | Identity, ownership/authority, consent, migration/save, observer-specific view |
| M5.2 | Establishment, hợp đồng/học nghề, 16 nhóm hàng, source/consumer, market và vận tải | Producer/consumer từng GoodId, Work/capacity/input/storage, cạnh tranh, transaction, route/stock, config/version |
| M5.3 | Tổ chức/chức, hôn nhân/escrow, di sản/nợ/claim, giáo hội | Membership/authority/fund, consent, claims priority, deceased owner and living co-owner, save/replay |
| M5.4 | Quốc gia 27 khu, thành thị/quân sự/ngoại thương, giao diện vua | Policy/governance command, treasury/account, food/route/ration, market impact, view/privacy, native integration |
| M5.5 | Profile B, stress/long-run, performance full cycle và package | Workload/seed manifest, phase profiling, correctness under load, memory/backlog, offline package/platform |

**Không được gộp:** hoàn tất một producer không có nghĩa đủ 27 sản phẩm; core pass không có nghĩa Python/native/Godot pass; mở được model không có nghĩa model quality đạt; mở được training không có nghĩa M5 hoàn tất.

## 8. M6 — một đời vua có thể chơi và kết thúc

**Kết quả sản phẩm.** Một campaign có thể tạo mới, lưu/tải, tạm dừng, kết thúc/ending; tutorial/F1, thông tin, UI, backup/accessibility và bản đồ nghệ thuật giúp người chơi hiểu nguyên nhân và hậu quả. Feature/action/observation/reward/scenario được freeze ở mốc xác định.

| Bài toán con | Input/constraints | Output đúng | Phép kiểm |
|---|---|---|---|
| M6.1 Vòng đời campaign | Scenario, seed, model, save/backup identity | New/load/resume/pause/end đúng một campaign | Save cũ/hỏng, restore, end không chạy thời gian hoặc settlement lần hai |
| M6.2 Tutorial và F1 | Hành động người chơi, mục tiêu bài học, điều kiện bắt đầu/kết thúc | Giải thích thao tác và hệ quả dựa trên state thật | Tutorial không thay luật; bỏ qua/retry/load vẫn đúng; keyboard/accessibility |
| M6.3 Thông tin/decision UI | PlayerView, knowledge, reports, map overlays | Mỗi panel chỉ hiện thông tin được phép và cập nhật đúng sau command | Observer privacy tests; dữ liệu không đủ phải được gọi là chưa biết |
| M6.4 Ending và nguyên nhân | Game events, policy/outcome, kết thúc campaign theo luật đã duyệt | Ending giải thích chuỗi nguyên nhân và trạng thái cuối | Không kết thúc do timeout UI giả; replay từ save cho cùng ending |
| M6.5 Freeze/tích hợp trải nghiệm | Versioned feature/action/obs/reward/scenario, native/Godot bundle | Build sản phẩm tái lập và tương thích model đúng | Smoke toàn luồng new→play→save/load→pause→end; regression focus/input/offline/package |

Chi tiết acceptance và luật ending nằm trong IMPLEMENTATION_PLAN; UI không được làm nguồn chính thức cho simulation state.

## 9. M7 — learned policy production, parity và release

**Kết quả sản phẩm.** Learned policy đã chọn qua đánh giá độc lập; export/manifest và native inference đúng cùng semantics; bản release chạy offline trên platform đã công bố, có evidence chất lượng và hiệu năng.

| Bài toán con | Input/constraints | Output đúng | Phép kiểm |
|---|---|---|---|
| M7.1 Freeze training environment | Core/save/model contract, config, scenario, reward/action/obs, seeds | Immutable run identity và source data provenance | C++→Python parity trước train; lock dependency và version; không dùng ruleset song song |
| M7.2 Tạo train/validation/holdout data | World/scenario seeds tách trước; raw transitions từ C++ core | Mỗi transition có actor, obs/mask, action/log-prob, reward/outcome/time/memory/reset/termination | Không trộn tập; chart run/version; không bịa labels/outcomes |
| M7.3 Train và chọn checkpoint | PPO updates, train split, validation split, initial/baseline | Checkpoint được chọn theo selector đã định trước | Nhiều seeds, dispersion; log training return riêng evaluation; không nhìn holdout để điều chỉnh |
| M7.4 Holdout quality | Candidate, baseline, initial policy, independent world seeds/families | Utility và metric liên quan cùng uncertainty | Gate RL R0–R5; survival/debt/agency/action-use theo câu hỏi; report thất bại và không chọn lại bằng holdout |
| M7.5 Export và runtime parity | Chosen checkpoint, ONNX opset, manifest, native sampler/state | Native result tương đương sampling/model contract kỳ vọng | Torch/ONNX/native paired parity; incompatible contract bị từ chối; memory không trộn actor |
| M7.6 Product release | Windows/offline package, model bundle, licenses/config, performance workload | Có thể cài/mở/chơi và khôi phục save trên platform công bố | Package integrity, TLS/dependency policy, cold start, full-cycle profile, support evidence |

## 10. S — khả năng chạy một triệu cá thể

Đây là mục tiêu độ lớn, không phải một milestone được coi là đạt chỉ vì cấu trúc sparse hoặc benchmark synthetic.

| Thành phần | Input/constraints | Output/bằng chứng |
|---|---|---|
| Workload thật | Một triệu identity, số relation/household/office/order và mật độ event nêu chính xác; profile đầy đủ theo ngày/campaign | Dataset/config/seed/workload manifest có thể tái tạo |
| Full simulation cycle | Clock, policy decision, command/transaction, market/route/lifecycle, save/backup | Days/s, decisions/s, latency theo phase, peak/steady RAM, backlog và số invariant |
| Chất lượng | Nhiều seed và scenario; sampling/runtime semantics giống app | Quality không suy từ người sống sót hoặc throughput đơn lẻ |
| Hạn chế | Hardware/toolchain/OS/compiler/build flags và độ dài run | Phân bố và run thất bại; nêu giới hạn extrapolation và bottleneck |

## 11. Cây phân rã bài toán và roadmap liên kết ra sao?

| Bài toán P | Các milestone thường dùng | Câu hỏi đặc trưng |
|---|---|---|
| P1 — identity/authority/ownership | M1, M2, M5, M6 | Ai sở hữu, ai đại diện, người nào còn sống và có quyền gì? |
| P2 — command/atomicity | M0–M7 | Reject có giữ nguyên toàn bộ state/sổ không? |
| P3 — finance/goods/ledger | M1, M4, M5, M6 | Tiền/hàng sinh, giữ, chuyển, tiêu ở đâu và đối ứng gì? |
| P4 — clock | M0, M1, M4–M7 | Tick, ngày, pause, focus, save và phase có đúng một lần không? |
| P5 — attention/schedule | M1–M5, M7 | Các cam kết đồng thời chia thời gian/thực hiện ra sao? |
| P6 — presence/effort/body | M1, M4, M5, M7 | Ai thật sự có mặt, công sức hiệu dụng và cơ thể đổi thế nào? |
| P7 — settlement/obligations | M1, M4, M5, M6 | Ai được trả, nghĩa vụ nào còn thiếu, có trả hai lần sau load không? |
| P8 — facts/knowledge/causality | M2, M4–M7 | Sự việc nào xảy ra, ai biết, nguyên nhân và độ trễ là gì? |
| P9 — logistics/routes | M1, M4, M5, M6 | Hàng đi đâu, mất phí/tải/thời gian nào, ai giữ khi đang đi? |
| P10 — observation/action mask | M0, M3–M7 | Actor được biết gì và có thể chọn lệnh nào? |
| P11 — inference/runtime memory | M0, M3–M7 | Model input/output, memory/reset và invalid action có đúng không? |
| P12 — dataset/PPO/selection | M3, M5, M7 | Dữ liệu đến từ core nào, train/validation/holdout tách sao? |
| P13 — individual differences/agency | M2, M3, M5–M7 | Năng lực, mục tiêu, cá tính làm hành vi thay đổi đo được không? |
| P14 — performance | M0, M2, M5.5, M7, S | Pha nào tốn thời gian/bộ nhớ ở workload đã khai báo? |
| P15 — save/load/migration | M0–M7 | Snapshot nào load được, lỗi có giữ nguyên world live, replay có trùng không? |
| P16 — end-to-end evidence/product | M0–M7, S | Chính xác lớp/version nào đã kiểm, điều đó chứng minh đến đâu? |

Bảng là crosswalk, không thay các nhánh chi tiết trong [TECHNICAL_PROBLEMS](TECHNICAL_PROBLEMS.md). Mỗi P vẫn có thể được dùng ở milestone khác nếu dependency/contract chứng minh cần nó.

## 12. Quy tắc mở nhánh mới và giữ blueprint không lỗi thời

1. Khi IMPLEMENTATION_PLAN thêm/chỉnh milestone, thêm dòng crosswalk ở đây và cập nhật PROJECT_MAP; ghi owner/nguồn trước khi nêu tên code.
2. Khi thêm bài toán P hoặc bài toán con, cấp mã ổn định trong TECHNICAL_PROBLEMS, không tái dùng mã cũ và không đưa trạng thái vào mã.
3. Mỗi leaf mới phải có input/output/constraints/algorithm/success/reject/edge/save/API owner/oracle/dependencies; nếu không đủ, ghi OPEN kèm quyết định cụ thể còn thiếu.
4. Khi code một leaf hoàn tất, cập nhật plan/blueprint tương ứng và CURRENT_TASK; kết quả chạy vào report/HISTORY. Không chỉ cập nhật milestone tổng.
5. Khi implementation thay API/class/module, đối chiếu source và test rồi mới cập nhật tên trong PROJECT_MAP; tên đề xuất trong blueprint không được tự nâng thành tên hiện thực.
6. Chạy kiểm tra docs theo phần đổi. Không build/test toàn game sau lần sửa docs hoặc helper; kiểm liên quan và nâng phạm vi theo DEVELOPMENT_WORKFLOW.

