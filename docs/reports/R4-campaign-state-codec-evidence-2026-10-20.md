# R4 Campaign state codec — evidence (2026-10-20)

Plan: [R4 Campaign state codec](../design/R4-campaign-state-codec-plan.md). Results apply to the dirty working tree based on `dff2781`; no commit or push.

## Delivered

- Added `core/include/polyec/campaign_state_codec.hpp` with stateless `polyec::detail::write_campaign_state` and `read_campaign_state` for the Campaign-owned payload between the `POLYEC_M4_1` magic and the quoted Realm save.
- `Campaign` retains the magic/schema wrapper, Realm quote boundary, Realm ownership, whole-Campaign invariant, error translation, and final move-commit. The codec stages payload state and derived bonded-goods totals; it does not validate or deserialize Realm state.
- Preserved token order and spacing, bool/`std::quoted` extraction, all existing group bounds, bonded accumulation order, and first-failure messages: `M4 save record bound`, `M4 path missing`, `M4 manifest invalid`, `M4 save incomplete`, and `M4 save trailing data`.
- Before editing `campaign.cpp`, captured the canonical payload from the unchanged implementation. Its fixed test literal is 622 bytes and has SHA-256 `7136e667aeb2c5c6d42e028e338beda1dece03c7d36aed01ca2ea2f51a722cb4`. A decode-back check of the embedded `\xHH` bytes matched the length and hash exactly.
- Added tests for golden writer bytes, the Campaign payload slice and quoted Realm boundary, staged values and bonded totals, count/record/truncation errors, and atomic rejection for both normal and pre-paused destination Campaigns. Existing continuation and file save/load tests remain.
- `core/src/campaign.cpp` already contained a dirty WorkAssignment field-path edit before this slice. That pre-existing line remains in the combined diff and was not part of the Campaign codec change.

## Windows verification

Reused the existing Visual Studio build directory `build/m5-core`; no reconfiguration or new build directory.

- Release build: exit 0.
- Full Release CTest: **10/10 passed** in 21.11 s.
- Direct Release `polyec_campaign_tests.exe`: **263/263 checks passed**, exit 0.
- Debug build: exit 0.
- Targeted Debug CTest (`campaign_tests|realm_tests|court_tests|schedule_tests|realm_clock_tests`): **5/5 passed** in 132.84 s.
- Scoped `git diff --check` for the tracked edited files: exit 0. The new header was separately checked for UTF-8 decoding and trailing whitespace; none found.
- `python scripts/check-repository-links.py`: exit 0; 666 local references passed.

## Limits

No schema, gameplay, API, binding, UI, action/observation/reward/model, native, Python, Godot, Linux, training or performance change/gate. No commit or push. The working tree's unrelated dirty files were preserved. R4's next candidate and remaining M5 work are not closed by this slice.
