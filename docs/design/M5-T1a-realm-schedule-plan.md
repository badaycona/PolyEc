# M5 T1a — Realm schedule timeline (implementation contract)

Status: Codex accepted the core Windows slice after source/test review and CTest 8/8; final defensive invariant correction also rebuilt/tested 8/8. T1b clock/native/Python parity and T2 settlement remain open; T1 is not complete.

## Files
- `core/include/polyec/schedule.hpp`
- `core/src/schedule.cpp`
- `core/include/polyec/realm.hpp`
- `core/src/realm.cpp`
- `core/tests/test_schedule.cpp`
- `core/CMakeLists.txt`
- `native/realm_runtime.cpp` (restore guard only)

## Contract
- Fixed point: `kSubdayUnitsPerDay = 64`; global `cursor` is absolute `int64_t` game tick. No physical minutes/physiology.
- Sparse `ScheduleBook` keyed by actor (`unordered_map<uint64_t, vector<ActivitySession>>`), plus global cursor, `next_session_id`, sticky `timeline_contract`.
- `ActivitySession` has stable positive ID, actor, `[begin,end)` ticks, priority, demand rate, bounded accumulated evaluator totals, retired status/cause/tick.
- `ScheduleCommand` operations: Submit, Cancel, Reprioritize. It shares the same `Person::decisions` sequence as `RealmCommand`.
- Submit requires `begin >= cursor`; cancel/reprioritize reject sessions with processed time in this first slice.
- `advance_ticks(n)` integrates attention over `[cursor, cursor+n)`, evaluates each crossed 64 boundary exactly once via existing `daily()`, and handles dead owners by retiring sessions without deleting evidence.
- `advance(days)` delegates to `advance_ticks(days*64)`.
- Legacy mode is preserved until first accepted schedule; then `timeline_contract` becomes sticky and old policy access throws an explicit contract error.
- `realm_schema` remains `realm-v6` for the pilot policy contract; save magic becomes `POLYEC_REALM_7`.
- Native runtime rejects timeline worlds before replacing the active world.
- T1 is not complete until T1b clock/pause binding tests. Python/native parity gates remain open until actually built.
- Implemented review invariants: stage-before-commit; no full population copy per command; checked horizon/totals and cursor/day relation in both modes; retired evidence preserved; `RealmResult.session` receipt plus explicit schedule fact; schedule state in `Realm::invariant`.
