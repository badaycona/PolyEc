"""Measure immediate C++ Realm utility changes from legal Economic Buy commands."""
import argparse
import hashlib
import json
from pathlib import Path
import sys

import numpy as np
import polyec_env as core

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "training"))
from realm_model_v11 import FEATURES, OBS, STATE
from realm_coplay_v11 import selected_action_good, selected_action_intent
from realm_coplay_env_v11 import RealmCoPlayBatch, WORKSITE_CURRICULUM_VERSION

GOOD_NAMES = (
    "DryFood", "SupplementaryFood", "Salt", "Timber", "Fuel", "Iron", "Copper", "Silver", "Gold",
    "Fiber", "Beeswax", "Cloth", "Silk", "Livestock", "Horse", "WarHorse", "Tools",
    "MilitarySupply", "Weapon", "ConstructionMaterial", "Paper", "Medicine", "Tea", "Incense",
    "DecorativeGlass", "FinePorcelain", "GeneralLuxury",
)
GOOD_REFERENCE_PRICE = (
    20, 28, 35, 45, 18, 45, 65, 100, 120, 14, 40, 35, 120, 50,
    90, 200, 65, 90, 110, 55, 60, 130, 100, 160, 250, 300, 150,
)
INTENT_BUY = 1


def make_world(seed, regions):
    world = core.Realm(seed, regions, 128)
    cards = world.view()["cards"]
    if not world.choose(cards[seed % len(cards)]):
        raise RuntimeError("initial draft selection failed")
    return world


def legal_buy_samples(seed, regions):
    world = make_world(seed, regions)
    samples = {}
    for actor in world.due(1):
        obs = np.asarray(world.observe(actor), dtype=np.float32)
        mask = np.asarray(world.mask(actor), dtype=np.float32)
        for action in np.flatnonzero(mask > 0.5):
            action = int(action)
            if selected_action_intent(obs, action) != INTENT_BUY:
                continue
            feature = STATE + action * FEATURES
            good = int(round(float(obs[feature + 2]) * (len(GOOD_NAMES) - 1)))
            quantity = int(round(float(obs[feature + 3]) * 200.0))
            if not 0 <= good < len(GOOD_NAMES) or quantity <= 0:
                raise RuntimeError("invalid Buy candidate observation parameter")
            samples.setdefault(good, (actor, action, quantity, float(obs[feature + 8])))
    return samples


def measure_buy(seed, regions, good, sample):
    actor, action, quantity, price_ratio = sample
    world = make_world(seed, regions)
    household = world.household_of(actor)
    person_before = world.person(actor)
    cash_before = int(person_before["cash"])
    fund_before = int(world.household_fund(household))
    debt_before = int(world.debt(actor))
    utility_before = float(world.utility(actor))
    ok, reason = world.step(actor, action)
    if not ok:
        raise RuntimeError(f"replayed Buy candidate rejected: {reason}")
    cash_after = int(world.person(actor)["cash"])
    fund_after = int(world.household_fund(household))
    debt_after = int(world.debt(actor))
    utility_after = float(world.utility(actor))
    actor_cash_spent = cash_before - cash_after
    household_fund_spent = fund_before - fund_after
    total_spent = actor_cash_spent + household_fund_spent
    reference_value = quantity * GOOD_REFERENCE_PRICE[good]
    expected_utility_delta = (reference_value - actor_cash_spent - (debt_after - debt_before)) / 100.0
    utility_delta = utility_after - utility_before
    if not world.invariant():
        raise RuntimeError("Realm invariant failed after Buy candidate")
    return {
        "seed": seed,
        "actor": actor,
        "action_slot": action,
        "good_id": good,
        "good": GOOD_NAMES[good],
        "quantity": quantity,
        "market_price_to_reference_ratio": price_ratio,
        "reference_unit_price": GOOD_REFERENCE_PRICE[good],
        "observed_unit_price": total_spent / quantity,
        "total_spent": total_spent,
        "actor_cash_spent": actor_cash_spent,
        "household_fund_spent": household_fund_spent,
        "debt_delta": debt_after - debt_before,
        "utility_delta": utility_delta,
        "utility_delta_from_actor_assets": expected_utility_delta,
        "utility_delta_residual": utility_delta - expected_utility_delta,
        "invariant": True,
    }


def training_transition(seed, regions, action):
    batch = RealmCoPlayBatch(1, seed, 1, regions, 1,
                             worksite_curriculum=WORKSITE_CURRICULUM_VERSION)
    _, masks = batch.observe()
    if masks[0, action] <= 0.5:
        raise RuntimeError("paired curriculum action is not legal in replay")
    scenario = batch.scenarios[0]
    _, _, rewards, dones, scores = batch.step(np.asarray([action], dtype=np.int64), False)
    if not bool(dones[0]):
        raise RuntimeError("one-step RealmBatch transition did not terminate")
    outcome = batch.outcomes_data[0]
    return {
        "reward": float(rewards[0]),
        "score": float(scores[0]),
        "raw_utility_delta": float(outcome["raw_utility_delta"]),
        "cash": int(outcome["cash"]),
        "debt": int(outcome["debt"]),
        "health": int(outcome["health"]),
        "nutrition": int(outcome["nutrition"]),
        "scenario": scenario,
        "invariant": bool(outcome["invariant"]),
    }


