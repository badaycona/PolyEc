# M5 v24 — semantic worksite action counts in sampled evaluation

## Issue and scope

`realm_coplay_v11.py` labels every sampled action slot `>=89` as a worksite action and derives authorization/submission from `(slot - 89) % 5`. Source review shows the current Realm v11 candidate table reserves slots 89–128 for eight five-action worksite groups, so the existing counts are correct under the current contract. The earlier hypothesis that marriage can appear at those slots was based on a synthetic observation that cannot be produced by Realm. Decode the selected candidate's Intent anyway to make the diagnostic track semantic actions if slot layout changes later.

## Before → after

Before: the counter is coupled to v11's fixed slot order and can silently become wrong if candidate layout changes.

After: classify `observation[STATE + action * FEATURES] * 32` as the selected C++ `Intent`; count worksite operations only for TransferEstablishmentGoods, AuthorizeWorkAtSite, and WorkAtSite. Reject malformed/nonintegral/out-of-range intent encoding. This hardens diagnostics; it corrects no known v11 full-world count or selection result. Utility, action selection, Realm transitions, and selector thresholds remain unchanged.

## Files and verification

- `ai/evaluation/realm_coplay_v11.py`: add selected-intent classifier and semantic counters.
- `ai/tests/realm_coplay_v11_checks.py`: test the actual Realm candidate layout, semantic decoding, and malformed encodings.
- Re-run three sampled v23 policies on the same 20 worlds ×128 days to compare semantic counts and confirm selector decision/utility is unchanged.
- Update evidence to correct the initial hypothesis and list source/evaluator version/hash. Do not rerun training or R1.

## Limits

This hardens evaluation instrumentation. It does not change policy performance or current v11 counts. A synthetic test that places marriage at slot 100 is not a valid Realm candidate layout; real Realm snapshots keep marriage candidates at 72–79 and worksite candidates at 89–128.
