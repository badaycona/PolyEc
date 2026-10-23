# M5 gameplay configuration loader — bounded plan

Contract reviewed by Codex. The 2026-10-20 follow-up review added the partial-wage and default-capacity rules below before wiring the config into Realm. Acceptance and source checkpoint: [config evidence](../reports/M5-gameplay-config-evidence-2026-10-20.md); current task status remains in CURRENT_TASK.

## Goal and boundary

Load an immutable, versioned gameplay snapshot before creating a Realm, use that snapshot for the existing work-unit, work-wage, default Establishment capacity, and three starter recipe quantities, and retain the exact snapshot in saves. Python must construct a Realm from the same C++ snapshot. This slice establishes the core/save/Python contract. Godot session selection and actual UI interaction stay as the next integration gate; this plan does not claim end-to-end M5 completion.

Keep the current default behavior: 16 ticks per work unit, work wage 1 money unit per work unit, default Establishment capacity 1 batch per day, and each existing recipe consumes/produces one unit of its fixed GoodId. Recipe identities, ownership, transaction behavior, retry/forfeit policy, stable IDs, and invariants are not configurable.

## Format decision

The bounded source survey found no TOML or JSON parser target in `core/CMakeLists.txt` or the root CMake dependency declarations. Do not add network-fetched or platform-specific dependencies. Use a strict TOML v1 subset for integer-only tables and key/value assignments: blank lines and comments are allowed; decimal signed integer literals are allowed; strings, floats, arrays, inline tables, and implicit type conversions are rejected. All accepted files remain valid TOML. The parser recognizes only the documented schema keys, rejects duplicate/unknown/missing keys with a field path, and parses into a temporary before validation/publication.

The top-level `schema_version` is 1. Fields are grouped under `[work]`, `[establishment]`, and `[recipes.cloth]`, `[recipes.tools]`, `[recipes.military_supply]`. Each work field is an integer count with explicit units. Recipe GoodIds are fixed by the source contract and are not user-editable; each recipe exposes its integer input amount(s) and output amount.

## Fields and validation

| TOML path | Type / unit | Default | Initial valid range |
|---|---|---:|---:|
| `work.ticks_per_unit` | positive integer game ticks | 16 | 1–64 |
| `work.wage_per_unit` | integer money units | 1 | 1–1,000,000 |
| `establishment.default_capacity` | integer complete batches per day | 1 | 1–1,000 |
| `recipes.cloth.fiber_input` | integer Fiber units per batch | 1 | 1–1,000,000 |
| `recipes.cloth.cloth_output` | integer Cloth units per batch | 1 | 1–1,000,000 |
| `recipes.tools.iron_input` | integer Iron units per batch | 1 | 1–1,000,000 |
| `recipes.tools.fuel_input` | integer Fuel units per batch | 1 | 1–1,000,000 |
| `recipes.tools.tools_output` | integer Tools units per batch | 1 | 1–1,000,000 |
| `recipes.military_supply.tools_input` | integer Tools units per batch | 1 | 1–1,000,000 |
| `recipes.military_supply.cloth_input` | integer Cloth units per batch | 1 | 1–1,000,000 |
| `recipes.military_supply.military_supply_output` | integer MilitarySupply units per batch | 1 | 1–1,000,000 |

Arithmetic uses checked integer multiplication/addition before stock or ledger mutation. Changing `ticks_per_unit` affects work settlement, payroll eligibility, and production eligibility consistently for a Realm. A Realm's snapshot is immutable for its lifetime.

## Save and API contract

- Add a `GameplayConfig` value type with default snapshot v1, validated construction, canonical serialization, and a deterministic fingerprint of the normalized field sequence.
- Add a file-loading API that returns a complete validated snapshot or a structured error; no global mutable current config.
- A Realm constructor accepts a snapshot, while existing constructors use the frozen historical default v1.
- Bump Realm save schema to 15 and add a versioned config section containing the normalized snapshot and fingerprint. Loading v15 validates both and uses the saved snapshot without consulting the external file. Schemas 7–14 load the frozen v1 defaults. Round-trip output for a Realm uses its snapshot.
- Python Realm creation accepts an optional config path and invokes the same C++ parser; expose the snapshot values and fingerprint read-only. No Python-side parsing or duplicated gameplay rules.
- Existing Campaign construction remains on historical defaults in this slice; carrying a selected config into Campaign is a later explicit integration decision.

