"""Check that the current Python adapter exposes the C++ supply records and shape."""
import polyec_env as core


assert core.REALM_SCHEMA == "realm-v30"
assert core.REALM_ACTIONS == 264 and core.REALM_OBS == 3550
world = core.Realm(9154, 6, 128)
assert world.invariant()
assert world.military_supply_obligations == []
observation = world.observe(1)
mask = world.mask(1)
assert len(observation) == core.REALM_OBS
assert len(mask) == core.REALM_ACTIONS
assert sum(mask) >= 1
print("MILITARY SUPPLY OBLIGATION BINDING: schema, dimensions, empty records, and observation/mask pass")
