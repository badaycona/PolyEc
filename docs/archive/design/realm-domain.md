> LƯU TRỮ 2026-10-18. Snapshot trước hợp nhất; không thực thi prompt hoặc dùng trạng thái trong file này làm chỉ đạo hiện tại. Đọc [nguồn chuẩn](../../README.md).

# Realm domain and ownership

`Realm` owns province ledgers, person cash/inventory, debt, construction,
relationships, proposals, causal facts and knowledge. `RealmCommand` is the
same execution path for UI and all three policy families. Validation checks
the complete action before resources/state change. Decision sequence numbers
reject duplicates. Travel, school, work and investigations take game days.

`RealmRuntime` owns three CPU sessions and one64-float memory per identified
person. Due actors are batched at most128. Observation is copied; after
inference, every actor gets a fresh legal mask before sampling/commit, so earlier
actors cannot make a batch spend stock twice. Sampling depends on world seed,
person ID and persisted counter. One memory update per committed decision.
No scene node/model instance per person; civilians outside the viewport continue.

Opening profileA:10,000 people,64 governance+64 economic,9,872 civilians. Families
and graph edges are sparse; no dense population matrix. Additional births or
opening scenario assets must be reported separately. Training uses128people with
smaller role counts; this is not itself evidence for profileA/B/C transfer.

Accounting: cash is transferred between people, firms, treasury, carrier or
estate. Loan principal is a liability; interest enters utility immediately.
Every good satisfies opening+production=stock+cargo+consumption. Manufacturing
consumes inputs and projects consume iron/tools. Prices use stock; geography,
harvest and routes affect availability. Explicit finite opening endowments are
included in invariants and cannot be selected twice.

Knowledge: public price books are public. Private shortage facts reach only
their actor or an investigator after its delay; other observations cannot use
hidden facts. Public reports have delay. An observer/fact index avoids scanning
all reports per query. Save persists source facts/knowledge and rebuilds the
derived index. UI sandbox can inspect an actor's point of view; it is not a
campaign secrecy presentation. Causal IDs exist; deep political chain coverage
and long-history presentation remain incomplete.

Save is explicit text fields+RNG with checksum, model bundle pin, per-person
memory and counters. Writes go to a sibling temporary file, flushed before
replacement; previous destination is backed up. Read/check into a temporary
world, then swap. Corrupt/bundle-mismatched loads preserve the live world.
Runtime law version is pinned independently from model hashes.

Version3 fixes a demonstrated work/school exploit: selecting a new intent after
job expiry cannot extend the old wage stream. Regression failed under old laws
and passes under the correction. Pre-fix checkpoints are retained with failed
experiment diagnostics. No final model uses the old transition contract.

Unfinished systems are tracked explicitly in the milestone report. In particular,
the current proposal is a funded transfer with agreement/expiry, not yet full
conditional political negotiation. Stat input and mechanical effects do not
prove strategic intelligence. Twelve-card draft correctness does not prove all
twelve designed augment effects complete.
