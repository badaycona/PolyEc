# Kế hoạch M5 — bảo toàn tài sản của người sống khi người khác chết

Ngày 2026-10-22. Core estate ledger design nêu rõ claim chỉ gồm tài sản của người chết và phải giữ nguyên tài sản của người sống. Các test hiện có tập trung vào người chết, người thừa kế và quỹ hộ; lát này bổ sung một Realm fixture để kiểm riêng những title, quyền sử dụng và xưởng vẫn thuộc các chủ thể sống khác.

## Hành vi cần chứng minh

- Ghi snapshot các land title không do người sắp chết đứng tên, land-use rights không do họ cấp và mọi xưởng không thuộc Person đó.
- Chạy Realm death phase với khoản vay và người hưởng như fixture hiện có.
- Mọi holding của chủ thể sống giữ cùng ID, holder, diện tích và vùng; mọi use right giữ cùng ID, holding, người cấp/người dùng; mọi xưởng giữ cùng owner, vùng/ngành/công suất, cash, stock, quyền worker và sổ công/lương.
- Tổng tiền, invariant và save/load tiếp tục đúng.

Đây là test-only; không tạo co-ownership, không chuyển quyền cho guardian, không thay luật probate hoặc thêm dữ liệu lưu.

## Phạm vi và kiểm tra

- Chỉ thêm test/fixture ở `core/tests/test_realm.cpp` cùng plan, report, CURRENT_TASK/HISTORY và manifest.
- Build Release `polyec_realm_tests` và chạy executable trực tiếp. CTest Release 19/19 và Debug 19/19 vừa qua trước test-only additions; không lặp toàn suite sau phép kiểm này.
- Không chạy lại Python/native/GDExt/Godot vì không đổi source xử lý/API/schema/binding.
