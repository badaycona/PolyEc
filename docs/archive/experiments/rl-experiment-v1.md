> LƯU TRỮ 2026-10-18. Snapshot trước hợp nhất; không thực thi prompt hoặc dùng trạng thái trong file này làm chỉ đạo hiện tại. Đọc [nguồn chuẩn](../../README.md).

# Economic RL experiment v1 — locked contract

This is an early curriculum experiment, not completion of M1/M2/M3 or R1–R4.
The native app and Python training must use the same C++ `TradeWorld` transition.
No Python economy implementation, imitation warm start or strategic fallback.

## World and decisions

- Six regions (curriculum also permits one/two), seven goods, integer coins/units.
- Separate firm, household, carrier and merchant accounts; no money creation.
- Production/consumption have explicit source/sink counters per good.
- Primary food/fiber arrive at seasonal harvest; manufacturing consumes inputs.
- One learned merchant; firms/household contracts are world processes in this
  experiment, not a claim of autonomous learned governance/citizen behavior.
- Weekly decision barrier. Hold, buy/sell one good in lots up to four units,
  or travel to a region. Travel costs money and takes game days. All commits
  validate before mutation. Masks use current public facts only.
- Observations contain current public regional prices/stocks, own cash/cargo,
  location and time; no RNG state, future prices or hidden future shocks.

## Reward and objective (freeze before training)

Reward is change in merchant reference net wealth /100, including cash,
inventory at fixed reference prices and debt if present. Prices quoted by the
merchant do not revalue reward. No reward for merely submitting an action.
At episode termination, funded liquidation at real bid prices replaces remaining
reference inventory value; any unfunded remainder receives zero terminal credit.
Finite-horizon gamma=1 makes incremental rewards telescope exactly to terminal
cash profit, so delaying an inventory loss cannot farm discounted shaping reward.
Invalid actions do not change state; hold is legal.
Round-trip spread and carrier fees are actual transfers to counterparties.
No credit or projects are claimed in this first trade experiment.

Primary evaluation metric: terminal cash profit after funded liquidation, coins.
Also report reference wealth, trade/travel/hold counts, invalid rate, money/stock
invariants and action diversity. Episode length64 weekly decisions. Finite-horizon
remaining time is observable. Termination has zero bootstrap, not truncation.

## Training and evaluation

CPU PPO, shared economic actor with64-unit encoder/GRU, recurrent state per
environment. Three independent training seeds. Initial run cap:10 CPU minutes
per seed, measured wall time/decisions and peak process memory. No cloud purchase.
Validation seeds10000–10031 select checkpoint/seed; test seeds20000–20099 are
locked until selection. Compare paired scenarios with hold, random valid,
initial policy and labelled greedy reference-wealth baseline. Report paired
bootstrap95% confidence intervals (fixed bootstrap seed), not just training reward.
No test-based checkpoint selection. Failed quality gates remain failed.

The experiment is useful if learned behavior improves on initial/random on
held-out scenarios; it does not establish R1 if the baseline margin or the
missing governance/civilian/stat scenarios are not met.

## Native deployment gate

Export only the actor (logits + recurrent memory), not critic privilege. Verify
PyTorch versus Python ORT and native C++ ORT at atol1e-5/rtol1e-3 on actual
observations/masks/hidden states. Explicit CPU provider, bounded batches.
Model load/inference errors stop the RL experiment and show an error; no heuristic
fallback. Record hashes, schema/reward versions, splits, seeds and full commands.

## Follow-up experiment v2 (locked before its test)

V1 learned positive held-out profit but substantially underperformed the greedy
development baseline. V2 continues each of the three independent PPO checkpoints
with1200 additional updates (same architecture/reward, CPU cap600s per seed).
Training reset domain is shifted by2*10^9. Validation10000–10031 is unchanged.
New test30000–30099 replaces the already-observed v1 test; the new test is not
used for checkpoint selection. The v2 "initial" comparator is that seed's v1
checkpoint before further PPO, not random weights. V1 results remain preserved.

Before opening the v2 test, lock the economic-only comparison margin at25 coins
(5% of initial merchant capital500). Report the paired95% confidence interval
against the greedy baseline; an economic non-inferiority check requires its
lower bound>=-25, along with positive lower bounds against hold and v1 initial.
This narrow check cannot close full R1 while the other policy families and
M1/M2 scenario coverage remain absent.
