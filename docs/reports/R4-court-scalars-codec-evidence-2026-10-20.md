# R4 court scalars codec - evidence (2026-10-20)

Plan: [R4 court scalars codec](../design/R4-court-scalars-codec-plan.md). Results apply to the dirty working tree based on current HEAD.

## Delivered

- Added `core/include/polyec/court_scalars_codec.hpp` with stateless `polyec::detail::write_court_scalars` and `read_court_scalars`.
- In `core/src/realm_court.cpp`, added the include and replaced only the leading seven-scalar write/read in `write_court`/`read_court`.
- Writer preserves the exact token order and trailing whitespace for `royal_reference_`, `credit_pool_`, `restricted_fund_`, `mediation_ready_`, `bonus_offices_`, `pact_`, `reference_scenario_`. No count prefix.
- Reader stages all seven values into local temporaries and publishes them to the output references only after every scalar has parsed. Parse failures throw `std::runtime_error("Save court fields")`; no `Save count limit` for this section. No range validation is added. Truncation is not field-boundary detection (unframed section).
- `Realm::read_court` converts the codec's expected `std::runtime_error` to `false`. Realm members remain unchanged on parse failure.
- Focused checks added in `core/tests/test_realm_court.cpp`: truncated streams of one through six tokens, bad input, output preservation for all seven scalars, canonical writer string `100 20 30 456 2 1 0 `, valid round trip of all seven scalars. Existing court/Realm save roundtrips remain the end-to-end oracle.
- No CMake, schema, gameplay, binding, native, Godot, Python, model or archive changes. `core/src/realm.cpp` and `realm.hpp` were not modified.
- Updated `docs/design/core-class-reference.md` to list Court-scalars-record among save codecs.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure or new build directory.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 19.99 s.
- Direct Release `polyec_court_tests.exe`: exit 0, **428 checks passed**.
- Debug build: exit 0.
- Targeted Debug CTest (`court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 89.55 s.
- Direct Debug `polyec_court_tests.exe`: exit 0, **428 checks passed**.
- `git diff --check`: exit 0 (CRLF-to-LF notice only for dirty `core/tests/test_realm.cpp`; LF-to-CRLF notice for dirty `docs/DEVELOPMENT_HISTORY.md`).
- `python scripts/check-repository-links.py`: exit 0; 641 local document/resource references passed.

## Limits

No Python/native/Godot/Linux, training or performance gate was run. No commit or push. R4/M5 remain open.
EOF