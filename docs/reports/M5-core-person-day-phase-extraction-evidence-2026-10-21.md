# Core refactor — tách pha xử lý cư dân theo ngày

Định nghĩa `Realm::day_people` được chuyển nguyên thân hàm từ `realm.cpp` sang `realm_lifecycle.cpp`; CMake thêm translation unit mới. Pha hiện vẫn do Realm gọi đúng vị trí trong pipeline. Các bước tuổi tác, trả lương chức vụ, di chuyển, ghi nhận công, học tập, ăn uống, xác định tử vong và phát sự kiện giữ nguyên thứ tự và công thức.

## Kiểm chứng

- Windows Release `ALL_BUILD`: pass; sinh `polyec_core` và toàn bộ executable test.
- Full CTest Release: 19/19 pass, tổng 674,66 giây. `realm_tests` 430,88 giây; `campaign_tests` 108,41 giây.
- `git diff --check` trên `core/src/realm.cpp` và `core/CMakeLists.txt`: exit 0. Cảnh báo duy nhất là Git chuẩn hóa CRLF của file `realm.cpp` ở checkout Windows.

Đây là refactor cấu trúc, không thêm hành vi cho người chơi và không đổi save/binding/observation/reward. Chưa chạy Debug, Python/native/GDExt/Godot hoặc profile. Review tự kiểm Codex, không phải reviewer độc lập. Không commit/push.

- Plan: [tách pha xử lý cư dân](../design/M5-core-person-day-phase-extraction-plan.md)
- Manifest/hash: [m5-core manifest](../../artifacts/m5-core-person-day-phase-extraction-2026-10-21-manifest.json)
