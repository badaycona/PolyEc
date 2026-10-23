"""Small adapter checks for the full-world sampled cross-play evaluator."""
import numpy as np
import torch
import polyec_env as core
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "training"))
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "evaluation"))
from realm_model_v11 import ACTIONS, FEATURES, OBS, STATE, Policy
from realm_coplay_v11 import (INTENT_AUTHORIZE_WORK_AT_SITE, INTENT_TRANSFER_ESTABLISHMENT_GOODS,
                              INTENT_WORK_AT_SITE, baseline_action, run_variant, sample_action,
                              selected_action_good, selected_action_intent)

assert core.REALM_SCHEMA == "realm-v11" and core.REALM_ACTIONS == ACTIONS and core.REALM_OBS == OBS
world = core.Realm(173000, 2, 128)
checks = 1
for family in range(3):
    actor = next(person_id for person_id in range(1, 129)
                 if world.person(person_id)["alive"] and world.person(person_id)["family"] == family)
    obs = np.asarray(world.observe(actor), dtype=np.float32)
    mask = np.asarray(world.mask(actor), dtype=np.float32)
    assert obs.shape == (OBS,) and mask.shape == (ACTIONS,) and mask.sum() > 0
    action = baseline_action(family, obs, mask)
    assert mask[action] > .5
    checks += 2

    torch.manual_seed(173000 + family)
    policy = Policy().eval()
    rng_a = np.random.default_rng(99)
    rng_b = np.random.default_rng(99)
    selected_a, hidden_a = sample_action(policy, obs, mask, torch.zeros(1, 64), rng_a)
    selected_b, hidden_b = sample_action(policy, obs, mask, torch.zeros(1, 64), rng_b)
    assert selected_a == selected_b and mask[selected_a] > .5
    assert torch.equal(hidden_a, hidden_b)
    checks += 2

owner = next(person_id for person_id in range(1, 129)
             if world.person(person_id)["family"] == 1 and world.person(person_id)["cash"] >= 25)
ok, reason = world.step(owner, 80)
assert ok, reason
site = world.establishments()[0]
ok, reason, _ = world.schedule_command(
    0, owner, world.person(owner)["decisions"] + 1, 0, world.game_tick,
    world.game_tick + 1, 0, 5, 0, -1, 0
)
assert ok, reason
assert len(world.observe(owner)) == OBS and len(world.mask(owner)) == ACTIONS
checks += 3
for slot, intent in ((89, INTENT_TRANSFER_ESTABLISHMENT_GOODS),
                     (92, INTENT_AUTHORIZE_WORK_AT_SITE), (93, INTENT_WORK_AT_SITE),
                     (100, INTENT_TRANSFER_ESTABLISHMENT_GOODS)):
    encoded = np.zeros(OBS, dtype=np.float32)
    encoded[STATE + slot * FEATURES] = intent / 32.0
    assert selected_action_intent(encoded, slot) == intent
assert selected_action_intent(encoded, 100) == INTENT_TRANSFER_ESTABLISHMENT_GOODS
encoded[STATE + 100 * FEATURES + 2] = 9 / 26.0
assert selected_action_good(encoded, 100) == 9
assert all(selected_action_intent(obs, slot) == 24 for slot in range(72, 80))
assert all(selected_action_intent(obs, slot) in (INTENT_TRANSFER_ESTABLISHMENT_GOODS,
                                                  INTENT_AUTHORIZE_WORK_AT_SITE,
                                                  INTENT_WORK_AT_SITE)
           for slot in range(89, ACTIONS))
bad_encoding = encoded.copy()
bad_encoding[STATE + 100 * FEATURES] = 29 / 32.0
try:
    selected_action_intent(bad_encoding, 100)
except ValueError as error:
    assert "encoding drift" in str(error)
else:
    raise AssertionError("out-of-range semantic action was accepted")
checks += 9
flow_run = run_variant(173001, 2, 1, {}, {"learned": {}, "initial": {}}, 173001)
for family_name in ("governance", "economic", "civilian"):
    assert sum(flow_run["selected_intent_counts_by_family"][family_name].values()) == sum(
        flow_run["action_counts_by_family"][family_name]
    )
    assert all(isinstance(value, int) for value in
               flow_run["immediate_cash_delta_by_intent_by_family"][family_name].values())
checks += 3
print(f"REALM V11 CROSS-PLAY ADAPTER: {checks} checks passed; schema={core.REALM_SCHEMA}; site={site['id']}; post-timeline candidates available")





