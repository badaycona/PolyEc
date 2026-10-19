# CURRENT_TASK — bàn giao M5

Nguồn điều hành tác vụ hiện tại. Cập nhật khởi tạo: 2026-10-18. Dành cho agent: đọc file này sau AGENTS, rồi chỉ mở linked contract/source cần cho lát đang làm. Scope toàn dự án: [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md). Lý do/phương pháp: [DEVELOPER_GUIDE](DEVELOPER_GUIDE.md). Kết quả cũ: [HISTORY](DEVELOPMENT_HISTORY.md).

## Handoff docs — thiết kế lại theo người đọc, 2026-10-18

Theo yêu cầu người dùng, ưu tiên thiết kế lại docs trước khi tiếp tục triển khai. Baseline checkout `6cc55ab`; giữ thay đổi phục hồi từ `730d341` đang có trong working tree. README giải thích mục đích/stack/folder tree/giai đoạn; [bản đồ docs](README.md) phân người đọc và nơi sở hữu thông tin; [cây bài toán](TECHNICAL_PROBLEMS.md) nối sáu nhóm lớn → P1–P16 → bước nhỏ/phương pháp/công thức/phép kiểm; [workflow](DEVELOPMENT_WORKFLOW.md) phục vụ Codex/OpenCode theo AGENTS. Player Guide bổ sung đường chơi thử dễ hiểu; History ghi mốc–vấn đề–giải pháp–evidence.

Giữ hướng agent-centric; sáu stat/XP kế thừa không là schema bắt buộc. Handoff docs-only này đã hoàn tất; nó không nghiệm thu thêm model hoặc performance. Trạng thái/gate hiện tại và next action được ghi trong các mục T1 bên dưới.

Quy ước trình bày theo yêu cầu người dùng: công thức dùng LaTeX inline/display, giải thích ký hiệu và đơn vị; không viết phương trình trong backtick. Preview Codex hiện tắt single-dollar math: dùng `$$...$$` cùng dòng văn bản cho inline và cặp `$$` trên dòng riêng cho display; kiểm bằng parser/render thay vì chỉ đếm ngoặc. Xem quy tắc biên tập ở docs/README.

## Quyền thực thi

- OpenCode triển khai chính; Codex review plan, diff và bằng chứng. Mỗi lần một lát có file/gate.
- Plan-only → Codex review → code/test → Codex acceptance. Contract thay đổi lớn phải review lại trước code.
- Không commit/push/publish, train dài hoặc chi phí cloud nếu chưa có yêu cầu tương ứng. Giữ thay đổi người dùng và tác vụ khác.
- Các lời giải thích/đề xuất trong tab thảo luận không tự thành gameplay/spec đã duyệt. M5 chưa hoàn thành.

## R1 — payroll prepare/apply (accepted 2026-10-19)

Refactor kiến trúc core theo [core architecture contract](design/core-architecture-refactor.md) §7.1/§9 đã được Codex nghiệm thu sau final diff review. Baseline `6cc55ab`; toàn bộ dirty T1/T2/docs giữ nguyên. Contract/evidence: [R1 evidence](reports/M5-R1-payroll-prepare-apply-evidence-2026-10-19.md).

