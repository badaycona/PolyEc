"""Exercise the Saltworks action and market chain through the shared C++ Python binding."""
import json

import polyec_env as core


region = 8
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
assert world.mask(governor)[89] == 1.0
province_view = world.view(governor)["provinces"][region]
assert len(province_view["stock"]) == 7 and len(province_view["prices"]) == 7
assert len(province_view["good_stock"]) == 27 and len(province_view["good_prices"]) == 27
checks = 7

# The policy-facing candidate creates the public Saltworks at the atlas region
# that permits salt production. Remaining operations call their typed commands.
ok, reason = world.step(governor, 89)
assert ok, reason
site = world.establishments()[0]["id"]
assert world.establishments()[0]["industry"] == 4
checks += 2

def command(active, method, actor, *args):
    result = method(actor, active.person(actor)["decisions"] + 1, *args)
    assert result[0], result
    return result


command(world, world.establishment_transfer, governor, site, 2, 4, 1)  # Deposit one Fuel.
command(world, world.work_authorization_command, governor, site, worker, 0, 0, 32)
result = world.schedule_command(0, worker, world.person(worker)["decisions"] + 1,
                                begin=0, end=32, demand_rate=1000, kind=1,
                                region=region, establishment_id=site)
assert result[0], result
world.advance_ticks(64)
stocks = world.establishments()[0]["stock"]
assert stocks[4] == 0 and stocks[2] == 1
checks += 5

saved = world.serialize("saltworks-binding-trial-1")
resumed = core.Realm(1, 1, 128)
assert resumed.deserialize(saved, "saltworks-binding-trial-1")[0]
resumed.advance_ticks(64)
assert resumed.establishments()[0]["stock"][2] == 1
checks += 2

site = resumed.establishments()[0]["id"]
market_before_withdrawal = resumed.view(governor)["provinces"][region]["good_stock"][2]
command(resumed, resumed.establishment_transfer, governor, site, 3, 2, 1)  # Withdraw Salt.
assert resumed.establishments()[0]["stock"][2] == 0
assert resumed.view(governor)["provinces"][region]["good_stock"][2] == market_before_withdrawal + 1
command(resumed, resumed.intent_command, buyer, 1, region, 2, 1)  # Buy Salt.
assert resumed.person(buyer)["inventory"][2] == 1
assert resumed.view(governor)["provinces"][region]["good_stock"][2] == market_before_withdrawal
command(resumed, resumed.intent_command, buyer, 2, region, 2, 1)  # Sell Salt.
assert resumed.person(buyer)["inventory"][2] == 0
assert resumed.view(governor)["provinces"][region]["good_stock"][2] == market_before_withdrawal + 1
command(resumed, resumed.intent_command, buyer, 1, region, 2, 1)  # Buy the same Salt again.
assert resumed.person(buyer)["inventory"][2] == 1
assert resumed.view(governor)["provinces"][region]["good_stock"][2] == market_before_withdrawal
assert resumed.invariant()
checks += 7

print(json.dumps({
    "checks": checks,
    "schema": core.REALM_SCHEMA,
    "observation_values": core.REALM_OBS,
    "action_choices": core.REALM_ACTIONS,
    "commands": ["Create Saltworks", "Deposit Fuel", "Authorize Work",
                 "Submit Work", "Produce Salt", "Save/load", "Withdraw Salt",
                 "Buy Salt", "Sell Salt", "Buy Salt again"],
    "shared_cpp_core": True,
    "invariant": resumed.invariant(),
    "learned_policy_inference": False,
}, sort_keys=True))





