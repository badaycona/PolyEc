"""Select a v11 training seed on validation only; output cannot become a peer yet."""
import argparse
import hashlib
import json
from pathlib import Path
import shutil
import sys

import polyec_env as core

sys.path.insert(0, str(Path(__file__).resolve().parent))
from realm_policy_holdout_v11 import evaluate_run, load_run
from realm_coplay_v11 import FAMILIES as FAMILY_NAMES, WORKSITE_ACTION_COUNT_VERSION


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--family", choices=FAMILY_NAMES, required=True)
    parser.add_argument("--runs", type=Path, nargs="+", required=True)
    parser.add_argument("--validation-seeds", type=int, nargs="+", required=True)
    parser.add_argument("--regions", type=int, default=6)
    parser.add_argument("--days", type=int, default=128)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if core.REALM_SCHEMA != "realm-v11":
        parser.error("validation selection requires Realm v11")
    if len(args.runs) < 3:
        parser.error("selection requires at least three independent training runs")
    if len(args.validation_seeds) < 20 or len(set(args.validation_seeds)) != len(args.validation_seeds):
        parser.error("validation needs at least 20 distinct world seeds")
    if args.output.exists() and any(args.output.iterdir()):
        parser.error("selection output directory must be empty")
    loaded = [load_run(path, args.family) for path in args.runs]
    manifests = [item[3] for item in loaded]
    seeds = [item[0] for item in loaded]
    if len(set(seeds)) != len(seeds):
        parser.error("training seeds must be distinct")
    locked_fields = ("family", "schema", "observation", "actions", "hidden", "model_contract", "reward_version",
                     "config_fingerprint", "regions", "envs", "horizon", "rollout_steps",
                     "minibatch_sequences", "updates_required", "decisions", "epochs", "learning_rate",
                     "entropy_coef", "algorithm", "peer_policies", "worksite_curriculum",
                     "action_diagnostics_version")
    reference = manifests[0]
    if args.regions != reference.get("regions"):
        parser.error("validation region count must match the training run")
    for manifest in manifests[1:]:
        if any(manifest.get(field) != reference.get(field) for field in locked_fields):
            parser.error("training runs differ in family, model contract, peer setup, or sample budget")
    results = [evaluate_run(run, args.family, args.validation_seeds, args.regions, args.days)
               for run in loaded]
    candidates = []
    for result in results:
        result["worksite_action_count_version"] = WORKSITE_ACTION_COUNT_VERSION
        baseline_ci = result["utility"]["learned_vs_baseline"]["ci95"]
        initial_ci = result["utility"]["learned_vs_initial"]["ci95"]
        survival_ci = result["outcomes"]["alive_delta_vs_baseline"]["ci95"]
        insolvency_ci = result["outcomes"]["insolvent_delta_vs_baseline"]["ci95"]
        defaults_ci = result["outcomes"]["defaults_delta_vs_baseline"]["ci95"]
        outcome_eligible = (survival_ci is not None and survival_ci[0] >= 0
                            and insolvency_ci is not None and insolvency_ci[1] <= 0
                            and defaults_ci is not None and defaults_ci[1] <= 0)
        eligible = ((result["updates"] or 0) >= 25
                    and baseline_ci is not None and initial_ci is not None
                    and baseline_ci[0] >= -0.05 and initial_ci[0] > 0
                    and outcome_eligible)
        result["validation_eligible"] = eligible
        result["validation_outcome_eligible"] = outcome_eligible
        if eligible:
            candidates.append(result)
    args.output.mkdir(parents=True, exist_ok=True)
    lock = {
        "family": args.family, "schema": core.REALM_SCHEMA,
        "reward_version": core.REALM_REWARD_VERSION,
        "model_contract": reference["model_contract"],
        "worksite_action_count_version": WORKSITE_ACTION_COUNT_VERSION,
        "training_seeds": seeds, "validation_seeds": args.validation_seeds,
        "regions": args.regions, "days": args.days,
        "selection_rule": "baseline paired CI lower >= -0.05; initial paired CI lower > 0; survival CI lower >= 0; insolvency/default CI upper <= 0; require >=25 updates; maximize validation paired mean vs baseline",
        "all_runs_same_fixed_budget": len({manifest.get("decisions") for manifest in manifests}) == 1,
        "runs": [{"seed": item[0], "path": str(path),
                  "policy_sha256": sha256(path / "policy.pt"),
                  "initial_sha256": sha256(path / "initial.pt"),
                  "decisions": manifest.get("decisions")}
                 for item, path, manifest in zip(loaded, args.runs, manifests)],
        "results": results,
        "selected_seed": None,
        "peer_eligible": False,
        "holdout_used_for_selection": False,
    }
    if candidates:
        selected_result = max(candidates,
                              key=lambda item: item["utility"]["learned_vs_baseline"]["paired_mean"])
        selected_seed = selected_result["training_seed"]
        selected_index = seeds.index(selected_seed)
        source = Path(args.runs[selected_index])
        shutil.copy2(source / "policy.pt", args.output / "policy.pt")
        shutil.copy2(source / "initial.pt", args.output / "initial.pt")
        selected_manifest = dict(manifests[selected_index])
        selected_manifest.update({
            "selected_seed": selected_seed,
            "selection": "validation_only",
            "validation_seeds": args.validation_seeds,
            "validation_eligible": True,
            "peer_eligible": False,
            "selected_policy_sha256": sha256(args.output / "policy.pt"),
        })
        (args.output / "manifest.json").write_text(
            json.dumps(selected_manifest, indent=2), encoding="utf-8"
        )
        lock["selected_seed"] = selected_seed
    (args.output / "selection.json").write_text(json.dumps(lock, indent=2), encoding="utf-8")
    print(json.dumps({"family": args.family, "training_seeds": seeds,
                      "validation_seeds": args.validation_seeds,
                      "eligible_seeds": [item["training_seed"] for item in candidates],
                      "selected_seed": lock["selected_seed"],
                      "peer_eligible": False}, sort_keys=True))
    return 0 if candidates else 2


if __name__ == "__main__":
    raise SystemExit(main())
