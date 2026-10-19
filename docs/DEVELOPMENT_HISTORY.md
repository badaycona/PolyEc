# PolyEc — lịch sử phát triển

Nhật ký giải thích **vấn đề đã gặp, lựa chọn xử lý, mốc kỹ thuật và giới hạn kết quả**. Đọc bảng dưới để định vị, rồi đọc entry/report tương ứng. Đây không phải backlog hay nguồn lệnh; trạng thái hiện hành ở [CURRENT_TASK](CURRENT_TASK.md). Evidence chỉ áp dụng revision/platform/workload được báo cáo.

## 2026-10-18 — sửa inline math trong Preview Codex

Người dùng báo nhiều inline không render. Kiểm trước chỉ ghép delimiter/ngoặc chưa phát hiện lỗi tương thích Markdown: renderer cài trên máy cấu hình remark-math `singleDollarTextMath: false`, trong khi tài liệu dùng `$...$`. Đổi 244 biểu thức inline của TECHNICAL_PROBLEMS, DEVELOPER_GUIDE và IMPLEMENTATION_PLAN sang `$$...$$` cùng dòng văn bản; giữ 66 khối display và thêm lời dẫn ở đầu đoạn để không bị hiểu thành block. Hai nhãn tiếng Việt trong các khối bị KaTeX strict báo unknown Unicode được đổi thành nhãn ASCII, phần giải thích tiếng Việt ngoài công thức giữ nguyên.

Kiểm lại bằng remark-parse + remark-gfm + remark-math đúng cấu hình trên, rồi rehype-katex và KaTeX strict: nhận đủ 244 inline, 66 display, không parse/render errors hoặc warnings. Đây là kiểm pipeline Markdown/LaTeX tương ứng cấu hình tìm thấy, chưa là quan sát trực tiếp toàn bộ trang Preview trong app. Dependency kiểm tạm nằm ngoài repository; không thay dependency dự án hoặc source gameplay.

## Bản đồ các mốc và bài học

| Mốc | Vấn đề/động lực | Hướng xử lý và kết quả ghi nhận | Evidence/giới hạn |
|---|---|---|---|
| Native/shared core, 14/10 | Train và app có thể lệch luật; prototype nhỏ không đại diện quốc gia | C++ core chung, Godot UI, Python binding, ONNX inference; roadmap M0–M7/S | Thiết kế nền, không chứng nhận mọi milestone hoàn thành |
| M3 pilot RL | Pipeline chạy nhưng governance/goal response và noninferiority chưa đủ | Tách quality gates khỏi việc train/export; giữ audit xấu để định hướng thí nghiệm | [M3](reports/M3-evidence.md); chưa RL production |
| M4 native, 15/10 | Cần nối quyền, tuyến, hàng, nợ và UI thành vòng chơi | Hắc Sơn, logistics hữu hạn, xưởng dệt và package Windows | [M4](reports/M4-evidence.md); controller có nhãn, chưa profile B |
| Membership/guardian M5 | Hộ không liền ID, guardian chết/thiếu, quyền sở hữu dễ nhầm | Index dẫn xuất, ID bền, reject và corrupt-load tests | [M5 progress](reports/M5-progress.md); chưa quỹ hộ/di sản đầy đủ |
| Review plan quỹ hộ | Consent/reservation/model compatibility chưa đủ | Chặn triển khai plan thiếu contract; yêu cầu sửa trước code | Entry 15/10 bên dưới; không lấy plan cũ làm tác vụ active |
| Core repair, 17/10 | Patch text vào source, sim/command sai chỗ, payroll sai thuế/rounding | OpenCode giữ intent refactor, sửa behavior, thêm test manager; Codex review/rerun | [Repair](reports/copilot-core-repair-2026-10-17.md), b8160f8, Windows 6/6; chưa app/AI build mới |
| Attention, 18/10 | Busy đơn giản không đủ cho nhiều hoạt động; tie/splitting cần semantics rõ | Evaluator thuần, counters riêng, priority, checked arithmetic; sửa test-gate | [Evaluator](reports/M5-activity-evaluator-2026-10-18.md), 7/7 rồi targeted 1/1; chưa body/output |
| T1a, 18/10 | Attention chưa nối cursor/command/save/vòng đời | Realm schedule, save v7, continuation, retirement và guards | [T1a](reports/M5-T1a-evidence-2026-10-18.md), core Windows 8/8; adapter/parity còn riêng |
| T2 employment foundation, 19/10 | Cần trạng thái Employed/Idle và nghề cụ thể cho settlement sau | Job query-only hierarchy (Worker/Teacher/AppointedOffice), `describe_job` pure resolver, Python person-dict read-only; không schema/lifecycle/payroll/RL change | [T2 plan](design/M5-T2-employment-classification-plan.md), [evidence](reports/M5-T2-employment-classification-evidence-2026-10-19.md), realm 25,322 checks, CTest 10/10; actual settlement còn mở |
| T1b2d, 19/10 | Campaign UI và core cần một nguồn pause, manual day phải chạy đủ daily effects đúng một lần | Campaign giao clock cho Realm; UI start/load/focus pause; chia sẻ settlement giữa automatic/manual day | [T1b2d](reports/M5-T1b2d-evidence-2026-10-19.md); Windows suites pass; horizon wrapper guard reviewed, no direct boundary fixture |
| Phạm vi M5 Windows-only, 19/10 | Linux parity cần môi trường Linux riêng trong khi người dùng chỉ yêu cầu Windows | Hoãn Linux build/runtime/package; giữ shared C++ core và Python binding, kiểm trên Windows | Quyết định người dùng; T1b Windows gates có evidence riêng tại T1b2a–T1b2d |
| Docs, 18/10 | Rút gọn làm mất đặc tả/công thức; lẫn thiết kế với trạng thái | Phục hồi từ 730d341, giữ agent-centric, sửa archive claims, phân docs theo người đọc | Các entry docs bên dưới; kiểm tài liệu không là test gameplay |

