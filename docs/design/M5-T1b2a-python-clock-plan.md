# M5 T1b2a — Python shared-core clock/schedule adapter contract

Status: approved by Codex 2026-10-19 for implementation. Scope is Python shared-core adapter and deterministic parity only.

## Files
- `bindings/python/realm_binding.cpp`
- `ai/tests/realm_clock_binding_checks.py`
- `core/tests/realm_clock_replay.cpp`
- `core/CMakeLists.txt`
- `docs/design/M5-T1b2a-python-clock-plan.md`

## Ownership
- `Realm` owns paused state, revision, cursor, and schedule book.
- Python receives detached copies and readonly records only.
- All clock and schedule mutations go through `Realm::execute`.

## Binding API
- `REALM_TICKS_PER_DAY = 64`.
- Readonly: `paused`, `pause_revision`, `game_tick`, `timeline_contract`, `pilot_policy_compatible`, `model_contract`, `schedule_cursor`, `schedule_next_session_id`, `schedule_sessions`.
- `pause(expected=None)` / `resume(expected=None)` delegate to `RealmClockCommand`.
- `advance_ticks(ticks)` delegates to `Realm::advance_ticks`.
- `schedule_command(operation, actor, sequence, session=0, begin=0, end=0, priority=0, demand_rate=0)` delegates to `ScheduleCommand`.

## Replay
- `polyec_realm_clock_replay` performs explicit checks and returns nonzero on failure.
- Covers overlap/ties, rejected state unchanged, day-boundary split, save/restore continuation, pause/stale, and nonzero attention totals.
- Python builds the same canonical trace and compares all fields, reject reasons, and serialized state.

## Limits
- No native runtime, Godot, Campaign, T2, M5, model, or training changes.
- Overflow is also exercised through serialized fixtures in Python.
