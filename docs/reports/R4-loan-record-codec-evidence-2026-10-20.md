# R4 Loan primary record codec — evidence (2026-10-20)

Plan: [R4 Loan primary record codec](../design/R4-loan-record-codec-plan.md). This evidence applies to the dirty working tree based on `HEAD dff2781`; it is not a clean-checkout result.

## Delivered

- OpenCode added `core/include/polyec/loan_section_codec.hpp` and replaced only the primary Loan count/record serialization and deserialization call sites in `core/src/realm.cpp`.
- The writer preserves count followed by `id`, `debtor`, `creditor`, `due`, `principal`, `interest`, `paid`, and `defaulted`, including the trailing space. Count limit remains 1,000,000. No new field range checks were added.
- The reader stages a vector, throws `Save count limit` for malformed/excessive count or incomplete primary record, and publishes only after all records parse. `royal_pool` and `cause` remain in the existing court extension section after the primary Loan records.
- Realm continues to own `loans_`, validate the complete staged world, and replace live state only after deserialization succeeds.
- `core/tests/test_realm.cpp` checks the exact writer string, valid field parsing, malformed and excessive counts, truncated-record error, unchanged output Loan including non-default court extensions on failure, and existing Realm save/load fingerprint behavior.
- No schema, gameplay, binding, API, CMake or command behavior changed.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure or new build directory was used.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 16.52 s.
- Debug build: exit 0.
- Targeted Debug CTest (`realm_tests|court_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 64.78 s.
- After tightening the final test oracle, rebuilt Release target `polyec_realm_tests` and ran `build/m5-core/core/Release/polyec_realm_tests.exe`: exit 0, **25,599 checks passed**.
- `git diff --check`: exit 0; Git emitted only the existing CRLF-to-LF notice for dirty `core/tests/test_realm.cpp`.

## Limits

No Python/native/Godot/Linux, training or performance gate was run. No commit or push. This is a serialization responsibility extraction, not a gameplay or ownership change. R4/M5 remain open.
