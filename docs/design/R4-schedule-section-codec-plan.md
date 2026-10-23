# R4 — ScheduleBook save section codec

## Status

Accepted by Codex on 2026-10-20 after diff review and Windows Release/Debug gates. The first OpenCode code pass had incorrect legacy count parsing and omitted the golden fixture; Codex repaired the bounded implementation under the explicit exception recorded in [R4 evidence](../reports/R4-schedule-section-codec-evidence-2026-10-20.md). This is a save/data separation refactor under [core architecture refactor](core-architecture-refactor.md) §7/§9. It does not add gameplay or Establishment rules.

## Before / after

`Realm::serialize` and `Realm::deserialize` currently contain the complete ScheduleBook section inline in `core/src/realm.cpp`. Extract that section grammar to a stateless codec while `Realm` remains the only owner of the live `ScheduleBook` and the whole-world commit boundary.

The latest writer remains `POLYEC_REALM_10` / `SCHEDULE_4`. The codec must emit byte-identical ScheduleBook text and read existing `SCHEDULE_1`/`2`/`3`/`4` formats selected by Realm schema 7/8/9/10. Do not bump either schema or change any field, count, order, spacing, migration result, or runtime behavior.

## Files and interfaces

- Add header-only `core/include/polyec/schedule_section_codec.hpp` with:

  ```cpp
  void write_schedule_section(std::ostream& out, const ScheduleBook& book);
  bool read_schedule_section(std::istream& in, int realm_schema,
                             size_t people_count, int regions, int realm_day,
                             ScheduleBook& out, std::string& error);
  ```

- Change only the ScheduleBook serialization block in `core/src/realm.cpp`:
  - `Realm::serialize` calls the writer after `CLOCK_1`; the writer owns the complete `SCHEDULE_4` tag, header/counts and records.
  - `Realm::deserialize` calls the reader at the current stream position after `CLOCK_1`, passing the staged world's person count, region count and day. The reader consumes exactly the ScheduleBook grammar; counts delimit its records and it leaves later data/EOF handling to Realm.
  - Realm continues remaining whole-world checks and assigns `*this = std::move(w)` only at the existing successful end.
- Add the canonical byte fixture to `core/tests/test_schedule.cpp`. No new target or CMake directory is needed.

## Ownership and validation

The codec owns no state and gets no mutable Realm/Person/Province references. It reads/writes values in the ScheduleBook grammar only. `Realm` retains `schedule_book_`, world-level validation, `fingerprint`, stream-tail validation and atomic load commit.

`read_schedule_section` parses into a local `ScheduleBook` and copies/moves it to `out` only after the entire section succeeds. It preserves the current schema-specific defaults and validates the existing section rules: expected tag; flags and header ranges; schema-dependent counts; section record counts; unique session IDs; actor/session/kind/region/range/counter validity; attendance/settlement limits and relationships; regional key bounds, uniqueness, positive values, attendance sums and paid-unit floors; timeline-off empty-map rules; and complete input for every record. Realm keeps day/cursor consistency, people/province count, remaining stream checks, and `invariant(bonded_goods)` before commit. Any parse or validation failure returns false with an error, and `Realm::deserialize` returns false without mutating the live Realm.

Migration behavior is unchanged:

- Realm 7 / `SCHEDULE_1`: Generic sessions with default region `-1`; no attendance, settlement or regional maps.
- Realm 8 / `SCHEDULE_2`: typed sessions and legacy global attendance; settlement and regional maps empty.
- Realm 9 / `SCHEDULE_3`: typed sessions, attendance and settled-day/units; regional maps empty.
- Realm 10 / `SCHEDULE_4`: full current ScheduleBook including regional ticks and paid watermarks.

The helper must preserve legacy global attendance and settlement exactly and must not attribute them retroactively.

## Test oracle and gates

Before the refactor, the current Release Python binding produced this deterministic ScheduleBook section for `Realm(7,2,128)`, Work(actor 1, region 2, `[0,64)`, rate 5), Generic(actor 2, `[0,32)`, rate 3), then `advance_ticks(64)`:

```text
 SCHEDULE_4 1 64 3 1 2 1 1 1 1 1 1 0 64 0 5 320 320 0 0 0 0 0 0 1 2 2 2 0 32 0 3 96 96 0 0 0 0 0 0 0 -1 1 64 1 4 1 2 64 1 2 4 
```

Its SHA-256 is `84d0be99d42af39dfdeeb2a5a732417feedabd5bb0f19bfd0b7997610e2bd87b`. Add a C++ fixture that constructs the same world, extracts the substring beginning at ` SCHEDULE_4 ` from `Realm::serialize`, and compares the exact bytes above. The fixture runs after extraction and guards the pre-change canonical output.

Retain and run existing tests for Realm 7/8/9 migration, v10 round-trip, mid-day continuation, malformed counts/IDs/regions/maps, atomic failed deserialize, C++/Python serialized replay parity, and schedule invariants. No new gameplay tests are required.

Use only the existing Windows VS 18 directories and executable paths recorded in [T2.3 evidence](../reports/M5-T2.3-work-pay-settlement-evidence-2026-10-20.md): Release full build and CTest at `build/m5-core`; Debug full build and targeted CTest `realm_tests|schedule_tests|manager_tests|realm_clock_replay`; Python/binding replay only if changed paths require it. Run `git diff --check`. Do not configure a build, choose another generator, or create a new build directory.

## Deferred

Establishment/worksite ownership and lifecycle, worker contracts, production, materials, recipes, account migration, commands/actions, bindings/views and policy inputs remain deferred until their gameplay/ownership contract is reviewed. The codec does not add a manager or a second world owner. No commit, push, publish, training, or platform expansion.
