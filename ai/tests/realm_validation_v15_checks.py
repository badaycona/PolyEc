"""Pure gate and split checks for the frozen Realm v15 R1 evaluator."""
import hashlib
import json
from pathlib import Path
import sys
import tempfile

AI_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(AI_ROOT / "evaluation"))
sys.path.insert(0, str(AI_ROOT / "training"))

from realm_policy_holdout_v15 import validate_selection
from realm_select_v15 import select_candidate
from realm_policy_validation_v15 import (
    HOLDOUT_SEEDS, VALIDATION_SEEDS, _load_candidate, json_default, paired_ci,
    plot_validation, validation_eligibility,
)
from realm_train_set_v15 import run_paths
import polyec_env as core
from realm_coplay_env_v15 import RealmCoPlayBatchV15, activate_timeline_contract


def check(name, condition):
    if not condition:
        raise AssertionError(name)
    print(f"PASS {name}")


def file_hash(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def eligible_summary():
    def metric(low, high, mean=0.0):
        return {"mean": mean, "ci95": [low, high]}
    return {
        "learned_vs_baseline": {"utility": metric(-0.04, 0.03, 0.01), "outcomes": {
            "alive_delta_vs_baseline": metric(0.0, 0.1),
            "insolvent_delta_vs_baseline": metric(-0.1, 0.0),
            "defaults_delta_vs_baseline": metric(-0.1, 0.0),
            "obligations_due_delta_vs_baseline": metric(-0.1, 0.0),
        }},
        "learned_vs_initial": {"utility": metric(0.001, 0.1, 0.03), "outcomes": {}},
    }


def test_lock_accepts_only_frozen_validation_winner():
    with tempfile.TemporaryDirectory(prefix="realm-v15-lock-check-") as directory:
        root = Path(directory)
        bundle = root / "selected"
        bundle.mkdir()
        (root / "validation.json").write_text("{}", encoding="utf-8")
        for name in ("validation.png", "validation-outcomes.png", "validation-intents.png"):
            (root / name).write_bytes(name.encode())
        (bundle / "policy.pt").write_bytes(b"test weights; not a model")
        (bundle / "initial.pt").write_bytes(b"test initial weights; not a model")
        validation = {
            "split": "validation", "split_version": "realm-v15-r1-seed-split-1",
            "smoke_only": False, "family": "economic",
            "evaluation_seeds": list(VALIDATION_SEEDS), "results": [
                {"training_seed": 20261101, "eligible": True,
                 "comparisons": {"learned_vs_baseline": {"utility": {"mean": 0.02}}},
                 "run_hashes": {"manifest.json": "a", "policy.pt": "b", "initial.pt": "c"}},
                {"training_seed": 20261102, "eligible": True,
                 "comparisons": {"learned_vs_baseline": {"utility": {"mean": 0.01}}},
                 "run_hashes": {"manifest.json": "d", "policy.pt": "e", "initial.pt": "f"}},
                {"training_seed": 20261103, "eligible": False,
                 "comparisons": {"learned_vs_baseline": {"utility": {"mean": 0.9}}},
                 "run_hashes": {"manifest.json": "g", "policy.pt": "h", "initial.pt": "i"}},
            ],
        }
        (root / "validation.json").write_text(json.dumps(validation), encoding="utf-8")
        manifest = {
            "seed": 20261101, "selected_seed": 20261101, "family": "economic",
            "selection": "validation_only", "validation_eligible": True,
            "peer_eligible": False, "release_eligible": False,
            "selected_policy_sha256": file_hash(bundle / "policy.pt"),
            "selected_initial_sha256": file_hash(bundle / "initial.pt"),
        }
        (bundle / "manifest.json").write_text(json.dumps(manifest), encoding="utf-8")
        selection = {
            "split_version": "realm-v15-r1-seed-split-1", "family": "economic",
            "holdout_used_for_selection": False, "peer_eligible": False,
            "release_eligible": False, "holdout_seeds": list(HOLDOUT_SEEDS),
            "validation_seeds": list(VALIDATION_SEEDS),
            "training_seeds": [20261101, 20261102, 20261103],
            "selected_seed": 20261101,
            "selected_bundle_manifest_sha256": file_hash(bundle / "manifest.json"),
            "selected_policy_sha256": file_hash(bundle / "policy.pt"),
            "selected_initial_sha256": file_hash(bundle / "initial.pt"),
            "validation_report_sha256": file_hash(root / "validation.json"),
            "validation_chart_sha256": {name: file_hash(root / name) for name in
                ("validation.png", "validation-outcomes.png", "validation-intents.png")},
            "selected_run_hashes": validation["results"][0]["run_hashes"],
        }
        validate_selection(selection, validation, bundle, "economic")
        check("validation lock accepts highest eligible seed without consulting holdout", True)
        altered = dict(selection, selected_seed=20261102)
        try:
            validate_selection(altered, validation, bundle, "economic")
        except ValueError:
            check("validation lock rejects a lower-scoring selected seed", True)
        else:
            check("validation lock rejects a lower-scoring selected seed", False)
        altered = dict(selection, holdout_seeds=list(VALIDATION_SEEDS))
        try:
            validate_selection(altered, validation, bundle, "economic")
        except ValueError:
            check("validation lock rejects validation/holdout seed reuse", True)
        else:
            check("validation lock rejects validation/holdout seed reuse", False)
        (bundle / "policy.pt").write_bytes(b"changed after selection")
        try:
            validate_selection(selection, validation, bundle, "economic")
        except ValueError:
            check("validation lock rejects changed selected weights", True)
        else:
            check("validation lock rejects changed selected weights", False)


def main():
    check("validation seeds and holdout seeds are disjoint",
          not set(VALIDATION_SEEDS).intersection(HOLDOUT_SEEDS))
    ci = paired_ci([1.0] * 20, "validation")
    check("paired confidence interval uses all 20 frozen worlds", ci["samples"] == 20
          and ci["mean"] == 1.0 and ci["ci95"] == [1.0, 1.0])
    import numpy as np
    check("report serializer converts NumPy metrics to JSON numbers",
          json.loads(json.dumps({"value": np.int64(7)}, default=json_default))["value"] == 7)
    try:
        paired_ci([0.0] * 19, "validation")
    except ValueError:
        check("paired confidence interval rejects incomplete samples", True)
    else:
        check("paired confidence interval rejects incomplete samples", False)
    check("R1 outcome and utility thresholds accept boundary-safe metrics",
          validation_eligibility(eligible_summary()))
    summary = eligible_summary()
    summary["learned_vs_baseline"]["outcomes"]["defaults_delta_vs_baseline"]["ci95"][1] = 0.001
    check("R1 rejects a confidence interval allowing more defaults", not validation_eligibility(summary))
    check("selector emits no selected model when no candidate passes", select_candidate([]) is None)
    run_dir, log_path = run_paths(Path("candidate"), 20261101)
    check("training log is outside the empty trainer output directory", log_path.parent != run_dir)
    curriculum_checks = 0
    for training_seed in (20261101, 20261102, 20261103):
        base_seed = 2**32 + training_seed * 10_000_000
        batch = RealmCoPlayBatchV15(16, base_seed, 1, 6, 512,
                                   worksite_curriculum="economic-worksite-reset-v1")
        check(f"Economic curriculum initializes all four contexts for seed {training_seed}",
              set(batch.scenarios) == {"natural", "worksite_no_input", "worksite_with_fiber",
                                       "authorized_work_at_site"})
        for index, scenario in enumerate(batch.scenarios):
            world = batch.worlds[index]
            actor = int(batch.actors[index])
            legal_transfers = batch._legal_actions(world, actor, 26)
            if scenario == "worksite_no_input":
                check(f"{training_seed} no-input context rejects transfer", not legal_transfers)
                curriculum_checks += 1
            elif scenario == "worksite_with_fiber":
                check(f"{training_seed} Fiber context has a legal transfer", bool(legal_transfers))
                curriculum_checks += 1
            elif scenario == "authorized_work_at_site":
                check(f"{training_seed} authorized context keeps Realm invariant", world.invariant())
                curriculum_checks += 1
    check("Economic curriculum rejects fixed slot assumptions across all frozen seeds",
          curriculum_checks == 36)
    with tempfile.TemporaryDirectory(prefix="realm-v15-candidate-reject-") as directory:
        run = Path(directory)
        (run / "manifest.json").write_text(json.dumps({"schema": "realm-v11"}), encoding="utf-8")
        try:
            _load_candidate(run, "economic", activate_timeline_contract(core.Realm(1, 6, 128)))
        except ValueError:
            check("candidate loader rejects legacy schema before reading weights", True)
        else:
            check("candidate loader rejects legacy schema before reading weights", False)
        (run / "manifest.json").write_text(json.dumps({
            "schema": "realm-v30", "smoke_only": True, "policy_candidate": False,
        }), encoding="utf-8")
        try:
            _load_candidate(run, "economic", activate_timeline_contract(core.Realm(1, 6, 128)))
        except ValueError:
            check("candidate loader rejects smoke run as policy evidence", True)
        else:
            check("candidate loader rejects smoke run as policy evidence", False)
    fake = eligible_summary()
    intent_counts = {"Hold": 2, "Work": 1}
    modes = {mode: {"selected_intents": intent_counts if mode == "learned" else {"Hold": 1, "Work": 2}}
             for mode in ("baseline", "learned", "initial", "hold", "random")}
    report = {
        "split": "validation", "family": "economic", "smoke_only": False,
        "evaluation_seeds": [303000],
        "results": [{"training_seed": 20261101, "comparisons": fake,
                     "worlds": [{"seed": 303000, "modes": modes}]}],
    }
    with tempfile.TemporaryDirectory(prefix="realm-v15-plot-check-") as directory:
        plot_validation(report, Path(directory) / "validation.png")
        check("validation plot writer creates utility, outcome, and Intent charts",
              all((Path(directory) / name).is_file() for name in
                  ("validation.png", "validation-outcomes.png", "validation-intents.png")))
    test_lock_accepts_only_frozen_validation_winner()
    print("PASS Realm v15 R1 pure checks")


if __name__ == "__main__":
    main()
