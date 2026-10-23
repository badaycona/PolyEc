"""Inspect sampled v11 policy choices in controlled C++ Realm workshop contexts."""
import argparse
import hashlib
import json
from pathlib import Path

import numpy as np
import torch
import polyec_env as core

from realm_model_v11 import ACTIONS, FEATURES, HIDDEN, OBS, STATE, Policy, masked_distribution

INTENT_NAMES = (
    "Hold", "Buy", "Sell", "Travel", "Borrow", "Repay", "Project", "Hire",
    "Tax", "Aid", "Investigate", "Offer", "Accept", "Reject", "Work", "Study",
    "Rest", "Social", "Join", "Migrate", "Support", "Appoint", "Mobilize",
    "Resign", "Marry", "CreateEstablishment", "TransferEstablishmentGoods",
    "AuthorizeWorkAtSite", "WorkAtSite",
)
REWARD_VERSION = "realm-v11-person-enterprise-equity-v1"
MODEL_CONTRACT = core.Realm(1, 6, 128).model_contract


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def contexts():
    world = core.Realm(370221, 6, 256)
    actor = next(
        person_id for person_id in range(1, 257)
        if world.person(person_id)["family"] == 1 and world.person(person_id)["cash"] >= 100
    )
    result = []

    def capture(name):
        observation = np.asarray(world.observe(actor), dtype=np.float32)
        mask = np.asarray(world.mask(actor), dtype=np.float32)
        if observation.shape != (OBS,) or mask.shape != (ACTIONS,) or mask.sum() <= 0:
            raise RuntimeError(f"malformed {name} observation or mask")
        if not world.invariant():
            raise RuntimeError(f"Realm invariant failed in {name}")
        transfer_feature = observation[STATE + 89 * FEATURES:STATE + 90 * FEATURES]
        result.append({
            "name": name,
            "observation": observation,
            "mask": mask,
            "day": world.day,
            "actor": actor,
            "cash": world.person(actor)["cash"],
            "transfer_candidate_legal": bool(mask[89] > 0.5),
            "transfer_fiber_inventory_feature": float(transfer_feature[10]),
            "establishment_count": len(world.establishments()),
            "invariant": world.invariant(),
        })

    capture("starting_world")
    ok, reason = world.step(actor, 80)
    if not ok or len(world.establishments()) != 1:
        raise RuntimeError(f"controlled Cloth workshop creation failed: {reason}")
    if world.mask(actor)[89] != 0.0:
        raise RuntimeError("Fiber transfer became legal without personal Fiber inventory")
    capture("cloth_workshop_without_fiber")

    observation = np.asarray(world.observe(actor), dtype=np.float32)
    fiber_candidates = [
        slot for slot in range(ACTIONS)
        if world.mask(actor)[slot] > 0.5
        and round(float(observation[STATE + slot * FEATURES]) * 32) == 1
        and round(float(observation[STATE + slot * FEATURES + 2]) * 26) == 9
    ]
    if not fiber_candidates:
        raise RuntimeError("controlled context has no legal canonical Fiber purchase")
    ok, reason = world.step(actor, fiber_candidates[0])
    if not ok or world.mask(actor)[89] != 1.0:
        raise RuntimeError(f"Fiber purchase did not expose transfer action: {reason}")
    capture("cloth_workshop_with_fiber")
    return result


