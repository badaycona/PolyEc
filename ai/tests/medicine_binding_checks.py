"""Check Medicinal Garden actions and production through the shared C++ Python binding."""
import json

import polyec_env as core


region = 20
medicine = 21
world = core.Realm(73021, 26, 512)
governor = region + 2
people = [world.person(person_id) for person_id in range(1, 513)]
worker = next(person["id"] for person in people
              if person["alive"] and person["region"] == region
              and person["family"] == 2 and person["age_days"] >= 14 * 365)
assert core.REALM_SCHEMA == "realm-v38"
assert core.REALM_ACTIONS == 400 and core.REALM_OBS == 5182
assert world.mask(governor)[266] == 1.0
assert world.step(governor, 266)[0]
site = world.establishments()[0]["id"]
garden = world.establishments()[0]
assert garden["industry"] == 15 and garden["source_good"] == medicine
assert garden["source_units"] == 100 and garden["stock"][medicine] == 0

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
garden = world.establishments()[0]
assert garden["source_units"] == 99 and garden["stock"][medicine] == 1

saved = world.serialize("medicine-source-and-treatment-v1")
assert saved.startswith('POLYEC_REALM_66 "')
resumed = core.Realm(1, 1, 128)
assert resumed.deserialize(saved, "medicine-source-and-treatment-v1")[0]
assert resumed.establishments()[0]["source_units"] == 99
assert resumed.establishments()[0]["stock"][medicine] == 1
assert resumed.invariant()

print(json.dumps({
    "checks": 14,
    "schema": core.REALM_SCHEMA,
    "observation_values": core.REALM_OBS,
    "action_choices": core.REALM_ACTIONS,
    "medicine_good_id": medicine,
    "source_units_after_one_batch": 99,
    "medicine_after_one_batch": 1,
    "shared_cpp_core": True,
    "learned_policy_inference": False,
}, sort_keys=True))
