# M5 — T1a Realm schedule timeline evidence (2026-10-18)

Build/test platform: Windows local, Clang 18.1.6 via Zig, CMake 3.31.6, Ninja.

Commands:
```
D:/Code/PolyEc/build/tooling/cmake/data/bin/cmake.exe -S . -B build/m5-t1a -G Ninja -DCMAKE_MAKE_PROGRAM=D:/Code/PolyEc/build/tooling/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=D:/Code/PolyEc/build/tooling/zig-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=OFF -DPOLYEC_BUILD_PYTHON=OFF
D:/Code/PolyEc/build/tooling/cmake/data/bin/cmake.exe --build build/m5-t1a --parallel 4
D:/Code/PolyEc/build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-t1a --output-on-failure
```

Result: **CTest 8/8 passed** (core, trade, realm, court, campaign, manager, activity, schedule). `schedule_tests` passed with 3513 assertions.

Correction 2026-10-18: removed redundant unsafe invariant line `if(s.capacity_unmet+s.unresolved_unmet!=s.unmet)return false;` from `Realm::invariant` schedule loop; correct subtraction check `if(s.unresolved_unmet!=s.unmet-s.capacity_unmet)return false;` remains, with existing `s.capacity_unmet<=s.unmet` guard. Verified unsafe addition pattern absent. Actual run: build `build/m5-t1a` succeeded (12 targets), CTest 8/8 passed (core, trade, realm, court, campaign, manager, activity, schedule), total 6.81 sec.

Scope delivered: sparse per-actor `ScheduleBook`, global cursor with 64 subdivisions/day, submit/cancel/reprioritize through `Realm::execute(ScheduleCommand)` on the shared `Person::decisions` sequence, `advance_ticks`, v7 save with `SCHEDULE_1`, native restore guard, invariant validation, sticky timeline contract and legacy policy guard.

Limitations: native/Python/schema parity not built or verified; T1b clock/pause adapter not implemented; T2 settlement not implemented; T1 is not complete. Performance claims are not made.
