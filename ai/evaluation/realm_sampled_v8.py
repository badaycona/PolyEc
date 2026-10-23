"""Paired sampled-policy evaluation on the declared Realm v8 holdout seeds."""
import argparse
import json
import sys
from pathlib import Path

import numpy as np
import torch
import polyec_env as core

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "training"))
from realm_model_v8 import Policy

torch.set_num_threads(2)
parser = argparse.ArgumentParser()
parser.add_argument("--models", type=Path, required=True)
parser.add_argument("--original", type=Path, required=True)
parser.add_argument("--test-seeds", type=int, nargs="+", default=[170000, 170099])
parser.add_argument("--families", choices=("governance", "economic", "civilian"), nargs="+",
                    default=["governance", "economic", "civilian"])
args = parser.parse_args()
if core.REALM_SCHEMA != "realm-v8" or core.REALM_OBS != 1004 or core.REALM_ACTIONS != 73:
    parser.error("v8 sampled evaluator requires the archived realm-v8 core binding")
if len(args.test_seeds) < 2 or len(set(args.test_seeds)) != len(args.test_seeds):
    parser.error("provide at least two distinct paired test seeds")


def evaluate(policy, role, mode, seed):
    batch = core.RealmBatch(100, seed, role, 6, 512 if role == 1 else 128)
    obs, masks = batch.observe()
    hidden = torch.zeros(100, 64)
    totals = np.zeros(100)
    counts = np.zeros(core.REALM_ACTIONS, dtype="int64")
    active = np.ones(100, dtype=bool)
    rng = np.random.default_rng(491)
    for _ in range(512 if role == 1 else 128):
        uniforms = rng.random(100)
        if mode == "sampled":
            with torch.no_grad():
                logits, _, hidden = policy(torch.from_numpy(obs), hidden)
            scores = np.where(masks > 0.5, logits.numpy(), -1e9)
            probs = np.exp(scores - scores.max(1, keepdims=True))
            probs /= probs.sum(1, keepdims=True)
            actions = (np.cumsum(probs, axis=1) < uniforms[:, None]).sum(1)
            actions = actions.clip(0, core.REALM_ACTIONS - 1).astype("int64")
        elif mode == "hold":
            actions = np.zeros(100, dtype="int64")
        elif mode == "random":
            actions = np.array([
                np.flatnonzero(mask)[min(int(u * sum(mask)), int(sum(mask)) - 1)]
                for mask, u in zip(masks, uniforms)
            ], dtype="int64")
        else:
            scores = np.zeros((100, core.REALM_ACTIONS), dtype="float32")
            if role == 0:
                scores[:, 2] = 0.2
                scores[:, 4] = 2
                scores[:, 5] = 1.5
                scores[:, 6:13] = np.where((obs[:, 99] < 0.9)[:, None], 4, 0)
            elif role == 1:
                features = obs[:, 128:].reshape(100, core.REALM_ACTIONS, 12)
                scores[:, 1:8] = 1 - features[:, 1:8, 8]
                scores[:, 8:15] = features[:, 8:15, 8] - 1
                scores[:, 30] = 0.3 * (1 + np.floor(obs[:, 4] * 4))
                scores[:, 32:52] = 1 - features[:, 32:52, 8]
                scores[:, 52:72] = features[:, 52:72, 8] - 1
            else:
                scores[:, 1:8] = 1
                scores[:, 15] = np.where(obs[:, 1] < 0.18, 3, 0.5)
                scores[:, 16] = np.where(obs[:, 3] < 0.6, 4, 0)
            actions = np.where(masks > 0.5, scores, -1e9).argmax(1).astype("int64")

        assert np.all(masks[np.arange(100), actions] > 0.5)
        counts += np.bincount(actions[active], minlength=core.REALM_ACTIONS)
        obs, masks, reward, done, score = batch.step(actions, False)
        totals += reward
        active = ~done
        if done.all():
            break

    assert done.all() and np.allclose(totals, score, atol=1e-4)
    outcomes = list(batch.outcomes())
    return {
        "seed": seed,
        "utility": score.tolist(),
        "mean_utility": float(score.mean()),
        "hold_rate": float(counts[0] / counts.sum()),
        "action_counts": counts.tolist(),
        "survival_rate": float(np.mean([item["alive"] for item in outcomes])),
        "mean_health": float(np.mean([item["health"] for item in outcomes])),
        "cash_shortfall_vs_debt_rate": float(np.mean([item["insolvent"] for item in outcomes])),
        "obligations_due": sum(item["obligations_due"] for item in outcomes),
        "defaults": sum(item["defaults"] for item in outcomes),
        "invalid_actions": 0,
    }


