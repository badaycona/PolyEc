# R4 Loan primary record codec — approved plan

Review date: 2026-10-20. This is a behavior-preserving continuation of R4 save-section extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9.

## Scope

Extract only the serialized `loans_` count and primary Loan records from `Realm::serialize` / `Realm::deserialize` into stateless inline helpers. Preserve the exact token position/order, whitespace, values, count cap (`1,000,000`), and externally observed `Save count limit` for a truncated primary record. Realm remains the owner of loans, whole-world validation, staging and atomic load commit. `royal_pool` and `cause` remain in `Realm::write_court` / `Realm::read_court`.

## Files and interfaces

- Add `core/include/polyec/loan_section_codec.hpp` with:
  - `write_loan_section(std::ostream&, const std::vector<Loan>&)`
  - `read_loan_section(std::istream&, std::vector<Loan>&)`
- Update `core/src/realm.cpp` only to include the helper and replace the inline Loan primary-section writer/reader.
- Add focused codec malformed-count/truncated-record checks to `core/tests/test_realm.cpp`, following the existing test conventions. Keep all unrelated dirty tests and source.
- No CMake, public gameplay API, bindings, UI, action, observation, reward, model or gameplay changes.

## Token grammar and failure behavior

Writer order remains:

```text
<count> (<id> <debtor> <creditor> <due> <principal> <interest> <paid> <defaulted>)*
```

The `Loan` court extension fields follow later in `write_court` and are not part of this codec.

Reader uses the existing maximum of 1,000,000. A malformed/excessive count throws `std::runtime_error("Save count limit")`. Read records into a local vector and publish only after every eight-field primary record parses. The current inline reader does not check extraction per Loan; a truncated record makes the immediately following Projects count check throw the same `Save count limit`. The codec may detect this failure at the Loan boundary, preserving that deserialize error and leaving its output vector unchanged. Do not add field range checks.

Tests directly verify malformed count and truncated record return/fail with the legacy error and do not replace an existing output vector. Existing `test_realm` Borrow/Repay then serialize/deserialize/fingerprint equality remains the canonical valid-record/token-order oracle.

## Ownership and atomicity

- Codec is stateless and reads/writes only the primary Loan fields.
- `read_court` continues to read `royal_pool`/`cause` from the staged Loans after the primary records are available.
- Realm still validates the complete staged world and commits with the existing final move only after every section succeeds. Failed deserialize leaves the live Realm unchanged.

## Windows gates

Reuse only `build/m5-core`; do not configure a new build directory or generator. Run:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Release --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Release --output-on-failure
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Debug --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Debug -R 'realm_tests|court_tests|schedule_tests|manager_tests|realm_clock_replay' --output-on-failure
git diff --check
```

## Deferred

Project/Fact/Knowledge/Proposal/Investigation sections, court extensions, schema changes, gameplay, bindings, native/Godot/Python, model artifacts, performance work, training, commit and push.
