# R4 Proposal primary record codec — evidence (2026-10-20)

Plan: [R4 Proposal primary record codec](../design/R4-proposal-record-codec-plan.md). Results apply to the dirty working tree based on `HEAD dff2781`.

## Delivered

- OpenCode added `core/include/polyec/proposal_section_codec.hpp` and replaced only the Proposal primary count/record writer/reader call sites in `core/src/realm.cpp`.
- The writer preserves count followed by `id`, `from`, `to`, `amount`, `due`, `resolved`, including existing whitespace. Count cap remains 1,000,000.
- Malformed/excessive count returns `Save count limit`. An incomplete primary Proposal record returns `Save court fields`, matching the current behavior when the following `read_court` encounters the failed stream.
- Reader stages records and publishes only after all six fields parse. Realm still owns Proposals, reads extensions into staged records, validates the complete world and commits only at the existing final move.
- Proposal extensions (`kind`, `ready`, `escrow`, `public_source`, `mediation`, `source_region`, `cause`, `parties`, `regions`, `pledged`) remain untouched in their existing later `write_court`/`read_court` position, after the other court extension loops.
- Tests cover exact writer bytes, valid parse, bad/excessive counts, truncated record error, output preservation and existing Realm/court save roundtrips. No schema, gameplay, binding, API, CMake, or model change.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure or new build directory.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 19.45 s.
- Debug build: exit 0.
- Targeted Debug CTest (`realm_tests|court_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 100.41 s.
- Direct Release `polyec_realm_tests.exe`: exit 0, **25,634 checks passed**.
- `git diff --check`: exit 0; only the CRLF-to-LF notice for dirty `core/tests/test_realm.cpp` was printed.

## Limits

No Python/native/Godot/Linux, training or performance gate was run. No commit or push. R4/M5 remain open.
