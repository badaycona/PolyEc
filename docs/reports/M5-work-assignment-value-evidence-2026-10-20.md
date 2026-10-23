# M5 WorkAssignment value composition — evidence

Ngày: 2026-10-20. Dirty working tree based on existing project state; no commit/push. Scope and gates: [approved bounded plan](../design/M5-work-assignment-value-plan.md).

## Result

Added `Person::work_assignment` containing the legacy `job` and `job_kind` values. `busy_until` remains a direct Person field. Updated EmploymentManager, Realm save/load and classifiers, Campaign migration clear, court checks, Python binding source, native benchmark, and tests. Updated the Person structure reference. No independent work-assignment ID/token/schema was added; the two fields continue through the same Realm save tokens and Python retains the `job_kind` dictionary key. No gameplay behavior was intentionally changed.

## Gates actually run

- VS18 Release core build: `cmake --build build/m5-core --config Release --parallel 4`, exit 0.
- Release CTest: full suite, **10/10 passed** in 16.21 s.
- VS18 Debug core build: exit 0.
- Debug CTest: `realm_tests|schedule_tests|manager_tests|realm_clock_replay`, **4/4 passed** in 53.21 s.
- Existing native build: `polyec_realm_benchmark` and `polyec_realm_contracts`, Release build exit 0; `runtime_contracts`, **1/1 passed** in 0.97 s.
- Python Release and Debug builds: `polyec_env` and `polyec_realm_transition_replay`, exit 0 in both configurations. Clock binding replay: **168 Python checks and 7 C++ checkpoints matched** in each configuration. Transition replay emitted **64 checkpoints** in each configuration.
- `rg --pcre2 '(?<!work_assignment)\.(job|job_kind)\b' core native bindings/python`: no matches.
- `git diff --check`: exit 0; Git emitted only its existing CRLF-to-LF working-copy notice for `core/tests/test_realm.cpp`.
- Existing Realm fingerprint oracle `Realm(7,2,128) == 7174156071039510872` passed as part of Release `realm_tests`, preserving serialized-content compatibility.

## Review notes and limitations

OpenCode's implementation pass initially omitted many call sites, produced an invalid comparison while editing `start_work`, and removed `Person::skill` from the field declaration. Codex stopped that pass, completed the mechanical field-path migration, restored `skill`, repaired formatting, and reviewed the final source with the gates above. The failed initial build was caused by the missing `skill`; it was rerun after repair and the final Release build and tests passed.

The value is only a transitional grouping of the two existing scalars. It does not implement the future EmploymentContract relationship, establishment ownership, or new work rules. No commit/push, Godot app, Linux, training, or benchmark run was performed; only the benchmark target was built.
