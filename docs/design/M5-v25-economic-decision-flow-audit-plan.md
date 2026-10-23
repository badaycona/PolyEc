# M5 v25 — Economic decision cash and debt flow audit

## Question

The v23 sampled holdouts show workshops and frequent purchases, but little paid site work, lower cash, and higher debt. Identify which selected Economic intents directly change the acting person's cash or debt, using the same sampled full-world Realm evaluator and C++ action path.

## Scope

- Extend `run_variant` diagnostics with counts and immediate cash/debt deltas by selected `Intent`, measured around the actual accepted `world.step`; split purchases by canonical GoodId and establishment creation by industry.
- Add a standalone audit runner for the three v23 Economic checkpoints and the same 20 validation worlds. Keep paired RNG seeds, policies, population, horizon, and action logic unchanged.
- Report direct command effects separately from daily expenses, interest, and production that happen between decisions; do not attribute asynchronous payroll/production to the latest action.
- Add checks for count totals, accepted actions, semantic mapping, and unchanged default evaluator behavior.

## Acceptance

Compile Python modules, run evaluator adapter checks, then audit the 3×20×128 sample. The output must include source/model hashes, action counts and immediate deltas by Intent, final cash/debt/work units, invariant results, and an explicit statement that it is diagnostic and does not select/promote a model.
