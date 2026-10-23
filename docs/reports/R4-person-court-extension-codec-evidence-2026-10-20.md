# R4 Person court extension codec - evidence (2026-10-20)

Plan: [R4 Person court extension codec](../design/R4-person-court-extension-codec-plan.md). Results apply to the dirty working tree based on `HEAD dff2781`.

## Delivered

- Added `core/include/polyec/person_court_extension_codec.hpp` with stateless `polyec::detail::write_person_court_extension_section` and `read_person_court_extension_section`.
- In `core/src/realm_court.cpp`, added the include and replaced only the per-Person court-extension loop in `write_court`/`read_court`.
- Writer preserves the exact token order and trailing whitespace for `candidate`, `office_appointment.office`, `special_advisor`, `office_appointment.hired_by`, `office_appointment.salary_due`, `office_appointment.office_notice`, `last_decision_day`. No count prefix.
- Reader stages a local copy of the incoming people vector, reads all seven court fields into that copy, and publishes only after every field in every record has parsed. Parse failures throw `std::runtime_error("Save court fields")`; no `Save count limit` for this section. No range validation is added.
- `Realm::read_court` converts the codec's expected `std::runtime_error` to `false` and preserves the existing `hired_by > people_.size()` validation immediately after codec read and before Province court counters. Primary Person fields remain untouched.
- Focused checks added in `core/tests/test_realm_court.cpp`: truncated extension record (seven fields partially present), bad input, output preservation of primary and court fields, canonical writer string `1 1 0 2 30 5 7 `, valid round trip of all seven fields, and input row order. Existing court/Realm save roundtrips remain the end-to-end oracle.
- No CMake, schema, gameplay, binding, native, Godot, Python, model or archive changes. `core/src/realm.cpp` and `person_section_codec.hpp` were not modified.
- Updated `docs/design/core-class-reference.md` to list Person-court-extension-record among save codecs.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure or new build directory.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 19.45 s.
- Direct Release `polyec_court_tests.exe`: exit 0, **352 checks passed**.
- Debug build: exit 0.
- Targeted Debug CTest (`court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 78.19 s.
- Direct Debug `polyec_court_tests.exe`: exit 0, **352 checks passed**.
- `git diff --check`: exit 0; only the CRLF-to-LF notice for dirty `core/tests/test_realm.cpp` was printed.
- `python scripts/check-repository-links.py`: exit 0; 625 local document/resource references passed.

## Limits

No Python/native/Godot/Linux, training or performance gate was run. No commit or push. R4/M5 remain open.
EOF