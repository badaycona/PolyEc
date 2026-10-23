# M5 core audit — allocator IDs và ledger tiền

Ngày 2026-10-20. Lát tiếp theo sau RealmCommand preflight; giữ scope hẹp trên whole-world invariant/save acceptance.

## Contract

`Realm::invariant` phải trả `false` an toàn khi allocator zero, record ID non-Person bằng zero/trùng/ngoài miền allocator, fact ID bằng zero/trùng/ngoài miền fact counter, money total không thể biểu diễn bằng `int64_t`, hoặc `opening_money + endowment` tràn. Person và non-Person là namespace riêng; ID Person có thể trùng số với Establishment/record khác. Loan invariant phải kiểm `principal + interest` trước khi cộng. `total_money()` không thực hiện signed overflow: trả tổng hợp lệ hoặc ném `std::overflow_error` nếu caller gọi trên state không hợp lệ.

Save lỗi vẫn phải reject atomically trong `deserialize`; không đổi token grammar/schema hoặc cách cấp ID ở trạng thái hợp lệ.

## Files/gates

- Core: `core/include/polyec/realm.hpp`, `core/src/realm.cpp`.
- Regression: `core/tests/test_realm.cpp` với save fixtures cho allocator zero/below-record, record ID zero, namespace Person overlap, money account sum overflow, opening+endowment overflow và loan interest overflow.
- Build và full core CTest Release/Debug tuần tự; direct `polyec_realm_tests` Release/Debug; `git diff --check`, UTF-8/no-BOM/trailing whitespace, repository links.

## Không thuộc lát này

Preflight cấp ID cho mọi mutating entry point ngoài Establishment/RealmCommand; Campaign IDs; overflow của `balances()`/loan API ngoài invariant; attendance indexing/profile B; Python/Godot, learned policy/model, performance và backlog gameplay. Nếu review phát hiện lỗ hổng không thể chặn ở invariant, ghi follow-up riêng thay vì mở scope ngầm.
