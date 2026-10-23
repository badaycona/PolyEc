# R4 Realm primary header codec — plan-only review

Review date: 2026-10-20. This is a **plan-only** behavior-preserving R4 save-section extraction under [core architecture refactor](core-architecture-refactor.md) §7/§9. No source, build, test or implementation change is included in this document.

Review: **approved-by-Codex 2026-10-20 with conditions** — canonical fixture correction: `Draft::rerolled` is boolean and is set to `true` (serialized as `1`). — preserve the legacy two-group parse/error mapping, keep Realm domain validation and atomic commit, and implement the codec-level exact canonical byte oracle. Implement only the named files, preserve exact token order/bytes, stage all header fields in a temporary struct, publish to the Realm-staged world only after the whole header parses, and keep Realm ownership, validation and atomic load commit.

## 1. Candidate

Extract the inline **top-level primary header** from `Realm::serialize` / `Realm::deserialize` into a stateless codec. This block sits immediately after the magic/bundle token and immediately before `detail::write_person_section` / `detail::read_person_section`. It is bounded because its grammar is already fixed, contains no per-record count beyond fixed-size arrays, and is owned by `Realm`.

Covered fields, in current order:

- `day_`
- `regions_`
- `seed_`
- `next_id_`
- `next_fact_`
- `opening_money_`
- `carrier_`
- `estate_`
- `endowment_`
- `Draft`: `tier`, `seen`, `chosen`, `rerolled`, `cards[0..2]`
- `opening_[0..6]`
- `produced_[0..6]`
- `consumed_[0..6]`
- `routes_[0..6][0..6]`
- `organization_funds_[0..27]`
- `rng_`

`POLYEC_REALM_10`, the bundle string, `CLOCK_1`, and the `SCHEDULE_4` section are **not** part of this codec. `CLOCK_1` and schedule extraction remain separate, already accepted/queued R4 work.

## 2. Scope and files

- Add `core/include/polyec/realm_primary_header_codec.hpp` with:
  - `polyec::detail::RealmPrimaryHeader` (a plain staging struct mirroring the fields above)
  - `polyec::detail::write_realm_primary_header(std::ostream&, const RealmPrimaryHeader&)`
  - `polyec::detail::read_realm_primary_header_initial(std::istream&, RealmPrimaryHeader&)` — parses and stages **only** the initial group (`day_` through `endowment_`); returns `bool` and publishes to the output struct only when all 9 fields parse. On failure the output struct is unchanged and the stream is left failed.
  - `polyec::detail::read_realm_primary_header_tail(std::istream&, RealmPrimaryHeader&)` — parses and stages **only** the tail group (`Draft` through `rng_`); returns `bool` and publishes only when every field parses. On failure the output struct is unchanged and the stream is left failed.
- In `core/src/realm.cpp`, add the include and replace only the inline header write/read in `Realm::serialize` / `Realm::deserialize`, keeping the two-phase order: initial read → regions validation → tail read → (stream may be failed → `read_person_section` reports `Save count limit`).
- In `core/tests/test_realm.cpp`, add focused codec checks and retain existing Realm save/load/fingerprint tests.
- No CMake, schema, gameplay, bindings, UI, native, Godot, Python, model or archive changes.
- Do not modify `core/src/realm_court.cpp`, other codec headers, or `Realm::invariant`.

## 3. Token grammar and exact byte order

The codec writes and reads the fields in the current order, with one trailing space after each token, exactly as the inline code does today:

```text
day regions seed next_id next_fact opening_money carrier estate endowment
draft.tier draft.seen draft.chosen draft.rerolled
draft.cards[0] draft.cards[1] draft.cards[2]
opening[0..6] produced[0..6] consumed[0..6]
routes[0][0..6] ... routes[6][0..6]
organization_funds[0..27]
rng
```

Writer order remains exactly:

```cpp
out << header.day << ' ' << header.regions << ' ' << header.seed << ' '
    << header.next_id << ' ' << header.next_fact << ' ' << header.opening_money << ' '
    << header.carrier << ' ' << header.estate << ' ' << header.endowment << ' ';
out << header.draft.tier << ' ' << header.draft.seen << ' ' << header.draft.chosen << ' '
    << header.draft.rerolled << ' ';
for (auto card : header.draft.cards) out << card << ' ';
for (auto const* array : {&header.opening, &header.produced, &header.consumed})
    for (auto value : *array) out << value << ' ';
for (auto const& route : header.routes)
    for (bool open : route) out << open << ' ';
for (auto funds : header.organization_funds) out << funds << ' ';
out << header.rng << ' ';
```