- Files: chỉ `core/include/polyec/managers/finance_manager.hpp` và `core/tests/test_managers.cpp`; không đổi `core/src/realm.cpp`, CMake, save schema, native/Godot/Python, action/observation/reward/model hoặc gameplay.
- API: `prepare(day, region, public, claims, people, provinces)` pure trả `PayrollPlan` opaque (private default ctor; chỉ `FinanceManager` friend) và `apply(plan, current_day, people, provinces)` validate read-set rồi commit scalar. Wrapper `payroll(...)` giữ nguyên chữ ký cho `Realm::payroll`; không còn thuật toán payroll song song.
- Behavior giữ nguyên trên input hợp lệ: thứ tự claim và claim trùng, pro-rata integer truncating, rotation theo ngày, budget-before-tax, `tax_remainder` từng người, `tax_receipts` chỉ là counter, treasury vừa trả lương vừa nhận thuế; alias treasury công khai chỉ ghi một lần.
- Boundary/atomicity: validation state-domain và checked arithmetic (due sum, budget*claim, cash+paid, assessed, account/treasury/receipts/finals) chỉ chạy trên due>0 và reject nguyên tử (`out_of_range`/`logic_error`/`overflow_error`). Apply kiểm day, region, account/treasury/tax/receipts, rồi index/id/cash/remainder của từng recipient; lần đọc recipient đầu yêu cầu canonical `Person::id == claim.first`; lookup recipient sparse average-O(1) bằng `unordered_map`, không iterate map để tính/commit.
- Windows gates: Release, Clang 18.1.6 qua Zig, Ninja; configure `build/m5-r1-payroll` thành công ở lát này, final source rebuild exit 0; manager tests pass (101 runtime checks, 19 test functions, cộng compile-time assertion `!is_default_constructible_v<PayrollPlan>`); full CTest 10/10 (9.05 s); scoped `git diff --check` exit 0. Không có gate Linux theo quyết định Windows-only 2026-10-19.
- Không commit/push/train. R1 là refactor kiến trúc, không thêm persistent settle ID, không đổi save format, không phải T2 settlement/gameplay và không đóng M5.

## R2 — Employment transitions (accepted 2026-10-19)

OpenCode triển khai theo plan R2 đã review, giới hạn ở EmploymentManager, Realm transition/counter wiring và test manager/Realm/court. Codex review diff và nghiệm thu sau khi bổ sung preflight Work/Appoint để mọi reject xảy ra trước clear assignment; không đổi gameplay đã chốt, save format, binding, observation, reward hay policy. Chi tiết: [R2 evidence](reports/M5-R2-employment-transitions-evidence-2026-10-19.md).

- Release Windows build pass; manager tests pass; Realm 25,349 checks; Court 196 checks; full CTest 10/10 (9.29 s); scoped `git diff --check` pass, có cảnh báo line endings.
- Các hành vi cũ được giữ: counters và cách rebuild mỗi ngày, thứ tự referral khi bổ nhiệm, lương/notice/từ chức, job fields/timer stale theo đường cũ và save schema. `EmploymentManager` không sở hữu state; Realm vẫn sở hữu người/địa bàn và là command boundary.
- Preflight từ chối overflow ngày/cash trước mutation. Những biên ngày/tiền này không thể tạo thành fixture Realm hợp lệ do giới hạn save/day và invariant tổng tiền; test trực tiếp pure preflight. Integration Realm xác nhận reject giữ nguyên fingerprint, assignment và counters.
- Chỉ Windows được nghiệm thu theo phạm vi người dùng đã chọn; không có gate Linux chặn.

### Gate tiếp theo: R3 — điều phối tiến thời gian (plan-only)

OpenCode khảo sát hẹp cách tách điều phối một ngày khỏi các phép tính nghiệp vụ, rồi trình plan-only theo `design/core-architecture-refactor.md` §7.3/§9 để Codex review trước khi sửa code. Plan phải giữ thứ tự phase, pause, nút Một ngày, hành vi chia nhỏ thời gian và luật hiện tại; làm rõ ownership, reject atomic, save/binding/action/observation/reward/model impact, test và lệnh nghiệm thu. R2 không đóng T2 settlement; câu hỏi gameplay về “work time thực tế” ở dưới vẫn chờ người dùng chốt trước khi triển khai lát settlement.

## Trạng thái đã được ghi nhận

