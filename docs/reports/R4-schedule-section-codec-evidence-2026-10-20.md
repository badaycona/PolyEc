# R4 — ScheduleBook save-section codec evidence

## Result

Accepted on 2026-10-20. Source context is the dirty working tree based on `HEAD dff2781`; existing R3/T2 changes were retained. No commit or push was created.

`write_schedule_section` now owns the complete deterministic `SCHEDULE_4` output. `read_schedule_section` reads the schema-selected `SCHEDULE_1` through `SCHEDULE_4` grammar into a temporary `ScheduleBook`; it preserves v7 Generic/region `-1` defaults, v8 attendance, v9 settlement, and v10 regional pay watermarks. `Realm` still owns the live book, whole-world validation, stream-tail checks, invariant check, and final atomic world assignment. No save schema, bytes, gameplay, or public API change was intended.

The implementation is bounded to `core/include/polyec/schedule_section_codec.hpp`, the serialization integration in `core/src/realm.cpp`, and the golden/migration coverage in `core/tests/test_schedule.cpp`. OpenCode's first code pass had incorrect legacy count parsing and omitted the required golden test. Codex stopped that pass, reviewed the approved R4 contract, repaired the bounded implementation, and independently verified it. This is an explicit implementation-owner deviation under `AGENTS.md`.

## Verification

- Existing Windows Release build `build/m5-core`: full build exit 0.
- Release CTest: **10/10 passed** (15.49 s).
- `polyec_schedule_tests.exe`: **3,736 checks passed**, including exact pre-refactor `SCHEDULE_4` bytes and v7/v8/v9 migrations.
- Existing Windows Debug build `build/m5-core`: full build exit 0.
- Targeted Debug CTest (`realm_tests|schedule_tests|manager_tests|realm_clock_replay`): **4/4 passed** (54.76 s).
- `git diff --check`: exit 0; only existing Git line-ending notices were printed.

No new build directory/generator was configured. Python/native/Godot/Linux and training gates were not run. No Establishment/worksite behavior was added.
