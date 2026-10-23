# R4 Province record codec — evidence (2026-10-20)

Plan: [R4 Province record codec](../design/R4-province-record-codec-plan.md). This extracts only the existing primary Province save record. It does not add gameplay behavior or public API.

## Delivered

- Added `province_section_codec.hpp` with stateless helpers in `polyec::detail`.
- Realm writes/reads the Province count and existing primary fields at the same token position/order. The court extension retains `tax_receipts`, `food_capacity`, and `teacher_skill_sum` exactly where they were.
- Realm still validates province count against `regions_ + 1`, refreshes demography, checks the world invariant, and commits only the fully valid temporary Realm.
- The existing fixed save fingerprint and roundtrip/replay tests passed.

## Windows verification

- Visual Studio 18 Release solution build: exit 0.
- Release full CTest: 10/10 passed (16.19 s).
- Visual Studio 18 Debug solution build: exit 0.
- Debug targeted `realm_tests|schedule_tests|manager_tests|realm_clock_replay`: 4/4 passed (54.66 s).
- `git diff --check`: exit 0, with only the existing CRLF normalization warning for dirty `core/tests/test_realm.cpp`.

## Process and limits

OpenCode still returned internal server errors in the available models; this infrastructure-only extraction followed the Codex-reviewed plan. No schema, token order/spacing, public API, gameplay, or binding change. Native, Python, GDExtension/Godot, Linux, training, and performance gates were not run. No commit/push.