| Phần | Bằng chứng | Phạm vi |
|---|---|---|
| Membership/guardian | [M5-progress](reports/M5-progress.md), Windows 5/5 tại revision báo cáo | Index dẫn xuất/load/reject; không quỹ hộ/thừa kế hoàn chỉnh |
| Copilot core refactor repair | b8160f8 đã push; [report](reports/copilot-core-repair-2026-10-17.md), Windows 6/6 | Manager/WorldState/API/payroll repair; không app/AI build mới |
| Attention evaluator thuần | [plan được review](design/M5-activity-evaluator-plan.md), [report](reports/M5-activity-evaluator-2026-10-18.md): full 7/7, targeted 1/1 sau test-gate fix | Một actor, abstract ticks, checked arithmetic; chưa Realm/clock/save/body/settlement |

Phiên này tạo một handoff checkpoint gồm các lát M5/T1/T2/docs hiện có; commit/remote được xác định từ `git log` và `git status` khi phiên sau tiếp tục. Kiểm status và report/evidence trước thao tác; không suy report là code đã commit hoặc giữ mãi cùng trạng thái. Không chạy lại cả suite cho thay đổi docs-only nếu không có lỗi code mới.

## Lát đã nghiệm thu: T1 — timeline Realm (Windows accepted; Linux deferred)

T1a–T1b2d đã được nghiệm thu core/adapter/native/Campaign/Godot trên Windows (CTest và các suite tương ứng theo evidence từng lát bên dưới); Linux build/runtime parity được hoãn theo quyết định người dùng 2026-10-19 và không chặn M5. Mục dưới giữ contract, gates và evidence lịch sử của T1. R1 và R2 đã accepted; R3 điều phối tiến thời gian là gate plan-only tiếp theo. T2 actual settlement vẫn cần gameplay review trước code, gồm câu hỏi work-time còn mở. File có chữ “implemented” không thay diff/test/acceptance.

### T1a: contract được ghi trong task record, cần kiểm chứng theo gate

- Files: `core/include/polyec/schedule.hpp`, `core/src/schedule.cpp`, `realm.hpp/realm.cpp`, `core/tests/test_schedule.cpp`, `core/CMakeLists.txt`; `native/realm_runtime.cpp` chỉ phần guard khi bị tác động.
- Đơn vị đề xuất trong T1a record: 64 subday units/day, absolute int64 cursor. Đây là fixed point, không phút thật hoặc công thức cơ thể; không đổi ngầm tuổi 18 hay lịch nghiệp vụ hiện có.
- Sparse ScheduleBook theo actor; session có ID, owner, interval, priority/demand, counters và retirement cause/time. Không bản sao money/world hoặc mutable state cho UI.
- Submit/Cancel/Reprioritize qua command/sequence có ownership; record hiện dùng cùng `Person::decisions`. Submit không nhận interval hồi tố. Giới hạn sửa session đã xử lý phải có lý do và được review; không chặn overcommit vì “xấu”.
- `advance_ticks` tích hợp attention theo window, xử lý crossed day boundaries đúng một lần; `advance(days)` đi qua clock domain tương ứng. Không tăng payroll/học/care từ attention trong T1a.
- Save session/cursor/sequence/counters/retirement và sticky contract; load tạm validate trước swap. Record đề xuất save `POLYEC_REALM_7`, pilot policy contract vẫn realm-v6: phải kiểm bằng guard thực, không chỉ đổi tên.
- Khi world đã có timeline semantics mới, old pilot policy không tự chạy. Revalidate native restore/load trước thay live state; phải inventory Python/adapter/Campaign wrapper và test phần bị ảnh hưởng.

Trên đây là bàn giao từ task record, không một lần duyệt gameplay/balance mới. Nếu implementation lệch, cập nhật plan và review; không tự chọn archive có contract dễ hơn.

### Gate cần cho T1a trước acceptance