No field is moved, renamed, added or removed. The trailing space after `rng_` matches the legacy writer and leaves the stream positioned for `detail::write_person_section`.

## 4. Legacy failure contract (must be preserved exactly)

The section has **no count prefix**. Fixed-size arrays are read by their compile-time sizes. No new error string or error mapping is introduced: in particular there is **no** `Save realm header`. The only caller-visible errors for this header are the legacy ones below.

The header is split into two legacy parse groups, and the codec must expose them as two staged reads so Realm can keep the original check order:

1. **Initial group — `day_` through `endowment_` (9 scalars).**  
   These are extracted in one `>>` chain and immediately checked together with the regions domain: `regions_` must be `1`, `2` or `6`. **Any** parse failure in this group **or** an invalid `regions_` surfaces as `std::runtime_error("Save geography")`. The codec's initial read therefore publishes its 9 fields only when all 9 parse; on failure it leaves the output struct unchanged and the stream failed. Realm then validates `regions_` before any tail parsing; an invalid value throws `Save geography` and the tail is never consumed.

2. **Tail group — `Draft` (`tier`, `seen`, `chosen`, `rerolled`, `cards[0..2]`), `opening_[0..6]`, `produced_[0..6]`, `consumed_[0..6]`, `routes_[0..6][0..6]`, `organization_funds_[0..27]`, and `rng_`.**  
   These have **no immediate check**. A parse failure simply leaves the stream failed; the immediately following `detail::read_person_section` then fails on its count read and reports `std::runtime_error("Save count limit")`. The codec's tail read publishes its fields only when every one of them parses; on failure it leaves the output struct unchanged and the stream failed, so the legacy `Save count limit` outcome is preserved; Realm must not throw or remap this failure before invoking `read_person_section`. `rng_` is read with `std::mt19937_64`'s stream operator; a malformed RNG state is a tail-group parse failure and therefore becomes `Save count limit`, not a new error.

- `day_` range and people-count checks stay in `Realm::deserialize` at their current final invariant point. `Draft` has no additional validation today; none is added. `regions_` domain validation stays in `Realm::deserialize` immediately after the initial read, as above.
- The codec performs no domain validation and no Realm invariant checks. It only stages fields and reports whether each group parsed.

## 5. Ownership and atomicity

- `Realm` remains the sole owner of all covered fields.
- The codec is stateless and touches only the staging struct. It does not own, transfer or replace Realm state.
- Each read function stages into a local `RealmPrimaryHeader` and publishes to the output reference only when its whole group parses. A parse failure leaves the caller's struct unchanged and the stream failed.
- `Realm::deserialize` uses a local `RealmPrimaryHeader header`, calls the initial read, validates `regions_` (throwing `Save geography` on failure or invalid regions), then calls the tail read. If the initial read returns false, Realm throws `Save geography` before validating/reading the tail. If the tail read returns false, Realm does not throw or remap; it immediately invokes `detail::read_person_section` on the still-failed stream, which reports `Save count limit`. Only after both reads succeed are fields copied from `header` into staged `Realm w`. A failed read or later section leaves `*this` unchanged because `deserialize` discards `w`.
- No transaction ID, settlement ID, schema change or gameplay state is added. No `Save realm header` string is introduced.

## 6. Tests

Codec-level checks in `core/tests/test_realm.cpp`:

- Initial-group truncation/non-numeric failure (among `day_`..`endowment_`) leaves the output struct unchanged; Realm maps it to `Save geography`.
- Tail-group truncation/non-numeric failure (in `Draft`, the accounting arrays, `routes_`, `organization_funds_`, or `rng_`) leaves the output struct unchanged and the stream failed; Realm maps it to `Save count limit`.
- Valid round trip preserves every field, including `Draft`, all three accounting arrays, `routes_`, `organization_funds_`, and `rng_`.
- `rng_` state equality after round trip.
- A parse failure does not mutate an existing output struct.
- **Exact canonical writer-byte oracle (codec-level fixture):** construct `detail::RealmPrimaryHeader` directly, assign deterministic values to every field, seed `header.rng` with `42`, call `detail::write_realm_primary_header`, and compare the complete `std::ostringstream` result byte-for-byte to the canonical expected string below. The fixture must not construct or mutate a `Realm`, must not extract a partial `Realm` serialization, and must retain the single trailing space after `rng_`. Suggested canonical field values:

