"""Trace immediate Economic actor cash/debt deltas in sampled shared-core worlds."""
import argparse
import hashlib
import json
from pathlib import Path

import polyec_env as core

from realm_coplay_v11 import FAMILIES, FAMILY_ID, GOOD_NAMES, INTENT_NAMES, run_variant
from realm_policy_holdout_v11 import load_run


def sha256(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--run", type=Path, action="append", required=True)
    parser.add_argument("--holdout-seeds", type=int, nargs="+", required=True)
    parser.add_argument("--regions", type=int, default=6)
    parser.add_argument("--days", type=int, default=128)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    family = "economic"
    if len(args.run) != 3 or len(args.holdout_seeds) < 20:
        parser.error("audit requires three Economic checkpoints and at least 20 holdout worlds")
    if len(set(args.holdout_seeds)) != len(args.holdout_seeds):
        parser.error("holdout seeds must be distinct")
    if args.output.exists() and any(args.output.iterdir()):
        parser.error("output directory must be empty")
    family_id = FAMILY_ID[family]
    worlds = []
    run_summaries = []
    for run_path in args.run:
        seed, learned, initial, manifest = load_run(run_path, family)
        policies = {"learned": {family_id: learned}, "initial": {family_id: initial}}
        controllers = {family_id: "learned"}
        for world_seed in args.holdout_seeds:
            result = run_variant(world_seed, args.regions, args.days, controllers, policies, world_seed)
            if not result["invariant"]:
                raise RuntimeError(f"invariant failed for training seed {seed}, world {world_seed}")
            counts = result["selected_intent_counts_by_family"][family]
            if sum(counts.values()) != sum(result["action_counts_by_family"][family]):
                raise RuntimeError("semantic Intent counts do not match selected action count")
            worlds.append({"training_seed": seed, "world": result})
        run_summaries.append({
            "training_seed": seed,
            "policy_sha256": sha256(run_path / "policy.pt"),
            "decisions": manifest.get("decisions"),
            "completed_updates": manifest.get("completed_updates"),
        })
    totals = {name: {"selected": 0, "immediate_cash_delta": 0, "immediate_debt_delta": 0}
              for name in INTENT_NAMES}
    buy_goods = {name: {"selected": 0, "immediate_cash_delta": 0} for name in GOOD_NAMES}
    establishment_industries = {name: 0 for name in ("ClothWorkshop", "ToolWorkshop", "MilitarySupplyWorkshop")}
    for item in worlds:
        result = item["world"]
        for name in INTENT_NAMES:
            totals[name]["selected"] += result["selected_intent_counts_by_family"][family][name]
            totals[name]["immediate_cash_delta"] += result["immediate_cash_delta_by_intent_by_family"][family][name]
            totals[name]["immediate_debt_delta"] += result["immediate_debt_delta_by_intent_by_family"][family][name]
        for name in GOOD_NAMES:
            buy_goods[name]["selected"] += result["buy_counts_by_good_by_family"][family][name]
            buy_goods[name]["immediate_cash_delta"] += result["buy_cash_delta_by_good_by_family"][family][name]
        for name in establishment_industries:
            establishment_industries[name] += result["establishment_counts_by_industry_by_family"][family][name]
    audit = {
        "schema": "realm-economic-flow-audit-v1",
        "realm_schema": core.REALM_SCHEMA,
        "model_contract": core.Realm(1, args.regions, 128).model_contract,
        "reward_version": core.REALM_REWARD_VERSION,
        "worksite_action_count_version": "realm-selected-intent-v1",
        "family": family,
        "regions": args.regions,
        "days": args.days,
        "holdout_seeds": args.holdout_seeds,
        "runs": run_summaries,
        "intent_totals": totals,
        "buy_goods": buy_goods,
        "establishment_industries": establishment_industries,
        "worlds": worlds,
        "diagnostic_limits": [
            "Cash and debt deltas are measured immediately around the selected C++ command.",
            "Daily living expenses, interest accrual, payroll, and production between decisions are not attributed to an intent.",
            "This audit does not select or promote a checkpoint."
        ],
    }
    args.output.mkdir(parents=True, exist_ok=True)
    output_path = args.output / "audit.json"
    output_path.write_text(json.dumps(audit, indent=2), encoding="utf-8")
    summary = {
        "output": str(output_path),
        "world_runs": len(worlds),
        "all_invariants": all(item["world"]["invariant"] for item in worlds),
        "top_intents": sorted(
            ({"intent": name, **values} for name, values in totals.items()),
            key=lambda item: item["selected"], reverse=True,
        )[:10],
        "audit_sha256": sha256(output_path),
    }
    print(json.dumps(summary, sort_keys=True))


if __name__ == "__main__":
    main()
