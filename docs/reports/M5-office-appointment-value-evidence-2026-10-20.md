# M5 — OfficeAppointment value composition evidence

## Result

Accepted on 2026-10-20. Source context is the dirty working tree based on `HEAD dff2781`; existing R3/T2/R4 changes were retained. No commit or push was created.

`Person::office`, `hired_by`, `salary_due`, and `office_notice` are now composed as `Person::office_appointment`. Existing values/defaults, predicates and transitions remain unchanged. `candidate`, `special_advisor`, `family`, authority, work assignment and ScheduleBook are unaffected. `last_decision_day` remains a direct Person field with its original default. `write_court` and `read_court` use the same per-person token order, and schema remains `POLYEC_REALM_10`.

The plan-only OpenCode response initially proposed an unapproved office/work attendance exclusion and save normalization; Codex rejected it. OpenCode then began the approved structural edit but its partial pass omitted `last_decision_day` and could not reliably edit the remaining test references. Codex stopped that pass, restored the retained field, completed only the exact approved field-path substitutions, updated the discovered native contract test, and added the frozen full-save fingerprint assertion. No gameplay behavior or contract was broadened.

## Verification

- Existing Windows VS18 `build/m5-core`: Release full build succeeded; full CTest **10/10 passed** (16.25 s).
- Release `polyec_realm_tests.exe`: **25,591 checks passed**; `polyec_manager_tests.exe` passed.
- Existing Windows VS18 `build/m5-runtime`: `polyec_realm_contracts` target built successfully; `runtime_contracts` CTest **1/1 passed** (1.13 s).
- `build/m5-core`: Debug full build succeeded; targeted CTest (`realm_tests|schedule_tests|manager_tests|realm_clock_replay`) **4/4 passed** (54.17 s).
- The Realm test verifies the frozen pre-refactor fingerprint `7174156071039510872` for `Realm(7,2,128)`, covering the complete serialized Realm string.
- `git diff --check` exit 0; Git printed only the existing CRLF-to-LF notice for `core/tests/test_realm.cpp`.

No Python binding, Godot, Linux or training gate was needed or run. The one native source change is only `native/realm_contracts.cpp` test access to the composed field; production native behavior is unchanged. No new build directory or generator was created.
