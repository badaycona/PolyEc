"""Labeled observation-only development controllers; never a learned policy."""
import numpy as np

from realm_model_v13 import ACTIONS, STATE

BASELINE_VERSION = "realm-v13-observation-dev-v1"


def baseline_action(family, observation, mask):
    scores = np.zeros(ACTIONS, dtype=np.float32)
    if family == 0:
        scores[2] = 0.2
        scores[4] = 2
        scores[5] = 1.5
        scores[6:13] = 4 if observation[99] < 0.9 else 0
    elif family == 1:
        features = observation[STATE:].reshape(ACTIONS, 12)
        scores[1:8] = 1 - features[1:8, 8]
        scores[8:15] = features[8:15, 8] - 1
        scores[30] = 0.3 * (1 + np.floor(observation[4] * 4))
        scores[32:52] = 1 - features[32:52, 8]
        scores[52:72] = features[52:72, 8] - 1
    elif family == 2:
        scores[1:8] = 1
        scores[15] = 3 if observation[1] < 0.18 else 0.5
        scores[16] = 4 if observation[3] < 0.6 else 0
    else:
        raise ValueError("unknown Realm family")
    legal_scores = np.where(mask > 0.5, scores, -1e9)
    action = int(legal_scores.argmax())
    if mask[action] <= 0.5:
        raise RuntimeError("development baseline selected an illegal action")
    return action

