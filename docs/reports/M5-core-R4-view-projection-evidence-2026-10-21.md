# M5 core R4 — RealmView projector evidence (2026-10-21)

Plan: [R4 view projection](../design/M5-core-R4-view-projection-plan.md). This result applies to the existing dirty working tree; it does not imply unrelated changes were reviewed.

## What changed

- Added a pure projector that builds the UI/runtime `RealmView` summary from read-only Realm data.
- `Realm::view(observer)` still decides fact visibility with `Realm::knows`; only after filtering facts does it pass data to the projector.
- Added focused checks for person/alive/child/decision totals, money and province copies, visible fact and draft copies, and the empty snapshot.
- No command, observation, reward, model, save schema, or public view fields changed.

## Verification on Windows

- Visual Studio 18 Release core build: passed.
- Focused projector and Realm CTest: 2/2 passed.
- Full Release CTest: 19/19 passed in 56.39 seconds.
- Python `polyec_env` Release binding build: passed and linked the updated core.
- Python Realm binding replay: 273 checks over 64 days; clock binding: 173 checks and 7 replay checkpoints.
- Windows Release GDExtension build: passed. Godot 4.5.1 headless worksite/model-compatibility smoke: 5/5 passed.
- Review was self-review by Codex against source and build/test results; no independent reviewer was used.

## Limits

This closes the view-projection extraction identified in the R4 codec report and verifies the existing Python and Godot integration paths against the rebuilt Windows binaries. It does not close gameplay contracts, learned-policy quality, or M5. The Godot check is headless compatibility coverage, not visual playtesting.
