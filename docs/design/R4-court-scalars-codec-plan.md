# R4 court scalars codec — plan-only review

Review date: 2026-10-20. This is a **plan-only** behavior-preserving R4 save-section extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9. No source, build, test or implementation change is included in this document.

## 1. Candidate

Extract only the **leading court scalar block** from `Realm::write_court` / `Realm::read_court`:

- `royal_reference_` (`int64_t`)
- `credit_pool_` (`int64_t`)
- `restricted_fund_` (`int64_t`)
- `mediation_ready_` (`int`)
- `bonus_offices_` (`int`)
- `pact_` (`bool`)
- `reference_scenario_` (`bool`)

These seven scalars are the first court block in `write_court`, written before referrals, investigations and all court extensions. This slice completes that leading part of the court save/read path without changing bytes, schema, validation order or gameplay. Referrals (`referrals_`) and every later court block remain untouched and are **not** part of this codec.

Candidate: court scalars section. It is bounded because it is exactly seven fixed Realm-owned scalar values with **no count prefix**, already owned by `Realm`, and no per-record loop. No validation currently exists for these fields and none is added.

## 2. Scope and files

- Add `core/include/polyec/court_scalars_codec.hpp` with:
  - `polyec::detail::write_court_scalars(std::ostream&, int64_t royal_reference, int64_t credit_pool, int64_t restricted_fund, int mediation_ready, int bonus_offices, bool pact, bool reference_scenario)`
  - `polyec::detail::read_court_scalars(std::istream&, int64_t& royal_reference, int64_t& credit_pool, int64_t& restricted_fund, int& mediation_ready, int& bonus_offices, bool& pact, bool& reference_scenario)`
- In `core/src/realm_court.cpp`, add the include and replace only the leading seven-scalar write/read in `write_court` / `read_court`.
- In `core/tests/test_realm_court.cpp`, add focused codec checks; keep existing court roundtrip and augment tests.
- No CMake, schema, gameplay, bindings, UI, native, Godot, Python, model or archive changes.
- Do not modify `core/src/realm.cpp`, `core/include/polyec/realm.hpp`, or any other codec header.

## 3. Token grammar and exact byte order

The codec writes and reads the seven court scalars in their current order, with **no count prefix**:

```text
<royal_reference> <credit_pool> <restricted_fund> <mediation_ready> <bonus_offices> <pact> <reference_scenario>
```

Writer order remains exactly:

```cpp
out << royal_reference << ' ' << credit_pool << ' ' << restricted_fund << ' '
    << mediation_ready << ' ' << bonus_offices << ' ' << pact << ' '
    << reference_scenario << ' ';
```

The section is the **first** block of `write_court`, immediately before `referrals_` (the referrals count and IDs), and in `read_court` at the same position. No field is moved, renamed, added or removed. The values are Realm-owned scalars, not a vector; there is no count and no per-record loop. `bool` values use the default stream formatting (`0`/`1`), matching the legacy `operator>>` for `bool`.

## 4. Legacy failure contract

- The section has **no count prefix**.
- Any non-numeric or truncated scalar causes the read to fail and is surfaced by `Realm::deserialize` as `std::runtime_error("Save court fields")`. The codec must throw exactly `std::runtime_error("Save court fields")` for every parse failure, including a stream that stops after any of the seven fields.
- The codec must **not** throw `Save count limit` for this section.
- **Legacy precision note:** the current `read_court` reads these seven scalars with a single unchecked extraction:
  ```cpp
  s >> royal_reference_ >> credit_pool_ >> restricted_fund_ >> mediation_ready_
    >> bonus_offices_ >> pact_ >> reference_scenario_;
  ```
  It does **not** test `s` immediately after this line. A truncated or non-numeric scalar therefore leaves the stream in a failed state but execution continues into the referrals count read (`s >> n`), which then fails, and/or the final `return bool(s);`. The observable result is still `read_court == false`, which `deserialize` converts to `Save court fields`. This codec makes the failure immediate at the scalar extraction point, but the caller-visible error string and the atomic load behavior are identical. Tests and reports must not claim that the legacy code rejected these scalars at the exact token where parsing stopped.
- **Staged parse:** the codec reads all seven values into local temporaries first and assigns them to the output references only after every scalar has parsed. A parse failure throws before assignment, so the caller's variables (Realm members when called from `read_court`) remain unchanged. This is the Realm assignment point: on success the seven Realm members are updated as a group; on failure none of them change. The assignment point occurs before referrals are read.
- **No range validation.** No range or domain checks are added for `royal_reference_`, `credit_pool_`, `restricted_fund_`, `mediation_ready_`, `bonus_offices_`, `pact_` or `reference_scenario_`; their legacy behavior is preserved exactly. There is no extra `read_court` validation after this codec read, matching the current code.
- **Truncation is not field-boundary detection.** Because this section is unframed (no count prefix, no per-field delimiter beyond whitespace), the helper cannot tell which bytes belong to this section versus the following referrals section. On truncated input the `>>` extraction may consume tokens that physically belong to the referrals block (or beyond) and still succeed; the helper only detects extraction failure (EOF or non-numeric token). This matches the legacy unframed extraction behavior. The plan and tests must not claim field-boundary or section-boundary detection, and must not add framing to create it.

## 5. Ownership and atomicity

- `Realm` remains the sole owner of `royal_reference_`, `credit_pool_`, `restricted_fund_`, `mediation_ready_`, `bonus_offices_`, `pact_` and `reference_scenario_`.
- The codec is stateless and touches only these seven values. It does not own, transfer or replace Realm state.
- `read_court` remains a staging operation inside `deserialize`: a failed codec read leaves `*this` unchanged because `deserialize` discards the temporary `Realm w`. In addition, the codec's own staging leaves the seven members unchanged on a parse failure.
- No transaction ID, settlement ID or gameplay state is added.

## 6. Tests

Add focused checks in `core/tests/test_realm_court.cpp`, modeled on the accepted Province/Loan blocks but adapted to scalar out-parameters (use local variables instead of a vector):

- Truncated scalar stream (fewer than seven tokens) throws `Save court fields` and preserves all seven output values. Suggested cases: one token, two tokens, ..., six tokens.
- Bad input (non-numeric first token) throws `Save court fields`.
- Output preservation on failure for representative values, including all seven scalars.
- Exact writer bytes for canonical scalars. Suggested canonical values: `royal_reference=100`, `credit_pool=20`, `restricted_fund=30`, `mediation_ready=456`, `bonus_offices=2`, `pact=true`, `reference_scenario=false`; expected writer string: `100 20 30 456 2 1 0 `.
- Valid round trip preserves all seven scalars.
- Existing full Realm court roundtrip and augment tests remain the end-to-end oracle.

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

- Referrals extraction (`referrals_` count + IDs).
- Other remaining court extension refactors.
- Schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
- No commit or push.
