"""Sampled, paired Realm v11 holdout for completed focal-family PPO runs."""
import argparse
import json
from pathlib import Path
import sys

import numpy as np
import torch
import polyec_env as core

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "training"))
from realm_model_v11 import ACTIONS, HIDDEN, OBS, Policy
from realm_dev_baseline_v11 import BASELINE_VERSION

sys.path.insert(0, str(Path(__file__).resolve().parent))
from realm_coplay_v11 import FAMILIES, FAMILY_ID, paired_utility_summary, run_variant

FAMILY_NAMES = FAMILIES


def load_run(path, family):
    path = Path(path)
    manifest = json.loads((path / "manifest.json").read_text(encoding="utf-8"))
    world = core.Realm(1, 6, 128)
    expected = {
        "family": family,
        "schema": "realm-v11",
        "observation": OBS,
        "actions": ACTIONS,
        "hidden": HIDDEN,
        "model_contract": world.model_contract,
        "config_fingerprint": world.config_fingerprint,
        "trained_with_rl": True,
        "imitation": False,
        "training_complete": True,
        "complete": True,
        "peer_fallback_version": BASELINE_VERSION,
        "reward_version": core.REALM_REWARD_VERSION,
    }
    if any(manifest.get(key) != value for key, value in expected.items()):
        raise ValueError(f"training run is incomplete or incompatible with {family}: {path}")
    seed = manifest.get("seed")
    if not isinstance(seed, int):
        raise ValueError(f"training manifest has no integer seed: {path}")
    learned = Policy()
    learned.load_state_dict(torch.load(path / "policy.pt", map_location="cpu", weights_only=True))
    learned.eval()
    initial = Policy()
    initial.load_state_dict(torch.load(path / "initial.pt", map_location="cpu", weights_only=True))
    initial.eval()
    return seed, learned, initial, manifest


def paired_metric_summary(base, trial, family, key, margin=0.0, lower_is_better=False):
    differences = np.asarray([
        trial_run["representative_outcomes"][family][key]
        - base_run["representative_outcomes"][family][key]
        for base_run, trial_run in zip(base, trial)
    ], dtype=np.float64)
    if len(differences) < 2:
        return {"mean": float(differences.mean()), "ci95": None, "samples": len(differences)}
    standard_error = differences.std(ddof=1) / np.sqrt(len(differences))
    t95 = {2: 12.706, 3: 4.303, 4: 3.182, 5: 2.776, 6: 2.571,
           7: 2.447, 8: 2.365, 9: 2.306, 10: 2.262}.get(len(differences), 1.96)
    mean = float(differences.mean())
    lower, upper = float(mean - t95 * standard_error), float(mean + t95 * standard_error)
    return {"mean": mean, "ci95": [lower, upper],
            "samples": len(differences), "noninferiority_margin": margin,
            "lower_is_better": lower_is_better,
            "noninferior": bool(upper <= margin if lower_is_better else lower >= -margin)}


