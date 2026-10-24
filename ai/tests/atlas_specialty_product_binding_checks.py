"""Exercise the Atlas specialty recipes through the shared C++ Python binding."""
import json

import polyec_env as core

CASES = [
    (10, 12, 27, 264, True),
    (15, 13, 28, 267, True),
    (17, 14, 29, 270, False),
]
checks = 0


def command(world, method, actor, *args):
    result = method(actor, world.person(actor)["decisions"] + 1, *args)
    assert result[0], result
    return result


for region, industry, product, candidate, burns_fuel in CASES:
    world = core.Realm(60100 + region, 26, 256)
    governor = region + 2
    worker = next(p["id"] for p in (world.person(i) for i in range(1, 257))
                  if p["alive"] and p["region"] == region and p["family"] == 2
                  and p["age_days"] >= 14 * 365)
    assert core.REALM_SCHEMA == "realm-v34"
    assert core.REALM_ACTIONS == 400 and core.REALM_OBS == 5182
    assert world.model_contract == "realm-household-fund-goods30-config-v45"
    assert world.mask(governor)[candidate] == 1.0
    assert world.step(governor, candidate)[0]
    site = world.establishments()[0]["id"]
    row = world.establishments()[0]
    assert len(row["stock"]) == 30 and row["industry"] == industry and row["source_good"] == product
    assert row["source_units"] == 100 and row["stock"][product] == 0
    checks += 7

    if burns_fuel:
        assert world.view(governor)["provinces"][region]["stock"][4] > 0
        command(world, world.establishment_transfer, governor, site, 2, 4, 1)
        checks += 2

    tick = world.game_tick
    command(world, world.work_authorization_command, governor, site, worker, 0, tick, tick + 32)
    result = world.schedule_command(
        0, worker, world.person(worker)["decisions"] + 1,
        begin=tick, end=tick + 32, demand_rate=1000, kind=1,
        region=region, establishment_id=site,
    )
    assert result[0], result
    world.advance_ticks(64)
    row = world.establishments()[0]
    assert row["stock"][product] == 1 and row["source_units"] == 99
    assert (not burns_fuel) or row["stock"][4] == 0
    checks += 4

    save = world.serialize("atlas-specialty-products-v1")
    assert save.startswith('POLYEC_REALM_67 "') and " ESTABLISH_14 1 " in save
    resumed = core.Realm(1, 1, 128)
    assert resumed.deserialize(save, "atlas-specialty-products-v1")[0]
    assert resumed.establishments()[0]["stock"][product] == 1
    assert resumed.establishments()[0]["source_units"] == 99 and resumed.invariant()
    checks += 4

print(json.dumps({
    "checks": checks,
    "schema": core.REALM_SCHEMA,
    "observation_values": core.REALM_OBS,
    "action_choices": core.REALM_ACTIONS,
    "products": [{"region": region, "industry": industry, "good_id": good,
                  "uses_real_fuel": fuel} for region, industry, good, _, fuel in CASES],
    "save_schema": 67,
    "shared_cpp_core": True,
    "training_or_policy_evaluation": False,
}, sort_keys=True))


