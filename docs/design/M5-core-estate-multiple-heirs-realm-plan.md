# Kế hoạch M5 — chia cash/hàng cho nhiều người hưởng qua Realm

Ngày 2026-10-22. Lát kiểm chuỗi Realm cho luật đã được chốt tại `IMPLEMENTATION_PLAN.md` §2: tiền và từng loại hàng chia đều cho hàng người hưởng đầu tiên; đơn vị lẻ theo ID nhỏ nhất. Land title và xưởng có nhiều người hưởng tiếp tục ở trong hồ sơ vì chưa có luật đồng sở hữu.

## Mục tiêu và hành vi

Dùng pha tử vong Realm thật với hai người con hợp pháp còn sống trong cùng hàng hưởng. Sau khi khoản vay đã thanh toán đủ, xác nhận cash và từng loại hàng được chia đúng; total money/invariant giữ nguyên; title, quyền sử dụng và xưởng không đổi chủ, ID vẫn được hồ sơ giữ; save/load tiếp tục đúng.

Để kiểm tra rõ phần dư, fixture cấp 5 Fiber cho hai người hưởng. Mỗi người nhận 2; đơn vị dư thứ năm về người có ID nhỏ hơn. Cash dùng cùng quy tắc chia nguyên và phần dư.

Không tạo co-ownership, không chia diện tích đất, không chọn một người đại diện cho xưởng. Đây là kiểm thử ranh giới hiện có, không mở rộng luật.

## Phạm vi

- `core/tests/test_realm.cpp`: thêm test-only fixture cấu hình hai child heirs, giữ người khác/cash tổng; chạy death phase, kiểm từng share, tài sản còn lại, loan, invariant và save/load.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`, report và manifest sau khi kiểm.
- Không sửa gameplay source, schema, API, binding, reward, observation hoặc model.

## Kiểm tra

Ngay: build Release target `polyec_realm_tests`, chạy trực tiếp executable; manager tests giữ oracle chia đều/reminder độc lập. Không chạy lại full CTest vừa pass trước các test-only M5 fixture. Adapter estate group vẫn chờ các quy tắc sở hữu chưa được định nghĩa; R4 adapter checks đã chạy riêng trên source hiện tại.
