# M5 T1b1 — Realm clock/pause adapter (implementation contract)

Status: core implementation accepted by Codex 2026-10-19 after overflow repair review, full rebuild and CTest 9/9; native/Python T1b2 remains separate.

## Files
- `core/include/polyec/realm_clock.hpp` (new)
- `core/src/realm_clock.cpp` (new)
- `core/tests/test_realm_clock.cpp` (new)
- `core/include/polyec/realm.hpp`
- `core/src/realm.cpp`
- `core/CMakeLists.txt`
- `docs/reports/M5-T1b1-evidence-...` (after run)

## Ownership / invariants
- `Realm` owns `paused_` (default `false`) and `pause_revision_` (default `0`).
- `Realm` already owns game time as `schedule_book_.cursor` (`game_tick()`, 64 ticks/day) and `day_`.
- `RealmClockController` owns only transient wall-clock state: `accum_ticks_`, `speed_`, `active_`, `seen_revision_`. It does **not** own paused state and is never serialized.
- No public `set_paused` setter on `Realm`; all pause/resume mutation goes through `Realm::execute(RealmClockCommand)`.
- Default bare `Realm` is unpaused so headless/training code and existing tests are unchanged. An interactive `RealmClockController` pauses the core in its constructor via a core command.

## Clock command contract
- `RealmClockCommand { Operation::Pause | Resume; std::optional<uint64_t> expected_pause_revision = std::nullopt; }` where `nullopt` means unconditional; a value (including `0`) is an exact expectation.
- `Realm::execute(RealmClockCommand)`:
  - unknown operation rejected atomically;
  - if `expected_pause_revision` has a value and it differs from `pause_revision_`, reject as stale with no mutation;
  - `0` is a valid expectation for the initial revision and becomes stale after any transition;
  - `Pause`: idempotent; no revision increment if already paused; otherwise set paused and increment revision with `UINT64_MAX` overflow guard;
  - `Resume`: idempotent; no revision increment if already unpaused; otherwise clear paused and increment revision with overflow guard;
  - no `Person`, money, payroll, schedule cursor, or NPC strategy mutation.
- `Realm::advance_ticks(n)` / `advance(days)`:
  - if `n > 0` (or `days > 0`) and `paused_`, throw `std::runtime_error` before any mutation;
  - `n == 0` / `days == 0` is always a no-op, even while paused;
  - otherwise existing checked bounds and integration behavior unchanged.

## Controller contract
- Constructor takes `Realm&` and speed (default `1.0`), validates speed to `[0,8]` before any Realm mutation and throws on invalid input, pauses core via `execute(Pause)`, clears accum, syncs `seen_revision_`; pause failure throws instead of leaving an unpaused controller.
- `set_active(false)`: pause core, drop accum, mark inactive.
- `set_active(true)`: on regain from inactive, pause core again, drop accum, sync revision, mark active; direct core resume while inactive does not auto-resume, so the core remains paused awaiting manual resume.
- `set_paused(true)`: delegate `Pause`.
- `set_paused(false)`: refused while inactive; otherwise delegate `Resume`, clearing accum and syncing revision on success.
- `set_speed`: accepts only finite values in `[0,8]`; invalid input returns `false` with no mutation.
- `update(dt)`:
  - reject NaN/Inf/negative `dt` with no mutation;
  - if inactive, drop accum and return `false`;
  - if `dt > 0.25`, pause core, drop accum, return `false`; if the Pause command fails (for example, unpaused core at `UINT64_MAX` pause revision), throw `std::runtime_error("clock pause failed")` before mutating accum, seen revision, or cursor, leaving the failed call atomic; an already-paused core at `UINT64_MAX` remains idempotent success;
  - if `seen_revision_ != realm.pause_revision()`, drop accum and sync (external pause/resume detection);
  - if core paused, drop accum and return `false`;
  - add `dt * speed * 64` ticks to accum;
  - speed `0` freezes (no advance);
  - advance only `floor(accum)` ticks, capped defensively at `8 * 64 = 512` ticks/update, retaining the remainder and fraction;
  - uses `Realm::advance_ticks`, so daily boundaries are crossed exactly once;
  - returns `true` for accepted (possibly zero-tick) updates.
- `step_day()`: advances exactly 64 ticks only when active and core unpaused, via `advance_ticks`; resets accum.
- `reset_after_restore()`: unconditionally clears residual accum and syncs `seen_revision_`, even when the restored revision equals the old seen revision.

## Save / load contract
- `POLYEC_REALM_7` magic and `realm-v6` pilot policy contract unchanged.
- New `CLOCK_1` section is serialized **before** `SCHEDULE_1`, containing `paused` (0/1) and `pause_revision`.
- Load validates the `CLOCK_1` tag, boolean flag, and temp-file success before replacing state; canonical paused flag and revision are preserved by pure Realm save/load.
- `POLYEC_RUNTIME_1` is unchanged: controller accum is transient and not serialized. `RealmRuntime` is not modified in T1b1.

## Tests
- `core/tests/test_realm_clock.cpp`, CTest `realm_clock_tests`.
- Release-runtime checks (no `assert`/NDEBUG-dependent checks).
- Covers subtick, focus, no-catchup, long gap, scales 0/1/4/8, revision detection, direct core pause rejection, invalid commands/speed/dt, save cursor + `reset_after_restore`, and phase boundaries. Cap is not exercised with invalid dt.

## Out of scope
- No native/Python changes yet (T1b2).
- No wages, health, RL, or campaign semantic changes.
- No commit/push/train.
EOF
