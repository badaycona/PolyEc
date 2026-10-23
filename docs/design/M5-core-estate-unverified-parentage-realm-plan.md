# Kế hoạch M5 — quan hệ cha-con chưa được công nhận không tạo quyền thừa kế

Ngày 2026-10-22. Contract nguồn: [IMPLEMENTATION_PLAN.md §2](../IMPLEMENTATION_PLAN.md) chia di sản cho **con hợp pháp**; giới hạn M5 hiện không có luật con ngoài hôn nhân/nhận con nuôi tùy ý. `Edge` lưu cờ `legal_parentage` riêng, vì vậy quan hệ cha-con thông thường không đủ để nhận di sản.

Manager direct tests đã xác nhận cạnh ParentChild không có cờ hợp pháp không chọn người hưởng. Lát này nối kiểm tra đó qua Realm death phase.

## Phép kiểm

- Fixture cho người chết 100 tiền và 5 Paper có nguồn; một người khác còn sống được nối cạnh ParentChild có `legal_parentage=false`.
- Sau pha tử vong, người kia không nhận cash hoặc hàng. Vì không còn thân nhân hợp pháp, 100 tiền ở balance quỹ di sản công và 5 Paper vào kho Province của người chết.
- Kiểm hồ sơ, tổng tiền, invariant và save/load.
- Không thêm phân loại con mới, không suy quyền từ cùng hộ, quan hệ thông thường hoặc guardian.

## Gate

- Build/chạy trực tiếp `polyec_realm_tests` Windows Release và Debug.
- Không chạy full CTest/adapters: chỉ thêm fixture/test, không đổi gameplay production, schema, API hoặc binding; ghi khoảng cách tới full CTest gần nhất.
- Tự review bằng so sánh số dư người không hợp pháp, quỹ công, kho, save/load; không có reviewer độc lập.
