"""Focal PPO adapter with day-by-day frozen peers on the shared C++ Realm."""
from copy import deepcopy

import numpy as np
import torch
import polyec_env as core

from realm_model_v13 import ACTIONS, FEATURES, HIDDEN, OBS, STATE, Policy
from realm_dev_baseline_v13 import BASELINE_VERSION, baseline_action

FAMILY_NAMES = ("governance", "economic", "civilian")
WORKSITE_CURRICULUM_VERSION = "economic-worksite-reset-v1"
TIMELINE_CONTRACT_FIXTURE_VERSION = "one-tick-generic-schedule-v1"


def activate_timeline_contract(world):
    """Enable current v33 model features through a deterministic, expiring fixture."""
    ok, reason, _ = world.schedule_command(
        0, 1, world.person(1)["decisions"] + 1, 0,
        world.game_tick, world.game_tick + 1, 0, 5,
    )
    if not ok:
        raise RuntimeError(f"timeline contract fixture failed: {reason}")
    if world.model_contract != "realm-worksite-payroll-goods27-config-v33":
        raise RuntimeError(f"timeline fixture produced unexpected contract: {world.model_contract}")
    return world


class RealmCoPlayBatchV13:
    """Batch-shaped focal environment; peers act through current Realm candidates."""

    def __init__(self, count, seed, role, regions, horizon, partner_policies=None, partner_metadata=None,
                 worksite_curriculum="none"):
        if count < 1 or count > 32 or role not in range(3) or regions not in (1, 2, 6, 26):
            raise ValueError("Realm co-play bounds")
        if horizon < 1 or horizon > 1024:
            raise ValueError("Realm co-play horizon")
        if worksite_curriculum not in ("none", WORKSITE_CURRICULUM_VERSION):
            raise ValueError("unknown Economic worksite curriculum")
        if worksite_curriculum != "none" and role != 1:
            raise ValueError("worksite curriculum applies only to Economic training")
        self.seed = int(seed)
        self.role = int(role)
        self.regions = int(regions)
        self.horizon = int(horizon)
        self.worksite_curriculum = worksite_curriculum
        self.partner_metadata = partner_metadata or {}
        self.policies = {}
        contract_world = activate_timeline_contract(core.Realm(1, self.regions, 128))
        self.model_contract = contract_world.model_contract
        self.config_fingerprint = contract_world.config_fingerprint
        for family, policy in (partner_policies or {}).items():
            if int(family) not in range(3):
                raise ValueError("unknown partner family")
            metadata = self.partner_metadata.get(int(family))
            expected = {
                "family": FAMILY_NAMES[int(family)],
                "schema": core.REALM_SCHEMA,
                "observation": OBS,
                "actions": ACTIONS,
                "hidden": HIDDEN,
                "model_contract": self.model_contract,
                "config_fingerprint": self.config_fingerprint,
                "reward_version": core.REALM_REWARD_VERSION,
            }
            if not isinstance(policy, Policy) or metadata is None or any(metadata.get(key) != value for key, value in expected.items()):
                raise ValueError(f"partner policy {family} does not match the frozen Realm v13 contract")
            frozen = deepcopy(policy).cpu().eval()
            for parameter in frozen.parameters():
                parameter.requires_grad_(False)
            self.policies[int(family)] = frozen
        self.worlds = [None] * count
        self.actors = np.zeros(count, dtype=np.int64)
        self.steps = np.zeros(count, dtype=np.int64)
        self.opening = np.zeros(count, dtype=np.float64)
        self.finished = np.zeros(count, dtype=bool)
        self.final_scores = np.zeros(count, dtype=np.float32)
        self.outcomes_data = [None] * count
        self.scenarios = ["natural"] * count
        self.peer_hidden = [dict() for _ in range(count)]
        self.peer_family = [dict() for _ in range(count)]
        self.peer_rng = [dict() for _ in range(count)]
        self.peer_decisions = 0
        for index in range(count):
            self._reset(index)

    @staticmethod
    def _actor(seed, role, regions):
        if role == 0:
            return 128 if seed % 2 == 0 else 1 + seed % (regions + 1)
        if role == 1:
            return 2 + regions + seed % (regions + 1)
        return 2 * (regions + 1) + 1 + seed % (128 - 2 * (regions + 1))

    @staticmethod
    def _legal_actions(world, actor, intent, good=None):
        observation = np.asarray(world.observe(actor), dtype=np.float32)
        mask = np.asarray(world.mask(actor), dtype=np.float32)
        features = observation[STATE:].reshape(ACTIONS, FEATURES)
        candidates = []
        for slot in range(ACTIONS):
            if mask[slot] <= 0.5 or round(float(features[slot, 0]) * 32) != intent:
                continue
            if good is not None and round(float(features[slot, 2]) * 26) != good:
                continue
            candidates.append(slot)
        return candidates

    @staticmethod
    def _economic_actor(world, preferred=None):
        if preferred is not None:
            person = world.person(preferred)
            if person["alive"] and person["family"] == 1 and person["cash"] >= 100:
                return int(preferred)
        for person_id in range(1, 129):
            person = world.person(person_id)
            if person["alive"] and person["family"] == 1 and person["cash"] >= 100:
                return person_id
        raise RuntimeError("worksite curriculum has no funded Economic owner")

    @staticmethod
    def _site_worker(world, owner):
        region = int(world.person(owner)["region"])
        for person_id in range(1, 129):
            person = world.person(person_id)
            if (person_id != owner and person["alive"] and person["family"] == 1
                    and person["region"] == region):
                return person_id
        for person_id in range(1, 129):
            person = world.person(person_id)
            if (person["alive"] and person["family"] == 2
                    and person["region"] == region):
                return person_id
        raise RuntimeError("worksite curriculum has no same-region worker")

    def _bootstrap_workshop(self, world, owner, with_fiber, deposit=False):
        if world.mask(owner)[80] <= 0.5:
            raise RuntimeError("worksite curriculum has no private Cloth workshop candidate")
        ok, reason = world.step(owner, 80)
        if not ok:
            raise RuntimeError(f"worksite curriculum workshop creation failed: {reason}")
        site = int(world.establishments()[0]["id"])
        if not with_fiber:
            if world.mask(owner)[89] > 0.5:
                raise RuntimeError("worksite curriculum transfer is legal without Fiber inventory")
            return site

        purchases = self._legal_actions(world, owner, 1, 9)
        if not purchases:
            raise RuntimeError("worksite curriculum has no legal Fiber purchase")
        ok, reason = world.step(owner, purchases[0])
        if not ok:
            raise RuntimeError(f"worksite curriculum Fiber purchase failed: {reason}")
        transfers = self._legal_actions(world, owner, 26)
        if not transfers:
            raise RuntimeError("worksite curriculum Fiber did not expose a transfer candidate")
        if deposit:
            ok, reason = world.step(owner, transfers[0])
            if not ok:
                raise RuntimeError(f"worksite curriculum Fiber deposit failed: {reason}")
        elif world.mask(owner)[89] <= 0.5:
            raise RuntimeError("worksite curriculum transfer candidate is not legal")
        return site

    def _reset(self, index):
        seed = self.seed
        self.seed += 1
        world = core.Realm(seed, self.regions, 128, "", self.role == 0 and seed % 2 == 0)
        if not world.choose(world.view()["cards"][seed % 3]):
            raise RuntimeError("Realm draft choice failed")
        activate_timeline_contract(world)
        actor = self._actor(seed, self.role, self.regions)
        scenario = "natural"
        if self.worksite_curriculum == WORKSITE_CURRICULUM_VERSION:
            variant = seed % 4
            if variant == 1:
                actor = self._economic_actor(world, preferred=actor)
                scenario = "worksite_no_input"
                self._bootstrap_workshop(world, actor, with_fiber=False)
            elif variant == 2:
                actor = self._economic_actor(world, preferred=actor)
                scenario = "worksite_with_fiber"
                self._bootstrap_workshop(world, actor, with_fiber=True)
            elif variant == 3:
                actor = self._economic_actor(world, preferred=actor)
                owner = actor
                scenario = "authorized_work_at_site"
                site = self._bootstrap_workshop(world, owner, with_fiber=True, deposit=True)
                worker = self._site_worker(world, owner)
                cursor = world.game_tick
                ok, reason, _ = world.work_authorization_command(
                    owner, world.person(owner)["decisions"] + 1, site, worker, 0,
                    cursor, cursor + 31 * 16,
                )
                if not ok:
                    raise RuntimeError(f"worksite curriculum authorization setup failed: {reason}")
                if not self._legal_actions(world, worker, 28):
                    raise RuntimeError("authorized curriculum worker has no WorkAtSite candidate")
            if not world.invariant():
                raise RuntimeError(f"worksite curriculum setup broke Realm invariant: {scenario}")
        if self.role == 2:
            while world.observe(actor)[102] > 0.5:
                actor = 2 * (self.regions + 1) + 1 + (actor - 2 * (self.regions + 1)) % (128 - 2 * (self.regions + 1))
        self.worlds[index] = world
        self.actors[index] = actor
        self.scenarios[index] = scenario
        self.steps[index] = 0
        self.opening[index] = world.utility(actor)
        self.finished[index] = False
        self.final_scores[index] = 0
        self.outcomes_data[index] = None
        self.peer_hidden[index].clear()
        self.peer_family[index].clear()
        self.peer_rng[index] = {
            person_id: np.random.default_rng((seed << 32) ^ person_id ^ 0xD0A1B41E91)
            for person_id in range(1, 129)
        }

    def observe(self):
        observations = np.empty((len(self.worlds), OBS), dtype=np.float32)
        masks = np.empty((len(self.worlds), ACTIONS), dtype=np.float32)
        for index, world in enumerate(self.worlds):
            actor = int(self.actors[index])
            observations[index] = world.observe(actor)
            masks[index] = world.mask(actor)
            if self.finished[index]:
                masks[index].fill(0)
                masks[index, 0] = 1
        return observations, masks

    def _baseline_peer(self, env_index, family, actor, mask):
        world = self.worlds[env_index]
        observation = np.asarray(world.observe(actor), dtype=np.float32)
        return baseline_action(family, observation, mask)

    def _peer_action(self, env_index, family, actor, observation, mask):
        world = self.worlds[env_index]
        policy = self.policies.get(family)
        if policy is None:
            return self._baseline_peer(env_index, family, actor, mask)
        if self.peer_family[env_index].get(actor) != family:
            self.peer_hidden[env_index][actor] = torch.zeros(1, HIDDEN)
            self.peer_family[env_index][actor] = family
        with torch.no_grad():
            logits, _, next_hidden = policy(
                torch.from_numpy(observation).unsqueeze(0),
                self.peer_hidden[env_index][actor],
            )
        self.peer_hidden[env_index][actor] = next_hidden
        scores = logits.squeeze(0).numpy().astype(np.float64)
        scores[mask <= 0.5] = -1e9
        scores -= scores.max()
        probabilities = np.exp(scores)
        probabilities /= probabilities.sum()
        action = int(np.searchsorted(
            np.cumsum(probabilities), self.peer_rng[env_index][actor].random(), side="right"
        ))
        action = min(action, ACTIONS - 1)
        if mask[action] <= 0.5:
            raise RuntimeError("frozen peer sampled an illegal action")
        return action

    def _transition(self, index, action):
        if self.finished[index]:
            return 0.0, True, float(self.final_scores[index])
        world = self.worlds[index]
        focal = int(self.actors[index])
        before = world.utility(focal)
        ok, reason = world.step(focal, int(action))
        if not ok:
            raise RuntimeError(f"focal candidate rejected: {reason}")

        days = 7 if self.role == 1 else 30
        for _ in range(days):
            for family in range(3):
                # The focal actor is stepped only by the PPO sampler between intervals.
                for peer in list(world.due(family)):
                    if peer == focal:
                        continue
                    observation = np.asarray(world.observe(peer), dtype=np.float32)
                    mask = np.asarray(world.mask(peer), dtype=np.float32)
                    if observation.shape != (OBS,) or mask.shape != (ACTIONS,) or mask.sum() <= 0:
                        raise RuntimeError("malformed peer observation or mask")
                    peer_action = self._peer_action(index, family, peer, observation, mask)
                    ok, reason = world.step(peer, peer_action)
                    if not ok:
                        raise RuntimeError(f"peer candidate rejected after current observation: {reason}")
                    self.peer_decisions += 1
            world.advance()
            if not world.invariant():
                raise RuntimeError("Realm accounting invariant failed during co-play")
            if not world.person(focal)["alive"]:
                break

        if not world.invariant():
            raise RuntimeError("Realm accounting invariant failed")
        reward = (world.utility(focal) - before) / max(1.0, abs(self.opening[index]))
        done = self.steps[index] + 1 >= self.horizon or not world.person(focal)["alive"]
        score = 0.0
        if done:
            person = world.person(focal)
            loans = [loan for loan in world.loans() if loan["debtor"] == focal]
            due_loans = [loan for loan in loans if loan["due"] <= world.day]
            debt = world.debt(focal)
            self.outcomes_data[index] = {
                "alive": person["alive"],
                "health": person["health"],
                "nutrition": person["nutrition"],
                "cash": person["cash"],
                "debt": debt,
                "insolvent": person["cash"] < debt,
                "obligations_due": len(due_loans),
                "defaults": sum(loan["defaulted"] for loan in due_loans),
                "days": world.day,
                "scenario": self.scenarios[index],
                "decisions": self.steps[index] + 1,
                "raw_utility_delta": world.utility(focal) - self.opening[index],
                "invariant": world.invariant(),
                "establishment_count": len(world.establishments()),
            }
            score = (world.utility(focal) - self.opening[index]) / max(1.0, abs(self.opening[index]))
            self.final_scores[index] = score
            self.finished[index] = True
        self.steps[index] += 1
        return reward, done, float(score)

    def step(self, actions, auto_reset=True):
        actions = np.asarray(actions, dtype=np.int64)
        if actions.shape != (len(self.worlds),):
            raise ValueError("Realm co-play action shape")
        observations, masks = self.observe()
        for index, action in enumerate(actions):
            if action < 0 or action >= ACTIONS or masks[index, action] <= 0.5:
                raise ValueError("invalid focal action")
        rewards = np.zeros(len(self.worlds), dtype=np.float32)
        dones = np.zeros(len(self.worlds), dtype=bool)
        scores = np.zeros(len(self.worlds), dtype=np.float32)
        for index, action in enumerate(actions):
            rewards[index], dones[index], scores[index] = self._transition(index, action)
        if auto_reset:
            for index in np.flatnonzero(dones):
                self._reset(int(index))
        next_observations, next_masks = self.observe()
        return next_observations, next_masks, rewards, dones, scores

    def outcomes(self):
        return [dict(item) if item is not None else {} for item in self.outcomes_data]

    def state_dict(self):
        """Capture the exact Realm and per-actor sampling/memory state at an update boundary."""
        return {
            "seed": self.seed,
            "role": self.role,
            "regions": self.regions,
            "horizon": self.horizon,
            "worksite_curriculum": self.worksite_curriculum,
            "timeline_contract_fixture": TIMELINE_CONTRACT_FIXTURE_VERSION,
            "model_contract": self.model_contract,
            "config_fingerprint": self.config_fingerprint,
            "worlds": [world.serialize("realm-coplay-v13") for world in self.worlds],
            "actors": self.actors.copy(),
            "scenarios": list(self.scenarios),
            "steps": self.steps.copy(),
            "opening": self.opening.copy(),
            "finished": self.finished.copy(),
            "final_scores": self.final_scores.copy(),
            "outcomes": deepcopy(self.outcomes_data),
            "peer_hidden": deepcopy(self.peer_hidden),
            "peer_family": deepcopy(self.peer_family),
            "peer_rng": deepcopy([{
                actor: rng.bit_generator.state for actor, rng in env.items()
            } for env in self.peer_rng]),
            "peer_decisions": self.peer_decisions,
            "baseline_version": BASELINE_VERSION,
            "reward_version": core.REALM_REWARD_VERSION,
        }

    def load_state_dict(self, state):
        expected = (len(self.worlds), self.role, self.regions, self.horizon)
        actual = (len(state["worlds"]), state["role"], state["regions"], state["horizon"])
        if actual != expected:
            raise ValueError("Realm co-play resume dimensions or role mismatch")
        if state.get("baseline_version") != BASELINE_VERSION:
            raise ValueError("Realm co-play resume baseline version mismatch")
        if state.get("reward_version") != core.REALM_REWARD_VERSION:
            raise ValueError("Realm co-play resume reward version mismatch")
        if state.get("worksite_curriculum") != self.worksite_curriculum:
            raise ValueError("Realm co-play resume curriculum version mismatch")
        if (state.get("timeline_contract_fixture") != TIMELINE_CONTRACT_FIXTURE_VERSION
                or state.get("model_contract") != self.model_contract
                or state.get("config_fingerprint") != self.config_fingerprint):
            raise ValueError("Realm co-play resume model contract or fixture version mismatch")
        restored = []
        for saved in state["worlds"]:
            world = core.Realm(1, self.regions, 128)
            ok, reason = world.deserialize(saved, "realm-coplay-v13")
            if not ok:
                raise ValueError(f"Realm co-play resume failed: {reason}")
            restored.append(world)
        self.seed = int(state["seed"])
        self.worlds = restored
        self.actors = np.asarray(state["actors"], dtype=np.int64).copy()
        self.scenarios = list(state["scenarios"])
        self.steps = np.asarray(state["steps"], dtype=np.int64).copy()
        self.opening = np.asarray(state["opening"], dtype=np.float64).copy()
        self.finished = np.asarray(state["finished"], dtype=bool).copy()
        self.final_scores = np.asarray(state["final_scores"], dtype=np.float32).copy()
        self.outcomes_data = deepcopy(state["outcomes"])
        self.peer_hidden = deepcopy(state["peer_hidden"])
        self.peer_family = deepcopy(state["peer_family"])
        self.peer_rng = []
        for saved_env in state["peer_rng"]:
            env = {}
            for actor, rng_state in saved_env.items():
                rng = np.random.default_rng()
                rng.bit_generator.state = rng_state
                env[int(actor)] = rng
            self.peer_rng.append(env)
        self.peer_decisions = int(state["peer_decisions"])


