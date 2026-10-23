# R4 referrals codec - evidence (2026-10-20)

Plan: [R4 referrals codec](../design/R4-referrals-codec-plan.md). Results apply to the dirty working tree based on current HEAD.

## Delivered

- Added `core/include/polyec/referrals_section_codec.hpp` with stateless `polyec::detail::write_referrals_section` and `read_referrals_section`.
- In `core/src/realm_court.cpp`, added the include and replaced only the referrals count/ID write and read in `write_court`/`read_court`.
- Writer preserves the exact token order and trailing whitespace: `<count> (<id>)*` with one count prefix. Canonical bytes for `{7, 8, 9}` are `3 7 8 9 `.
- Reader reads the count into a local bound check against `people_size` before allocating the staged vector, then reads every ID into that staged vector, and publishes it to the output reference only after all IDs parse. Count parse failure, excessive count, and truncated ID stream throw `std::runtime_error("Save court fields")` before assignment, so the caller's vector remains unchanged. `people_size` is read-only and never mutated.
- Per-ID domain validation (`1 <= id <= people_.size()`) remains in `Realm::read_court` immediately after the codec read and before Investigation is read. `Realm::read_court` calls the codec into a local `std::vector<uint64_t> staged_referrals`, validates every ID, and only then moves the staged vector into `referrals_`. A failed codec read or failed ID validation leaves `referrals_` unchanged; `deserialize` converts `read_court == false` to `std::runtime_error("Save court fields")`.
- Focused codec checks added in `core/tests/test_realm_court.cpp`: excessive count, bad/non-numeric count, truncated ID stream, codec-level ID non-validation (`2 0 4` parses into `{0, 4}`), canonical writer bytes, valid round trip, and input order preservation.
- Realm-level corrupted-save checks added using the augmented fixture (`fixture(6)` and `fixture(11)`): one referrals ID token mutated to `0` and one mutated to `257` (for the 256-person fixture), with deserialization rejecting as `Save court fields` and leaving the destination Realm fingerprint unchanged. The referrals block was isolatable by a unique token sequence in the serialized court byte stream, located before ` CLOCK_1 `.
- No CMake, schema, gameplay, binding, native, Godot, Python, model or archive changes. `core/src/realm.cpp` and `realm.hpp` were not modified.
- Marked `docs/design/R4-referrals-codec-plan.md` as approved-by-Codex with condition.

## Windows verification

Reused Visual Studio 18 build directory `build/m5-core`; no configure or new build directory.

- Release build: exit 0.
- Full Release CTest: 10/10 passed in 15.56 s.
- Direct Release `polyec_court_tests.exe`: exit 0, **450 checks passed**.
- Debug build: exit 0.
- Targeted Debug CTest (`court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_replay`): 5/5 passed in 70.60 s.
- Direct Debug `polyec_court_tests.exe`: exit 0, **450 checks passed**.
- `git diff --check`: exit 0 (CRLF-to-LF notice for dirty `core/tests/test_realm.cpp`; LF-to-CRLF notice for dirty `docs/DEVELOPMENT_HISTORY.md`).
- `python scripts/check-repository-links.py`: exit 0; 647 local document/resource references passed.

## Limits

No Python/native/Godot/Linux, training or performance gate was run. No commit or push. R4/M5 remain open.
EOF