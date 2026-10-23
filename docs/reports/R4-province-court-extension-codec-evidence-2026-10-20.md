# R4 Province court extension codec - evidence (2026-10-20)

Plan: [R4 Province court extension codec](../design/R4-province-court-extension-codec-plan.md). Results apply to the dirty working tree based on current HEAD.

## Delivered

- Added `core/include/polyec/province_court_extension_codec.hpp` with stateless `polyec::detail::write_province_court_extension_section` and `read_province_court_extension_section`.
- In `core/src/realm_court.cpp`, added the include and replaced only the Province court-counter loop in `write_court`/`read_court`.
- Writer preserves the exact token order and trailing whitespace for `tax_receipts`, `food_capacity`, `teacher_skill_sum`. No count prefix.
- Reader stages a local copy of the incoming provinces vector, reads all three extension fields into that copy, and publishes only after every field in every record has parsed. Parse failures throw `std::runtime_error("Save court fields")`; no `Save count limit` for this section. No range validation is added.
- `Realm::read_court` converts the codec's expected `std::runtime_error` to `false`. Primary Province fields remain untouched.
- Focused checks added in `core/tests/test_realm_court.cpp`: truncated extension (`tax_receipts` present, `food_capacity`/`teacher_skill_sum` missing; then `tax_receipts` and `food_capacity` present, `teacher_skill_sum` missing), bad input, output preservation including representative primary fields, canonical writer string `20 30 50 `, valid round trip of all three extension fields, and input row order. Existing court/Realm save roundtrips remain the end-to-end oracle.
- No CMake, schema, gameplay, binding, native, Godot, Python, model or archive changes. `core/src/realm.cpp` and `province_section_codec.hpp` were not modified.
- Updated `docs/design/core-class-reference.md` to list Province-court-extension-record among save codecs.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure or new build directory.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 19.33 s.
- Direct Release `polyec_court_tests.exe`: exit 0, **412 checks passed**.
- Debug build: exit 0.
- Targeted Debug CTest (`court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 91.46 s.
- Direct Debug `polyec_court_tests.exe`: exit 0, **412 checks passed**.
- `git diff --check`: exit 0 (CRLF-to-LF notice only for dirty `core/tests/test_realm.cpp`).
- `python scripts/check-repository-links.py`: exit 0; 631 local document/resource references passed.

## Limits

No Python/native/Godot/Linux, training or performance gate was run. No commit or push. R4/M5 remain open.
EOF
