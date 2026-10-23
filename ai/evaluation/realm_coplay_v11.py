"""Sampled full-world co-play using one shared C++ Realm for all policy families."""
import argparse
import json
from pathlib import Path
import sys
import time

import numpy as np
import torch
import polyec_env as core

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "training"))
from realm_model_v11 import ACTIONS, FEATURES, HIDDEN, OBS, STATE, Policy
from realm_dev_baseline_v11 import baseline_action as dev_baseline_action
torch.set_num_threads(1)

FAMILIES = ("governance", "economic", "civilian")
FAMILY_ID = {name: index for index, name in enumerate(FAMILIES)}
INTENT_COUNT = 29
INTENT_TRANSFER_ESTABLISHMENT_GOODS = 26
INTENT_AUTHORIZE_WORK_AT_SITE = 27
INTENT_WORK_AT_SITE = 28
WORKSITE_ACTION_COUNT_VERSION = "realm-selected-intent-v1"
INTENT_NAMES = (
    "Hold", "Buy", "Sell", "Travel", "Borrow", "Repay", "Project", "Hire", "Tax", "Aid",
    "Investigate", "Offer", "Accept", "Reject", "Work", "Study", "Rest", "Social", "Join",
    "Migrate", "Support", "Appoint", "Mobilize", "Resign", "Marry", "CreateEstablishment",
    "TransferEstablishmentGoods", "AuthorizeWorkAtSite", "WorkAtSite",
)
GOOD_NAMES = (
    "DryFood", "SupplementaryFood", "Salt", "Timber", "Fuel", "Iron", "Copper", "Silver", "Gold",
    "Fiber", "Beeswax", "Cloth", "Silk", "Livestock", "Horse", "WarHorse", "Tools",
    "MilitarySupply", "Weapon", "ConstructionMaterial", "Paper", "Medicine", "Tea", "Incense",
    "DecorativeGlass", "FinePorcelain", "GeneralLuxury",
)


def baseline_action(family, obs, mask):
    return dev_baseline_action(family, obs, mask)


def sample_action(policy, obs, mask, hidden, rng):
    with torch.no_grad():
        logits, _, next_hidden = policy(torch.from_numpy(obs).unsqueeze(0), hidden)
    scores = logits.squeeze(0).numpy().astype(np.float64)
    scores[mask <= 0.5] = -1e9
    scores -= scores.max()
    probabilities = np.exp(scores)
    probabilities /= probabilities.sum()
    action = int(np.searchsorted(np.cumsum(probabilities), rng.random(), side="right"))
    action = min(action, ACTIONS - 1)
    if mask[action] <= 0.5:
        raise RuntimeError("categorical sampler selected an illegal action")
    return action, next_hidden


def selected_action_intent(observation, action):
    """Decode the selected candidate's C++ Intent from its matching observation."""
    observation = np.asarray(observation, dtype=np.float32)
    if observation.shape != (OBS,) or action < 0 or action >= ACTIONS:
        raise ValueError("selected-action Intent dimensions")
    raw = float(observation[STATE + action * FEATURES]) * 32.0
    intent = int(round(raw))
    if not np.isfinite(raw) or abs(raw - intent) > 1e-3 or not 0 <= intent < INTENT_COUNT:
        raise ValueError("selected-action Intent observation encoding drift")
    return intent


def selected_action_good(observation, action):
    """Decode the selected candidate's canonical GoodId parameter."""
    observation = np.asarray(observation, dtype=np.float32)
    if observation.shape != (OBS,) or action < 0 or action >= ACTIONS:
        raise ValueError("selected-action GoodId dimensions")
    raw = float(observation[STATE + action * FEATURES + 2]) * (len(GOOD_NAMES) - 1)
    good = int(round(raw))
    if not np.isfinite(raw) or abs(raw - good) > 1e-3 or not 0 <= good < len(GOOD_NAMES):
        raise ValueError(f"selected-action GoodId observation encoding drift: {raw}")
    return good


