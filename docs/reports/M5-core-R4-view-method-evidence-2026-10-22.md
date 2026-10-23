# Bằng chứng R4 — tách điểm vào tạo RealmView

Ngày 2026-10-22. Plan: [tách điểm vào `Realm::view()`](../design/M5-core-R4-view-method-plan.md). Đây là thay đổi cấu trúc, không thay đổi màn hình hoặc thông tin game cung cấp.

## Phần code đã thay đổi

Định nghĩa `Realm::view(observer)` được chuyển từ `core/src/realm.cpp` sang `core/src/realm_view.cpp`. Nó vẫn duyệt các sự kiện và chỉ đưa sự kiện mà `Realm::knows(observer, fact)` cho phép vào danh sách; sau đó gọi projector thuần hiện có để tính tổng người, tổng người sống/trẻ em/quyết định, tổng tiền và sao chép vùng, sự kiện, bản nháp sang `RealmView`. Không đổi trường public, quy tắc quyền biết, action, observation, reward, model, save hoặc binding.

## Kiểm tra đã chạy

- Windows Visual Studio Release build targets `polyec_realm_view_projector_tests` và `polyec_realm_tests`: thành công.
- `polyec_realm_view_projector_tests.exe`: **5 checks passed**.
- `polyec_realm_tests.exe`: **20.639 checks passed**, bao gồm đường Realm thật và workload profile A 10.000 người/19.743 quan hệ thưa/90 ngày; không phải profile B.
- Sau khi hai phần R4 ổn định cùng nhau, build toàn bộ targets core Release thành công và CTest **19/19 pass** (39,35 giây). Lượt CTest trước đó phát hiện ba oracle token count cũ chưa tính header rỗng `PARENTAGE_1`; chúng đã được sửa trong `core/tests/test_schedule.cpp`, rồi build và chạy lại thành công.
- Sau đó build lại Python extension và C++ replay với core hiện tại: Python Realm replay **273 checks/64 ngày**, clock **173 checks/7 checkpoint**; tất cả pass. Hai test Python còn giữ nhãn schema24 dù writer đã lên schema25; đã sửa phép dựng schema21 và magic kỳ vọng clock trước khi rerun.
- Native runtime contracts `runtime_contracts`: **1/1 pass**. GDExtension Windows Release build pass. Godot 4.5.1 headless `realm_worksite_smoke.gd`: **5/5 pass**, xác nhận extension nhận schema hiện tại và từ chối bundle v6 cũ.
- Review source do Codex tự thực hiện: xác nhận thân hàm chỉ được di chuyển, Realm vẫn lọc quyền xem trước khi project. Không có reviewer độc lập.

## Chưa kiểm tra và giới hạn kết luận

Không chạy Debug, full 64-day Realm smoke bằng model tương thích, neural inference/learned-policy quality, profile B hoặc package Windows trên source này. Godot headless chỉ kiểm model-guard/native wiring, không phải UI playtest. R4 đã có C++/Python/native/GDExt/Godot checks trên source hiện tại; điều đó không chứng minh M5 hoặc learned policy hoàn tất. Không commit/push.

Manifest hash: [đối chiếu file của lát](../../artifacts/m5-core-r4-view-method-2026-10-22-manifest.json).
