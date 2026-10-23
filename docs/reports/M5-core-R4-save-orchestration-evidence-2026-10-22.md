# Bằng chứng R4 — tách điều phối lưu và tải Realm

Ngày 2026-10-22. Plan: [tách điều phối save/load](../design/M5-core-R4-save-orchestration-plan.md). Đây là thay đổi cấu trúc trong core, không bổ sung tính năng game.

## Phần code đã thay đổi

Năm hàm thành viên `Realm::serialize`, `Realm::deserialize`, `Realm::save`, `Realm::load` và `Realm::fingerprint` được chuyển nguyên khỏi file mô phỏng lớn sang translation unit `core/src/realm_save.cpp`. CMake đưa file mới vào thư viện core. File mô phỏng không còn kéo trực tiếp toàn bộ header codec chỉ để lưu/tải; helper checksum FNV-1a 64-bit được đặt riêng trong file save.

Realm vẫn là nơi sở hữu dữ liệu chính thức. Các hàm và codec hiện có tiếp tục quyết định cách tuần tự hóa từng miền, migration schema cũ, kiểm invariant, và lúc nào save đã tải được phép thay thế thế giới đang chạy. Không thay byte grammar, schema 7–25, thứ tự section, checksum, API, binding hoặc hành vi gameplay.

## Kiểm tra đã chạy

- Windows Visual Studio Release build targets `polyec_realm_tests` và `polyec_manager_tests`: thành công.
- `polyec_manager_tests.exe`: pass.
- `polyec_realm_tests.exe`: **20.639 checks passed**. Bộ chạy Realm này có workload profile A 10.000 người, 19.743 quan hệ thưa, 90 ngày; đây không phải profile B.
- Realm tests chứa roundtrip save/load, schema migration và trường hợp save không hợp lệ không được phép ghi đè Realm đích; các kiểm tra đã chạy đều pass.
- Mốc core Release cuối: build toàn bộ targets trong dự án `build/m5-core` thành công; CTest **19/19 pass** (39,35 giây). Lượt CTest đầu trước đó là 18/19: `schedule_tests` kỳ vọng thiếu hai token của header `PARENTAGE_1` rỗng trong ba phép đếm. Đã sửa đúng ba oracle trong `core/tests/test_schedule.cpp`, không thay save writer; build lại và CTest pass toàn bộ.
- Kiểm adapter sau khi build lại từ source hiện tại: Python `polyec_env` và C++ replay target build pass; Python Realm binding parity **273 checks/64 ngày**; clock binding **173 checks/7 checkpoint**. Hai test Python có kỳ vọng schema cũ (21 test cắt từ magic 24, clock test đòi magic 24); đã cập nhật theo schema 25 và chạy lại pass.
- Native runtime contracts: CTest `runtime_contracts` **1/1 pass**. Windows Release GDExtension target `polyec` build pass. Godot 4.5.1 headless `realm_worksite_smoke.gd`: **5/5 pass**, kiểm schema/model guard và từ chối bundle v6 cũ.
- Review nguồn do Codex tự thực hiện: đối chiếu thân hàm đã chuyển, helper checksum giữ cùng hằng số/thuật toán, và xem các đường CMake/link. Không có reviewer độc lập.

## Chưa kiểm tra và giới hạn kết luận

Không chạy Debug, full Realm 64-day campaign smoke bằng model tương thích, neural inference/learned-policy quality, profile B hoặc package Windows trên source này. Godot smoke là guard tương thích model cũ, không phải UI playtest; không có claim model đã chạy. Adapter replay và native contract nêu trên đã chạy trên bản source mới, không dùng kết quả cũ. Việc tách này là một phần của R4, không tự chứng minh M5 hoặc learned policy hoàn tất. Không commit/push.

Manifest hash: [đối chiếu file của lát](../../artifacts/m5-core-r4-save-orchestration-2026-10-22-manifest.json).
