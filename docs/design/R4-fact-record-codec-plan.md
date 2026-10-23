# R4 Fact record codec — approved plan

Review date: 2026-10-20. This is a behavior-preserving R4 save-record extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9.

## Scope and files

Extract only the `facts_` count and primary Fact records from `Realm::serialize` / `Realm::deserialize` into stateless inline helpers.

- Add `core/include/polyec/fact_section_codec.hpp` with `polyec::detail::write_fact_section(std::ostream&, const std::vector<Fact>&)` and `read_fact_section(std::istream&, std::vector<Fact>&)`.
- In `core/src/realm.cpp`, add the include and replace only the inline Fact writer/reader loops.
- In `core/tests/test_realm.cpp`, add focused codec tests modeled on the accepted Loan/Project codec blocks.
- No CMake, schema, gameplay, court, bindings, UI, native, Godot, Python, model, or archive changes.

## Token grammar and errors

Writer order remains:

```text
<count> (<id> <parent> <actor> <target> <day> <region> <kind> <amount> <secret>)*
```

Count cap remains 8,192. Malformed/excessive count and incomplete record throw `std::runtime_error("Save count limit")`. The current inline reader notices a truncated record at the next Knowledge count; the helper may reject at the Fact boundary with the same error. Add no field validation. Read to a local vector and publish only after all nine fields per record parse.

Tests check excessive/bad count, truncated record, output preservation after error, exact writer bytes `1 7 8 9 10 11 12 13 14 0 ` and valid fields. Existing Realm fingerprint/continuation and court tests remain the end-to-end oracle.

## Ownership and derived indexes

The codec is stateless and handles only primary Fact fields. Realm remains owner of `facts_`, stages the full load, validates and atomically commits at the existing final move. Facts have no derived index. The existing `knowledge_index_` reconstruction from `knowledge_`, household reindex, and all whole-world checks stay in Realm. No court extension applies to Fact.

## Windows gates

Reuse only `build/m5-core`; no configure or new generator. Run the Release full build/CTest, Debug full build/targeted CTest, and `git diff --check` commands from [R4 Loan codec plan](R4-loan-record-codec-plan.md).

## Deferred

Knowledge/Proposal/Investigation records, court extensions, schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
