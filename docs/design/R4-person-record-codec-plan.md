# R4 Person record codec — bounded plan

Review date: 2026-10-20. This is a behavior-preserving save-code refactor under the R4 row in [core architecture refactor](core-architecture-refactor.md) §7 and its reading contract in §9. The next M5 gameplay slices described by the roadmap need separate contracts; this slice keeps progress on the core data/save refactor without inventing those rules.

## Scope

- Extract only the serialized Person count and per-Person fields from `Realm::serialize` / `Realm::deserialize` into stateless `write_person_section` and `read_person_section` helpers.
- Preserve the exact token order, values, whitespace, count limit, validation order/messages, and schema behavior. The Person record remains schema-independent for Realm schemas 7–10.
- Keep `Realm` as owner of `people_`; after decode, Realm still calls `reindex_households`, validates the whole world, and commits the temporary Realm atomically.
- Keep `HouseholdMembership`, `WorkAssignment`, and other value records as in-memory composition only; serialize the same scalar fields at their existing token positions.

## Files and boundaries

- Add `core/include/polyec/person_section_codec.hpp` with inline helpers over `Person` records and streams. No state or public gameplay API is added.
- Update `core/src/realm.cpp` to call the helpers at the existing position after RNG state and before Province records. Keep whole-world deserialize staging, household reindex, and final invariant/commit in Realm.
- Add or adjust only the Realm save/load regression assertions in `core/tests/test_realm.cpp` if existing exact fingerprint/roundtrip coverage does not directly guard the extracted section.
- No CMake, binding, UI, action, observation, reward, model, or gameplay change.

## Invariants and edge cases

- Person count remains capped at 1,000,000; region, Family enum, and household label retain current checks and error text.
- Malformed/truncated streams retain existing failure behavior and cannot partially replace the running Realm.
- The helper does not rebuild indexes or validate cross-record ownership; Realm remains responsible.
- Save/load/save must produce byte-identical canonical text and retain the existing Realm fingerprint oracle.

## Gates

- Build `build/m5-core` Release and run full CTest.
- Build `build/m5-core` Debug and run `realm_tests|schedule_tests|manager_tests|realm_clock_replay`.
- Run `git diff --check` and inspect the serialized token sequence against the pre-slice implementation.

## Risks / exclusions

The core source is already dirty from accepted R1–R4/M5 slices; preserve all unrelated changes. Do not alter future EmploymentContract semantics, household funds/rights, ownership, land, or lifecycle. No commit/push.

## Plan review note

The requested OpenCode plan-only call was attempted with the configured OpenCode provider across three available models; each returned an internal server error. This narrow infrastructure-only plan was reviewed by Codex against the above architecture scope before implementation. If OpenCode service access recovers, resume its normal implementation role on the next slice.
