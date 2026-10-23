"""Resumable recurrent PPO for Realm v14 with explicitly frozen peer policies."""
import argparse
import copy
import hashlib
import json
import os
from pathlib import Path
import random
import time

import numpy as np
import psutil
import torch
import polyec_env as core

from realm_model_v14 import ACTIONS, FEATURES, HIDDEN, OBS, STATE, Policy, masked_distribution
from realm_coplay_env_v14 import (
    FAMILY_NAMES, TIMELINE_CONTRACT_FIXTURE_VERSION, RealmCoPlayBatchV14,
    activate_timeline_contract,
)
from realm_dev_baseline_v14 import BASELINE_VERSION

torch.set_num_threads(2)
torch.set_num_interop_threads(1)

INTENT_NAMES = (
    "Hold", "Buy", "Sell", "Travel", "Borrow", "Repay", "Project", "Hire",
    "Tax", "Aid", "Investigate", "Offer", "Accept", "Reject", "Work", "Study",
    "Rest", "Social", "Join", "Migrate", "Support", "Appoint", "Mobilize",
    "Resign", "Marry", "CreateEstablishment", "TransferEstablishmentGoods",
    "AuthorizeWorkAtSite", "WorkAtSite", "BuyMilitaryRations", "ShipMilitaryRations", "FundMunicipal", "ForeignTrade",
)
ACTION_DIAGNOSTICS_VERSION = "realm-intent-coverage-v2"


def summarize_action_intents(observations, masks, actions, rewards, advantages):
    """Summarize legal opportunities and learning signals by selected Intent."""
    observations = np.asarray(observations, dtype=np.float32)
    masks = np.asarray(masks, dtype=np.float32)
    actions = np.asarray(actions, dtype=np.int64).reshape(-1)
    rewards = np.asarray(rewards, dtype=np.float64).reshape(-1)
    advantages = np.asarray(advantages, dtype=np.float64).reshape(-1)
    if observations.ndim != 2 or observations.shape[1] != OBS:
        raise ValueError("action diagnostic observation shape")
    if masks.shape != (len(actions), ACTIONS) or len(observations) != len(actions):
        raise ValueError("action diagnostic batch shape")
    if len(rewards) != len(actions) or len(advantages) != len(actions):
        raise ValueError("action diagnostic signal shape")
    if np.any(actions < 0) or np.any(actions >= ACTIONS) or not np.isfinite(observations).all():
        raise ValueError("action diagnostic action or observation value")
    candidate_features = observations[:, STATE:].reshape(-1, ACTIONS, FEATURES)
    raw_intents = candidate_features[:, :, 0] * 32.0
    intent_ids = np.rint(raw_intents).astype(np.int64)
    if (np.any(np.abs(raw_intents - intent_ids) > 1e-3) or np.any(intent_ids < 0)
            or np.any(intent_ids >= len(INTENT_NAMES))):
        raise ValueError("action diagnostic Intent observation encoding drift")
    legal = masks > 0.5
    if np.any(~legal[np.arange(len(actions)), actions]):
        raise ValueError("action diagnostic selected action was not legal")
    selected_intents = intent_ids[np.arange(len(actions)), actions]
    result = {}
    for intent, name in enumerate(INTENT_NAMES):
        selected = selected_intents == intent
        decisions = int(selected.sum())
        result[name] = {
            "legal_candidates": int((legal & (intent_ids == intent)).sum()),
            "decisions": decisions,
            "reward_mean": float(rewards[selected].mean()) if decisions else None,
            "advantage_mean": float(advantages[selected].mean()) if decisions else None,
            "positive_advantage_fraction": float((advantages[selected] > 0).mean()) if decisions else None,
        }
    if sum(item["decisions"] for item in result.values()) != len(actions):
        raise RuntimeError("action diagnostic selected decision totals do not match")
    return result


def sequence_forward(policy, observations, hidden, resets):
    logits, values = [], []
    for step in range(observations.shape[1]):
        hidden = hidden * (~resets[:, step]).float().unsqueeze(-1)
        action_logits, value, hidden = policy(observations[:, step], hidden)
        logits.append(action_logits)
        values.append(value)
    return torch.stack(logits, 1), torch.stack(values, 1)


