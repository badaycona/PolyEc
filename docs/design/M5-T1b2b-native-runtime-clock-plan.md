# M5 T1b2b — native RealmRuntime pause/clock bridge

Status: accepted by Codex 2026-10-19; native pause bridge and pre-inference guards verified. Horizon boundary lacks a direct native runtime fixture.

## Files
- `native/include/polyec/realm_runtime.hpp`
- `native/realm_runtime.cpp`
- `native/realm_contracts.cpp`
- `docs/design/M5-T1b2b-native-runtime-clock-plan.md`

## Ownership and API
- `Realm` owns `paused_`, `pause_revision_`, and game time.
- `RealmRuntime` exposes pause/resume only through `Realm::execute(RealmClockCommand)`.
- `RealmRuntime` has no raw subday tick or `advance_ticks` API. Day advancement remains `advance(days)`, which runs learned family decisions before `Realm::advance()`.
- `paused()` and `pause_revision()` are read-only bridges to core state.
- Headless load remains unpaused.

## Semantics
- `pause(expected)` / `resume(expected)`: `nullopt` unconditional; any value, including 0, is an exact expectation.
- `advance(days)` performs a horizon preflight before any policy decision or inference; requests exceeding the Realm maximum cursor fail with `Realm advance exceeds maximum horizon`.
- `advance(days)` rejects with `Realm is paused` before decisions, inference, memory sync, or world mutation.
- Successful advance preserves existing order: Governance, Economic, Civilian decisions, then one core day.
- `command()` behavior is unchanged.

## Atomicity and restore
- Pause/resume validation and overflow checks occur before mutation.
- Failed restore leaves runtime state unchanged; Realm serialization restores paused flag and revision.
- Runtime has no transient clock adapter; if one is added later, call `reset_after_restore()` after successful restore.

## Tests
- `polyec_realm_contracts` checks unloaded rejection, unpaused load, idempotent pause, stale expected rejection, paused advance rejection with unchanged fingerprint/tick/inference count, resume, stale resume rejection, paused save/restore, and resumed advance.

## Out of scope
- No Godot/Campaign/UI changes, Python binding changes, model changes, or raw tick API.
- No commit/push/train/cloud.
EOF
