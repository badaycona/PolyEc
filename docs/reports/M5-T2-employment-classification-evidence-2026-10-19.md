# M5 — T2 Employment/Job classification foundation evidence (2026-10-19)

Plan: [M5-T2-employment-classification-plan](../design/M5-T2-employment-classification-plan.md).

Build/test platform: Windows local, Clang 18.1.6 via Zig toolchain, CMake 3.31.6, Ninja, Release.

## Implementation files

- New: `core/include/polyec/jobs/job.hpp`, `worker_job.hpp`, `teacher_job.hpp`, `office_role_job.hpp`.
- Modified: `core/include/polyec/realm.hpp` (resolver declaration + query declarations), `core/src/realm.cpp` (resolver + delegation), `bindings/python/realm_binding.cpp` (read-only person-dict fields), `core/tests/test_realm.cpp` (classification tests).
- No change: `core/CMakeLists.txt`, save schema, lifecycle, payroll, counters, RL fields, native, Godot.

## Commands and results

```
build/tooling/cmake/data/bin/cmake.exe -S . -B build/m5-job-classify -G Ninja -DCMAKE_MAKE_PROGRAM=D:/Code/PolyEc/build/tooling/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=D:/Code/PolyEc/build/tooling/zig-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=OFF -DPOLYEC_BUILD_PYTHON=OFF
build/tooling/cmake/data/bin/cmake.exe --build build/m5-job-classify --parallel 4
build/m5-job-classify/core/polyec_realm_tests.exe
build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-job-classify --output-on-failure
```

- Realm tests: **25,322 checks passed** (exit 0).
- Full CTest: **10/10 passed** (core, trade, realm, court, campaign, manager, activity, schedule, realm_clock, realm_clock_replay), total 7.44 sec.

Python binding (separate dir, pybind11 from tooling cache):

```
build/tooling/cmake/data/bin/cmake.exe -S . -B build/m5-job-classify-python -G Ninja -DCMAKE_MAKE_PROGRAM=D:/Code/PolyEc/build/tooling/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=D:/Code/PolyEc/build/tooling/zig-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=OFF -DPOLYEC_BUILD_PYTHON=ON -Dpybind11_DIR=D:/Code/PolyEc/build/tooling/pybind11/share/cmake/pybind11
build/tooling/cmake/data/bin/cmake.exe --build build/m5-job-classify-python --parallel 4
& C:/Users/Admin/scoop/apps/python/current/python.exe -c "import sys; sys.path.insert(0, 'build/m5-job-classify-python'); import polyec_env as pe; w=pe.Realm(37,6,256); p=w.person(129); assert p['employment_state']=='Idle' and p['job_type']=='None'; assert pe.REALM_OBS==512; print('Python view OK; REALM_OBS', pe.REALM_OBS)"
```

- Module `polyec_env.cp314-win_amd64.pyd` linked OK.
- Smoke: person dict `employment_state: Idle`, `job_type: None` asserted; `REALM_OBS` asserted 512 (unchanged observation shape); output `Python view OK; REALM_OBS 512`.
- `git diff --check`: exit 0 (only CRLF/LF line-ending warnings from Git; no whitespace errors).

## Fix evidence

- First realm-test run failed at check 416 ("office classification"): the deterministic appointment test was checking a different Realm instance than the one the appointment happened in. Fixed by asserting on the instance that performed the appointment.
- Deterministic appointment test refactored to the court-fixture pattern (`Realm(seed,6,256)`, seed limit 2000, Appoint via `candidates(1)[15]`, unused `slot` variable removed) per Codex test-quality review.
- `job_descriptor` returns `const Job*` (nullptr for None/Idle/dead/expired/unknown), per Codex API review; a pure resolver `describe_job(const Person&, int)` was added so `job_type`/`employment_state`/`job_descriptor` share one logic path.

## Limitations

- No commit/push; baseline `6cc55ab` and existing T1 dirty files preserved.
- This slice is read-only classification only: no T2 actual work settlement, no native/Godot/RL change. M5 remains open.
