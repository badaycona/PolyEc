# R4 Knowledge record codec — approved plan

Review date: 2026-10-20. This is a behavior-preserving R4 save-record extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9.

## Scope and files

Extract only the `knowledge_` count and primary records from `Realm::serialize` / `Realm::deserialize` into stateless inline helpers.

- Add `core/include/polyec/knowledge_section_codec.hpp` with `polyec::detail::write_knowledge_section(std::ostream&, const std::vector<Knowledge>&)` and `read_knowledge_section(std::istream&, std::vector<Knowledge>&)`.
- In `core/src/realm.cpp`, add the include and replace only the inline Knowledge writer/reader records. Keep the index rebuild immediately after parsing in Realm.
- In `core/tests/test_realm.cpp`, add focused codec tests modeled on the accepted Fact/Project blocks.
- No CMake, schema, gameplay, court, bindings, UI, native, Godot, Python, model, or archive changes.

## Token grammar and errors

Writer order remains:

```text
<count> (<observer> <fact> <arrives>)*
```

Count cap remains 1,000,000. Malformed/excessive count and incomplete records throw `std::runtime_error("Save count limit")`. The current inline reader detects a truncated row when it tries to read the following Proposal count; the helper may detect it at the Knowledge boundary with the same error. Add no field validation. Read to a local vector and publish only when all three fields per record parse.

Tests check malformed/excessive count, truncated record, output preservation after error, canonical writer bytes `1 7 8 9 `, valid field parsing, and that duplicate `(observer, fact)` rows remain in input order in the codec vector. Existing Realm save/load fingerprint and continuation tests remain the end-to-end oracle.

## Derived index behavior and ownership

The existing Realm code after parsing rebuilds `knowledge_index_` from every Knowledge row and stores the minimum `arrives` for duplicate `(observer, fact)` pairs. Preserve this loop's behavior and placement after `read_knowledge_section`; do not put the index in the stateless codec. `knowledge_index_` is private and there is no public index accessor, so avoid serializer-token surgery solely to expose it in a new test. Normal Realm save/load and query tests plus direct diff review cover the unchanged rebuild loop.

Realm remains owner of `knowledge_`, stages all deserialization, checks the full world and commits at the existing final move. Failed load leaves the live Realm unchanged.

## Windows gates

Reuse only `build/m5-core`; no configure or new generator. Run Release full build/CTest, Debug full build/targeted CTest, and `git diff --check` as listed in [R4 Loan codec plan](R4-loan-record-codec-plan.md).

## Deferred

Proposal/Investigation records, court extension changes, schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