1. Nhiều phiên một actor, compatible/exclusive, overcommit, priority/ties/counters; input order không chọn thắng.
2. Reject wrong owner/ID/dead actor, invalid range/rate, stale/duplicate, overflow; fingerprint hoặc toàn state liên quan không đổi, lý do đúng validator cần kiểm.
3. Advance qua ranh giới ngày và horizon splitting tại non-event boundary; không double daily/settlement, không replay attention đã tích lũy. Save/load giữa interval khớp continuation.
4. Session owner chết/retired và canceled/reprioritized có quy tắc audit; không tự xóa chứng cứ hoặc nhận credit sau chết.
5. Pilot model guard thực trên inference/restore; save cũ/new và Campaign/binding có behavior rõ. Không silent compatibility từ shape giống nhau.
6. Unrelated commands giữ job theo contract được review; nếu sửa cổng busy/candidate/observation thì mở lại version/adapter tests đúng phạm vi, không giữ behavior sai chỉ để giữ test count.
7. Windows related suite + regression thực; Python/native/clock tests khi tác động. T1 chưa đóng nếu T1b pause/focus/units path trên Windows còn mở. Linux parity nằm ngoài phạm vi build/nghiệm thu hiện tại theo quyết định người dùng.

### Ngoài scope T1a

Không nhận attention như wage/learning/care/restoration. Không tự thiết kế fatigue/illness/death rates, reward hoặc chiến lược. Không train/model export/profile B khi chưa có tác vụ tương ứng. Chưa gọi nhiều hoạt động gameplay hoàn chỉnh chỉ từ schedule record.

## Backlog M5 giữ nguyên scope

| Gate | Kết quả cần có | Trạng thái bàn giao |
|---|---|---|
| T1a | Realm schedule/command/time/save + guards | Core Windows nghiệm thu 8/8 CTest; xem [evidence](reports/M5-T1a-evidence-2026-10-18.md). Native/Python actual parity tiếp tục ở T1b |
| T1b | Native clock/pause/focus, explicit units và adapter/binding parity trên Windows | Windows gates đã nghiệm thu; theo quyết định 2026-10-19, Linux build/runtime parity được hoãn và không chặn M5 |
| T2 | Actual presence/attendance/progress/output/settlement; no double credit | Foundation accepted ([plan](design/M5-T2-employment-classification-plan.md), [evidence](reports/M5-T2-employment-classification-evidence-2026-10-19.md)). Actual settlement còn mở |
| T3 | Sleep/exertion/recovery, care/lifecycle và personality/knowledge theo contract | Còn mở |
| M5.0/1 | Registry/schema/quyền; hộ động/quỹ/đất/địa vị/guardian, bảo toàn/save | Còn mở |
| M5.2 | Cơ sở/hợp đồng/học nghề, đủ 16 hàng, vận tải/thị trường, A/B | Còn mở |
| M5.3 | Tổ chức/chức, cưới/escrow/consent, estate/claim/nợ, giáo hội, C/D/E | Còn mở |
| M5.4 | Quốc gia 24+3, thành thị/quân sự/ngoại thương/UI vua, 3 causal chain | Còn mở |
| M5.5 | B/100k, stress/long-run, full-cycle perf và Windows package | Chưa có nghiệm thu M5 |
| Shared core/Python | Python gọi cùng C++ core; binding/parity/guards được kiểm trên Windows theo revision | Còn mở cho world M5; không yêu cầu Linux build/runtime trong phạm vi hiện tại |

[Execution tracker của tab chính](handoff/M5-execution-tracker.md) được giữ tại đường dẫn cũ để tránh đụng tác vụ đang chạy. Đây là working ledger hỗ trợ; sau safe handoff đưa trạng thái/next action vào file CURRENT_TASK này, không duy trì hai nguồn chỉ đạo cạnh tranh.

## Mẫu plan trước code

```text
Task / baseline revision / dirty files cần giữ:
Before → after:
Files + class/functions; owner/state/index; units:
Success / reject nguyên tử / accepted-but-failed:
IDs, quyền/consent, chết, shortage, stale/duplicate, cạnh tranh, overflow:
Save / Campaign / binding / action / observation / reward / model impact:
Tests + commands + platform; chưa xác minh; ngoài scope:
Review: pending | approved-by-Codex + ngày + điều kiện
```

## Commands Windows core đã có

