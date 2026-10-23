"""Replay the actual Buy/Create/Transfer/Work commands against the C++ driver."""
import argparse
import json
import subprocess

import polyec_env as core

parser = argparse.ArgumentParser()
parser.add_argument("--replay", required=True)
args = parser.parse_args()
expected = [list(map(int, line.split())) for line in subprocess.check_output(
    [args.replay, "--replay"], text=True
).splitlines()]
world = core.Realm(71, 2, 128)
worker = 65
region = world.person(worker)["region"]
opening_money = world.view()["money"]
assert world.intent_command(worker, 1, 1, region, 9, 2)[0]  # canonical GoodId::Fiber: 2 units
created = world.establishment_command(worker, 2, 0, worker, region, 1, 1, 10)
assert created[0], created
site = created[2]
assert world.establishment_transfer(worker, 3, site, 2, 9, 2)[0]
before = world.fingerprint()
assert not world.establishment_transfer(worker, 3, site, 2, 9, 2)[0]
assert world.fingerprint() == before
assert not world.establishment_transfer(66, 1, site, 3, 9, 1)[0]
assert world.fingerprint() == before
assert world.schedule_command(0, worker, 4, begin=0, end=32,
                              demand_rate=1000, kind=1, region=region,
                              establishment_id=site)[0]
checks = 8
previous = 0
for tick, fingerprint, cash, fiber, cloth in expected:
    world.advance_ticks(tick - previous)
    previous = tick
    observed = world.establishments()[0]
    assert world.fingerprint() == fingerprint
    assert (observed["cash"], observed["stock"][9], observed["stock"][11]) == (cash, fiber, cloth)
    assert world.invariant() and world.view()["money"] == opening_money
    checks += 3
    if tick == 16:
        saved = world.serialize("transfer-test")
assert observed["stock"][9] == 1 and observed["stock"][11] == 1
restored = core.Realm(1, 1, 128)
assert restored.deserialize(saved, "transfer-test")[0]
restored.advance_ticks(128 - 16)
assert restored.fingerprint() == world.fingerprint()
checks += 3
print(json.dumps({"checks": checks, "cpp_python_checkpoints": len(expected),
                  "inputs_supplied_by_commands": True,
                  "scope": "transfers and production; establishment payroll is a later gate"}))





