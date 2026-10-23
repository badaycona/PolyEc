# R4 Edge record codec — approved bounded plan

Review date: 2026-10-20. This is a behavior-preserving continuation of R4 save-section extraction under [core architecture refactor](core-architecture-refactor.md) §7 (R4) and §9.

## Scope

- Extract only the serialized `edges_` count and records from `Realm::serialize` / `Realm::deserialize` into stateless helpers in `polyec::detail`.
- Preserve the exact token position/order, trailing whitespace, record values, count limit (`8,000,000`), and existing failure text.
- Keep Realm as owner of `edges_`, whole-world invariant validation, staging, and atomic live-state replacement.

## Files

- Add `core/include/polyec/edge_section_codec.hpp` with inline `write_edge_section(std::ostream&, const std::vector<Edge>&)` and `read_edge_section(std::istream&, std::vector<Edge>&)`.
- Update `core/src/realm.cpp` only at the existing Edge section and include the helper.
- No CMake, public gameplay API, bindings, UI, action, observation, reward, model, or gameplay changes.

## Invariants and failure behavior

- Write count then each `from`, `to`, `kind` in the existing order.
- Read count with the existing 8,000,000 cap; parse failure or excessive count throws `std::runtime_error("Save count limit")`.
- Do not add range checks for edge fields; current code has none.
- Read into a temporary vector. If stream extraction fails while reading records, throw the same `Save count limit` used by the immediately following legacy collection-count reader; do not publish a partial vector.
- Realm still performs later reference/invariant checks and only commits the staging Realm after full deserialization succeeds.

## Gates

- Existing fixed fingerprint and save/load/continued replay tests are the byte-order oracle.
- Build the Windows Release solution and run full CTest; build Debug and run `realm_tests|schedule_tests|manager_tests|realm_clock_replay`.
- Run `git diff --check`; preserve all unrelated dirty files. No commit/push.

## Review note

OpenCode proposed this slice plan-only. Codex accepted it with the explicit truncated-record error-preservation rule above before implementation.