def combine(rows):
    utilities = [value for row in rows for value in row["utility"]]
    counts = np.sum([row["action_counts"] for row in rows], axis=0)
    episode_count = sum(len(row["utility"]) for row in rows)
    return {
        "by_seed": {str(row["seed"]): row for row in rows},
        "utility": utilities,
        "mean_utility": float(np.mean(utilities)),
        "hold_rate": float(counts[0] / counts.sum()),
        "action_counts": counts.tolist(),
        "survival_rate": float(np.mean([row["survival_rate"] for row in rows])),
        "mean_health": float(np.mean([row["mean_health"] for row in rows])),
        "cash_shortfall_vs_debt_rate": float(np.mean([row["cash_shortfall_vs_debt_rate"] for row in rows])),
        "obligations_due": sum(row["obligations_due"] for row in rows),
        "defaults": sum(row["defaults"] for row in rows),
        "invalid_actions": sum(row["invalid_actions"] for row in rows),
        "episodes": episode_count,
    }


def ci(difference):
    values = np.asarray(difference, dtype=np.float64)
    rng = np.random.default_rng(491)
    boot = values[rng.integers(0, len(values), (5000, len(values)))].mean(1)
    return {"mean": float(values.mean()), "ci95": np.quantile(boot, [0.025, 0.975]).tolist()}


report = {
    "schema": "realm-v8",
    "observation": core.REALM_OBS,
    "actions": core.REALM_ACTIONS,
    "test_seeds": args.test_seeds,
    "families_requested": args.families,
    "episodes_per_seed": 100,
    "sampling": "categorical; common uniform seed 491 per environment seed",
    "selection": "validation only, frozen before this audit",
    "families": {},
    "metric": "normalized terminal utility delta / abs(opening utility), not coins",
    "full_R1_pass": False,
}

for family in args.families:
    role = ("governance", "economic", "civilian").index(family)
    base = args.models / family
    manifest = json.loads((base / "manifest.json").read_text())
    seed = manifest["selected_seed"]
    assert manifest.get("family") == family and manifest.get("schema") == "realm-v8"
    assert manifest.get("observation") == core.REALM_OBS
    assert manifest.get("actions") == core.REALM_ACTIONS and manifest.get("hidden") == 64
    assert manifest.get("model_contract") == core.Realm(1, 6, 128).model_contract
    original_manifest = json.loads((args.original / family / "manifest.json").read_text())
    assert original_manifest.get("family") == family and original_manifest.get("schema") == "realm-v8"
    assert original_manifest.get("observation") == core.REALM_OBS
    assert original_manifest.get("actions") == core.REALM_ACTIONS and original_manifest.get("hidden") == 64
    assert original_manifest.get("model_contract") == core.Realm(1, 6, 128).model_contract
    assert original_manifest.get("selected_seed") == seed

    selected = Policy()
    selected.load_state_dict(torch.load(base / f"seed-{seed}.pt", weights_only=True))
    selected.eval()
    original = Policy()
    original.load_state_dict(torch.load(args.original / family / f"initial-{seed}.pt", weights_only=True))
    original.eval()
    warm = Policy()
    warm.load_state_dict(torch.load(base / f"initial-{seed}.pt", weights_only=True))
    warm.eval()

    evaluations = {
        "learned": combine([evaluate(selected, role, "sampled", test_seed) for test_seed in args.test_seeds]),
        "original_untrained": combine([evaluate(original, role, "sampled", test_seed) for test_seed in args.test_seeds]),
        "before_v8": combine([evaluate(warm, role, "sampled", test_seed) for test_seed in args.test_seeds]),
    }
    for mode in ("hold", "random", "baseline"):
        evaluations[mode] = combine([evaluate(selected, role, mode, test_seed) for test_seed in args.test_seeds])
    comparisons = {
        name: ci(np.asarray(evaluations["learned"]["utility"]) - np.asarray(value["utility"]))
        for name, value in evaluations.items() if name != "learned"
    }
    report["families"][family] = {
        "selected_seed": seed,
        "evaluation": evaluations,
        "paired": comparisons,
        "original_improvement": comparisons["original_untrained"]["ci95"][0] > 0,
        "baseline_noninferiority": comparisons["baseline"]["ci95"][0] >= -0.05,
    }
    print(family, json.dumps({"mean_utility": evaluations["learned"]["mean_utility"], "paired": comparisons}), flush=True)

(args.models / "runtime-sampling-audit.json").write_text(json.dumps(report, indent=2))

