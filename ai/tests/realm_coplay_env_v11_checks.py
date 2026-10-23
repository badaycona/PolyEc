"""Verify full Realm training co-play and immutable frozen peer weights."""
import sys
from time import perf_counter
from pathlib import Path

import numpy as np
import torch
import polyec_env as core

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "training"))
from realm_model_v11 import ACTIONS, OBS, Policy
from realm_dev_baseline_v11 import BASELINE_VERSION, baseline_action
from realm_coplay_env_v11 import RealmCoPlayBatch

assert core.REALM_SCHEMA == "realm-v11" and core.REALM_ACTIONS == ACTIONS and core.REALM_OBS == OBS
assert core.REALM_REWARD_VERSION == "realm-v11-person-enterprise-equity-v1"
checks = 1
start = perf_counter()

first = RealmCoPlayBatch(1, 173000, 1, 2, 1)
second = RealmCoPlayBatch(1, 173000, 1, 2, 1)
actor = int(first.actors[0])
assert first.worlds[0].person(actor)["family"] == 1
obs, mask = first.observe()
assert obs.shape == (1, OBS) and mask.shape == (1, ACTIONS) and mask[0, 0] > .5
checks += 2

civilian = RealmCoPlayBatch(1, 173002, 2, 6, 128)
civilian_actor = int(civilian.actors[0])
assert civilian.worlds[0].person(civilian_actor)["family"] == 2
assert civilian.worlds[0].observe(civilian_actor)[102] <= 0.5
assert civilian.worlds[0].invariant()
checks += 3

for family in range(3):
    peer = next(person_id for person_id in range(1, 129)
                if first.worlds[0].person(person_id)["family"] == family)
    peer_obs = np.asarray(first.worlds[0].observe(peer), dtype=np.float32)
    peer_mask = np.asarray(first.worlds[0].mask(peer), dtype=np.float32)
    choice = baseline_action(family, peer_obs, peer_mask)
    assert peer_mask[choice] > .5
    assert first._baseline_peer(0, family, peer, peer_mask) == choice
    checks += 2

next_a = first.step(np.asarray([0], dtype=np.int64), auto_reset=False)
next_b = second.step(np.asarray([0], dtype=np.int64), auto_reset=False)
for a, b in zip(next_a, next_b):
    assert np.array_equal(a, b)
checks += 1
assert next_a[2].shape == (1,) and next_a[3].tolist() == [True]
assert first.worlds[0].invariant() and first.outcomes()[0]["invariant"]
assert first.outcomes()[0]["decisions"] == 1
checks += 2

torch.manual_seed(173000)
peer_model = Policy().eval()
peer_before = {name: value.detach().clone() for name, value in peer_model.state_dict().items()}
contract_world = core.Realm(1, 2, 128)
peer_metadata = {1: {"family": "economic", "schema": core.REALM_SCHEMA, "observation": OBS, "actions": ACTIONS,
                     "hidden": 64, "model_contract": contract_world.model_contract,
                     "config_fingerprint": contract_world.config_fingerprint,
                     "reward_version": core.REALM_REWARD_VERSION}}
with_frozen_peer = RealmCoPlayBatch(1, 173001, 1, 2, 1, partner_policies={1: peer_model}, partner_metadata=peer_metadata)
frozen_replay = RealmCoPlayBatch(1, 173001, 1, 2, 1, partner_policies={1: peer_model}, partner_metadata=peer_metadata)
resumed = RealmCoPlayBatch(1, 173099, 1, 2, 1, partner_policies={1: peer_model}, partner_metadata=peer_metadata)
resumed.load_state_dict(with_frozen_peer.state_dict())
focal_mask = np.asarray(with_frozen_peer.worlds[0].mask(int(with_frozen_peer.actors[0])))
assert focal_mask[0] > .5
step_started = perf_counter()
frozen_step = with_frozen_peer.step(np.asarray([0], dtype=np.int64), auto_reset=False)
step_seconds = perf_counter() - step_started
replay_step = frozen_replay.step(np.asarray([0], dtype=np.int64), auto_reset=False)
resumed_step = resumed.step(np.asarray([0], dtype=np.int64), auto_reset=False)
for a, b in zip(frozen_step, replay_step):
    assert np.array_equal(a, b)
for a, b in zip(frozen_step, resumed_step):
    assert np.array_equal(a, b)
assert frozen_step[3].tolist() == [True] and with_frozen_peer.worlds[0].invariant()
assert with_frozen_peer.peer_decisions > 0
assert with_frozen_peer.outcomes() == frozen_replay.outcomes()
assert with_frozen_peer.outcomes() == resumed.outcomes()
assert all(torch.equal(peer_before[name], value) for name, value in peer_model.state_dict().items())
assert all(not parameter.requires_grad for parameter in with_frozen_peer.policies[1].parameters())
checks += 7
assert with_frozen_peer.state_dict()["baseline_version"] == BASELINE_VERSION
assert with_frozen_peer.state_dict()["reward_version"] == core.REALM_REWARD_VERSION
bad_state = with_frozen_peer.state_dict()
bad_state["baseline_version"] = "stale-baseline"
try:
    resumed.load_state_dict(bad_state)
except ValueError:
    checks += 1
else:
    raise AssertionError("resume accepted a different peer baseline version")

bad_reward_state = with_frozen_peer.state_dict()
bad_reward_state["reward_version"] = "old-reward"
try:
    resumed.load_state_dict(bad_reward_state)
except ValueError:
    checks += 1
else:
    raise AssertionError("resume accepted a different reward version")

bad_metadata = {1: {**peer_metadata[1], "family": "civilian"}}
try:
    RealmCoPlayBatch(1, 173002, 1, 2, 1, partner_policies={1: peer_model}, partner_metadata=bad_metadata)
except ValueError:
    checks += 1
else:
    raise AssertionError("mismatched frozen peer family was accepted")

elapsed = perf_counter() - start
print(f"REALM V11 FROZEN-PEER ENVIRONMENT: {checks} checks passed; schema={core.REALM_SCHEMA}; paired_peer_decisions={with_frozen_peer.peer_decisions}; transition_seconds={step_seconds:.3f}; setup_and_checks_seconds={elapsed:.3f}")





