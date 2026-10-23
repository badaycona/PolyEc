# Core refactor — staged death settlement boundary

Date: 2026-10-21. This is an infrastructure slice for the open M5 estate/ownership work; it preserves the exact existing death disposition and does not claim inheritance or probate.

## Contract and scope

Today `Realm::day_people` marks a deceased Person, moves all remaining personal cash into the shared `estate_` balance, moves inventory into the person's current Province stock, and emits the death fact. Extract only the first three state changes into a stateless `EstateManager` prepare/apply boundary. Realm retains death eligibility, event ordering, Person/Province/fund ownership, day phase order, RNG, and save ownership.

Preserve existing gameplay exactly: no beneficiary selection, no new estate records or claims, no transfer of title/household money, no debt settlement, no inventory liquidation, and no change to dead Person identity/history. The manager plan snapshots the Person and Province read set plus estate balance, checks nonnegative values and checked additions before apply, then validates the exact read set before any write. A failed prepare or stale apply leaves all inputs unchanged. Apply marks the Person dead, zeros only remaining personal cash/inventory, adds cash to `estate_`, and adds each inventory quantity to Province stock.

## Files and ownership

- Add `core/include/polyec/managers/estate_manager.hpp`, a pure value helper with an opaque `EstateDeathPlan`.
- Change `core/src/realm.cpp` only at the existing death settlement block; Realm keeps eligibility and calls prepare/apply before the existing death event.
- Add direct manager oracles in `core/tests/test_managers.cpp`; preserve Realm death/save/fingerprint coverage in `core/tests/test_realm.cpp`.
- No save schema, Python, native, GDExt, Godot, action, observation, reward, model, or gameplay change.

## Gates

1. Pure prepare leaves inputs unchanged; applying to matching input reproduces exact old disposition and conserves total money/goods.
2. Negative/overflowing cash, estate or inventory destination rejects before mutation; stale Person, Province or estate read-set rejects atomically; successful plan cannot be applied twice.
3. Windows Release/Debug builds and full CTest 19/19 each; direct manager checks and Realm death/serialization tests. Scoped diff and repository-link check.

Self-review: the boundary extracts only already-running behavior and does not choose heirs. It is a safe staging point for the later, separately contracted estate waterfall. No independent reviewer is assigned.
