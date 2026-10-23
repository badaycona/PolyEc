"""Checks PPO semantic action coverage summaries without running an update."""
import sys
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "ai" / "training"))
from realm_model_v11 import ACTIONS, FEATURES, OBS, STATE  # noqa: E402
from realm_train_v11 import INTENT_NAMES, summarize_action_intents  # noqa: E402


observations = np.zeros((3, OBS), dtype=np.float32)
masks = np.zeros((3, ACTIONS), dtype=np.float32)
actions = np.array([1, 2, 3], dtype=np.int64)
rewards = np.array([0.5, -0.5, 1.0], dtype=np.float32)
advantages = np.array([1.0, -2.0, 3.0], dtype=np.float32)

# Action slots expose Intent in feature 0; one legal candidate per selected Intent.
for row, (slot, intent) in enumerate(((1, 27), (2, 28), (3, 1))):
    observations[row, STATE + slot * FEATURES] = intent / 32.0
    masks[row, slot] = 1.0
    masks[row, 0] = 1.0  # legal Hold opportunity, never selected

summary = summarize_action_intents(observations, masks, actions, rewards, advantages)
assert summary["AuthorizeWorkAtSite"]["legal_candidates"] == 1
assert summary["AuthorizeWorkAtSite"]["decisions"] == 1
assert summary["AuthorizeWorkAtSite"]["advantage_mean"] == 1.0
assert summary["WorkAtSite"]["reward_mean"] == -0.5
assert summary["Buy"]["positive_advantage_fraction"] == 1.0
assert summary["Hold"]["legal_candidates"] == 3
assert summary["Hold"]["decisions"] == 0
assert summary["Hold"]["advantage_mean"] is None
assert sum(item["decisions"] for item in summary.values()) == 3

bad = observations.copy()
bad[0, STATE + FEATURES] = 29 / 32.0
try:
    summarize_action_intents(bad, masks, actions, rewards, advantages)
except ValueError as error:
    assert "encoding drift" in str(error)
else:
    raise AssertionError("invalid Intent encoding was accepted")

print(f"PPO action diagnostics: {len(INTENT_NAMES)} intents, 9 checks passed")





