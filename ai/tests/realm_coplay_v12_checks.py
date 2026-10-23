"""Current-contract C++ binding, action, and serialized adapter checks."""
import sys
import json
import tempfile
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "training"))
from realm_coplay_env_v12 import RealmCoPlayBatchV12, TIMELINE_CONTRACT_FIXTURE_VERSION
from realm_model_v12 import ACTIONS, OBS
from realm_train_v12 import load_peer


env = RealmCoPlayBatchV12(1, 730001, 1, 1, 3)
assert env.model_contract == "realm-worksite-payroll-goods27-config-v32"
obs, masks = env.observe()
assert obs.shape == (1, OBS) and masks.shape == (1, ACTIONS)
assert np.isfinite(obs).all() and masks.sum() > 0
action = int(np.flatnonzero(masks[0] > 0.5)[0])
_, _, rewards, dones, _ = env.step(np.asarray([action]), auto_reset=False)
assert rewards.shape == (1,) and np.isfinite(rewards).all() and dones.shape == (1,)

saved = env.state_dict()
assert saved["timeline_contract_fixture"] == TIMELINE_CONTRACT_FIXTURE_VERSION
restored = RealmCoPlayBatchV12(1, 990001, 1, 1, 3)
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
    legacy_bundle = Path(directory)
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

print("REALM V12 COPLAY: current C++ contract, legal action transition, serialized resume, and legacy peer rejection passed")