def evaluate_run(run, family, holdout_seeds, regions, days):
    training_seed, learned, initial, manifest = run
    family_id = FAMILY_ID[family]
    policies = {"learned": {family_id: learned}, "initial": {family_id: initial}}
    controllers = {family_id: "learned"}
    baseline = [run_variant(seed, regions, days, {}, policies, seed) for seed in holdout_seeds]
    learned_runs = [run_variant(seed, regions, days, controllers, policies, seed) for seed in holdout_seeds]
    controllers[family_id] = "initial"
    initial_runs = [run_variant(seed, regions, days, controllers, policies, seed) for seed in holdout_seeds]
    controllers[family_id] = "hold"
    hold_runs = [run_variant(seed, regions, days, controllers, policies, seed) for seed in holdout_seeds]
    controllers[family_id] = "random"
    random_runs = [run_variant(seed, regions, days, controllers, policies, seed) for seed in holdout_seeds]
    utility = {
        "learned_vs_baseline": paired_utility_summary(baseline, learned_runs)[family],
        "learned_vs_initial": paired_utility_summary(initial_runs, learned_runs)[family],
        "learned_vs_hold": paired_utility_summary(hold_runs, learned_runs)[family],
        "learned_vs_random_legal": paired_utility_summary(random_runs, learned_runs)[family],
    }
    outcomes = {}
    for metric, margin, lower_is_better in (("alive", 0.0, False), ("health", 2.0, False),
                                            ("cash", 0.0, False), ("debt", 0.0, True),
                                            ("insolvent", 0.0, True),
                                            ("obligations_due", 0.0, True),
                                            ("defaults", 0.0, True)):
        outcomes[metric + "_delta_vs_baseline"] = paired_metric_summary(
            baseline, learned_runs, family, metric, margin, lower_is_better
        )
    return {
        "training_seed": training_seed,
        "decisions": manifest.get("decisions"),
        "updates": manifest.get("completed_updates"),
        "holdout_seeds": holdout_seeds,
        "utility": utility,
        "outcomes": outcomes,
        "worksite_actions": [item["worksite_actions_by_family"][family] for item in learned_runs],
        "work_authorizations": [item["work_authorizations_by_family"][family] for item in learned_runs],
        "work_submissions": [item["work_submissions_by_family"][family] for item in learned_runs],
        "action_counts": [item["action_counts_by_family"][family] for item in learned_runs],
        "establishment_count": [item["establishment_count"] for item in learned_runs],
        "cloth_stock": [item["cloth_stock"] for item in learned_runs],
        "fiber_stock": [item["fiber_stock"] for item in learned_runs],
        "paid_site_work_units": [item["work_units_paid_at_sites"] for item in learned_runs],
        "invariants": all(item["invariant"] for item in baseline + learned_runs + initial_runs + hold_runs + random_runs),
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--family", choices=FAMILY_NAMES, required=True)
    parser.add_argument("--runs", type=Path, nargs="+", required=True)
    parser.add_argument("--holdout-seeds", type=int, nargs="+", default=[180000, 180099])
    parser.add_argument("--regions", type=int, default=6)
    parser.add_argument("--days", type=int, default=128)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--selected-bundle", type=Path)
    parser.add_argument("--smoke", action="store_true")
    args = parser.parse_args()
    if core.REALM_SCHEMA != "realm-v11" or core.REALM_OBS != OBS or core.REALM_ACTIONS != ACTIONS:
        parser.error("holdout requires the Realm v11 C++ binding")
    if args.regions < 1 or args.regions > 6 or args.days < 1 or args.days > 8192:
        parser.error("regions must be 1..6 and days 1..8192")
    if len(args.holdout_seeds) != len(set(args.holdout_seeds)) or not args.holdout_seeds:
        parser.error("holdout seeds must be non-empty and unique")
    if not args.smoke and (len(args.runs) < 3 or len(args.holdout_seeds) < 100):
        parser.error("full R1 evaluation requires at least 3 training runs and 100 paired holdout seeds")
    loaded = [load_run(path, args.family) for path in args.runs]
    train_seeds = [run[0] for run in loaded]
    if len(set(train_seeds)) != len(train_seeds):
        parser.error("each run must use a distinct training seed")
    selection = None
    if args.selected_bundle:
        selection_path = args.selected_bundle / "selection.json"
        if not selection_path.exists():
            parser.error("selected bundle lacks validation selection record")
        selection = json.loads(selection_path.read_text(encoding="utf-8"))
        if (selection.get("family") != args.family
                or selection.get("reward_version") != core.REALM_REWARD_VERSION
                or selection.get("selected_seed") is None
                or set(selection.get("training_seeds", [])) != set(train_seeds)
                or set(selection.get("validation_seeds", [])) & set(args.holdout_seeds)):
            parser.error("holdout runs/seeds do not match the validation-only selection record")
    results = [evaluate_run(run, args.family, args.holdout_seeds, args.regions, args.days)
               for run in loaded]
    all_initial = all(item["utility"]["learned_vs_initial"]["ci95"] is not None
                      and item["utility"]["learned_vs_initial"]["ci95"][0] > 0 for item in results)
    all_baseline = all(item["utility"]["learned_vs_baseline"]["ci95"] is not None
                       and item["utility"]["learned_vs_baseline"]["ci95"][0] >= -0.05 for item in results)
    all_survival = all(item["outcomes"]["alive_delta_vs_baseline"]["ci95"] is not None
                       and item["outcomes"]["alive_delta_vs_baseline"]["ci95"][0] >= 0 for item in results)
    all_insolvency = all(item["outcomes"]["insolvent_delta_vs_baseline"]["ci95"] is not None
                         and item["outcomes"]["insolvent_delta_vs_baseline"]["ci95"][1] <= 0 for item in results)
    all_defaults = all(item["outcomes"]["defaults_delta_vs_baseline"]["ci95"] is not None
                       and item["outcomes"]["defaults_delta_vs_baseline"]["ci95"][1] <= 0 for item in results)
    minimum_training = all((item["updates"] or 0) >= 25 for item in results)
    focal_decisions = 512 if args.family == "economic" else 128
    days_per_decision = 7 if args.family == "economic" else 30
    required_world_days = focal_decisions * days_per_decision
    full_horizon = args.days >= required_world_days
    r1 = (args.regions == 6 and full_horizon and len(results) >= 3
          and minimum_training and len(args.holdout_seeds) >= 100 and all_initial and all_baseline
          and all_survival and all_insolvency and all_defaults)
    report = {
        "schema": core.REALM_SCHEMA, "family": args.family,
        "model_contract": core.Realm(1, args.regions, 128).model_contract,
        "regions": args.regions, "days": args.days,
        "holdout_count": len(args.holdout_seeds), "training_seeds": train_seeds,
        "paired_sampling": True, "policy_status": "sampled PPO checkpoint; not a release model",
        "criteria": {"at_least_3_training_seeds": len(results) >= 3,
                     "at_least_100_holdouts": len(args.holdout_seeds) >= 100,
                     "all_seeds_improve_initial_ci_lower_gt_0": all_initial,
                     "all_seeds_baseline_noninferior_margin_0.05": all_baseline,
                     "all_seeds_survival_noninferior": all_survival,
                     "all_seeds_insolvency_not_worse": all_insolvency,
                     "all_seeds_defaults_not_worse": all_defaults,
                     "all_runs_at_least_25_updates": minimum_training,
                     "six_regions": args.regions == 6,
                     "focal_decisions_required": focal_decisions,
                     "days_per_focal_decision": days_per_decision,
                     "required_realm_days": required_world_days,
                     "family_holdout_horizon": full_horizon},
        "r1_pass": bool(r1), "runs": results,
        "note": "R1 false means these runs cannot enter a learned-policy release or peer bundle.",
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    if r1 and selection is not None:
        bundle_manifest_path = args.selected_bundle / "manifest.json"
        bundle_manifest = json.loads(bundle_manifest_path.read_text(encoding="utf-8"))
        if (bundle_manifest.get("selected_seed") != selection["selected_seed"]
                or bundle_manifest.get("reward_version") != core.REALM_REWARD_VERSION):
            parser.error("selected bundle seed does not match its validation lock")
        bundle_manifest.update({"peer_eligible": True, "r1_pass": True,
                               "holdout_report": str(args.output),
                               "holdout_seeds": args.holdout_seeds})
        bundle_manifest_path.write_text(json.dumps(bundle_manifest, indent=2), encoding="utf-8")
    print(json.dumps({key: value for key, value in report.items() if key != "runs"}, sort_keys=True))
    for result in results:
        print(json.dumps({"training_seed": result["training_seed"], "utility": result["utility"],
                          "invariants": result["invariants"],
                          "worksite_actions": result["worksite_actions"]}, sort_keys=True))
    if not report["r1_pass"]:
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
