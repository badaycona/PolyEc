"""Check current-schema housing and council projection through the shared C++ binding."""

import polyec_env as core


assert core.REALM_SCHEMA == "realm-v45"
assert core.REALM_ACTIONS == 400 and core.REALM_OBS == 5182

world = core.Realm(20261024, 26, 512)
governor = 8  # Atlas region 6 Governance actor
census = world.view(governor)["urban_census"]
assert len(census) == 1 and census[0]["territory_id"] == 6
row = census[0]
assert row["housing_capacity"] % 8 == 0
assert row["housing_capacity"] >= row["urban_residents"]
assert row["unhoused_residents"] == max(0, row["urban_residents"] - row["housing_capacity"])
assert row["available_beds"] == max(0, row["housing_capacity"] - row["urban_residents"])
assert row["council_representative_id"] == 0 and row["council_nominee_id"] == 0

saved = world.serialize("urban-housing-binding-v1")
assert saved.startswith('POLYEC_REALM_75 "')
restored = core.Realm(1, 1, 128)
ok, error = restored.deserialize(saved, "urban-housing-binding-v1")
assert ok, error
assert restored.view(governor)["urban_census"] == census
assert restored.invariant()

print("urban_housing_binding_checks: 9 assertions passed")