```powershell
& build/tooling/cmake/data/bin/cmake.exe -S . -B build/m5-activity-evaluator -G Ninja -DCMAKE_MAKE_PROGRAM=D:/Code/PolyEc/build/tooling/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=D:/Code/PolyEc/build/tooling/zig-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=OFF -DPOLYEC_BUILD_PYTHON=OFF
& build/tooling/cmake/data/bin/cmake.exe --build build/m5-activity-evaluator --parallel 4
& build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-activity-evaluator --output-on-failure
```

Giữ build dir/compiler đã cấu hình trừ khi task cần dir riêng. Flags OFF chỉ kiểm core; nếu thay native/binding phải thêm lệnh/gate cụ thể trước code. Test không dùng `assert` bị Release/NDEBUG vô hiệu; test reject không được pass nhờ stale sequence sai fixture.

## T1b1 — core review accepted 2026-10-19

- Codex tiếp tục qua OpenCode tại baseline `6cc55ab`, giữ nguyên dirty docs của người dùng. OpenCode sửa nhánh long-gap bỏ qua Pause failure; Codex duyệt plan rồi review diff. Contract: [T1b1](design/M5-T1b1-realm-clock-plan.md); [evidence mới](reports/M5-T1b1-review-2026-10-19.md).
- Codex rebuild toàn bộ `build/m5-t1b` và chạy CTest độc lập: exit 0, 9/9 pass; chạy trực tiếp clock test: 231 checks, exit 0. Diff check trong phạm vi sửa pass. Đây là acceptance core T1b1; không chứng nhận native/Python, T1b2 hoặc M5.
- Lỗi Pause do tràn revision được throw trước khi đổi controller state; không có cơ chế latch hoặc ép Realm thành paused khi command reject.
- **Follow-up at that time (completed):** OpenCode surveyed T1b2 and presented the native/Campaign/Godot clock and Python parity plans; subsequent accepted slices are recorded below.
- OpenCode model cấu hình `step-5-preview-free` không còn được CLI nhận diện; lượt này dùng `start-opencode.ps1 run -m opencode/beam-free`, không đổi cấu hình. Không commit/push/train.

### Bản bàn giao trước review (giữ để đối chiếu)

- Build dir: `build/m5-t1b`, Release, Clang/Zig, Ninja.
- New files: `core/include/polyec/realm_clock.hpp`, `core/src/realm_clock.cpp`, `core/tests/test_realm_clock.cpp`.
- Modified: `core/include/polyec/realm.hpp`, `core/src/realm.cpp`, `core/CMakeLists.txt`.
- Contract doc: `docs/design/M5-T1b1-realm-clock-plan.md`.
- CTest: 9/9 passed (`core_tests`, `trade_tests`, `realm_tests`, `court_tests`, `campaign_tests`, `manager_tests`, `activity_tests`, `schedule_tests`, `realm_clock_tests`); `realm_clock_tests` reports 170 checks, and `check()` now throws `std::runtime_error` on failure.
- Scope delivered: Realm-owned paused flag/revision (default bare Realm unpaused), `RealmClockCommand` Pause/Resume with stale/overflow/unknown guards, paused-aware `advance_ticks`/`advance`, `RealmClockController` wall-clock adapter, CLOCK_1 save/load before SCHEDULE_1, reset_after_restore.
- Not delivered: native/Python adapter changes, T1b2, T2, M5 completion. No commit/push/train.

## T1b2a — Python shared-core clock/schedule adapter accepted 2026-10-19

- OpenCode implemented only the approved Python binding and deterministic clock/schedule replay. Contract and run evidence: [T1b2a plan](design/M5-T1b2a-python-clock-plan.md), [report](reports/M5-T1b2a-evidence-2026-10-19.md).
- Codex independently rebuilt core (Ninja reported no work), ran full CTest: 10/10 passed in 7.64s; rebuilt Python binding and transition replay; existing Python binding replay passed 258 checks (2048 partner decisions, no neural inference); clock binding passed 103 counted checks and matched seven checkpoints against exact C++ serialization and sorted session records. Scoped diff check passed.
- **Follow-up at that time (completed):** OpenCode planned and delivered the native RealmRuntime clock integration while preserving learned-decision ordering; later Godot/Campaign slices are recorded below.
- T1b2a does not establish native learned-policy parity, Godot behavior, Campaign pause, portable-platform parity, T2 or M5 completion. No commit/push/train.

