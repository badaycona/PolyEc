# M5 core audit — production attendance index

Ngày 2026-10-21. Accepted sau Codex self-review; không có reviewer độc lập. Plan/workload: [site attendance index](../design/M5-core-production-attendance-index-plan.md).

## Thay đổi và hiệu năng

`ProductionManager::plan` tạo sparse map `establishment_id -> [(actor, ticks)]` một lượt từ map attendance theo `(actor, establishment)`, nhờ đó thứ tự actor tăng dần được giữ. Mỗi site chỉ duyệt attendance của chính nó. Validation actor/ticks/person, watermark, dead-worker forfeiture, orphan record, stable site order, recipe/capacity/ledger arithmetic và staged commit giữ nguyên. Không đổi schema/save/command.

Microbenchmark C++ thuần Release (Visual Studio 18), 1 warmup + median 9 lượt, 1.200 site × 8 worker/site = 9.600 attendance:

| Trạng thái | Median planner |
|---|---:|
| Trước index | 96,39 ms |
| Sau index | 6,83 ms |

Kết quả tương đương khoảng 14,1× nhanh hơn workload này. Debug sau index: 49,41 ms median. Benchmark không gồm full Realm pipeline, serialization, UI hay inference; đây chưa phải profile B.

## Gate chạy

| Gate | Kết quả |
|---|---|
| Release `polyec_production_tests` | 26 checks passed |
| Release `polyec_realm_tests` | 25.793 checks passed; profile A 10.000 người |
| Debug `polyec_production_tests` | 26 checks passed |
| Debug `polyec_realm_tests` | 25.793 checks passed; profile A 10.000 người |
| Production planner microbenchmark | Release 96,39 → 6,83 ms median; Debug sau 49,41 ms |

Direct target tests chạy sau thay đổi. Full CTest 15/15 Release/Debug gần nhất thuộc ledger/allocator lát trước đó; lát này không rerun toàn bộ 15 target. Repository links và scoped whitespace/UTF-8 checks được chạy sau cập nhật docs.

## Giới hạn và tiếp tục

Self-review; chưa có reviewer độc lập. Chưa nghiệm thu B/100k, multi-day profile, memory/RSS hoặc Windows package. Tiếp tục T3/M5 gameplay và learned-policy gates; performance profile B vẫn mở.
