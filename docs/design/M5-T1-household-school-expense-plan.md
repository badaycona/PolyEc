# M5 T1 follow-up — household-funded study expense

## Goal and baseline

Build on Realm schema20 `HOUSEHOLD_1` and the account transfer contract in `M5-T1-household-fund-account-plan.md`. Make the required M5 transaction “household pays tuition without spending the wrong account” real. Current `available_funds()` and `spend()` include a guardian's private cash, which conflicts with IMPLEMENTATION_PLAN §2 (dependent owns property; guardian manages but does not own) and §7.

## Working rule (delegated M5 assumption)

- A person may use their own cash, then their household's common fund, for their own Study tuition and personal DryFood purchase only. The payment goes to the existing school/province or merchant/province account; it never enters the person's cash.
- The household fund is not visible as the guardian's personal funds. A guardian's private cash is unavailable unless they explicitly deposit it into the household account. General withdrawal remains restricted to household authority.
- `available_funds(actor)` means own cash plus the actor's household fund; it remains a read-only affordability projection. Exact Realm validation is authoritative at execution.
- This changes semantics of observation feature 94 with the same shape. Current realm-v6 pilot ONNX models are incompatible. Change `model_contract`, reject them in production `RealmRuntime::load_models` by default, and allow only explicit labeled development benchmarking through an override. Do not alter existing model manifests to claim compatibility.

## Files / ownership

- `core/src/realm.cpp`: affordability projection and Study/Buy cash→household fund staged spend. Realm owns both balances and commits.
- `core/include/polyec/realm.hpp`: versioned model-contract label and historical pilot compatibility result.
- `native/include/polyec/realm_runtime.hpp`, `native/realm_runtime.cpp`: explicit development-only override; normal load/restore rejects incompatible legacy pilot.
- `native/realm_benchmark.cpp`, `native/realm_contracts.cpp`: label benchmark override and prove default rejection versus explicit test/development admission.
- `core/tests/test_realm.cpp`, `core/tests/test_household_fund.cpp`: correct payer, no guardian-private fallback, affordability/action/mask, conservation, stale/reject atomicity and save continuation.
- `bindings/python/realm_binding.cpp`, `ai/tests/household_fund_binding_checks.py`: same C++ purchase/Study path and model contract projection.
- `docs/IMPLEMENTATION_PLAN.md`, CURRENT_TASK, HISTORY, report and source manifest.

## Save / API contract

No schema addition: account balances are already schema20 `HOUSEHOLD_1`; a purchase changes only the existing province account and fund/cash. Save/load mid-transaction continuation must match uninterrupted execution. No new Intent/action slot, observation shape, reward, Python economic implementation or Godot-side mutation.

## Gates

1. Numerical oracle: household fund 10, learner cash 0, tuition 5 → fund 5, Province firm +5, person cash unchanged, total money unchanged. If fund is 0 while guardian private cash is positive, Study must reject unchanged.
2. Personal cash first; if personal covers tuition, household fund does not change. If personal plus household is short, reject atomically.
3. Save/load after deposit and before Study matches uninterrupted fingerprint/ledger; schema20 remains stable.
4. Release and Debug full core CTest; native Release runtime contracts; Python Release/Debug household command checks and replay; Release Godot extension build.
5. Historical pilot compatibility guard: normal runtime load rejects, explicitly labeled development test/benchmark can load; restore respects how it was loaded. Existing model files/manifests remain untouched.
6. Scoped `git diff --check`, repository links, plan/report/hash checkpoint. Self-review limit stated.

## Not included

No general-purpose household allowance, third-party dependent expense/guardian approval, estate distribution, land/title transfer, household dissolution, action/observation shape change, new RL training, or completion claim for M5. Godot-side direct household fund UI is a later integration gate; this slice relies on the common C++ command path.
