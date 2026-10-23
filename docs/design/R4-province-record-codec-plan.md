# R4 Province record codec — bounded plan

Review date: 2026-10-20. This is a behavior-preserving continuation of R4 save section extraction under [core architecture refactor](core-architecture-refactor.md) §7 and §9.

## Scope

- Extract the existing Province count and primary Province record fields from `Realm::serialize` / `Realm::deserialize` into stateless stream helpers.
- Preserve exact count token, record field order, values, whitespace, count cap (7), and Realm schema behavior.
- Keep `Realm` ownership, count-vs-geography validation (`regions_ + 1`), demography refresh, whole-world invariant checks, and atomic replacement.
- Court extension fields (`tax_receipts`, `food_capacity`, `teacher_skill_sum`) remain in their existing court section; do not duplicate them in this record codec.

## Files and boundaries

- Add `core/include/polyec/province_section_codec.hpp` with inline helpers in `polyec::detail`.
- Update only `core/src/realm.cpp` at the existing Province section position.
- Existing save fingerprint, roundtrip, malformed-save, and replay tests are the oracle; add a focused assertion only if review shows a gap.
- No gameplay, public API, CMake, binding, UI, action, observation, reward, or model change.

## Invariants / gates

- The section writes and reads the same seven primary Province fields plus seven stock and seven price values, in existing order.
- Count remains at most 7 here; Realm still requires exactly `regions_ + 1` after all sections are parsed.
- Run the Release full CTest suite and Debug `realm_tests|schedule_tests|manager_tests|realm_clock_replay` subset; run `git diff --check` and inspect the relevant save tokens.

## Process note

OpenCode CLI calls currently return an internal server error despite a configured OpenCode credential. This narrow infrastructure-only plan is reviewed by Codex; if the provider recovers, return implementation to OpenCode on a subsequent slice.
