> LƯU TRỮ 2026-10-18. Snapshot trước hợp nhất; không thực thi prompt hoặc dùng trạng thái trong file này làm chỉ đạo hiện tại. Đọc [nguồn chuẩn](../../README.md).

# Realm v3 — wage contract exploit fix, locked before the new test

Regression first failed: completing a job contract then starting school kept
job active under the new busy_until. This gave simultaneous school/wages.
Fix: a new non-hold intent releases the previous contract before execution;
continuing a job may retain its slot. Regression now passes. Preserve v2
checkpoints/evaluation as pre-fix experiments, not validated final AI.

Continue each seed's own PPO checkpoint from v2; no imitation. 3 seeds/family,
90updates or90s per seed, two CPU threads,32environments. Curriculum1/2/6 was
completed in v2; continue six regions under corrected laws. New untouched
test80000..80099; validation10000..10031; selection frozen before test. Same
utility/margin5 and CI protocol. Config records warm start; model schema realm-v3.
Paired agency probes70000..70099; parity60000+ never chooses checkpoints.
Budget at most810s training (+ bounded evaluation); no paid compute.
Full R1/R2/R3 require independent diagnostics and remain false unless proved.

Before v3 starts: household grouping is explicit for small training worlds as
well as profileA. Dependent children may spend funded guardian resources for
essentials and school, with private household budget in observation94. Joining
one of four local organizations funds a real mutual-aid account; changing/joining
does not grant repeat social reward. Membership/funds are observed at95/96.
Adult key actors in profileA are initialized as adults. Knowledge lookup is
indexed and redundant public report rows removed without changing disclosure.
These changes are pinned in the realm-v3 source snapshot before training.