```cpp
detail::RealmPrimaryHeader header{};
header.day = 1;
header.regions = 2;
header.seed = 3;
header.next_id = 4;
header.next_fact = 5;
header.opening_money = 6;
header.carrier = 7;
header.estate = 8;
header.endowment = 9;
header.draft.tier = 10;
header.draft.seen = 11;
header.draft.chosen = 12;
header.draft.rerolled = true;
header.draft.cards = {14, 15, 16};
header.opening = {17, 18, 19, 20, 21, 22, 23};
header.produced = {24, 25, 26, 27, 28, 29, 30};
header.consumed = {31, 32, 33, 34, 35, 36, 37};
for (int i = 0; i < 7; ++i)
    for (int j = 0; j < 7; ++j)
        header.routes[i][j] = (i != j);
for (int i = 0; i < 28; ++i)
    header.organization_funds[i] = 100 + i;
header.rng.seed(42);
```

Canonical expected output (one trailing space after the final RNG state token, no trailing newline):

```
1 2 3 4 5 6 7 8 9 10 11 12 1 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 0 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 0 1 1 1 1 1 1 1 0 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127 42 9039304369631583587 2283055167971690556 9177550844646897295 13550982684768831738 16380940252606110621 7381580021950463180 12486906944293397264 12541923849746012210 10593621568320655481 14703885749797857961 12900731172429026797 4909915023231560719 3491819129004011651 3236151435002971157 7265048005026587328 6440966352990231293 7368476272839262813 1568116804285402686 3308734883911680761 13923991122731584729 16229732751562319975 13168876560692131242 12392268102046795423 12390251145896938673 15658171049170258064 17548526328989462257 2447245712840183461 4121873197844147741 1558016622116830774 16084850481134434716 1256135157059011090 16731881842112727882 4201803915299664630 10740149290755620704 1802854857209971293 17282820808481312125 16689137133073719627 10507901240392372686 726453335504426243 8084085897301896879 10404249991387875007 1427836468041020003 3838041749463152786 9513904526930796502 17987061965901890673 2017508727074510392 13403981429655826439 1962219741662424081 15050422893708389934 2588463136552153371 4653342703430485746 7398299803198359019 10129959396504051799 15706240325048982055 12156777104408786059 9496403210466571085 3887069901997815324 6327070857336874790 5933690183251884822 161210316150017095 1426808349032997304 8460078168959579542 18106911104167875786 13182292611477986965 1065139866928744908 13595696492699118878 5186796912873345583 14805453030994773082 10861547215454235370 15506271316633969422 11059285686376684688 17021854235608069106 5543100395345817766 14288094800392513189 300547157182408057 3830282941087473041 11615712289761780170 1244323806969391222 16659132406351095565 14274828402241638342 653628023125278962 9507900589600389340 12724161701097480537 4791690302019502675 4891154404913026751 15060913023581536700 15992626101855178730 18006669856913564493 14246848543245264655 4889557568684265846 10752353950127201094 7235083889429645136 9957636317901125786 9376362405856975638 11442020376717307619 4172385337436462317 3631249306741860618 3136451496051174692 12567252047648197559 6754091270661790261 10256939868640126857 4196406758327576789 6306176435230382296 15645769135448884621 11269138165240556639 12626991274535006147 12463751755600864344 6033173000122042942 4566466618008833664 17200122761100561646 3197023343380093208 13616782373667705256 9136158898138771795 16940653553080146396 14268035315350252966 407508645262909813 13912059946092506374 6588622398084670807 11799841316210211477 17511776766147098115 10756278050614099577 13258884624779837721 7936062393540908858 14910414234792942299 11816345853189791861 5220985822543560297 9680352822392754375 4289383163771432481 14429787083982142286 13428658519156973579 8272826134278428952 1756878092494016745 8594182567542591610 1744253847463127845 3465762830955491592 3279375835834619632 5256658858688977081 4609272067171066082 13862061777696745029 14230220440768787674 15315272018223591858 8683655195399932843 2077412792930149233 6148514411146660461 5653268203477445005 7033085894347080238 16959322448661181398 15069542444230477829 18196506747895276451 4022580513178084278 14152616245590872469 14750982468555588086 1097981614891082666 6958104614425041788 3748646399561062548 13521743936709656224 15735208818483601687 12959715558457457954 1119619928448900927 17922116358056820659 15491043909693801105 4156192729950052940 18322441902001388543 180140457229144048 12883854554113351125 10942528246126831729 8828010763098483166 10907640046270430683 4420151587089188558 17536617827050579168 9625704226636224914 1207251011681693436 11068132455051282169 8732150094255194061 15806710427833707403 14819624155717541784 7256424075685000176 10227320712006430991 8431503602054419196 8362397798112959021 1479831632138759793 13801856796233988499 8278395645478784564 5030248212788546313 5122518077079611681 711110551738506074 4071382358437475725 1758174431726558597 296608292357696542 5603748493652780036 1918799066351308960 18151172522489466080 2699986578762604968 8277096999019463498 13838612032621414898 4031395115955375905 3390075727568291986 10212201551765352560 1644456413191443153 3369802872886361733 10428507510517414186 10867748143863620818 10987216003878447451 8550466116312870001 15233179228584599677 17726130611366656244 5668412005345938746 18443879325856487727 2927770925891256717 9561186678224271771 5913086278321506232 423839968416113497 15102197374850850426 9128681748069458715 13161403581809136233 3767161637586916007 15731027254438846772 13637836328773410949 17047708410721694102 2385744930192679693 16140728414163590438 1967199398485867359 587884480703500690 128288473857688970 9656069484589936675 11960347386902451631 16927316905379177292 97378850942510791 9405045148329560800 2814956762827901088 3887355653164541191 15104438378403232803 12944060410227176073 2970818551795832921 1097374116144918416 6056622196373409084 14808838234390452134 2822206205772547055 15682451709218915058 7048051025709175885 9580980336772662861 14036189991798214101 17264110677108332945 10136649909963058590 6569884273403308385 16395949139172189910 10411483955870515048 11275246407172078746 8585125197028364209 9122169528424239850 5398569295867898186 13708497615011362859 4255106881583560498 13596395530327098056 783994614355047553 1585784730169096877 279434325590058090 13057044518729645476 12539991729972212017 9624231129610400763 8214643653470169802 15001114300488924085 7074256657083681285 18398500904505114556 15081173105137792668 17788425372413000701 10660164351825890993 16708967032034468739 13883443920095493773 9513828425876271620 11466309414947456541 8694219242788779395 4141432904964908779 2630151784956980577 10237128161043372576 768775286244751118 8130015042709037195 7237134454058725208 10249227510932273852 301203608102950526 11197309942784997183 10595491670153755091 7598898951381752280 10332747448128418625 10577960001895186660 5875690306849080204 8020303750115529704 16122306334008645653 13221404914174174975 5874561016152055451 8142472460753471541 9747589850937802312 10498662525401246247 17611414879616091303 11601671564882537979 8948832458993786093 5098901250299685541 16957098694840556542 18219994183994077092 16091304755444873863 436998082075887201 12199841538986295099 16720906611761505588 10650818569947387099 1464078540366238294 903138242931336272 13870965860576793411 17537083584728277876 4722776545644594720 14163352717751792643 5050079075493979447 
```

Realm-level checks:

- Existing Realm save/load retains state-equivalent fingerprint behavior; exact header bytes are covered by the codec canonical oracle above.
- Corrupt-save coverage: initial-group failure or invalid `regions_` -> `Save geography`; tail-group failure -> `Save count limit`; existing corrupt-save and invariant tests remain unchanged.
- No Realm private-field construction, partial Realm serialization, new gameplay, or economic assertions are added.

## 7. Exact gates

Reuse only `build/m5-core`; do not configure or create a new build directory. Run:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Release --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Release --output-on-failure
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-core --config Debug --parallel 4
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe' --test-dir build/m5-core -C Debug -R 'realm_tests|court_tests|schedule_tests|manager_tests|realm_clock_replay' --output-on-failure
git diff --check
python scripts/check-repository-links.py
```

Record build exit codes, CTest pass counts, targeted test counts, and `git diff --check` result in the evidence report. Do not claim direct executable verification beyond the CTest gates above.

## 8. Deferred

- `CLOCK_1` extraction, if separately approved.
- `Realm::view` / RealmProjection construction boundary.
- Schema, gameplay, bindings, native/Godot/Python, model artifacts, performance, training, commit and push.
- No commit or push.
- No M5 gameplay/economic policy is invented by this slice.
