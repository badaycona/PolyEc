# R4 Proposal court extension codec — evidence (2026-10-20)

Plan: [R4 Proposal court extension codec](../design/R4-proposal-court-extension-codec-plan.md). Results apply to the dirty working tree based on `HEAD dff2781`.

## Delivered

- Added `core/include/polyec/proposal_court_extension_codec.hpp` with stateless `polyec::detail::write_proposal_court_extension_section` and `read_proposal_court_extension_section`.
- In `core/src/realm_court.cpp`, added the include and replaced only the Proposal court-extension loops in `write_court`/`read_court`.
- Writer preserves the exact token order and trailing whitespace for `kind`, `ready`, `escrow`, `public_source`, `mediation`, `source_region`, `cause`, `parties[0..2]`, `regions[0..2]`, `pledged[0..2]`. No count prefix.
- Reader stages a local copy of the incoming proposals vector, reads all extension fields into that copy, and publishes only after every field in every record has parsed. Parse failures throw `std::runtime_error("Save court fields")`; no `Save count limit` for this section.
- `Realm::read_court` converts the codec's expected `std::runtime_error` to `false`, then retains the existing `kind`, `source_region` and `regions` validation immediately after the codec read and before return. Primary Proposal fields remain untouched.
- Focused checks added in `core/tests/test_realm_court.cpp`: truncated pledged/regions/parties, bad input, output preservation, canonical writer string `1 12 13 1 0 14 15 16 17 18 1 2 3 0 1 0 `, valid round trip of every extension field, and input row order. Existing court/Realm save roundtrips remain the end-to-end oracle.
- No CMake, schema, gameplay, binding, native, Godot, Python, model or archive changes. `core/src/realm.cpp` and `proposal_section_codec.hpp` were not modified.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure or new build directory.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 19.71 s.
- Direct Release `polyec_court_tests.exe`: exit 0, **263 checks passed**.
- Debug build: exit 0.
- Targeted Debug CTest (`court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 83.47 s.
- Direct Debug `polyec_court_tests.exe`: exit 0, **263 checks passed**.
- `git diff --check`: exit 0; only the CRLF-to-LF notice for dirty `core/tests/test_realm.cpp` was printed.

## Limits

No Python/native/Godot/Linux, training or performance gate was run. No commit or push. R4/M5 remain open.
EOF
filePath: C:\prc\PolyEc\docs\reports\R4-proposal-court-extension-codec-evidence-2026-10-20.md