"""Verify the Economic worksite curriculum uses real, legal shared-core states."""
import sys
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "ai" / "training"))
import polyec_env as core  # noqa: E402
from realm_coplay_env_v11 import RealmCoPlayBatch, WORKSITE_CURRICULUM_VERSION  # noqa: E402
from realm_model_v11 import ACTIONS, FEATURES, OBS, STATE  # noqa: E402


batch = RealmCoPlayBatch(16, 1000, 1, 6, 8, worksite_curriculum=WORKSITE_CURRICULUM_VERSION)
observations, masks = batch.observe()
assert observations.shape == (16, OBS) and masks.shape == (16, ACTIONS)
assert {name: batch.scenarios.count(name) for name in set(batch.scenarios)} == {
    "natural": 4,
    "worksite_no_input": 4,
    "worksite_with_fiber": 4,
    "authorized_work_at_site": 4,
}
checks = 1
for index, scenario in enumerate(batch.scenarios):
    world = batch.worlds[index]
    actor = int(batch.actors[index])
    assert world.invariant()
    if scenario == "worksite_no_input":
        assert len(world.establishments()) == 1 and masks[index, 89] == 0.0
        checks += 1
    elif scenario == "worksite_with_fiber":
        assert len(world.establishments()) == 1 and masks[index, 89] == 1.0
        checks += 1
    elif scenario == "authorized_work_at_site":
        legal_work = []
        for worker in range(1, 129):
            person = world.person(worker)
            if person["family"] != 2:
                continue
            features = np.asarray(world.observe(worker), dtype=np.float32)[STATE:].reshape(ACTIONS, FEATURES)
            worker_mask = np.asarray(world.mask(worker), dtype=np.float32)
            legal_work.extend(slot for slot in range(ACTIONS)
                              if worker_mask[slot] > 0.5
                              and round(float(features[slot, 0]) * 32) == 28)
        assert legal_work and world.person(actor)["family"] == 1
        checks += 1

saved = batch.state_dict()
restored = RealmCoPlayBatch(16, 1000, 1, 6, 8, worksite_curriculum=WORKSITE_CURRICULUM_VERSION)
restored.load_state_dict(saved)
restored_observations, restored_masks = restored.observe()
assert restored.scenarios == batch.scenarios
assert np.array_equal(restored_observations, observations)
assert np.array_equal(restored_masks, masks)
checks += 1

default = RealmCoPlayBatch(2, 1000, 1, 6, 8)
assert default.scenarios == ["natural", "natural"]
checks += 1

wrong_version = RealmCoPlayBatch(16, 1000, 1, 6, 8)
try:
    wrong_version.load_state_dict(saved)
except ValueError as error:
    assert "curriculum version mismatch" in str(error)
else:
    raise AssertionError("resume accepted a different curriculum version")
checks += 1

try:
    RealmCoPlayBatch(16, 1000, 0, 6, 8, worksite_curriculum=WORKSITE_CURRICULUM_VERSION)
except ValueError as error:
    assert "only to Economic" in str(error)
else:
    raise AssertionError("worksite curriculum accepted non-Economic role")
checks += 1

print(f"Economic worksite curriculum: {checks} checks passed across 16 contexts")