def load_peer(path, family, regions):
    if path is None:
        return None, None
    path = Path(path)
    manifest = json.loads((path / "manifest.json").read_text(encoding="utf-8"))
    contract_world = activate_timeline_contract(core.Realm(1, regions, 128))
    expected = {
        "family": FAMILY_NAMES[family], "schema": core.REALM_SCHEMA,
        "observation": OBS, "actions": ACTIONS, "hidden": HIDDEN,
        "model_contract": contract_world.model_contract,
        "config_fingerprint": contract_world.config_fingerprint,
        "reward_version": core.REALM_REWARD_VERSION,
        "action_diagnostics_version": ACTION_DIAGNOSTICS_VERSION,
    }
    if any(manifest.get(key) != value for key, value in expected.items()):
        raise ValueError(f"peer bundle does not match family {FAMILY_NAMES[family]} and Realm v14 contract")
    if manifest.get("trained_with_rl") is not True or manifest.get("peer_eligible") is not True:
        raise ValueError("peer bundle has not passed the learned-policy selection/evaluation gates")
    policy = Policy()
    policy.load_state_dict(torch.load(path / "policy.pt", map_location="cpu", weights_only=True))
    return policy.eval(), expected


def checkpoint_write(path, data):
    temporary = path.with_suffix(path.suffix + ".tmp")
    torch.save(data, temporary)
    os.replace(temporary, path)