def audit_policy(model_path, manifest_path, expected_seed, cases):
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    if (manifest.get("family") != "economic" or manifest.get("schema") != core.REALM_SCHEMA
            or manifest.get("model_contract") != MODEL_CONTRACT
            or manifest.get("reward_version") != REWARD_VERSION
            or manifest.get("seed") != expected_seed
            or manifest.get("minibatch_sequences") != 16
            or manifest.get("complete") is not True
            or manifest.get("training_complete") is not True
            or manifest.get("peer_eligible") is not False
            or manifest.get("selected_seed") is not None):
        raise ValueError(f"seed {expected_seed} is not the expected completed, non-selected v20 diagnostic")
    policy = Policy().eval()
    policy.load_state_dict(torch.load(model_path, map_location="cpu", weights_only=True))
    result = {
        "seed": expected_seed,
        "policy_sha256": sha256(model_path),
        "manifest_sha256": sha256(manifest_path),
        "peer_eligible": False,
        "contexts": {},
    }
    for case_index, case in enumerate(cases):
        observation = torch.from_numpy(case["observation"]).unsqueeze(0)
        mask = torch.from_numpy(case["mask"]).unsqueeze(0)
        with torch.no_grad():
            logits, _, _ = policy(observation, torch.zeros(1, HIDDEN))
            distribution = masked_distribution(logits, mask)
            probabilities = distribution.probs.squeeze(0).cpu().numpy()
        if not np.isclose(probabilities.sum(), 1.0, atol=1e-6):
            raise RuntimeError("masked categorical probabilities do not sum to one")
        features = case["observation"][STATE:].reshape(ACTIONS, FEATURES)
        intents = np.rint(features[:, 0] * 32).astype(np.int64)
        if np.any(intents < 0) or np.any(intents >= len(INTENT_NAMES)):
            raise RuntimeError("Realm Intent observation encoding drift")
        by_intent = {}
        for intent, name in enumerate(INTENT_NAMES):
            legal = case["mask"] > 0.5
            selected = intents == intent
            by_intent[name] = {
                "legal_candidates": int((legal & selected).sum()),
                "probability_mass": float(probabilities[selected].sum()),
            }
        generator = torch.Generator(device="cpu").manual_seed(expected_seed * 1000 + case_index)
        samples = torch.multinomial(distribution.probs.squeeze(0), 256, replacement=True, generator=generator).numpy()
        counts = np.bincount(intents[samples], minlength=len(INTENT_NAMES))
        for intent, name in enumerate(INTENT_NAMES):
            by_intent[name]["sampled_decisions"] = int(counts[intent])
        fiber_buy = ((intents == 1)
                     & (np.rint(features[:, 2] * 26).astype(np.int64) == 9))
        fiber_transfer = np.zeros(ACTIONS, dtype=bool)
        fiber_transfer[89] = True
        focused = {}
        for name, candidate_mask in (("BuyFiber", fiber_buy), ("TransferFiber", fiber_transfer)):
            candidate_mask &= case["mask"] > 0.5
            focused[name] = {
                "legal_candidates": int(candidate_mask.sum()),
                "probability_mass": float(probabilities[candidate_mask].sum()),
                "sampled_decisions": int(candidate_mask[samples].sum()),
            }
        result["contexts"][case["name"]] = {
            "day": case["day"],
            "actor": case["actor"],
            "cash": case["cash"],
            "transfer_candidate_legal": case["transfer_candidate_legal"],
            "transfer_fiber_inventory_feature": case["transfer_fiber_inventory_feature"],
            "establishment_count": case["establishment_count"],
            "invariant": case["invariant"],
            "action_intents": by_intent,
            "focused_actions": focused,
            "sample_count": int(len(samples)),
        }
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--runset", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if core.REALM_SCHEMA != "realm-v11" or core.REALM_OBS != OBS or core.REALM_ACTIONS != ACTIONS:
        raise RuntimeError("loaded C++ Realm binding does not match model v11")
    cases = contexts()
    run_root = args.runset / "economic"
    seeds = [audit_policy(run_root / f"seed-{seed}" / "policy.pt",
                          run_root / f"seed-{seed}" / "manifest.json", seed, cases)
             for seed in (1, 2, 3)]
    payload = {
        "task": "M5-v22-worksite-context-policy-audit",
        "schema": core.REALM_SCHEMA,
        "reward_version": core.REALM_REWARD_VERSION,
        "model_contract": MODEL_CONTRACT,
        "regions": 6,
        "runtime_sampling": "masked categorical; 256 fixed-seed draws per context/seed",
        "model_selection": "diagnostic only; all seeds remain peer_eligible=false",
        "contexts": [{key: value for key, value in case.items() if key not in ("observation", "mask")}
                     for case in cases],
        "seeds": seeds,
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    print(json.dumps({"output": str(args.output), "seeds": len(seeds),
                      "contexts": len(cases), "invariants": all(
                          case["invariant"] for case in cases)}, sort_keys=True))


if __name__ == "__main__":
    main()
