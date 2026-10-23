# R4 Person record codec — evidence (2026-10-20)

Plan: [R4 Person record codec](../design/R4-person-record-codec-plan.md). The slice extracts only the schema-independent Person record tokens from Realm save/load; it adds no gameplay behavior or public gameplay API.

## Delivered

- Added `person_section_codec.hpp` with stateless helpers in `polyec::detail`.
- Realm writes/reads the Person count and per-Person values at the same position and token order. Scalar values remain scalar on disk, including `household_membership.household_id`, `work_assignment.job`, and `work_assignment.job_kind`.
- Realm retains Person ownership, household reindex, whole-world invariant validation, temporary-world staging, and final atomic replacement.
- The existing fixed save fingerprint oracle (`7174156071039510872`) and save/load/continued replay checks remained unchanged and passed.

## Windows verification

- Visual Studio 18 Release solution build: exit 0.
- Release full CTest: 10/10 passed (16.01 s).
- Visual Studio 18 Debug solution build: exit 0.
- Debug targeted `realm_tests|schedule_tests|manager_tests|realm_clock_replay`: 4/4 passed (55.33 s).
- `git diff --check`: exit 0; the existing dirty `core/tests/test_realm.cpp` emits a CRLF normalization warning only.

## Process and limits

The OpenCode plan-only request was attempted with three available OpenCode models; all returned an internal server error. To keep the user-authorized work moving, Codex wrote/reviewed this narrow infrastructure plan and implemented the mechanical extraction. No schema/tag/token/spacing/API/gameplay change was made. Native, Python, GDExtension/Godot, Linux, training, and performance gates were not run. No commit/push.