## 2026-10-18 — thiết kế lại bộ docs theo người đọc

Bổ sung theo yêu cầu trình bày: chuyển công thức text/code trong cây bài toán và plan sang LaTeX, tách phương trình ra khối và giải thích ký hiệu. Kiểm delimiter/ngoặc: TECHNICAL_PROBLEMS có 17 khối, DEVELOPER_GUIDE giữ 48 khối, IMPLEMENTATION_PLAN có 1 khối; biểu thức inline cũng ghép cặp. Link checker vẫn đạt 383 tham chiếu; không tuyên bố đã kiểm render trực quan chỉ từ kiểm cú pháp nguồn.

Người dùng yêu cầu bộ docs phải bao phủ mục đích/stack/folder tree/giai đoạn, hướng dẫn chơi dễ tiếp cận, phân rã kỹ thuật từ lớn tới nhỏ có phương pháp/công thức, hướng sản phẩm, công cụ làm việc cho Codex/OpenCode và lịch sử vấn đề/giải pháp. Không giới hạn số file theo khuôn năm tài liệu.

Giải pháp: mở rộng README thành trang tổng quan; docs/README là bản đồ người đọc và nơi sở hữu thông tin; thêm TECHNICAL_PROBLEMS chia sáu nhóm lớn thành P1–P16 và bước nhỏ, dẫn paper sâu; thêm DEVELOPMENT_WORKFLOW diễn giải AGENTS; bổ sung lộ trình chơi thử/thuật ngữ/xử lý vướng mắc trong PLAYER_GUIDE; thêm bảng mốc–vấn đề–giải pháp–evidence ở đây. Giữ plan/paper chi tiết đã phục hồi và trạng thái gate kỹ thuật hiện tại. Không đổi gameplay, không giao coding mới.

Kiểm chứng tài liệu: `python scripts/check-repository-links.py` exit 0, 383 tham chiếu local tài liệu/resource hợp lệ; `git diff --check` exit 0 (Git chỉ nhắc chuyển LF/CRLF). Kiểm cấu trúc 12 file Markdown đang thay đổi/thêm: đủ P1–P16 với 48 bước nhỏ và 4 bước tích hợp UI; giữ đủ mục plan 1–21, 12 augment và 48 khối công thức paper; 22 thư mục trong folder tree tồn tại. Các kiểm này xác nhận source/path/coverage, chưa phải nghiệm thu render hoặc thẩm định khoa học mọi công thức. Không chạy CTest vì chỉ sửa docs; chưa commit/push.

