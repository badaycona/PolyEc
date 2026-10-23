# R4 Proposal court extension codec — plan-only review

Review date: 2026-10-20. This is a **plan-only** behavior-preserving R4 save-section extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9. No source, build, test or implementation change is included in this document.

## 1. Candidate

Extract only the Proposal **court extension fields** from `Realm::write_court` / `Realm::read_court` into a stateless inline codec. The six primary Proposal fields are already extracted by `R4-proposal-record-codec`; this slice completes the Proposal save/read path without changing bytes, schema, validation order or gameplay.

Candidate: `Proposal` court extension section. It is bounded because it is one record type, already sized by the primary Proposal section, and has no count prefix of its own.

## 2. Scope and files

- Add `core/include/polyec/proposal_court_extension_codec.hpp` with:
  - `polyec::detail::write_proposal_court_extension_section(std::ostream&, const std::vector<Proposal>&)`
  - `polyec::detail::read_proposal_court_extension_section(std::istream&, std::vector<Proposal>&)`
- In `core/src/realm_court.cpp`, add the include and replace only the Proposal court-extension loop in `write_court` / `read_court`.
- In `core/tests/test_realm_court.cpp`, add focused codec checks; keep existing court roundtrip and Proposal tests.
- No CMake, schema, gameplay, bindings, UI, native, Godot, Python, model or archive changes.
- Do not modify `core/src/realm.cpp` or `core/include/polyec/proposal_section_codec.hpp`.

## 3. Token grammar and exact byte order

The codec writes and reads the Proposal court extension fields in their current order, with no count prefix:

```text
(<kind> <ready> <escrow> <public_source> <mediation> <source_region> <cause>
 <parties[0]> <parties[1]> <parties[2]>
 <regions[0]> <regions[1]> <regions[2]>
 <pledged[0]> <pledged[1]> <pledged[2]>)*
```

Writer order remains exactly:

```cpp
for (const auto& o : proposals) {
    out << o.kind << ' ' << o.ready << ' ' << o.escrow << ' '
        << o.public_source << ' ' << o.mediation << ' '
        << o.source_region << ' ' << o.cause << ' ';
    for (auto id : o.parties) out << id << ' ';
    for (auto r : o.regions) out << r << ' ';
    for (bool b : o.pledged) out << b << ' ';
}
```

The section appears in `write_court` after court scalars, referrals, Investigation, Person office fields, Province court counters, Loan court extensions and Project court extensions, and immediately before the final `return` of `read_court`. It is the last court block. No field is moved, renamed, added or removed.

## 4. Legacy failure contract

- The section has no count prefix. The number of records is fixed by `proposals_`, which is already sized and validated by the primary Proposal section.
- Any non-numeric or truncated extension field causes the read to fail and is surfaced by `Realm::deserialize` as `std::runtime_error("Save court fields")`. The codec must throw exactly `std::runtime_error("Save court fields")` for every parse failure, including a record that stops midway through `parties`, `regions` or `pledged`.
- The codec must not throw `Save count limit` for this section.
- The codec stages a local copy of the incoming `proposals` vector, reads all extension fields into that copy, and publishes it only after every field in every record has parsed. A failed read leaves the caller's vector unchanged.
- Primary Proposal fields (`id`, `from`, `to`, `amount`, `due`, `resolved`) are not read or written by this codec and must remain untouched on both success and failure.
- Range validation stays in `Realm::read_court` after the codec read, preserving the current checks:
  - `o.kind >= 0 && o.kind <= 2`
  - `o.source_region >= 0 && o.source_region <= regions_`
  - each `r` in `o.regions`: `r >= 0 && r <= regions_`
- Validation failure returns `false` from `read_court`, which `deserialize` converts to `Save court fields`. Validation occurs before `deserialize` commits the staged world, so failed validation leaves the live Realm unchanged.
- No additional validation is added for `parties`, `pledged`, `ready`, `escrow`, `public_source`, `mediation` or `cause`; their legacy behavior is preserved exactly.

## 5. Ownership and atomicity

- `Realm` remains the sole owner of `proposals_`.
- The codec is stateless and touches only the nine court extension values listed above.
- `Realm` performs its existing validation after codec read and before the final `return bool(s)`.
- `read_court` remains a staging operation inside `deserialize`: a failed codec read or validation leaves `*this` unchanged because `deserialize` discards the temporary `Realm w`.
- No transaction ID, settlement ID or gameplay state is added.

## 6. Tests

Add focused checks in `core/tests/test_realm_court.cpp`, modeled on the accepted Investigation block:

- Truncated extension record throws `Save court fields` and preserves the output Proposal, including all primary and extension fields.
- Output preservation on failure for an existing multi-field Proposal.
- Exact writer bytes for a canonical Proposal. Suggested canonical extension values:
  - `kind=1, ready=12, escrow=13, public_source=true, mediation=false, source_region=14, cause=15`
  - `parties={16,17,18}`, `regions={1,2,3}`, `pledged={false,true,false}`
  - expected writer string: `1 12 13 1 0 14 15 16 17 18 1 2 3 0 1 0 `
- Valid round trip preserves every extension field and input row order.
- Existing full Realm court roundtrip, Proposal generation/query tests and full save/load fingerprint tests remain the end-to-end oracle.

## 7. Exact gates

Reuse only `build/m5-core`; do not configure or create a new build directory. Run:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Release --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Release --output-on-failure
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Debug --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Debug -R 'court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_replay' --output-on-failure
git diff --check
```

Record build exit codes, CTest pass counts, targeted test counts, and `git diff --check` result in the evidence report. Do not claim direct executable verification beyond the CTest gates above.

## 8. Deferred

- Person office, Province counter, Loan, Project and other remaining court extension refactors.
- Court scalars and referrals extraction.
- Schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
- No commit or push.