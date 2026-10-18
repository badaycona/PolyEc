# M5 — Pure Activity Evaluator (prerequisite slice) — evidence report (2026-10-18)

Scope: [implementation plan](../design/M5-activity-evaluator-plan.md) (approved
contract, corrected under Codex pre-source and pre-acceptance review). OpenCode
implementation; awaiting Codex acceptance review. **No commitment made** — repo
still dirty at review hand-off.

## Deliverables

- `core/include/polyec/activity.hpp` — `Horizon{int64_t begin,end}` absolute
  half-open window (`0 <= begin < end`, no implicit origin 0, no length
  parameter); `SessionView` (immutable POD: positive unique id, matching
  positive actor, nonnegative half-open begin/end, explicit int32 priority,
  demand rate `[1,1000]`); **no per-session capacity field** — capacity is
  constant global `kActorCapacityRate = 1000` attention rate per tick.
  `SessionResult` exact integer counters (`requested/delivered/unmet/
  capacity_unmet/unresolved_unmet`); `RejectReason` enum; `EvaluateResult`
  ok/reason/results.
- `core/src/activity.cpp` — pure stateless evaluator; joins existing
  `polyec_core`. Rejects produce EMPTY output with no input mutation. Priority
  comparator uses comparisons only (no subtraction overflow). All rate sums,
  `rate*len` products and accumulations go through checked int64 add/mul
  helpers; any refusal → `RejectReason::Overflow` with empty output.
- `core/tests/test_activity.cpp` — 18 test functions, hand-computed expected
  integers (no second allocator implementation), runtime checks active in
  Release (plain `check()`/fprintf pattern per `test_managers.cpp`).
- `core/CMakeLists.txt` — `src/activity.cpp` added to `polyec_core`; new
  executable `polyec_activity_tests` + CTest `activity_tests`; existing 6 test
  targets untouched (+5 lines diff).

## Scheduling semantics implemented (per approved contract)

Clipping to `[horizon.begin, horizon.end)`; segmentation at endpoints;
priorities descending per segment; remaining is a RATE in `[0,1000]` that
descends across group consumption (`remaining -= G`, never `G*len`);
`rate*len` multiplication only when accumulating integer units. Group fits →
full; singleton overflow → assigned `remaining` rate then `remaining = 0`
(capacity shortfall); tied overflow with positive remaining → capacity
withheld, all tied + all lower demand unresolved; remaining already 0 from
delivered/assigned higher priority → `capacity_unmet` even for ties. No
ID/input-order winner anywhere. Results sorted by session id; one row per valid
session (zero-total rows for sessions clipped out).

## Verification (Windows local, Release)

Commands run exactly as scheduled:

```
build/tooling/cmake/data/bin/cmake.exe -S . -B build/m5-activity-evaluator -G Ninja
  -DCMAKE_MAKE_PROGRAM=D:/Code/PolyEc/build/tooling/bin/ninja.exe
  -DCMAKE_TOOLCHAIN_FILE=D:/Code/PolyEc/build/tooling/zig-toolchain.cmake
  -DCMAKE_BUILD_TYPE=Release -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=OFF
  -DPOLYEC_BUILD_PYTHON=OFF
cmake.exe --build build/m5-activity-evaluator --parallel 4
ctest.exe --test-dir build/m5-activity-evaluator --output-on-failure
```

Result: **CTest 7/7 passed** (6 pre-existing + `activity_tests`):

| # | Test | Time |
|---|------|------|
| 1 | core_tests | 0.05 s |
| 2 | trade_tests | 0.06 s |
| 3 | realm_tests | 5.55 s |
| 4 | court_tests | 0.54 s |
| 5 | campaign_tests | 0.36 s |
| 6 | manager_tests | 0.03 s |
| 7 | activity_tests | 0.05 s |

**Codex independent verification:** production algorithm accepted; Codex ran
the full 7-suite CTest independently in this build dir — **7/7 pass (6.61 s)**
with no gameplay change.

