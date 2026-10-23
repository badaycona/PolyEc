"""Compare a real C++ market/workshop command chain with the Python binding."""
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
    [replay, "--market-replay"], text=True
).split()))
assert len(expected) == 14, expected

world = core.Realm(71, 2, 128)
worker = 65
region = world.person(worker)["region"]
opening_money = world.view()["money"]
checks = 0

result = world.intent_command(worker, world.person(worker)["decisions"] + 1,
                              1, region, 9, 2)  # Buy two canonical Fiber.
assert result[0], result
checks += 1
created = world.establishment_command(worker, world.person(worker)["decisions"] + 1,
                                      0, worker, region, 1, 1, 10)
assert created[0], created
site = created[2]
checks += 1
result = world.establishment_transfer(worker, world.person(worker)["decisions"] + 1,
                                      site, 2, 9, 2)  # Deposit Fiber.
assert result[0], result
checks += 1
result = world.schedule_command(0, worker, world.person(worker)["decisions"] + 1,
                                begin=0, end=32, demand_rate=1000, kind=1,
                                region=region, establishment_id=site)
assert result[0], result
checks += 1
world.advance_ticks(64)
assert world.invariant() and world.view()["money"] == opening_money
site_state = next(row for row in world.establishments() if row["id"] == site)
assert site_state["stock"][9] == 1 and site_state["stock"][11] == 1
checks += 2

result = world.establishment_transfer(worker, world.person(worker)["decisions"] + 1,
                                      site, 3, 11, 1)  # Withdraw produced Cloth.
assert result[0], result
checks += 1
result = world.intent_command(worker, world.person(worker)["decisions"] + 1,
                              2, region, 11, 1)  # Sell one Cloth.
assert result[0], result
checks += 1

buyer = expected[2]
buyer_state = world.person(buyer)
assert buyer != worker and buyer_state["alive"] and buyer_state["region"] == region
assert buyer_state["family"] == 2 and buyer_state["age_days"] >= 6 * 365
result = world.intent_command(buyer, buyer_state["decisions"] + 1,
                              1, region, 11, 1)  # Another resident buys Cloth.
assert result[0], result
checks += 2

view = world.view()
province = view["provinces"][region]
site_state = next(row for row in world.establishments() if row["id"] == site)
actual = [
    world.fingerprint(),
    world.person(worker)["cash"],
    buyer,
    world.person(buyer)["cash"],
    site_state["cash"],
    site_state["stock"][9],
    site_state["stock"][11],
    province["firm"],
    province["treasury"],
    province["stock"][2],  # Cloth is legacy market slot 2.
    world.view()["money"],
    int(world.invariant()),
]
assert actual[0] == expected[0], ("C++/Python full-state fingerprint mismatch", actual, expected)
assert actual[1:10] == expected[1:10], ("C++/Python ledger mismatch", actual, expected)
assert actual[10:] == expected[12:14], ("C++/Python conservation mismatch", actual, expected)
checks += len(actual) - 1

saved = world.serialize("market-binding")
restored = core.Realm(1, 1, 128)
assert restored.deserialize(saved, "market-binding")[0]
assert restored.fingerprint() == world.fingerprint() and restored.invariant()
checks += 2

print(json.dumps({
    "checks": checks,
    "cpp_python_fingerprint_equal": True,
    "commands": ["Buy Fiber", "CreateEstablishment", "DepositGoods", "Submit Work",
                 "WithdrawGoods", "Sell Cloth", "Buy Cloth"],
    "saved_world_roundtrip": True,
    "shared_cpp_core": True,
    "learned_policy_inference": False,
}))