### Reviewed boundary corrections, 2026-10-20

For wages above one money unit, FinanceManager's pro-rata allocation may pay part of a work unit. Retain integer completed paid units and add a sparse actor/region money remainder with `0 < remainder < wage_per_unit`. The next claim subtracts that already-paid remainder; whole units advance only when accumulated gross money reaches the configured wage. Save this mutable state in a separate `WORKPAY_1` section after the normalized `CONFIG_1` snapshot, not inside the snapshot or fingerprint. Validate it against eligible attendance and paid units before loading/committing. Historical saves have no remainder. Preserve the existing retry/death rules and FinanceManager tax/conservation semantics.

Append config/workpay sections after Establishments for schema 15. Parse the snapshot before schedule validation so custom work-unit denominators are checked consistently, then validate the entire temporary Realm. Add a trailing optional denominator parameter to the schedule reader; historical callers keep 16. EstablishmentCommand uses capacity `-1` as an explicit request for the snapshot default; capacity 0 and other invalid values remain rejects. Explicit positive capacities retain their current meaning. A nondefault config is incompatible with the old pilot model until the model gate supports it.

The implementation boundary includes a small rejection guard in `native/realm_runtime.cpp` and a focused `native/realm_contracts.cpp` check: an old model runtime must reject a saved nondefault snapshot before replacing its live world. This does not enable configurable native gameplay or certify model quality. Validation order retains existing header/clock error messages before config/schedule validation. A historical payroll fixture also requires its Province stride corrected from the obsolete 26-token layout to the canonical 66-token layout.

`native/realm_benchmark.cpp` also records the running world's config fingerprint in its JSON output. A bounded 32-day default-config pilot run verifies this reporting path; it is not the full M5 performance/quality gate and does not include rendering or saving.

## Bounded implementation files

1. New `core/include/polyec/gameplay_config.hpp` and `core/src/gameplay_config.cpp`: typed fields, frozen defaults, parser, validation, canonical bytes, and fingerprint.
2. `core/include/polyec/schedule.hpp`, `core/src/realm.cpp`, `core/include/polyec/production_manager.hpp`, and Establishment creation: use the Realm snapshot for work units, wages, recipe quantities, and default capacity; preserve current defaults exactly.
3. `core/include/polyec/realm.hpp`, save section codec, `core/src/realm.cpp`: Realm-owned immutable snapshot, schema 15 read/write, transactional load, v7–v14 default migration.
4. `bindings/python/realm_binding.cpp` and `core/tests`: optional config file at Python Realm construction; tests for defaults, modified hand-calculated output/pay/attendance, parse/validation rejection before Realm creation, modified external file after save, same-snapshot replay, and legacy schema migration.
5. Update `docs/IMPLEMENTATION_PLAN.md`, the relevant core class reference, and task/evidence docs after implementation review.

No action/observation/reward/model shape change is included. Godot file selection, native configuration path, runtime interaction, model compatibility, and player-facing tuning UI are separate reviewed slices.

## Gate commands and review

Use the existing Windows `build/m5-core` Release and Debug builds and full CTest. Build Python bindings and run Release/Debug C++/Python replay parity with the same modified config snapshot. Add focused config parser/config snapshot tests to the relevant core target. Run scoped `git diff --check` and `python scripts/check-repository-links.py`.

This plan chooses an integer-only TOML subset because no reusable core parser dependency is declared. Review confirmed the listed files cover every current compile-time consumer of work-unit, wage, capacity, and recipe values. Before code, recheck the consumer list against source and preserve the versioned default/save contract; if the bounded TOML grammar must expand, revise this plan before implementation.
