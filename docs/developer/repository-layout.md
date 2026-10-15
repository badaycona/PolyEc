# Quy ước thư mục

Root giữ README, WORK_STATUS, AGENTS, CMake và cấu hình/launcher OpenCode. [Mục lục docs](../README.md) phân tài liệu theo mục đích.

| Thư mục | Trách nhiệm |
|---|---|
| `core/include/polyec`, `core/src` | C++20 sở hữu state, validator, transaction và invariant; Realm là nền, Campaign chứa logistics/case M4 |
| `core/tests` | Test luật, hạch toán, replay và rejection nguyên tử |
| `bindings/python` | Training gọi cùng C++ core |
| `native` | ONNX CPU, batch, memory/sampling, parity và runtime contracts |
| `gdext` | Adapter snapshot/command C++ ↔ Godot |
| `godot_project/ui` | Màn hình theo campaign, realm, labs, prototype; main.gd điều hướng |
| `godot_project/tests` | Smoke, capture, profile và fixture Godot |
| `godot_project/models`, `fonts` | Tài nguyên runtime đóng gói local |
| `ai/training`, `evaluation`, `tests` | Training, đánh giá và kiểm tra ML/binding |
| `scripts` | Build, test, train, package và kiểm tra link repo |
| `docs/design` | Đích sản phẩm, domain và đề xuất chưa duyệt |
| `docs/developer` | Bài thuyết minh và cách tổ chức source |
| `docs/experiments` | Protocol/schema theo phiên bản |
| `docs/guides` | Hướng dẫn người chơi |
| `docs/handoff` | Workflow, bàn giao và review |
| `docs/reports` | Kết quả đã chạy, phương pháp và giới hạn |
| `artifacts/models`, `artifacts/visuals` | Hồ sơ thí nghiệm đã ghi và bằng chứng theo milestone/run |

`build/`, `dist/`, `third_party/`, `.godot/` và native binary sinh ra là output/dependency local, không đưa vào commit source. Các model runtime đã theo dõi bằng Git vẫn giữ nguyên.

Đặt script và `.uid` cạnh nhau trong nhóm UI tương ứng, giữ UID khi di chuyển. Scene khởi động vẫn là `godot_project/main.tscn`. Sau khi đổi đường dẫn, cập nhật preload/scene, script package và link tài liệu, rồi chạy smoke liên quan cùng `python scripts/check-repository-links.py`.

Giữ nguyên source lock/manifest cũ để truy nguyên theo revision. Không chỉnh checkpoint hoặc số liệu lịch sử khi chỉ sắp xếp folder.

## Ghi chú Godot 4.5.1 trên Windows

Sau lần chuyển đường dẫn UI, các smoke từ source vẫn pass nhưng import/export headless đã có lần crash. Lần kiểm tra này đã tạo lại cache local và chạy editor thêm 30 frame trước khi export; cả warmup và package sau đó exit 0. Có [báo cáo upstream về crash headless import với GDExtension](https://github.com/godotengine/godot/issues/111645); chưa khẳng định cùng stack trace vì binary local không có debug symbols.

```powershell
<Godot-console.exe> --headless --editor --frame-delay 250 --path godot_project --quit-after 30
.\scripts\package-windows.ps1 -Godot <Godot-console.exe> -BuildDir build/windows -OutputDir dist/PolyEc-M4-Windows -IncludeM4
```

Kiểm tra exit code/log của từng bước; không coi pack cũ còn trên đĩa là bằng chứng export mới thành công. Cache cũ của lần kiểm tra được giữ trong `build/godot-cache-before-layout` để có thể phục hồi, không đưa vào Git.
