# CURRENT_TASK — bàn giao M5

Nguồn điều hành tác vụ hiện tại. Cập nhật khởi tạo: 2026-10-18. Dành cho agent: đọc file này sau AGENTS, rồi chỉ mở linked contract/source cần cho lát đang làm. Scope toàn dự án: [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md). Lý do/phương pháp: [DEVELOPER_GUIDE](DEVELOPER_GUIDE.md). Kết quả cũ: [HISTORY](DEVELOPMENT_HISTORY.md).

## Quyền thực thi

- OpenCode triển khai chính; Codex review plan, diff và bằng chứng. Mỗi lần một lát có file/gate.
- Plan-only → Codex review → code/test → Codex acceptance. Contract thay đổi lớn phải review lại trước code.
- Không commit/push/publish, train dài hoặc chi phí cloud nếu chưa có yêu cầu tương ứng. Giữ thay đổi người dùng và tác vụ khác.
- Các lời giải thích/đề xuất trong tab thảo luận không tự thành gameplay/spec đã duyệt. M5 chưa hoàn thành.

## Trạng thái đã được ghi nhận

| Phần | Bằng chứng | Phạm vi |
|---|---|---|
| Membership/guardian | [M5-progress](reports/M5-progress.md), Windows 5/5 tại revision báo cáo | Index dẫn xuất/load/reject; không quỹ hộ/thừa kế hoàn chỉnh |
| Copilot core refactor repair | b8160f8 đã push; [report](reports/copilot-core-repair-2026-10-17.md), Windows 6/6 | Manager/WorldState/API/payroll repair; không app/AI build mới |
| Attention evaluator thuần | [plan được review](design/M5-activity-evaluator-plan.md), [report](reports/M5-activity-evaluator-2026-10-18.md): full 7/7, targeted 1/1 sau test-gate fix | Một actor, abstract ticks, checked arithmetic; chưa Realm/clock/save/body/settlement |

Repo đang được tab chính triển khai tiếp; kiểm `git status` trước thao tác, không suy report là code đã commit hoặc giữ mãi cùng trạng thái. Không chạy lại cả suite cho thay đổi docs-only nếu không có lỗi code mới.

## Lát đang triển khai: T1 — nối timeline vào Realm

T1a có [supporting contract](design/M5-T1a-realm-schedule-plan.md) và [evidence](reports/M5-T1a-evidence-2026-10-18.md); lịch sử/tab chính đã ghi nghiệm thu core Windows với CTest 8/8. Đây là bằng chứng của tác vụ đó, không phải phiên chỉnh docs chạy lại. Native/Python parity, T1b clock/pause adapter và T2 actual settlement là các gate riêng. Trạng thái T1b1 được ghi cụ thể bên dưới; file có chữ “implemented” không thay diff/test/acceptance.

Trước sửa tiếp: xác nhận phần nào trong supporting contract đã được Codex review ở tab chính, code/evidence thực và cập nhật trạng thái dưới đây tại safe handoff. Không mở một lượt OpenCode cạnh tranh cùng file.

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
7. Windows related suite + regression thực; Python/native/clock tests khi tác động. T1 chưa đóng nếu T1b pause/focus/units path và portable/parity còn mở.

### Ngoài scope T1a

Không nhận attention như wage/learning/care/restoration. Không tự thiết kế fatigue/illness/death rates, reward hoặc chiến lược. Không train/model export/profile B khi chưa có tác vụ tương ứng. Chưa gọi nhiều hoạt động gameplay hoàn chỉnh chỉ từ schedule record.

## Backlog M5 giữ nguyên scope

| Gate | Kết quả cần có | Trạng thái bàn giao |
|---|---|---|
| T1a | Realm schedule/command/time/save + guards | Core Windows nghiệm thu 8/8 CTest; xem [evidence](reports/M5-T1a-evidence-2026-10-18.md). Native/Python actual parity tiếp tục ở T1b |
| T1b | Native clock/pause/focus, explicit units và adapter/binding parity | Còn mở |
| T2 | Actual presence/attendance/progress/output/settlement; no double credit | Còn mở |
| T3 | Sleep/exertion/recovery, care/lifecycle và personality/knowledge theo contract | Còn mở |
| M5.0/1 | Registry/schema/quyền; hộ động/quỹ/đất/địa vị/guardian, bảo toàn/save | Còn mở |
| M5.2 | Cơ sở/hợp đồng/học nghề, đủ 16 hàng, vận tải/thị trường, A/B | Còn mở |
| M5.3 | Tổ chức/chức, cưới/escrow/consent, estate/claim/nợ, giáo hội, C/D/E | Còn mở |
| M5.4 | Quốc gia 24+3, thành thị/quân sự/ngoại thương/UI vua, 3 causal chain | Còn mở |
| M5.5 | B/100k, stress/long-run, full-cycle perf và Windows package | Chưa có nghiệm thu M5 |
| Shared core/portable | Python cùng core, parity/guards và portable tests theo revision | Còn mở cho world M5 |

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

## T1b1 implementation submitted for Codex review

- Build dir: `build/m5-t1b`, Release, Clang/Zig, Ninja.
- New files: `core/include/polyec/realm_clock.hpp`, `core/src/realm_clock.cpp`, `core/tests/test_realm_clock.cpp`.
- Modified: `core/include/polyec/realm.hpp`, `core/src/realm.cpp`, `core/CMakeLists.txt`.
- Contract doc: `docs/design/M5-T1b1-realm-clock-plan.md`.
- CTest: 9/9 passed (`core_tests`, `trade_tests`, `realm_tests`, `court_tests`, `campaign_tests`, `manager_tests`, `activity_tests`, `schedule_tests`, `realm_clock_tests`); `realm_clock_tests` reports 170 checks, and `check()` now throws `std::runtime_error` on failure.
- Scope delivered: Realm-owned paused flag/revision (default bare Realm unpaused), `RealmClockCommand` Pause/Resume with stale/overflow/unknown guards, paused-aware `advance_ticks`/`advance`, `RealmClockController` wall-clock adapter, CLOCK_1 save/load before SCHEDULE_1, reset_after_restore.
- Not delivered: native/Python adapter changes, T1b2, T2, M5 completion. No commit/push/train.

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
