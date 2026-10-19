# M5-T1b2c evidence — manual Realm day and Godot clock synchronization

## Scope
Realm `StepOneDay`, native runtime manual day bridge, GDExt clock bindings, Godot Realm panel/main focus synchronization, and related tests/fixture. No Campaign clock migration, Python parity acceptance, T2 settlement, model/action/observation/reward changes, commit, push, training or cloud purchase.

## Files changed
- `core/include/polyec/realm_clock.hpp`
- `core/include/polyec/realm.hpp`
- `core/src/realm.cpp`
- `core/tests/test_realm_clock.cpp`
- `native/include/polyec/realm_runtime.hpp`
- `native/realm_runtime.cpp`
- `native/realm_contracts.cpp`
- `gdext/polyec_session.cpp`
- `godot_project/ui/main.gd`
- `godot_project/ui/realm/realm_panel.gd`
- `godot_project/tests/realm_smoke.gd`
- `godot_project/tests/realm_expected.json`
- `docs/design/M5-T1b2c-godot-clock-plan.md`

Unrelated dirty docs and other-task files were preserved.

## Build
Platform: Windows, Clang/Zig, Ninja, Release, Godot 4.5.1 console, ONNX Runtime 1.31.0.

Dedicated build directory:
```powershell
& build/tooling/cmake/data/bin/cmake.exe -S . -B build/m5-t1b-godot -G Ninja `
  -DCMAKE_MAKE_PROGRAM=D:/Code/PolyEc/build/tooling/bin/ninja.exe `
  -DCMAKE_TOOLCHAIN_FILE=D:/Code/PolyEc/build/tooling/zig-toolchain.cmake `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_SIZEOF_VOID_P=8 `
  -DORT_ROOT=D:/Code/PolyEc/build/tooling/ort-1.31.0/onnxruntime-win-x64-1.31.0 `
  -DPOLYEC_BUILD_APP=ON -DPOLYEC_BUILD_AI=ON -DPOLYEC_BUILD_PYTHON=OFF `
  -DGODOTCPP_DISABLE_EXCEPTIONS=OFF
```

Two configure details were required:
- `CMAKE_SIZEOF_VOID_P=8` was set because godot-cpp otherwise failed with `math cannot parse the expression: " * 8"`.
- `GODOTCPP_DISABLE_EXCEPTIONS=OFF` was set because existing native code uses `try/catch`.

Build:
```powershell
& build/tooling/cmake/data/bin/cmake.exe --build build/m5-t1b-godot --parallel 4
```
Result: build success. Only existing deprecated `std::filesystem::u8path` warnings were emitted.

## Verification
Full CTest:
```powershell
& build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-t1b-godot -C Release --output-on-failure
```
Result: `100% tests passed, 0 tests failed out of 11`; total 11.27 s.

Core clock test:
```powershell
& build/m5-t1b-godot/core/polyec_realm_clock_tests.exe
```
Result: `realm clock checks=254`, exit 0.

Native runtime contracts:
```powershell
& build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-t1b-godot -C Release -R runtime_contracts --output-on-failure -V
```
Result: `RUNTIME CONTRACTS: 112 checks passed`, exit 0. Includes missing-save restore rejection with unchanged live fingerprint and counters.

Full native/Godot gate:
```powershell
& scripts/test-native.ps1 `
  -Godot "D:/Code/PolyEc/build/tooling/godot-4.5.1/Godot_v4.5.1-stable_win64_console.exe" `
  -CTest "D:/Code/PolyEc/build/tooling/cmake/data/bin/ctest.exe" `
  -BuildDir "build/m5-t1b-godot" -Config Release -IncludeRealm
```
Result: exit 0; CTest 11/11; `NATIVE SMOKE: 19/19`; `UI SMOKE: 18/18`; `CAMPAIGN SMOKE: 39/39`; `REALM SMOKE: 471/471`.

Standalone Realm smoke:
```powershell
& "D:/Code/PolyEc/build/tooling/godot-4.5.1/Godot_v4.5.1-stable_win64_console.exe" `
  --headless --path "D:/Code/PolyEc/godot_project" --script "res://tests/realm_smoke.gd"
```
Result: `REALM SMOKE: 471/471 checks passed`, exit 0.

## Verified behavior
- Manual `StepOneDay` advances exactly 64 subday units and leaves both pause state and pause revision unchanged.
- Ordinary paused `advance`/`advance_ticks` still reject.
- Maximum-horizon final day is accepted; the next manual day is rejected without mutation.
- Runtime manual step requires a loaded world and chosen draft, performs Governance → Economic → Civilian inference, then issues the core command.
- Interactive model load starts paused; UI resume requires explicit `realm_resume()`.
- Focus-out pauses the core and stops the UI, including app-level focus loss after session deactivation ordering was corrected.
- Focus-in does not resume the core or UI.
- Manual “Một ngày” pauses/stops a running UI loop before stepping.
- UI save/load stops the loop and leaves the world paused; a failed load preserves the live world.
- Corrupt/missing save restore no longer throws across the Godot extension boundary.
- Guide, long-frame guard, close panel, back-to-map and app focus loss all fail closed; a failed back/close pause keeps the panel visible with the pause error.

## Fixture change
`godot_project/tests/realm_expected.json` fingerprints were regenerated for the new interactive-load pause/resume revision state. The `money`, `alive` and `decisions` fields were unchanged. Fingerprints include `CLOCK_1` pause state/revision, so the old fixture could not match after the approved load-paused behavior.

## Failure fixed during verification
Before the restore refactor, `realm_smoke` printed `445/445 checks passed` but exited with `-1073741819` (`0xC0000005`) when a failed restore path threw a C++ exception. A native equivalent returned a normal error and exit 0. Expected restore validation failures now return `error_` instead of throwing; the smoke exits 0. This was a GDExt/Godot boundary failure, not a Realm invariants failure.

## Limitations
- Godot smoke is headless; no packaged-window or visual review.
- Campaign’s separate clock path was not migrated in this slice.
- Python portable parity is not re-accepted here.
- No commit, push, training, long profiling or cloud resources.
