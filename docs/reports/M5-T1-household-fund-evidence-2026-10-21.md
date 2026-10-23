# M5 T1 — Household fund account evidence

Ngày 2026-10-21. Lát account/transfer nền theo [plan](../design/M5-T1-household-fund-account-plan.md), trên working tree vốn đã dirty; không commit/push. Đây là self-review của Codex, không có reviewer độc lập.

Source checkpoint hashes: [manifest](../../artifacts/m5-t1-household-fund-2026-10-21-manifest.json).

## Thay đổi

- Realm sở hữu sổ thưa `household_id -> cash`; số dư ban đầu bằng 0 và không nằm trong Person hoặc HouseholdIndex.
- Lệnh nộp yêu cầu actor sống, đủ 18 tuổi game time, là thành viên hộ và có cash cá nhân. Lệnh rút yêu cầu actor hiện là `household_authority`. Số tiền phải dương; stale sequence, sai quyền/hộ, thiếu tiền hoặc overflow bị từ chối trước mutation.
- Giao dịch giữ tổng tiền; cập nhật decision/sampling và fact sau khi validation/preallocation hoàn tất. Không thêm candidate/action, observation, reward, heuristic hoặc model behavior.
- `total_money()`/invariant tính quỹ; `balances()` giữ slot 0–9 và append tổng quỹ tại slot 10. Save schema20 thêm sparse `HOUSEHOLD_1`; schema7–19 tải quỹ 0. Python gọi cùng C++ transaction và có query read-only.

## Kết quả kiểm tra thực

- Release core full CTest **16/16 pass**, tổng 23,97 giây.
- Debug core full CTest **16/16 pass**, tổng 303,49 giây. Household fund test trực tiếp báo 19 checks ở Release; Debug test có trong full CTest.
- Release native benchmark/contracts build pass; `runtime_contracts` **1/1 pass** (1,40 giây). Godot GDExtension `polyec` Release build pass.
- Python Release: household command/save **11 checks**; frozen v16 worksite/payroll migration **40 checks**; gameplay config parity **49 checks / 7 checkpoints**; clock binding **173 checks / 7 replay checkpoints**; work authorization binding **7 checkpoints**.
- Python Debug: household command/save **11 checks**; clock binding **173 checks / 7 replay checkpoints**; work authorization binding **7 checkpoints**.
- Household oracle tính tay: nộp 60 từ người đóng góp → fund 60 và cash cá nhân giảm 60; authority rút 25 → fund 35 và cash người đại diện tăng 25; total money và toàn bộ `balances()` reconciled; round-trip fingerprint khớp.
- Schema19 → 20 migration tạo account rỗng. Save HOUSEHOLD_1 có ID ngoài miền bị reject mà không đổi destination Realm.

## Lệnh chính

- `cmake --build build/m5-core --config Release` và `ctest --test-dir build/m5-core -C Release --output-on-failure`: 16/16.
- `cmake --build build/m5-core --config Debug` và `ctest --test-dir build/m5-core -C Debug --output-on-failure`: 16/16.
- `cmake --build build/m5-runtime --config Release --target polyec_realm_benchmark polyec_realm_contracts`; `ctest --test-dir build/m5-runtime -C Release --output-on-failure -R runtime_contracts`: 1/1.
- `cmake --build build/windows --config Release --target polyec`: GDExtension build.
- Python 3.12 với `PYTHONPATH=build/m5-python-ai/Release` và `.../Debug`: `household_fund_binding_checks.py`, `worksite_payroll_migration_checks.py`, `gameplay_config_binding_checks.py --replay build/m5-core/core/<Config>/polyec_gameplay_config_tests.exe`, `realm_clock_binding_checks.py --replay build/m5-python-ai/core/<Config>/polyec_realm_clock_replay.exe`, và `work_authorization_binding_checks.py build/m5-core/core/<Config>/polyec_establishment_transfer_tests.exe` theo cấu hình ghi ở trên.
- `git diff --check` trên các file thuộc lát, whitespace scan file mới, và `python scripts/check-repository-links.py`: 819 local references pass.

## Giới hạn còn mở

Đây chưa là Household entity/lifecycle; chưa có đổi membership/marriage/inheritance, land/holding/rights, guardian asset management, rule chi quỹ cho Study/Food/Aid, UI/command trong Godot, household policy observation/action hoặc learned-policy quality. GDExtension chỉ build; chưa chạy thao tác household trong Godot. Không dùng kết quả này để gọi M5.0/1 hay M5 hoàn tất. Profile B, stress/20-year multi-seed, trained policy gates và Windows package cũng còn mở.

## Quy trình và lỗi phát hiện

Lần CTest Release đầu có 2 fail do fixture giả định schema19/tail BODY_2 là section cuối; cập nhật migration fixture/token-count oracle cho schema20, sau đó full Release và Debug suites đều pass. Một lần Python clock script nhận nhầm `polyec_realm_transition_replay` thay cho `polyec_realm_clock_replay`; lần đúng đã pass. Không ghi hai lần chạy sai này thành gate pass. Không train dài, không commit/push.
