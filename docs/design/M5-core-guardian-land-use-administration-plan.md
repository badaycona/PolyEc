# M5.0/1 — guardian administration of dependent land-use permission

Date: 2026-10-21. The implementation plan says dependents retain ownership while a guardian may administer assets within limits. This slice uses the existing abstract whole-holding registry and the existing no-price/no-yield permission record; it does not create title transfer authority.

## Contract

- An adult living title-holder may continue to grant, replace, and revoke a land-use permission for their own holding.
- A living adult currently returned by `Realm::guardian(title_holder)` may perform those same permission operations only when the living title-holder is a minor. Guardian selection remains derived from current household membership and person state.
- The dependent remains title-holder. A guardian command records its actor in the event fact while `LandUseRight::grantor` remains the title-holder, preserving the registry's ownership invariant and revocation/transfer protection.
- Guardians cannot transfer the dependent's title, withdraw money, receive the holding, or grant a permission to themselves. Rights remain unpriced and produce no goods, tenancy, or yield. No save schema or model/action/observation/reward changes.
- A guardian whose authority ceases cannot revoke an existing permission; the adult dependent can revoke after majority. This avoids stale guardian authority being persisted.

## Files and ownership

- `core/src/realm_land.cpp`: central authorization check shared by grant/revoke, with stable reject reasons and validation before mutation.
- `core/tests/test_land_title.cpp`: guardian grant/revoke for minor-owned title, title remains unchanged, event/counter ownership, dead/non-guardian/adult-holder reject atomicity, transfer remains blocked while permission is active.
- This plan and the standard current-task/history/evidence/checkpoint files after the slice passes.

## Gate

1. Release/Debug direct land-title tests and full CTest.
2. Python replay and parity/preflight, native/GDExt build and Godot smoke using a freshly generated fixture; existing command signatures remain identical.
3. Repository local link audit, scoped diff check, hash manifest.

Self-review only. This contract is limited to permission administration. It does not authorize a guardian to sell, gift, pledge, lease for rent, partition, or inherit the dependent's property.
