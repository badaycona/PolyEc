# M5.3 — marriage consent and settlement escrow

Date: 2026-10-21. Scope: one core gameplay slice in the active M5 delegation. This plan follows the marriage contract in `IMPLEMENTATION_PLAN.md` and uses the existing proposal/escrow lifecycle.

## Contract

- Two living people can marry only with an explicit offer and the named recipient's later acceptance. Reject and expiry refund the exact escrow to the proposer. Acceptance pays the proposed settlement from escrow to the recipient exactly once.
- Both parties must be at least 18 game years old, have no living spouse, and have no ancestor/descendant or sibling relation in the current typed relation graph. Every number of parent-child steps counts toward a direct-line relation. Eligibility is checked at offer and again at acceptance; guardians cannot consent for either person.
- A marriage records a symmetric spouse relation. It does not merge households, transfer land/title, create an alliance, or move any other property. Existing marriage is exclusive while both spouses live; death does not create a second live spouse.
- The existing Offer amount is the settlement amount. The deterministic candidate uses new action slot 72 with the current 50-unit offer and 30-day term. Slots 0–71 remain unchanged. This expands the action shape from 72 to 73 and observation from 992 to 1004; the action/model contract advances from `realm-v7` to `realm-v8`, so v7 weights are incompatible and cannot be used as a product model. The amount is an initial integration default, not a balance claim.
- Proposal records already serialize kind, amount, escrow, ready and due. Adding `RelationKind::Spouse=3` requires save schema 23; schema 7–22 saves without spouse edges remain readable and valid old token/order semantics remain unchanged.

## Files and invariants

- `core/include/polyec/realm.hpp`: append a typed `Marry` intent and spouse relation value without renumbering existing intent/relation values; advance the model contract dimensions/version.
- `core/src/realm.cpp`: produce slot 32, share a deterministic spouse-eligibility check across offer/accept, validate pending-consent conflicts, and keep rejected commands atomic.
- `core/src/realm_court.cpp`: stage the marriage proposal escrow, return it through existing refund/expiry paths, and settle/create spouse edges only on acceptance.
- `core/include/polyec/edge_section_codec.hpp`, `core/src/realm.cpp`: write/read spouse kind under schema 23, update invariant and schema header/parser.
- Current Python/native/Godot model contract surfaces: advance to realm-v8/1004 observation/73 actions; keep v7 artifacts historical and reject them for runtime.
- `core/tests/test_realm_court.cpp`, `core/tests/test_realm.cpp`: exact success, no consent before acceptance, reject/expiry refund, invalid age/kinship/already-married/stale acceptance, duplicate proposals, round-trip and old-schema compatibility.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`, evidence report and manifest: update only after gates actually run.

## Acceptance gates

- Release build and full core CTest; Debug focused Realm/court tests, then full Debug CTest if implementation changes shared persistence or transaction paths.
- Direct state/ledger oracle verifies escrow conservation and atomic rejection, direct and multi-generation lineal kinship rejection at offer and acceptance, symmetric relation, save/load continuation and schema-22 compatibility.
- Run current applicable Python binding/parity and Godot smoke gates because action candidates and save schema are surfaced across those layers. Do not claim learned-policy quality from candidate plumbing.
- Scoped `git diff --check`, links audit, and checkpoint hashes. Review is Codex self-review under the current user-authorized execution experiment; no independent reviewer or commit/push.

## Self-review before implementation

- Marriage is an explicit new intent at action slot 72; previous action IDs stay fixed, while the new shape/version intentionally invalidates v7 weights.
- `Proposal` already owns escrow and refund-on-reject/expiry, avoiding a parallel transaction state machine.
- The plan limits settlement to the existing named amount and does not infer land, household, alliance, divorce, adoption, will, or non-marital-child rules.
- Schema-gating the new edge value prevents older save versions from accepting a relation they could not have written.
