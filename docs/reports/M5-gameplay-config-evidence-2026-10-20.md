# M5 — cấu hình gameplay và kiểm tra lại Gate 4

Ngày 2026-10-20. Lát core/save/Python được nghiệm thu sau triển khai, review source và các gate dưới đây. Tự review bởi Codex; không có review độc lập được thực hiện. [Plan](../design/M5-gameplay-config-loader-plan.md) và [gate tích hợp](../design/M5-integration-and-tuning-gates.md) vẫn là contract. M5 và core refactor toàn phần còn mở.

## Thay đổi đã kiểm

`GameplayConfig` đọc tập con TOML chỉ có bảng và số nguyên; reject khóa/bảng lạ, trùng, thiếu, số sai kiểu/vượt miền và file quá 64 KiB với đường dẫn trường. Snapshot đã validate được giữ riêng trong Realm, chỉ có accessor const. Work settlement, payroll và production dùng cùng ticks/unit; lương, capacity mặc định và lượng recipe lấy từ snapshot. Defaults hiện hành không đổi. File thực có loader tại [config/gameplay.toml](../../config/gameplay.toml), chọn qua API tạo Realm hoặc Python `config_path`.

Realm ghi `POLYEC_REALM_15`, giữ `SCHEDULE_5` và `ESTABLISH_3`, thêm `CONFIG_1` snapshot/fingerprint và `WORKPAY_1` phần tiền lương trả dở. Schema 7–14 nhận defaults lịch sử cố định. Load dùng snapshot trong save dù file ngoài đã thay đổi; toàn bộ parse/invariant trên Realm tạm rồi mới thay destination. Phần lương trả dở giữ chính xác nghĩa vụ tiền khi lương/công lớn hơn 1 mà quỹ chưa trả đủ. Ledger vẫn nguyên và được bảo toàn.

Runtime dùng model pilot cũ reject config không mặc định trước thay world/memory; kiểm native xác nhận fingerprint và inference count không đổi. Benchmark JSON và replay ghi fingerprint config. Fingerprint default là `7186450176306598410`; bộ config tính tay (8 ticks/unit, lương 3, capacity 2, 2 Fiber → 3 Cloth) là `8575599135640176315`. Điều này chưa chứng nhận learned policy hoặc model M5 tương thích.

Kiểm tra lại phát hiện/sửa hai lỗ hổng: loader Gate 4 đã so chéo watermark giữa người cùng xưởng, và fixture payroll còn stride Province 26 thay vì 66. Thêm case hai người có số công khác nhau. Oracle fingerprint lịch sử v14 được giữ nguyên bằng cách loại section config mới; không thay số golden để che thay đổi default behavior.

## Bằng chứng chạy

- VS18 Release build và full core CTest: **12/12**, 32,57 s.
- VS18 Debug build và full core CTest: **12/12**, 276,16 s.
- Realm **25.756**, Schedule **3.717**, Production **23**, Campaign **273**, GameplayConfig **32** checks. Checks vẫn hoạt động trong Release.
- Python bindings Release và Debug build thành công. Mỗi cấu hình: parity config **45 checks / 7 checkpoint**, parity replay cũ **259 checks / 64 ngày**. File cấu hình bị đổi sau save vẫn tiếp diễn đúng snapshot.
- Native runtime contract Release: **115 checks**, gồm reject config không được model hỗ trợ và giữ state nguyên.
- Native benchmark Release, seed 37, 10.000 người, 32 ngày với các model pilot hiện có: **10.800 quyết định**, p95 cả ngày **12,6298 ms**, invariant và tổng tiền đúng. [JSON thực chạy](../../artifacts/m5-config-default-native-32d-2026-10-20.json). Chưa đo rendering/save, chưa có worksite trong workload này; không dùng số này thay gate performance M5.
- Scoped `git diff --check` qua; file mới qua UTF-8 không BOM và trailing-whitespace check. Không phát hiện chữ Cyrillic trong source core/binding/native/Godot và các tài liệu tác vụ được kiểm. Kiểm link trước cập nhật report này: **718 references** qua.

Lệnh chính dùng CMake/CTest từ `C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin`, cấu hình hiện có `build/m5-core` và `build/m5-python-ai`; không đổi flags/dependencies/TLS. Parity mới chạy `ai/tests/gameplay_config_binding_checks.py --replay <polyec_gameplay_config_tests.exe>` với `PYTHONPATH` của đúng Release/Debug. Runtime gate chạy CTest `-R '^runtime_contracts$'`.

## Checkpoint và giới hạn

Follow-up trong Gate 5 đã chặn UTF-8/control characters/newline hỏng bên trong comment trước parse; core config checks tăng 32 → 36 và Python 45 → 46 mỗi cấu hình. [Evidence source mới](M5-T2.4-gate5-evidence-2026-10-20.md). Checkpoint bên dưới giữ đúng bản cấu hình đầu tiên đã chạy, không bao gồm sửa follow-up này.

HEAD: `dff2781a41ae52a4bd476b4b1460ad0f09ef5dc3`, cùng working tree có nhiều thay đổi trước đó. [Manifest SHA-256 của 87 file](../../artifacts/m5-gameplay-config-126da340eef0.manifest.json) và [source snapshot](../../artifacts/m5-gameplay-config-126da340eef0.source.zip) lưu trước cập nhật docs nghiệm thu; không coi toàn diff với HEAD là thay đổi riêng của lát này. Không commit/push.

Chưa có lựa chọn config trong Godot, payroll xưởng, transfer thật, learned-policy action/observation mới hoặc M5 performance toàn chu kỳ. Bước tiếp theo là [Gate 5 transfer command](../design/M5-T2.4-gate5-transfers-plan.md), sau đó payroll tài khoản xưởng và chu trình Godot thật. Goal toàn M5/core refactor tiếp tục hoạt động, giữ nguyên backlog M5.0–M5.5.