def load_policies(models_dir):
    current = core.Realm(1, 6, 128)
    expected_contract = current.model_contract
    policies = {"learned": {}, "initial": {}}
    for family in FAMILIES:
        family_dir = models_dir / family
        manifest = json.loads((family_dir / "manifest.json").read_text(encoding="utf-8"))
        if (manifest.get("family") != family or manifest.get("schema") != "realm-v11"
                or manifest.get("model_contract") != expected_contract
                or manifest.get("observation") != OBS or manifest.get("actions") != ACTIONS
                or manifest.get("hidden") != HIDDEN or manifest.get("trained_with_rl") is not True
                or manifest.get("imitation") is not False
                or manifest.get("config_fingerprint") != current.config_fingerprint
                or manifest.get("reward_version") != core.REALM_REWARD_VERSION):
            raise ValueError(f"incompatible v11 checkpoint manifest: {family}")
        if not isinstance(manifest.get("selected_seed"), int):
            raise ValueError(f"checkpoint manifest lacks selected_seed: {family}")
        selected = manifest["selected_seed"]
        for mode, filename in (("learned", f"seed-{selected}.pt"),
                               ("initial", f"initial-{selected}.pt")):
            model = Policy()
            model.load_state_dict(torch.load(family_dir / filename, map_location="cpu", weights_only=True))
            model.eval()
            policies[mode][FAMILY_ID[family]] = model
    return policies


