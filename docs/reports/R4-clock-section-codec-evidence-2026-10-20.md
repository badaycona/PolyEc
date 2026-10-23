# R4 clock section codec - evidence (2026-10-20)

Plan: [R4 clock section codec](../design/R4-clock-section-codec-plan.md). Results apply to the dirty working tree based on current HEAD.

## Delivered

- Added `core/include/polyec/clock_section_codec.hpp` with `polyec::detail::write_clock_section` and `polyec::detail::read_clock_section`.
- Replaced only the inline `CLOCK_1` write in `Realm::serialize` and the inline `CLOCK_1` read in `Realm::deserialize`. Court and schedule calls remain at their existing positions.
- The writer preserves the exact legacy bytes ` CLOCK_1 <paused 0|1> <pause_revision> `.
- The reader stages the tag, flag and revision, throws `std::runtime_error("Save clock header")` for tag/flag/revision parse failure, and publishes only after a complete successful read. No range or semantic validation was added; negative tokens accepted by `std::istream` extraction into `uint64_t` (including `-1` yielding `UINT64_MAX` under MSVC) are preserved.
- `Realm` retains ownership of `paused_` and `pause_revision_`, and deserialize remains atomic.
- Added codec checks for canonical bytes, round trips including `0` and `UINT64_MAX`, bad tag, out-of-range flag, truncated flag/revision, revision parse failure, unsigned magnitude overflow, negative token, empty input, and output preservation.
- Added Realm-level malformed-save checks for tag mutation, flag mutation to `2`, and truncated revision, each verifying rejection with `Save clock header` and an unchanged destination fingerprint.
- Existing clock, pause, overflow, controller and roundtrip tests remain active.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure or new build directory.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 17.24 s.
- Direct Release `polyec_realm_clock_tests.exe`: exit 0, 301 checks.
- Debug build: exit 0.
- Targeted Debug CTest (`court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_tests|realm_clock_replay`): 6/6 passed in 76.62 s.
- `git diff --check`: exit 0 (CRLF/LF notices only).
- `python scripts/check-repository-links.py`: exit 0; 662 local document/resource references passed.

## Limits

No Python/native/Godot/Linux, training, or performance gate was run. No commit or push. R4/M5 remain open.
EOF