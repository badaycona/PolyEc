# M5 — R1 payroll prepare/apply evidence (2026-10-19)

Contract: [core architecture refactor](../design/core-architecture-refactor.md) §7.1/§9.

Baseline: `6cc55ab`; existing dirty T1/T2/docs files preserved. This slice changed only the two files listed below.

## Implementation files

- Modified: `core/include/polyec/managers/finance_manager.hpp` — opaque `PayrollPlan`, pure `prepare(...)`, validating `apply(plan, current_day, people, provinces)`, thin `payroll(...)` wrapper, checked arithmetic.
- Modified: `core/tests/test_managers.cpp` — compile-time plan-opacity assertion plus R1 behavior/reject/overflow tests. Existing manager tests unchanged (zero deleted lines).
- No change: `core/src/realm.cpp` (caller `Realm::payroll` still calls the wrapper), `core/src/finance_manager.cpp`, `core/CMakeLists.txt`, save schema, native/Godot/Python bindings, action/observation/reward/model, gameplay rules.

## Behavior boundaries

- `PayrollPlan` has a private default constructor with `FinanceManager` as the only friend; callers receive plans only from `prepare` and cannot forge one. Accessors are const and scalar-only.
- `prepare` mutates nothing. It preserves the original early-return order (empty claims → region → per-claim ID/negative validation → due==0) and, on valid input, claim order and duplicates, pro-rata integer truncation, day-keyed rotation, budget-before-tax, per-person `tax_remainder`, `tax_receipts` as counter, and treasury as both wage source (public) and tax destination.
- New state-domain validation (negative tax/account/treasury/receipts/cash, remainder outside 0–99) and checked arithmetic run only on the `due>0` path, before any write. Reject types are preserved: `std::out_of_range` for region/ID/index, `std::logic_error` for unfunded/negative/stale/domain errors, `std::overflow_error` for checked arithmetic.
- Checked arithmetic covers the due sum, `budget*claim`, cash+paid, assessed tax, and account/treasury/`tax_receipts`/final balances.
- `apply` revalidates the plan's exact read set before writing: day, region, account/treasury (public alias checked as one field), tax, receipts, and each recipient index + canonical id + cash + remainder. It allocates nothing and performs no throwing operation after the first write. A public plan does not read `firm`; non-recipient people are outside the read set.
- Canonical ID: on the first read of a recipient, `prepare` requires `people[idx].id == claims[i].first` and rejects inconsistency before planning. Production `Realm` already enforces `p.id == index+1` (`realm.cpp:434`).
- Sparse lookup: recipient slots are found through `std::unordered_map<size_t,size_t>` (index → slot) using only `find`/`emplace`. The recipient vector is still appended in first-encounter order, and results are computed by iterating claims, never the map.

## Commands and results

Platform: Windows local, Clang 18.1.6 via Zig toolchain, CMake 3.31.6, Ninja, Release. Build directory `build/m5-r1-payroll` was configured successfully earlier in this slice with:

```
build/tooling/cmake/data/bin/cmake.exe -S . -B build/m5-r1-payroll -G Ninja -DCMAKE_MAKE_PROGRAM=D:/Code/PolyEc/build/tooling/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=D:/Code/PolyEc/build/tooling/zig-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=OFF -DPOLYEC_BUILD_PYTHON=OFF
```

Final corrected source was then rebuilt and tested:

```
build/tooling/cmake/data/bin/cmake.exe --build build/m5-r1-payroll --parallel 4
build/m5-r1-payroll/core/polyec_manager_tests.exe
build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-r1-payroll --output-on-failure
git diff --check -- core/include/polyec/managers/finance_manager.hpp core/tests/test_managers.cpp
```

- Configure: exit 0; `Clang 18.1.6` compiler identification, configure/generate OK.
- Build: exit 0. Only pre-existing `std::filesystem::u8path` deprecation warnings from `realm.cpp`/`campaign.cpp`; no new warnings from the changed files.
- Manager tests: `manager tests passed`, exit 0.
- Full CTest: **10/10 passed** (`core_tests`, `trade_tests`, `realm_tests`, `court_tests`, `campaign_tests`, `manager_tests`, `activity_tests`, `schedule_tests`, `realm_clock_tests`, `realm_clock_replay`), total 9.05 s, exit 0.
- Scoped `git diff --check`: exit 0.

Test surface: 19 test functions and 101 runtime `check()` call sites (35 pre-existing + 66 R1-related), plus one compile-time assertion. R1 coverage includes high-tax-funded-by-opening-cash (tax above wage), underfunded-tax atomic reject, duplicate-claim sequential remainder, prepare purity/determinism, double-apply rejection, stale day/tax/firm/treasury/person-id/recipient-cash/recipient-remainder/shrunk-vector rejects with no additional mutation, exact read-set scope, state-domain validation, canonical-ID rejection, due/budget-product/final-cash/receipts overflow rejects with no mutation, early-return ordering, and wrapper/`prepare`+`apply` parity.

## Compile-time nonconstructibility

`test_managers.cpp` contains:

```
static_assert(!std::is_default_constructible_v<polyec::PayrollPlan>,
              "PayrollPlan must not be default constructible by callers");
```

A standalone probe compiled with the same toolchain (`build/tooling/ziglang/zig.exe c++ -std=c++20 -c`, exit 0) confirmed this libc++ reports `is_default_constructible_v` false for a private default constructor, so the assertion is meaningful and compiled in the manager target.

## Correction-round evidence

Codex final diff review requested four changes; all were applied without altering behavior/API or existing tests:

1. Private `PayrollPlan()` (with `FinanceManager` friend) and the compile-time assertion above.
2. Sparse `unordered_map` recipient lookup replacing the linear scan, preserving claim order and deterministic recipient order (no map iteration for results).
3. Canonical-ID validation on first recipient read, with an atomic rejection test (`people[0].id = 2` vs claim `{1,100}` → `logic_error`, all balances/receipts unchanged).
4. Read-set drift tests for recipient cash and `tax_remainder`, plus two reachable overflow fixtures:
   - final cash addition: `cash=INT64_MAX`, `firm=1`, `tax=0`, claim `{1,1}` → `std::overflow_error`; `World::money()` intentionally not used in this fixture because summing `INT64_MAX+1` would overflow the test helper;
   - `tax_receipts` addition: `firm=110`, `treasury=0`, `tax=10`, `tax_receipts=INT64_MAX-10`, claim `{1,110}` → tax 11; treasury stays small so the receipts addition is the first overflow.

Correction round was rebuilt (`--target polyec_manager_tests` then full `--parallel 4`), direct manager tests passed, and full CTest was 10/10 (9.05 s). No additional negative-control mutation was performed in the correction round.

Earlier in the slice, one negative control was executed before acceptance: a single expectation was flipped (`cash == 60` → `61`), the manager target rebuilt, and the run produced `FAIL: tax above wage funded by opening cash`, `1 manager test failure(s)`, exit 1; the expectation was then reverted and the target rebuilt green.

## Limitations

- Windows-only evidence. Per the user decision of 2026-10-19, Linux build/runtime/package parity is deferred and is not an M5 gate; no Linux check was run.
- No commit/push/train and no cloud spend.
- No save-schema change, no persistent settlement ID, no T2 settlement or gameplay change, and no R2 code started. M5 remains open.
- The "no allocation/no throw after first write" property of `apply` is code-reviewed rather than instrumented at runtime.
