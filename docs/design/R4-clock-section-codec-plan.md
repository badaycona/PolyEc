# R4 clock section codec — plan-only review

Review date: 2026-10-20. This is a **plan-only** behavior-preserving R4 save-section extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9. No source, build, test or implementation change is included in this document.

Review: **approved-by-Codex 2026-10-20 with conditions** — preserve the legacy `Save clock header` mapping, stage outputs, retain `uint64_t` extraction semantics including negative `-1` converting to `UINT64_MAX` under the Windows MSVC runtime, and keep Realm ownership/atomic commit.

## Status

Implemented and accepted by Codex on 2026-10-20 after diff review and Windows Release/Debug gates. Evidence: [R4 clock section codec report](../reports/R4-clock-section-codec-evidence-2026-10-20.md).

## 1. Candidate

Extract only the inline **`CLOCK_1` section** from `Realm::serialize` and `Realm::deserialize` in `core/src/realm.cpp`:

- section tag `CLOCK_1`
- `paused_` (`bool`, serialized as `0`/`1`)
- `pause_revision_` (`uint64_t`)

This block sits immediately after the court section (`write_court` / `read_court`) and immediately before the schedule section (`write_schedule_section` / `read_schedule_section`). It is bounded because it is one fixed tag followed by exactly two fixed Realm-owned scalar values, with no count prefix and no per-record loop. `Realm` continues to own `paused_` and `pause_revision_`; no validation beyond the legacy parse checks is added.

## 2. Scope and files

- Add `core/include/polyec/clock_section_codec.hpp` with:
  - `polyec::detail::write_clock_section(std::ostream& out, bool paused, uint64_t pause_revision)`
  - `polyec::detail::read_clock_section(std::istream& in, bool& paused, uint64_t& pause_revision)`
- In `core/src/realm.cpp`, add the include and replace only the inline `CLOCK_1` write in `Realm::serialize` and the inline `CLOCK_1` read in `Realm::deserialize`. Court and schedule calls remain at their existing positions.
- In `core/tests/test_realm_clock.cpp`, add focused codec checks and, if feasible in the existing harness, Realm-level malformed-save checks; keep existing clock, pause, overflow and roundtrip tests.
- No CMake, schema, gameplay, bindings, UI, native, Godot, Python, model or archive changes.
- Do not modify `core/src/realm_court.cpp`, any court codec header, `core/include/polyec/schedule_section_codec.hpp`, or `core/src/schedule.cpp`.

## 3. Token grammar and exact byte order

The codec writes and reads the tag plus the two clock fields in their current order, with **no count prefix**:

```text
 CLOCK_1 <paused 0|1> <pause_revision> 
```

Writer order remains exactly:

```cpp
out << " CLOCK_1 " << (paused ? 1 : 0) << ' ' << pause_revision << ' ';
```

The section is the block between court and schedule. In `Realm::serialize` it remains immediately after `write_court(s)` and immediately before `write_schedule_section(s, schedule_book_)`. In `Realm::deserialize` it remains immediately after `w.read_court(s)` succeeds and immediately before `read_schedule_section(...)`. No field is moved, renamed, added or removed. The leading and trailing spaces match the legacy writer.

The `bool` pause flag is serialized as an integer `0` or `1` via the existing `(paused_ ? 1 : 0)` expression. The reader parses it as `int` and accepts only `0` or `1`, preserving the legacy `(pausedFlag != 0 && pausedFlag != 1)` check.

## 4. Legacy failure contract

- The section has **no count prefix**.
- Malformed or truncated tag: if extraction of `clockTag` fails, or `clockTag != "CLOCK_1"`, the read throws `std::runtime_error("Save clock header")`. This covers a missing tag, a truncated stream before the tag, and a wrong tag such as `CLOCK_X`.
- Pause flag outside `{0,1}`: if extraction of `pausedFlag` fails, or the parsed integer is neither `0` nor `1`, the read throws `std::runtime_error("Save clock header")`. This covers a truncated stream after the tag and a value such as `2`.
- Revision parse failure: if extraction of `pauseRevision` as `uint64_t` fails, the read throws `std::runtime_error("Save clock header")`. This covers a truncated stream after the flag, a non-numeric token, and a magnitude above `UINT64_MAX`. Windows MSVC 18.10.2 was probed directly: formatted extraction of `-1` into `uint64_t` succeeds and yields `UINT64_MAX`. That behavior is legacy and must be preserved; the codec must not add a sign check or otherwise reject a negative token that the existing unsigned extraction accepts.
- The codec must **not** throw `Save count limit` or any other error string for this section.
- **Staged parse:** the codec reads the tag, flag and revision into local temporaries first and assigns them to the output references only after the tag matches, the flag is in `{0,1}` and the revision has parsed. A failure throws before assignment, so the caller's values remain unchanged.
- **No range or semantic validation.** No check is added for `pause_revision` (all `uint64_t` values, including `UINT64_MAX`, remain valid and loadable), for consistency with `day_`/`schedule_book_.cursor`, or for any pause-state invariant. The legacy behavior is preserved exactly.
- **Truncation is not field-boundary detection.** Because this section is unframed beyond its tag and two fields, the helper cannot tell whether trailing bytes belong to this section or to the following schedule section. On truncated input the `>>` extraction fails; the helper only detects extraction failure. This matches the legacy unframed extraction behavior. The plan and tests must not claim field-boundary or section-boundary detection, and must not add framing to create it.

