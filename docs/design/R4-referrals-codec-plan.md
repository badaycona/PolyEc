# R4 referrals codec — plan-only review

Review date: 2026-10-20. This is a **plan-only** behavior-preserving R4 save-section extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9. No source, build, test or implementation change is included in this document. This revision supersedes the earlier referrals draft after review: it removes the incorrect implication that existing tests already cover referrals validation, makes the Realm staging/validation order explicit, fixes the test API, and records the intended Realm-level malformed-save fixtures.

Review: **approved-by-Codex 2026-10-20 with condition** — implement only the named files, preserve exact token order/bytes, codec stages output into a local vector, Realm validates each ID `1..people_.size()` before moving into `referrals_`, codec count cap before allocation, parse/count failure is `std::runtime_error("Save court fields")`, no gameplay/schema changes.

## 1. Candidate

Extract only the **referrals count plus IDs** from `Realm::write_court` / `Realm::read_court`:

- `referrals_` count (`std::vector<uint64_t>::size_type`, serialized as `size_t`)
- `referrals_` IDs (`uint64_t` each)

This block sits immediately after the seven court scalars and immediately before the Investigation section. It is bounded because it is exactly one count prefix followed by that many ID tokens, already owned by `Realm`, with no per-record struct beyond the ID. Referrals are person IDs; `Realm` continues to own the vector and all domain validation of those IDs. Court scalars and every later court block remain untouched and are **not** part of this codec.

## 2. Scope and files

- Add `core/include/polyec/referrals_section_codec.hpp` with:
  - `polyec::detail::write_referrals_section(std::ostream&, const std::vector<uint64_t>& referrals)`
  - `polyec::detail::read_referrals_section(std::istream&, std::vector<uint64_t>& referrals, size_t people_size)`
- In `core/src/realm_court.cpp`, add the include and replace only the referrals count/ID write and read in `write_court` / `read_court`.
- In `core/tests/test_realm_court.cpp`, add focused codec checks and, if feasible in the existing harness, Realm-level malformed-save checks; keep existing court roundtrip and augment tests.
- No CMake, schema, gameplay, bindings, UI, native, Godot, Python, model or archive changes.
- Do not modify `core/src/realm.cpp`, `core/include/polyec/realm.hpp`, or any other codec header.

## 3. Token grammar and exact byte order

The codec writes and reads the referrals count and IDs in their current order, with **one count prefix**:

```text
<count> (<id>)*
```

Writer order remains exactly:

```cpp
out << referrals.size() << ' ';
for (auto id : referrals) out << id << ' ';
```

The section is the **second** block of `write_court`, immediately after court scalars and immediately before Investigation. No field is moved, renamed, added or removed. The trailing space after the last ID matches the legacy writer.

## 4. Legacy failure contract

- The section has **one count prefix**.
- Any non-numeric or truncated count, or a count greater than `people_.size()`, causes the read to fail and is surfaced by `Realm::deserialize` as `std::runtime_error("Save court fields")`. The codec must throw exactly `std::runtime_error("Save court fields")` for every count parse failure and for excessive count. It must **not** throw `Save count limit`.
- Any truncated ID stream (fewer ID tokens than the count promises) causes the read to fail and is surfaced as `std::runtime_error("Save court fields")`.
- **Allocation safety / count cap:** the codec enforces the count against `people_size` (the number of people in the Realm) before allocating the staged vector. This preserves the legacy guard `n > people_.size()` and prevents allocating a vector larger than the people vector. `people_size` is an input bound only: it is a parameter to `read_referrals_section`, it is not a parameter to `write_referrals_section`, and it is never mutated by either function.
- **Per-ID domain validation stays in Realm:** the codec does **not** reject `id == 0` or `id > people_.size()`. Those checks remain in `Realm::read_court` immediately after the codec read and before Investigation is read, exactly as today. This keeps the codec limited to allocation-safe count enforcement plus parse, while `Realm` retains ownership and domain validation.
- **Staged parse in the codec:** the codec reads the count and all IDs into a local vector first and publishes that vector to the output reference only after every ID has parsed. A count or parse failure throws before assignment, so the caller's vector remains unchanged. On a successful codec read, the output vector may contain IDs that are invalid under Realm's domain rules; the codec does not treat those as parse failures.
- **Staged validation in Realm:** `Realm::read_court` calls the codec into a local `std::vector<uint64_t> staged_referrals`, validates every ID as `1 <= id <= people_.size()`, and only then moves the staged vector into `referrals_`. This prevents assignment before domain checks and leaves `referrals_` unchanged on an ID-domain failure.
- **Truncation is not field-boundary detection.** This section is unframed beyond its count prefix: the count tells how many IDs follow, but the helper cannot tell whether trailing bytes belong to this section or to Investigation. On a truncated stream the `>>` extraction fails; the helper only detects extraction failure (EOF or non-numeric token). This matches the legacy unframed extraction behavior. The plan and tests must not claim field-boundary or section-boundary detection, and must not add framing to create it.

