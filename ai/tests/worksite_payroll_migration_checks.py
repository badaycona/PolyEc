import hashlib
import json
import pathlib
import sys

import polyec_env as core

root = pathlib.Path(__file__).resolve().parents[2]
fixture_dir = root / "core" / "tests" / "fixtures" / "work-v16"
manifest = json.loads((fixture_dir / "manifest.json").read_text(encoding="utf-8"))
checks = 0
rows = {}

for item in manifest["fixtures"]:
    data = (fixture_dir / item["file"]).read_bytes()
    assert hashlib.sha256(data).hexdigest() == item["sha256"], item["file"]
    source = data.decode("utf-8")
    world = core.Realm(1, 1, 128)
    ok, error = world.deserialize(source, item["bundle"])
    assert ok, (item["file"], error)
    assert world.invariant(), item["file"]
    fingerprint = world.fingerprint()
    saved = world.serialize(item["bundle"])
    restored = core.Realm(1, 1, 128)
    roundtrip, roundtrip_error = restored.deserialize(saved, item["bundle"])
    assert roundtrip, (item["file"], roundtrip_error)
    assert restored.fingerprint() == fingerprint, item["file"]
    rows[pathlib.Path(item["file"]).stem] = world
    checks += 5

partial = rows["partial-site"]
assert not partial.work_credits
assert not partial.establishments()[0]["legacy_credit"]
checks += 2

for name in ("split-paid", "mixed-unassigned", "split-partial-money"):
    world = rows[name]
    assert len(world.work_credits) == 1
    (key, credit), = world.work_credits.items()
    assert key[0] == 65 and credit == (1, 0, 1), (name, key, credit)
    checks += 2

debt = rows["legacy-debt"]
assert len(debt.establishments()) == 2
assert [site["legacy_credit"].get(65, 0) for site in debt.establishments()] == [2, 1]
assert list(debt.work_credits.values()) == [(3, 0, 0)]
checks += 2

print(json.dumps({"checks": checks, "frozen_v16_saves": len(rows),
                  "migration_parity": "Python binding preserves frozen v16 migration ledger and roundtrip state",
                  "schema": core.REALM_SCHEMA}, sort_keys=True))





