"""Exercise Warhorse Stable training through the shared C++ Realm binding."""
import json

import polyec_env as core

REGION = 16
HORSE = 14
WARHORSE = 15
world = core.Realm(49401, 26, 256)
governor = REGION + 2
people = [world.person(person_id) for person_id in range(1, 257)]
worker = next(p["id"] for p in people if p["alive"] and p["region"] == REGION
              and p["family"] == 2 and p["age_days"] >= 14 * 365)
merchant = next(p["id"] for p in people if p["alive"] and p["region"] == REGION
                and p["family"] == 1 and p["age_days"] >= 14 * 365)

assert core.REALM_SCHEMA == "realm-v30"
assert core.REALM_ACTIONS == 264 and core.REALM_OBS == 3550
assert len(world.observe(governor)) == 3484 and len(world.mask(governor)) == 263
assert list(world.mask(governor)[167:170]) == [1.0, 1.0, 1.0]
assert world.view(governor)["provinces"][REGION]["stock"][6] > 0
checks = 5

ok, reason = world.step(governor, 167)
assert ok, reason
site = world.establishments()[0]["id"]
row = world.establishments()[0]
assert row["industry"] == 10 and row["stock"][WARHORSE] == 0
checks += 3

def authorize_and_work():
    result = world.work_authorization_command(
        governor, world.person(governor)["decisions"] + 1, site, worker,
        0, world.game_tick, world.game_tick + 32,
    )
    assert result[0], result
    tick = world.game_tick
    result = world.schedule_command(
        0, worker, world.person(worker)["decisions"] + 1,
        begin=tick, end=tick + 32, demand_rate=1000, kind=1,
        region=REGION, establishment_id=site,
    )
    assert result[0], result


authorize_and_work()
world.advance_ticks(64)
assert world.establishments()[0]["stock"][WARHORSE] == 0
checks += 1

result = world.establishment_transfer(
    governor, world.person(governor)["decisions"] + 1, site, 2, HORSE, 1
)
assert result[0], result
authorize_and_work()
world.advance_ticks(64)
row = world.establishments()[0]
assert row["stock"][HORSE] == 0 and row["stock"][WARHORSE] == 1
checks += 3

saved = world.serialize("warhorse-stable-trial-1")
assert saved.startswith('POLYEC_REALM_52 "') and " ESTABLISH_13 1 " in saved
resumed = core.Realm(1, 1, 128)
assert resumed.deserialize(saved, "warhorse-stable-trial-1")[0]
assert resumed.establishments()[0]["stock"][WARHORSE] == 1
checks += 3

result = resumed.establishment_transfer(
    governor, resumed.person(governor)["decisions"] + 1, site, 3, WARHORSE, 1
)
assert result[0], result
result = resumed.intent_command(
    merchant, resumed.person(merchant)["decisions"] + 1, 4, REGION, 0, 300, 180
)
assert result[0], result
result = resumed.intent_command(
    merchant, resumed.person(merchant)["decisions"] + 1, 1, REGION, WARHORSE, 1
)
assert result[0], result
assert resumed.person(merchant)["inventory"][WARHORSE] == 1
result = resumed.intent_command(
    merchant, resumed.person(merchant)["decisions"] + 1, 2, REGION, WARHORSE, 1
)
assert result[0], result
assert resumed.person(merchant)["inventory"][WARHORSE] == 0 and resumed.invariant()
checks += 6

print(json.dumps({
    "checks": checks,
    "schema": core.REALM_SCHEMA,
    "observation_values": core.REALM_OBS,
    "action_choices": core.REALM_ACTIONS,
    "trained_with_existing_horse_input": True,
    "shared_cpp_core": True,
    "learned_policy_inference": False,
    "invariant": resumed.invariant(),
}, sort_keys=True))





