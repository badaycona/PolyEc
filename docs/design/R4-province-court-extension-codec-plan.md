# R4 Province court-extension codec — plan-only review

Review date: 2026-10-20. This is a **plan-only** behavior-preserving R4 save-section extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9. No source, build, test or implementation change is included in this document.

## 1. Candidate

Extract only the **Province court-extension fields** (`tax_receipts`, `food_capacity`, `teacher_skill_sum`) from `Realm::write_court` / `Realm::read_court` into a stateless inline codec. The seven primary Province fields plus seven stock and seven price values are already handled by `R4-province-record-codec`; this slice completes the Province court-counter part of the save/read path without changing bytes, schema, validation order or gameplay.

Candidate: `Province` court extension section. It is bounded because it is one record type, already sized by the primary Province section (`provinces_`, count cap 7), and has **no count prefix** of its own. No validation currently exists for these fields and none is added.

## 2. Scope and files

- Add `core/include/polyec/province_court_extension_codec.hpp` with:
  - `polyec::detail::write_province_court_extension_section(std::ostream&, const std::vector<Province>&)`
  - `polyec::detail::read_province_court_extension_section(std::istream&, std::vector<Province>&)`
- In `core/src/realm_court.cpp`, add the include and replace only the Province court-counter loop in `write_court` / `read_court`.
- In `core/tests/test_realm_court.cpp`, add focused codec checks; keep existing court roundtrip and Province tests.
- No CMake, schema, gameplay, bindings, UI, native, Godot, Python, model or archive changes.
- Do not modify `core/src/realm.cpp` or `core/include/polyec/province_section_codec.hpp`.

## 3. Token grammar and exact byte order

The codec writes and reads the Province court extension fields in their current order, with **no count prefix**:

```text
(<tax_receipts> <food_capacity> <teacher_skill_sum>)*
```

Writer order remains exactly:

```cpp
for (const auto& v : provinces) {
    out << v.tax_receipts << ' ' << v.food_capacity << ' ' << v.teacher_skill_sum << ' ';
}
```

The section appears in `write_court` **immediately after the Person court extension** and **immediately before the Loan court extension**, and in `read_court` at the same position. No field is moved, renamed, added or removed. The number of records is fixed by `provinces_`, which is already sized and validated by the primary Province section.

## 4. Legacy failure contract

- The section has **no count prefix**.
- Any non-numeric or truncated extension field causes the read to fail and is surfaced by `Realm::deserialize` as `std::runtime_error("Save court fields")`. The codec must throw exactly `std::runtime_error("Save court fields")` for every parse failure, including a record that stops after `tax_receipts` before `food_capacity`, or after `food_capacity` before `teacher_skill_sum`.
- The codec must **not** throw `Save count limit` for this section.
- The codec stages a **local copy** of the incoming `provinces` vector, reads all three court fields into that copy, and publishes it only after every field in every record has parsed. A failed read leaves the caller's vector unchanged.
- **No primary Province field edits.** Primary Province fields (`treasury`, `firm`, `stock`, `price`, `tax`, `school`, `school_used`, `jobs`, `employed`, `road`, `unrest`, `teachers`, `charter`, `residents`, `health_sum`, `sales_tax_remainder`) are not read or written by this codec and must remain untouched on both success and failure. The codec touches only the three court fields listed above. Because staging copies the whole struct, primary fields are preserved bit-for-bit on both success and failure.
- **No validation.** No range validation is added for `tax_receipts`, `food_capacity` or `teacher_skill_sum`; their legacy behavior is preserved exactly. There is no extra `read_court` validation after this codec read, matching the current code. `read_court` performs no domain checks on these fields.
- **Truncation is not field-boundary detection.** Because this section is unframed (no count prefix, no per-record delimiter beyond whitespace), the helper cannot tell which bytes belong to this section versus a later section. On truncated input the `>>` extraction may consume tokens that physically belong to the following Loan court extension section (or beyond) and still "succeed"; the helper only detects extraction failure (EOF or non-numeric token). This matches the legacy unframed `for (auto&v:provinces_) s >> v.tax_receipts >> v.food_capacity >> v.teacher_skill_sum;` behavior. The plan and tests must not claim field-boundary or section-boundary detection, and must not add framing to create it.

## 5. Ownership and atomicity

- `Realm` remains the sole owner of `provinces_`.
- The codec is stateless and touches only the three court-extension values listed above.
- `read_court` remains a staging operation inside `deserialize`: a failed codec read leaves `*this` unchanged because `deserialize` discards the temporary `Realm w`. Note that `read_court` currently reads these fields without a try/catch; this slice wraps the codec call in try/catch and returns false on throw, consistent with the other court-extension sections.
- No transaction ID, settlement ID or gameplay state is added.

## 6. Tests

Add focused checks in `core/tests/test_realm_court.cpp`, modeled on the accepted Loan/Project/Person blocks:

- Truncated extension record (`tax_receipts` present, `food_capacity`/`teacher_skill_sum` missing) throws `Save court fields` and preserves the output Province, including all primary and court fields.
- Truncated after two fields (`tax_receipts` and `food_capacity` present, `teacher_skill_sum` missing) throws `Save court fields`.
- Bad input (non-numeric) throws `Save court fields`.
- Output preservation on failure for an existing Province, including representative primary fields (`treasury`, `firm`, `stock[0..6]`, `price[0..6]`, `tax`, `school`, `school_used`, `jobs`, `employed`, `road`, `unrest`, `teachers`, `charter`, `residents`, `health_sum`, `sales_tax_remainder`) and all three court fields.
- Exact writer bytes for a canonical Province. Suggested canonical court values: `tax_receipts=20`, `food_capacity=30`, `teacher_skill_sum=50`; expected writer string: `20 30 50 `.
- Valid round trip preserves every court field and input row order.
- Existing full Realm court roundtrip and Province tests remain the end-to-end oracle.

## 7. Exact gates

Reuse only `build/m5-core`; do not configure or create a new build directory. Run:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Release --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Release --output-on-failure
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Debug --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Debug -R 'court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_replay' --output-on-failure
git diff --check
python scripts/check-repository-links.py
```

Record build exit codes, CTest pass counts, targeted test counts, and `git diff --check` result in the evidence report. Do not claim direct executable verification beyond the CTest gates above.

## 8. Deferred

- Court scalars (`royal_reference_`, `credit_pool_`, `restricted_fund_`, `mediation_ready_`, `bonus_offices_`, `pact_`, `reference_scenario_`) and referrals extraction.
- Other remaining court extension refactors.
- Schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
- No commit or push.
filePath">
C:\prc\PolyEc\docs\design\R4-province-court-extension-codec-plan.md