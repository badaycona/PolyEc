# M5 T2 — Employment/Job classification foundation (implementation contract)

Status: Codex accepted the foundation slice after source/test review; realm tests 25,322 checks, full CTest 10/10, Python smoke pass. T2 actual work/settlement (presence/attendance/output/settlement) remains open; this plan covers only the read-only classification foundation.

## Files

- `core/include/polyec/jobs/job.hpp` (new) — `EmploymentState {Idle, Employed}`, `JobType {None, Worker, Teacher, AppointedOffice}`, abstract `Job` with `type()`, `name()` returning `std::string_view`, `is_work_job()`; all `noexcept`.
- `core/include/polyec/jobs/worker_job.hpp` (new) — `WorkerJob : public Job`; shared singleton `kWorkerJob`.
- `core/include/polyec/jobs/teacher_job.hpp` (new) — `TeacherJob : public Job`; shared singleton `kTeacherJob`.
- `core/include/polyec/jobs/office_role_job.hpp` (new) — `AppointedOfficeJob : public Job`; shared singleton `kAppointedOfficeJob`.
- `core/include/polyec/realm.hpp` — declares pure resolver `describe_job(const Person&, int current_day)` after Person and three read-only Realm queries.
- `core/src/realm.cpp` — implements `describe_job` and Realm query delegation.
- `bindings/python/realm_binding.cpp` — read-only `employment_state`/`job_type` strings in person dict.
- `core/tests/test_realm.cpp` — classification tests via pure resolver fixtures and deterministic Court appointment.

## Contract

- Query-only inheritance: `Worker`, `Teacher`, `AppointedOffice` subclass `Job`; no `can_start/on_start/on_clear/on_expire`; no per-NPC heap allocation, no new Person field, no `job_type_id`.
- Stateless shared singletons (`kWorkerJob`, `kTeacherJob`, `kAppointedOfficeJob`); resolver dispatches to singletons only for alive + valid canonical assignment.
- Pure resolver `describe_job(const Person&, int current_day)` returns `const Job*`: `nullptr` for no active job (dead, idle, expired, unknown `job_kind`, invalid office). Appointed office (`office && hired_by!=0 && family==Governance`) takes precedence; active work job is `job>=0 && busy_until>=day_` with `job_kind==0` (Worker) or `job_kind==1` (Teacher); unknown `job_kind` (not 0/1) → `nullptr`.
- `Realm::job_type`/`employment_state`/`job_descriptor` delegate to `describe_job`; `job_descriptor` returns `const Job*` (nullptr for None/Idle). Implementation and direct tests share one logic path.
- Canonical save source: Person work fields (`job`, `job_kind`, `busy_until`) are serialized in the Person block in `core/src/realm.cpp`; office/court fields (`candidate`, `office`, `special_advisor`, `hired_by`, `salary_due`, `office_notice`, `last_decision_day`) via `Realm::write_court`/`read_court` in `core/src/realm_court.cpp`. No schema change; `realm_schema` stays `realm-v6`.
- Python person dict gains read-only `"employment_state"` (`"Idle"`/`"Employed"`) and `"job_type"` (`"None"`/`"Worker"`/`"Teacher"`/`"AppointedOffice"`). `REALM_OBS` stays 512; observation/action/reward/policy code untouched.
- No lifecycle, payroll, counter, eligibility, or save-format change in this slice. No `ActivitySession.delivered` semantics change (`delivered` remains attention units, not real work).
- Windows-only verification.

## Out of scope

- T2 actual work-time accounting, end-of-day production/payment/material settlement, RL observation/action/reward changes, model retraining, native/Godot changes.
