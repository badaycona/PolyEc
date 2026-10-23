# R4 Edge record codec — evidence (2026-10-20)

Plan: [R4 Edge record codec](../design/R4-edge-record-codec-plan.md). The slice extracts only the existing Edge count and records from the Realm save stream.

## Delivered

- OpenCode added `edge_section_codec.hpp` with stateless `polyec::detail` write/read helpers and changed only the two Edge section call sites in `Realm::serialize` / `Realm::deserialize`.
- The section still writes count then `from`, `to`, `kind`; count remains capped at 8,000,000. No new Edge field validation was added.
- The reader stages records and preserves `Save count limit` for count parse/excess and truncated record extraction. Realm remains the owner and still performs whole-world checks and atomic load commit.
- Existing fixed fingerprint, roundtrip, and replay tests passed; save schema and token sequence are unchanged.

## Windows verification

- OpenCode run: Visual Studio 18 Release build + full CTest 10/10 (16.02 s); Debug build + targeted `realm_tests|schedule_tests|manager_tests|realm_clock_replay` CTest 4/4 (55.83 s).
- After Codex review added the direct `<utility>` include for `std::move`; final Release build + full CTest 10/10 (16.16 s), final Debug build + targeted CTest 4/4 (54.68 s).
- `git diff --check`: exit 0, with only the existing CRLF normalization warning for dirty `core/tests/test_realm.cpp`.

## Process and limits

OpenCode plan-only was reviewed by Codex with an explicit truncated-record error-preservation correction, then OpenCode implemented the approved scope. The project OpenCode config now selects a unique primary `polyec` agent and `opencode/beam-free`; the prior default agent name collided with a subagent, and the paid model returned `Insufficient account funds`. The free model reached successfully. No gameplay, API, binding, CMake, native/Python/Godot/Linux, training, or performance gate was changed/run. No commit/push.
