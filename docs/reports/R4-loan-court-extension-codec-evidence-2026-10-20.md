# R4 Loan court extension codec — evidence (2026-10-20)

Plan: [R4 Loan court extension codec](../design/R4-loan-court-extension-codec-plan.md). Results apply to the dirty working tree based on `HEAD dff2781`.

## Delivered

- Added `core/include/polyec/loan_court_extension_codec.hpp` with stateless `polyec::detail::write_loan_court_extension_section` and `read_loan_court_extension_section`.
- In `core/src/realm_court.cpp`, added the include and replaced only the Loan court-extension loops in `write_court`/`read_court`.
- Writer preserves the exact token order and trailing whitespace for `royal_pool`, `cause`. No count prefix.
- Reader stages a local copy of the incoming loans vector, reads both extension fields into that copy, and publishes only after every field in every record has parsed. Parse failures throw `std::runtime_error("Save court fields")`; no `Save count limit` for this section. No range validation is added.
- `Realm::read_court` converts the codec's expected `std::runtime_error` to `false`. Primary Loan fields remain untouched.
- Focused checks added in `core/tests/test_realm_court.cpp`: truncated extension (`royal_pool` present, `cause` missing), bad input, output preservation, canonical writer string `1 15 `, valid round trip of both extension fields, and input row order. Existing court/Realm save roundtrips remain the end-to-end oracle.
- No CMake, schema, gameplay, binding, native, Godot, Python, model or archive changes. `core/src/realm.cpp` and `loan_section_codec.hpp` were not modified.
- Fixed the existing broken DEVELOPMENT_HISTORY link from `R4-person-section-codec-plan.md` to the actual `R4-person-record-codec-plan.md`.
- Updated `docs/design/core-class-reference.md` to list Loan-court-extension-record among save codecs.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure or new build directory.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 19.46 s.
- Direct Release `polyec_court_tests.exe`: exit 0, **274 checks passed**.
- Debug build: exit 0.
- Targeted Debug CTest (`court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 89.31 s.
- Direct Debug `polyec_court_tests.exe`: exit 0, **274 checks passed**.
- `git diff --check`: exit 0; only the CRLF-to-LF notice for dirty `core/tests/test_realm.cpp` was printed.
- `python scripts/check-repository-links.py`: exit 0; 618 local document/resource references passed.

## Limits

No Python/native/Godot/Linux, training or performance gate was run. No commit or push. R4/M5 remain open.
EOF
