"""Plot reward and PPO value-target summaries from one explicitly test-only run."""
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
    if manifest.get("run_kind") != "smoke_only" or manifest.get("release_eligible") is not False:
        parser.error("plot accepts only a v14 adapter smoke run")
    rows = [json.loads(line) for line in (args.run / "history.jsonl").read_text(encoding="utf-8").splitlines()
            if line.strip()]
    if len(rows) != 1 or rows[0].get("update") != 1:
        parser.error("expected exactly one completed smoke update")
    row = rows[0]
    decisions = row["decisions"]
    fig, axes = plt.subplots(1, 2, figsize=(10, 4), constrained_layout=True)
    axes[0].plot([decisions], [row["reward_mean"]], marker="o", color="#2364aa")
    axes[0].set_title("Mean C++ transition reward")
    axes[0].set_xlabel("Policy decisions")
    axes[0].set_ylabel("Mean reward per sampled decision")
    axes[0].grid(True, alpha=0.25)
    axes[1].plot([decisions], [row["return_distribution"]["mean"]], marker="o", color="#d1495b")
    axes[1].set_title("Mean PPO return target")
    axes[1].set_xlabel("Policy decisions")
    axes[1].set_ylabel("Mean critic target in rollout")
    axes[1].grid(True, alpha=0.25)
    fig.suptitle("Realm v14 adapter smoke — one update; not a policy evaluation")
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
        "decisions": decisions,
        "update": row,
        "note": "reward and PPO return target are rollout diagnostics from one smoke update; not independent policy evaluation",
    }
    args.output.with_suffix(".json").write_text(json.dumps(source, indent=2), encoding="utf-8")
    print(f"V14 SMOKE CHART: {args.output}; data={args.output.with_suffix('.json')}; decisions={decisions}")


if __name__ == "__main__":
    main()
