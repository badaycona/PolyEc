# R4 Fact record codec — evidence (2026-10-20)

Plan: [R4 Fact record codec](../design/R4-fact-record-codec-plan.md). Results apply to the dirty working tree based on `HEAD dff2781`.

## Delivered

- OpenCode added `core/include/polyec/fact_section_codec.hpp` and replaced only the Fact primary record writer/reader loops in `core/src/realm.cpp`.
- The codec preserves count plus `id`, `parent`, `actor`, `target`, `day`, `region`, `kind`, `amount`, `secret`, including whitespace. Count remains capped at 8,192. No new Fact field validation was added.
- The reader stages records, rejects malformed/excessive count or truncated record with `Save count limit`, then publishes only after all nine fields parse.
- Realm remains owner of Facts, keeps `knowledge_index_` reconstruction from Knowledge in place, validates the complete staging world, and commits only after all sections succeed.
- Tests check exact writer bytes, valid fields, bad/excessive counts, truncated record and output-vector preservation. Existing Realm save/load fingerprint and continuation tests remain active.
- No schema, gameplay, court, API, bindings, CMake or model changes.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure/new build directory.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 20.64 s.
- Debug build: exit 0.
- Targeted Debug CTest (`realm_tests|court_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 87.02 s.
- After correcting a test label, rebuilt Release target `polyec_realm_tests` and ran it directly: exit 0, **25,617 checks passed**.
- `git diff --check`: exit 0; only the CRLF-to-LF notice for dirty `core/tests/test_realm.cpp` was printed.

## Limits

No Python/native/Godot/Linux, training or performance gate was run. No commit or push. R4/M5 remain open.
