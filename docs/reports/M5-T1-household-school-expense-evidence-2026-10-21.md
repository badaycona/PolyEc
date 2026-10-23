# M5 T1 follow-up — household-funded study expense

Ngày 2026-10-21. Plan: [household expense](../design/M5-T1-household-school-expense-plan.md). Đây là lát tiếp nối account schema20, không đóng household lifecycle hay M5. Tự review của Codex, không có reviewer độc lập. Không commit/push.

Source checkpoint: [manifest](../../artifacts/m5-t1-household-school-expense-2026-10-21-manifest.json).

## Thay đổi

- `available_funds(actor)` bằng cash riêng của actor cộng fund của hộ actor. Không cộng tiền riêng guardian.
- Study và mua cá nhân DryFood stage cash riêng trước rồi fund hộ; tiền đi thẳng tới Province firm. Validation affordability và execution cùng dùng nguồn này; thiếu tiền reject nguyên tử.
- Save schema20 không đổi. Feature 94 giữ shape nhưng đổi ý nghĩa, vì vậy `model_contract` chuyển sang household-fund labels (timeline và config có biến thể). Historical realm-v6 pilot bị đánh dấu không tương thích; runtime bình thường từ chối. Benchmark/dev runtime contract cần truyền override rõ ràng và output báo nhãn legacy pilot.
- Không thay manifest pilot, action/observation dimensions, reward hoặc gameplay config.

## Bằng chứng chạy thực

- Household fund direct test: **35 checks**; Realm direct test: **24.267 checks**.
- Release full core CTest: **16/16 pass**; Debug full core CTest: **16/16 pass**.
- Release native `runtime_contracts`: **1/1 pass**, gồm mặc định từ chối pilot cũ, explicit dev-only override và save/restore contract.
- GDExtension Release `polyec` build hoàn tất.
- Python Release/Debug household binding: **11 fund transaction checks + 7 Study checks** mỗi cấu hình; clock binding **173 checks / 7 replay checkpoints**; migration **40 checks**; gameplay config parity **49 checks / 7 checkpoints**.
- Godot 4.5.1 headless smoke: **4/4 pass**. Nó nạp bundle realm-v6 hiện có và xác nhận GDExtension trả đúng `Model contract mismatch: realm-household-fund-v1`, giữ Realm unloaded. Worksite/household transaction smoke cũ không còn chạy được qua đường runtime sản phẩm với bundle đó.

## Giới hạn và next gate

Không có model đã khai báo contract mới và đạt learned-policy gate; do đó chưa chạy được thao tác Study/household expense trong UI Godot sau thay đổi này. Godot smoke chỉ chứng minh compatibility rejection thực tế, không chứng minh transaction UI. Benchmark realm-v6 cũ là số đo pilot lịch sử/dev-only, không bằng chứng M5 model quality. Còn mở: model contract/manifest cho model huấn luyện mới, learned-policy gates, household land/title, guardian/lifecycle, M5.2–M5.5, profile B, full-cycle performance và Windows package.

## Lệnh kiểm chứng chính

- `cmake --build build/m5-core --config Release` + `ctest --test-dir build/m5-core -C Release --output-on-failure`: 16/16.
- Cùng lệnh ở Debug: 16/16.
- `cmake --build build/m5-runtime --config Release --target polyec_realm_benchmark polyec_realm_contracts polyec_realm_replay`; runtime contracts CTest: 1/1.
- `cmake --build build/windows --config Release --target polyec`: GDExtension build.
- `Godot_v4.5.1-stable_win64_console.exe --headless --path godot_project --script res://tests/realm_worksite_smoke.gd`: household model guard 4/4.
- Python 3.12 scripts: `household_fund_binding_checks.py`, `realm_clock_binding_checks.py --replay .../polyec_realm_clock_replay.exe`, `worksite_payroll_migration_checks.py`, `gameplay_config_binding_checks.py --replay .../polyec_gameplay_config_tests.exe` Release/Debug như tương ứng.

