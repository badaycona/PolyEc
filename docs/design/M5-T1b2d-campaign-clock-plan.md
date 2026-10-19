# M5 T1b2d — Campaign clock, manual day, and UI synchronization

Status: approved by Codex review and the user on 2026-10-19. This is the
supporting contract for implementation and acceptance; it does not close T1b or
M5.

## Scope

This slice moves the Campaign UI loop onto the Realm-owned pause clock and adds
an explicit one-Campaign-day command. It covers the Campaign core, the GDExt
bridge, the Campaign panel, app focus handling, and targeted tests.

Approved gameplay semantics:

- A newly opened Campaign starts paused.
- An interactive Campaign restore always returns paused.
- “+1 ngày” stops the UI loop, advances exactly one Campaign day through the
  existing labeled M4 controller and Campaign daily effects, and remains paused.
- Campaign commands remain available while paused.
- 1x/2x/4x remain UI pacing only; no Campaign clock speed is added to the core.
- Existing rules, balance values, controller behavior, observations, rewards,
  and model behavior are unchanged.

## Files

- `core/include/polyec/campaign.hpp`
- `core/src/campaign.cpp`
- `gdext/polyec_session.cpp`
- `godot_project/ui/campaign/campaign_panel.gd`
- `godot_project/ui/main.gd`
- `core/tests/test_campaign.cpp`
- `godot_project/tests/campaign_smoke.gd`
- `godot_project/tests/campaign_profile.gd`
- `docs/design/M5-T1b2d-campaign-clock-plan.md`

## Ownership and invariants

- `Realm` remains the sole owner of the game cursor, `paused_`, and
  `pause_revision_`.
- `Campaign` owns only its scenario/controller state and delegates all Campaign
  pause, resume, and one-day clock operations to
  `RealmClockCommand` through its embedded `Realm`.
- The Campaign UI `running` flag is loop intent only. The core pause state is
  authoritative.
- Pause, resume, and manual day are control-plane operations. They do not alter
  money, inventory, citizen state, policy, or strategy except through the normal
  daily pipeline when a day is actually advanced.
- Automatic Campaign advance requires an unpaused Realm. A paused or
  horizon-overflowing request is rejected before the labeled controller runs or
  any world/Campaign mutation occurs.
- A manual Campaign day runs the existing labeled controller once, advances
  exactly `kSubdayUnitsPerDay` (64) subday units, then executes Campaign daily
  effects exactly once while preserving the prior pause state and pause
  revision.
- Daily effects previously inside `Campaign::advance` are shared by the
  automatic and manual paths; they must not be duplicated.
- Save data remains `POLYEC_M4_1` with the embedded `CLOCK_1`; no schema bump or
  migration is added.
- Headless/core restore preserves the saved pause state. Interactive GDExt
  restore loads into staged state, normalizes that state to paused, validates it,
  and only then swaps it into the live Campaign. A failed restore leaves the live
  Campaign unchanged.
- No Campaign Python binding, action, observation, reward, ONNX model, or
  training path changes in this slice.

## Core API

`Campaign` additions:

- `bool paused() const`
- `uint64_t pause_revision() const`
- `void pause(std::optional<uint64_t> expected = std::nullopt)`
- `void resume(std::optional<uint64_t> expected = std::nullopt)`
- `void step_one_day(std::optional<uint64_t> expected = std::nullopt)`
- `CampaignView` reports `paused` and `pause_revision`.
- `load`/`deserialize` gain an interactive flag; interactive restore normalizes
  to paused after staged load and before swap.

`Campaign::advance(days)` preflights, before the controller or any mutation:

1. The existing 1..30 chunk bounds.
2. That the campaign is not paused.
3. That the entire requested horizon fits the Realm maximum.

`Campaign::step_one_day()` preflights the expected pause revision, Campaign
horizon, and then runs the controller, `RealmClockCommand::StepOneDay`, and the
shared daily settle path. It rejects a stale expected revision before the
controller runs.

All rejected operations throw/return without partial state changes. GDExt
converts expected core failures into `campaign_error` and returns `false`.

## GDExt bridge

Additive methods:

- `m4_step_one_day() -> bool`
- `m4_pause() -> bool`
- `m4_resume() -> bool`
- `m4_paused() -> bool`
- `m4_pause_revision() -> int`

`m4_snapshot()` adds `paused` and `pause_revision`. Existing keys are unchanged.
`m4_start` stages, pauses, then publishes the new Campaign. `m4_pause` works even
when the session is inactive, matching the Realm bridge. `m4_resume` and
`m4_step_one_day` require an active session. Interactive `m4_save(restore=true)`
normalizes to paused. Campaign commands remain available while paused.

## Campaign UI and focus

- The panel reads `paused` from the snapshot for its state label and run button.
- Toggle-to-run resumes the core first and sets `running=true` only on success.
- Toggle-to-stop, long-frame stop, guide, save, load, close/back, focus-out, and
  action commands pause the core and stop the UI first.
- A failed pause is fail-closed: the panel stays visible, the UI remains stopped,
  and the pause error is shown. No save/load/command/close proceeds.
- Automatic stepping checks `running`, then the core pause state, then calls
  `m4_step(1)`. A rejected step stops the loop and pauses the core if still
  possible.
- Manual “+1 ngày” pauses/stops first, calls `m4_step_one_day()` exactly once,
  leaves the core paused, and refreshes without restarting the loop.
- The app-level focus-out pauses the visible Campaign panel before deactivating
  the session. Focus-in does not resume.
- Long frames (`delta > 0.25`) pause and drop elapsed time; no catch-up.

## Tests and gate

Core tests cover pause/resume, stale revision rejection, paused automatic
advance rejection without mutation, manual day while paused and unpaused,
one-day-only effects, pause preservation, and interactive restore normalization.

Godot Campaign smoke covers start-paused, explicit resume, automatic step,
manual day while paused, pause failure fail-closed, save/load, guide,
focus-out/in, long-frame stop, commands while paused, and close/back.

Required verification:

```powershell
& build/tooling/cmake/data/bin/cmake.exe --build build/m5-t1b-godot --parallel 4
& build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-t1b-godot -C Release -R campaign_tests --output-on-failure -V
& scripts/test-native.ps1 `
  -Godot "D:/Code/PolyEc/build/tooling/godot-4.5.1/Godot_v4.5.1-stable_win64_console.exe" `
  -CTest "D:/Code/PolyEc/build/tooling/cmake/data/bin/ctest.exe" `
  -BuildDir "build/m5-t1b-godot" -Config Release -IncludeRealm
git diff --check -- <scoped files>
```

## Out of scope

- Portable Python/platform parity and its acceptance.
- T2 settlement, Campaign schema changes, new gameplay rules or balance values.
- Action, observation, reward, controller, ONNX, and training changes.
- Commit, push, training runs, or cloud purchases.
- `docs/CURRENT_TASK.md` and `docs/DEVELOPMENT_HISTORY.md` edits before Codex
  acceptance.
