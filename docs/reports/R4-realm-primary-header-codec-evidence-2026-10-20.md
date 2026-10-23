# R4 Realm primary header codec - evidence (2026-10-20)

Plan: [R4 Realm primary header codec](../design/R4-realm-primary-header-codec-plan.md). Results apply to the dirty working tree based on current HEAD.

## Delivered

- Added `core/include/polyec/realm_primary_header_codec.hpp` with a staging `RealmPrimaryHeader`, exact-order writer, and separate initial/tail readers.
- Replaced only the primary header write/read in `Realm::serialize` and `Realm::deserialize`. The save magic/bundle, Person section, court sections, `CLOCK_1`, and schedule section retain their existing order.
- The initial group (`day` through `endowment`) retains `Save geography` for parse failure and invalid `regions`; the tail (`Draft` through RNG) leaves the stream failed so `read_person_section` retains `Save count limit`. No new header error string or domain validation was added. Realm still performs final validation and commits the staged world atomically.
- Added codec tests for initial/tail parsing and output preservation, all-field/RNG round trip, and a complete canonical writer-byte oracle including the seeded RNG state and final trailing space.
- Added Realm malformed-save tests for invalid regions, truncated initial header (`Save geography`), and truncated tail (`Save count limit`), each preserving the destination fingerprint.
- Existing Realm state/fingerprint save-load and corruption tests remain active.

## Review corrections

- The first Release CTest run failed 6/10 because the initial implementation accidentally called `read_person_section` twice. Codex review removed the duplicate call and temporary stderr instrumentation, restored the deserialize catch/function close, then rebuilt and reran Release CTest successfully.
- The first canonical-byte test failed because its fixture assigned `13` to boolean `Draft::rerolled`, while the legacy writer serializes that field as `1`. The plan and fixture were corrected to use `true`; the expected bytes now contain `1`. No gameplay or save change was involved.
- OpenCode's initial debug attempt briefly introduced a missing include/brace while adding temporary output; that instrumentation was removed before final verification.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure or new build directory.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 6.92 s.
- Direct Release `polyec_realm_tests.exe`: exit 0, **25,654 checks passed**.
- Debug build: exit 0.
- Targeted Debug CTest (`realm_tests|court_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 50.57 s.
- `git diff --check`: exit 0; existing CRLF/LF notices only.
- `python scripts/check-repository-links.py`: exit 0; 649 local document/resource references passed.

## Limits

No Python/native/Godot/Linux, training, or performance gate was run. No commit or push. The remaining R4 view projection boundary and M5 gameplay/economic contracts remain open.
