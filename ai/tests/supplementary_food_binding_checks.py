"""Check Seed Press Works through the shared C++ Python binding."""
import json

import polyec_env as core


region = 2
supplementary_food = 1
world = core.Realm(73102, 26, 512)
governor = region + 2
worker = next(
    person_id for person_id in range(1, 513)
    if (world.person(person_id)["alive"]
        and world.person(person_id)["region"] == region
        and world.person(person_id)["family"] == 2
        and world.person(person_id)["age_days"] >= 14 * 365)
)

assert core.REALM_SCHEMA == "realm-v43"
assert core.REALM_ACTIONS == 400 and core.REALM_OBS == 5182
assert world.mask(governor)[266] == 1.0
assert world.step(governor, 266)[0]
site = world.establishments()[0]["id"]
establishment = world.establishments()[0]
assert establishment["industry"] == 16
assert establishment["source_good"] == supplementary_food
assert establishment["source_units"] == 100
assert establishment["cash"] == 100
assert establishment["stock"][supplementary_food] == 0

tick = world.game_tick
result = world.work_authorization_command(
    governor, world.person(governor)["decisions"] + 1,
    site, worker, 0, tick, tick + 32,
)
assert result[0], result
result = world.schedule_command(
    0, worker, world.person(worker)["decisions"] + 1,
    begin=tick, end=tick + 32, demand_rate=1000,
    kind=1, region=region, establishment_id=site,
)
assert result[0], result
world.advance_ticks(64)
establishment = world.establishments()[0]
assert establishment["source_units"] == 99
assert establishment["stock"][supplementary_food] == 1
assert establishment["stock"][0] == 0

saved = world.serialize("supplementary-food-source-v1")
assert saved.startswith('POLYEC_REALM_66 "')
resumed = core.Realm(1, 1, 128)
assert resumed.deserialize(saved, "supplementary-food-source-v1")[0]
establishment = resumed.establishments()[0]
assert establishment["source_units"] == 99
assert establishment["stock"][supplementary_food] == 1
assert resumed.invariant()

print(json.dumps({
    "checks": 19,
    "schema": core.REALM_SCHEMA,
    "observation_values": core.REALM_OBS,
    "action_choices": core.REALM_ACTIONS,
    "supplementary_food_good_id": supplementary_food,
    "source_units_after_one_batch": 99,
    "supplementary_food_after_one_batch": 1,
    "shared_cpp_core": True,
    "learned_policy_inference": False,
}, sort_keys=True))
