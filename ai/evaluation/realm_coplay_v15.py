"""Paired sampled policy evaluation using the current C++ Realm contract."""
import time

import numpy as np
import torch
import polyec_env as core

from realm_dev_baseline_v15 import baseline_action
from realm_model_v15 import ACTIONS, FEATURES, HIDDEN, OBS, STATE, Policy
from realm_coplay_env_v15 import FAMILY_NAMES, RealmCoPlayBatchV15

INTENT_NAMES = (
    "Hold", "Buy", "Sell", "Travel", "Borrow", "Repay", "Project", "Hire",
    "Tax", "Aid", "Investigate", "Offer", "Accept", "Reject", "Work", "Study",
    "Rest", "Social", "Join", "Migrate", "Support", "Appoint", "Mobilize",
    "Resign", "Marry", "CreateEstablishment", "TransferEstablishmentGoods",
    "AuthorizeWorkAtSite", "WorkAtSite", "BuyMilitaryRations", "ShipMilitaryRations",
    "FundMunicipal", "ForeignTrade",
)


def selected_intent(observation, action):
    raw = float(observation[STATE + action * FEATURES]) * 32.0
    intent = int(round(raw))
    if not np.isfinite(raw) or abs(raw - intent) > 1e-3 or not 0 <= intent < len(INTENT_NAMES):
        raise ValueError("Intent encoding does not match Realm v15")
    return intent


def _policy_action(policy, observation, mask, hidden, rng):
    with torch.no_grad():
        logits, _, next_hidden = policy(torch.from_numpy(observation).unsqueeze(0), hidden)
    scores = logits.squeeze(0).cpu().numpy().astype(np.float64)
    scores[mask <= 0.5] = -1e9
    scores -= scores.max()
    probabilities = np.exp(scores)
    probabilities /= probabilities.sum()
    action = int(np.searchsorted(np.cumsum(probabilities), rng.random(), side="right"))
    action = min(action, ACTIONS - 1)
    if mask[action] <= 0.5:
        raise RuntimeError("paired evaluator sampled an illegal action")
    return action, next_hidden


def run_variant(seed, family, mode, policy, regions=6, horizon=None):
    """Run one focal actor to its full horizon; other actors use the locked dev peer."""
    if family not in FAMILY_NAMES or mode not in ("baseline", "learned", "initial", "hold", "random"):
        raise ValueError("unknown family or evaluation mode")
    if regions != 6:
        raise ValueError("R1 sampled evaluation is fixed to the six-region profile")
    expected_horizon = 512 if family == "economic" else 128
    if horizon is None:
        horizon = expected_horizon
    if horizon != expected_horizon:
        raise ValueError("R1 evaluation requires the full focal horizon")
    if mode in ("learned", "initial") and not isinstance(policy, Policy):
        raise ValueError("learned and initial modes require a Realm v15 policy")

    family_id = FAMILY_NAMES.index(family)
    env = RealmCoPlayBatchV15(1, seed, family_id, regions, horizon)
    observation, mask = env.observe()
    observation, mask = observation[0], mask[0]
    actor = int(env.actors[0])
    sampling = np.random.default_rng((int(seed) << 32) ^ actor ^ 0x15A11CE)
    hidden = torch.zeros(1, HIDDEN)
    legal_intent_counts = np.zeros(len(INTENT_NAMES), dtype=np.int64)
    selected_intent_counts = np.zeros(len(INTENT_NAMES), dtype=np.int64)
    action_counts = np.zeros(ACTIONS, dtype=np.int64)
    started = time.perf_counter()
    steps = 0
    done = False
    while not done:
        features = observation[STATE:].reshape(ACTIONS, FEATURES)
        intent_ids = np.rint(features[:, 0] * 32.0).astype(np.int64)
        legal = mask > 0.5
        if not np.any(legal) or np.any(intent_ids < 0) or np.any(intent_ids >= len(INTENT_NAMES)):
            raise RuntimeError("paired evaluator received an invalid C++ observation or mask")
        legal_intent_counts += np.bincount(intent_ids[legal], minlength=len(INTENT_NAMES))
        if mode in ("learned", "initial"):
            action, hidden = _policy_action(policy, observation, mask, hidden, sampling)
        elif mode == "baseline":
            action = int(baseline_action(family_id, observation, mask))
        elif mode == "hold":
            action = 0
            if not legal[action]:
                raise RuntimeError("Hold is not legal in the current Realm view")
        else:
            action = int(sampling.choice(np.flatnonzero(legal)))
        action_counts[action] += 1
        selected_intent_counts[selected_intent(observation, action)] += 1
        next_observation, next_mask, _, dones, _ = env.step(np.asarray([action]), auto_reset=False)
        observation, mask = next_observation[0], next_mask[0]
        steps += 1
        done = bool(dones[0])
        if steps > horizon:
            raise RuntimeError("paired evaluator exceeded the locked focal horizon")
    outcome = env.outcomes()[0]
    if not outcome or not outcome.get("invariant") or outcome.get("decisions") != steps:
        raise RuntimeError("paired evaluator ended without a complete invariant-valid outcome")
    world = env.worlds[0]
    if not world.invariant():
        raise RuntimeError("Realm invariant failed after paired evaluation")
    return {
        "seed": int(seed), "family": family, "mode": mode,
        "decisions": steps, "days": int(outcome["days"]),
        "normalized_utility": float(outcome["raw_utility_delta"] / max(1.0, abs(env.opening[0]))),
        "outcome": outcome, "invariant": True,
        "action_counts": action_counts.tolist(),
        "legal_candidates_by_intent": dict(zip(INTENT_NAMES, legal_intent_counts.tolist())),
        "selected_intents": dict(zip(INTENT_NAMES, selected_intent_counts.tolist())),
        "peer_policy": "realm-v15-observation-dev-v1",
        "elapsed_seconds": time.perf_counter() - started,
    }
