# Core refactor — typed Realm relation kinds

Date: 2026-10-21. Short source-only refactor within the user's ongoing M5/core-refactor delegation.

## Contract

- Replace `Edge::kind` raw `int` with `RelationKind : uint8_t` values `ParentChild=0`, `Sibling=1`, and `Agreement=2`, matching the currently observed save values and call sites.
- Keep Realm ownership of relation records and preserve save tokens, ordering, schema, fingerprints for valid existing worlds, and public relationship queries.
- Edge codec remains stateless: serialize the enum as its current integer token; parse through an integer temporary and reject values outside 0..2 before publishing staged records.
- Update only relation producers/consumers and tests. No new relation semantics, marriage, ownership, inheritance, or model changes.

## Files and gates

- `core/include/polyec/realm.hpp`, `core/include/polyec/edge_section_codec.hpp`, `core/src/realm.cpp`, `core/src/realm_court.cpp`, and relation-focused tests.
- Release full CTest; Debug targeted Realm/court tests; verify valid serialization token parity and invalid relation-kind atomic rejection; scoped `git diff --check`.

## Review status

Codex self-reviewed contract and implementation. Enum names mirror current source use (birth/parent edge, sibling edge, accepted agreement edge); values are pinned to established save tokens. This is source-level type safety only. Final diff was self-reviewed; no independent reviewer.

## Acceptance evidence

- VS18 Release build completed; full CTest **16/16 passed**.
- VS18 Debug build completed for `polyec_realm_tests` and `polyec_court_tests`; targeted CTest **2/2 passed** (128.67 s).
- Direct codec checks verify the exact existing token `1 7 8 2 `, enum roundtrip, and atomic rejection of kind `3`.
- Existing Realm migration/save/replay coverage remained green. Schema and valid save token order did not change.
- `git diff --check` scoped to the slice passed. No Python/native/Godot behavior was changed by this type-only refactor.

Evidence report: [M5 core relation-kind report](../reports/M5-core-relation-kind-evidence-2026-10-21.md).
