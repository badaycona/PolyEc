"""Evaluate only the policy already locked by the Realm v15 validation selector."""
import argparse
import json
from pathlib import Path

import polyec_env as core

from realm_coplay_env_v15 import FAMILY_NAMES
from realm_policy_validation_v15 import (
    HOLDOUT_SEEDS, SPLIT_VERSION, evaluate_candidates, plot_validation, sha256,
    json_default, validation_eligibility,
)


def validate_selection(selection, validation_report, bundle, family):
    """Reject an incomplete, altered, or non-winning validation selection lock."""
    bundle = Path(bundle).resolve()
    if selection.get("split_version") != SPLIT_VERSION or selection.get("family") != family:
        raise ValueError("selection lock uses another family or seed split")
    if selection.get("holdout_used_for_selection") is not False or selection.get("peer_eligible") is not False:
        raise ValueError("selection lock already used holdout or peer eligibility")
    if selection.get("release_eligible") is not False:
        raise ValueError("validation selection cannot grant release eligibility")
    if tuple(selection.get("holdout_seeds", ())) != HOLDOUT_SEEDS:
        raise ValueError("selection lock holdout seeds differ from the frozen split")
    if selection.get("validation_seeds") != list(range(303000, 303020)):
        raise ValueError("selection lock validation seeds differ from the frozen split")
    if sha256(bundle / "manifest.json") != selection.get("selected_bundle_manifest_sha256"):
        raise ValueError("selected bundle manifest no longer matches the validation lock")
    if sha256(bundle / "policy.pt") != selection.get("selected_policy_sha256"):
        raise ValueError("selected policy weights no longer match the validation lock")
    if sha256(bundle / "initial.pt") != selection.get("selected_initial_sha256"):
        raise ValueError("selected initial weights no longer match the validation lock")
    if sha256(bundle.parent / "validation.json") != selection.get("validation_report_sha256"):
        raise ValueError("validation report no longer matches the validation lock")
    expected_charts = selection.get("validation_chart_sha256", {})
    for name in ("validation.png", "validation-outcomes.png", "validation-intents.png"):
        if sha256(bundle.parent / name) != expected_charts.get(name):
            raise ValueError(f"validation chart no longer matches the validation lock: {name}")
    if validation_report.get("split") != "validation" or validation_report.get("smoke_only") is not False:
        raise ValueError("selection references smoke data instead of validation")
    if validation_report.get("family") != family or validation_report.get("split_version") != SPLIT_VERSION:
        raise ValueError("validation report family or split is inconsistent")
    if validation_report.get("evaluation_seeds") != list(range(303000, 303020)):
        raise ValueError("validation report does not contain all frozen validation worlds")
    selected_seed = selection.get("selected_seed")
    if selected_seed not in selection.get("training_seeds", []):
        raise ValueError("selection lock has no selected training seed")
    eligible = [row for row in validation_report.get("results", []) if row.get("eligible")]
    if not eligible:
        raise ValueError("validation report contains no eligible candidate")
    winner = max(eligible, key=lambda row: row["comparisons"]["learned_vs_baseline"]["utility"]["mean"])
    if winner.get("training_seed") != selected_seed:
        raise ValueError("selected seed is not the highest-scoring validation-eligible candidate")
    if winner.get("run_hashes") != selection.get("selected_run_hashes"):
        raise ValueError("selected training run hashes differ from the validation report")
    manifest = json.loads((bundle / "manifest.json").read_text(encoding="utf-8"))
    if (manifest.get("seed") != selected_seed or manifest.get("selected_seed") != selected_seed
            or manifest.get("family") != family or manifest.get("selection") != "validation_only"
            or manifest.get("validation_eligible") is not True
            or manifest.get("peer_eligible") is not False or manifest.get("release_eligible") is not False):
        raise ValueError("bundle manifest is not the validation-selected, non-peer candidate")
    if manifest.get("selected_policy_sha256") != sha256(bundle / "policy.pt"):
        raise ValueError("bundle manifest policy hash does not match copied weights")
    if manifest.get("selected_initial_sha256") != sha256(bundle / "initial.pt"):
        raise ValueError("bundle manifest initial-weight hash does not match copied weights")
    return manifest


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--selection", type=Path, required=True,
                        help="directory containing selection.json, validation.json, and selected bundle files")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    selection_path = args.selection.resolve() / "selection.json"
    bundle = selection_path.parent
    selection = json.loads(selection_path.read_text(encoding="utf-8"))
    family = selection.get("family")
    if family not in FAMILY_NAMES or core.REALM_SCHEMA != "realm-v30":
        parser.error("holdout requires a validation-selected v15 family and current C++ contract")
    output = args.output.resolve()
    if output.exists() and any(output.iterdir()):
        parser.error("holdout output must be empty")
    validation_report = json.loads((bundle / "validation.json").read_text(encoding="utf-8"))
    try:
        manifest = validate_selection(selection, validation_report, bundle, family)
    except (ValueError, OSError, KeyError, TypeError) as exc:
        parser.error(str(exc))
    report = evaluate_candidates([bundle], family, HOLDOUT_SEEDS, "holdout")
    result = report["results"][0]
    passed = validation_eligibility(result["comparisons"])
    output.mkdir(parents=True, exist_ok=True)
    report["selected_seed"] = manifest["seed"]
    report["selection_lock_sha256"] = sha256(selection_path)
    report["selected_bundle_manifest_sha256"] = sha256(bundle / "manifest.json")
    report["r1_holdout_passed"] = passed
    (output / "holdout.json").write_text(
        json.dumps(report, indent=2, default=json_default), encoding="utf-8")
    plot_validation(report, output / "holdout.png")

    # Holdout cannot pick a replacement. It only changes whether this locked model
    # can be considered as a peer by a later, separately gated training run.
    manifest["peer_eligible"] = bool(passed)
    manifest["release_eligible"] = False
    manifest["holdout"] = {
        "split_version": SPLIT_VERSION,
        "seeds": list(HOLDOUT_SEEDS),
        "passed_r1": bool(passed),
        "report_sha256": sha256(output / "holdout.json"),
        "chart_sha256": {
            name: sha256(output / name)
            for name in ("holdout.png", "holdout-outcomes.png", "holdout-intents.png")
        },
    }
    (bundle / "manifest.json").write_text(
        json.dumps(manifest, indent=2, default=json_default), encoding="utf-8")
    selection["peer_eligible"] = bool(passed)
    selection["release_eligible"] = False
    selection["holdout_used_for_selection"] = False
    selection["holdout_result"] = {
        "passed_r1": bool(passed),
        "report": str((output / "holdout.json").resolve()),
        "report_sha256": sha256(output / "holdout.json"),
        "bundle_manifest_sha256_after_holdout": sha256(bundle / "manifest.json"),
    }
    selection_path.write_text(
        json.dumps(selection, indent=2, default=json_default), encoding="utf-8")
    print(json.dumps({"family": family, "selected_seed": manifest["seed"],
                      "holdout_worlds": len(HOLDOUT_SEEDS), "r1_holdout_passed": passed,
                      "peer_eligible": bool(passed), "release_eligible": False,
                      "output": str(output)}, sort_keys=True))
    return 0 if passed else 2


if __name__ == "__main__":
    raise SystemExit(main())
