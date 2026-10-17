# Copilot core repair plan (approved, 2026-10-17)

Scope approved by Codex. Goal: repair revision `a5647a5` damage (literal patch
snippets committed into core files; misplaced sim implementation) while preserving
the intended manager extraction and the `WorldState` refactor from `4ac32a3`.

## Files in scope

- `core/CMakeLists.txt` — restore full file from `d5751bb`, keep manager sources
  in the `polyec_core` target, add `tests/test_managers.cpp` target + `add_test`.
- `core/include/polyec/realm.hpp` — restore complete content from
  `git show d5751bb:core/include/polyec/realm.hpp`; **no** public mutable
  accessors (the `*_mutable()` block from `3bc1b98` is dropped).
- `core/src/realm.cpp` — restore complete content from
  `git show d5751bb:core/src/realm.cpp`, then re-apply the intended extraction
  selectively:
  - include `managers/{household_index,finance_manager,project_manager}.hpp`;
  - `Realm::transfer` delegates to `FinanceManager::transfer` (same throw
    semantics: `logic_error("unfunded transfer")`);
  - `Realm::reindex_households` delegates to `HouseholdIndex::build`, which must
    preserve the zero-label check (`logic_error("household membership label")`)
    instead of silently skipping `household == 0`;
  - `Intent::Project` legality check uses
    `ProjectManager::active_projects_count(projects_, p.region, c.good)` with the
    original exact criteria (`!completed && region && kind`) and original
    call-site threshold (`c.good==3 && draft_.chosen==10 ? 2 : 1`);
  - `Realm::payroll` delegates to `FinanceManager::payroll` with full parity
    (see below).
- `core/include/polyec/command.hpp` — restore original declaration-only content
  from `d5751bb` (CommandType/Command/CommandResult/PlayerView).
- `core/src/sim.cpp` — hold the `GameWorld`/`SimulationController`
  implementations currently (wrongly) sitting in `command.hpp`, using
  `m_state` (`WorldState`), preserving `4ac32a3` snapshot/restore behavior.
- `core/include/polyec/sim.hpp`, `core/include/polyec/types.hpp` — unchanged
  (current `WorldState` ownership stays).
- Managers (`core/include/polyec/managers/*.hpp`, `core/src/*.cpp`):
  `finance_manager.hpp` rewritten (below); `household_index.hpp` keeps
  zero-label throw + `<algorithm>`; `project_manager.hpp` keeps original
  criteria; headers include correct types (`realm.hpp` for Person/Province/
  Project) with no include cycles (managers include realm.hpp; realm.cpp
  includes managers).
- `core/tests/test_managers.cpp` — new focused tests (below).
- `WORK_STATUS.md` / this doc — status note only if needed.

No action/schema/reward/model changes, no gameplay changes, no commit/push,
no training, no subagents. User doc changes kept.

## FinanceManager::payroll parity contract (restored original behavior)

Original `Realm::payroll` (d5751bb):

1. Empty claims → return, no mutation.
2. `account = public_employer ? province.treasury : province.firm`;
   `due = sum(claims)`, `budget = min(account, due)`, pro-rata
   `paid[i] = budget * claim[i] / due`, remainder distributed by rotating index
   `(day + offset) % n` — **day-based**, not region-based.
3. Per claim: `transfer(account, p.cash, paid[i])`; tax =
   `(paid[i]*v.tax + p.tax_remainder) / 100`, remainder kept on the person;
   tax moves person cash → treasury via `transfer(p.cash, v.treasury, tax)`
   and increments `province.tax_receipts` (counter only; `Realm::total_money`
   excludes `tax_receipts`).

Repairs vs the broken manager: pass `day` for rounding; tax debits person cash
(conservation); `tax_receipts` incremented; prevalidate claim person IDs
(`id == 0 || id > people.size()` → `std::out_of_range("person ID")`) and
`due == 0`/empty before any mutation so invalid input never partially mutates
state (valid-input behavior unchanged); zero total handled safely (no div-by-
zero). Invalid region index also rejected before mutation. Realm payroll
call-site in `daily()` unchanged.

## Tests (focused manager parity; existing 5 CTest targets retained)

`core/tests/test_managers.cpp` (plain asserts, new `manager_tests` target):
- valid private (firm) and public (treasury) payroll: amounts, tax, receipts,
  day-based rotating remainder (two different days give different rounding
  beneficiary);
- partial funds: `budget < due` pro-rata split + remainder;
- conservation: people cash + firm + treasury constant across payroll;
  `tax_receipts` counter grows, not part of money conservation;
- empty claims, all-zero claims (zero total), invalid person ID and invalid
  region → throw/no-mutation (state snapshot equal);
- transfer: valid move, unfunded/negative throw;
- HouseholdIndex::build parity: sorted members, zero-label throws;
- ProjectManager: active criteria (`!completed`, region, kind) exact.

Existing targets kept: `core_tests`, `trade_tests`, `realm_tests`,
`court_tests`, `campaign_tests` (plus replay executables, unadded to CTest as
before).

## Acceptance commands (run as separate operations)

```powershell
& build/tooling/cmake/data/bin/cmake.exe -S . -B build/copilot-core-repair -G Ninja -DCMAKE_MAKE_PROGRAM=D:/Code/PolyEc/build/tooling/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=D:/Code/PolyEc/build/tooling/zig-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=OFF -DPOLYEC_BUILD_PYTHON=OFF
cmake --build build/copilot-core-repair --parallel 4
build/tooling/cmake/data/bin/ctest.exe --test-dir build/copilot-core-repair --output-on-failure
```

Restore via `git show d5751bb:<path>` (no checkout/reset); writes use .NET
UTF-8 (no BOM) to preserve source encoding. Stop after reporting actual
results for Codex review.
