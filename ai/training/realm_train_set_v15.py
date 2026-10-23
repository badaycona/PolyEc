"""Run the frozen three-seed Realm v15 candidate set for one policy family."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

import onnx  # noqa: F401 - fail before creating any training output if export support is absent
import numpy as np
import polyec_env as core
import torch

from realm_coplay_env_v15 import FAMILY_NAMES, TIMELINE_CONTRACT_FIXTURE_VERSION, activate_timeline_contract
from realm_model_v15 import ACTIONS, HIDDEN, OBS
from realm_train_v15 import ACTION_DIAGNOSTICS_VERSION
from realm_dev_baseline_v15 import BASELINE_VERSION
from realm_policy_validation_v15 import DECISIONS_REQUIRED, SPLIT_VERSION, TRAINING_SEEDS, UPDATES_REQUIRED

ROOT = Path(__file__).resolve().parents[2]
TRAINING = Path(__file__).resolve().parent


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def training_identity(family):
    if family not in FAMILY_NAMES:
        raise ValueError("unknown policy family")
    world = activate_timeline_contract(core.Realm(1, 6, 128))
    return {
        "family": family,
        "schema": core.REALM_SCHEMA,
        "observation": OBS,
        "actions": ACTIONS,
        "hidden": HIDDEN,
        "model_contract": world.model_contract,
        "config_fingerprint": world.config_fingerprint,
        "reward_version": core.REALM_REWARD_VERSION,
        "action_diagnostics_version": ACTION_DIAGNOSTICS_VERSION,
        "timeline_contract_fixture": TIMELINE_CONTRACT_FIXTURE_VERSION,
        "split_version": SPLIT_VERSION,
        "training_seeds": list(TRAINING_SEEDS),
        "validation_seeds": list(range(303000, 303020)),
        "holdout_seeds": list(range(404000, 404100)),
        "regions": 6,
        "envs": 16,
        "rollout_steps": 64,
        "updates": UPDATES_REQUIRED,
        "decisions_per_run": DECISIONS_REQUIRED,
        "curriculum": "economic-worksite-reset-v1" if family == "economic" else "none",
        "peer_policy": BASELINE_VERSION,
        "algorithm": "recurrent PPO; categorical masked actions",
        "epochs": 3,
        "learning_rate": 3e-4,
        "entropy_coef": 0.01,
        "device": "cpu",
        "torch_version": torch.__version__,
        "numpy_version": np.__version__,
        "onnx_version": onnx.__version__,
        "python_version": sys.version.split()[0],
    }


def source_hashes():
    relative = (
        "ai/training/realm_model_v15.py",
        "ai/training/realm_coplay_env_v15.py",
        "ai/training/realm_dev_baseline_v15.py",
        "ai/training/realm_train_v15.py",
        "ai/evaluation/realm_coplay_v15.py",
        "ai/evaluation/realm_policy_validation_v15.py",
        "ai/evaluation/realm_select_v15.py",
        "ai/evaluation/realm_policy_holdout_v15.py",
        "ai/training/realm_train_set_v15.py",
        "scripts/plot_realm_v15_training.py",
        "core/include/polyec/realm.hpp",
        "bindings/python/realm_binding.cpp",
    )
    result = {name: sha256(ROOT / name) for name in relative}
    binding = Path(core.__file__).resolve()
    result["python_binding_binary"] = sha256(binding)
    return result


def run_paths(output, seed):
    output = Path(output)
    return output / f"seed-{seed}", output / "logs" / f"seed-{seed}.trainer.log"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--family", choices=FAMILY_NAMES, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if core.REALM_SCHEMA != "realm-v30" or core.REALM_OBS != OBS or core.REALM_ACTIONS != ACTIONS:
        parser.error("training set requires the current Realm v15 C++ contract")
    output = args.output.resolve()
    if output.exists() and any(output.iterdir()):
        parser.error("training set output must be empty; do not overwrite prior runs")
    output.mkdir(parents=True, exist_ok=True)
    identity = training_identity(args.family)
    locked_hashes = source_hashes()
    manifest = {
        "split_version": SPLIT_VERSION,
        "family": args.family,
        "identity": identity,
        "source_sha256": locked_hashes,
        "completed_runs": [],
        "complete": False,
        "candidate_set": True,
        "validation_used_for_training": False,
        "holdout_used": False,
    }
    runset_path = output / "runset.json"
    runset_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    for seed in TRAINING_SEEDS:
        if source_hashes() != locked_hashes:
            raise RuntimeError("a locked source file changed during the three-seed training set")
        run_dir, log_path = run_paths(output, seed)
        log_path.parent.mkdir(parents=True, exist_ok=True)
        command = [
            sys.executable, str(TRAINING / "realm_train_v15.py"),
            "--role", str(FAMILY_NAMES.index(args.family)),
            "--seed", str(seed), "--output", str(run_dir),
            "--regions", "6", "--envs", "16", "--rollout-steps", "64",
            "--updates", str(UPDATES_REQUIRED), "--no-time-limit",
            "--epochs", "3", "--learning-rate", "0.0003", "--entropy-coef", "0.01",
            "--worksite-curriculum", identity["curriculum"],
        ]
        with log_path.open("w", encoding="utf-8") as log:
            completed = subprocess.run(command, cwd=ROOT, env=os.environ.copy(),
                                       stdout=log, stderr=subprocess.STDOUT, check=False)
        if completed.returncode != 0:
            manifest["failed_seed"] = seed
            manifest["failed_exit_code"] = completed.returncode
            runset_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
            raise RuntimeError(f"Realm v15 training failed at seed {seed}; see {log_path}")
        run_manifest_path = run_dir / "manifest.json"
        run_manifest = json.loads(run_manifest_path.read_text(encoding="utf-8"))
        if (run_manifest.get("training_complete") is not True
                or run_manifest.get("completed_updates") != UPDATES_REQUIRED
                or run_manifest.get("decisions") != DECISIONS_REQUIRED
                or run_manifest.get("smoke_only") is not False
                or run_manifest.get("policy_candidate") is not True
                or run_manifest.get("peer_eligible") is not False):
            manifest["failed_seed"] = seed
            runset_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
            raise RuntimeError(f"trainer output is incomplete or mislabelled at seed {seed}")
        chart = run_dir / "training.png"
        plot_command = [sys.executable, str(ROOT / "scripts/plot_realm_v15_training.py"),
                        "--run", str(run_dir), "--output", str(chart)]
        plotted = subprocess.run(plot_command, cwd=ROOT, env=os.environ.copy(),
                                 capture_output=True, text=True, check=False)
        if plotted.returncode != 0:
            raise RuntimeError(f"training chart failed at seed {seed}: {plotted.stderr.strip()}")
        manifest["completed_runs"].append({
            "seed": seed,
            "path": str(run_dir.resolve()),
            "manifest_sha256": sha256(run_manifest_path),
            "policy_sha256": sha256(run_dir / "policy.pt"),
            "initial_sha256": sha256(run_dir / "initial.pt"),
            "history_sha256": sha256(run_dir / "history.jsonl"),
            "chart_sha256": sha256(chart),
            "chart_source_sha256": sha256(chart.with_suffix(".json")),
            "intent_chart_sha256": sha256(run_dir / "training-intents.png"),
            "intent_source_sha256": sha256(run_dir / "training-intents.json"),
        })
        runset_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    manifest["complete"] = True
    runset_path.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(json.dumps({"family": args.family, "seeds": list(TRAINING_SEEDS),
                      "updates_per_run": UPDATES_REQUIRED, "decisions_per_run": DECISIONS_REQUIRED,
                      "output": str(output)}, sort_keys=True))


if __name__ == "__main__":
    main()
