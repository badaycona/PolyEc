"""Tensor and candidate-mask checks for the current Realm policy architecture."""
import sys
from pathlib import Path

import torch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "training"))
from realm_model_v15 import ACTIONS, FEATURES, HIDDEN, OBS, STATE, Policy, masked_distribution


assert (STATE, ACTIONS, FEATURES, OBS) == (382, 264, 12, 3550)
policy = Policy().eval()
observations = torch.zeros(2, OBS)
hidden = torch.zeros(2, HIDDEN)
with torch.no_grad():
    logits, values, next_hidden = policy(observations, hidden)
assert logits.shape == (2, ACTIONS)
assert values.shape == (2,)
assert next_hidden.shape == (2, HIDDEN)

sequence = torch.zeros(2, 16, OBS)
sequence_hidden = torch.zeros(2, HIDDEN)
with torch.no_grad():
    sequence_logits, sequence_values, sequence_next = policy.sequence(sequence, sequence_hidden)
assert sequence_logits.shape == (2, 16, ACTIONS)
assert sequence_values.shape == (2, 16)
assert sequence_next.shape == (2, HIDDEN)

mask = torch.zeros_like(logits)
mask[:, 0] = 1
mask[:, ACTIONS - 1] = 1
distribution = masked_distribution(logits, mask)
sampled = distribution.sample()
assert torch.all(mask[torch.arange(len(sampled)), sampled] > 0.5)

try:
    masked_distribution(logits, torch.zeros_like(mask))
except ValueError:
    pass
else:
    raise AssertionError("the current policy accepted an empty legal-action mask")

print("REALM V15 MODEL: tensor shape, recurrent sequence, legal-action sampling, and empty-mask rejection passed")
