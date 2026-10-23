"""Execute a real same-region Buy/Sell round trip in saved v23 Realm states."""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import sys

import numpy as np
import torch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "training"))
from realm_coplay_v11 import GOOD_NAMES, selected_action_good, selected_action_intent
from realm_coplay_env_v11 import RealmCoPlayBatch, WORKSITE_CURRICULUM_VERSION

INTENT_BUY = 1
INTENT_SELL = 2
QUANTITY = 4
LEGACY_GOODS = (0, 9, 11, 5, 4, 16, 14)
LEGACY_SLOT = {good: slot for slot, good in enumerate(LEGACY_GOODS)}


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def one_env_state(state, index):
    return {
        "seed": state["seed"], "role": state["role"], "regions": state["regions"],
        "horizon": state["horizon"], "worksite_curriculum": state["worksite_curriculum"],
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


def restore_one(state, index, regions):
    subset = one_env_state(state, index)
    batch = RealmCoPlayBatch(1, subset["seed"], 1, regions, subset["horizon"],
                             worksite_curriculum=WORKSITE_CURRICULUM_VERSION)
    batch.load_state_dict(subset)
    return batch


def observed_legacy_inventory(batch, actor):
    observation, _ = batch.observe()
    return [int(round(float(observation[0, 73 + slot]) * 16.0))
            for slot in range(len(LEGACY_GOODS))]


def command(world, actor, intent, region, good, amount):
    sequence = int(world.person(actor)["decisions"]) + 1
    success, reason, fact = world.intent_command(
        actor, sequence, intent, region, good, amount, 0)
    if not success:
        raise RuntimeError(f"Realm rejected intent {intent} for GoodId {good}: {reason}")
    return int(fact)


def audit_one(saved_state, index, regions, action, good):
    if good not in LEGACY_SLOT:
        raise RuntimeError(f"v23 selected a non-legacy GoodId unsupported by the observed stock oracle: {good}")
    batch = restore_one(saved_state, index, regions)
    world = batch.worlds[0]
    actor = int(batch.actors[0])
    person = world.person(actor)
    region = int(person["region"])
    household = int(world.household_of(actor))
    province_before = world.view(actor)["provinces"][region]
    price = int(province_before["prices"][LEGACY_SLOT[good]])
    if price < 1:
        raise RuntimeError("saved Realm exposed a non-positive local Buy price")
    opening = {
        "fingerprint": int(world.fingerprint()),
        "utility": float(world.utility(actor)),
        "cash": int(person["cash"]),
        "household_fund": int(world.household_fund(household)),
        "money": int(world.view(actor)["money"]),
        "province_firm": int(province_before["firm"]),
        "inventory": observed_legacy_inventory(batch, actor),
        "invariant": bool(world.invariant()),
    }
    if not opening["invariant"]:
        raise RuntimeError("saved Realm violates its invariant before Buy")
    if batch.observe()[1][0, action] <= 0.5:
        raise RuntimeError("selected Buy action is not legal in the restored state")

    buy_fact = command(world, actor, INTENT_BUY, region, good, QUANTITY)
    bought = {
        "utility": float(world.utility(actor)),
        "cash": int(world.person(actor)["cash"]),
        "household_fund": int(world.household_fund(household)),
        "inventory": observed_legacy_inventory(batch, actor),
        "invariant": bool(world.invariant()),
    }
    command(world, actor, INTENT_SELL, region, good, QUANTITY)
    province_after = world.view(actor)["provinces"][region]
    closing = {
        "fingerprint": int(world.fingerprint()),
        "utility": float(world.utility(actor)),
        "cash": int(world.person(actor)["cash"]),
        "household_fund": int(world.household_fund(household)),
        "money": int(world.view(actor)["money"]),
        "province_firm": int(province_after["firm"]),
        "inventory": observed_legacy_inventory(batch, actor),
        "invariant": bool(world.invariant()),
    }
    buy_cost = QUANTITY * (price + 1)
    sell_proceeds = QUANTITY * max(1, price - 1)
    expected_actor_delta = sell_proceeds - buy_cost
    actual_actor_delta = closing["cash"] - opening["cash"]
    expected_utility_delta = expected_actor_delta / 100.0
    if bought["cash"] - opening["cash"] != -buy_cost:
        raise RuntimeError("Buy did not debit the actor by the validated price plus fee")
    if bought["household_fund"] != opening["household_fund"]:
        raise RuntimeError("unexpected household-fund payment source in saved round-trip sample")
    if bought["inventory"][LEGACY_SLOT[good]] != opening["inventory"][LEGACY_SLOT[good]] + QUANTITY:
        raise RuntimeError("Buy did not add the requested inventory quantity")
    if closing["inventory"] != opening["inventory"]:
        raise RuntimeError("Sell did not restore all observed legacy inventory")
    if actual_actor_delta != expected_actor_delta:
        raise RuntimeError("round-trip actor cash does not match the actual Buy/Sell spread")
    if abs(closing["utility"] - opening["utility"] - expected_utility_delta) > 1e-9:
        raise RuntimeError("round-trip utility delta does not match retained cash loss")
    if closing["money"] != opening["money"]:
        raise RuntimeError("Buy/Sell changed Realm total money")
    if not bought["invariant"] or not closing["invariant"]:
        raise RuntimeError("Realm invariant failed during Buy/Sell")
    if closing["household_fund"] != opening["household_fund"]:
        raise RuntimeError("round-trip changed the household account")
    return {
        "world_index": index, "scenario": saved_state["scenarios"][index],
        "actor": actor, "good_id": good, "good": GOOD_NAMES[good],
        "buy_action_index": action, "buy_fact": buy_fact, "quantity": QUANTITY,
        "region": region, "local_buy_unit_price": price + 1,
        "local_sell_unit_price": max(1, price - 1),
        "buy_cost": buy_cost, "sell_proceeds": sell_proceeds,
        "expected_round_trip_actor_cash_delta": expected_actor_delta,
        "actual_round_trip_actor_cash_delta": actual_actor_delta,
        "utility_delta_on_buy": bought["utility"] - opening["utility"],
        "utility_delta_after_actual_resale": closing["utility"] - opening["utility"],
        "opening": opening, "after_buy": bought, "closing": closing,
        "same_inventory_after_resale": closing["inventory"] == opening["inventory"],
        "money_conserved": closing["money"] == opening["money"],
        "household_fund_unchanged": closing["household_fund"] == opening["household_fund"],
        "all_invariants": opening["invariant"] and bought["invariant"] and closing["invariant"],
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", type=Path, action="append", required=True)
    parser.add_argument("--regions", type=int, default=6)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if len(args.run) != 3:
        parser.error("requires the three v23 Economic checkpoints")
    if args.output.exists() and any(args.output.iterdir()):
        parser.error("output directory must be empty")
    args.output.mkdir(parents=True, exist_ok=True)

    samples = []
    runs = []
    for run_path in args.run:
        checkpoint_path = run_path / "checkpoint.pt"
        policy_path = run_path / "policy.pt"
        checkpoint = torch.load(checkpoint_path, map_location="cpu", weights_only=False)
        saved = checkpoint["environment"]
        batch = RealmCoPlayBatch(len(saved["worlds"]), saved["seed"], 1, args.regions,
                                 saved["horizon"], worksite_curriculum=WORKSITE_CURRICULUM_VERSION)
        batch.load_state_dict(saved)
        observations, masks = batch.observe()
        state = batch.state_dict()
        by_context_good = {}
        for index, (observation, mask) in enumerate(zip(observations, masks)):
            if bool(saved["finished"][index]):
                continue
            for action in np.flatnonzero(np.asarray(mask) > 0.5):
                action = int(action)
                if selected_action_intent(observation, action) != INTENT_BUY:
                    continue
                good = selected_action_good(observation, action)
                by_context_good.setdefault((saved["scenarios"][index], good), (index, action))
        run_samples = []
        for (scenario, good), (index, action) in by_context_good.items():
            sample = audit_one(state, index, args.regions, action, good)
            sample["training_seed"] = int(checkpoint["identity"]["seed"])
            sample["update"] = int(checkpoint["update"])
            sample["scenario"] = scenario
            run_samples.append(sample)
            samples.append(sample)
        runs.append({
            "training_seed": int(checkpoint["identity"]["seed"]),
            "update": int(checkpoint["update"]),
            "checkpoint_sha256": sha256(checkpoint_path),
            "policy_sha256": sha256(policy_path),
            "legal_buy_contexts": len(run_samples),
        })

    if len(samples) != 15:
        raise RuntimeError(f"expected the exact 15 saved v27 Buy contexts, got {len(samples)}")
    summary = {}
    for sample in samples:
        row = summary.setdefault(sample["good"], {
            "good": sample["good"], "samples": 0,
            "mean_utility_delta_on_buy": 0.0,
            "mean_utility_delta_after_actual_resale": 0.0,
            "mean_actor_cash_delta_after_actual_resale": 0.0,
        })
        row["samples"] += 1
        row["mean_utility_delta_on_buy"] += sample["utility_delta_on_buy"]
        row["mean_utility_delta_after_actual_resale"] += sample["utility_delta_after_actual_resale"]
        row["mean_actor_cash_delta_after_actual_resale"] += sample["actual_round_trip_actor_cash_delta"]
    for row in summary.values():
        for key in ("mean_utility_delta_on_buy", "mean_utility_delta_after_actual_resale",
                    "mean_actor_cash_delta_after_actual_resale"):
            row[key] /= row["samples"]

    data = {
        "schema": "realm-v23-buy-sell-roundtrip-audit-v1",
        "regions": args.regions, "quantity_per_trade": QUANTITY,
        "runs": runs, "samples": samples,
        "summary_by_good": list(summary.values()),
        "all_invariants": all(sample["all_invariants"] for sample in samples),
        "all_inventory_restored": all(sample["same_inventory_after_resale"] for sample in samples),
        "all_money_conserved": all(sample["money_conserved"] for sample in samples),
        "limits": [
            "These are 15 legal contexts selected from saved update-25 v23 checkpoints, not new PPO rollout rows.",
            "Buy and Sell execute in the same region and day without advancing time or allowing another actor to act.",
            "The audit does not choose an inventory cost-basis rule or change utility, reward, model, or gameplay.",
        ],
    }
    output = args.output / "audit.json"
    output.write_text(json.dumps(data, indent=2), encoding="utf-8")
    print(json.dumps({
        "samples": len(samples), "all_invariants": data["all_invariants"],
        "all_inventory_restored": data["all_inventory_restored"],
        "all_money_conserved": data["all_money_conserved"],
        "summary_by_good": data["summary_by_good"],
        "audit_sha256": sha256(output),
    }, sort_keys=True))


if __name__ == "__main__":
    main()
