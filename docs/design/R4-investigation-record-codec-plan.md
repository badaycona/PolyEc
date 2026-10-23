# R4 Investigation record codec — approved plan

Review date: 2026-10-20. This is a behavior-preserving court-save extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9.

## Scope and files

Extract only the Investigation count and six-field records from `Realm::write_court` / `Realm::read_court` into stateless inline helpers.

- Add `core/include/polyec/investigation_section_codec.hpp` with `polyec::detail::write_investigation_section(std::ostream&, const std::vector<Investigation>&)` and `read_investigation_section(std::istream&, std::vector<Investigation>&)`.
- In `core/src/realm_court.cpp`, add the include and replace only the Investigation count/record loops.
- In `core/tests/test_realm_court.cpp`, add focused codec checks; keep existing court roundtrip and Investigation tests.
- No CMake, schema, gameplay, bindings, UI, native, Godot, Python, model or archive changes.

## Grammar and exact failure behavior

Writer order remains:

```text
<count> (<id> <owner> <cause> <region> <due> <completed>)*
```

Count cap remains 1,000,000. In the current `read_court`, malformed/non-numeric/excessive Investigation count makes the method return false; truncated primary records also make it return false at the final `return bool(s)`. Both become `std::runtime_error("Save court fields")` in `Realm::deserialize`. Preserve this exact caller-visible error for all Investigation count and record parse failures; do not use `Save count limit` here. Stage a local vector and publish only after all six fields in every record parse.

Codec performs no owner/region checks. Keep the existing Realm validation after codec read: `owner ∈ [1, people_.size()]` and `region ∈ [0, regions_]`, returning false from `read_court` on violation. Existing code verifies these checks before reading later extensions. Keep that ordering.

Tests check excessive and bad counts, truncated record, output preservation, exact writer bytes `1 7 8 9 10 11 0 `, valid field roundtrip and input order. Full Realm court roundtrip and current Investigation generation/query tests remain the end-to-end oracle. Direct review confirms the existing Realm reference validation stays immediately after the codec read; no serializer surgery is needed to test a private helper.

## Ownership and atomicity

Codec has no state and reads/writes only Investigation values. Realm owns the vector, performs its existing owner/region validation, reads remaining court extensions and commits the staged world only at the existing final move. Failed deserialization leaves the live Realm unchanged. Investigation remains at its current byte position inside the court section; Proposal and other extensions remain untouched.

## Windows gates

Reuse only `build/m5-core`; do not configure or change generator. Run:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Release --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Release --output-on-failure
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Debug --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Debug -R 'court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_replay' --output-on-failure
git diff --check
```

## Deferred

Proposal and other court extension refactors, schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
