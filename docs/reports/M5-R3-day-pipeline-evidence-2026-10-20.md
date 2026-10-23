# M5 R3 — Day pipeline orchestration evidence (2026-10-20)

## Result

R3 separates one-day orchestration from business calculations. A stateless `RealmDayPipeline` owns only the reviewed phase order and delegates each phase to private `Realm` methods. `Realm` remains the sole owner of mutable world state, `schedule_book_.cursor`, day boundary, and all calculations. Codex reviewed the final diff and accepted this slice. No save schema, binding, action, observation, reward, model, gameplay, or T2 settlement contract changed.

## Contract and reviewed design

- [Core architecture refactor](../design/core-architecture-refactor.md) §7.3 / §9.
- [Core class reference](../design/core-class-reference.md) §12.8 (`RealmDayPipeline`).

Approved phase order: `day_begin`, `day_payroll`, `day_people`, `day_production`, `day_harvest`, `finish_projects`, `day_investigations`, `day_treasury`, `day_loans`, `day_proposals`, `day_births`, `day_end`.

## Scope

Changed files:

- `core/include/polyec/realm.hpp`
- `core/src/realm.cpp`

`RealmDayPipeline` is stateless: `static void run(Realm&)`. After the reviewed follow-up, `run` is private with `friend class Realm`; `Realm` retains `friend class RealmDayPipeline` so the pipeline can call Realm's private phase methods. The only invoker remains the day-boundary path in `Realm::advance_ticks_preserving_paused`.

## No behavior or contract changes

- The phase bodies were moved without changing statements, per-person loop order, RNG call order, event order, arithmetic, or conditionals.
- `advance_ticks_preserving_paused` chunking, paused checks, `advance`/`advance_ticks` guards, `StepOneDay` at 64 ticks, day boundary, and horizon/reject behavior are unchanged.
- No change to save/serialization, Campaign/Godot/native/Python bindings, action/observation/reward/model, gameplay, or T2 work-time/settlement rules.

## Windows verification

Build configuration reused: `build/m5-core`, Visual Studio generator, Release, `POLYEC_BUILD_APP=OFF`, `POLYEC_BUILD_AI=OFF`, `POLYEC_BUILD_PYTHON=OFF`.

Chronology: initial extraction was configured and built successfully; after the follow-up private-access change, the final rebuild succeeded and CTest passed 10/10 in 14.73 s.

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' -S . -B build/m5-core -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=OFF -DPOLYEC_BUILD_PYTHON=OFF
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Release --parallel 4
```
Result: configure and rebuild exit 0; all core/test executables linked.

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Release --output-on-failure
```
Result: **10/10 passed**, 0 failed, 14.73 s: `core_tests`, `trade_tests`, `realm_tests`, `court_tests`, `campaign_tests`, `manager_tests`, `activity_tests`, `schedule_tests`, `realm_clock_tests`, `realm_clock_replay`.

```powershell
& 'C:\prc\PolyEc\build\m5-core\core\Release\polyec_realm_transition_replay.exe'
```
Result: exit 0; 64 replay checkpoints emitted.

```powershell
git diff --check -- core/include/polyec/realm.hpp core/src/realm.cpp
```
Result: exit 0.

## Split equivalence and StepOneDay/pause coverage

- Split equivalence: phase statements and their order were preserved by construction; the deterministic `realm_tests`, `campaign_tests`, and `realm_clock_tests` suites still pass.
- `realm_clock_replay` fixture compares a split advance (`advance_ticks(60)`, then `+5` across the day boundary, then `+63`) with an uninterrupted `advance_ticks(128)` and with save/restore across the day boundary; the serialized states match after continuation.
- `realm_clock_tests` covers paused and unpaused `StepOneDay` at exactly 64 ticks (one day, paused flag/revision unchanged), stale `StepOneDay` revision, ordinary advance rejected while paused without tick mutation, and horizon (final day allowed, next `StepOneDay` rejected with no time mutation).
- `polyec_realm_transition_replay.exe` completed with 64 checkpoints.
- No new dedicated phase-order test was added; equivalence relies on changed-code review plus the existing deterministic regression.

## Limitations

- No new targeted test isolates each extracted phase; equivalence relies on unchanged statement order plus existing deterministic regression.
- Only Windows `build/m5-core` was built and run. Native runtime, Godot/Campaign UI bindings, Python binding, and Linux parity were not exercised in this slice.
- R3 does not implement actual work-time accounting or end-of-day production/payment/material settlement, and does not close T2 or M5.
- The next actionable M5 gate starts with T2 contract review before implementation for actual settlement. The gameplay question of what "actual work time" means (explicit Work schedule block, unscheduled gap, or a full day while Employed) remains open and no law has been chosen.
- No commit, push, or network action was performed.
