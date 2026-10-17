# M5 refactor notes

This document explains the recent refactor work to prepare the codebase for M5-level simulation changes.

Goals
- Separate mutable world state from runtime logic (WorldState) to make command validation atomic.
- Introduce small manager helpers (HouseholdIndex, FinanceManager, ProjectManager) as stable building blocks for further decomposition of Realm.
- Keep Realm as authoritative owner of its data while exposing controlled mutable accessors for stepwise refactor.
- Preserve public APIs (execute, observation, candidates) so external bindings (Godot, Python) remain stable.

What changed
- core/include/polyec/types.hpp: added WorldState struct to hold date, selected, territories, pause flag and pause_revision.
- core/include/polyec/sim.hpp, core/src/sim.cpp: GameWorld now stores WorldState and uses snapshot/restore to ensure rejected commands leave state unchanged.
- Added small manager helpers (header-only style) in core/include/polyec/managers/:
  - household_index.hpp
  - finance_manager.hpp
  - project_manager.hpp
- Exposed controlled mutable accessors in Realm to enable moving functionality into managers incrementally.
- Updated core/CMakeLists.txt to compile small manager source files.

Why this helps M5
- Atomic command validation: execute() snapshots state and restores it on rejection; this makes adding multi-step actions safer.
- Managers give clear places to move domain-specific logic (households, finance, projects) without creating large monolithic methods.
- The refactor is intentionally incremental and conservative: runtime behavior remains unchanged until managers replace Realm internal implementations.

Next steps
- Move financial logic (payroll, transfer, loans) from Realm into FinanceManager and replace direct calls with manager APIs.
- Move household indexing and related helpers into HouseholdIndex usage points and remove reindex_households from Realm when ready.
- Add unit tests covering manager behavior and verify Realm invariants after replacing implementations.

How to run tests (local)

cmake -S . -B build
cmake --build build --target polyec_tests
ctest --test-dir build --output-on-failure

(Tests were not run by the refactor operation; please run them locally or in CI and report failures.)
