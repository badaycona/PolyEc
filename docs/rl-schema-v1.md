# Trade v1 observation / action / reward contract

This schema belongs to the isolated economic curriculum. It does not stand in
for future governance/civilian schemas or the full campaign simulation.

## Ownership

`TradeWorld` in core owns integer money/stock, production/consumption counters,
travel, prices, clock and RNG. Python only calls the compiled binding; native
Godot uses the same class through `EconomicLab`. There is one merchant per world.
Firms/households have fixed contracts/baskets as world rules, not release AI.
Reset creates a new independent training world, not money inside a campaign.

## Observations:310 float32 values

| Indices | Public or owned facts |
|---|---|
|0|Merchant cash /500|
|1|Remaining decisions / horizon|
|2–7|Current region one-hot|
|8–14|Owned cargo per good /16|
|15–56|Current public region-major prices / fixed reference price|
|57–98|Current public region-major stock /120, clipped to5|
|99|Remaining travel days /7|
|100–309|21 candidate rows of10 features|

Candidate features0–3: buy/sell/travel/hold one-hot. Feature4: public ask/ref or
bid/ref for a trade. Feature5: currently executable lot /4 or travel distance /3.
Feature6: owned cash /500. Feature7: cargo for that good /16, or total cargo /16
for travel. Feature8: local public stock /120 for a trade, or the difference
between target and current public quotes for the owned portfolio /100.
Feature9: public carrier fee /100 for travel. Unused fields are zero.
Quotes are provided for every accessible target; the engine does not select an
optimal strategy. The network sees no future yield, RNG state or future prices.
The actor clamps normalized values to[-10,10] inside the exported graph.

##21 flat prototype actions

0 hold;1–7 buy food/fiber/cloth/iron/fuel/tools/horses;8–14 sell those goods;
15–20 travel to region0–5. Trade lot is up to4 units; cargo capacity16. The
mask checks money, goods, counterparty funds, free capacity, destination and
travel state. Hold is available during travel. Terminal worlds cannot commit;
the runner resets only after recording the terminated transition.

Current quotes use ask=mid+1, bid=max(1,mid-1). Carrier fee4+2*shortest ring
distance is paid to a separate carrier account. Travel takes distance+1 days.
An action is committed before seven daily world updates, so it cannot buy/sell
in a target region before arrival. Every rejection happens before mutation.
Batch input is fully validated before the first member is committed.

## Reward and finite horizon

Reference prices are[20,14,35,45,18,65,90]. Intermediate reward is change in
cash plus reference-valued cargo, divided by100. At decision64, funded sales
at actual bid replace inventory reference value; any unsold remainder has zero
terminal scoring credit. Gamma1 makes summed rewards exactly terminal cash
profit /100. Borrowing, projects, credit defaults and campaign saves are absent.
No action-submission bonus or circular transfer reward exists.

## Network and memory

Public/owned state100→MLP64→GRU64. A shared candidate scorer takes recurrent
context plus candidate features and emits21 logits. Critic is training-only.
PPO uses16-step BPTT,64-decision terminated episodes, GAE lambda.95, gamma1,
clip.2 and three update epochs. Each rollout has64 environments with independent
world seeds and recurrent memory. Reset zeroes memory. UI snapshots never run
the network or update memory. Native inference runs once per committed action.

## Tests and deployment

Binding replay, account/stock invariants and20-year weekly sandbox tests precede
training. Schema/reward/splits/config, validation selection and100 test scenarios
are saved. R5 checks PyTorch/Python ORT/C++ ORT logits, memory, masked distribution
and64-step native replay. Godot checks the same64 actions/fingerprints through
the extension. Missing/bad checksum/shape/non-finite input stops the lab; it
cannot fall back to an untrained network or the labelled evaluation baseline.
