"""Verify region 23 produces canonical Silver through the bound C++ Realm."""
import json

import polyec_env as core


region = 23
silver = 7
world = core.Realm(49201, 26, 256)
governor = region + 2
people = [world.person(person_id) for person_id in range(1, 257)]
buyer = next(person["id"] for person in people
             if person["alive"] and person["region"] == region and person["family"] == 1)
worker = next(person["id"] for person in people
              if person["alive"] and person["region"] == region
              and person["family"] == 2 and person["age_days"] >= 14 * 365)

assert core.REALM_SCHEMA == "realm-v30"
assert core.REALM_ACTIONS == 264 and core.REALM_OBS == 3550
assert len(world.observe(governor)) == 3484 and len(world.mask(governor)) == 263
assert list(world.mask(governor)[95:98]) == [1.0, 1.0, 1.0]
checks = 5

ok, reason = world.step(governor, 95)
assert ok, reason
site = world.establishments()[0]["id"]
site_row = world.establishments()[0]
assert site_row["industry"] == 6 and site_row["source_good"] == silver
assert site_row["source_units"] == 100 and site_row["forest_units"] == 0
checks += 4

result = world.work_authorization_command(
    governor, world.person(governor)["decisions"] + 1,
    site, worker, 0, 0, 32,
)
assert result[0], result
result = world.schedule_command(
    0, worker, world.person(worker)["decisions"] + 1,
    begin=0, end=32, demand_rate=1000, kind=1,
    region=region, establishment_id=site,
)
assert result[0], result
world.advance_ticks(64)
site_row = world.establishments()[0]
assert site_row["source_good"] == silver and site_row["source_units"] == 99
assert site_row["stock"][silver] == 1 and site_row["stock"][6] == 0
checks += 4

saved = world.serialize("silver-mine-source-trial-1")
resumed = core.Realm(1, 1, 128)
assert resumed.deserialize(saved, "silver-mine-source-trial-1")[0]
assert resumed.establishments()[0]["source_good"] == silver
assert resumed.establishments()[0]["source_units"] == 99
assert resumed.establishments()[0]["stock"][silver] == 1
checks += 4

result = resumed.establishment_transfer(
    governor, resumed.person(governor)["decisions"] + 1, site, 3, silver, 1
)
assert result[0], result
result = resumed.intent_command(
    buyer, resumed.person(buyer)["decisions"] + 1, 1, region, silver, 1
)
assert result[0], result
assert resumed.person(buyer)["inventory"][silver] == 1
result = resumed.intent_command(
    buyer, resumed.person(buyer)["decisions"] + 1, 2, region, silver, 1
)
assert result[0], result
assert resumed.person(buyer)["inventory"][silver] == 0 and resumed.invariant()
checks += 5

print(json.dumps({
    "checks": checks,
    "schema": core.REALM_SCHEMA,
    "observation_values": core.REALM_OBS,
    "action_choices": core.REALM_ACTIONS,
    "region": region,
    "silver_units_remaining": resumed.establishments()[0]["source_units"],
    "shared_cpp_core": True,
    "learned_policy_inference": False,
    "invariant": resumed.invariant(),
}, sort_keys=True))