## 5. Ownership and atomicity

- `Realm` remains the sole owner of `paused_` and `pause_revision_`.
- The codec is stateless and touches only these two values. It does not own, transfer or replace Realm state.
- `Realm::deserialize` calls the codec into `w.paused_` and `w.pause_revision_`. The codec's staged parse publishes only after a complete successful read; on failure it throws before assigning, leaving the temporary Realm's clock state unchanged.
- `deserialize` remains atomic: a failed codec read causes `deserialize` to return `false` with `error == "Save clock header"` and leaves `*this` unchanged because the temporary `Realm w` is discarded. A successful read commits both `paused_` and `pause_revision_` together with the rest of the loaded world.
- No transaction ID, settlement ID, schema change or gameplay state is added. Controller/transient timing state is not serialized; `RealmClockController::reset_after_restore()` remains responsible for clearing residual accumulator state after load.

## 6. Tests

Add focused checks in `core/tests/test_realm_clock.cpp`, modeled on the existing clock malformed-save checks but adapted to the codec API:

Codec-level checks:

- Exact writer bytes for a canonical clock state. Suggested canonical values: `paused = true`, `pause_revision = 42`; expected writer string: ` CLOCK_1 1 42 `.
- Valid round trip preserves `paused` and `pause_revision`, including `paused = false` / `pause_revision = 0` and `pause_revision = UINT64_MAX`.
- Bad tag: input `CLOCK_X 1 0` throws `Save clock header` and preserves both output values.
- Pause flag outside `{0,1}`: input `CLOCK_1 2 0` throws `Save clock header` and preserves both output values.
- Truncated flag: input `CLOCK_1` throws `Save clock header` and preserves both output values.
- Truncated revision: input `CLOCK_1 1` throws `Save clock header` and preserves both output values.
- Revision parse failure: input `CLOCK_1 1 not-a-number` and an unsigned magnitude above `UINT64_MAX` throw `Save clock header` and preserve both output values.
- Negative revision token: input `CLOCK_1 1 -1` parses successfully with Windows MSVC `operator>> uint64_t` and yields `UINT64_MAX`; add a codec check expecting success and `pause_revision == UINT64_MAX`.
- Empty input throws `Save clock header` and preserves both output values.
- Output preservation on failure for representative values, including `paused = false`, `pause_revision = 7`.

Realm-level checks, if feasible in the existing harness:

- Build a base Realm, serialize it with bundle `"test"`, mutate the `CLOCK_1` tag to `CLOCK_X`, deserialize into a destination Realm, and verify rejection with `Save clock header` and an unchanged destination fingerprint.
- Repeat with the pause flag mutated to `2`, then with the revision truncated, and verify the same rejection and unchanged destination fingerprint.
- These fixtures are new but follow the existing malformed `CLOCK_1` checks already present in `test_realm_clock.cpp`; the plan does not require weakening those checks.

Existing clock, pause, overflow, controller and roundtrip tests remain the end-to-end oracle for successful saves.

## 7. Exact gates

Reuse only `build/m5-core`; do not configure or create a new build directory. Run:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Release --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Release --output-on-failure
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Debug --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Debug -R 'court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_tests|realm_clock_replay' --output-on-failure
git diff --check
python scripts/check-repository-links.py
```

Record build exit codes, CTest pass counts, targeted test counts, and `git diff --check` result in the evidence report. Do not claim direct executable verification beyond the CTest gates above.

## 8. Deferred

- Other remaining R4 section/record/extension refactors.
- Schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
- No commit or push.