## T1b2b — native RealmRuntime pause bridge accepted 2026-10-19

- OpenCode added pause/resume delegates and read-only clock state to `RealmRuntime`; day `advance` rejects pause and maximum-horizon overflow before policy inference. It keeps load unpaused and retains Governance → Economic → Civilian inference before each Realm day. Contract and evidence: [T1b2b plan](design/M5-T1b2b-native-runtime-clock-plan.md), [report](reports/M5-T1b2b-evidence-2026-10-19.md).
- Codex independently ran the native runtime contracts with the three local realm-v6 ONNX models: 75 checks passed; native `runtime_contracts` CTest 1/1 passed; core CTest 10/10 passed; scoped diff check passed. Horizon preflight is code-reviewed but not exercised in a native test fixture.
- **Next action:** OpenCode plan-only for Godot Realm UI/session integration: synchronize UI pause/run/focus/save/restore with RealmRuntime clock, preserving explicit day-step semantics and inference order. Campaign's separate clock and portable Python parity remain open. T1/T2 and M5 remain incomplete.
- No commit/push/train/cloud.

## T1b2c — Godot Realm manual day/clock synchronization accepted 2026-10-19

- Codex reviewed and accepted the Godot Realm slice after the final fail-closed fixes. Contract/evidence: [T1b2c plan](design/M5-T1b2c-godot-clock-plan.md), [report](reports/M5-T1b2c-evidence-2026-10-19.md).
- Delivered: core `StepOneDay`; native `RealmRuntime::step_one_day`; GDExt clock bindings; interactive load paused; UI run/focus/long-frame/guide/save/load/manual-day/close/back-to-map synchronization. Failed long-frame, close and back-to-map pauses keep the panel visible with the pause error.
- Final Windows evidence: CTest **11/11**, Native **19/19**, UI **18/18**, Campaign **39/39**, Realm **471/471**; scoped `git diff --check` exit 0. No commit/push/train.
- T1b2c không đóng T1b (Campaign clock, portable parity), T2 hoặc M5. Giữ dirty/user files; không commit/push/train.

## T1b2d — Campaign clock/manual day accepted 2026-10-19

- Codex reviewed the OpenCode implementation against the user-approved Campaign semantics and supporting contract: [T1b2d plan](design/M5-T1b2d-campaign-clock-plan.md), [evidence](reports/M5-T1b2d-evidence-2026-10-19.md).
- Delivered: Campaign delegates pause ownership to Realm; automatic advance rejects paused state; manual day runs the labeled M4 controller and shared daily settle path once; interactive start/load pause; Campaign UI and app focus synchronize with core pause. Save schema and gameplay rules are unchanged.
- Windows verification: CTest 11/11, Native 19/19, UI 18/18, Campaign 49/49, Realm 471/471; independent targeted Campaign CTest 1/1 with 214/214 checks; profile workload restored with invariant true; scoped diff check passed. The Campaign wrapper horizon guard was reviewed in code but lacks a dedicated boundary fixture.
- T1b2d không đóng T2 hoặc M5. Tại thời điểm nghiệm thu, Linux parity còn được xem là mở; quyết định phạm vi Windows-only ngày 2026-10-19 hoãn gate đó. Giữ dirty/user files; không commit/push/train.

## Cập nhật phạm vi nền tảng — 2026-10-19