def train(args):
    role = args.role
    family = FAMILY_NAMES[role]
    torch.manual_seed(args.seed)
    np.random.seed(args.seed)
    random.seed(args.seed)
    contract_world = activate_timeline_contract(core.Realm(1, args.regions, 128))
    if core.REALM_SCHEMA != "realm-v29" or core.REALM_OBS != OBS or core.REALM_ACTIONS != ACTIONS:
        raise RuntimeError("loaded C++ binding dimensions do not match the Realm v14 model interface")
    if args.smoke_only and (args.updates != 1 or args.resume):
        raise ValueError("a smoke run must be one fresh update and cannot resume a checkpoint")
    if args.envs < 16 or args.envs % 16:
        raise ValueError("--envs must be a multiple of 16 for complete PPO minibatches")
    if args.envs * args.rollout_steps // 16 % 64:
        raise ValueError("envs * rollout_steps / 16 must fill complete 64-sequence minibatches")
    sequence_count = args.envs * (args.rollout_steps // 16)
    if (args.minibatch_sequences < 1 or args.minibatch_sequences > sequence_count
            or sequence_count % args.minibatch_sequences):
        raise ValueError("minibatch-sequences must divide the rollout recurrent-sequence count")
    if args.updates < 1 or (not args.no_time_limit and args.seconds < 1):
        raise ValueError("updates must be positive; seconds must be positive when the time limit is enabled")
    if not args.smoke_only:
        try:
            import onnx  # noqa: F401
        except ImportError as exc:
            raise RuntimeError(f"ONNX dependency must be available before rollout: {exc}") from exc

    output = Path(args.output)
    if output.exists() and any(output.iterdir()) and not args.resume:
        raise ValueError("output directory must be empty unless resuming its checkpoint")
    output.mkdir(parents=True, exist_ok=True)
    peer_paths = [args.peer_governance, args.peer_economic, args.peer_civilian]
    peers, peer_metadata = {}, {}
    peer_labels, peer_hashes = {}, {}
    for peer_family, path in enumerate(peer_paths):
        if peer_family == role:
            continue
        policy, metadata = load_peer(path, peer_family, args.regions)
        if policy is not None:
            peers[peer_family] = policy
            peer_metadata[peer_family] = metadata
            peer_labels[FAMILY_NAMES[peer_family]] = str(path)
            peer_hashes[FAMILY_NAMES[peer_family]] = {
                name: hashlib.sha256((path / name).read_bytes()).hexdigest()
                for name in ("manifest.json", "policy.pt")
            }
        else:
            peer_labels[FAMILY_NAMES[peer_family]] = "explicit dev baseline (no checkpoint supplied)"

    base_seed = 2**32 + args.seed * 10**7
    env = RealmCoPlayBatchV14(args.envs, base_seed, role, args.regions, args.horizon,
                           partner_policies=peers, partner_metadata=peer_metadata,
                           worksite_curriculum=args.worksite_curriculum)
    obs, masks = env.observe()
    hidden = torch.zeros(args.envs, HIDDEN)
    policy = Policy()
    optimizer = torch.optim.Adam(policy.parameters(), lr=args.learning_rate, eps=1e-5)
    rng = np.random.default_rng(args.seed)
    start_update = 0
    identity = {
        "role": role, "regions": args.regions, "envs": args.envs,
        "horizon": args.horizon, "seed": args.seed,
        "schema": core.REALM_SCHEMA, "model_contract": contract_world.model_contract,
        "config_fingerprint": contract_world.config_fingerprint,
        "timeline_contract_fixture": TIMELINE_CONTRACT_FIXTURE_VERSION,
        "reward_version": core.REALM_REWARD_VERSION,
        "action_diagnostics_version": ACTION_DIAGNOSTICS_VERSION,
        "worksite_curriculum": args.worksite_curriculum,
        "rollout_steps": args.rollout_steps, "epochs": args.epochs,
        "minibatch_sequences": args.minibatch_sequences,
        "learning_rate": args.learning_rate, "entropy_coef": args.entropy_coef,
        "peer_bundle_sha256": peer_hashes, "baseline_version": BASELINE_VERSION,
    }
    if args.resume:
        saved = torch.load(args.resume, map_location="cpu", weights_only=False)
        if saved["identity"] != identity:
            raise ValueError("resume checkpoint training contract mismatch")
        policy.load_state_dict(saved["policy"])
        optimizer.load_state_dict(saved["optimizer"])
        env.load_state_dict(saved["environment"])
        hidden = saved["focal_hidden"]
        rng.bit_generator.state = saved["numpy_rng"]
        torch.set_rng_state(saved["torch_rng"])
        random.setstate(saved["python_rng"])
        start_update = int(saved["update"])
        obs, masks = env.observe()
    else:
        torch.save(policy.state_dict(), output / "initial.pt")

    manifest = {
        "family": family, "schema": core.REALM_SCHEMA, "observation": OBS,
        "actions": ACTIONS, "hidden": HIDDEN, "model_contract": contract_world.model_contract,
        "config_fingerprint": contract_world.config_fingerprint,
        "imitation": False,
        "complete": False, "peer_eligible": False, "peer_policies": peer_labels,
        "run_kind": "smoke_only" if args.smoke_only else "training_candidate",
        "trained_with_rl": True,
        "smoke_only": args.smoke_only,
        "policy_candidate": not args.smoke_only,
        "policy_selection_eligible": False,
        "release_eligible": False,
        "peer_fallback_label": "dev-only observation baseline",
        "peer_fallback_version": BASELINE_VERSION,
        "reward_version": core.REALM_REWARD_VERSION,
        "action_diagnostics_version": ACTION_DIAGNOSTICS_VERSION,
        "worksite_curriculum": args.worksite_curriculum,
        "timeline_contract_fixture": TIMELINE_CONTRACT_FIXTURE_VERSION,
        "reward": "normalized focal utility delta including net assets of person-owned establishments",
        "regions": args.regions, "horizon": args.horizon,
        "algorithm": "recurrent PPO; categorical masked actions", "gamma": 1.0,
        "gae_lambda": 0.95, "clip": 0.2, "epochs": args.epochs,
        "learning_rate": args.learning_rate, "entropy_coef": args.entropy_coef,
        "rollout_steps": args.rollout_steps, "bptt_steps": 16,
        "minibatch_sequences": args.minibatch_sequences,
        "envs": args.envs, "decisions_per_update": args.envs * args.rollout_steps,
        "updates_required": args.updates, "seed": args.seed,
        "wall_clock_limit_enabled": not args.no_time_limit,
        "checkpoint_resume": ["policy", "optimizer", "Torch RNG", "NumPy RNG", "Python RNG",
                              "C++ Realm worlds", "focal hidden state", "peer hidden state and RNG"],
        "torch_version": torch.__version__, "device": "cpu",
    }
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    started = time.perf_counter()
    for update in range(start_update, args.updates):
        policy.train()
        obs_hist, mask_hist, hidden_hist, scenario_hist = [], [], [], []
        action_hist, logp_hist, value_hist, reward_hist, done_hist = [], [], [], [], []
        peer_start = env.peer_decisions
        rollout_start = time.perf_counter()
        with torch.no_grad():
            for _ in range(args.rollout_steps):
                observation = torch.from_numpy(obs)
                mask = torch.from_numpy(masks)
                hidden_hist.append(hidden)
                logits, value, next_hidden = policy(observation, hidden)
                distribution = masked_distribution(logits, mask)
                action = distribution.sample()
                log_probability = distribution.log_prob(action)
                obs_hist.append(observation)
                mask_hist.append(mask)
                scenario_hist.append(tuple(env.scenarios))
                action_hist.append(action)
                logp_hist.append(log_probability)
                value_hist.append(value)
                obs, masks, reward, done, _ = env.step(action.numpy())
                reward_hist.append(torch.from_numpy(reward))
                done_hist.append(torch.from_numpy(done))
                hidden = next_hidden * (~torch.from_numpy(done)).float().unsqueeze(-1)
            _, bootstrap, _ = policy(torch.from_numpy(obs), hidden)
        rollout_seconds = time.perf_counter() - rollout_start
        rewards = torch.stack(reward_hist)
        values = torch.stack(value_hist)
        dones = torch.stack(done_hist)
        flat_actions = torch.stack(action_hist).reshape(-1).numpy()
        flat_masks = torch.stack(mask_hist).reshape(-1, ACTIONS).numpy()
        flat_observations = torch.stack(obs_hist).reshape(-1, OBS).numpy()
        flat_rewards = rewards.detach().cpu().numpy().reshape(-1)
        flat_scenarios = np.asarray(scenario_hist, dtype=object).reshape(-1)
        legal_counts = (flat_masks > 0.5).sum(axis=1)
        action_by_legal_count = {}
        for label, selected in {
            "one": legal_counts == 1,
            "two_to_four": (legal_counts >= 2) & (legal_counts <= 4),
            "five_to_eight": (legal_counts >= 5) & (legal_counts <= 8),
            "nine_or_more": legal_counts >= 9,
        }.items():
            action_by_legal_count[label] = {
                "decisions": int(selected.sum()),
                "action_counts": np.bincount(flat_actions[selected], minlength=ACTIONS).tolist(),
            }
        advantages = torch.zeros_like(rewards)
        last = torch.zeros(args.envs)
        for t in reversed(range(args.rollout_steps)):
            next_value = bootstrap if t == args.rollout_steps - 1 else values[t + 1]
            live = (~dones[t]).float()
            delta = rewards[t] + next_value * live - values[t]
            last = delta + 0.95 * live * last
            advantages[t] = last
        returns = advantages + values
        flat_advantages = advantages.detach().cpu().numpy().reshape(-1)
        action_intent_diagnostics = summarize_action_intents(
            flat_observations, flat_masks, flat_actions, flat_rewards, flat_advantages
        )
        action_intent_by_scenario = {
            scenario: summarize_action_intents(
                flat_observations[flat_scenarios == scenario],
                flat_masks[flat_scenarios == scenario],
                flat_actions[flat_scenarios == scenario],
                flat_rewards[flat_scenarios == scenario],
                flat_advantages[flat_scenarios == scenario],
            )
            for scenario in sorted(set(flat_scenarios.tolist()))
        }
        explained_variance = 1.0 - float((returns - values).var(unbiased=False) /
                                         (returns.var(unbiased=False) + 1e-8))
        normalized_advantages = (advantages - advantages.mean()) / (advantages.std() + 1e-8)

        def chunks(tensor):
            trailing = tensor.shape[2:]
            return tensor.transpose(0, 1).reshape(args.envs * (args.rollout_steps // 16), 16, *trailing)

        obs_chunks = chunks(torch.stack(obs_hist))
        masks_chunks = chunks(torch.stack(mask_hist))
        actions_chunks = chunks(torch.stack(action_hist))
        logp_chunks = chunks(torch.stack(logp_hist))
        adv_chunks = chunks(normalized_advantages)
        return_chunks = chunks(returns)
        initial_hidden = torch.stack(hidden_hist)[::16].transpose(0, 1).reshape(-1, HIDDEN)
        reset_history = torch.cat((torch.zeros_like(dones[:1]), dones[:-1]))
        reset_chunks = chunks(reset_history)
        losses, policy_losses, value_losses = [], [], []
        entropies, kl_values, clips, ratios, gradient_norms = [], [], [], [], []
        update_start = time.perf_counter()
        for _ in range(args.epochs):
            for indices in rng.permutation(len(obs_chunks)).reshape(-1, args.minibatch_sequences):
                logits, value = sequence_forward(policy, obs_chunks[indices], initial_hidden[indices], reset_chunks[indices])
                distribution = masked_distribution(logits, masks_chunks[indices])
                log_ratio = distribution.log_prob(actions_chunks[indices]) - logp_chunks[indices]
                ratio = log_ratio.exp()
                advantage = adv_chunks[indices]
                policy_loss = -torch.minimum(ratio * advantage,
                                             ratio.clamp(0.8, 1.2) * advantage).mean()
                value_loss = (value - return_chunks[indices]).square().mean()
                entropy = distribution.entropy().mean()
                loss = policy_loss + 0.5 * value_loss - args.entropy_coef * entropy
                optimizer.zero_grad(set_to_none=True)
                loss.backward()
                gradient_norm = torch.nn.utils.clip_grad_norm_(policy.parameters(), 0.5)
                optimizer.step()
                with torch.no_grad():
                    updated_logits, _ = sequence_forward(
                        policy, obs_chunks[indices], initial_hidden[indices], reset_chunks[indices]
                    )
                    updated_distribution = masked_distribution(updated_logits, masks_chunks[indices])
                    updated_log_ratio = updated_distribution.log_prob(actions_chunks[indices]) - logp_chunks[indices]
                    updated_ratio = updated_log_ratio.exp()
                losses.append(float(loss.detach()))
                policy_losses.append(float(policy_loss.detach()))
                value_losses.append(float(value_loss.detach()))
                entropies.append(float(entropy.detach()))
                kl_values.append(float(((updated_ratio - 1.0) - updated_log_ratio).mean()))
                clips.append(float(((updated_ratio - 1.0).abs() > 0.2).float().mean()))
                ratios.append(float(updated_ratio.mean()))
                gradient_norms.append(float(gradient_norm.detach()))
        optimizer_seconds = time.perf_counter() - update_start
        update_seconds = time.perf_counter() - rollout_start
        current_update = update + 1

        def distribution_summary(values):
            values = np.asarray(values, dtype=np.float64)
            return {
                "mean": float(values.mean()), "std": float(values.std()),
                "min": float(values.min()), "p05": float(np.quantile(values, 0.05)),
                "median": float(np.quantile(values, 0.5)),
                "p95": float(np.quantile(values, 0.95)), "max": float(values.max()),
            }

        flat_normalized_advantages = normalized_advantages.detach().cpu().numpy().reshape(-1)
        flat_returns = returns.detach().cpu().numpy().reshape(-1)
        snapshot = {
            "identity": identity, "update": current_update,
            "policy": policy.state_dict(), "optimizer": optimizer.state_dict(),
            "environment": env.state_dict(), "focal_hidden": hidden,
            "numpy_rng": rng.bit_generator.state, "torch_rng": torch.get_rng_state(),
            "python_rng": random.getstate(),
        }
        checkpoint_write(output / "checkpoint.pt", snapshot)
        row = {
            "update": current_update, "decisions": current_update * args.envs * args.rollout_steps,
            "rollout_seconds": rollout_seconds, "optimizer_seconds": optimizer_seconds,
            "update_seconds": update_seconds, "decisions_per_second": args.envs * args.rollout_steps / update_seconds,
            "peer_decisions": env.peer_decisions - peer_start,
            "loss": float(np.mean(losses)), "entropy": float(np.mean(entropies)),
            "policy_loss": float(np.mean(policy_losses)), "value_loss": float(np.mean(value_losses)),
            "approx_kl": float(np.mean(kl_values)), "clip_fraction": float(np.mean(clips)),
            "ratio_mean": float(np.mean(ratios)), "gradient_norm_mean": float(np.mean(gradient_norms)),
            "critic_explained_variance": explained_variance,
            "reward_mean": float(rewards.mean()), "reward_std": float(rewards.std(unbiased=False)),
            "reward_distribution": distribution_summary(rewards.detach().cpu().numpy().reshape(-1)),
            "advantage_distribution": distribution_summary(flat_advantages),
            "normalized_advantage_distribution": distribution_summary(flat_normalized_advantages),
            "return_distribution": distribution_summary(flat_returns),
            "action_counts": np.bincount(flat_actions, minlength=ACTIONS).tolist(),
            "action_counts_by_legal_choice_count": action_by_legal_count,
            "action_intent_diagnostics": action_intent_diagnostics,
            "action_intent_diagnostics_by_scenario": action_intent_by_scenario,
            "peak_rss_bytes": int(psutil.Process().memory_info().peak_wset),
        }
        with (output / "history.jsonl").open("a", encoding="utf-8") as history:
            history.write(json.dumps(row) + "\n")
        concise = {key: value for key, value in row.items()
                   if key not in ("action_counts", "action_counts_by_legal_choice_count")}
        print(json.dumps(concise), flush=True)
        if (not args.no_time_limit and time.perf_counter() - started >= args.seconds
                and current_update < args.updates):
            (output / "INCOMPLETE.json").write_text(json.dumps({
                "complete": False, "update": current_update, "required_updates": args.updates,
                "required_decisions": args.updates * args.envs * args.rollout_steps,
                "checkpoint": str(output / "checkpoint.pt"), "reason": "wall_clock_limit",
            }, indent=2), encoding="utf-8")
            return 2

    final_state = torch.load(output / "checkpoint.pt", map_location="cpu", weights_only=False)
    if args.smoke_only:
        manifest["smoke_update_complete"] = True
        manifest["completed_updates"] = args.updates
        manifest["decisions"] = args.updates * args.envs * args.rollout_steps
        manifest["note"] = "one-update adapter smoke; not a training candidate or runtime model"
    else:
        torch.save(final_state["policy"], output / "policy.pt")
        manifest["complete"] = True
        manifest["training_complete"] = True
        manifest["completed_updates"] = args.updates
        manifest["decisions"] = args.updates * args.envs * args.rollout_steps
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    (output / "INCOMPLETE.json").unlink(missing_ok=True)
    return 0


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--role", type=int, choices=range(3), required=True)
    parser.add_argument("--seed", type=int, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--resume", type=Path)
    parser.add_argument("--regions", type=int, choices=(1, 2, 6, 26), default=26,
                        help="26 selects all 27 Atlas territory IDs (0 through 26)")
    parser.add_argument("--horizon", type=int)
    parser.add_argument("--envs", type=int, default=16)
    parser.add_argument("--rollout-steps", type=int, default=64)
    parser.add_argument("--minibatch-sequences", type=int, default=64)
    parser.add_argument("--updates", type=int, default=1)
    parser.add_argument("--seconds", type=int, default=300)
    parser.add_argument("--no-time-limit", action="store_true",
                        help="continue until the requested update count without a wall-clock cutoff")
    parser.add_argument("--smoke-only", action="store_true",
                        help="run one C++-backed PPO update; output is never a training candidate")
    parser.add_argument("--epochs", type=int, default=3)
    parser.add_argument("--learning-rate", type=float, default=3e-4)
    parser.add_argument("--entropy-coef", type=float, default=0.01)
    parser.add_argument("--worksite-curriculum", choices=("none", "economic-worksite-reset-v1"), default="none")
    parser.add_argument("--peer-governance", type=Path)
    parser.add_argument("--peer-economic", type=Path)
    parser.add_argument("--peer-civilian", type=Path)
    args = parser.parse_args()
    if args.horizon is None:
        args.horizon = 512 if args.role == 1 else 128
    if args.rollout_steps < 16 or args.rollout_steps % 16:
        parser.error("--rollout-steps must be a multiple of 16")
    raise SystemExit(train(args))


if __name__ == "__main__":
    main()

