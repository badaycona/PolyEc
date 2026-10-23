"""Validation-only paired evaluation for completed Realm v15 training runs."""
import argparse
import hashlib
import json
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import torch
import polyec_env as core

from realm_coplay_env_v15 import FAMILY_NAMES, TIMELINE_CONTRACT_FIXTURE_VERSION, activate_timeline_contract
from realm_coplay_v15 import run_variant
from realm_model_v15 import ACTIONS, HIDDEN, OBS, Policy
from realm_train_v15 import ACTION_DIAGNOSTICS_VERSION
from realm_dev_baseline_v15 import BASELINE_VERSION

SPLIT_VERSION = "realm-v15-r1-seed-split-1"
VALIDATION_SEEDS = tuple(range(303000, 303020))
HOLDOUT_SEEDS = tuple(range(404000, 404100))
TRAINING_SEEDS = (20261101, 20261102, 20261103)
VALIDATION_T95 = 2.093024054
HOLDOUT_T95 = 1.984216952
UPDATES_REQUIRED = 25
DECISIONS_REQUIRED = 25600
WORKSITE_CURRICULUM = "economic-worksite-reset-v1"


def json_default(value):
    if isinstance(value, np.generic):
        return value.item()
    if isinstance(value, np.ndarray):
        return value.tolist()
    if isinstance(value, Path):
        return str(value)
    raise TypeError(f"unsupported validation report value: {type(value).__name__}")


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def paired_ci(differences, split):
    values = np.asarray(differences, dtype=np.float64)
    expected, critical = ((20, VALIDATION_T95) if split == "validation"
                          else (100, HOLDOUT_T95) if split == "holdout" else (None, None))
    if expected is None or values.shape != (expected,) or not np.isfinite(values).all():
        raise ValueError("paired CI requires the complete, finite, versioned seed split")
    mean = float(values.mean())
    standard_error = float(values.std(ddof=1) / np.sqrt(expected))
    return {
        "mean": mean,
        "standard_error": standard_error,
        "ci95": [mean - critical * standard_error, mean + critical * standard_error],
        "samples": expected,
        "method": "paired Student t interval, two-sided 95%, frozen critical value by split size",
    }


def _load_candidate(path, family, expected_world):
    path = Path(path)
    manifest_path = path / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    expected_curriculum = WORKSITE_CURRICULUM if family == "economic" else "none"
    expected_horizon = 512 if family == "economic" else 128
    expected_peers = {
        other: "explicit dev baseline (no checkpoint supplied)"
        for other in FAMILY_NAMES if other != family
    }
    expected = {
        "family": family,
        "schema": "realm-v30",
        "observation": OBS,
        "actions": ACTIONS,
        "hidden": HIDDEN,
        "model_contract": expected_world.model_contract,
        "config_fingerprint": expected_world.config_fingerprint,
        "reward_version": core.REALM_REWARD_VERSION,
        "action_diagnostics_version": ACTION_DIAGNOSTICS_VERSION,
        "timeline_contract_fixture": TIMELINE_CONTRACT_FIXTURE_VERSION,
        "worksite_curriculum": expected_curriculum,
        "regions": 6,
        "horizon": expected_horizon,
        "envs": 16,
        "rollout_steps": 64,
        "decisions_per_update": 1024,
        "updates_required": UPDATES_REQUIRED,
        "completed_updates": UPDATES_REQUIRED,
        "decisions": DECISIONS_REQUIRED,
        "training_complete": True,
        "complete": True,
        "trained_with_rl": True,
        "imitation": False,
        "smoke_only": False,
        "policy_candidate": True,
        "peer_eligible": False,
        "policy_selection_eligible": False,
        "release_eligible": False,
        "peer_fallback_version": BASELINE_VERSION,
        "peer_fallback_label": "dev-only observation baseline",
        "peer_policies": expected_peers,
        "algorithm": "recurrent PPO; categorical masked actions",
        "gamma": 1.0,
        "gae_lambda": 0.95,
        "clip": 0.2,
        "epochs": 3,
        "learning_rate": 3e-4,
        "entropy_coef": 0.01,
        "bptt_steps": 16,
        "minibatch_sequences": 64,
        "wall_clock_limit_enabled": False,
        "device": "cpu",
    }
    if any(manifest.get(key) != value for key, value in expected.items()):
        raise ValueError(f"candidate manifest is incomplete or differs from Realm v15 R1: {path}")
    if not isinstance(manifest.get("seed"), int) or not (path / "policy.pt").is_file() or not (path / "initial.pt").is_file():
        raise ValueError(f"candidate weights or integer training seed are missing: {path}")
    for filename in ("policy.pt", "initial.pt"):
        if (path / filename).stat().st_size == 0:
            raise ValueError(f"candidate weight file is empty: {path / filename}")
    learned, initial = Policy(), Policy()
    learned.load_state_dict(torch.load(path / "policy.pt", map_location="cpu", weights_only=True))
    initial.load_state_dict(torch.load(path / "initial.pt", map_location="cpu", weights_only=True))
    learned.eval()
    initial.eval()
    return {
        "path": str(path.resolve()), "seed": int(manifest["seed"]),
        "manifest": manifest, "learned": learned, "initial": initial,
        "hashes": {name: sha256(path / name) for name in ("manifest.json", "policy.pt", "initial.pt")},
    }