## 2026-10-18 — phục hồi chi tiết sau bản rút gọn

Theo yêu cầu người dùng, pull fast-forward `730d341` → `6cc55ab`, sau đó phục hồi các phần bị lược từ `730d341`: đặc tả 11–21, quality gates và profile/budget; paper P1–P16, toán/thuật toán, save/parity và giới hạn; lịch sử nghiệm thu cùng liên kết evidence; ma trận tài liệu. Giữ nội dung agent-centric của bản mới, làm rõ learned RL bắt buộc và overcommit hợp lệ. Sáu stat/XP là phương án kế thừa, không bắt buộc schema mới; mapping capability và hệ số cần contract trước code.

Đính chính các tuyên bố trong bản rút gọn: archive chưa được chuyển toàn bộ sang archive_snapshots; file complexity ở snapshot chỉ là stub, không chứa nội dung gốc. Developer Guide không tự trở thành nguồn luật hộ/di sản; không lấy tóm tắt thứ tự chủ nợ làm quyết định đã duyệt. Phục hồi entry T1a và kết quả/giới hạn M3/M4 thay vì thay bằng khẳng định chung. Đây là sửa tài liệu; không có build/train/benchmark hoặc acceptance gameplay mới.

Kiểm tra lần phục hồi: 65 liên kết local trong 7 tài liệu sửa đều tồn tại; đủ mục 1–21, 12 augment và P1–P16; code fences và 48 khối công thức display-math ghép cặp; `git diff --check` không báo lỗi whitespace. Đây là kiểm tra source Markdown, không phải nghiệm thu render hay thẩm định hệ số. Không chạy CTest vì không sửa code.

## 2026-10-19 — T2 employment classification foundation

Người dùng chốt cả ba vai trò trong hierarchy; OpenCode triển khai read-only Job classification (Worker/Teacher/AppointedOffice, singleton dispatch, `describe_job` pure resolver, `job_descriptor` trả `const Job*`, Python person-dict read-only). Codex review yêu cầu sửa `job_descriptor` trả nullptr thay vì mặc định Worker, thêm pure resolver dùng chung, và test deterministic theo court fixture; các sửa đó đã rebuild/test. Nghiệm thu Windows: realm tests 25,322 checks, full CTest 10/10, Python smoke (`employment_state`/`job_type` mới, `REALM_OBS` giữ 512). [Plan](design/M5-T2-employment-classification-plan.md), [evidence](reports/M5-T2-employment-classification-evidence-2026-10-19.md). Không commit; baseline `6cc55ab` và dirty T1 giữ nguyên. Gate tiếp theo: plan-only cho actual work-time accounting và end-of-day settlement (package T2 đã duyệt); M5 còn mở.

## 2026-10-18 — T1a core timeline

OpenCode triển khai Realm schedule/command/game-time/save v7; Codex review và nghiệm thu core Windows, CTest 8/8. Có continuation giữa interval, death retirement, shared command sequence, overcommit và reject/load hỏng có đúng lý do. [Bằng chứng](reports/M5-T1a-evidence-2026-10-18.md). Native/Python parity và T1b clock/pause chưa đạt; M5 chưa hoàn thành.

Đây là nhật ký theo thời điểm, không phải lệnh triển khai. Đọc [CURRENT_TASK](CURRENT_TASK.md) để tiếp tục công việc và [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md) để biết scope đích. Kết quả cũ chỉ áp dụng revision/workload được dẫn, không tự chứng nhận HEAD hiện tại.

## 2026-10-14 — hướng native/OOP/RL và roadmap v3

Hợp nhất đích một quốc gia, một đời vua; C++ core, Godot native, Python shared-core training và ONNX CPU. Roadmap M0–M7 và nghiên cứu mở rộng S phân biệt nền engine, pilot RL và production training. Mô tả cũ về prototype 4–6 vùng không là quyết định giảm scope quốc gia.

## M3 — pilot khả thi và các kết quả chưa đạt

Đã có sandbox sáu lãnh địa + đất triều đình, khoảng 10k người, ba họ policy và đường recurrent PPO→ONNX/native. Báo cáo giữ cả audit xấu: governance không cải thiện so untrained trong categorical audit; các họ chưa qua đầy đủ baseline noninferiority; default cao, goal/INT response yếu; sandbox dài chưa chứng minh xã hội cân bằng.

