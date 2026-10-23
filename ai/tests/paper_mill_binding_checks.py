"""Exercise Forestry-to-Paper through Python calls into the shared C++ Realm."""
import json

import polyec_env as core


FOREST_REGION = 6
MILL_REGION = 13
TIMBER = 3
PAPER = 20
FORESTRY_ACTION = 92
PAPER_MILL_ACTION = 98
TRAVEL_TO_MILL_ACTION = 141 + MILL_REGION - 7

world = core.Realm(49201, 26, 256)
governor = FOREST_REGION + 2
mill_authority = MILL_REGION + 2
people = [world.person(person_id) for person_id in range(1, 257)]
buyer = max((person for person in people if person["alive"] and person["region"] == FOREST_REGION
             and person["family"] == 1 and person["age_days"] >= 14 * 365),
            key=lambda person: person["cash"])["id"]
forest_worker = next(person["id"] for person in people if person["alive"]
                     and person["region"] == FOREST_REGION and person["family"] == 2
                     and person["age_days"] >= 14 * 365)

assert core.REALM_SCHEMA == "realm-v30"
assert core.REALM_ACTIONS == 264 and core.REALM_OBS == 3550
assert len(world.observe(governor)) == 3484 and len(world.mask(governor)) == 263
assert world.mask(governor)[FORESTRY_ACTION] == 1.0
assert world.mask(mill_authority)[PAPER_MILL_ACTION] == 1.0
assert world.mask(buyer)[TRAVEL_TO_MILL_ACTION] == 1.0
checks = 6


def command(active, method, actor, *args):
    result = method(actor, active.person(actor)["decisions"] + 1, *args)
    assert result[0], result
    return result


ok, reason = world.step(governor, FORESTRY_ACTION)
assert ok, reason
forest_site = world.establishments()[0]["id"]
assert world.establishments()[0]["industry"] == 5
command(world, world.work_authorization_command, governor, forest_site, forest_worker, 0, 0, 32)
result = world.schedule_command(0, forest_worker, world.person(forest_worker)["decisions"] + 1,
                                begin=0, end=32, demand_rate=1000, kind=1,
                                region=FOREST_REGION, establishment_id=forest_site)
assert result[0], result
world.advance_ticks(64)
assert world.establishments()[0]["source_units"] == 99
assert world.establishments()[0]["stock"][TIMBER] == 1
checks += 5

command(world, world.establishment_transfer, governor, forest_site, 3, TIMBER, 1)
command(world, world.intent_command, buyer, 1, FOREST_REGION, TIMBER, 1)
assert world.person(buyer)["inventory"][TIMBER] == 1
opening_cash = world.person(buyer)["cash"]
created = world.transport_shipment_command(
    buyer, world.person(buyer)["decisions"] + 1, shipment_id=0, operation=0,
    destination=MILL_REGION, good=TIMBER, quantity=1, goal=0,
)
assert created[0], created
shipment_id = created[3]
shipment = next(row for row in world.transport_shipments if row["id"] == shipment_id)
assert shipment["fee_escrow"] > 0 and world.person(buyer)["inventory"][TIMBER] == 0
assert world.person(buyer)["cash"] == opening_cash - shipment["fee_escrow"]
ok, reason = world.step(buyer, TRAVEL_TO_MILL_ACTION)
assert ok, reason
world.advance(40)
shipment = next(row for row in world.transport_shipments if row["id"] == shipment_id)
assert shipment["delivered"] == 1 and world.person(buyer)["region"] == MILL_REGION
assert world.person(buyer)["inventory"][TIMBER] == 1 and world.invariant()
checks += 8

mill = world.establishment_command(
    buyer, world.person(buyer)["decisions"] + 1, 0, buyer, MILL_REGION, 7, 1, 25,
)
assert mill[0], mill
mill_site = mill[2]
mill_row = world.establishments()[-1]
assert mill_row["industry"] == 7 and mill_row["source_good"] == -1 and mill_row["source_units"] == 0
mill_worker = next(person_id for person_id in range(1, 257)
                   if (person := world.person(person_id))["alive"]
                   and person["region"] == MILL_REGION and person["family"] == 2
                   and person["age_days"] >= 14 * 365)
command(world, world.establishment_transfer, buyer, mill_site, 2, TIMBER, 1)
command(world, world.work_authorization_command, buyer, mill_site, mill_worker, 0,
        world.game_tick, world.game_tick + 32)
result = world.schedule_command(0, mill_worker, world.person(mill_worker)["decisions"] + 1,
                                begin=world.game_tick, end=world.game_tick + 32,
                                demand_rate=1000, kind=1, region=MILL_REGION,
                                establishment_id=mill_site)
assert result[0], result
saved = world.serialize("paper-mill-python-trial-1")
restored = core.Realm(1, 1, 128)
loaded, error = restored.deserialize(saved, "paper-mill-python-trial-1")
assert loaded, error
world.advance_ticks(64)
restored.advance_ticks(64)
assert world.fingerprint() == restored.fingerprint()
assert world.establishments()[-1]["stock"][TIMBER] == 0
assert world.establishments()[-1]["stock"][PAPER] == 1
command(world, world.establishment_transfer, buyer, mill_site, 3, PAPER, 1)
command(world, world.intent_command, buyer, 2, MILL_REGION, PAPER, 1)
assert world.person(buyer)["inventory"][PAPER] == 0 and world.invariant() and restored.invariant()
checks += 9

print(json.dumps({
    "checks": checks,
    "schema": core.REALM_SCHEMA,
    "observation_values": core.REALM_OBS,
    "action_choices": core.REALM_ACTIONS,
    "route_delivery": "region 6 to region 13 using a paid shipment and a legal Travel action",
    "recipe": "one Timber plus one full Work unit creates one Paper",
    "paper_sold_through_existing_market": True,
    "save_load_replay_equal": True,
    "shared_cpp_core": True,
    "learned_policy_inference": False,
    "invariant": world.invariant(),
}, sort_keys=True))