def _mode_rows(seed, family, candidate):
    rows = {"baseline": run_variant(seed, family, "baseline", None)}
    rows["learned"] = run_variant(seed, family, "learned", candidate["learned"])
    rows["initial"] = run_variant(seed, family, "initial", candidate["initial"])
    rows["hold"] = run_variant(seed, family, "hold", None)
    rows["random"] = run_variant(seed, family, "random", None)
    return rows


def _comparison(rows, comparison, split):
    baseline = rows["baseline"]
    trial = rows["learned"]
    utility = paired_ci([
        item["normalized_utility"] - baseline[index]["normalized_utility"]
        for index, item in enumerate(trial)
    ], split)
    if comparison == "initial":
        other = rows["initial"]
        utility = paired_ci([
            item["normalized_utility"] - other[index]["normalized_utility"]
            for index, item in enumerate(trial)
        ], split)
    result = {"utility": utility, "outcomes": {}}
    for field in ("alive", "insolvent", "defaults", "obligations_due"):
        differences = [
            float(item["outcome"][field]) - float(baseline[index]["outcome"][field])
            for index, item in enumerate(trial)
        ]
        result["outcomes"][field + "_delta_vs_baseline"] = paired_ci(differences, split)
    return result


def validation_eligibility(summary):
    baseline = summary["learned_vs_baseline"]["utility"]["ci95"]
    initial = summary["learned_vs_initial"]["utility"]["ci95"]
    outcomes = summary["learned_vs_baseline"]["outcomes"]
    return bool(
        baseline[0] >= -0.05 and initial[0] > 0
        and outcomes["alive_delta_vs_baseline"]["ci95"][0] >= 0
        and outcomes["insolvent_delta_vs_baseline"]["ci95"][1] <= 0
        and outcomes["defaults_delta_vs_baseline"]["ci95"][1] <= 0
        and outcomes["obligations_due_delta_vs_baseline"]["ci95"][1] <= 0
    )


