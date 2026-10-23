# M5 core audit — allocator ID và ledger tiền

Ngày 2026-10-21. Accepted sau Codex self-review; không có reviewer độc lập. Plan: [allocator IDs và ledger tiền](../design/M5-core-ledger-id-invariant-plan.md).

## Thay đổi

- `Realm::invariant` kiểm allocator ID/fact khác zero; mọi loan/project/investigation/proposal/Establishment có ID khác zero, nhỏ hơn `next_id_`, duy nhất trong shared non-Person namespace. Fact ID khác zero, nhỏ hơn `next_fact_`, duy nhất. Person và non-Person numeric IDs được phép trùng theo contract hiện hữu; births có thể mở rộng dải Person ID.
- `Realm::total_money()` dùng checked accumulation qua từng tài khoản, không còn signed overflow; nếu caller gọi trên ledger âm/không biểu diễn được thì ném `std::overflow_error`. `invariant()` dùng cùng checked sum và kiểm `opening_money + endowment` trước phép cộng.
- Loan invariant kiểm biên `principal + interest` trước khi so `paid`.
- Save fixtures kiểm next ID zero/below allocated record, fact allocator zero/không vượt record, duplicate Fact ID, ID record zero, namespace overlap Person được chấp nhận, hai kiểu money overflow, và loan interest overflow. Mọi save lỗi giữ destination fingerprint nguyên vẹn. Existing cross-domain loan/Establishment collision test tiếp tục xác nhận shared non-Person uniqueness.

## Gate chạy

| Gate | Kết quả |
|---|---|
| Release direct Realm test | 25.793 checks passed |
| Debug direct Realm test | 25.793 checks passed |
| Release full core CTest | 15/15, 47,31 giây |
| Debug full core CTest | 15/15, 589,97 giây |
| Repository links | 782 local references passed |
| Scoped diff / UTF-8 / no BOM / whitespace | Pass; Git chỉ báo chuẩn hóa CRLF/LF ở source/test |

Trong quá trình test, fixture vay 50 đồng không đủ Province funds; đổi sang 100/180 đã có tiền lệ trong Realm tests. Một lần đầu thêm nhầm quy tắc “record ID phải lớn hơn Person count” làm test birth-save thất bại; sửa theo contract namespace đang có, rồi full Release/Debug CTest xanh.

## Giới hạn và bước tiếp

Lát này bảo vệ invariant/save acceptance và accessor `total_money`. Chưa kiểm soát mọi đường cấp ID trước mutation ngoài các command đã có preflight; chưa sửa `balances()` overflow, allocator Campaign, attendance scan, hay T3/M5 gameplay. Review là self-review. Next: đo/giảm production attendance scans trước profile B, rồi tiếp tục T3 và backlog M5.0–M5.5 cùng learned policy/model/performance/package gates.
