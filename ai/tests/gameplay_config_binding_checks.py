"""Shared C++ snapshot/parser and production replay; no Python economic rules."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile

import polyec_env as core

parser = argparse.ArgumentParser()
parser.add_argument("--replay", required=True)
args = parser.parse_args()
template = (Path(__file__).resolve().parents[2] / "config/gameplay.toml").read_text(encoding="utf-8")
changed = template.replace("ticks_per_unit = 16", "ticks_per_unit = 8").replace(
    "wage_per_unit = 1", "wage_per_unit = 3"
).replace("default_capacity = 1", "default_capacity = 2").replace(
    "fiber_input = 1", "fiber_input = 2"
).replace("cloth_output = 1", "cloth_output = 3")
checks = 0
with tempfile.TemporaryDirectory(prefix="polyec-config-parity-") as directory:
    config = Path(directory) / "gameplay.toml"
    config.write_text(changed, encoding="utf-8")
    expected = [list(map(int, row.split())) for row in subprocess.check_output(
        [args.replay, "--replay", str(config)], text=True
    ).splitlines()]
    world = core.Realm(71, 2, 128, config_path=str(config))
    assert world.model_contract == "realm-household-fund-goods30-config-v48"
    timeline = core.Realm(71, 2, 128, config_path=str(config))
    ok, reason, _ = timeline.schedule_command(0, 1, 1, 0, 0, 1, 0, 5)
    assert ok, reason
    assert timeline.model_contract == "realm-worksite-payroll-goods30-config-v48"
    checks += 3
    worker = 65
    region = world.person(worker)["region"]
    owner = 1 if region == 2 else region + 2
    result = world.establishment_command(owner, world.person(owner)["decisions"] + 1,
                                         1, region, region, 1, -1, 30)
    assert result[0], result
    site = result[2]
    assert world.establishment_transfer(owner, world.person(owner)["decisions"] + 1,
                                         site, 2, 9, 5)[0]
    assert world.work_authorization_command(owner, world.person(owner)["decisions"] + 1,
                                             site, worker, 0, 0, 32)[0]
    result = world.schedule_command(0, worker, world.person(worker)["decisions"] + 1,
                                    begin=0, end=32, demand_rate=1000,
                                    kind=1, region=region, establishment_id=site)
    assert result[0], result
    assert world.config_values == [8, 3, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1000, 1000, 1, 1000000, 1, 1]
    assert not world.pilot_policy_compatible
    checks += 5
    previous = 0
    for tick, fingerprint, config_fingerprint, fiber, cloth, units, remainder in expected:
        world.advance_ticks(tick - previous)
        previous = tick
        observed = world.establishments()[0]
        assert world.fingerprint() == fingerprint
        assert world.config_fingerprint == config_fingerprint
        assert observed["stock"][9] == fiber and observed["stock"][11] == cloth
        assert observed["wage_remainders"].get(worker, 0) == remainder
        assert world.invariant()
        # The core owns the watermarks; their serialized state is already part
        # of the full fingerprint comparison rather than recomputed here.
        checks += 5
        if tick == 16:
            saved = world.serialize("config-test")
    assert observed["stock"][9] == 1 and observed["stock"][11] == 6
    checks += 1
    config.write_text("schema_version = 999\n", encoding="utf-8")
    restored = core.Realm(71, 2, 128)
    assert restored.deserialize(saved, "config-test")[0]
    restored.advance_ticks(128 - 16)
    assert restored.fingerprint() == world.fingerprint()
    assert restored.config_fingerprint == world.config_fingerprint
    checks += 3
    try:
        core.Realm(71, 2, 128, config_path=str(config))
    except ValueError as error:
        assert "schema_version" in str(error)
        checks += 1
    else:
        raise AssertionError("invalid config created a world")
    config.write_bytes(changed.encode("utf-8") + b"# invalid comment \xff\n")
    try:
        core.Realm(71, 2, 128, config_path=str(config))
    except ValueError as error:
        assert "$encoding" in str(error)
        checks += 1
    else:
        raise AssertionError("invalid UTF-8 in a config comment was accepted")
print(json.dumps({"checks": checks, "cpp_python_checkpoints": len(expected),
                  "saved_snapshot_after_file_change": True,
                  "config_fingerprint": world.config_fingerprint}))