**Pre-close test-gate fix (per Codex final review):**
`test_horizon_split_additivity` previously guarded the additive assertions only
inside `if (l.results.size() == whole.results.size() && r.results.size() ==
whole.results.size())`, so a size mismatch would silently skip ALL additive
checks and the test would vacuously pass. Two explicit gates added BEFORE the
guard: `check(l.results.size() == whole.results.size(), "split left rows")` and
`check(r.results.size() == whole.results.size(), "split right rows")` — size
mismatch now fails loudly instead of skipping. Production code and the test's
scenario untouched. Rebuilt only the affected target and re-ran the affected
test (full 7 was already independently verified by Codex):

```
cmake.exe --build build/m5-activity-evaluator --target polyec_activity_tests --parallel 4
  → [1/2] Building CXX object core/CMakeFiles/polyec_activity_tests.dir/tests/test_activity.cpp.obj
  → [2/2] Linking CXX executable core\polyec_activity_tests.exe
ctest.exe --test-dir build/m5-activity-evaluator --output-on-failure -R activity_tests
  → 1/1 Test #7: activity_tests ... Passed 0.03 sec; 100% tests passed, 0 failed out of 1
```

Build output: zero warnings from `activity.cpp` / `test_activity.cpp`; only the
pre-existing `std::filesystem::u8path` deprecation warnings in untouched
`realm.cpp` / `campaign.cpp`. Toolchain as configured: Clang 18.1.6 via Zig,
CMake 3.31.6.

## Test inventory (activity_tests)

exclusive full · compatible fit (600+400=1000) · singleton partial (600/600,
partition of 6000 into 4000 delivered + 2000 shortfall) · overcommit valid
(3×500) · tie overflow withholding lower-priority session (no ID winner) ·
remaining==0 after higher full fit ⇒ ties get `capacity_unmet` NOT unresolved ·
mixed reason counters across sessions (X full, Y singleton partial, U/V/W tied
unresolved, Z blocked unresolved) · **single session accumulating both reason
counters across segments** (capacity_unmet 500 + unresolved_unmet 500 in one
spanning session) · tie after partially delivered higher singleton (remaining
exactly 0 ⇒ capacity shortfall for ties) · tie after partially consumed
remaining (400 < 600 ⇒ withheld with unresolved) · clipping to horizon +
nonoverlap zero-total rows kept in results · empty sessions ok (valid actor +
horizon only) · reorder invariance (one local vector, in-place reverse; field
comparator, no invented operator==) · horizon split additive at NON-event
boundary m=4 over all five counters · translation (+7 shift of window and all
sessions, totals equal) · rate bounds 1 / 1000 + negative explicit priority
ordering · rejection battery: duplicate id, mixed actor (incl. zero actor
session), rate 0/1001, empty/reversed/negative session range, zero/reversed/
negative horizon, zero caller actor (with/without sessions), checked int64
overflow (rate 1000 × near-INT64_MAX length) — all with empty output; input
mutation check after both an ok and a rejected call · id-sorted output.

Per Codex acceptance notes: UB (two-temporary `rbegin/rend`) removed; result
comparison via test-local field comparator (no API expansion for tests);
`test_horizon_split_additivity` name mismatch fixed; dual-counter scenario and
both tie-after-partial cases added — gate not bypassed, all verified by the new
CTest run above.

## Limits / non-goals

- Pure evaluator only: no Realm, save, action, observation, reward, model
  change; no health/wage/care effects; no per-person model; benchmarks not
  extended beyond this slice.
- Complexity (corrected per Codex review, worst-case): over `S` sessions the
  evaluator builds `O(S)` endpoint segments; per segment it scans items `O(S)`
  and sorts the active set `O(S log S)` — **O(S^2 log S) time over O(S) endpoint
  segments, O(S) memory** (items, bounds, per-segment active set and result rows
  are all O(S)). This is a stated bound, NOT a benchmark: no scaling/performance
  claim is made from the above run.
- Tick is abstract; NO physical minutes/day tuning anywhere in this slice.
- `RejectReason::Overflow` also serves as the internal-inconsistency tripwire
  for the delivered+unmet==requested invariant (practically unreachable; any
  breach yields empty output, never corrupt totals).

## Next gate (NOT delivered here)

Realm integration: command/clock/save wiring with **explicit units and a
version guard** — separate task per handoff procedure. Working tree left
uncommitted for Codex review.
