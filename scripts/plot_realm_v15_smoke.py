"""Create a chart and source JSON from one Realm v15 adapter smoke run."""
import argparse
import json
from pathlib import Path

import matplotlib.pyplot as plt


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    manifest = json.loads((args.run / "manifest.json").read_text(encoding="utf-8"))
    if (manifest.get("schema") != "realm-v30" or manifest.get("observation") != 3550
            or manifest.get("actions") != 264 or manifest.get("run_kind") != "smoke_only"
            or manifest.get("smoke_only") is not True or manifest.get("policy_candidate") is not False
            or manifest.get("policy_selection_eligible") is not False
            or manifest.get("release_eligible") is not False):
        parser.error("plot accepts only a non-candidate smoke for the Realm v15 contract")
    history = args.run / "history.jsonl"
    rows = [json.loads(line) for line in history.read_text(encoding="utf-8").splitlines() if line.strip()]
    if len(rows) != 1 or rows[0].get("update") != 1:
        parser.error("expected exactly one completed smoke update")
    row = rows[0]
    expected_decisions = manifest["envs"] * manifest["rollout_steps"]
    if row.get("decisions") != expected_decisions or expected_decisions != 1024:
        parser.error("smoke must contain exactly 1,024 focal decisions")
    intent_counts = {
        name: values["decisions"]
        for name, values in row["action_intent_diagnostics"].items()
        if values["decisions"]
    }
    if sum(intent_counts.values()) != expected_decisions:
        parser.error("selected Intent totals do not match the sampled decision count")

    decisions = row["decisions"]
    fig, axes = plt.subplots(1, 2, figsize=(10, 4), constrained_layout=True)
    axes[0].plot([decisions], [row["reward_mean"]], marker="o", color="#2364aa")
    axes[0].set_title("Mean reward returned by C++ Realm")
    axes[0].set_xlabel("Focal policy decisions")
    axes[0].set_ylabel("Mean reward per sampled decision")
    axes[0].grid(True, alpha=0.25)
    axes[1].plot([decisions], [row["return_distribution"]["mean"]], marker="o", color="#d1495b")
    axes[1].set_title("Mean PPO return target")
    axes[1].set_xlabel("Focal policy decisions")
    axes[1].set_ylabel("Mean critic target in rollout")
    axes[1].grid(True, alpha=0.25)
    fig.suptitle("Realm v15 adapter smoke — one update; not a policy evaluation")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(args.output, dpi=160)
    plt.close(fig)
    source = {
        "run_kind": "smoke_only",
        "model_contract": manifest["model_contract"],
        "realm_schema": manifest["schema"],
        "reward_version": manifest["reward_version"],
        "config_fingerprint": manifest["config_fingerprint"],
        "seed": manifest["seed"],
        "regions": manifest["regions"],
        "environments": manifest["envs"],
        "rollout_steps": manifest["rollout_steps"],
        "decisions": decisions,
        "selected_intent_counts": intent_counts,
        "update": row,
        "note": "Reward and PPO return are rollout diagnostics from one smoke update, not an independent policy evaluation. One point does not show a trend.",
    }
    args.output.with_suffix(".json").write_text(json.dumps(source, indent=2), encoding="utf-8")
    diagnostics = row["action_intent_diagnostics"]
    names = list(diagnostics)
    fig, axes = plt.subplots(1, 2, figsize=(12, max(6, len(names) * 0.28)), sharey=True,
                             constrained_layout=True)
    positions = list(range(len(names)))
    axes[0].barh(positions, [diagnostics[name]["legal_candidates"] for name in names], color="#9aa6b2")
    axes[0].set_title("Legal candidate appearances")
    axes[0].set_xlabel("Occurrences across sampled decisions")
    axes[1].barh(positions, [diagnostics[name]["decisions"] for name in names], color="#2364aa")
    axes[1].set_title("Actions selected")
    axes[1].set_xlabel("Count in the 1,024 focal decisions")
    axes[1].set_yticks(positions, names)
    axes[1].invert_yaxis()
    fig.suptitle("Realm v15 smoke — legal opportunities and sampled actions; one update only")
    intent_chart = args.output.with_name(args.output.stem + "-intents.png")
    fig.savefig(intent_chart, dpi=160)
    plt.close(fig)
    intent_source = {
        "run_kind": "smoke_only",
        "seed": manifest["seed"],
        "model_contract": manifest["model_contract"],
        "decisions": decisions,
        "intent_diagnostics": diagnostics,
        "note": "Opportunity counts and action choices are from one random-initialized PPO rollout; not a quality evaluation.",
    }
    intent_chart.with_suffix(".json").write_text(json.dumps(intent_source, indent=2), encoding="utf-8")
    print(f"Realm v15 smoke charts: {args.output}, {intent_chart}; decisions={decisions}")


if __name__ == "__main__":
    main()
