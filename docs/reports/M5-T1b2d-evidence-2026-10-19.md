# T1b2d Campaign clock — evidence

Date: 2026-10-19
Workspace: `D:\Code\PolyEc`, Windows, Release, build directory `build/m5-t1b-godot`
Implementation: OpenCode under the reviewed contract [M5-T1b2d](../design/M5-T1b2d-campaign-clock-plan.md).
Scope: Campaign core, GDExt bridge, Campaign UI/focus, and related tests. Existing dirty user/task files were preserved; no commit, push, training, or cloud action.

## Verification

- `cmake --build build/m5-t1b-godot --parallel 4`: passed. Compiler emitted existing `std::filesystem::u8path` deprecation warnings.
- `ctest --test-dir build/m5-t1b-godot -C Release -R campaign_tests --output-on-failure -V`: passed independently after implementation; 1/1 CTest, Campaign reports 214/214 checks.
- `scripts/test-native.ps1 -IncludeRealm` with the local Godot 4.5.1 and CMake/CTest paths from the approved plan: exit 0. CTest 11/11, Native smoke 19/19, UI smoke 18/18, Campaign smoke 49/49, Realm smoke 471/471.
- `campaign_profile.gd`, 10,000 residents: `restore_success=true`, invariant true; frame latency p50 6.894 ms, p95 6.975 ms, p99 7.366 ms; day plus snapshot p95 2.362 ms; save 326 ms, restore 119 ms, save size 1.47 MB.
- Scoped `git diff --check`: exit 0. New contract file produced only Git's LF/CRLF warning in the no-index check, with no whitespace diagnostics.

## Review result and limits

Accepted behavior: Campaign starts paused and interactive restore returns paused; commands remain usable while paused; automatic time rejects while paused; manual day executes the existing labeled controller once, advances one Campaign day, applies shared daily effects once, and preserves pause; speed remains UI pacing; focus loss pauses without auto-resume. Save schema, rules, balance, policy, action, observation, reward, and model behavior did not change.

Code review confirmed `Campaign::advance` and `Campaign::step_one_day` preflight the Realm cursor against the maximum horizon before invoking the controller. The Campaign tests do not include a direct serialized boundary fixture for this wrapper-level guard; the guard is accepted by code review, and this remains a specific evidence limitation. The Realm core's own horizon checks remain covered separately.

This is Windows evidence for T1b2d only. It does not establish portable Python/platform parity, T2 settlement, completed M5, trained RL quality, or M5.5 scale/performance acceptance.
