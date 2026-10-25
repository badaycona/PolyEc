"""Check that Python consumes the same schema and saved course records as C++."""

import polyec_env as core


assert core.REALM_SCHEMA == "realm-v45"
assert core.REALM_ACTIONS == 400 and core.REALM_OBS == 5182
world = core.Realm(20261098, 6, 256)
assert world.model_contract == "realm-household-fund-goods30-config-v54-atlas-north-pass-v1"
assert world.apprenticeships == []

saved = world.serialize("formal-apprenticeship-python-v1")
assert saved.startswith('POLYEC_REALM_75 "')
assert " APPRENTICESHIP_1 0 " in saved
restored = core.Realm(1, 1, 128)
ok, error = restored.deserialize(saved, "formal-apprenticeship-python-v1")
assert ok, error
assert restored.apprenticeships == [] and restored.invariant()

observation = world.observe(129)
assert len(observation) == core.REALM_OBS
assert len(world.mask(129)) == core.REALM_ACTIONS

print("formal_apprenticeship_binding_checks: schema, course property, empty-section roundtrip and observation passed")
