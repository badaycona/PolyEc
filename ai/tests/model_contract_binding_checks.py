"""Verify Python's C++ contract label and preserve legacy pilot classification."""
import argparse
import json
from pathlib import Path

import polyec_env as core


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("models", type=Path, nargs="?", default=Path("godot_project/models/realm-v6"))
    args = parser.parse_args()
    checks = 0
    world = core.Realm(37, 6, 128)
    assert world.model_contract == "realm-household-fund-goods30-config-v50", world.model_contract
    assert core.REALM_SCHEMA == "realm-v39" and core.REALM_ACTIONS == 400 and core.REALM_OBS == 5182
    checks += 2
    civilians_without_tea = [actor for actor in range(1, 129)
                             if world.person(actor)["family"] == 2 and world.person(actor)["alive"]]
    assert civilians_without_tea and all(world.mask(actor)[24] == 0.0 for actor in civilians_without_tea)
    checks += 1
    assert world.observe(1)[88:90] == [0.0, 0.0]
    checks += 1
    timeline = core.Realm(37, 6, 128)
    ok, reason, _ = timeline.schedule_command(0, 1, 1, 0, 0, 1, 0, 5)
    assert ok, reason
    assert timeline.model_contract == "realm-worksite-payroll-goods30-config-v50", timeline.model_contract
    assert timeline.observe(1)[108:110] == [0.0, 0.0]
    assert timeline.observe(1)[88:90] == [0.0, 0.0]
    traits = world.observe(1)[110:116]
    assert len(traits) == 6 and all(0.25 <= value <= 0.75 for value in traits), traits
    assert "personality" not in world.person(1) and "traits" not in world.person(1)
    checks += 5
    for family in ("governance", "economic", "civilian"):
        manifest = json.loads((args.models / family / "manifest.json").read_text(encoding="utf-8"))
        assert manifest["family"] == family
        assert manifest["schema"] == "realm-v6"
        assert "model_contract" not in manifest, "historical pilot manifest must remain untouched"
        assert manifest["trained_with_rl"] is True and manifest["imitation"] is False
        assert manifest["full_R1_pass"] is False
        checks += 4
    print(f"model_contract_binding_checks={checks} legacy_pilot=realm-v6 current={world.model_contract}")


if __name__ == "__main__":
    main()