Chi tiết, số seed, checkpoint, CI/platform và chất lượng: [M3 evidence](reports/M3-evidence.md). Artifact realm-v1…v6 là pilot/source-locked experiments; không model release mặc định.

## 2026-10-15 — M4 native và nền xã hội M5

M4 có map, quyền đèo, logistics hữu hạn, Hắc Sơn/nợ và xưởng dệt. Core/native/UI/Python/package Windows được ghi trong [M4 evidence](reports/M4-evidence.md); việc thường lệ dùng controller có nhãn, không chứng nhận RL production. Profile M4 khác workload ONNX và chưa B/C.

Người dùng chốt luật chung toàn quốc, bình đẳng giới, consent hôn nhân, tách tài sản/tước/chức, giáo hội giả tưởng và vai trò vua. Duyệt M5 với 24+3 khu, 16 hàng và profile B; ưu tiên Windows. Ngày 19/10, người dùng làm rõ chỉ cần Windows, vì vậy Linux build/runtime/package parity được hoãn; kiến trúc shared C++ core/Python binding vẫn giữ. Production training sau feature freeze.

OpenCode là bên triển khai chính; Codex giao lát nhỏ, review plan rồi diff/evidence. Membership hộ thưa/guardian được nghiệm thu, giữ realm-v6 save vì index dẫn xuất. Có test hộ không liền ID, guardian chết/thiếu, wrong-target/stale và corrupt load. [M5 progress](reports/M5-progress.md), commit [d5751bb](https://github.com/badaycona/PolyEc/commit/d5751bb).

Plan quỹ hộ đầu tiên chưa được duyệt code vì consent/reservation/model compatibility chưa đủ. Các yêu cầu cũ chặn nhận việc vì xung đột thời gian sau đó bị thiết kế 17/10 thay thế; không quay lại plan đó chỉ vì nó có sẵn trong repo.

## 2026-10-17 — làm rõ mô phỏng và sửa core refactor

Duyệt hướng nhiều phiên/ngày; tách intention, attention, presence, service và output; sleep/exertion/health riêng; cho nhận quá tải, thực hiện có giới hạn và hậu quả. Không hard-gate lựa chọn kém khôn ngoan; world không tự chọn chiến lược. Hệ số cơ thể/cadence mới chưa được chốt.

Người dùng xác nhận các commit refactor core tới a5647a5 do GitHub Copilot thực hiện. Sau pull phát hiện patch text thay source và sim/command sai vị trí, cùng payroll extraction sai thuế/rounding. OpenCode sửa giữ intent manager/WorldState, bỏ public mutable access, khôi phục behavior và thêm test manager. Codex review/rerun Windows CTest 6/6. Đã push [b8160f8](https://github.com/badaycona/PolyEc/commit/b8160f88379418c168f53fd7f951e0772075fb0f). [Evidence](reports/copilot-core-repair-2026-10-17.md).

## 2026-10-18 — attention evaluator và tác vụ tiếp theo

OpenCode triển khai evaluator stateless một actor, immutable sessions, absolute half-open horizon, rate capacity 1000, explicit priorities, overcommit hợp lệ và counters phân biệt capacity/unresolved. Tick trừu tượng, không sinh output/care/health hoặc tự chọn thắng tie theo ID.

[Report](reports/M5-activity-evaluator-2026-10-18.md) ghi full CTest 7/7 Codex chạy độc lập, rồi targeted 1/1 sau sửa size gate của split-additivity test. Không dùng số test này chứng nhận clock/save/binding hoặc profile B. Lát tiếp theo T1 nối Realm/command/time/save/version; công việc tab chính tiến tiếp theo [CURRENT_TASK](CURRENT_TASK.md). Không cố định trạng thái dirty/commit của tác vụ đang chạy vào nhật ký này.

## 2026-10-18 — chuẩn hóa tài liệu theo yêu cầu người dùng

Tạo năm nguồn chuẩn: IMPLEMENTATION_PLAN, PLAYER_GUIDE, DEVELOPER_GUIDE theo dạng paper, DEVELOPMENT_HISTORY và CURRENT_TASK. README/WORK_STATUS/AGENTS dẫn tới chúng. Nội dung design/developer/experiments/handoff cũ vào archive với đường dẫn cũ làm redirect; giữ nguyên file tác vụ mới mà tab chính đang làm.

Reports giữ evidence theo run; guides `.txt` giữ input đóng gói theo phiên bản. Checkpoint/source-lock/manifest/artifact không xóa hoặc sửa để giả tương thích. Bản lưu còn truy nguyên được nhưng không là nguồn lệnh hiện hành. Công việc docs không chứng nhận build/train/benchmark mới và không tự commit/push.

Kiểm tra cho lần tổ chức này: checker repo qua 322 tham chiếu local tài liệu/resource; diff whitespace và JSON OpenCode hợp lệ; 27 bản lưu giữ nguyên prose ngoài banner/đích link; sáu input đóng gói còn nguyên đường dẫn/nội dung; runtime models và artifact dữ liệu không đổi. Không chạy lại CTest gameplay cho thay đổi docs-only, không lấy test do tab chính đang chạy làm bằng chứng riêng của việc tổ chức docs.

## 2026-10-18 — bổ sung lại chiều sâu và độ bao phủ của bộ docs

Người dùng phản hồi Developer Guide thiếu tính technical và toàn bộ docs còn thiếu nhiều nội dung. Nguyên nhân của bản hợp nhất đầu là rút gọn quá mạnh, để chi tiết sản phẩm/phương pháp chỉ còn trong archive. Bản sửa đưa nội dung cần dùng trở lại nguồn chuẩn; archive vẫn là bản lưu truy nguyên.

- IMPLEMENTATION_PLAN thêm đặc tả chi tiết vòng chơi/ending, danh mục 24+3 khu và tuyến, chính trị, thông tin/điều tra, kinh tế, chính sách, quân sự/ngoại thương, UI, đủ 12 augment, stat/traits/XP và sự kiện trồi sinh.
- DEVELOPER_GUIDE phân rã P1–P16, ghi đơn vị/ký hiệu; phục hồi toán hạch toán/lương/thuế, graph logistics, GRU, reward/GAE/PPO/BPTT; bổ sung attention, time splitting, settlement, memory/throughput và phương pháp đánh giá. Công thức đề xuất không được trình bày như implementation đã nghiệm thu.
- PLAYER_GUIDE giải thích hướng chơi toàn bộ bằng ngôn ngữ người chơi, tách bản M4 hiện có khỏi sản phẩm dự kiến. CURRENT_TASK bổ sung checklist tác động domain/clock/save/adapter/RL/test, giữ tác vụ đang làm ở tab chính. docs/README có ma trận bao phủ theo chủ đề.

Giữ rõ hai khoảng trống: dân số bối cảnh atlas v3 chưa có contract ánh xạ sang cá thể profile B; hệ số cân bằng kế thừa chưa được hiệu chuẩn và không tự tương thích timeline subday. Các quality gates, body/reward mới và quy mô chưa đo tiếp tục để mở. Lần sửa này chỉ là docs; không tạo bằng chứng build/train/gameplay mới, không commit/push.

Kiểm tra tài liệu lần bổ sung: 339 tham chiếu local tài liệu/resource hợp lệ; cấu trúc heading/code fence hợp lệ; Developer Guide có 48 khối công thức LaTeX với delimiter ghép cặp. Có đủ 12 định nghĩa augment và 24 lãnh địa trong plan chuẩn. Đây là kiểm tra source Markdown, chưa phải nghiệm thu render PDF hay thẩm định khoa học mọi công thức. Không chạy lại CTest gameplay cho sửa docs.

## 2026-10-19 — nghiệm thu sửa lỗi overflow clock T1b1

Codex review phát hiện nhánh long-gap bỏ qua kết quả Pause: khi revision đạt UINT64_MAX và Realm đang chạy, command reject nhưng controller xử lý như đã pause. OpenCode lập plan được Codex duyệt, sửa để throw trước mọi thay đổi controller state và thêm test overflow Pause/Resume, reject nguyên tử, cùng Pause idempotent. Không đổi schema/model/gameplay.

Codex review diff và rebuild toàn bộ Windows Release `build/m5-t1b`, CTest 9/9 pass (8.93 s), clock executable 231 checks pass. Nghiệm thu core T1b1; native/Python/T1b2/T2 và M5 còn mở. [Evidence](reports/M5-T1b1-review-2026-10-19.md). Giữ dirty docs có sẵn, không commit/push/train.

## 2026-10-19 — nghiệm thu T1b2a Python clock/schedule parity

OpenCode thêm binding Python cho cùng clock/schedule validators của Realm và replay deterministic so toàn serialized state/session fields với C++. Codex review yêu cầu numerical attention oracle, reject atomic, restore giữa ngày và không để detached Python record sửa Realm. Codex độc lập rebuild: full CTest 10/10 (7.64 s), binding replay 258 checks, clock binding 103 checks/7 checkpoint; scoped diff check pass. Chỉ đóng T1b2a Python; native/Godot/Campaign/portable/T2/M5 còn mở. [Evidence](reports/M5-T1b2a-evidence-2026-10-19.md). Không commit/push/train.

## 2026-10-19 — nghiệm thu T1b2b native RealmRuntime clock

OpenCode nối pause/resume qua RealmClockCommand trong RealmRuntime và thêm guard để tiến ngày khi paused hoặc vượt horizon thất bại trước inference. Giữ headless load unpaused và thứ tự learned decisions theo họ. Codex review diff sau sửa stale-resume fixture; độc lập chạy runtime contracts 75 checks, native CTest 1/1, core CTest 10/10 và scoped diff check. Horizon guard chưa có fixture runtime ở giới hạn tối đa; còn được ghi là giới hạn. T1b2b native accepted; Godot/Campaign/portable/T2/M5 chưa đóng. [Evidence](reports/M5-T1b2b-evidence-2026-10-19.md). Không commit/push/train.

## 2026-10-19 — nghiệm thu T1b2c Godot Realm clock/manual day

OpenCode nối Realm `StepOneDay` qua native runtime/GDExt vào UI Godot: load tương tác tạm dừng, chạy chỉ resume khi core chấp nhận, focus-out/guide/long-frame/save/load/đóng panel dừng đúng thứ tự, và “Một ngày” tạm dừng loop trước khi tiến đúng 64 subday. Codex review phát hiện các bypass fail-closed ở long-frame, close và nút về địa đồ; bản cuối giữ panel hiển thị cùng lỗi pause thay vì ẩn hoặc ghi đè trạng thái.

Codex accepted T1b2c. Bằng chứng cuối trên Windows: CTest 11/11, Native 19/19, UI 18/18, Campaign 39/39, Realm 471/471; scoped `git diff --check` exit 0. Không commit/push/train. T1b2c không đóng Campaign clock, T2 hoặc M5. [Evidence](reports/M5-T1b2c-evidence-2026-10-19.md).

## 2026-10-19 — nghiệm thu T1b2d Campaign clock/manual day

Campaign dùng Realm làm nguồn sở hữu pause/cursor; UI mở và tải save ở trạng thái pause, lệnh vẫn dùng được khi pause, còn “+1 ngày” chạy controller M4 cùng daily effects đúng một lần rồi giữ pause. OpenCode triển khai theo contract đã được review; Codex kiểm tra diff, chạy lại Campaign CTest độc lập và đối chiếu full Windows gate. CTest 11/11, Native 19/19, UI 18/18, Campaign 49/49, Realm 471/471; Campaign profile 10k đạt invariant. Guard horizon của Campaign wrapper được review trong code nhưng chưa có fixture biên trực tiếp. T1b Windows gates đã nghiệm thu; T2 và M5 vẫn mở. [Evidence](reports/M5-T1b2d-evidence-2026-10-19.md).

## 2026-10-19 — phạm vi M5 chỉ Windows

Người dùng xác nhận không cần build Linux, chỉ cần Windows. Cập nhật task/roadmap để hoãn Linux build, runtime parity và package; không để Linux là gate chặn M5. Giữ kiến trúc Python gọi cùng C++ core qua pybind11 và kiểm binding trên Windows. Helper Linux-only T1b2e chưa được nghiệm thu nên gỡ khỏi working tree, không ghi như kết quả đạt. T1b2a–T1b2d vẫn có evidence Windows riêng; lát kế tiếp là plan-only cho T2 để Codex review trước triển khai. Không đổi luật/gameplay, không commit/push.

## 2026-10-19 — nghiệm thu R1 payroll prepare/apply

OpenCode tách tính lương khỏi mutate trong `FinanceManager`: `prepare` pure trả `PayrollPlan` opaque (private default ctor, chỉ friend), `apply` validate lại read-set rồi commit scalar, wrapper `payroll(...)` giữ chữ ký cũ cho `Realm::payroll`. Codex review plan rồi diff; vòng sửa cuối yêu cầu plan không thể default-construct từ caller, lookup recipient sparse `unordered_map` thay quét tuyến tính, kiểm canonical `Person::id == claim.first`, và thêm test drift cash/remainder cùng overflow cộng final cash/receipts. Behavior trên input hợp lệ giữ nguyên: thứ tự/claim trùng, pro-rata truncating, rotation theo ngày, budget-before-tax, `tax_remainder` từng người, `tax_receipts` counter, treasury alias ghi một lần; reject (`out_of_range`/`logic_error`/`overflow_error`) nguyên tử trước mọi write.

Chỉ sửa `core/include/polyec/managers/finance_manager.hpp` và `core/tests/test_managers.cpp`; baseline `6cc55ab`, dirty T1/T2/docs giữ nguyên; không đổi save schema, `realm.cpp`, CMake, native/Godot/Python/RL. Codex accepted sau final diff review. Windows Release (Clang 18.1.6/Zig, Ninja): rebuild exit 0, manager tests pass (101 runtime checks, 19 test functions, cộng compile-time `!is_default_constructible_v<PayrollPlan>`), full CTest 10/10 (9.05 s), scoped `git diff --check` exit 0. Không có gate Linux theo quyết định Windows-only 2026-10-19. [Evidence](reports/M5-R1-payroll-prepare-apply-evidence-2026-10-19.md). Gate tiếp theo: R2 — Employment transitions, plan-only (survey giới hạn + short plan review trước code); T2 settlement vẫn xếp hàng và câu hỏi work-time còn mở. Không commit/push/train; M5 chưa hoàn thành.

## 2026-10-19 — nghiệm thu R2 Employment transitions

Theo R2 plan đã review, OpenCode tách transition lao động/bổ nhiệm/từ chức và helpers counters vào stateless `EmploymentManager`; Realm giữ quyền sở hữu state, command validation và lựa chọn referral. Codex review phát hiện preflight overflow phải nằm trong `Realm::legal` vì lệnh có thể clear assignment trước transition. OpenCode bổ sung day-aware Work và Appoint preflight, giữ nguyên referral order và không đổi behavior/save/binding/observation/reward/model. Realm integration xác nhận appointment reject giữ nguyên fingerprint, actor assignment và counters. Biên day/cash không dựng được trong valid Realm do bound/invariant nên kiểm pure preflight trực tiếp.

Windows Release build pass; manager tests pass; Realm 25,349 checks, Court 196 checks; CTest 10/10 (9.29 s); scoped `git diff --check` sạch ngoài cảnh báo line endings. R2 accepted. [Evidence](reports/M5-R2-employment-transitions-evidence-2026-10-19.md). Gate kế tiếp R3 — điều phối tiến thời gian, plan-only theo `design/core-architecture-refactor.md` §7.3/§9. T2 actual work-time settlement vẫn chờ gameplay review; Linux không thuộc gate theo quyết định Windows-only. Chưa commit/push tại thời điểm ghi entry; M5 chưa hoàn thành.

## Cách cập nhật nhật ký

Mỗi entry mới nên trả lời: thời điểm/revision nào; vấn đề hoặc mục tiêu gì; nguyên nhân đã xác định tới đâu; giải pháp/đánh đổi nào; bằng chứng/test nào; giới hạn và phần chưa đạt. Có thể dùng đoạn văn ngắn, không bắt buộc một template dài. Nếu sửa kết luận cũ, thêm đính chính có nguồn; không xóa thất bại để tạo lịch sử chỉ toàn thành công.

Thêm sự kiện khi có quyết định, nghiệm thu hoặc thay contract đáng kể. Ghi ngày, revision/source context, điều đã làm, evidence và phần chưa đạt. Không ghi mọi tool call. Nếu phát hiện kết quả cũ sai, thêm đính chính có căn cứ; không xóa dữ liệu xấu hoặc sửa artifact khóa nguồn.
