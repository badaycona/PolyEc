# Core refactor — tách pha xử lý cư dân theo ngày

## Contract

Di chuyển nguyên hàm `Realm::day_people` từ `core/src/realm.cpp` sang `core/src/realm_lifecycle.cpp`; thêm file vào `polyec_core`. Không đổi thân hàm, thứ tự pha, công thức, event, save format, binding hoặc reward. Realm tiếp tục sở hữu dữ liệu, RNG và quyết định thời điểm gọi pha.

## Phạm vi

- `core/src/realm.cpp`: bỏ định nghĩa duy nhất của `day_people`.
- `core/src/realm_lifecycle.cpp`: định nghĩa hàm với dependency trực tiếp `EmploymentManager`, `EstateManager`, goods catalog.
- `core/CMakeLists.txt`: thêm translation unit.
- Không đổi header public, source khác, action, observation, model hay gameplay.

## Kiểm chứng

Windows Release build `ALL_BUILD`, full CTest, và diff check. So sánh replay/save fixture về bytes/fingerprint nếu harness hiện tại cung cấp oracle ổn định. Review tự kiểm của Codex, không phải review độc lập.
