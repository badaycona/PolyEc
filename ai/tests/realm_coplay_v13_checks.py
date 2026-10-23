"""Current-contract C++ binding, action, and serialized adapter checks."""
import sys
import json
import tempfile
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "training"))
import polyec_env as core
from realm_coplay_env_v13 import RealmCoPlayBatchV13, TIMELINE_CONTRACT_FIXTURE_VERSION
from realm_model_v13 import ACTIONS, OBS
from realm_train_v13 import load_peer


env = RealmCoPlayBatchV13(1, 730001, 1, 1, 3)
assert env.model_contract == "realm-worksite-payroll-goods27-config-v33"
trade_world = core.Realm(730002, 26, 128)
partners = trade_world.foreign_trade_partners
assert len(partners) == 3
assert [int(row["partner_id"]) for row in partners] == [0, 1, 2]
assert [int(row["money"]) for row in partners] == [5000, 5000, 5000]
assert [int(row["goods"][0]) for row in partners] == [32, 32, 32]
assert [int(row["salt_capacity"]) for row in partners] == [32, 32, 32]
obs, masks = env.observe()
assert obs.shape == (1, OBS) and masks.shape == (1, ACTIONS)
assert np.isfinite(obs).all() and masks.sum() > 0
action = int(np.flatnonzero(masks[0] > 0.5)[0])
_, _, rewards, dones, _ = env.step(np.asarray([action]), auto_reset=False)
assert rewards.shape == (1,) and np.isfinite(rewards).all() and dones.shape == (1,)

saved = env.state_dict()
assert saved["timeline_contract_fixture"] == TIMELINE_CONTRACT_FIXTURE_VERSION
restored = RealmCoPlayBatchV13(1, 990001, 1, 1, 3)
restored.load_state_dict(saved)
restored_obs, restored_masks = restored.observe()
current_obs, current_masks = env.observe()
assert np.array_equal(current_obs, restored_obs)
assert np.array_equal(current_masks, restored_masks)
assert restored.worlds[0].model_contract == env.model_contract

bad_fixture = dict(saved)
bad_fixture["timeline_contract_fixture"] = "old-or-unknown"
try:
    restored.load_state_dict(bad_fixture)
except ValueError as error:
    assert "fixture version" in str(error)
else:
    raise AssertionError("resume accepted a different timeline contract fixture")

with tempfile.TemporaryDirectory() as directory:
    legacy_bundle = Path(directory) / "v11"
    legacy_bundle.mkdir()
    (legacy_bundle / "manifest.json").write_text(json.dumps({
        "family": "governance", "schema": "realm-v11", "observation": 1676,
        "actions": 129, "hidden": 64,
        "model_contract": "realm-household-fund-goods27-v5",
    }), encoding="utf-8")
    try:
        load_peer(legacy_bundle, 0, 1)
    except ValueError as error:
        assert "does not match" in str(error)
    else:
        raise AssertionError("the current trainer accepted a legacy v11 peer bundle")

    old_contract_bundle = Path(directory) / "v12"
    old_contract_bundle.mkdir()
    (old_contract_bundle / "manifest.json").write_text(json.dumps({
        "family": "governance", "schema": "realm-v27", "observation": 2935,
        "actions": 220, "hidden": 64,
        "model_contract": "realm-worksite-payroll-goods27-config-v32",
    }), encoding="utf-8")
    try:
        load_peer(old_contract_bundle, 0, 26)
    except ValueError as error:
        assert "does not match" in str(error)
    else:
        raise AssertionError("the v13 trainer accepted a v12 peer bundle")

print("REALM V13 COPLAY: current C++ contract, legal action transition, serialized resume, and legacy peer rejection passed")