- Người dùng xác nhận chỉ cần build và kiểm chứng Windows; Linux build, runtime parity và package Linux được hoãn, không là gate chặn T1b/M5 hiện tại.
- Giữ kiến trúc Python dùng cùng C++ core qua pybind11; các binding và luật chung vẫn phải được kiểm trên Windows. Quyết định này chỉ thay phạm vi nền tảng nghiệm thu, không thay gameplay, model hay chất lượng M5.
- Helper Linux-only T1b2e chưa được nghiệm thu và đã được gỡ cùng golden fixture của nó; các kiểm Windows đã có ở T1b2a–T1b2d vẫn là evidence riêng theo revision.
- Người dùng chốt cả ba vai trò (lao động, giáo viên, quan chức được bổ nhiệm) trong hierarchy; Codex review và nghiệm thu foundation Employment/Job classification trên Windows (realm tests 25,322 checks, full CTest 10/10, Python smoke pass). Contract và evidence: [T2 employment plan](design/M5-T2-employment-classification-plan.md), [report](reports/M5-T2-employment-classification-evidence-2026-10-19.md). Read-only: `Job` query-only với singleton dispatch, `describe_job` pure resolver, `job_descriptor` trả `const Job*` (nullptr = Idle/None), Python person dict thêm `employment_state`/`job_type` đọc không đổi. Không schema/lifecycle/payroll/counter/RL change.
- **T2 settlement còn hàng đợi:** Foundation Employment/Job classification và R2 Employment transitions đã accepted. Actual work-time accounting và end-of-day production/payment/material settlement chưa khởi sự. R3 — điều phối tiến thời gian — là gate plan-only kế tiếp; settlement cần gameplay review trước khi implement. Câu hỏi còn mở: thế nào là “work time thực tế” — chỉ explicit Work schedule block, unscheduled gap, hay đủ ngày khi Employed? Docs không tự trả lời; chờ người dùng chọn.

### Kiểm kê contract trước mỗi lát có tác động xuyên hệ thống

Bảng này là checklist bàn giao, không là tác vụ triển khai mới song song với tab chính. File/gate chính xác lấy từ supporting plan đang active; không mở rộng lát đang làm để sửa mọi hàng cùng lúc.

| Bề mặt | Phải ghi rõ trong plan/evidence |
|---|---|
| Domain và quyền | Ai sở hữu record/tài khoản; ai được sửa; nguồn truth và index dẫn xuất |
| Clock và hoạt động | Tick/ngày, phase boundaries, pause, pending và đã xử lý; split continuation |
| Transaction | Phạm vi mutate; reject so với nỗ lực thất bại; sequence/settlement ID; overflow |
| Save/load | Field/schema đổi; save cũ/new; parse tạm; corrupted reference/count; continuation |
| Campaign/Godot | Wrapper và controller có gọi đúng clock/command; UI chỉ nhận view theo quyền biết |
| Python/native | Binding có gọi cùng transition; restore không vượt guard; GIL/thread ownership |
| RL | Action/observation/reward/cadence/partner đổi hay giữ; bundle nào được phép; mask/hidden/reset |
| Chất lượng test | Fixture chạm đúng validator; numerical oracle; Release checks còn hoạt động; regression liên quan |
| Bằng chứng | Revision/dirty scope, lệnh/exit/platform, counts/workload, phần không chạy và gate còn mở |
| Docs | Scope trong plan; phương pháp trong dev guide; tính năng chơi trong player guide; acceptance vào history/report |

Phân rã kỹ thuật theo P1–P16 và công thức/giới hạn xem [DEVELOPER_GUIDE](DEVELOPER_GUIDE.md); chi tiết vùng, thẻ, thông tin, chính trị, kinh tế và UI xem phần II [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md). Không lấy tham số cân bằng v3 làm lựa chọn đã review cho timeline mới.

## Cập nhật khi bàn giao

Ghi source/revision + dirty scope, plan được review, diff, commands/exit/test counts, evidence, giới hạn và một next action rõ. Đánh dấu accepted chỉ sau review và test thật. Đưa việc đã xong vào HISTORY, số liệu vào reports. Xem archive chỉ khi cần provenance; không thực hiện prompt lịch sử.
