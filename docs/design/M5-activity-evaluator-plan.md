# M5 — Pure Activity Evaluator (prerequisite slice) — implementation plan (approved)

Status: **Plan approved by Codex review (2026-10-17 conversation); the initial
record drifted from the approved contract on Horizon shape, capacity ownership,
rate-based remaining and exact counters, and was corrected under Codex review
before any source was written — the approved contract governs, not any doc
self-label. Suite extensions requested by Codex pre-acceptance review were added
to the test list (single-session dual reason counters; tie-after-partial
delivery both with remaining exhausted and with remaining merely reduced).**
This is a **prerequisite slice only**: a pure stateless per-actor evaluator,
NOT Realm integration. Realm command/clock/save integration is the next gate
and is out of scope here.

## Objective

`polyec::activity::evaluateActivity(actor_id, horizon, sessions)` — pure, stateless,
checked-arithmetic scheduler for one actor's attention-demanding sessions over an
absolute half-open window. No Realm, no save, no action/model involvement; no
physical minutes/day semantics — the tick is an abstract quantizer only.

## Scope

- `core/include/polyec/activity.hpp` (new)
- `core/src/activity.cpp` (new, joins existing `polyec_core`)
- `core/tests/test_activity.cpp` (new)
- `core/CMakeLists.txt` (add source + `polyec_activity_tests` CTest target)
- `docs/reports/M5-activity-evaluator-2026-10-18.md`, `WORK_STATUS.md`

Nothing else. Existing 6 CTest targets preserved.

## API contract

```cpp
namespace polyec::activity {
struct Horizon { int64_t begin, end; };  // absolute half-open [begin, end)
struct SessionView;             // immutable POD input view
struct SessionResult;           // per-session totals
struct EvaluateResult;          // ok/reason/results, aggregated counters

EvaluateResult evaluateActivity(
    uint64_t actor_id, Horizon horizon,
    const std::vector<SessionView> &sessions);
}
```

**Input invariants (validated before any output construction):**

- `sessions` immutable; caller's vector never mutated.
- Optional sessions: empty vector is valid **only for a valid actor and a valid
  horizon**; it yields empty results (no output entries) and zero totals.
- Session fields:
  - `id`: positive unique `uint64_t`.
  - `actor`: positive, must equal `actor_id` (mixed-actor input rejected).
  - `begin`, `end`: `int64_t`, nonnegative, `begin < end` (half-open).
  - `priority`: explicit `int32_t` (negative allowed, ordering only).
  - `demand_rate`: integer attention **rate** per tick in `[1, 1000]` (inclusive).
- **Capacity ownership:** capacity is a constant global `1000`
  attention rate per tick for the single actor. There is NO per-session
  `capacity` field and no custom capacity input; sessions carry only
  `demand_rate`. Requested units = `demand_rate * window/segment length`.
**Input invariant — Horizon:** `horizon` is struct `Horizon{int64_t begin, end;}`
— an **absolute** half-open window `[begin, end)` with `0 <= begin < end`
strictly required. No implicit origin 0 anywhere in source or tests; no
`int64_t` length parameter; no `horizon == 0` success path. Sessions may lie
before, inside, after, or straddle the window and are clipped to it.

**Rejections (EMPTY output — `results` empty, all totals zero, `ok=false`,
no input mutation):** invalid structure (`id==0`, duplicate ids, actor mismatch /
mixed actors, nonpositive actor, begin/end out of order or negative, rates out of
`[1,1000]`, `horizon.begin >= horizon.end`, negative `horizon.begin` or
`horizon.end`), and checked-overflow failures during demand summation. Rejection
never partially constructs output. Empty `sessions` with a valid actor+horizon
is a valid call: `ok=true`, zero totals, empty results.

## Scheduling rules (reviewed, rate-based contract)

- Clip each session to `[horizon.begin, horizon.end)`:
  `b = max(begin, horizon.begin)`, `e = min(end, horizon.end)`; drop if `b >= e`
  (fully clipped out ⇒ contributes **zero totals**, still listed in results with
  its own id).
- Partition timelines by segmenting at session endpoints; process **priorities
  descending**; equal priority = one contested group.
- **`remaining` is a RATE in `[0, 1000]` attention units per tick**, initialized
  to the global `1000` and descending through priority group consumption.