def evaluate_candidates(paths, family, seeds, split, smoke=False):
    if family not in FAMILY_NAMES:
        raise ValueError("unknown Realm policy family")
    expected_seeds = tuple(seeds)
    if split == "validation" and not smoke and expected_seeds != VALIDATION_SEEDS:
        raise ValueError("validation seeds differ from the frozen v15 split")
    if split == "holdout" and expected_seeds != HOLDOUT_SEEDS:
        raise ValueError("holdout seeds differ from the frozen v15 split")
    if smoke and (split != "validation" or not expected_seeds or len(expected_seeds) > 2):
        raise ValueError("evaluation smoke accepts at most two validation worlds only")
    expected_world = activate_timeline_contract(core.Realm(1, 6, 128))
    candidates = [_load_candidate(path, family, expected_world) for path in paths]
    if not smoke and split == "validation" and len(candidates) != 3:
        raise ValueError("R1 validation requires exactly three training runs")
    if not smoke and split == "holdout" and len(candidates) != 1:
        raise ValueError("R1 holdout accepts only the single validation-selected training run")
    if not smoke and split not in ("validation", "holdout"):
        raise ValueError("unknown evaluation split")
    training_seeds = [candidate["seed"] for candidate in candidates]
    if not smoke and split == "validation" and (
            len(set(training_seeds)) != 3 or set(training_seeds) != set(TRAINING_SEEDS)):
        raise ValueError("candidate training seeds do not match the frozen v15 split")
    if not smoke and split == "holdout" and training_seeds[0] not in TRAINING_SEEDS:
        raise ValueError("selected holdout run was not trained with a frozen v15 training seed")
    locked_fields = (
        "family", "schema", "observation", "actions", "hidden", "model_contract",
        "config_fingerprint", "reward_version", "action_diagnostics_version",
        "timeline_contract_fixture", "worksite_curriculum", "regions", "envs",
        "rollout_steps", "updates_required", "decisions", "epochs", "learning_rate",
        "entropy_coef", "algorithm", "peer_policies", "peer_fallback_version",
    )
    reference = candidates[0]["manifest"]
    for candidate in candidates[1:]:
        if any(candidate["manifest"].get(field) != reference.get(field) for field in locked_fields):
            raise ValueError("candidate runs differ in training family, budget, peers, model, or config")

    results = []
    for candidate in candidates:
        per_seed = []
        for seed in expected_seeds:
            modes = _mode_rows(seed, family, candidate)
            per_seed.append({"seed": seed, "modes": modes})
        if smoke:
            comparisons = None
            eligible = False
        else:
            modes = {
                name: [item["modes"][name] for item in per_seed]
                for name in ("baseline", "learned", "initial", "hold", "random")
            }
            comparisons = {
                "learned_vs_baseline": _comparison(modes, "baseline", split),
                "learned_vs_initial": _comparison(modes, "initial", split),
            }
            eligible = validation_eligibility(comparisons)
        results.append({
            "training_seed": candidate["seed"], "run_path": candidate["path"],
            "run_hashes": candidate["hashes"], "completed_updates": reference["completed_updates"],
            "decisions": reference["decisions"], "eligible": eligible,
            "comparisons": comparisons, "worlds": per_seed,
        })
    return {
        "split_version": SPLIT_VERSION, "split": split, "smoke_only": bool(smoke),
        "family": family, "schema": core.REALM_SCHEMA,
        "model_contract": expected_world.model_contract,
        "config_fingerprint": expected_world.config_fingerprint,
        "observation": OBS, "actions": ACTIONS,
        "reward_version": core.REALM_REWARD_VERSION,
        "training_seeds": training_seeds, "evaluation_seeds": list(expected_seeds),
        "paired_modes": ["baseline", "learned", "initial", "hold", "random"],
        "peer_policy": BASELINE_VERSION, "results": results,
    }


