# M5 — OfficeAppointment value composition plan

## Status

Accepted by Codex on 2026-10-20 after final diff review and Windows core/native gates. This is a structural class-relationship refactor under [core architecture refactor](core-architecture-refactor.md) and [core class reference](core-class-reference.md) §12.3. It preserves the existing appointment behavior and does not add gameplay semantics. Evidence: [OfficeAppointment value composition](../reports/M5-office-appointment-value-evidence-2026-10-20.md).

The first OpenCode proposal was rejected because it invented an office/work-attendance exclusion rule, session retirement on appointment, and save normalization. Those are outside this contract. The revised proposal is limited to data composition.

## Before / after

Today `Person` has four direct appointment fields: `office`, `hired_by`, `salary_due`, and `office_notice`. Group these exact values in an `OfficeAppointment` value subrecord composed inside `Person`. Person remains the stable appointee identity; `hired_by` remains the same appointing Person ID. The record adds no ID, title, terms, derived state, or second source of truth.

Keep `candidate` and `special_advisor` separate. Keep `family`, `sovereign`, `taxing_authority`, work assignment, schedule sessions, attendance, settlement, and pay behavior unchanged. Do not add an appointment-versus-work restriction or mutate ScheduleBook from appointment transitions.

## Files and responsibilities

- `core/include/polyec/realm.hpp`: define `OfficeAppointment` with the existing defaults (`office=false`, `hired_by=0`, `salary_due=30`, `office_notice=0`) and replace those four direct `Person` fields with one composed value member. Keep `last_decision_day=-1` as a direct `Person` field; do not otherwise change `Person`.
- `core/include/polyec/managers/employment_manager.hpp`: update its four-field accesses only. Preserve preflight, work clearing, appointment fee, due/notice calculations, end-office behavior and exception guarantees.
- `core/src/realm.cpp`: update the current direct reads in candidate generation, timeline-off role top-ups, office salary processing, and job classification. Preserve all predicates/order/results.
- `core/src/realm_court.cpp`: update `office_used`, `write_court`, and `read_court`. Write/read the same four values in their existing positions and order among the seven per-Person court values. Preserve all surrounding fields and stream grammar.
- `core/tests/test_managers.cpp`, `core/tests/test_realm.cpp`, `core/tests/test_realm_court.cpp`, `native/realm_contracts.cpp`: update direct field references and retain existing appointment, resignation, salary, classification, invalid-reference, save/load and atomic rejection coverage. Add the full-save fingerprint oracle below. The native file is a contract test that directly reads `Person::hired_by`; no production native behavior changes.

No Python binding, native runtime, Godot, action, observation, reward, model, CMake, or other save-codec change is in scope. The `Person` C++ field access changes from four direct members to `person.office_appointment.<field>`; update the one in-repository native contract test that accesses this layout. No binary ABI stability is promised for direct C++ consumers.

## Ownership and current invariants

- Realm continues to own each Person. The appointment value is composed in that person's record; its `hired_by` is a reference to another existing Person, not ownership of that Person.
- Preserve the existing semantics exactly: `office` and `hired_by` remain separate fields (do not infer one from the other); `can_resign` checks `hired_by != 0 && day >= office_notice`; `salary_due` remains a game-day value; ending an office clears `office`, `hired_by`, and changes `family` as today but retains stale `salary_due`/`office_notice` values.
- Keep existing appointment/work interactions exactly as implemented, including appointment's existing `clear_work` behavior. Do not alter schedule records or attendance; do not assume mutual exclusion beyond existing transition code.
- Keep current person ID/reference validation and whole-Realm invariant behavior. Do not add new invariant or reject cases.

## Flows and edge cases to preserve

- Appoint success/failure: same candidate selection/referral order, preflight, fee transfer, prior-work clearing, fields set, return value and atomic reject behavior.
- Resign before/at notice day: same `can_resign` boundary and state writes. Keep deadline values after ending office.
- Salary due/short treasury: same day predicate, source account, amount, due-date advancement and end-office/event path.
- Query/classification: same `office_used`, office-capacity, valid appointment priority, invalid appointment fall-through, and role top-up predicates.
- Fresh construction, Realm 7/8/9/10 load, round-trip and corrupt `hired_by`: same defaults, token validation and failure behavior.

## Save compatibility oracle

Do not bump `POLYEC_REALM_10`, add fields, or alter token ordering/spacing. `write_court`/`read_court` must still serialize:

```text
candidate office special_advisor hired_by salary_due office_notice last_decision_day
```

in the existing per-person order. Before editing, the current Release Python binding produced `Realm(7, 2, 128).fingerprint() == 7174156071039510872` (`fingerprint` hashes the complete `serialize("fingerprint")` text, 22,978 bytes). Add a C++ test asserting this value for the same fresh Realm so the complete serialized Realm remains byte-compatible. Retain explicit migration and failed-load atomicity tests.

Because the save bytes and schema stay identical, old code can read new saves and new code can read old saves under this schema. The internal C++ `Person` member names/layout change is a source/layout change for direct C++ consumers; repository adapters do not expose that layout, and this slice has no promise of binary ABI stability.

## Cross-surface and acceptance gates

- Only core headers/source and the listed core tests change. Public command, read-only views, Python binding keys, action/observation/reward/model shape and behavior stay unchanged.
- Use only existing Windows `build/m5-core` VS18 configuration. Run full Release build + CTest 10/10; full Debug build + targeted CTest `realm_tests|schedule_tests|manager_tests|realm_clock_replay`; build and run the existing `build/m5-runtime` native `runtime_contracts` target/test if its configured cache is available; record test counts and `git diff --check`.
- No new build directory, generator, dependency, gameplay rule, commit, push, training, or platform expansion.

## OpenCode implementation handoff

OpenCode provided the plan-only design and began implementation. Its partial pass missed a retained Person field and did not complete the remaining test edits; Codex completed only the bounded mechanical substitutions and oracle after reviewing the partial diff. If later source work requires changing serialization bytes or behavior, stop and return with the precise blocker rather than expanding the contract.
