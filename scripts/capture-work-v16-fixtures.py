"""Freeze migration oracles with the accepted v16 C++ binding, before rebuilding it.

Refuses any newer engine. Fixture generation is explicit test setup; no live
game uses the balanced shortage snapshot edit below.
"""
from pathlib import Path
import hashlib
import json
import tempfile
import polyec_env as core

root = Path(__file__).resolve().parents[1]
folder = root / "core/tests/fixtures/work-v16"
folder.mkdir(parents=True, exist_ok=True)
assert core.Realm(1, 1, 128).serialize("test").startswith("POLYEC_REALM_16 "), "requires accepted v16 engine"
records = []


def save(name, world, bundle="migration-v16", note=""):
    text = world.serialize(bundle)
    assert text.startswith("POLYEC_REALM_16 ") and world.invariant()
    path = folder / (name + ".save")
    path.write_text(text, encoding="utf-8")
    records.append({"file": path.name, "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
                    "fingerprint": world.fingerprint(), "bundle": bundle, "note": note})


def site(world, actor=65, public=False, regions=2):
    region = world.person(actor)["region"]
    owner = (1 if region == regions else region + 2) if public else actor
    result = world.establishment_command(owner, world.person(owner)["decisions"] + 1,
                                         1 if public else 0, region if public else actor,
                                         region, 1, 1, 1 if public else 10)
    assert result[0], result
    if public and actor != owner:
        assert world.work_authorization_command(owner, world.person(owner)["decisions"] + 1,
                                                 result[2], actor, 0, 0, 365000 * 64)[0]
    return result[2]


def work(world, actor, site_id, begin, end):
    result = world.schedule_command(0, actor, world.person(actor)["decisions"] + 1,
                                    begin=begin, end=end, demand_rate=1000, kind=1,
                                    region=world.person(actor)["region"], establishment_id=site_id)
    assert result[0], result


def shortage(world, actor=65):
    words = world.serialize("migration-v16").split()
    people, start = world.view()["persons"], None
    for index in range(100, len(words) - 1 - 56 * people):
        if words[index] == str(people) and all(words[index + 1 + 56 * p] == str(p + 1) for p in range(people)):
            start = index + 1
            break
    assert start is not None
    region = world.person(actor)["region"]
    firm = start + people * 56 + 1 + 66 * region + 1
    cash = start + 56 * (actor - 1) + 16
    words[cash] = str(int(words[cash]) + int(words[firm]) - 1)
    words[firm] = "1"
    ok, reason = world.deserialize(" ".join(words), "migration-v16")
    assert ok, reason


world = core.Realm(37, 6, 256)
first = site(world, 129, True, regions=6)
work(world, 129, first, 0, 64)
assert world.schedule_command(0, 129, world.person(129)["decisions"] + 1,
                              begin=0, end=64, demand_rate=500)[0]
region = world.person(130)["region"]
owner = 1 if region == 6 else region + 2
assert world.work_authorization_command(owner, world.person(owner)["decisions"] + 1,
                                         first, 130, 0, 0, 365000 * 64)[0]
result = world.schedule_command(0, 130, 1, begin=0, end=64, demand_rate=500,
                                kind=1, region=region, establishment_id=first)
assert result[0], result
world.advance_ticks(64)
assert world.paid_work_units_by_region(129, region) == 4 and world.paid_work_units_by_region(130, region) == 4
save("codec-cs", world, "test", "Actual v16 regional payroll; public site retains its one funded cash")

for name, lengths, drain in [("partial-site", (8,), False), ("split-paid", (8, 8), False),
                             ("legacy-debt", (32, 16), True)]:
    world = core.Realm(71, 2, 128)
    sites = [site(world) for _ in lengths]
    begin = 0
    for site_id, length in zip(sites, lengths):
        work(world, 65, site_id, begin, begin + length)
        begin += length
    if drain:
        shortage(world)
    world.advance_ticks(64)
    assert world.paid_work_units_by_region(65, world.person(65)["region"]) == (1 if name != "partial-site" else 0)
    save(name, world, note="Balanced firm-to-person test snapshot before actual v16 shortage payroll" if drain else "Actual v16 command/attendance/payroll")

world = core.Realm(71, 2, 128)
site_id = site(world)
for begin, end in [(0, 8), (128, 136)]:
    assert world.schedule_command(0, 65, world.person(65)["decisions"] + 1,
                                  begin=begin, end=end, demand_rate=1000)[0]
words = world.serialize("migration-v16").split()
schedule = words.index("SCHEDULE_5") + 1
for row in range(int(words[schedule + 4])):
    session = schedule + 10 + 17 * row
    words[session + 14] = "1"  # literal pre-site Work, accepted by v16 reader
    words[session + 15] = str(world.person(65)["region"])
assert world.deserialize(" ".join(words), "migration-v16")[0]
work(world, 65, site_id, 8, 16)
world.advance_ticks(64)
assert world.work_present_ticks_by_region(65, world.person(65)["region"]) == 16
assert world.paid_work_units_by_region(65, world.person(65)["region"]) == 1
save("mixed-unassigned", world, note="Literal historical unassigned Work incl. future accepted shift; v16 reader validates, actual v16 payroll")

with tempfile.TemporaryDirectory(prefix="polyec-v16-credit-") as directory:
    text = (root / "config/gameplay.toml").read_text(encoding="utf-8").replace("wage_per_unit = 1", "wage_per_unit = 3")
    config = Path(directory) / "gameplay.toml"
    config.write_text(text, encoding="utf-8")
    world = core.Realm(71, 2, 128, config_path=str(config))
    sites = [site(world), site(world)]
    work(world, 65, sites[0], 0, 8)
    work(world, 65, sites[1], 8, 16)
    shortage(world)
    world.advance_ticks(64)
    assert world.work_pay_remainder(65, world.person(65)["region"]) == 1
    save("split-partial-money", world, note="Wage3, balanced shortage fixture, actual v16 gross1 payment")

manifest = {"source_checkpoint": "m5-work-authorization-4a52a99cdf02", "engine_schema": 16,
            "binding_sha256": hashlib.sha256(Path(core.__file__).read_bytes()).hexdigest(),
            "generator_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(), "fixtures": records}
(folder / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
print(json.dumps({"captured_v16_fixtures": len(records), "binding_sha256": manifest["binding_sha256"]}))
