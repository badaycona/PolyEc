"""Exercise the Weapon Forge using the shared C++ Realm binding."""
import json

import polyec_env as core

REGION = 19
IRON = 5
TOOLS = 16
WEAPON = 18
world = core.Realm(49501, 26, 256)
governor = REGION + 2
people = [world.person(person_id) for person_id in range(1, 257)]
worker = next(p["id"] for p in people if p["alive"] and p["region"] == REGION
              and p["family"] == 2 and p["age_days"] >= 14 * 365)
merchant = next(p["id"] for p in people if p["alive"] and p["region"] == REGION
                and p["family"] == 1 and p["age_days"] >= 14 * 365)

assert core.REALM_SCHEMA == "realm-v30"
assert core.REALM_ACTIONS == 264 and core.REALM_OBS == 3550
assert len(world.observe(governor)) == 3484 and len(world.mask(governor)) == 263
assert list(world.mask(governor)[170:173]) == [1.0, 1.0, 1.0]
assert world.view(governor)["provinces"][REGION]["stock"][3] > 0
assert world.view(governor)["provinces"][REGION]["stock"][5] > 0
checks = 6

ok, reason = world.step(governor, 170)
assert ok, reason
site = world.establishments()[0]["id"]
assert world.establishments()[0]["industry"] == 11
checks += 2

def command(active, method, actor, *args):
    result = method(actor, active.person(actor)["decisions"] + 1, *args)
    assert result[0], result
    return result


command(world, world.establishment_transfer, governor, site, 2, IRON, 1)

def authorize_and_work():
    tick = world.game_tick
    command(world, world.work_authorization_command, governor, site, worker, 0, tick, tick + 32)
    result = world.schedule_command(
        0, worker, world.person(worker)["decisions"] + 1,
        begin=tick, end=tick + 32, demand_rate=1000, kind=1,
        region=REGION, establishment_id=site,
    )
    assert result[0], result


authorize_and_work()
world.advance_ticks(64)
row = world.establishments()[0]
assert row["stock"][IRON] == 1 and row["stock"][TOOLS] == 0 and row["stock"][WEAPON] == 0
checks += 1

command(world, world.establishment_transfer, governor, site, 2, TOOLS, 1)
authorize_and_work()
world.advance_ticks(64)
row = world.establishments()[0]
assert row["stock"][IRON] == 0 and row["stock"][TOOLS] == 0 and row["stock"][WEAPON] == 1
checks += 3

saved = world.serialize("weapon-forge-trial-1")
assert saved.startswith('POLYEC_REALM_52 "') and " ESTABLISH_13 1 " in saved
resumed = core.Realm(1, 1, 128)
assert resumed.deserialize(saved, "weapon-forge-trial-1")[0]
assert resumed.establishments()[0]["stock"][WEAPON] == 1
checks += 3

command(resumed, resumed.establishment_transfer, governor, site, 3, WEAPON, 1)
command(resumed, resumed.intent_command, merchant, 4, REGION, 0, 300, 180)
command(resumed, resumed.intent_command, merchant, 1, REGION, WEAPON, 1)
assert resumed.person(merchant)["inventory"][WEAPON] == 1
command(resumed, resumed.intent_command, merchant, 2, REGION, WEAPON, 1)
assert resumed.person(merchant)["inventory"][WEAPON] == 0 and resumed.invariant()
checks += 6

print(json.dumps({
    "checks": checks,
    "schema": core.REALM_SCHEMA,
    "observation_values": core.REALM_OBS,
    "action_choices": core.REALM_ACTIONS,
    "exact_inputs": [IRON, TOOLS],
    "output_good": WEAPON,
    "shared_cpp_core": True,
    "learned_policy_inference": False,
    "invariant": resumed.invariant(),
}, sort_keys=True))

