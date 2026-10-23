"""Create training curves and selected-Intent coverage from one Realm v15 run."""
import argparse
import json
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    manifest = json.loads((args.run / "manifest.json").read_text(encoding="utf-8"))
    if (manifest.get("schema") != "realm-v30" or manifest.get("observation") != 3550
            or manifest.get("actions") != 264 or manifest.get("run_kind") != "training_candidate"
            or manifest.get("smoke_only") is not False or manifest.get("training_complete") is not True
            or manifest.get("completed_updates") != manifest.get("updates_required")):
        parser.error("training chart requires a complete Realm v15 candidate run")
    rows = [json.loads(line) for line in (args.run / "history.jsonl").read_text(encoding="utf-8").splitlines()
            if line.strip()]
    if len(rows) != manifest["completed_updates"] or any(row.get("update") != index + 1
                                                          for index, row in enumerate(rows)):
        parser.error("training history is incomplete or update sequence is not contiguous")
    updates = np.asarray([row["update"] for row in rows])
    decisions = np.asarray([row["decisions"] for row in rows])
    fig, axes = plt.subplots(1, 2, figsize=(11, 4.5), constrained_layout=True)
    axes[0].plot(decisions, [row["reward_mean"] for row in rows], marker=".", color="#2364aa")
    axes[0].set_title("Mean C++ transition reward")
    axes[0].set_xlabel("Focal policy decisions")
    axes[0].set_ylabel("Mean reward per rollout decision")
    axes[0].grid(True, alpha=0.25)
    axes[1].plot(decisions, [row["return_distribution"]["mean"] for row in rows], marker=".", color="#d1495b")
    axes[1].set_title("Mean PPO return target")
    axes[1].set_xlabel("Focal policy decisions")
    axes[1].set_ylabel("Mean critic target in rollout")
    axes[1].grid(True, alpha=0.25)
    fig.suptitle(f"Realm v15 {manifest['family']} training seed {manifest['seed']} — raw update means")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(args.output, dpi=160)
    plt.close(fig)
    source = {
        "family": manifest["family"], "seed": manifest["seed"],
        "schema": manifest["schema"], "model_contract": manifest["model_contract"],
        "reward_version": manifest["reward_version"],
        "updates": updates.tolist(), "decisions": decisions.tolist(), "history": rows,
        "note": "Raw update means from this training run. No smoothing or cross-run averaging.",
    }
    args.output.with_suffix(".json").write_text(json.dumps(source, indent=2), encoding="utf-8")

    intents = list(rows[0]["action_intent_diagnostics"])
    matrix = np.asarray([
        [row["action_intent_diagnostics"][name]["decisions"] for name in intents]
        for row in rows
    ], dtype=np.int64)
    fig, ax = plt.subplots(figsize=(12, max(6, len(intents) * 0.25)), constrained_layout=True)
    image = ax.imshow(matrix.T, aspect="auto", interpolation="nearest", cmap="Blues")
    ax.set_yticks(np.arange(len(intents)), intents)
    ax.set_xticks(np.arange(len(rows)), [str(value) for value in updates])
    ax.set_xlabel("PPO update")
    ax.set_title(f"Intent selected per update — {manifest['family']} seed {manifest['seed']}")
    ax.set_ylabel("Intent (a blank row means zero sampled actions in every shown update)")
    fig.colorbar(image, ax=ax, label="Selected focal actions in the update")
    intent_output = args.output.with_name(args.output.stem + "-intents.png")
    fig.savefig(intent_output, dpi=160)
    plt.close(fig)
    intent_source = {
        "family": manifest["family"], "seed": manifest["seed"],
        "decisions": decisions.tolist(), "intent_names": intents,
        "selected_decisions_by_update": matrix.tolist(),
        "note": "Counts from the action diagnostics of this training run only.",
    }
    intent_output.with_suffix(".json").write_text(json.dumps(intent_source, indent=2), encoding="utf-8")
    print(f"Realm v15 training charts: {args.output}, {intent_output}; updates={len(rows)}")


if __name__ == "__main__":
    main()
