# R4 Investigation record codec — evidence (2026-10-20)

Plan: [R4 Investigation record codec](../design/R4-investigation-record-codec-plan.md). Results apply to the dirty working tree based on `HEAD dff2781`.

## Delivered

- Added `core/include/polyec/investigation_section_codec.hpp` with `polyec::detail::write_investigation_section(std::ostream&, const std::vector<Investigation>&)` and `read_investigation_section(std::istream&, std::vector<Investigation>&)`.
- In `core/src/realm_court.cpp`, added the include and replaced only the Investigation count/record loops with codec calls.
- The writer preserves count followed by `id`, `owner`, `cause`, `region`, `due`, `completed`, including existing whitespace. Count cap remains 1,000,000.
- Malformed/non-numeric/excessive Investigation count and truncated primary record all throw `std::runtime_error("Save court fields")` (not `Save count limit`), matching the legacy `read_court` caller-visible error. Reader stages records and publishes only after all six fields parse.
- Realm keeps owner/region validation immediately after codec read and before later court extensions; validation failure leaves `investigations_` unchanged.
- Investigation remains at its current byte position inside the court section; Proposal and other extensions remain untouched.
- Added focused codec checks in `core/tests/test_realm_court.cpp`: excessive/bad counts, truncated record, output preservation, exact writer bytes `1 7 8 9 10 11 0 `, valid field roundtrip and input order. Existing court roundtrip and Investigation tests remain.
- No CMake, schema, gameplay, bindings, UI, native, Godot, Python, model or archive changes.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure or new build directory.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 20.10 s.
- Direct Release `polyec_court_tests.exe`: exit 0, **205 checks passed**.
- Direct Release `polyec_realm_tests.exe`: exit 0, **25,634 checks passed**.
- Debug build: exit 0.
- Targeted Debug CTest (`court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 100.80 s.
- `git diff --check`: exit 0; only the pre-existing CRLF-to-LF notice for dirty `core/tests/test_realm.cpp` was printed.

## Limits

No Python/native/Godot/Linux, training or performance gate was run. No commit or push. R4/M5 remain open.

## Follow-up (Codex review, 2026-10-20)

- Codex review replaced the catch-all with `catch(const std::runtime_error&)` only.
- Exact VS18 Release build and Debug build both exit 0.
- Release `court_tests` 1/1 pass and Debug `court_tests` 1/1 pass.
- `git diff --check` exit 0 with the existing CRLF notice.
- This follow-up does not claim direct exe verification.
