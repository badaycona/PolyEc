# R4 Person court-extension codec — plan-only review

Review date: 2026-10-20. This is a **plan-only** behavior-preserving R4 save-section extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9. No source, build, test or implementation change is included in this document.

## 1. Candidate

Extract only the **Person court-extension fields** from `Realm::write_court` / `Realm::read_court` into a stateless inline codec. The seven primary/extension fields listed below are written per Person in the existing court section; this slice completes that part of the Person save/read path without changing bytes, schema, validation order or gameplay.

Candidate: `Person` court extension section. It is bounded because it is one record type, already sized by the primary Person section (`people_`), and has **no count prefix** of its own.

## 2. Scope and files

- Add `core/include/polyec/person_court_extension_codec.hpp` with:
  - `polyec::detail::write_person_court_extension_section(std::ostream&, const std::vector<Person>&)`
  - `polyec::detail::read_person_court_extension_section(std::istream&, std::vector<Person>&)`
- In `core/src/realm_court.cpp`, add the include and replace only the per-Person court-extension loop in `write_court` / `read_court`.
- In `core/tests/test_realm_court.cpp`, add focused codec checks; keep existing court roundtrip and Person tests.
- No CMake, schema, gameplay, bindings, UI, native, Godot, Python, model or archive changes.
- Do not modify `core/src/realm.cpp` or `core/include/polyec/person_section_codec.hpp`.

## 3. Token grammar and exact byte order

The codec writes and reads the Person court extension fields in their current order, with **no count prefix**:

```text
(<candidate> <office_appointment.office> <special_advisor>
 <office_appointment.hired_by> <office_appointment.salary_due>
 <office_appointment.office_notice> <last_decision_day>)*
```

Writer order remains exactly:

```cpp
for (const auto& p : people) {
    out << p.candidate << ' ' << p.office_appointment.office << ' '
        << p.special_advisor << ' ' << p.office_appointment.hired_by << ' '
        << p.office_appointment.salary_due << ' '
        << p.office_appointment.office_notice << ' '
        << p.last_decision_day << ' ';
}
```

The section appears in `write_court` **immediately after the Investigation section** and **immediately before the Province court counters** (`tax_receipts` / `food_capacity` / `teacher_skill_sum`), and in `read_court` at the same position. No field is moved, renamed, added or removed. The number of records is fixed by `people_`, which is already sized and validated by the primary Person section.

## 4. Legacy failure contract

- The section has **no count prefix**.
- Any non-numeric or truncated extension field causes the read to fail and is surfaced by `Realm::deserialize` as `std::runtime_error("Save court fields")`. The codec must throw exactly `std::runtime_error("Save court fields")` for every parse failure, including a record that stops midway through the seven fields.
- The codec must **not** throw `Save count limit` for this section.
- The codec stages a **local copy** of the incoming `people` vector, reads all seven court fields into that copy, and publishes it only after every field in every record has parsed. A failed read leaves the caller's vector unchanged.
- **No primary Person field edits.** Primary Person fields (`id`, `household_membership.household_id`, `decisions`, `sampling`, `region`, `age_days`, `health`, `skill`, `work_assignment.job`, `busy_until`, `destination`, `goal`, `school_days`, `social`, `alive`, `family`, `cash`, `tax_remainder`, `organization`, `work_assignment.job_kind`, `taxing_authority`, `sovereign`, `nutrition`, `stats`, `inventory`) are not read or written by this codec and must remain untouched on both success and failure. The codec touches only the seven court fields listed above.
- **Validation stays in Realm.** The existing `hired_by > people_.size()` check is preserved as a **domain validation performed by `Realm::read_court` immediately after the codec read**, before the Province court counters are read. The codec itself performs no range validation. On violation, `read_court` returns `false`, which `deserialize` converts to `Save court fields`; validation occurs before `deserialize` commits the staged world, so a failed validation leaves the live Realm unchanged.
- No additional validation is added for `candidate`, `office`, `special_advisor`, `salary_due`, `office_notice` or `last_decision_day`; their legacy behavior is preserved exactly.

## 5. Ownership and atomicity

- `Realm` remains the sole owner of `people_`.
- The codec is stateless and touches only the seven court-extension values listed above.
- `Realm` performs its existing `hired_by` validation immediately after the codec read and before reading the Province court counters.
- `read_court` remains a staging operation inside `deserialize`: a failed codec read or validation leaves `*this` unchanged because `deserialize` discards the temporary `Realm w`.
- No transaction ID, settlement ID or gameplay state is added.

## 6. Tests

Add focused checks in `core/tests/test_realm_court.cpp`, modeled on the accepted Investigation/Proposal/Loan/Project blocks:

- Truncated extension record (some of the seven fields present, then EOF) throws `Save court fields` and preserves the output Person, including all primary and court fields.
- Bad input (non-numeric) throws `Save court fields`.
- Output preservation on failure for an existing Person.
- Exact writer bytes for a canonical Person. Suggested canonical court values:
  - `candidate=true, office=true, special_advisor=false, hired_by=2, salary_due=30, office_notice=5, last_decision_day=7`
  - expected writer string: `1 1 0 2 30 5 7 `
- Valid round trip preserves every court field and input row order.
- Existing full Realm court roundtrip, appointment/resignation/salary tests and full save/load fingerprint tests remain the end-to-end oracle.

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

- Province counter and other remaining court extension refactors.
- Court scalars and referrals extraction.
- Schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
- No commit or push.
