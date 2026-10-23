"""Select one Realm v15 training seed from validation only."""
import argparse
import json
from pathlib import Path
import shutil

import polyec_env as core

from realm_coplay_env_v15 import FAMILY_NAMES
from realm_policy_validation_v15 import (
    SPLIT_VERSION, VALIDATION_SEEDS, evaluate_candidates, json_default, plot_validation,
)
from realm_train_set_v15 import sha256, source_hashes


def select_candidate(results):
    eligible = [result for result in results if result["eligible"]]
    if not eligible:
        return None
    return max(eligible, key=lambda result: result["comparisons"]["learned_vs_baseline"]["utility"]["mean"])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--family", choices=FAMILY_NAMES, required=True)
    parser.add_argument("--runset", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    runset_path = args.runset / "runset.json"
    runset = json.loads(runset_path.read_text(encoding="utf-8"))
    if (runset.get("complete") is not True or runset.get("candidate_set") is not True
            or runset.get("holdout_used") is not False or runset.get("validation_used_for_training") is not False
            or runset.get("split_version") != SPLIT_VERSION or runset.get("family") != args.family):
        parser.error("runset is incomplete, mislabeled, or not eligible for v15 validation")
    if runset.get("source_sha256") != source_hashes():
        parser.error("current evaluator/trainer source does not match the locked runset hashes")
    runs = runset.get("completed_runs", [])
    seeds = [item.get("seed") for item in runs]
    if len(runs) != 3 or set(seeds) != {20261101, 20261102, 20261103}:
        parser.error("validation requires the three fixed, distinct training seeds")
    for item in runs:
        run_path = Path(item["path"]).resolve()
        locked_files = {
            "manifest.json": "manifest_sha256", "policy.pt": "policy_sha256",
            "initial.pt": "initial_sha256", "history.jsonl": "history_sha256",
            "training.png": "chart_sha256", "training.json": "chart_source_sha256",
            "training-intents.png": "intent_chart_sha256",
            "training-intents.json": "intent_source_sha256",
        }
        if not run_path.is_dir() or any(
                sha256(run_path / filename) != item.get(hash_key)
                for filename, hash_key in locked_files.items()):
            parser.error(f"completed run files no longer match the locked runset entry: {run_path}")
    output = args.output.resolve()
    if output.exists() and any(output.iterdir()):
        parser.error("selection output must be empty")
    report = evaluate_candidates([Path(item["path"]) for item in runs], args.family,
                                 VALIDATION_SEEDS, "validation")
    selected = select_candidate(report["results"])
    output.mkdir(parents=True, exist_ok=True)
    (output / "validation.json").write_text(
        json.dumps(report, indent=2, default=json_default), encoding="utf-8")
    plot_validation(report, output / "validation.png")
    lock = {
        "split_version": SPLIT_VERSION,
        "family": args.family,
        "schema": core.REALM_SCHEMA,
        "model_contract": report["model_contract"],
        "config_fingerprint": report["config_fingerprint"],
        "reward_version": core.REALM_REWARD_VERSION,
        "training_seeds": [int(seed) for seed in seeds],
        "validation_seeds": list(VALIDATION_SEEDS),
        "holdout_seeds": list(range(404000, 404100)),
        "selection_rule_version": "realm-v15-r1-validation-rule-1",
        "selection_rule": {
            "learned_minus_baseline_utility_ci95_lower_at_least": -0.05,
            "learned_minus_initial_utility_ci95_lower_greater_than": 0.0,
            "alive_delta_ci95_lower_at_least": 0.0,
            "insolvent_delta_ci95_upper_at_most": 0.0,
            "defaults_delta_ci95_upper_at_most": 0.0,
            "obligations_due_delta_ci95_upper_at_most": 0.0,
            "minimum_updates": 25,
            "validation_worlds": 20,
            "paired_student_t_ci95": True,
        },
        "results": report["results"],
        "selected_seed": None if selected is None else selected["training_seed"],
        "peer_eligible": False,
        "release_eligible": False,
        "holdout_used_for_selection": False,
    }
    if selected is not None:
        source = Path(selected["run_path"])
        shutil.copy2(source / "policy.pt", output / "policy.pt")
        shutil.copy2(source / "initial.pt", output / "initial.pt")
        manifest = json.loads((source / "manifest.json").read_text(encoding="utf-8"))
        selected_hashes = selected["run_hashes"]
        manifest.update({
            "selected_seed": selected["training_seed"],
            "selection": "validation_only",
            "selection_rule_version": lock["selection_rule_version"],
            "validation_seeds": list(VALIDATION_SEEDS),
            "validation_eligible": True,
            "peer_eligible": False,
            "release_eligible": False,
            "selected_policy_sha256": selected_hashes["policy.pt"],
            "selected_initial_sha256": selected_hashes["initial.pt"],
            "selected_source_manifest_sha256": selected_hashes["manifest.json"],
        })
        (output / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
        lock["selected_run_path"] = selected["run_path"]
        lock["selected_run_hashes"] = selected["run_hashes"]
        lock["selected_validation_mean_vs_baseline"] = selected["comparisons"][
            "learned_vs_baseline"]["utility"]["mean"]
        lock["selected_bundle_manifest_sha256"] = sha256(output / "manifest.json")
        lock["selected_policy_sha256"] = sha256(output / "policy.pt")
        lock["selected_initial_sha256"] = sha256(output / "initial.pt")
    lock["validation_report_sha256"] = sha256(output / "validation.json")
    lock["validation_chart_sha256"] = {
        name: sha256(output / name) for name in
        ("validation.png", "validation-outcomes.png", "validation-intents.png")
    }
    (output / "selection.json").write_text(
        json.dumps(lock, indent=2, default=json_default), encoding="utf-8")
    print(json.dumps({"family": args.family, "training_seeds": seeds,
                      "validation_seeds": list(VALIDATION_SEEDS),
                      "eligible_seeds": [item["training_seed"] for item in report["results"] if item["eligible"]],
                      "selected_seed": lock["selected_seed"], "peer_eligible": False}, sort_keys=True))
    return 0 if selected is not None else 2


if __name__ == "__main__":
    raise SystemExit(main())
