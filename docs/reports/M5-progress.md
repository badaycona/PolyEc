# M5 — Bằng chứng tiến độ (cập nhật 2026-10-15)

Ghi những gì **đã thực sự chạy** cho lát cắt M5 đầu tiên. Không gộp với kết quả M4 cũ; không phải báo cáo M5 hoàn chỉnh.

## Phạm vi lát cắt đã xong

Chỉ một lát cắ: **household membership + giám hộ** trong `Realm`. Chưa có hôn nhân, thừa kế, chuyển hộ, quốc gia/quân sự/ngoại thương hay profile B.

### Thay đổi code chính

- `core/include/polyec/realm.hpp`
  - Bỏ `guardian(const Person&)` kiểu quét `household..household+3`; thêm index dẫn xuất `std::unordered_map<uint64_t,std::vector<uint64_t>> households_` (label → member ID tăng dần).
  - API truy vấn mới: `household_of`, `household_members`, `same_household`, `guardian`, `household_authority`, `household_records`.
  - Helper nội bộ: `index_household`, `reindex_households`, `households_consistent`, `guardian_of`.
- `core/src/realm.cpp`
  - Dựng index sau khi khởi tạo dân số; thêm member khi sinh con; dựng lại sau `deserialize`.
  - `guardian`/`available_funds`/`spend` dùng membership record, không dùng cửa sổ ID cố định.
  - `Intent::Support` đổi target từ `p.household` sang `household_authority(...)`; `legal()` reject `c.target<=0` **trước** khi `person(c.target)`, tránh crash khi household không còn adult sống.
  - `deserialize` reject household label `0` hoặc `> people_.size()` ("Save person household"); `invariant()` kiểm cả index nhất quán (đủ member, đúng label, không trùng).
- `core/tests/test_realm.cpp`
  - Fixture bằng scalar surgery trên save text thật (`patch_save`, `load_fixture`, helper `rejects`) — không friend, không sửa private state.
  - Bao phủ: newborn thừa hưởng household (chọn newborn thật có living adult), membership không liền nhau với adult ngoài cửa sổ ID, giám hộ chết/thiếu, authority cùng hộ, từ chối target hộ khác/self/0/dead, roundtrip save/load giữ membership và guardian, corrupt household label bị reject mà world gọi `deserialize` không đổi.
  - Mọi rejection dùng `decisions+1` hiện tại và kiểm `reason != "Stale or duplicate decision ID"`, để chắc chắn đi qua hạch toán household chứ không bị chặn sớm vì stale sequence.

### Không đổi gì

- Schema save `realm-v6` / magic `POLYEC_REALM_6` **giữ nguyên**; không thêm dynamic household command, nên không có gói model cũ bị từ chối vì tương thích giả tạo.
- Không thêm tài khoản tiền, không nhân bản world; `households_` là state dẫn xuất (không vào `serialize`/`fingerprint`).

## Toolchain và lệnh đã chạy (Windows)

- Compiler: Zig 0.13 + Clang 18, C++20; CMake 3.31.6 (`build/tooling/cmake/data/bin/cmake.exe`).
- Build: `& build/tooling/cmake/data/bin/cmake.exe --build build/m5-core --parallel 4` → exit 0.
- Test: `& build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-core --output-on-failure` → **CTest 5/5, exit 0**.
- `realm_tests`: 25.298 check (profile A 10.000 người, 90 ngày ≈ 0,07 s; sandbox 20 năm × 8 seed).

## Giới hạn của kết quả này

- Chỉ Windows core. **Chưa** build lại Godot app, ONNX Runtime, binding Python, package Linux, profile B, hoặc model đã train.
- App/package Linux tạm hoãn theo ưu tiên tài nguyên của người dùng (workflow `native-linux.yml` chuyển sang chạy tay `workflow_dispatch`); CI core matrix vẫn chạy tự động. Việc kiểm chứng app/package Linux bị hoãn, không bị coi là đã đạt.
- Linux chưa chạy lại cho thay đổi M5 này; không suy từ kết quả Windows.