def plot_validation(report, output):
    if report["smoke_only"]:
        raise ValueError("smoke evaluation must not be plotted as validation selection evidence")
    labels, baseline_means, baseline_errors, initial_means, initial_errors = [], [], [], [], []
    for result in report["results"]:
        labels.append(f"training seed {result['training_seed']}")
        baseline = result["comparisons"]["learned_vs_baseline"]["utility"]
        initial = result["comparisons"]["learned_vs_initial"]["utility"]
        baseline_means.append(baseline["mean"])
        baseline_errors.append((baseline["ci95"][1] - baseline["ci95"][0]) / 2)
        initial_means.append(initial["mean"])
        initial_errors.append((initial["ci95"][1] - initial["ci95"][0]) / 2)
    fig, axes = plt.subplots(1, 2, figsize=(11, 4.5), constrained_layout=True)
    y = np.arange(len(labels))
    axes[0].errorbar(baseline_means, y, xerr=baseline_errors, fmt="o", capsize=4, color="#2364aa")
    axes[0].axvline(-0.05, color="#d1495b", linestyle="--", label="allowed non-inferiority margin")
    axes[0].axvline(0, color="#555555", linewidth=0.8)
    axes[0].set_yticks(y, labels)
    axes[0].set_title("Learned minus dev baseline utility")
    axes[0].set_xlabel("Paired normalized utility difference with 95% CI")
    axes[0].legend(fontsize=8)
    axes[0].grid(True, axis="x", alpha=0.25)
    axes[1].errorbar(initial_means, y, xerr=initial_errors, fmt="o", capsize=4, color="#2a9d8f")
    axes[1].axvline(0, color="#d1495b", linestyle="--", label="must be greater than zero")
    axes[1].set_yticks(y, labels)
    axes[1].set_title("Learned minus initial policy utility")
    axes[1].set_xlabel("Paired normalized utility difference with 95% CI")
    axes[1].legend(fontsize=8)
    axes[1].grid(True, axis="x", alpha=0.25)
    fig.suptitle(f"Realm v15 {report['split']} — {report['family']}; {len(report['evaluation_seeds'])} paired worlds")
    output = Path(output)
    output.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(output, dpi=160)
    plt.close(fig)

    fields = ("alive", "insolvent", "defaults", "obligations_due")
    fig, ax = plt.subplots(figsize=(10, 5), constrained_layout=True)
    positions, labels, means, errors = [], [], [], []
    for candidate_index, result in enumerate(report["results"]):
        outcomes = result["comparisons"]["learned_vs_baseline"]["outcomes"]
        for field_index, field in enumerate(fields):
            metric = outcomes[field + "_delta_vs_baseline"]
            positions.append(candidate_index * (len(fields) + 1) + field_index)
            labels.append(field.replace("_", " "))
            means.append(metric["mean"])
            errors.append((metric["ci95"][1] - metric["ci95"][0]) / 2)
    ax.errorbar(means, positions, xerr=errors, fmt="o", capsize=3, color="#6a4c93")
    ax.axvline(0, color="#555555", linewidth=0.8)
    outcome_labels = [f"{result['training_seed']}: {field.replace('_', ' ')}"
                      for result in report["results"] for field in fields]
    ax.set_yticks(positions, outcome_labels)
    ax.set_xlabel("Learned minus baseline paired outcome difference with 95% CI")
    ax.set_title("Survival and adverse outcomes across the same worlds")
    ax.grid(True, axis="x", alpha=0.25)
    fig.savefig(output.with_name(output.stem + "-outcomes.png"), dpi=160)
    plt.close(fig)

    intent_names = list(report["results"][0]["worlds"][0]["modes"]["learned"]["selected_intents"])
    intent_delta = []
    for result in report["results"]:
        totals = {mode: {intent: 0 for intent in intent_names}
                  for mode in ("baseline", "learned")}
        for world in result["worlds"]:
            for mode in totals:
                for intent, count in world["modes"][mode]["selected_intents"].items():
                    totals[mode][intent] += int(count)
        learned_total = max(1, sum(totals["learned"].values()))
        baseline_total = max(1, sum(totals["baseline"].values()))
        intent_delta.append([
            totals["learned"][intent] / learned_total - totals["baseline"][intent] / baseline_total
            for intent in intent_names
        ])
    fig, ax = plt.subplots(figsize=(14, max(5, len(intent_delta) * 0.7)), constrained_layout=True)
    image = ax.imshow(np.asarray(intent_delta), aspect="auto", interpolation="nearest",
                      cmap="coolwarm", vmin=-max(0.01, float(np.max(np.abs(intent_delta)))),
                      vmax=max(0.01, float(np.max(np.abs(intent_delta)))))
    ax.set_yticks(np.arange(len(labels) // len(fields)),
                  [f"training seed {result['training_seed']}" for result in report["results"]])
    ax.set_xticks(np.arange(len(intent_names)), intent_names, rotation=65, ha="right")
    ax.set_title("Learned minus baseline share of selected actions by Intent")
    ax.set_ylabel("Each row is one training candidate; each column is an Intent")
    fig.colorbar(image, ax=ax, label="Difference in action share across paired worlds")
    fig.savefig(output.with_name(output.stem + "-intents.png"), dpi=160)
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--family", choices=FAMILY_NAMES, required=True)
    parser.add_argument("--runs", type=Path, nargs="+")
    parser.add_argument("--smoke-checkpoint", type=Path)
    parser.add_argument("--smoke-seed", type=int, action="append")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if core.REALM_SCHEMA != "realm-v30" or core.REALM_OBS != OBS or core.REALM_ACTIONS != ACTIONS:
        parser.error("validation requires the Realm v15 C++ contract")
    if args.output.exists() and any(args.output.iterdir()):
        parser.error("validation output must be empty")
    if args.smoke_checkpoint:
        if args.runs or not args.smoke_seed or len(args.smoke_seed) > 2:
            parser.error("smoke mode takes a checkpoint and one or two explicit smoke seeds")
        checkpoint = torch.load(args.smoke_checkpoint / "checkpoint.pt", map_location="cpu", weights_only=False)
        manifest = json.loads((args.smoke_checkpoint / "manifest.json").read_text(encoding="utf-8"))
        if manifest.get("smoke_only") is not True or checkpoint.get("identity", {}).get("schema") != "realm-v30":
            parser.error("smoke evaluator accepts only a v15 smoke checkpoint")
        candidate = {
            "policy": checkpoint["policy"],
            "initial": torch.load(args.smoke_checkpoint / "initial.pt", map_location="cpu", weights_only=True),
        }
        models = {}
        for mode in ("learned", "initial"):
            model = Policy()
            model.load_state_dict(candidate["policy"] if mode == "learned" else candidate["initial"])
            model.eval()
            models[mode] = model
        worlds = []
        for seed in args.smoke_seed:
            worlds.append({"seed": seed, "modes": {
                mode: run_variant(seed, args.family, mode, models.get(mode))
                for mode in ("baseline", "learned", "initial", "hold", "random")
            }})
        report = {
            "split_version": SPLIT_VERSION, "split": "evaluator-smoke-only", "smoke_only": True,
            "family": args.family, "schema": core.REALM_SCHEMA,
            "model_contract": checkpoint["identity"]["model_contract"],
            "source_smoke_manifest_sha256": sha256(args.smoke_checkpoint / "manifest.json"),
            "evaluation_seeds": args.smoke_seed, "worlds": worlds,
            "note": "short validation-runner smoke with a non-candidate adapter checkpoint; never selection or policy evidence",
        }
    else:
        if not args.runs:
            parser.error("full validation requires completed candidate run directories")
        report = evaluate_candidates(args.runs, args.family, VALIDATION_SEEDS, "validation")
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "validation.json").write_text(json.dumps(report, indent=2, default=json_default), encoding="utf-8")
    if report["smoke_only"]:
        # Smoke receives its own plot and cannot be consumed by the validation selector.
        utilities = {mode: [item["modes"][mode]["normalized_utility"] for item in report["worlds"]]
                     for mode in ("baseline", "learned", "initial", "hold", "random")}
        fig, ax = plt.subplots(figsize=(8, 4.5), constrained_layout=True)
        names = list(utilities)
        means = [float(np.mean(utilities[name])) for name in names]
        ax.bar(names, means, color=["#9aa6b2", "#2364aa", "#2a9d8f", "#d9a441", "#888888"])
        ax.set_ylabel("Normalized utility; smoke worlds only")
        ax.set_title("Realm v15 evaluator smoke — not policy validation")
        ax.grid(True, axis="y", alpha=0.25)
        fig.savefig(args.output / "validation-smoke.png", dpi=160)
        plt.close(fig)
    else:
        plot_validation(report, args.output / "validation.png")
    print(json.dumps({"family": args.family, "smoke_only": report["smoke_only"],
                      "seeds": report["evaluation_seeds"], "output": str(args.output)}, sort_keys=True))


if __name__ == "__main__":
    main()