def run_variant(seed, regions, days, controllers, policies, sampling_seed):
    started = time.perf_counter()
    world = core.Realm(seed, regions, 128)
    opening_money = world.view()["money"]
    draft = world.view()["cards"]
    if not world.choose(draft[seed % len(draft)]):
        raise RuntimeError("initial draft selection failed")
    representatives = {}
    opening_utility = {}
    for family in range(3):
        representative = next(
            (person_id for person_id in range(1, 129)
             if world.person(person_id)["alive"] and world.person(person_id)["family"] == family),
            None,
        )
        if representative is None:
            raise RuntimeError(f"no representative actor for {FAMILIES[family]}")
        representatives[family] = representative
        opening_utility[family] = world.utility(representative)

    rngs = {person_id: np.random.default_rng((sampling_seed << 32) ^ person_id)
            for person_id in range(1, 129)}
    hidden = {}
    hidden_family = {}
    hidden_mode = {}
    action_counts = np.zeros((3, ACTIONS), dtype=np.int64)
    intent_counts = np.zeros((3, INTENT_COUNT), dtype=np.int64)
    cash_delta_by_intent = np.zeros((3, INTENT_COUNT), dtype=np.int64)
    debt_delta_by_intent = np.zeros((3, INTENT_COUNT), dtype=np.int64)
    buy_counts_by_good = np.zeros((3, len(GOOD_NAMES)), dtype=np.int64)
    buy_cash_delta_by_good = np.zeros((3, len(GOOD_NAMES)), dtype=np.int64)
    establishment_counts_by_industry = np.zeros((3, 4), dtype=np.int64)
    worksite_actions = np.zeros(3, dtype=np.int64)
    work_authorizations = np.zeros(3, dtype=np.int64)
    work_submissions = np.zeros(3, dtype=np.int64)
    for _ in range(days):
        for family in range(3):
            for person_id in list(world.due(family)):
                person = world.person(person_id)
                active_family = person["family"]
                obs = np.asarray(world.observe(person_id), dtype=np.float32)
                mask = np.asarray(world.mask(person_id), dtype=np.float32)
                if mask.shape != (ACTIONS,) or not np.any(mask > 0.5):
                    raise RuntimeError("empty or malformed action mask")
                mode = controllers.get(active_family, "baseline")
                if mode in ("learned", "initial"):
                    if hidden_family.get(person_id) != active_family or hidden_mode.get(person_id) != mode:
                        hidden[person_id] = torch.zeros(1, HIDDEN)
                        hidden_family[person_id] = active_family
                        hidden_mode[person_id] = mode
                    action, next_hidden = sample_action(
                        policies[mode][active_family], obs, mask, hidden[person_id], rngs[person_id]
                    )
                    hidden[person_id] = next_hidden
                elif mode == "hold":
                    action = 0
                elif mode == "random":
                    action = int(rngs[person_id].choice(np.flatnonzero(mask > 0.5)))
                else:
                    action = baseline_action(active_family, obs, mask)
                action_counts[active_family, action] += 1
                intent = selected_action_intent(obs, action)
                intent_counts[active_family, intent] += 1
                selected_good = selected_action_good(obs, action) if intent in (1, 25) else None
                if intent == 1:
                    buy_counts_by_good[active_family, selected_good] += 1
                elif intent == 25 and 1 <= selected_good <= 3:
                    establishment_counts_by_industry[active_family, selected_good] += 1
                worksite_actions[active_family] += int(intent in (
                    INTENT_TRANSFER_ESTABLISHMENT_GOODS,
                    INTENT_AUTHORIZE_WORK_AT_SITE,
                    INTENT_WORK_AT_SITE,
                ))
                work_authorizations[active_family] += int(intent == INTENT_AUTHORIZE_WORK_AT_SITE)
                work_submissions[active_family] += int(intent == INTENT_WORK_AT_SITE)
                before_cash = world.person(person_id)["cash"]
                before_debt = world.debt(person_id)
                ok, reason = world.step(person_id, action)
                if not ok:
                    raise RuntimeError(f"Realm rejected legal sampled action: {reason}")
                cash_delta_by_intent[active_family, intent] += world.person(person_id)["cash"] - before_cash
                debt_delta_by_intent[active_family, intent] += world.debt(person_id) - before_debt
                if intent == 1:
                    buy_cash_delta_by_good[active_family, selected_good] += (
                        world.person(person_id)["cash"] - before_cash
                    )
        world.advance()
        if not world.invariant():
            raise RuntimeError("whole-world accounting invariant failed")

    utility = {}
    representative_outcomes = {}
    for family, person_id in representatives.items():
        initial = opening_utility[family]
        utility[FAMILIES[family]] = (world.utility(person_id) - initial) / max(1.0, abs(initial))
        person = world.person(person_id)
        loans = [loan for loan in world.loans() if loan["debtor"] == person_id]
        due = [loan for loan in loans if loan["due"] <= world.day]
        debt = world.debt(person_id)
        representative_outcomes[FAMILIES[family]] = {
            "alive": person["alive"], "health": person["health"],
            "nutrition": person["nutrition"], "cash": person["cash"],
            "debt": debt, "insolvent": person["cash"] < debt,
            "obligations_due": len(due),
            "defaults": sum(loan["defaulted"] for loan in due),
        }
    sites = world.establishments()
    return {
        "seed": seed,
        "controllers": {FAMILIES[index]: controllers.get(index, "baseline") for index in range(3)},
        "utility_by_representative": utility,
        "representative_outcomes": representative_outcomes,
        "money": world.view()["money"],
        "money_delta": world.view()["money"] - opening_money,
        "alive": sum(world.person(person_id)["alive"] for person_id in range(1, 129)),
        "worksite_actions_by_family": dict(zip(FAMILIES, worksite_actions.tolist())),
        "worksite_action_count_version": WORKSITE_ACTION_COUNT_VERSION,
        "work_authorizations_by_family": dict(zip(FAMILIES, work_authorizations.tolist())),
        "work_submissions_by_family": dict(zip(FAMILIES, work_submissions.tolist())),
        "action_counts_by_family": {family: action_counts[i].tolist() for i, family in enumerate(FAMILIES)},
        "selected_intent_counts_by_family": {
            family: dict(zip(INTENT_NAMES, intent_counts[i].tolist())) for i, family in enumerate(FAMILIES)
        },
        "immediate_cash_delta_by_intent_by_family": {
            family: dict(zip(INTENT_NAMES, cash_delta_by_intent[i].tolist())) for i, family in enumerate(FAMILIES)
        },
        "immediate_debt_delta_by_intent_by_family": {
            family: dict(zip(INTENT_NAMES, debt_delta_by_intent[i].tolist())) for i, family in enumerate(FAMILIES)
        },
        "buy_counts_by_good_by_family": {
            family: dict(zip(GOOD_NAMES, buy_counts_by_good[i].tolist())) for i, family in enumerate(FAMILIES)
        },
        "buy_cash_delta_by_good_by_family": {
            family: dict(zip(GOOD_NAMES, buy_cash_delta_by_good[i].tolist())) for i, family in enumerate(FAMILIES)
        },
        "establishment_counts_by_industry_by_family": {
            family: {"ClothWorkshop": int(establishment_counts_by_industry[i, 1]),
                     "ToolWorkshop": int(establishment_counts_by_industry[i, 2]),
                     "MilitarySupplyWorkshop": int(establishment_counts_by_industry[i, 3])}
            for i, family in enumerate(FAMILIES)
        },
        "establishment_count": len(sites),
        "cloth_stock": sum(site["stock"][11] for site in sites),
        "fiber_stock": sum(site["stock"][9] for site in sites),
        "work_units_paid_at_sites": sum(
            sum(site["paid_work_units"].values()) for site in sites
        ),
        "invariant": world.invariant(),
        "seconds": time.perf_counter() - started,
    }


