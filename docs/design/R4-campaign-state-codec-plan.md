# R4 — Campaign state codec

## Status

**Accepted by Codex** (2026-10-20) after final diff review and the approved Windows gates. The pre-change 622-byte golden payload was embedded and its SHA-256 verified before implementation edits. Evidence: [R4 Campaign state codec report](../reports/R4-campaign-state-codec-evidence-2026-10-20.md).

## Task, baseline, dirty files

- **Task:** extract only the Campaign-owned save payload — the bytes between the `POLYEC_M4_1` magic token and the trailing `std::quoted` Realm save string — from `Campaign::state()`/`Campaign::deserialize` into a stateless, header-only codec. `Campaign` keeps: the magic/schema wrapper, the quoted Realm emission and consumption, the whole-state invariant check and the final move-commit. `Realm` keeps owning and serializing its own world via `Realm::serialize`/`Realm::deserialize`; the payload codec must not see into the Realm.
- **Baseline:** current working tree with the accepted R3/T2/R4 dirty slices already in place; preserve all existing dirty state ([CURRENT_TASK](../CURRENT_TASK.md) is the authority). Run `git status` before implementation; this slice may add or modify only the files listed below.
- **Method precedent:** same slicing pattern as [R4 ScheduleBook codec](R4-schedule-section-codec-plan.md) and the R4 record/court-extension codecs, under [core architecture refactor](core-architecture-refactor.md) §7/§9 (owner keeps live state and atomic commit; stateless codecs are grammar-only staging).

## Before → after

**Before.** `core/src/campaign.cpp` lines 189–213 are fully inline:

- `Campaign::state()` writes `POLYEC_M4_1 `, seven scalars (`seed_ revision_ sequence_ next_record_ inspecting_until_ surveyed_ workshop_output_`), then routes (count + per-route `from to days capacity quoted(terrain) open claimed works_due`), cargo (count + per-cargo `id cause from to good depart arrives quantity cost delivered path_count` + path ints + `leg_starts`/`leg_ends`/`leg_arrivals` ints), cases (count + per-case `id cause region opened reported closed evidence quoted(title)`), journal (count + per-fact `id parent day region kind amount`), workers (count + ids), each token followed by a single space, then `std::quoted(world_.serialize(bundle))`. `serialize()` and `fingerprint()` delegate to `state()`.
- `Campaign::deserialize()` throws, in first-failure order: `"M4 save exceeds size limit"` (256 MiB), `"M4 schema mismatch"` (magic), then inline payload parsing with `"M4 save record bound"` (the `size` lambda: `!s || n>maximum`, used for routes 9, cargo 100000, cargo path 7, cases 100000, journal 1000000, workers 6), `"M4 path missing"` (`path.size()<2`), `"M4 manifest invalid"` (`good<0||good>6||quantity<1||quantity>5000`), then `"M4 save incomplete"` (`!s` after reading the quoted Realm string), `"M4 save trailing data"` (`s>>std::ws && !s.eof()`), then `"M4 world/accounting invalid: "+error` (world deserialize failure and again for `invariant()` failure). Derived `std::array<int64_t,7> bonded` totals are accumulated per cargo record, after that record's manifest check and only `if(!c.delivered) bonded[c.good]+=c.quantity`, in cargo write order. Stages everything into local `restored`, pauses it if `interactive`, runs `restored.invariant()`, and commits with `*this = std::move(restored)` only at the end.

**After.**

- New header-only stateless codec owns exactly the payload grammar described above (scalars + five groups + trailing space), as bytes on a stream.
- `Campaign::state()` keeps emitting `"POLYEC_M4_1 "`, then calls the codec writer for the payload, then keeps `s<<std::quoted(world_.serialize(bundle));` unchanged (the existing `bundle` argument stays in Campaign code).
- `Campaign::deserialize()` keeps the size-limit check and magic check (wrapper), then calls the codec reader for the payload, then keeps everything from `s>>std::quoted(realm); if(!s) throw "M4 save incomplete"; s>>std::ws; if(!s.eof()) throw "M4 save trailing data";` onward unchanged: `restored.world_.deserialize(realm,bundle,error,bonded)` receives the codec-derived bonded totals, interactive `restored.pause()`, `restored.invariant()`, `*this=std::move(restored)` move-commit, and the `catch(const std::exception&)` mapping `error=e.what()`.
- No observable byte, error string, ordering, or gameplay change. No save version/schema/API/gameplay/binding/UI/RL/model change.

