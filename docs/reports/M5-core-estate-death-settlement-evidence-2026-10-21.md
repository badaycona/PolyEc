# Core refactor — staged death settlement boundary

> Historical snapshot: this report describes the earlier implementation, which returned inventory to Province. It was superseded by [the estate ledger update](M5-core-estate-ledger-evidence-2026-10-21.md), which moves inventory into a decedent case and writes it in schema24.

Date: 2026-10-21. Plan: [Estate death settlement boundary](../design/M5-core-estate-death-settlement-plan.md). Checkpoint hashes: [manifest](../../artifacts/m5-core-estate-death-settlement-2026-10-21-manifest.json).

`EstateManager` now stages the existing death disposition as an opaque `EstateDeathPlan`. `prepare_death` checks a living canonical Person, nonnegative money/goods and checked estate/stock additions without mutation. `apply_death` validates the complete Person/Province/estate read set before its first write; success marks the Person dead, transfers remaining cash into the shared estate balance and moves each inventory quantity into the current Province. The death trigger, daily ordering, facts, Realm ownership and save format remain in `Realm`.

This is an infrastructure extraction only. It does not distribute assets to heirs, record claims, settle debts, transfer land titles, or introduce probate or guardian administration. The shared estate balance remains the existing aggregate sink.

## Verification

- Windows Release and Debug `ALL_BUILD`: pass.
- Full CTest: Release 19/19 (46.66 s); Debug 19/19 (253.15 s).
- Direct manager tests: Release and Debug pass, including prepare purity, cash/goods conservation, stale Person/Province/estate read-set rejection, overflow/negative rejection and double-apply denial.
- Repository links: 890 local references passed. Scoped `git diff --check`: exit 0.
- No Python/native/GDExt/Godot/model gate was needed by this infrastructure-only plan: command, save bytes, bindings and policy interface are unchanged. No performance claim was measured.

Review was self-review only; no independent reviewer. No commit or push.
