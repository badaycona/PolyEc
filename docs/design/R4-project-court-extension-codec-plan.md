# R4 Project court extension codec — plan-only review

Review date: 2026-10-20. This is a **plan-only** behavior-preserving R4 save-section extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9. No source, build, test or implementation change is included in this document.

## 1. Candidate

Extract only the Project **court extension fields** (`engineer`, `stalled`, `cause`) from `Realm::write_court` / `Realm::read_court` into a stateless inline codec. The seven primary Project fields are already extracted by `R4-project-record-codec`; this slice completes the Project save/read path without changing bytes, schema, validation order or gameplay.

Candidate: `Project` court extension section. It is bounded because it is one record type, already sized by the primary Project section, and has no count prefix of its own.

## 2. Scope and files

- Add `core/include/polyec/project_court_extension_codec.hpp` with:
  - `polyec::detail::write_project_court_extension_section(std::ostream&, const std::vector<Project>&)`
  - `polyec::detail::read_project_court_extension_section(std::istream&, std::vector<Project>&)`
- In `core/src/realm_court.cpp`, add the include and replace only the Project court-extension loop in `write_court` / `read_court`.
- In `core/tests/test_realm_court.cpp`, add focused codec checks and include the new header; keep existing court roundtrip and Project tests.
- No CMake, schema, gameplay, bindings, UI, native, Godot, Python, model or archive changes.
- Do not modify `core/src/realm.cpp`, `core/include/polyec/project_section_codec.hpp`, or any Project gameplay code.

## 3. Token grammar and exact byte order/location

The codec writes and reads the Project court extension fields in their current order, with no count prefix:

```text
(<engineer> <stalled> <cause>)*
```

Writer order remains exactly:

```cpp
for (const auto& j : projects) {
    out << j.engineer << ' ' << j.stalled << ' ' << j.cause << ' ';
}
```

The section appears in `write_court` after the Loan court extension section and immediately before the Proposal court extension section, and in `read_court` at the same position:

```cpp
detail::write_loan_court_extension_section(s, loans_);
for (auto& j : projects_) s << j.engineer << ' ' << j.stalled << ' ' << j.cause << ' ';
detail::write_proposal_court_extension_section(s, proposals_);
```

No field is moved, renamed, added or removed. The number of records is fixed by `projects_`, which is already sized and validated by the primary Project section. Primary Project fields (`id`, `owner`, `region`, `due`, `kind`, `budget`, `completed`) are not read or written by this codec and must remain untouched on both success and failure.

## 4. Legacy failure contract

- The section has no count prefix.
- Any non-numeric or truncated extension field causes the read to fail and is surfaced by `Realm::deserialize` as `std::runtime_error("Save court fields")`. The codec must throw exactly `std::runtime_error("Save court fields")` for every parse failure, including a record that stops after `engineer` or after `stalled` before `cause`.
- The codec must not throw `Save count limit` for this section.
- The codec stages a local copy of the incoming `projects` vector, reads all extension fields into that copy, and publishes it only after every field in every record has parsed. A failed read leaves the caller's vector unchanged.
- No range validation is added for `engineer`, `stalled` or `cause`; their legacy behavior is preserved exactly. There is no extra `read_court` validation after this codec read, matching the current code.

## 5. Ownership and atomicity

- `Realm` remains the sole owner of `projects_`.
- The codec is stateless and touches only the three court extension values listed above.
- `read_court` remains a staging operation inside `deserialize`: a failed codec read leaves `*this` unchanged because `deserialize` discards the temporary `Realm w`.
- No transaction ID, settlement ID or gameplay state is added.

## 6. Validation placement

None. The current `read_court` performs no validation immediately after the Project court extension loop; the only later validation is for Proposal extension fields. This plan preserves that: no validation is added before or after the codec read. Realm retains full-world validation and atomic commit in `deserialize`.

## 7. Tests

Add focused checks in `core/tests/test_realm_court.cpp`, modeled on the accepted Loan/Proposal blocks:

- Truncated extension record (`engineer` present, `stalled` or `cause` missing) throws `Save court fields` and preserves the output Project, including all primary and extension fields.
- Bad input throws `Save court fields`.
- Output preservation on failure for an existing Project.
- Exact writer bytes for a canonical Project. Suggested canonical extension values:
  - `engineer=true, stalled=false, cause=15`
  - expected writer string: `1 0 15 `
- Valid round trip preserves every extension field and input row order.
- Existing full Realm court roundtrip and Project generation/query tests remain the end-to-end oracle.

## 8. Exact gates

Reuse only `build/m5-core`; do not configure or create a new build directory. Run:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Release --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Release --output-on-failure
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Debug --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Debug -R 'court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_replay' --output-on-failure
git diff --check
```

Record build exit codes, CTest pass counts, targeted test counts, and `git diff --check` result in the evidence report. Do not claim direct executable verification beyond the CTest gates above.

## 9. Deferred

- Person office, Province counter and other remaining court extension refactors.
- Court scalars and referrals extraction.
- Schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
- No commit or push.
