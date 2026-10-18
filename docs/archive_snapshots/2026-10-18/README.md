# Archive snapshots — 2026-10-18

Tệp này lưu các snapshot cũ trước khi hợp nhất bộ docs chính. Đây không phải source of truth.

## Nội dung có thể xem lại

- `docs/archive/design/M5-simulation-design-decisions.md` — quy tắc thiết kế M5, attention, presence, schedule, transaction, và các nguồn cảm hứng.
- `docs/archive/design/M5-draft.md` — bản draft M5 trước khi hợp nhất.
- `docs/archive/design/M5-1a-household-accounts-plan.md` — kế hoạch account/ledger/household trước khi tích hợp vào DEVELOPER_GUIDE.
- `docs/archive/design/simulation-complexity-scope.md` — tradeoff quy mô, scaling, perf, và ranh giới mô phỏng.
- `docs/archive/design/implementation-plan.md` — bản kế hoạch lớn cũ, dùng như lịch sử quyết định.

## Những phần đã được hợp nhất vào docs chính

- household/accounting rules → DEVELOPER_GUIDE.md
- scaling/performance tradeoff → IMPLEMENTATION_PLAN.md
- simulation design D1–D8, causal evidence, agent-centric policy → IMPLEMENTATION_PLAN.md & DEVELOPER_GUIDE.md
- bug/decision timeline → DEVELOPMENT_HISTORY.md

## Mục đích của snapshot

Giữ được nguyên bản lịch sử để đối chiếu, nhưng không để docs chính phụ thuộc vào nó. Source of truth là:

- docs/IMPLEMENTATION_PLAN.md
- docs/DEVELOPER_GUIDE.md
- docs/CURRENT_TASK.md
- docs/DEVELOPMENT_HISTORY.md