def paired_utility_summary(baseline_runs, variant_runs):
    summary = {}
    t95 = {2: 12.706, 3: 4.303, 4: 3.182, 5: 2.776, 6: 2.571,
           7: 2.447, 8: 2.365, 9: 2.306, 10: 2.262}
    for family in FAMILIES:
        differences = np.asarray([
            run["utility_by_representative"][family] - base["utility_by_representative"][family]
            for base, run in zip(baseline_runs, variant_runs)
        ], dtype=np.float64)
        if len(differences) < 2:
            summary[family] = {"paired_mean": float(differences.mean()), "ci95": None,
                               "samples": len(differences)}
            continue
        standard_error = differences.std(ddof=1) / np.sqrt(len(differences))
        margin = t95.get(len(differences), 1.96) * standard_error
        summary[family] = {"paired_mean": float(differences.mean()),
                           "ci95": [float(differences.mean() - margin),
                                    float(differences.mean() + margin)],
                           "samples": len(differences)}
    return summary


def main():
    parser = argparse.ArgumentParser()
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--models-dir", type=Path)
    group.add_argument("--smoke-random-models", action="store_true")
    parser.add_argument("--seeds", type=int, nargs="+", default=[170000, 170099])
    parser.add_argument("--regions", type=int, default=6)
    parser.add_argument("--days", type=int, default=128)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    if core.REALM_SCHEMA != "realm-v11" or core.REALM_OBS != OBS or core.REALM_ACTIONS != ACTIONS:
        parser.error("v11 cross-play evaluator requires the current realm-v11 C++ binding")
    if args.regions < 1 or args.regions > 6 or args.days < 1 or args.days > 512:
        parser.error("regions must be 1..6 and days 1..512")
    if not args.seeds or len(args.seeds) != len(set(args.seeds)):
        parser.error("paired evaluation needs distinct seeds")
    if not args.smoke_random_models and len(args.seeds) < 2:
        parser.error("trained checkpoint evaluation needs at least two paired holdout seeds")
    if args.smoke_random_models:
        torch.manual_seed(20261021)
        policies = {mode: {family: Policy().eval() for family in range(3)}
                    for mode in ("learned", "initial")}
    else:
        policies = load_policies(args.models_dir)

    variants = {
        "baseline": {},
        "hold": {0: "hold", 1: "hold", 2: "hold"},
        "random_legal": {0: "random", 1: "random", 2: "random"},
        "initial_all_families": {0: "initial", 1: "initial", 2: "initial"},
        "all_learned": {0: "learned", 1: "learned", 2: "learned"},
    }
    variants.update({f"{family}_learned_vs_fixed_peers": {index: "learned"}
                     for index, family in enumerate(FAMILIES)})
    result = {
        "schema": core.REALM_SCHEMA,
        "reward_version": core.REALM_REWARD_VERSION,
        "observation": OBS,
        "actions": ACTIONS,
        "model_contract": core.Realm(1, args.regions, 128).model_contract,
        "regions": args.regions,
        "days": args.days,
        "seed_pairs": args.seeds,
        "policy_status": "plumbing_only_random_initialization" if args.smoke_random_models else "trained_checkpoint",
        "variants": {},
    }
    for name, controllers in variants.items():
        runs = [run_variant(seed, args.regions, args.days, controllers, policies, seed)
                for seed in args.seeds]
        result["variants"][name] = runs
    baseline_runs = result["variants"]["baseline"]
    result["paired_utility_vs_baseline"] = {
        name: paired_utility_summary(baseline_runs, runs)
        for name, runs in result["variants"].items() if name != "baseline"
    }
    if args.output:
        args.output.write_text(json.dumps(result, indent=2), encoding="utf-8")
    print(json.dumps({key: value for key, value in result.items() if key != "variants"}, sort_keys=True))
    for name, runs in result["variants"].items():
        print(json.dumps({"variant": name, "runs": len(runs),
                          "utility_by_representative": [run["utility_by_representative"] for run in runs],
                          "worksite_actions": [run["worksite_actions_by_family"] for run in runs],
                          "invariant": all(run["invariant"] for run in runs)}, sort_keys=True))
    for name, paired in result["paired_utility_vs_baseline"].items():
        print(json.dumps({"paired_vs_baseline": name, "summary": paired}, sort_keys=True))


if __name__ == "__main__":
    main()
