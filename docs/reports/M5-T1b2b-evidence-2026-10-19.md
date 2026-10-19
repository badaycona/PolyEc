# M5-T1b2b evidence — native RealmRuntime pause/clock bridge

## Scope
Native `RealmRuntime` pause/clock bridge only. No Godot, Campaign, Python, model, action, observation, or reward changes.

## Files changed
- `native/include/polyec/realm_runtime.hpp`
- `native/realm_runtime.cpp`
- `native/realm_contracts.cpp`
- `docs/design/M5-T1b2b-native-runtime-clock-plan.md`

## Verification
Platform: Windows, Clang/Zig, Ninja, Release.

Native AI contract build:
```powershell
$env:ORT_ROOT="D:\Code\PolyEc\build\tooling\ort-1.31.0\onnxruntime-win-x64-1.31.0"
cmake -S . -B build\m5-t1b-native -G Ninja -DCMAKE_MAKE_PROGRAM=.../ninja.exe -DCMAKE_TOOLCHAIN_FILE=.../zig-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=ON -DPOLYEC_BUILD_PYTHON=OFF -DORT_ROOT=$env:ORT_ROOT
cmake --build build\m5-t1b-native --target polyec_realm_contracts --parallel 4
```
Result: build success.

Contract run:
```powershell
build\m5-t1b-native\polyec_realm_contracts.exe <three realm-v6 ONNX paths> contract-test-v6 build\m5-t1b-native\runtime-contracts.polyec
```
Result: `RUNTIME CONTRACTS: 75 checks passed`.

Core regression:
```powershell
ctest --test-dir build\m5-t1b --output-on-failure
```
Result: 10/10 tests passed.

## Notes
- Codex independently ran the exact native contract executable with local realm-v6 models: 75 checks passed. `ctest --test-dir build/m5-t1b-native --output-on-failure -R runtime_contracts`: 1/1 passed. Core CTest independently passed 10/10; scoped `git diff --check` exited 0.
- `advance` rejects while paused before learned decisions and core day advance.
- `advance` now preflights the Realm maximum horizon before any policy decision or inference.
- Horizon overflow is not exercised in the native contract binary because reaching the 365000-day cursor requires an impractical full-length run or a non-approved save fixture; the bound-safe early check uses `365000 * kSubdayUnitsPerDay`.
- No raw tick-advance API was added.
- Runtime load remains unpaused.
- Headless CLI and model compatibility unchanged.
EOF
