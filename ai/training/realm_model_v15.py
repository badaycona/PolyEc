"""Recurrent actor for the current Realm contract; transitions stay in polyec_env.pyd."""
import math

import torch
from torch import nn

OBS, STATE, ACTIONS, FEATURES, HIDDEN = 3550, 382, 264, 12, 64


class Policy(nn.Module):
    def __init__(self):
        super().__init__()
        self.encoder = nn.Sequential(nn.Linear(STATE, 128), nn.Tanh())
        self.memory = nn.GRU(128, HIDDEN, batch_first=True)
        self.scorer = nn.Sequential(
            nn.Linear(HIDDEN + FEATURES, 64), nn.Tanh(), nn.Linear(64, 1)
        )
        self.critic = nn.Linear(HIDDEN, 1)
        for layer in self.modules():
            if isinstance(layer, nn.Linear):
                nn.init.orthogonal_(layer.weight, math.sqrt(2))
                nn.init.zeros_(layer.bias)
        nn.init.orthogonal_(self.scorer[-1].weight, 0.01)
        nn.init.orthogonal_(self.critic.weight, 1.0)

    def sequence(self, obs, hidden):
        """Encode every fixed Realm state field, then score each candidate."""
        obs = obs.clamp(-10, 10)
        encoded = self.encoder(obs[..., :STATE])
        context, next_hidden = self.memory(encoded, hidden.unsqueeze(0))
        candidates = obs[..., STATE:].reshape(obs.shape[0], obs.shape[1], ACTIONS, FEATURES)
        repeated = context.unsqueeze(-2).expand(-1, -1, ACTIONS, -1)
        logits = self.scorer(torch.cat((repeated, candidates), dim=-1)).squeeze(-1)
        return logits, self.critic(context).squeeze(-1), next_hidden.squeeze(0)

    def forward(self, obs, hidden):
        logits, value, next_hidden = self.sequence(obs.unsqueeze(1), hidden)
        return logits[:, 0], value[:, 0], next_hidden


class ActorExport(nn.Module):
    def __init__(self, policy):
        super().__init__()
        self.policy = policy

    def forward(self, obs, hidden):
        logits, _, next_hidden = self.policy(obs, hidden)
        return logits, next_hidden


def masked_distribution(logits, mask):
    if logits.shape != mask.shape or logits.shape[-1] != ACTIONS:
        raise ValueError("Realm v15 logits/mask dimensions")
    if torch.any(mask.sum(dim=-1) < 1):
        raise ValueError("Realm action mask has no legal candidate")
    return torch.distributions.Categorical(logits=logits.masked_fill(mask < 0.5, -1e9))
