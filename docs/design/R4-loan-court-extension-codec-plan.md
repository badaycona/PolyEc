# R4 Loan court extension codec — plan-only review

Review date: 2026-10-20. This is a **plan-only** behavior-preserving R4 save-section extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9. No source, build, test or implementation change is included in this document.

## 1. Candidate

Extract only the Loan **court extension fields** (`royal_pool`, `cause`) from `Realm::write_court` / `Realm::read_court` into a stateless inline codec. The eight primary Loan fields are already extracted by `R4-loan-record-codec`; this slice completes the Loan save/read path without changing bytes, schema, validation order or gameplay.

Candidate: `Loan` court extension section. It is bounded because it is one record type, already sized by the primary Loan section, and has no count prefix of its own.

## 2. Scope and files

- Add `core/include/polyec/loan_court_extension_codec.hpp` with:
  - `polyec::detail::write_loan_court_extension_section(std::ostream&, const std::vector<Loan>&)`
  - `polyec::detail::read_loan_court_extension_section(std::istream&, std::vector<Loan>&)`
- In `core/src/realm_court.cpp`, add the include and replace only the Loan court-extension loop in `write_court` / `read_court`.
- In `core/tests/test_realm_court.cpp`, add focused codec checks; keep existing court roundtrip and Loan tests.
- No CMake, schema, gameplay, bindings, UI, native, Godot, Python, model or archive changes.
- Do not modify `core/src/realm.cpp` or `core/include/polyec/loan_section_codec.hpp`.

## 3. Token grammar and exact byte order

The codec writes and reads the Loan court extension fields in their current order, with no count prefix:

```text
(<royal_pool> <cause>)*
```

Writer order remains exactly:

```cpp
for (const auto& l : loans) {
    out << l.royal_pool << ' ' << l.cause << ' ';
}
```

The section appears in `write_court` after Province court counters and immediately before Project court extensions, and in `read_court` at the same position. No field is moved, renamed, added or removed. The number of records is fixed by `loans_`, which is already sized and validated by the primary Loan section.

## 4. Legacy failure contract

- The section has no count prefix.
- Any non-numeric or truncated extension field causes the read to fail and is surfaced by `Realm::deserialize` as `std::runtime_error("Save court fields")`. The codec must throw exactly `std::runtime_error("Save court fields")` for every parse failure, including a record that stops after `royal_pool` before `cause`.
- The codec must not throw `Save count limit` for this section.
- The codec stages a local copy of the incoming `loans` vector, reads all extension fields into that copy, and publishes it only after every field in every record has parsed. A failed read leaves the caller's vector unchanged.
- Primary Loan fields (`id`, `debtor`, `creditor`, `due`, `principal`, `interest`, `paid`, `defaulted`) are not read or written by this codec and must remain untouched on both success and failure.
- No range validation is added for `royal_pool` or `cause`; their legacy behavior is preserved exactly. There is no extra `read_court` validation after this codec read, matching the current code.

## 5. Ownership and atomicity

- `Realm` remains the sole owner of `loans_`.
- The codec is stateless and touches only the two court extension values listed above.
- `read_court` remains a staging operation inside `deserialize`: a failed codec read leaves `*this` unchanged because `deserialize` discards the temporary `Realm w`.
- No transaction ID, settlement ID or gameplay state is added.

## 6. Tests

Add focused checks in `core/tests/test_realm_court.cpp`, modeled on the accepted Proposal/Investigation blocks:

- Truncated extension record (`royal_pool` present, `cause` missing) throws `Save court fields` and preserves the output Loan, including all primary and extension fields.
- Bad input throws `Save court fields`.
- Output preservation on failure for an existing Loan.
- Exact writer bytes for a canonical Loan. Suggested canonical extension values:
  - `royal_pool=true, cause=15`
  - expected writer string: `1 15 `
- Valid round trip preserves every extension field and input row order.
- Existing full Realm court roundtrip and Loan generation/query tests remain the end-to-end oracle.

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

- Person office, Province counter, Project and other remaining court extension refactors.
- Court scalars and referrals extraction.
- Schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
- No commit or push.
filePath">
C:\prc\PolyEc\docs\design\R4-loan-court-extension-codec-plan.md