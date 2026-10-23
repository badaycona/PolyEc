# Core refactor — typed Realm relation kinds

Ngày 2026-10-21. Plan: [typed relation kinds](../design/M5-core-relation-kind-plan.md); checkpoint [manifest](../../artifacts/m5-core-relation-kind-2026-10-21-manifest.json). Codex self-review; không có reviewer độc lập. Không commit/push.

## Thay đổi

- Thay `Edge::kind` từ `int` bằng `RelationKind : uint8_t`: ParentChild=0, Sibling=1, Agreement=2. Các đường tạo quan hệ dùng enum thay cho magic number.
- Codec tiếp tục ghi đúng integer token cũ. Reader parse vào temporary, reject kind ngoài 0..2 trước khi thay vector kết quả. Realm invariant cũng kiểm miền enum.
- Không đổi schema, token hợp lệ, thứ tự dữ liệu, relationship rules, model hoặc API binding.

## Kiểm tra thực tế

- VS18 Release core build và full CTest: **16/16 pass**, 37,05 giây.
- VS18 Debug build `polyec_realm_tests` + `polyec_court_tests`; targeted CTest: **2/2 pass**, 128,67 giây.
- Realm test có direct codec oracle: serialize đúng `1 7 8 2 `, roundtrip enum và reject kind 3 mà giữ nguyên kết quả trước đó.
- `git diff --check` cho các file lát và repository links pass sau khi cập nhật report.

## Giới hạn

Enum phản ánh các quan hệ đang được tạo trong code, không phải quan hệ hôn nhân, custody, thừa kế hoặc quyền tài sản hoàn chỉnh. Chưa chạy Python/native/Godot vì không có binding riêng cho `Edge`; core M5 vẫn mở.

