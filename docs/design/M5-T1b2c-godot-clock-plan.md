# M5 T1b2c — manual Realm day and Godot clock synchronization

Status: accepted by Codex 2026-10-19; final Windows evidence in the linked report. Scope is the explicit one-day clock command and the Godot Realm sandbox UI/session bridge.

## Files
- `core/include/polyec/realm_clock.hpp`
- `core/include/polyec/realm.hpp`
- `core/src/realm.cpp`
- `core/tests/test_realm_clock.cpp`
- `native/include/polyec/realm_runtime.hpp`
- `native/realm_runtime.cpp`
- `native/realm_contracts.cpp`
- `gdext/polyec_session.cpp`
- `godot_project/ui/realm/realm_panel.gd`
- `godot_project/ui/main.gd`
- `godot_project/tests/realm_smoke.gd`
- `godot_project/tests/realm_expected.json`
- `docs/design/M5-T1b2c-godot-clock-plan.md`

## Ownership and API
- `Realm` owns the absolute subday cursor, game day, `paused_`, `pause_revision_`, schedule book, world state and RNG.
- `RealmRuntime` owns policy bundles, model memory, inference counters and the live `Realm`; it exposes clock operations only through Realm commands.
- GDExt is a read-only view plus command bridge. It does not advance time or mutate world state.
- The UI reads snapshots and sends commands. It does not own game time, funds, inventory or citizen state.

## Manual one-day semantics
- `RealmClockCommand::Operation::StepOneDay` is an explicit command that advances exactly `kSubdayUnitsPerDay` (64) subday units.
- `Realm::execute(StepOneDay)` validates stale revision and cursor bounds before mutation. It calls `Realm::advance_ticks_preserving_paused(64)`, which uses the same schedule/daily pipeline as ordinary advancement but does not reject merely because the Realm is paused.
- The command preserves `paused_` and `pause_revision_`. It does not pause, resume or create a temporary controller state.
- Manual stepping is separate from automatic day stepping. The automatic UI path keeps using `realm_step(1)`, which runs policy decisions before `Realm::advance(1)`.
- `RealmRuntime::step_one_day()` requires a loaded Realm and a chosen draft; it preflights the maximum-horizon boundary. It then runs learned decisions in order: Governance, Economic, Civilian. Only after successful inference and memory synchronization does it issue the core one-day command.
- A failed preflight or inference leaves ticks, day, pause state and revision unchanged, except for the runtime error string.

## Godot pause/run/focus
- Interactive model load starts paused. Loading models alone must not start automatic advancement.
- Starting the UI loop calls `realm_resume()` first and sets `running=true` only after success. Pausing sets `running=false` before calling `realm_pause()`.
- Every automatic day checks `running`, then rejects if the core is paused. A rejected step stops the UI loop and reports the core error.
- Manual “Một ngày” pauses/stops the UI loop first, then calls `realm_step_one_day()`. This prevents an explicit manual day from racing the automatic timer.
- Focus-out pauses the core and stops the UI loop. The app-level focus-out path pauses before deactivating the session so the pause command is not blocked by session inactivity.
- Focus-in reactivates the session but never auto-resumes the UI or the core.
- Long frames (`delta > 0.25`) pause and stop the loop. Help/guide and closing the Realm panel also pause and stop the loop.
- Escape closes the Realm panel through `close_panel()` and stops its UI loop.

## Save, restore and failure
- Save and load first stop the UI and pause the live Realm. If that pause fails, no save/load occurs.
- `RealmRuntime::restore_interactive` stages the loaded Realm, normalizes it to paused, then swaps. The live world is untouched until the staged load succeeds.
- If restore fails after pausing the live world, the live world remains the same, now paused, and the UI remains stopped. A failed restore must not clear or partially replace live world memory or counters.
- Restore is non-throwing on expected validation failures; errors are returned through the runtime error string. This avoids relying on a caught C++ exception crossing the Godot extension boundary.
- A non-interactive restore retains the saved pause state. Interactive restore always yields a paused Realm so UI/automatic advancement cannot start without an explicit resume.
- Save remains `POLYEC_REALM_7` with `CLOCK_1` and `SCHEDULE_1`; no schema change in this slice.

## Tests
- Core clock tests: paused manual step, unpaused manual step, stale revision, ordinary paused advance rejection, exact 64-tick progress, pause-state/revision preservation, max-horizon final day allowed and next day rejected without mutation.
- Native runtime contracts: unloaded/manual-step gates, missing draft, manual success while paused, pause-state/revision preservation, interactive load pause, failed missing-save restore keeping the live fingerprint and counters, and resumed advancement.
- Godot Realm smoke: interactive load starts paused; draft/resume; 64-day learned fixture; save/restore and corrupt-save atomicity; UI toggle, focus-out/in, guide, manual day while paused and while running, save/load through the panel, long-frame stop, app focus loss, close panel.

## Out of scope
- Campaign clock migration, Python portability/parity acceptance, T2 settlement, model/action/observation/reward changes.
- No commit, push, training or cloud purchase.