## 5. Ownership and atomicity

- `Realm` remains the sole owner of `referrals_`.
- The codec is stateless and touches only the referrals vector. It does not own, transfer or replace Realm state.
- `read_court` remains a staging operation inside `deserialize`: a failed codec read or a failed ID validation leaves `*this` unchanged because `deserialize` discards the temporary `Realm w`. In addition, the codec's own staging leaves the caller's vector unchanged on a parse/count failure, and `Realm`'s post-codec ID validation leaves `referrals_` unchanged on an ID-domain failure.
- No transaction ID, settlement ID, schema change or gameplay state is added.

## 6. Tests

The test API is exact. The codec header exposes only:

```cpp
namespace polyec::detail {
void write_referrals_section(std::ostream& out,
                            const std::vector<uint64_t>& referrals);
void read_referrals_section(std::istream& in,
                           std::vector<uint64_t>& referrals,
                           size_t people_size);
}
```

`people_size` is a read-only parameter to `read_referrals_section` only.

Add focused checks in `core/tests/test_realm_court.cpp`, modeled on the accepted Investigation block but adapted to a count-plus-ID vector and a `people_size` parameter:

Codec-level checks:

- Excessive count: with `people_size == 2`, input `"3 1 2 3"` throws `Save court fields` and preserves the output vector and its size/contents.
- Bad/non-numeric count: with `people_size == 2`, input `"not-a-number"` throws `Save court fields` and preserves output.
- Truncated ID stream: with `people_size` large enough, input `"3 7 8"` (count promises three IDs but only two are present) throws `Save court fields` and preserves output.
- Codec-level ID non-validation: with `people_size == 3`, input `"2 0 4"` parses successfully into `{0, 4}` without throwing. This confirms that `0` and out-of-range IDs are not codec parse failures.
- Exact writer bytes for a canonical referrals vector. Canonical values: `{7, 8, 9}`; expected writer string: `3 7 8 9 `.
- Valid round trip preserves count and IDs in order.
- Input order is preserved for multiple IDs.

Realm-level checks, if feasible in the existing test harness:

- Build a base Realm using the existing augment path (`fixture(6)` or `fixture(11)`), serialize it with bundle `"court"`, mutate one referrals ID token in the serialized court referrals block to `0`, deserialize into a destination Realm, and verify that deserialization rejects with `Save court fields` and leaves the destination Realm fingerprint unchanged.
- Repeat the same fixture with one referrals ID token mutated to `people_.size() + 1` (for the 256-person fixture, `257`), and verify the same rejection and unchanged destination fingerprint.
- These fixtures are new. No existing test explicitly targets referrals parsing or referrals ID validation; the augment (`card == 6` / `card == 11`) and appointment-consumption paths provide behavior context but do not directly test malformed referrals.

If the existing harness cannot reliably isolate the referrals ID token in the unframed court byte stream, the implementation must record that limitation in the evidence report instead of weakening the codec-level checks or claiming coverage that was not run.

Existing full Realm court roundtrip and augment tests remain the end-to-end oracle for successful saves.

## 7. Exact gates

Reuse only `build/m5-core`; do not configure or create a new build directory. Run:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Release --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Release --output-on-failure
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Debug --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Debug -R 'court_tests|realm_tests|schedule_tests|manager_tests|realm_clock_replay' --output-on-failure
git diff --check
python scripts/check-repository-links.py
```

Record build exit codes, CTest pass counts, targeted test counts, and `git diff --check` result in the evidence report. Do not claim direct executable verification beyond the CTest gates above.

## 8. Deferred

- Other remaining court extension refactors.
- Schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
- No commit or push.
filePath">
C:\prc\PolyEc\docs\design\R4-referrals-codec-plan.md