> LƯU TRỮ 2026-10-18. Snapshot trước hợp nhất; không thực thi prompt hoặc dùng trạng thái trong file này làm chỉ đạo hiện tại. Đọc [nguồn chuẩn](../../README.md).

# Runtime sampling audit v3 — locked before evaluation

This additional audit does not replace preserved argmax test80000..80099 or
select checkpoints. Model/seed selection remains validation-only. Native app
samples categorical actor distributions, so evaluate that deployment behavior
on new100worlds90000..90099. Shared uniform variates (seed491) pair learned and
original untrained policies. The original control is initial-N.pt from realm-v1,
before any PPO, not the already-trained checkpoint before v3 fine-tuning.

Controllers: selected learned sampled; original untrained sampled; before-v3
sampled; labelled baseline; hold; uniform valid random. Same corrected C++ world
and training-only civilian partners for every controller.64decisions, role game
durations preserved. Utility, hold, survival, health, cash/debt shortfall, due
obligations/defaults reported. CI: paired bootstrap5000,95%, seed491. Baseline
noninferiority margin5utility fixed as earlier. No checkpoint tuning on this set.

Limits: full legal bankruptcy/conditional negotiation quality are not modeled.
These are focal-actor audits; separate native co-play measures actual decisions,
accounting, population and performance, not a substitute for all R1 quality
metrics. Report diagnoses and full_R1_pass=false while these omissions remain.
