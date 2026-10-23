"""Compare Realm transport commands exposed to Python with the C++ Realm replay."""
import argparse
import json
from pathlib import Path
import subprocess

import polyec_env as core


parser = argparse.ArgumentParser()
parser.add_argument("--replay", required=True)
args = parser.parse_args()
replay = str(Path(args.replay).resolve())
expected = list(map(int, subprocess.check_output(
    [replay, "--binding-replay"], text=True
).split()))
assert len(expected) == 10, expected

world = core.Realm(804, 26, 128)
routes = world.transport_routes
pair = next(
    (i, j)
    for i, route in enumerate(routes)
    for j in range(i + 1, len(routes))
    if routes[j]["atlas_link"] == route["atlas_link"]
    and routes[j]["from"] == route["to"]
    and routes[j]["to"] == route["from"]
)
route = routes[pair[0]]
source, destination = route["from"], route["to"]
good = 5  # Canonical Iron, the fourth historical Realm market good.
actor = next(
    person_id for person_id in range(1, 129)
    if (person := world.person(person_id))["alive"]
    and person["family"] == 1
    and person["region"] == source
)
opening_money = world.view()["money"]
checks = 0

person = world.person(actor)
purchase = world.intent_command(actor, person["decisions"] + 1,
                                1, source, good, 5)
assert purchase[0], purchase
assert world.person(actor)["inventory"][good] == 5
checks += 2

person = world.person(actor)
created = world.transport_shipment_command(
    actor, person["decisions"] + 1, shipment_id=0, operation=0,
    destination=destination, good=good, quantity=5, goal=0,
)
assert created[0] and created[3] > 0, created
shipment_id = created[3]
shipment = next(row for row in world.transport_shipments if row["id"] == shipment_id)
assert shipment["quantity"] == 5 and shipment["fee_escrow"] > 0
assert world.person(actor)["inventory"][good] == 0
assert world.view()["money"] == opening_money and world.invariant()
checks += 4

world.advance()
assert world.transport_cargo_lots and world.person(actor)["inventory"][good] == 0
midway = world.serialize("transport-python-roundtrip")
restored = core.Realm(1, 1, 128)
loaded, error = restored.deserialize(midway, "transport-python-roundtrip")
assert loaded, error
assert restored.fingerprint() == world.fingerprint() and restored.invariant()
checks += 2
restored.advance(9)
assert restored.person(actor)["inventory"][good] == 5
assert restored.view()["money"] == opening_money
assert restored.transport_escrow == 0 and not restored.transport_cargo_lots
assert restored.invariant()
shipment = next(row for row in restored.transport_shipments if row["id"] == shipment_id)
assert shipment["delivered"] == 5 and shipment["canceled"] == 0
checks += 5

actual = [
    restored.fingerprint(),
    restored.person(actor)["cash"],
    restored.person(actor)["inventory"][good],
    restored.view()["provinces"][route["fee_recipient_region"]]["treasury"],
    restored.view()["money"],
    len(restored.transport_cargo_lots),
    shipment["delivered"],
    shipment["canceled"],
    shipment["fee_escrow"],
    int(restored.invariant()),
]
assert actual == expected, ("C++/Python transport checkpoint mismatch", actual, expected)
checks += len(actual)

fingerprint = restored.fingerprint()
person = restored.person(actor)
stale = restored.transport_shipment_command(
    actor, person["decisions"], shipment_id=0, operation=0,
    destination=destination, good=good, quantity=1, goal=0,
)
assert not stale[0] and restored.fingerprint() == fingerprint
checks += 1
rejected = restored.transport_shipment_command(
    actor, person["decisions"] + 1, shipment_id=0, operation=0,
    destination=destination, good=good, quantity=6, goal=0,
)
assert not rejected[0] and restored.fingerprint() == fingerprint, (
    "insufficient-goods reject changed state", rejected, fingerprint, restored.fingerprint()
)
invalid_enum = restored.transport_shipment_command(
    actor, person["decisions"] + 1, shipment_id=0, operation=256,
    destination=destination, good=good, quantity=1, goal=0,
)
assert not invalid_enum[0] and restored.fingerprint() == fingerprint
invalid_goal = restored.transport_shipment_command(
    actor, person["decisions"] + 1, shipment_id=0, operation=0,
    destination=destination, good=good, quantity=1, goal=256,
)
assert not invalid_goal[0] and restored.fingerprint() == fingerprint
checks += 4

print(json.dumps({
    "checks": checks,
    "cpp_python_checkpoint_equal": True,
    "purchase_and_shipment_use_same_realm_core": True,
    "save_load_mid_transit": True,
    "goods_delivered": 5,
    "money_conserved": True,
    "invalid_commands_atomic": True,
    "learned_policy_action_added": False,
}))





