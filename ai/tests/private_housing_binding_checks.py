"""Check private-home command and observer projection through shared C++ Realm."""

import polyec_env as core


assert core.REALM_SCHEMA == "realm-v41"
assert core.REALM_ACTIONS == 400 and core.REALM_OBS == 5182
assert core.REALM_PRIVATE_HOUSING_CANDIDATE_BEGIN == 336
assert core.REALM_PRIVATE_BUILD_CANDIDATE_BEGIN == 336
assert core.REALM_PRIVATE_BUILD_CANDIDATE_COUNT == 16
assert core.REALM_PRIVATE_OFFER_CANDIDATE_BEGIN == 352
assert core.REALM_PRIVATE_OFFER_CANDIDATE_COUNT == 16
assert core.REALM_PRIVATE_RESPONSE_CANDIDATE_BEGIN == 368
assert core.REALM_PRIVATE_RESPONSE_CANDIDATE_COUNT == 16
assert core.REALM_PRIVATE_END_LEASE_CANDIDATE_BEGIN == 384
assert core.REALM_PRIVATE_END_LEASE_CANDIDATE_COUNT == 16

world = core.Realm(20261024, 26, 512)
sovereign = world.view(1)
assert sovereign["private_residences"] == []
before = world.fingerprint()

# Unknown title and residence IDs are rejected by the same Realm commands as
# native callers; a rejected request must not alter money, inventory, or day.
ok, reason, _ = world.private_residence_build(1, world.person(1)["decisions"] + 1, 0)
assert not ok and reason and world.fingerprint() == before
ok, reason, _ = world.private_housing_command(
    1, world.person(1)["decisions"] + 1, 0, 0, 0, 0
)
assert not ok and reason and world.fingerprint() == before
ok, reason, _ = world.private_housing_command(
    1, world.person(1)["decisions"] + 1, 0, 99, 0, 0
)
assert not ok and reason == "Unknown private housing operation"
assert world.fingerprint() == before and world.invariant()

print("private_housing_binding_checks: 8 assertions passed")
