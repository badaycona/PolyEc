# M5-T1b2a evidence — 2026-10-19

## Scope
Python shared-core clock/schedule adapter and deterministic C++/Python transition parity only. No native runtime, Godot, Campaign, T2, M5, model, or training changes.

## Files
- `bindings/python/realm_binding.cpp`
- `ai/tests/realm_clock_binding_checks.py`
- `core/tests/realm_clock_replay.cpp`
- `core/CMakeLists.txt`
- `docs/design/M5-T1b2a-python-clock-plan.md`

## Verification
Platform: Windows (Clang/Zig, Ninja, Release).

Core:
```powershell
cmake --build build/m5-t1b --target polyec_realm_clock_replay --parallel 4
ctest --test-dir build/m5-t1b --output-on-failure
```
Result: build success; 10/10 tests passed; `realm_clock_replay` passed.

Python binding:
```powershell
cmake --build build/m5-t1b-python --target polyec_env polyec_realm_transition_replay --parallel 4
$env:PYTHONPATH="D:\Code\PolyEc\build\m5-t1b-python"
python ai/tests/realm_binding_checks.py --replay build/m5-t1b-python\core\polyec_realm_transition_replay.exe
python ai/tests/realm_clock_binding_checks.py --replay build/m5-t1b\core\polyec_realm_clock_replay.exe
```
Result: module build success; existing binding checks 258 checks passed; new clock binding checks 103 Python checks passed and 7 checkpoints matched exact serialized state and sorted session fields.

## Limits

Codex acceptance, 2026-10-19: independent full core build (Ninja no work), full CTest 10/10 passed in 7.64 seconds; Python module and transition replay build succeeded; existing Python binding replay 258 checks passed; new clock binding replay 103 counted checks passed and seven checkpoints matched; scoped `git diff --check` exit 0. Core T1b2a accepted. This does not accept native/Godot/Campaign or portable-platform gates.

- Native policy parity remains a later gate.
- Godot, Campaign, T2, and M5 remain untouched.
- No commit, push, train, cloud, or model action performed.
