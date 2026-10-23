"""Run fixed-budget PPO v11 seeds sequentially and lock a comparable training set."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

from realm_coplay_env_v11 import FAMILY_NAMES


def file_hash(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--role", type=int, choices=range(3), required=True)
    parser.add_argument("--seeds", type=int, nargs="+", default=[1, 2, 3])
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--regions", type=int, default=6)
    parser.add_argument("--horizon", type=int)
    parser.add_argument("--envs", type=int, default=16)
    parser.add_argument("--rollout-steps", type=int, default=64)
    parser.add_argument("--minibatch-sequences", type=int, default=64)
    parser.add_argument("--updates", type=int, default=25)
    parser.add_argument("--seconds-per-seed", type=int, default=300)
    parser.add_argument("--epochs", type=int, default=3)
    parser.add_argument("--learning-rate", type=float, default=3e-4)
    parser.add_argument("--entropy-coef", type=float, default=0.01)
    parser.add_argument("--worksite-curriculum", choices=("none", "economic-worksite-reset-v1"), default="none")
    parser.add_argument("--peer-governance", type=Path)
    parser.add_argument("--peer-economic", type=Path)
    parser.add_argument("--peer-civilian", type=Path)
    args = parser.parse_args()
    if len(args.seeds) < 3 or len(set(args.seeds)) != len(args.seeds):
        parser.error("training set needs at least three distinct seeds")
    if args.output.exists() and any(args.output.iterdir()):
        parser.error("training-set output directory must be empty")
    family = FAMILY_NAMES[args.role]
    args.output.mkdir(parents=True, exist_ok=True)
    trainer = Path(__file__).with_name("realm_train_v11.py")
    results = []
    peers = {
        "governance": args.peer_governance,
        "economic": args.peer_economic,
        "civilian": args.peer_civilian,
    }
    for seed in args.seeds:
        run_dir = args.output / family / f"seed-{seed}"
        command = [sys.executable, str(trainer), "--role", str(args.role),
                   "--seed", str(seed), "--output", str(run_dir),
                   "--regions", str(args.regions), "--envs", str(args.envs),
                   "--rollout-steps", str(args.rollout_steps), "--updates", str(args.updates),
                   "--minibatch-sequences", str(args.minibatch_sequences),
                   "--seconds", str(args.seconds_per_seed), "--epochs", str(args.epochs),
                   "--learning-rate", str(args.learning_rate), "--entropy-coef", str(args.entropy_coef),
                   "--worksite-curriculum", args.worksite_curriculum]
        if args.horizon is not None:
            command.extend(("--horizon", str(args.horizon)))
        option_names = {"governance": "--peer-governance", "economic": "--peer-economic",
                        "civilian": "--peer-civilian"}
        for peer_family, path in peers.items():
            if path is not None and peer_family != family:
                command.extend((option_names[peer_family], str(path)))
        print(json.dumps({"seed": seed, "family": family, "command": command}), flush=True)
        code = subprocess.run(command, check=False).returncode
        if code != 0:
            failure = {"complete": False, "family": family, "completed_runs": results,
                       "failed_seed": seed, "exit_code": code}
            (args.output / "INCOMPLETE.json").write_text(json.dumps(failure, indent=2), encoding="utf-8")
            return code
        manifest_path = run_dir / "manifest.json"
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        if not manifest.get("training_complete") or not manifest.get("complete"):
            (args.output / "INCOMPLETE.json").write_text(json.dumps({
                "complete": False, "family": family, "completed_runs": results,
                "failed_seed": seed, "reason": "trainer output did not complete its fixed budget",
            }, indent=2), encoding="utf-8")
            return 2
        results.append({"seed": seed, "path": str(run_dir),
                        "manifest_sha256": file_hash(manifest_path),
                        "policy_sha256": file_hash(run_dir / "policy.pt"),
                        "decisions": manifest["decisions"],
                        "updates": manifest["completed_updates"]})

    decisions = {item["decisions"] for item in results}
    updates = {item["updates"] for item in results}
    if len(decisions) != 1 or len(updates) != 1:
        (args.output / "INCOMPLETE.json").write_text(json.dumps({
            "complete": False, "family": family, "runs": results,
            "reason": "training seeds did not receive equal decision/update budgets",
        }, indent=2), encoding="utf-8")
        return 2
    lock = {"family": family, "seeds": args.seeds, "runs": results,
            "minibatch_sequences": args.minibatch_sequences,
            "worksite_curriculum": args.worksite_curriculum,
            "fixed_decisions_per_seed": results[0]["decisions"],
            "updates_per_seed": results[0]["updates"],
            "all_budgets_equal": True, "peer_eligible": False}
    (args.output / "TRAINING-SET.json").write_text(json.dumps(lock, indent=2), encoding="utf-8")
    print(json.dumps({"family": family, "training_seeds": args.seeds,
                      "decisions_per_seed": results[0]["decisions"],
                      "updates_per_seed": results[0]["updates"],
                      "peer_eligible": False}, sort_keys=True), flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
