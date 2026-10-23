"""Exercise Incense Works through Python calls into the shared C++ Realm."""
import json

import polyec_env as core

REGION = 11
INCENSE = 23
world = core.Realm(49301, 26, 256)
governor = REGION + 2
people = [world.person(person_id) for person_id in range(1, 257)]
worker = next(p["id"] for p in people if p["alive"] and p["region"] == REGION
              and p["family"] == 2 and p["age_days"] >= 14 * 365)
merchant = next(p["id"] for p in people if p["alive"] and p["region"] == REGION
                and p["family"] == 1 and p["age_days"] >= 14 * 365)

assert core.REALM_SCHEMA == "realm-v30"
assert core.REALM_ACTIONS == 264 and core.REALM_OBS == 3550
assert len(world.observe(governor)) == 3484 and len(world.mask(governor)) == 263
assert list(world.mask(governor)[164:167]) == [1.0, 1.0, 1.0]
checks = 5

ok, reason = world.step(governor, 164)
assert ok, reason
site = world.establishments()[0]["id"]
row = world.establishments()[0]
assert row["industry"] == 9 and row["source_good"] == INCENSE and row["source_units"] == 100
checks += 4

result = world.work_authorization_command(
    governor, world.person(governor)["decisions"] + 1, site, worker, 0, 0, 32
)
assert result[0], result
result = world.schedule_command(
    0, worker, world.person(worker)["decisions"] + 1,
    begin=0, end=32, demand_rate=1000, kind=1,
    region=REGION, establishment_id=site,
)
assert result[0], result
world.advance_ticks(64)
row = world.establishments()[0]
assert row["source_units"] == 99 and row["stock"][INCENSE] == 1
assert row["stock"][22] == 0
checks += 4

saved = world.serialize("incense-works-trial-1")
assert saved.startswith('POLYEC_REALM_52 "') and " ESTABLISH_13 1 " in saved
resumed = core.Realm(1, 1, 128)
assert resumed.deserialize(saved, "incense-works-trial-1")[0]
assert resumed.establishments()[0]["source_units"] == 99
checks += 3

result = resumed.establishment_transfer(
    governor, resumed.person(governor)["decisions"] + 1, site, 3, INCENSE, 1
)
assert result[0], result
result = resumed.intent_command(
    merchant, resumed.person(merchant)["decisions"] + 1, 4,
    REGION, 0, 300, 180,
)
assert result[0], result
result = resumed.intent_command(
    merchant, resumed.person(merchant)["decisions"] + 1, 1, REGION, INCENSE, 1
)
assert result[0], result
assert resumed.person(merchant)["inventory"][INCENSE] == 1
result = resumed.intent_command(
    merchant, resumed.person(merchant)["decisions"] + 1, 2, REGION, INCENSE, 1
)
assert result[0], result
assert resumed.person(merchant)["inventory"][INCENSE] == 0 and resumed.invariant()
checks += 6

resumed.advance(180)
assert resumed.establishments()[0]["source_units"] == 99
print(json.dumps({
    "checks": checks + 1,
    "schema": core.REALM_SCHEMA,
    "observation_values": core.REALM_OBS,
    "action_choices": core.REALM_ACTIONS,
    "source_units_after_180_days": resumed.establishments()[0]["source_units"],
    "shared_cpp_core": True,
    "learned_policy_inference": False,
    "invariant": resumed.invariant(),
}, sort_keys=True))







