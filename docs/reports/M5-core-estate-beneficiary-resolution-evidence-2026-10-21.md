# Core refactor — phân giải thứ tự người hưởng di sản

`EstateManager::resolve_beneficiaries` đọc người chết, danh sách ID và các quan hệ typed, rồi trả về nhóm thân nhân còn sống đầu tiên theo thứ tự trong `IMPLEMENTATION_PLAN` §2: con trực tiếp, phối ngẫu, cha mẹ, anh chị em; nếu không có thì chọn nhánh quỹ di sản công. Danh sách ID được sắp tăng dần, loại trùng. Quan hệ `ParentChild` theo chiều child → parent; anh chị em được nhận diện qua edge trực tiếp hoặc cùng cha/mẹ.

Đầu vào sai ID/edge hoặc ID người còn sống bị từ chối. Không có dữ liệu nào bị đổi. Query này chưa được gọi trong death transition: Realm vẫn chuyển cash còn lại vào `estate_`, đưa inventory về Province và để loan gắn với người chết. Chưa có probate ledger, thanh toán nợ trước, chia tài sản, chuyển title/xưởng, hoặc sổ quỹ di sản công. Vì vậy đây là nền phân giải thuần, chưa phải tính năng thừa kế trong game.

## Kiểm chứng

- Release và Debug `polyec_manager_tests`: pass với mã cuối.
- Full Release build `ALL_BUILD`: pass.
- Full CTest Release: 19/19 pass, 950,24 giây. Lượt này chạy song song với Debug build nên dài hơn lần trước.
- `git diff --check` trong phạm vi source: exit 0; Git báo chuẩn hóa CRLF trên `realm.cpp` ở checkout Windows.

Review tự kiểm Codex. Chưa có review độc lập; chưa chạy Godot/Python/native cho query nội bộ này.

- Plan: [phân giải người hưởng di sản](../design/M5-core-estate-beneficiary-resolution-plan.md)
- Manifest/hash: [m5-core manifest](../../artifacts/m5-core-estate-beneficiary-resolution-2026-10-21-manifest.json)
