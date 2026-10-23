"""Command-only replay against the C++ authorization driver."""
import subprocess
import sys
import polyec_env as core

expected = [list(map(int, row.split())) for row in subprocess.check_output(
    [sys.argv[1], "--authorization-replay"], text=True).splitlines()]
world = core.Realm(71, 2, 128)
ok, reason, site = world.establishment_command(1, 1, 1, 2, 2, 1, 1, 10)
assert ok, reason
worker = next(i for i in range(2, 129) if world.person(i)["alive"] and world.person(i)["region"] == 2)
checks = 0

def point(index):
    global checks
    assert [index, world.fingerprint(), worker, site] == expected[index], index
    checks += 1

def grant(operation, begin=0, end=0):
    return world.work_authorization_command(1, world.person(1)["decisions"] + 1,
                                             site, worker, operation, begin, end)

point(0)
assert grant(0, 0, 96)[0]
point(1)
assert world.schedule_command(0, worker, world.person(worker)["decisions"] + 1,
                              0, 16, 64, 0, 1000, 1, 2, site)[0]
point(2)
before = world.fingerprint()
ok, reason, _ = grant(1)
assert not ok and reason == "Authorization would cut accepted Work" and world.fingerprint() == before
point(3)
world.advance_ticks(24)
point(4)
saved = world.serialize("authorization-replay")
loaded = core.Realm(1, 1, 128)
ok, reason = loaded.deserialize(saved, "authorization-replay")
assert ok, reason
world = loaded
world.advance_ticks(40)
point(5)
assert grant(1)[0]
point(6)
before = world.fingerprint()
result = world.schedule_command(0, worker, world.person(worker)["decisions"] + 1,
                                0, 64, 80, 0, 1000, 1, 2, site)
assert not result[0] and result[1] == "Work requires owner authorization" and world.fingerprint() == before
print(f"Work authorization parity: {checks} C++ checkpoints; rejected commands preserve state")





