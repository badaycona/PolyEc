# R4 Project primary record codec — approved plan

Review date: 2026-10-20. This is a behavior-preserving R4 save-record extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9.

## Scope and files

Extract only the `projects_` count and primary Project fields from `Realm::serialize` / `Realm::deserialize` into stateless inline helpers.

- Add `core/include/polyec/project_section_codec.hpp` with `polyec::detail::write_project_section(std::ostream&, const std::vector<Project>&)` and `read_project_section(std::istream&, std::vector<Project>&)`.
- In `core/src/realm.cpp`, add the include and replace only the inline primary Project writer/reader loops.
- In `core/tests/test_realm.cpp`, add focused codec tests modeled on the accepted R4 Loan codec block.
- Do not modify `core/src/realm_court.cpp`, the Project definition, CMake, bindings, UI, gameplay, action, observation, reward or model.

## Token grammar and errors

Writer order remains:

```text
<count> (<id> <owner> <region> <due> <kind> <budget> <completed>)*
```

Count cap remains 1,000,000. Malformed/excessive count and a truncated primary record produce `std::runtime_error("Save count limit")`. The current inline reader detects a truncated record when it next reads the Facts count; the extracted reader may reject at the Project boundary with the same error. Do not add field validation. Read into a temporary vector and publish only after all seven-field records parse.

Tests check malformed and excessive count, truncated record, unchanged output on failure, the exact writer string `1 7 8 9 10 11 12 1 `, and valid parsed fields. Existing full Realm roundtrip/fingerprint/continuation tests and court tests remain the end-to-end oracle.

## Court extension, ownership and atomicity

`engineer`, `stalled`, and `cause` are not part of this codec. The existing `Realm::write_court` / `Realm::read_court` loops remain in place and order after Loan extensions and before Proposal extensions. Realm remains sole owner of `projects_`, stages the load, validates the full world, and commits only at the existing final move. Failed deserialization leaves the live Realm unchanged.

## Windows gates

Reuse only `build/m5-core`; do not configure a build or generator. Run the exact Release full build/CTest and Debug full build/targeted CTest commands in [R4 Loan codec plan](R4-loan-record-codec-plan.md), then `git diff --check`.

## Deferred

Project gameplay/lifecycle, court extension changes, Fact/Knowledge/Proposal/Investigation records, schema, API/bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
