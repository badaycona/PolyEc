# M5 core audit — index site attendance cho production

Ngày 2026-10-21. Lát sau allocator/ledger invariant; mục tiêu là bỏ scan toàn bộ attendance cho từng Establishment trước khi profile B.

## Contract

`ProductionManager::plan` lập sparse index theo `establishment_id` một lần từ `Input::site_attendance`, rồi chỉ duyệt attendance của site tương ứng. Giữ thứ tự actor tăng dần, recipe/watermark/capacity/ledger results, kiểm orphan/actor, skip ngày đã chạy, và fail trước mutation. Tác vụ nên tăng theo attendance + sites (cộng chi phí map/index), không theo tích số sites × attendance.

## Files và gate

- Core: `core/include/polyec/production_manager.hpp`.
- Oracle regression: `core/tests/test_production.cpp` giữ/bao phủ nhiều site, dead actor, watermark, invalid actor và orphan site.
- Microbenchmark Release: `core/benchmarks/production_attendance_benchmark.cpp` và target `polyec_production_profile`; workload 1.200 site × 8 actor/site, lặp nhiều lần. Ghi baseline trước và kết quả sau cùng workload/compiler/config. Đây không phải full-cycle profile B.
- Build + direct production/Realm regression Release/Debug tuần tự; scoped diff/encoding/link checks. Re-run full CTest chỉ nếu contract/schema files liên quan đổi.

## Không đổi

Không thay save, ownership, schedule attendance, production recipe, transaction commit, gameplay config, action/model contract hoặc M5.5 threshold. Nếu microbenchmark nhỏ không dự đoán workload thực, ghi rõ và vẫn để profile B mở.

## Kết quả đo

Visual Studio 18, Release, một warmup rồi median 9 lượt: trước **96,39 ms**, sau **6,83 ms** cho 1.200 site / 9.600 attendance / 8 worker mỗi site (xấp xỉ **14,1×**). Debug sau thay đổi: 49,41 ms median. Đây là planner microbenchmark, không gồm Realm day pipeline/save/UI/inference và không nghiệm thu profile B.
