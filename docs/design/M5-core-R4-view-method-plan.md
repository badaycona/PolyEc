# Kế hoạch R4 — tách điểm vào `Realm::view()`

Ngày 2026-10-22. Lát này tiếp nối bộ chiếu RealmView thuần đã có và việc tách phần điều phối save/load. Không thay đổi thông tin người chơi nhìn thấy.

## Mục tiêu và hành vi

`Realm::view(observer)` hiện nằm trong translation unit mô phỏng lớn. Nó lọc danh sách sự kiện bằng `Realm::knows(observer, fact)`, sau đó gọi `detail::project_realm_view` để tạo bản đọc chỉ chứa dữ liệu UI/runtime cần. Chuyển nguyên hàm thành viên này sang `core/src/realm_view.cpp`. Realm tiếp tục quyết định người quan sát được xem sự kiện nào; projector tiếp tục tạo các tổng số và bản sao dữ liệu hiển thị.

Không đổi visibility rule, trường của `RealmView`, action, observation, reward, save, binding hoặc model.

## Phạm vi file

- `core/src/realm.cpp`: bỏ định nghĩa `Realm::view` và include projector chỉ dùng bởi hàm đó.
- `core/src/realm_view.cpp`: định nghĩa hàm thành viên nguyên trạng và gọi projector hiện có.
- `core/CMakeLists.txt`: thêm translation unit mới.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`, report và manifest sau khi kiểm tra.

## Kiểm tra

Ngay: build Release targets `polyec_realm_view_projector_tests` và `polyec_realm_tests`; chạy hai executable. Bộ Realm kiểm quyền biết, tổng số, bản sao view và đường gọi Realm thật; projector test giữ oracle độc lập cho phần tổng hợp.

Không chạy full CTest, Python/native/GDExt/Godot hoặc package: không đổi chữ ký hoặc dữ liệu public. Mốc tích hợp adapter vẫn là khi nhóm ownership/di sản ổn định; báo cáo sẽ ghi phiên bản source chưa chạy adapter.
