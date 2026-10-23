# R4 Project primary record codec — evidence (2026-10-20)

Plan: [R4 Project primary record codec](../design/R4-project-record-codec-plan.md). Results are from the dirty working tree based on `HEAD dff2781`.

## Delivered

- OpenCode added `core/include/polyec/project_section_codec.hpp` and replaced only the Project primary record writer/reader call sites in `core/src/realm.cpp`.
- The writer preserves the existing count then `id`, `owner`, `region`, `due`, `kind`, `budget`, `completed` sequence and trailing space. The count cap remains 1,000,000; no Project field validation was added.
- The reader stages records and throws `Save count limit` on malformed/excessive count or incomplete primary record. Realm retains Project ownership, full-world validation and atomic load commit.
- Court fields `engineer`, `stalled`, and `cause` remain in their existing `write_court`/`read_court` loops and token position.
- Focused tests in `core/tests/test_realm.cpp` cover malformed/excessive counts, truncated records, unchanged output including non-default court fields after failure, exact writer bytes, and valid primary field parsing. Existing Realm and court serialization/continuation tests exercise integrated Projects.
- No schema, gameplay, API, bindings, CMake, or model changes.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure or new generator/build directory was used.

- Release build: exit 0. MSBuild reported one transient access-denied retry for a target state file; it retried successfully.
- Full Release CTest: 10/10 passed in 19.25 s.
- Debug build: exit 0.
- Targeted Debug CTest (`realm_tests|court_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 98.59 s.
- Direct Release `polyec_realm_tests.exe`: exit 0, **25,607 checks passed**.
- `git diff --check`: exit 0; only the CRLF-to-LF notice for the existing dirty `core/tests/test_realm.cpp` was printed.

## Limits

No Python/native/Godot/Linux, training or performance gate was run. No commit or push. R4 and M5 remain open.
