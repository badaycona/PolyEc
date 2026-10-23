"""Exercise the Tea Garden through Python calls into the shared C++ Realm."""
import json

import polyec_env as core


region = 14
tea = 22
world = core.Realm(49202, 26, 256)
governor = region + 2
people = [world.person(person_id) for person_id in range(1, 257)]
worker = next(person["id"] for person in people
              if person["alive"] and person["region"] == region
              and person["family"] == 2 and person["age_days"] >= 14 * 365)
merchant = next(person["id"] for person in people
                if person["alive"] and person["region"] == region and person["family"] == 1)

assert core.REALM_SCHEMA == "realm-v30"
assert core.REALM_ACTIONS == 264 and core.REALM_OBS == 3550
assert len(world.observe(governor)) == 3484 and len(world.mask(governor)) == 263
assert list(world.mask(governor)[161:164]) == [1.0, 1.0, 1.0]
checks = 4

ok, reason = world.step(governor, 161)
assert ok, reason
site = world.establishments()[0]["id"]
row = world.establishments()[0]
assert row["industry"] == 8 and row["source_good"] == tea and row["source_units"] == 25
checks += 3

result = world.work_authorization_command(
    governor, world.person(governor)["decisions"] + 1, site, worker, 0, 0, 32
)
assert result[0], result
result = world.schedule_command(
    0, worker, world.person(worker)["decisions"] + 1,
    begin=0, end=32, demand_rate=1000, kind=1,
    region=region, establishment_id=site,
)
assert result[0], result
world.advance_ticks(64)
row = world.establishments()[0]
assert row["source_units"] == 24 and row["stock"][tea] == 1 and row["stock"][12] == 0
checks += 4

saved = world.serialize("tea-garden-trial-1")
assert saved.startswith('POLYEC_REALM_52 "') and " ESTABLISH_13 1 " in saved
resumed = core.Realm(1, 1, 128)
assert resumed.deserialize(saved, "tea-garden-trial-1")[0]
assert resumed.establishments()[0]["source_units"] == 24
assert resumed.establishments()[0]["stock"][tea] == 1
checks += 3

result = resumed.establishment_transfer(
    governor, resumed.person(governor)["decisions"] + 1, site, 3, tea, 1
)
assert result[0], result
result = resumed.intent_command(
    merchant, resumed.person(merchant)["decisions"] + 1, 4,
    region, 0, 100, 180,
)
assert result[0], result
result = resumed.intent_command(
    merchant, resumed.person(merchant)["decisions"] + 1, 1, region, tea, 1
)
assert result[0], result
assert resumed.person(merchant)["inventory"][tea] == 1
result = resumed.intent_command(
    merchant, resumed.person(merchant)["decisions"] + 1, 2, region, tea, 1
)
assert result[0], result
assert resumed.person(merchant)["inventory"][tea] == 0 and resumed.invariant()
checks += 6

resumed.advance(88)
assert resumed.view(governor)["day"] == 89 and resumed.establishments()[0]["source_units"] == 24
resumed.advance(1)
assert resumed.view(governor)["day"] == 90 and resumed.establishments()[0]["source_units"] == 49
checks += 2

print(json.dumps({
    "checks": checks,
    "schema": core.REALM_SCHEMA,
    "observation_values": core.REALM_OBS,
    "action_choices": core.REALM_ACTIONS,
    "source_units_at_day_90": resumed.establishments()[0]["source_units"],
    "shared_cpp_core": True,
    "learned_policy_inference": False,
    "invariant": resumed.invariant(),
}, sort_keys=True))







