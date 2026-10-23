# R4 RealmProjection view-construction boundary — plan-only assessment

Review date: 2026-10-20. This is a **plan-only** assessment under [core architecture refactor](core-architecture-refactor.md) §7/§9 and [core class reference](core-class-reference.md) §12.10. No source, build, test or implementation change is included.

Review: **not worthwhile for this slice** — bounded reason below. The selected next R4 task is **R4 CLOCK_1 save-section codec** (plan-only, pending Codex review). No commit/push.

## 1. Candidate inspected

- `Realm::view(uint64_t observer) const` in `core/src/realm.cpp:739`, declared in `core/include/polyec/realm.hpp:93`.
- `RealmView` in `core/include/polyec/realm.hpp:67`.
- Private `Realm::knows(uint64_t observer, const Fact& f) const` in `core/src/realm.cpp:179`.
- Direct callers: `bindings/python/realm_binding.cpp` (`realm_view`), `core/src/campaign.cpp` (`world_.view()`), core tests (`test_realm.cpp`, `test_realm_court.cpp`, `test_schedule.cpp`, `realm_clock_replay.cpp`, `test_campaign.cpp`), and Godot-facing read paths through Campaign/Realm views.

## 2. Current behavior (must be preserved if ever extracted)

```cpp
RealmView Realm::view(uint64_t observer) const {
    RealmView v;
    v.day = day_;
    v.regions = regions_;
    v.persons = people_.size();
    v.money = total_money();
    v.provinces = provinces_;          // full copy, intentional
    v.draft = draft_;
    for (auto& p : people_) {
        v.alive += p.alive;
        v.children += p.alive && p.age_days < 18 * 365;
        v.decisions += p.decisions;
    }
    for (auto& f : facts_)
        if (knows(observer, f)) v.facts.push_back(f);
    return v;
}
```

Preservation requirements if touched later:
- observer knowledge filtering via `knows`, including secret-fact investigation timing and regional/public reporting delay;
- aggregate counts (`persons`, `alive`, `children`, `decisions`, `money`);
- province vector copy and order;
- fact order (input order of `facts_`);
- draft copy;
- no writable access to Realm state through `RealmView`;
- no projection cache, index, model or action changes.

## 3. Bounded reason: extraction is not worthwhile now

1. `Realm::view` is already a single `const` query returning a value copy. It performs no mutation, no save serialization, no cache update and no index rebuild.
2. Its only nontrivial part is observer filtering, which is implemented by private `Realm::knows` and depends on `people_`, `facts_`, `knowledge_index_`, `report_delay`, and day state. Moving construction into a separate stateless component would require either friend access or exposing private Realm state, adding coupling without reducing risk.
3. The Province vector is intentionally copied in full, as documented in `core-class-reference.md` §3.11. Extracting the constructor cannot change that privacy scope without a separate contract and would only relocate the same copy.
4. There is no independent oracle beyond comparing identical output; the extraction would be mechanical relocation, not a behavior-preserving boundary with new test value.
5. `core-architecture-refactor.md` warns against adding classes that only forward calls. A `RealmProjection` wrapper around this method would be such a class unless a concrete permission/UI contract first requires it.

## 4. Selected next clearly contract-backed R4 task

- **R4 CLOCK_1 save-section codec** (plan-only next).
- It has a fixed byte grammar documented in `M5-T1b1-realm-clock-plan.md`, known validation (`paused` boolean, `pause_revision`, tag), and follows the accepted R4 stateless codec pattern (stage locally, preserve stream position and error mappings, Realm retains ownership/atomic commit).
- No schema, gameplay, binding, native/Godot/Python, model, performance or training change.
- Plan and gates to be drafted separately and reviewed by Codex before any code.

## 5. Deferred

- Revisit `Realm::view` / `RealmProjection` only when a concrete contract requires changing observer permission scope, copy isolation guarantees, or UI/observation consumption.
- `CLOCK_1` extraction remains the next R4 candidate.
- No commit/push.
- M5 gameplay/economic contracts remain deferred.
