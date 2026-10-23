# R4 Knowledge record codec — evidence (2026-10-20)

Plan: [R4 Knowledge record codec](../design/R4-knowledge-record-codec-plan.md). Results apply to the dirty working tree based on `HEAD dff2781`.

## Delivered

- OpenCode added `core/include/polyec/knowledge_section_codec.hpp` and replaced only the primary Knowledge writer/reader section in `core/src/realm.cpp`.
- The codec preserves count followed by `observer`, `fact`, `arrives`, including whitespace; count remains capped at 1,000,000. Malformed/excessive counts and truncated rows retain `Save count limit` behavior.
- The reader stages the vector and publishes it only after every row parses. Duplicate rows and their order are retained.
- Realm continues rebuilding `knowledge_index_` immediately after codec read. Its existing rule remains: for duplicate `(observer, fact)` rows, keep the minimum `arrives`. The codec does not own or modify the index.
- Tests cover exact writer bytes, valid parse, malformed/excessive count, truncated row, output preservation and duplicate row order. Existing Realm fingerprint/continuation tests remain active.
- No schema, gameplay, court, API, binding, CMake or model change.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no new build directory was configured.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 21.51 s.
- Debug build: exit 0.
- Targeted Debug CTest (`realm_tests|court_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 103.00 s.
- Release `polyec_realm_tests.exe`: exit 0, **25,626 checks passed**.
- `git diff --check`: exit 0; only the CRLF-to-LF notice for dirty `core/tests/test_realm.cpp` was printed.

OpenCode completed the code edits but its verification turn entered an unbounded recursive search for CMake and was stopped. Codex ran the exact approved VS18 build/test commands directly against the existing build directory. No test result is inferred from OpenCode's interrupted turn.

## Limits

No Python/native/Godot/Linux, training or performance gate was run. No commit or push. R4/M5 remain open.