## Files and interfaces

- **New** `core/include/polyec/campaign_state_codec.hpp` — header-only, stateless, namespace `polyec::detail`:

```cpp
#include "polyec/campaign.hpp"   // MapRoute, Cargo, Case; realm.hpp supplies Fact
struct CampaignStatePayload {
    uint64_t seed=0, revision=0, sequence=1, next_record=1;
    int inspecting_until=0;
    bool surveyed=false;
    int64_t workshop_output=0;
    std::vector<MapRoute> routes;
    std::vector<Cargo> cargo;
    std::vector<Case> cases;
    std::vector<Fact> journal;
    std::vector<uint64_t> workers;
    std::array<int64_t,7> bonded_goods{};   // reader-derived output only
};
void write_campaign_state(std::ostream& out, const CampaignStatePayload& payload);
void read_campaign_state(std::istream& in, CampaignStatePayload& out);
```

- `read_campaign_state` throws `std::runtime_error` with exactly `"M4 save record bound"`, `"M4 path missing"` or `"M4 manifest invalid"` (the same exception flow the inline code used; `Campaign::deserialize`'s existing catch-all remains the only translation point into the `error` out-parameter). It stages into a local payload and writes `out` only on full payload success.
- **Modified** `core/src/campaign.cpp` — only `state()` and `deserialize()` as above; `save()`, `load()`, `serialize()`, `fingerprint()` untouched.
- **Modified** `core/tests/test_campaign.cpp` — appended fixtures only (below); existing tests preserved verbatim.
- **Untouched:** `core/include/polyec/campaign.hpp` (class API unchanged; the payload type lives in the codec header), all CMake files (header-only; existing include paths suffice — no new target or build directory), `campaign_state`-adjacent Python/native/Godot bindings, all other core files.

## Byte grammar contract (both directions pin these)

Groups appear in this exact order, every token separated by exactly one space, and the payload ends with one trailing space immediately before the wrapper's quoted Realm string:

1. Scalars: `seed revision sequence next_record inspecting_until surveyed workshop_output` (bool as the default unquoted `0`/`1`; no `boolalpha` anywhere).
2. Routes: `routes.size()` then per route `from to days capacity std::quoted(terrain) open claimed works_due`.
3. Cargo: `cargo.size()` then per cargo `id cause from to good depart arrives quantity cost delivered path.size()`, then `path` ints, then leg ints in the fixed triple order `leg_starts`, `leg_ends`, `leg_arrivals` (each of length `path.size()-1`).
4. Cases: `cases.size()` then per case `id cause region opened reported closed evidence std::quoted(title)`.
5. Journal: `journal.size()` then per fact `id parent day region kind amount`.
6. Workers: `workers.size()` then the ids.

The writer is a pure emitter: no reordering, no normalization, no range enforcement, no validation, no count-summing beyond emitting the container sizes as-is. The reader uses the same stream expressions in the same order as the current inline code (`s>>` field by field, `std::quoted` for `terrain`/`title`, plain `bool` extraction for `delivered`/`open`/`claimed`/`evidence`/`surveyed`, `size_t` count tokens) so that stream-state behavior — failbit stickiness, whitespace skipping, `eof` — is bit-compatible with today.

## Limits, errors and first-failure ordering (must not change)

| Check | Message (verbatim) | Owner after refactor |
|---|---|---|
| Input/file > 256 MiB | `M4 save exceeds size limit` | Campaign wrapper (deserialize + load) |
| Magic token != `POLYEC_M4_1` | `M4 schema mismatch` | Campaign wrapper |
| Any `size` token unreadable or over its bound (routes 9, cargo 100000, cargo path 7, cases 100000, journal 1000000, workers 6) — including any earlier scalar/group extraction failure surfacing at the next `size()` call | `M4 save record bound` | Codec |
| Cargo `path.size() < 2` after reading path | `M4 path missing` | Codec |
| Cargo `good` outside `[0,6]` or `quantity` outside `[1,5000]` | `M4 manifest invalid` | Codec |
| Quoted Realm string unreadable/absent | `M4 save incomplete` | Campaign wrapper |
| Non-whitespace tail after quoted Realm | `M4 save trailing data` | Campaign wrapper |
| `world_.deserialize` false, or `restored.invariant()` false | `M4 world/accounting invalid: <reason>` | Campaign wrapper |

Ordering pinpoints to preserve exactly:

- Per-cargo, in cargo order: path-count bound → path read → `M4 path missing` → leg read → `M4 manifest invalid` → (only then) bonded accumulation `if(!c.delivered) bonded[c.good]+=c.quantity`. Bonded totals are returned derived from the unmutated payload — no validation is added and no arithmetic rules change (sums stay plain `int64_t` `+=`, overflow semantics as today).
- Any malformed/truncated token inside a group surfaces at the next count token as `M4 save record bound` (failbit stickiness); a truncated worker-id tail surfaces at the quoted Realm read as `M4 save incomplete`. The codec must not add a success-side `!s` check of its own at the end of the payload — that behavior belongs to the wrapper's Realm-quote read.
- The reader leaves the stream positioned at the whitespace immediately preceding the quoted Realm token; it must not consume the Realm quote, not call `Realm::deserialize` (or anything else on the streamed suffix), not call `invariant()`, and not touch any live `Campaign`: it stages locally and publishes to `out` only on full payload success. All range/ID/consent-style checks stay in `Campaign::invariant()` and `Realm`.
- Wrapper responsibilities stay in `Campaign::deserialize`: size limit, magic check, Realm quote + incomplete/trailing checks, `world_.deserialize(...,bonded)`, interactive pause, whole-state `invariant()`, `error` mapping through the single catch, and the final move-commit.

## Test additions (core/tests/test_campaign.cpp only)

Follow the existing harness (`check()` counters, no `assert`, Release/NDEBUG-safe). All existing tests — controller/command rejections, save/restore roundtrips, mid-run continuation determinism, corrupt checksum, failed interactive restore, file `.bak` handling, jammed reservation, stress seeds, 10k profile — remain unchanged and must still pass.

1. **Byte-exact canonical codec writer fixture.** The golden was captured before any source edit from the unchanged implementation using the canonical fixture `Campaign f(37,256);`, then `Weave` target `2` and `Relief` target `1`, quantity `200`, through the existing test helper's command semantics. The captured payload is **622 bytes** with SHA-256 `7136e667aeb2c5c6d42e028e338beda1dece03c7d36aed01ca2ea2f51a722cb4`. The probe confirmed the 12-byte prefix `POLYEC_M4_1 `, a unique quoted `POLYEC_REALM_10` suffix beginning at full-string offset `634`, and exactly one separator space at the payload end. Before editing any source/test file, OpenCode must copy the exact captured bytes from `C:\Users\Hi\AppData\Local\Temp\opencode\r4probe\r4_payload.bin` into a fixed UTF-8 C++ test literal and verify that literal's bytes hash to the value above; do not derive the golden from post-refactor output. Populate a `CampaignStatePayload` by parsing `full.substr(12)` with the codec, then assert direct writer emission equals the fixed literal. Also parse the full pre-change fixture through the codec from offset `12`, use its `tellg()` position plus the one preserved trailing separator to select the payload slice, and assert that slice equals the fixed literal; parse the remaining bytes with `std::quoted` and confirm they contain exactly one Realm token and no trailing data. This pins both the original Campaign output and the standalone writer without a circular `write_len` oracle.
2. **Staged roundtrip + bonded totals:** run `read_campaign_state` on the payload slice; assert staged scalars/vectors match the canonical fixture, bonded totals equal the manual per-undelivered-cargo sum (`bonded_good == quantity` for the fixture's single cargo, all other slots 0), and re-emitting `write_campaign_state(staged)` equals the original payload bytes.
3. **Malformed/truncated values and counts (all through `Campaign::deserialize`, each asserting exact error string and `fingerprint()==before` on the destination `Campaign`, with a pre-paused destination also asserting its pause state is unchanged):** self-contained literal saves (no dependency on the canonical fixture's Realm tail): per-group count over-limit (`10` routes; `100001` cargo, cases, journal; `7` workers) → `M4 save record bound`; truncated scalar tail → `M4 save record bound`; cargo path count `0`/`1` → `M4 path missing`; cargo `good=7` and `quantity=0` → `M4 manifest invalid`; payload truncated inside a route record and a cargo record → `M4 save record bound`; workers count `2` with only one id → `M4 save incomplete`; valid payload + `+" junk"` → `M4 save trailing data` (existing test already covers; keep).
4. **Malformed Realm suffix through staged-payload splicing:** compute suffix boundaries arithmetically (`full = "POLYEC_M4_1 " + payload + quoted_realm`, no scanning), then build `input = "POLYEC_M4_1 " + write_campaign_state(valid_payload) + quoted_alternate` where `quoted_alternate` is (a) a garbage body (e.g. one invalid token) → deserialize fails with error starting with `M4 world/accounting invalid` and destination fingerprint unchanged; (b) no quoted Realm at all → `M4 save incomplete`; (c) a mutated payload (path count 0) whose Realm suffix is valid → still `M4 path missing` (proves the codec error outranks the suffix, first-failure order). All variants: destination unchanged.

## Windows-only gates (existing scope, reuse without reconfiguration)

- Reuse `build/m5-core` (Visual Studio, Release + Debug configurations already configured per R3/R4 slices; do not reconfigure and do not create a new build directory).
- Release: full build exit 0; full CTest 10/10; direct campaign test executable (`polyec_campaign_tests.exe`; verify the concrete name via `ctest -N -R campaign` first if it differs) exit 0, target == the fixture count plus all existing checks.
- Debug: full build exit 0; targeted CTest `campaign_tests|realm_tests|court_tests|schedule_tests|realm_clock_tests` 5/5.
- Scoped `git diff --check` (`core/src/campaign.cpp`, `core/tests/test_campaign.cpp`, new header) exit 0; CRLF warnings are acceptable if pre-existing style matches.
- `python scripts/check-repository-links.py` exit 0 from the repo root (this plan file references existing sibling docs only).
- Windows-only per the 2026-10-19 user decision: no Linux/native/Godot/Python gates for this slice.

## Risks, limits and non-goals

- **Extraction fidelity:** the payload encoding leans on default iostream behavior (bool `0`/`1`, `std::quoted` escaping, failbit stickiness for first-failure ordering). Any stylistic "clean-up" of the extraction code can silently change bytes or error mapping; the golden fixture and the error matrix above exist to catch exactly that. The fixed literal must come from the recorded pre-change capture and match SHA-256 `7136e667aeb2c5c6d42e028e338beda1dece03c7d36aed01ca2ea2f51a722cb4`; do not regenerate it from the refactored writer.
- **Transient cost:** `state()` now copies the five payload vectors into the staging payload, and the reader stages a full payload before it is applied to the local `restored` Campaign — bounded by the existing save limits, serialization-path only; no perf gate is planned for this slice.
- **No behavior upgrades:** the codec must not gain validation, migration, schema knowledge, or Realm/Population awareness; bonded totals are the only derived value. Report any need for such logic back to Codex rather than extending this contract.
- **Out of scope / deferred (unchanged disposition):** no save version or `POLYEC_M4_*` schema change, no gameplay, command, action/observation/reward, model, binding, UI change; Establishment/worksite ownership campaign-side stays deferred; no app/Godot/native/Python/Linux gates; no training; no benchmark/profile; no commit/push/publish. R4/M5 remain open.

## Gate summary (to run at implementation, not yet executed)

1. Copy the already captured pre-change payload into the fixed `test_campaign.cpp` literal and verify its 622 bytes match the recorded SHA-256 before any source edits.
2. Extract codec header; rewire `state()`/`deserialize()`; nothing else.
3. Release full build + full CTest; direct campaign exe; Debug build + targeted CTest 5/5; `git diff --check`; link checker.
4. Report evidence (executed commands/output) to Codex for diff review; only Codex acceptance closes this slice and updates `CURRENT_TASK`/evidence.

## Review

Accepted by Codex on 2026-10-20 after final diff review and the Release/Debug gates in the linked evidence report.
