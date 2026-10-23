"""Exercise Forestry through the Python binding to the shared C++ Realm core."""
import json

import polyec_env as core


region = 6
timber = 3
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
assert world.mask(governor)[92] == 1.0
assert len(world.view(governor)["provinces"][region]["good_stock"]) == 27
checks = 5

ok, reason = world.step(governor, 92)
assert ok, reason
site_row = world.establishments()[0]
site = site_row["id"]
assert site_row["industry"] == 5 and site_row["source_good"] == timber
assert site_row["source_units"] == 100 and site_row["forest_units"] == 100
checks += 3

def command(active, method, actor, *args):
    result = method(actor, active.person(actor)["decisions"] + 1, *args)
    assert result[0], result
    return result


command(world, world.work_authorization_command, governor, site, worker, 0, 0, 32)
result = world.schedule_command(0, worker, world.person(worker)["decisions"] + 1,
                                begin=0, end=32, demand_rate=1000, kind=1,
                                region=region, establishment_id=site)
assert result[0], result
world.advance_ticks(64)
site_row = world.establishments()[0]
assert site_row["source_good"] == timber and site_row["source_units"] == 99
assert site_row["forest_units"] == 99 and site_row["stock"][timber] == 1
checks += 4

saved = world.serialize("timber-forest-source-trial-1")
resumed = core.Realm(1, 1, 128)
assert resumed.deserialize(saved, "timber-forest-source-trial-1")[0]
resumed.advance_ticks(64)
site_row = resumed.establishments()[0]
assert site_row["source_good"] == timber and site_row["source_units"] == 99
assert site_row["forest_units"] == 99 and site_row["stock"][timber] == 1
checks += 2

command(resumed, resumed.establishment_transfer, governor, site, 3, timber, 1)
command(resumed, resumed.intent_command, buyer, 1, region, timber, 1)
assert resumed.person(buyer)["inventory"][timber] == 1
command(resumed, resumed.intent_command, buyer, 2, region, timber, 1)
assert resumed.person(buyer)["inventory"][timber] == 0
command(resumed, resumed.intent_command, buyer, 1, region, timber, 1)
assert resumed.person(buyer)["inventory"][timber] == 1
assert resumed.invariant()
checks += 7

print(json.dumps({
    "checks": checks,
    "schema": core.REALM_SCHEMA,
    "observation_values": core.REALM_OBS,
    "action_choices": core.REALM_ACTIONS,
    "commands": ["Create Forestry", "Authorize Work", "Submit Work",
                 "Produce Timber", "Save/load", "Withdraw Timber", "Buy/Sell/Buy"],
    "shared_cpp_core": True,
    "remaining_forest_units": resumed.establishments()[0]["forest_units"],
    "learned_policy_inference": False,
    "invariant": resumed.invariant(),
}, sort_keys=True))





