# Bằng chứng M5 — chia tiền/hàng cho nhiều người hưởng trong Realm

Ngày 2026-10-22. Plan: [multi-heir Realm test](../design/M5-core-estate-multiple-heirs-realm-plan.md). Đây là một test cho luật chia tiền/hàng hiện có, không thêm luật đồng sở hữu.

## Hành vi được kiểm

Một người chết để lại khoản vay 20, năm đơn vị Fiber, cash và title/xưởng cá nhân. Hai người con còn sống được đánh dấu là con hợp pháp cùng hàng hưởng. Pha xử lý tử vong của Realm thanh toán khoản vay đủ trước khi phân phối cash/hàng.

- Cash chia hai phần bằng nhau; nếu còn một đơn vị không chia hết, người có ID nhỏ hơn nhận nó.
- Năm Fiber chia thành 2 và 2, đơn vị thứ năm cho người có ID nhỏ hơn.
- Không ghi nợ lên hai người hưởng; tổng tiền Realm và invariant giữ nguyên.
- Holding và quyền sử dụng đất đang có cùng xưởng không bị gán cho một người tùy tiện hoặc chia thành nhiều mảnh. Các record giữ nguyên owner/decedent và case tiếp tục tham chiếu chúng.
- Save/load bảo toàn phần chia, khoản vay, title/quyền dùng/xưởng và fingerprint.

## Thay đổi trong test

`core/tests/test_realm.cpp` bổ sung fixture test-only dựng hai child heir hợp pháp, không đổi Realm gameplay. Helper hiện có tiếp tục tạo khoản vay, tiền/hàng, title, permission và xưởng, vì vậy test chạy death settlement thực thay vì gọi manager riêng lẻ.

## Kiểm tra đã chạy

- Windows Visual Studio Release build `polyec_realm_tests` và `polyec_manager_tests`: pass.
- `polyec_realm_tests.exe`: **20.659 checks passed**, gồm workload profile A 10.000 người/19.743 quan hệ thưa/90 ngày; không phải profile B.
- `polyec_manager_tests.exe`: pass; chia đều và phần dư có oracle manager độc lập.
- Core CTest **19/19** đã pass trước các fixture-only bổ sung; không chạy lại toàn suite. Target Realm/manager bị ảnh hưởng đã build và chạy trực tiếp.
- Review do Codex tự thực hiện; chưa có reviewer độc lập.

## Chưa kiểm và giới hạn

Không chạy Debug, Python/native/GDExt/Godot, learned policy, profile B hoặc package cho lát test này. R4 đã có adapter gates riêng trên source hiện tại. M5 ownership/di sản vẫn chưa xác định cách giữ title/xưởng cho nhiều người cùng hưởng, guardian quản lý tiền/xưởng của trẻ, hoặc xử lý tài sản hiện vật khi nợ vượt cash. Lát này không giải quyết các luật đó. Không commit/push.

Manifest SHA-256: [multi-heir estate Realm](../../artifacts/m5-core-estate-multiple-heirs-realm-2026-10-22-manifest.json).
