# M5 core audit — RealmCommand preflight

Ngày 2026-10-20. Lát nhỏ tiếp theo sau Gate 6b, theo yêu cầu tự tiếp tục M5/core refactor.

## Contract và phạm vi

Sửa `Realm::execute(const RealmCommand&)` trong `core/src/realm.cpp` để mọi command bị từ chối trước mutation nếu actor đã chết, sequence counter không còn giá trị kế tiếp biểu diễn được, `sampling` không thể tăng, hoặc fact ID không thể cấp. `Hold` chịu cùng các điều kiện actor/counter như các action khác. Giữ thông báo stale sequence cho counter còn hợp lệ; không thay luật gameplay, save grammar, action IDs hay binding. Cùng lượt điều tra, phục hồi bảng nhãn action `Realm::action_name` thành tiếng Việt UTF-8 hợp lệ; kiểm encoding cho hai nhãn có dấu.

Files dự kiến: `core/src/realm.cpp`, `core/tests/test_realm.cpp`, tài liệu plan/evidence và `CURRENT_TASK`/`DEVELOPMENT_HISTORY`.

## Invariant và luồng

- Tra actor trước, rồi reject actor chết.
- Reject `decisions == UINT64_MAX` hoặc `sampling == UINT64_MAX` trước phép cộng.
- Reject `next_fact_ == UINT64_MAX` trước hành động vì mỗi command thành công phát một Fact.
- Sau preflight, kiểm `sequence == decisions + 1`, rồi chạy `legal` và mutation hiện hữu.
- Các reject giữ fingerprint và counter nguyên trạng. Sequence sai ở counter không sát trần vẫn giữ lỗi stale hiện hành.

## Gate

- Regression test: Hold bởi actor chết; `decisions` tối đa kể cả sequence 0; `sampling` tối đa; fact counter tối đa; từng reject không đổi fingerprint; nhãn action mẫu giữ đúng tiếng Việt.
- Build và chạy `realm_tests` riêng Release rồi Debug; không chạy hai cấu hình đồng thời.
- Scoped diff check và repository links sau khi cập nhật report.

## Giới hạn

Đây chỉ là RealmCommand. Schedule/clock/establishment/work-authorization commands có preflight riêng. Record ID allocator, checked money ledger và production attendance index là follow-up khác; không được xem gate này là nghiệm thu toàn bộ core audit.
