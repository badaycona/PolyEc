"""Offline valuation audit over exact saved v23 Economic checkpoint states."""
import argparse
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

REFERENCE = (20, 28, 35, 45, 18, 45, 65, 100, 120, 14, 40, 35, 120, 50, 90,
             200, 65, 90, 110, 55, 60, 130, 100, 160, 250, 300, 150)
LEGACY_GOODS = (0, 9, 11, 5, 4, 16, 14)


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def state_at(saved, index):
    return {
        "seed": saved["seed"], "role": saved["role"], "regions": saved["regions"],
        "horizon": saved["horizon"], "worksite_curriculum": saved["worksite_curriculum"],
        "worlds": [saved["worlds"][index]],
        "actors": np.asarray([saved["actors"][index]], dtype=np.int64),
        "scenarios": [saved["scenarios"][index]],
        "steps": np.asarray([saved["steps"][index]], dtype=np.int64),
        "opening": np.asarray([saved["opening"][index]], dtype=np.float64),
        "finished": np.asarray([saved["finished"][index]], dtype=bool),
        "final_scores": np.asarray([saved["final_scores"][index]], dtype=np.float32),
        "outcomes": [saved["outcomes"][index]], "peer_hidden": [saved["peer_hidden"][index]],
        "peer_family": [saved["peer_family"][index]], "peer_rng": [saved["peer_rng"][index]],
        "peer_decisions": saved["peer_decisions"], "baseline_version": saved["baseline_version"],
        "reward_version": saved["reward_version"],
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", type=Path, action="append", required=True)
    parser.add_argument("--regions", type=int, default=6)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if len(args.run) != 3:
        parser.error("requires all three v23 Economic checkpoints")
    args.output.mkdir(parents=True, exist_ok=True)
    output = args.output / "audit.json"
    if output.exists():
        parser.error("output audit already exists")

    buy_rows, movement_rows, runs = [], [], []
    for run_path in args.run:
        checkpoint_path = run_path / "checkpoint.pt"
        checkpoint = torch.load(checkpoint_path, map_location="cpu", weights_only=False)
        saved = checkpoint["environment"]
        batch = RealmCoPlayBatch(len(saved["worlds"]), saved["seed"], 1, args.regions,
                                 saved["horizon"], worksite_curriculum=WORKSITE_CURRICULUM_VERSION)
        batch.load_state_dict(saved)
        observations, masks = batch.observe()
        buy_keys = set()
        for index, (obs, mask) in enumerate(zip(observations, masks)):
            if bool(saved["finished"][index]):
                continue
            obs = np.asarray(obs, dtype=np.float32)
            for action in np.flatnonzero(np.asarray(mask) > 0.5):
                action = int(action)
                if selected_action_intent(obs, action) != 1:
                    continue
                good = selected_action_good(obs, action)
                key = (saved["scenarios"][index], good)
                if key in buy_keys:
                    continue
                buy_keys.add(key)
                feature = float(obs[STATE + action * FEATURES + 8])
                market_price = int(round(feature * REFERENCE[good]))
                quantity = 4
                buy_cost = quantity * (market_price + 1)
                liquidation = max(1, market_price - 1)
                ref_mark_delta = quantity * REFERENCE[good] - buy_cost
                liquidation_mark_delta = quantity * min(REFERENCE[good], liquidation) - buy_cost
                buy_rows.append({
                    "training_seed": int(checkpoint["identity"]["seed"]), "world_index": index,
                    "scenario": saved["scenarios"][index], "good_id": good,
                    "good": GOOD_NAMES[good], "quantity": quantity,
                    "market_price": market_price, "buy_unit_cost": market_price + 1,
                    "sell_unit_proceeds": liquidation, "reference_mark_reward_delta": ref_mark_delta / 100.0,
                    "liquidation_mark_reward_delta": liquidation_mark_delta / 100.0,
                    "buy_cost": buy_cost,
                })

            # The seven legacy goods expose local prices in every region and
            # personal stock in the same exact checkpoint observation.
            actor_region = int(batch.worlds[index].person(int(batch.actors[index]))["region"])
            for legacy, good in enumerate(LEGACY_GOODS):
                quantity = int(round(float(obs[73 + legacy]) * 16.0))
                if quantity <= 0:
                    continue
                values = []
                prices = []
                for region in range(args.regions + 1):
                    market_price = int(round(float(obs[24 + region * 7 + legacy]) * REFERENCE[good]))
                    sell_price = max(1, market_price - 1)
                    prices.append(sell_price)
                    values.append(quantity * min(REFERENCE[good], sell_price))
                movement_rows.append({
                    "training_seed": int(checkpoint["identity"]["seed"]), "world_index": index,
                    "scenario": saved["scenarios"][index], "good": GOOD_NAMES[good],
                    "quantity": quantity, "actor_region": actor_region,
                    "actor_region_mark": values[actor_region], "min_region_mark": min(values),
                    "max_region_mark": max(values), "mark_swing": max(values) - min(values),
                    "region_sell_prices": prices,
                })
        runs.append({"training_seed": int(checkpoint["identity"]["seed"]),
                     "checkpoint_sha256": sha256(checkpoint_path),
                     "policy_sha256": sha256(run_path / "policy.pt"),
                     "update": int(checkpoint["update"])})

    summary = {}
    for row in buy_rows:
        summary.setdefault(row["good"], []).append(row)
    good_summary = []
    for good, rows in sorted(summary.items()):
        good_summary.append({
            "good": good, "samples": len(rows),
            "reference_mark_mean_delta": float(np.mean([r["reference_mark_reward_delta"] for r in rows])),
            "liquidation_mark_mean_delta": float(np.mean([r["liquidation_mark_reward_delta"] for r in rows])),
            "reference_mark_wins": sum(r["reference_mark_reward_delta"] > 0 for r in rows),
            "liquidation_mark_wins": sum(r["liquidation_mark_reward_delta"] > 0 for r in rows),
        })
    data = {
        "schema": "realm-v23-inventory-marking-counterfactual-v1", "regions": args.regions,
        "runs": runs, "buy_samples": buy_rows, "buy_summary_by_good": good_summary,
        "travel_mark_samples": movement_rows,
        "travel_mark_summary": {
            "samples": len(movement_rows),
            "positive_mark_swings": sum(row["mark_swing"] > 0 for row in movement_rows),
            "total_mark_swing": sum(row["mark_swing"] for row in movement_rows),
            "max_mark_swing": max((row["mark_swing"] for row in movement_rows), default=0),
        },
        "limits": ["Offline accounting counterfactual; it does not change C++ utility or game outcomes.",
                   "Liquidation marks use the current region sell price and therefore can vary with location."],
    }
    output.write_text(json.dumps(data, indent=2), encoding="utf-8")
    print(json.dumps({"buy_samples": len(buy_rows), "buy_summary_by_good": good_summary,
                      "travel_mark_summary": data["travel_mark_summary"],
                      "audit_sha256": sha256(output)}, sort_keys=True))


if __name__ == "__main__":
    main()