def paired_training_buy_samples(seed, regions):
    batch = RealmCoPlayBatch(1, seed, 1, regions, 1,
                             worksite_curriculum=WORKSITE_CURRICULUM_VERSION)
    observations, masks = batch.observe()
    obs = np.asarray(observations[0], dtype=np.float32)
    mask = np.asarray(masks[0], dtype=np.float32)
    scenario = batch.scenarios[0]
    choices = {}
    for action in np.flatnonzero(mask > 0.5):
        action = int(action)
        if selected_action_intent(obs, action) == INTENT_BUY:
            good = selected_action_good(obs, action)
            choices.setdefault(good, action)
    if not choices:
        return []
    hold = training_transition(seed, regions, 0)
    pairs = []
    for good, action in choices.items():
        buy = training_transition(seed, regions, action)
        pairs.append({
            "seed": seed,
            "scenario": scenario,
            "good": GOOD_NAMES[good],
            "action_slot": action,
            "buy": buy,
            "hold": hold,
            "reward_delta_vs_hold": buy["reward"] - hold["reward"],
            "score_delta_vs_hold": buy["score"] - hold["score"],
            "raw_utility_delta_vs_hold": buy["raw_utility_delta"] - hold["raw_utility_delta"],
        })
    return pairs


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--seeds", type=int, nargs="+", required=True)
    parser.add_argument("--training-seeds", type=int, nargs="+", default=[1, 2, 3])
    parser.add_argument("--regions", type=int, default=6)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if len(args.seeds) < 20 or len(set(args.seeds)) != len(args.seeds):
        parser.error("audit requires at least 20 distinct world seeds")
    if args.output.exists() and any(args.output.iterdir()):
        parser.error("output directory must be empty")
    samples = []
    paired_samples = []
    for seed in args.seeds:
        for good, candidate in legal_buy_samples(seed, args.regions).items():
            samples.append(measure_buy(seed, args.regions, good, candidate))
    paired_base_seeds = [2**32 + training_seed * 10**7 + env_index
                         for training_seed in args.training_seeds for env_index in range(16)]
    for seed in paired_base_seeds:
        paired_samples.extend(paired_training_buy_samples(seed, args.regions))
    if not samples:
        raise RuntimeError("no legal Economic Buy candidates found in requested snapshots")
    by_good = {}
    for item in samples:
        aggregate = by_good.setdefault(item["good"], {
            "samples": 0, "actor_cash_spent": 0, "household_fund_spent": 0,
            "total_spent": 0, "reference_value": 0, "utility_delta": 0.0,
            "utility_delta_residual": 0.0,
        })
        aggregate["samples"] += 1
        for key in ("actor_cash_spent", "household_fund_spent", "total_spent"):
            aggregate[key] += item[key]
        aggregate["reference_value"] += item["quantity"] * item["reference_unit_price"]
        aggregate["utility_delta"] += item["utility_delta"]
        aggregate["utility_delta_residual"] += item["utility_delta_residual"]
    training_by_good = {}
    for item in paired_samples:
        aggregate = training_by_good.setdefault(item["good"], {
            "samples": 0, "buy_reward_mean": 0.0, "hold_reward_mean": 0.0,
            "reward_delta_vs_hold": 0.0, "score_delta_vs_hold": 0.0,
            "raw_utility_delta_vs_hold": 0.0,
        })
        aggregate["samples"] += 1
        aggregate["buy_reward_mean"] += item["buy"]["reward"]
        aggregate["hold_reward_mean"] += item["hold"]["reward"]
        for key in ("reward_delta_vs_hold", "score_delta_vs_hold", "raw_utility_delta_vs_hold"):
            aggregate[key] += item[key]
    for aggregate in training_by_good.values():
        count = aggregate["samples"]
        for key in ("buy_reward_mean", "hold_reward_mean", "reward_delta_vs_hold", "score_delta_vs_hold",
                    "raw_utility_delta_vs_hold"):
            aggregate[key] /= count
    audit = {
        "schema": "realm-economic-inventory-valuation-audit-v1",
        "realm_schema": core.REALM_SCHEMA,
        "model_contract": core.Realm(1, args.regions, 128).model_contract,
        "reward_version": core.REALM_REWARD_VERSION,
        "seeds": args.seeds,
        "training_seeds": args.training_seeds,
        "paired_training_base_seeds": paired_base_seeds,
        "regions": args.regions,
        "buy_samples": samples,
        "summary_by_good": by_good,
        "paired_training_buy_samples": paired_samples,
        "paired_training_summary_by_good": training_by_good,
        "limits": [
            "Each sample measures one legal accepted C++ Buy command from the same deterministic initial world.",
            "No day advances between before/after utility measurements.",
            "This is an audit only; it changes no reward, policy, price, or gameplay rule."
        ],
    }
    args.output.mkdir(parents=True, exist_ok=True)
    output = args.output / "audit.json"
    output.write_text(json.dumps(audit, indent=2), encoding="utf-8")
    summary = {
        "samples": len(samples),
        "paired_training_samples": len(paired_samples),
        "goods": sorted(by_good),
        "household_funded_samples": sum(item["household_fund_spent"] > 0 for item in samples),
        "positive_utility_samples": sum(item["utility_delta"] > 0 for item in samples),
        "max_abs_residual": max(abs(item["utility_delta_residual"]) for item in samples),
        "all_invariants": all(item["invariant"] for item in samples),
        "audit_sha256": sha256(output),
    }
    print(json.dumps(summary, sort_keys=True))


if __name__ == "__main__":
    main()
