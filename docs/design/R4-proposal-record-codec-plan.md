# R4 Proposal primary record codec — approved plan

Review date: 2026-10-20. This is a behavior-preserving R4 save-record extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9.

## Scope and files

Extract only the `proposals_` count and six primary Proposal fields from `Realm::serialize` / `Realm::deserialize` into stateless inline helpers.

- Add `core/include/polyec/proposal_section_codec.hpp` with `polyec::detail::write_proposal_section(std::ostream&, const std::vector<Proposal>&)` and `read_proposal_section(std::istream&, std::vector<Proposal>&)`.
- In `core/src/realm.cpp`, add the include and replace only the Proposal primary writer/reader loops.
- In `core/tests/test_realm.cpp`, add focused codec tests modeled on the accepted Fact/Knowledge blocks.
- Do not modify `core/src/realm_court.cpp`, CMake, schema, bindings, UI, native, Godot, Python, model, or gameplay.

## Token grammar and failure behavior

Writer order remains:

```text
<count> (<id> <from> <to> <amount> <due> <resolved>)*
```

Count cap remains 1,000,000. Malformed/excessive count throws `std::runtime_error("Save count limit")`. The current primary reader does not check each record; an incomplete primary record causes `read_court` to fail and the caller returns `Save court fields`. The codec must preserve this observable error: it may detect the incomplete record at the Proposal boundary but throws `std::runtime_error("Save court fields")`. No primary field range validation is added. Stage locally and publish only after all six fields in each record parse.

Focused tests check excessive/bad count, truncated primary record with the legacy `Save court fields` error, unchanged output on failure, canonical writer bytes `1 7 8 9 10 11 0 `, valid field parsing and input row order. Existing Realm save/load fingerprint and continuation, plus court roundtrip/Proposal tests, remain the end-to-end oracle.

## Court extension order, ownership and atomicity

Proposal court extensions remain in `Realm::write_court` / `Realm::read_court` at their current later position: after court scalars, referrals, investigations, Person extensions, Province extensions, Loan extensions, and Project extensions. Each Proposal extension remains `kind`, `ready`, `escrow`, `public_source`, `mediation`, `source_region`, `cause`, `parties[3]`, `regions[3]`, `pledged[3]`. Do not move or extract these fields.

The codec is stateless and touches only six primary fields. Realm remains owner of `proposals_`, reads extensions into the staged records, validates the full world and commits at the existing final move. Failed load leaves live Realm unchanged.

## Windows gates

Reuse only `build/m5-core`; no configure or new generator. Run Release full build/CTest, Debug full build/targeted CTest and `git diff --check` as listed in [R4 Loan codec plan](R4-loan-record-codec-plan.md).

## Deferred

Proposal extension refactors, Investigation record, schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
