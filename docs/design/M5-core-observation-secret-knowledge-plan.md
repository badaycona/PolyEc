# Core/M5.5 — secret-knowledge observation index

Date: 2026-10-21. Follow-up to the decision snapshot profile. The current observation builder scans every retained fact for every actor to compute feature 87, then performs a knowledge-map lookup for secret facts. With up to 8,192 retained facts and population-scale inference, this repeats work that is unchanged across most observations.

## Contract

- Preserve observation feature 87 exactly: count secret facts for which the existing `knows(observer, fact)` returns true, divided by 10.
- Keep candidate arrays, mask, actions, save bytes, schema, Python/Godot bindings and model contract unchanged.
- Maintain a derived per-person count. Rebuild it at day boundaries and load to account for active knowledge arrivals; update it for secret events and completed investigations. Direct actor/target visibility must not be double-counted with investigation knowledge.
- If facts are pruned, rebuild from retained facts and current knowledge index.

## Files and gate

- `core/include/polyec/realm.hpp`, `core/src/realm.cpp`: derived count storage/rebuild/update and constant-time observation read.
- `core/tests/test_decision_snapshot.cpp` or existing Realm test: compare feature 87 against an independent naive fact/knowledge oracle over saved fixtures and observation parity; include population/day advancement and serialization roundtrip.
- Build and run targeted plus full Release/Debug CTest. Run the same 100k/128 and 100k/512 three-repeat profile B and report whether the ≤80 ms gate passes.

Self-review only; this is a measured core optimization and does not close M5.5 or the larger M5 goal.
