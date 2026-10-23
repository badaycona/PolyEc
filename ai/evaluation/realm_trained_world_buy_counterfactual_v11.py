"""Pair Buy and Hold in exact saved v23 Economic rollout states."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import sys

import numpy as np
import torch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "training"))
from realm_model_v11 import FEATURES, STATE
from realm_coplay_v11 import GOOD_NAMES, selected_action_good, selected_action_intent
from realm_coplay_env_v11 import RealmCoPlayBatch, WORKSITE_CURRICULUM_VERSION

INTENT_BUY = 1


def one_env_state(state, index):
    return {
        "seed": state["seed"],
        "role": state["role"],
        "regions": state["regions"],
        "horizon": state["horizon"],
        "worksite_curriculum": state["worksite_curriculum"],
        "worlds": [state["worlds"][index]],
        "actors": np.asarray([state["actors"][index]], dtype=np.int64),
        "scenarios": [state["scenarios"][index]],
        "steps": np.asarray([state["steps"][index]], dtype=np.int64),
        "opening": np.asarray([state["opening"][index]], dtype=np.float64),
        "finished": np.asarray([state["finished"][index]], dtype=bool),
        "final_scores": np.asarray([state["final_scores"][index]], dtype=np.float32),
        "outcomes": [copy.deepcopy(state["outcomes"][index])],
        "peer_hidden": [copy.deepcopy(state["peer_hidden"][index])],
        "peer_family": [copy.deepcopy(state["peer_family"][index])],
        "peer_rng": [copy.deepcopy(state["peer_rng"][index])],
        "peer_decisions": state["peer_decisions"],
        "baseline_version": state["baseline_version"],
        "reward_version": state["reward_version"],
    }


def replay_one(state, index, regions, action):
    subset = one_env_state(state, index)
    immediate = RealmCoPlayBatch(1, subset["seed"], 1, regions, subset["horizon"],
                                 worksite_curriculum=WORKSITE_CURRICULUM_VERSION)
    immediate.load_state_dict(subset)
    immediate_actor = int(immediate.actors[0])
    immediate_world = immediate.worlds[0]
    household = immediate_world.household_of(immediate_actor)
    immediate_before_utility = float(immediate_world.utility(immediate_actor))
    immediate_before_cash = int(immediate_world.person(immediate_actor)["cash"])
    immediate_before_fund = int(immediate_world.household_fund(household))
    immediate_before_debt = int(immediate_world.debt(immediate_actor))
    ok, reason = immediate_world.step(immediate_actor, action)
    if not ok:
        raise RuntimeError(f"immediate counterfactual action rejected: {reason}")
    immediate_after_utility = float(immediate_world.utility(immediate_actor))
    immediate_after_cash = int(immediate_world.person(immediate_actor)["cash"])
    immediate_after_fund = int(immediate_world.household_fund(household))
    immediate_after_debt = int(immediate_world.debt(immediate_actor))

    batch = RealmCoPlayBatch(1, subset["seed"], 1, regions, subset["horizon"],
                             worksite_curriculum=WORKSITE_CURRICULUM_VERSION)
    batch.load_state_dict(subset)
    observations, masks = batch.observe()
    if masks[0, action] <= 0.5:
        raise RuntimeError("counterfactual action was not legal in restored snapshot")
    actor = int(batch.actors[0])
    world = batch.worlds[0]
    start_fingerprint = world.fingerprint()
    peer_rng_sha256 = hashlib.sha256(
        json.dumps(batch.state_dict()["peer_rng"][0], sort_keys=True).encode("utf-8")
    ).hexdigest()
    before_utility = float(world.utility(actor))
    before_person = world.person(actor)
    _, _, rewards, dones, _ = batch.step(np.asarray([action], dtype=np.int64), False)
    if not batch.worlds[0].invariant():
        raise RuntimeError("counterfactual violated Realm invariant")
    world = batch.worlds[0]
    person = world.person(actor)
    sites = world.establishments()
    return {
        "action": action,
        "reward": float(rewards[0]),
        "done": bool(dones[0]),
        "raw_utility_delta": float(world.utility(actor) - before_utility),
        "cash_delta": int(person["cash"] - before_person["cash"]),
        "debt": int(world.debt(actor)),
        "health": int(person["health"]),
        "nutrition": int(person["nutrition"]),
        "paid_site_work_units": sum(sum(site["paid_work_units"].values()) for site in sites),
        "fiber_stock": sum(site["stock"][9] for site in sites),
        "cloth_stock": sum(site["stock"][11] for site in sites),
        "day": int(world.day),
        "invariant": True,
        "immediate_utility_delta": immediate_after_utility - immediate_before_utility,
        "immediate_actor_cash_delta": immediate_after_cash - immediate_before_cash,
        "immediate_household_fund_delta": immediate_after_fund - immediate_before_fund,
        "immediate_debt_delta": immediate_after_debt - immediate_before_debt,
        "start_fingerprint": start_fingerprint,
        "peer_rng_sha256": peer_rng_sha256,
    }


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", type=Path, action="append", required=True)
    parser.add_argument("--regions", type=int, default=6)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if len(args.run) != 3:
        parser.error("counterfactual audit requires all three v23 Economic checkpoints")
    if args.output.exists() and any(args.output.iterdir()):
        parser.error("output directory must be empty")
    pairs = []
    run_hashes = []
    for run_path in args.run:
        checkpoint = torch.load(run_path / "checkpoint.pt", map_location="cpu", weights_only=False)
        saved = checkpoint["environment"]
        batch = RealmCoPlayBatch(len(saved["worlds"]), saved["seed"], 1, args.regions,
                                 saved["horizon"], worksite_curriculum=WORKSITE_CURRICULUM_VERSION)
        batch.load_state_dict(saved)
        observations, masks = batch.observe()
        state = batch.state_dict()
        selected_by_context_good = {}
        for index in range(len(saved["worlds"])):
            if bool(saved["finished"][index]):
                continue
            context = saved["scenarios"][index]
            obs = np.asarray(observations[index], dtype=np.float32)
            mask = np.asarray(masks[index], dtype=np.float32)
            for action in np.flatnonzero(mask > 0.5):
                action = int(action)
                if selected_action_intent(obs, action) != INTENT_BUY:
                    continue
                good = selected_action_good(obs, action)
                selected_by_context_good.setdefault((context, good), (index, action))
        if not selected_by_context_good:
            run_hashes.append({
                "training_seed": int(checkpoint["identity"]["seed"]),
                "checkpoint_sha256": sha256(run_path / "checkpoint.pt"),
                "policy_sha256": sha256(run_path / "policy.pt"),
                "update": int(checkpoint["update"]),
                "legal_buy_counterfactuals": 0,
            })
            continue
        for (context, good), (index, buy_action) in selected_by_context_good.items():
            hold = replay_one(state, index, args.regions, 0)
            buy = replay_one(state, index, args.regions, buy_action)
            if hold["start_fingerprint"] != buy["start_fingerprint"]:
                raise RuntimeError("paired counterfactual Realm fingerprints differ")
            if hold["peer_rng_sha256"] != buy["peer_rng_sha256"]:
                raise RuntimeError("paired counterfactual peer RNG states differ")
            if not hold["invariant"] or not buy["invariant"]:
                raise RuntimeError("counterfactual invariant failure")
            pairs.append({
                "training_seed": int(checkpoint["identity"]["seed"]),
                "update": int(checkpoint["update"]),
                "environment_index": index,
                "scenario": context,
                "good_id": good,
                "good": GOOD_NAMES[good],
                "buy_action": buy_action,
                "buy": buy,
                "hold": hold,
                "reward_delta_buy_minus_hold": buy["reward"] - hold["reward"],
                "immediate_utility_delta_buy_minus_hold": (
                    buy["immediate_utility_delta"] - hold["immediate_utility_delta"]
                ),
                "utility_delta_buy_minus_hold": buy["raw_utility_delta"] - hold["raw_utility_delta"],
                "paid_work_delta_buy_minus_hold": buy["paid_site_work_units"] - hold["paid_site_work_units"],
                "same_start_fingerprint": hold["start_fingerprint"] == buy["start_fingerprint"],
                "same_peer_rng": hold["peer_rng_sha256"] == buy["peer_rng_sha256"],
            })
        run_hashes.append({
            "training_seed": int(checkpoint["identity"]["seed"]),
            "checkpoint_sha256": sha256(run_path / "checkpoint.pt"),
            "policy_sha256": sha256(run_path / "policy.pt"),
            "update": int(checkpoint["update"]),
            "legal_buy_counterfactuals": sum(
                1 for item in pairs if item["training_seed"] == int(checkpoint["identity"]["seed"])
            ),
        })
    summary = {}
    for item in pairs:
        key = f"{item['scenario']}|{item['good']}"
        aggregate = summary.setdefault(key, {
            "scenario": item["scenario"], "good": item["good"], "samples": 0,
            "reward_delta_sum": 0.0, "utility_delta_sum": 0.0,
            "immediate_utility_delta_sum": 0.0,
            "paid_work_delta_sum": 0, "buy_wins": 0,
        })
        aggregate["samples"] += 1
        aggregate["reward_delta_sum"] += item["reward_delta_buy_minus_hold"]
        aggregate["utility_delta_sum"] += item["utility_delta_buy_minus_hold"]
        aggregate["immediate_utility_delta_sum"] += item["immediate_utility_delta_buy_minus_hold"]
        aggregate["paid_work_delta_sum"] += item["paid_work_delta_buy_minus_hold"]
        aggregate["buy_wins"] += int(item["reward_delta_buy_minus_hold"] > 0)
    for aggregate in summary.values():
        count = aggregate["samples"]
        aggregate["reward_delta_mean"] = aggregate.pop("reward_delta_sum") / count
        aggregate["utility_delta_mean"] = aggregate.pop("utility_delta_sum") / count
        aggregate["immediate_utility_delta_mean"] = aggregate.pop("immediate_utility_delta_sum") / count
        aggregate["paid_work_delta_mean"] = aggregate.pop("paid_work_delta_sum") / count
    output = args.output / "audit.json"
    args.output.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps({
        "schema": "realm-v23-checkpoint-buy-counterfactual-v1",
        "regions": args.regions,
        "runs": run_hashes,
        "paired_samples": pairs,
        "summary_by_context_good": list(summary.values()),
        "all_invariants": all(item["buy"]["invariant"] and item["hold"]["invariant"] for item in pairs),
        "limits": ["One 7-day counterfactual from each saved checkpoint state; it is not a policy evaluation."],
    }, indent=2), encoding="utf-8")
    print(json.dumps({
        "pairs": len(pairs),
        "all_invariants": True,
        "buy_beats_hold": sum(item["reward_delta_buy_minus_hold"] > 0 for item in pairs),
        "audit_sha256": sha256(output),
    }, sort_keys=True))


if __name__ == "__main__":
    main()
