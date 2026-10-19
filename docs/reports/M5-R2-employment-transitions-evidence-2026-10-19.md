# M5 R2 — Employment transitions evidence (2026-10-19)

## Result

R2 extracts work/office transition mechanics into stateless `EmploymentManager` helpers while keeping `Realm` as sole owner of `Person` and `Province` state and the command legality boundary. Codex reviewed the final diff and accepted this slice. No save schema, binding, observation, reward, model, or gameplay contract changed.

## Scope

The implementation stayed within the approved boundary:

- `core/include/polyec/managers/employment_manager.hpp`
- `core/src/employment_manager.cpp`
- `core/CMakeLists.txt`
- `core/src/realm.cpp`
- `core/tests/test_managers.cpp`
- `core/tests/test_realm.cpp`
- `core/tests/test_realm_court.cpp`

The manager owns no persistent data. Realm still selects referral candidates, validates commands, owns counters and assignments, and invokes transitions.

## Compatibility and atomicity

Existing work eligibility, teacher eligibility, counter rebuild/accounting, assignment clearing, appointment referral order, appointment fee, salary period, notice and resignation behavior are preserved. Expired/stale assignment fields retain the legacy behavior. The save format and T2 production/payment rules are unchanged.

Pure preflights now include representable `day + term` for Work and the appointment cash/fee and day/timer boundaries for Appoint. `Realm::legal` calls these checks before `execute` can clear an existing assignment. The appointment query validates the first living candidate in the existing referral order; it does not skip that candidate for a later one. The Realm integration test confirms a rejected appointment leaves the fingerprint, actor assignment and province employment counters unchanged.

The live Realm format bounds the day to 365000 and `invariant()` requires a representable total-money ledger. Therefore day/cash overflow states cannot be represented by a valid Realm fixture. Those boundaries are tested directly on the pure preflights; they are not claimed as live Realm scenarios.

## Windows verification

Release build command:

```powershell
& "build\tooling\cmake\data\bin\cmake.exe" --build "build\m5-r1-payroll" --config Release
```

Result: exit 0. Two existing `std::filesystem::u8path` deprecation warnings appeared in `realm.cpp` save/load.

Targeted executables:

```powershell
& "build\m5-r1-payroll\core\polyec_manager_tests.exe"
& "build\m5-r1-payroll\core\polyec_realm_tests.exe"
& "build\m5-r1-payroll\core\polyec_court_tests.exe"
```

Results: manager tests passed; Realm 25,349 checks passed; Court 196 checks passed. The Realm profile reported 10,000 persons, 19,743 sparse edges, and 90 days in 0.225 seconds for this run.

Full regression:

```powershell
& "build\tooling\cmake\data\bin\ctest.exe" --test-dir "build\m5-r1-payroll" -C Release --output-on-failure
```

Result: 10/10 passed in 9.29 seconds.

Scoped whitespace check:

```powershell
git diff --check -- core/CMakeLists.txt core/src/realm.cpp core/tests/test_managers.cpp core/tests/test_realm.cpp core/tests/test_realm_court.cpp
```

Result: clean; Git printed line-ending conversion warnings for several existing mixed-line-ending files.

## Remaining work

R2 does not implement actual work-time accounting or end-of-day production/payment/material settlement. R3 time orchestration is the next plan-only gate under `docs/design/core-architecture-refactor.md` §7.3/§9. The T2 gameplay choice for what counts as actual work time remains open and requires user review before that settlement work begins. Only Windows is in the accepted platform scope; Linux parity is deferred by the user's decision.