- Within a segment, for each priority group in descending priority order:
  - group demand rate `G` = sum of session demand rates active in the segment
    (checked summation).
  - if `G <= remaining`: all group members fully delivered for the segment;
    `remaining -= G` (rate units, NOT `G * len`); each member's delivered total
    accumulates `rate * len` integer units.
  - else `G > remaining` (overflow):
    - singleton group: assign RATE `remaining` for the segment
      (`delivered += remaining_rate * len` remains integer units), then
      `remaining = 0`; its shortfall is `capacity_unmet`.
    - tied group (two or more members, `remaining > 0` before the comparison):
      capacity is **withheld** for this segment — no member receives anything,
      every member's shortfall is `unresolved_unmet`, and **all lower-priority
      demand in the same segment is also `unresolved_unmet`** with nothing
      delivered, regardless of what would fit.
  - if `remaining == 0` at a group's turn (fully consumed by deliveries or
    singleton assignment from higher groups): that group (even if tied) and all
    lower-priority demand in the segment get `capacity_unmet`, NOT
    `unresolved_unmet` — i.e. `remaining == 0` from delivered higher priority
    outranks tie status for reason attribution; only an actual withholding event
    in this segment (tied overflow with positive remaining) blocks lower demand
    as `unresolved_unmet`.
- No ID order, no input order winner anywhere: singleton partial is the only
  delivery under overflow (single consumer), ties withhold instead of splitting.
- Delivered + unmet == requested totals per session, exactly (integer units,
  never proportional blends).

## Counters & reasons (per session + aggregate)

Per session: `requested`, `delivered`, `unmet`, `capacity_unmet`,
`unresolved_unmet`, with `delivered + unmet == requested`, `unmet == capacity_unmet
+ unresolved_unmet`. Aggregate counters summed across sessions. Results sorted by
session `id` ascending.

## Checked arithmetic

All rates, segment lengths, rate products and sums go through hand-rolled
overflow-checked helpers on `int64_t` (checked add for rate sums, checked mul for
`rate * len` and requested/delivered accumulation; all operands nonnegative, so a
single `b > INT64_MAX - a` style guard suffices per op). Comparator sorts by
(priority desc, begin asc, end asc, id asc) using comparisons only — no
subtraction overflow.

## Determinism/invariance requirements covered by tests

- Numerical small cases: exclusive/compatible sessions deliver exactly.
- Singleton partial delivery under shortage.
- Overcommit is valid: overlapping sessions whose per-tick rate sum exceeds the
  global 1000 are resolved by the shortage rules above, never structurally
  rejected.
- Ties overflow: withhold, mark unresolved, propagate to lower priorities.
- High-priority full + `remaining == 0` ⇒ lower ties get `capacity_unmet`.
- Mixed reason counters in one run.
- Single-session dual cause: one session accumulating `capacity_unmet` in one
  segment (higher priority exhausted capacity by delivery) and
  `unresolved_unmet` in another segment (tied overflow withheld), both nonzero.
- Tie after a partially delivered higher singleton: remaining exactly 0 ⇒ even
  tied lower sessions get `capacity_unmet` (never `unresolved_unmet`); tie after
  merely reduced-but-positive remaining ⇒ withheld with `unresolved_unmet`.
- Clipping: sessions beyond horizon contribute zero totals but present in results.
- Non-overlap: zero totals.
- Reorder invariance: shuffling input order cannot change results.
- Horizon-split additivity: evaluating `[a, m)` and `[m, c)` separately (with `m`
  NOT a session/event boundary) and summing per-session totals equals evaluating
  `[a, c)` in one call.
- Translation invariance: shifting all sessions and the window by the same offset
  preserves totals.
- Rejections: duplicate id, mixed actor, invalid rate/bounds/horizon,
  checked-overflow demand sum.

## Tests: `core/tests/test_activity.cpp`

Pattern per `test_managers.cpp`: plain `main()`, `check()` counting failures to
stderr, hardcoded expected numbers (no recomputation in assertions), **runtime
checks active in Release** (no `assert()`, no NDEBUG-out checks; printed
"activity tests passed" on success). No per-session allocator/state — pure
stateless evaluator, no heap identity.

## CMake

- Add `src/activity.cpp` to `polyec_core`.
- New target `polyec_activity_tests` (tests/test_activity.cpp → polyec_core) with
  CTest name `activity_tests`; existing 6 targets untouched.

## Build & verify (separate build dir, Release)

```
D:/Code/PolyEc/build/tooling/cmake/data/bin/cmake.exe -S . -B build/m5-activity-evaluator -G Ninja -DCMAKE_MAKE_PROGRAM=D:/Code/PolyEc/build/tooling/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=D:/Code/PolyEc/build/tooling/zig-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=OFF -DPOLYEC_BUILD_PYTHON=OFF
D:/Code/PolyEc/build/tooling/cmake/data/bin/cmake.exe --build build/m5-activity-evaluator --parallel 4
D:/Code/PolyEc/build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-activity-evaluator --output-on-failure
```

Expected: CTest 7/7 (existing 6 + `activity_tests`), Release runtime checks active.

## Explicit non-goals (this slice)

- No Realm/save/Action/binding/model changes; no health/wage/care side effects.
- No physical time semantics (minutes/day) or fixtures for them.
- No allocator for sessions in tests; no duplicate production payroll-style logic.
- Next gate (not delivered): Realm command/clock/save integration with explicit
  units and a version guard.
