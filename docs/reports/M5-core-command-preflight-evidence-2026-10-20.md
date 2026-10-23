# M5 core audit — RealmCommand preflight

Ngày 2026-10-20. Accepted sau Codex self-review; không có reviewer độc lập. Plan: [RealmCommand preflight](../design/M5-core-command-preflight-plan.md).

## Thay đổi

- `Realm::execute(RealmCommand)` reject actor chết, `decisions`/`sampling` tràn và `next_fact_` tràn trước `legal` hoặc mutation. Nhờ đó `Hold` không đi vòng qua alive check và sequence không thể wrap từ max về 0.
- Khôi phục các nhãn `Realm::action_name` thành tiếng Việt UTF-8 đúng. Điều tra tìm thấy mojibake ở bảng nhãn cũ (`Tuyển dụng`, `Đề nghị`, `Tham gia hội`, …); không tìm thấy Cyrillic trong code/docs đã quét. Đây giải thích ký tự lạ trong nhãn game, nhưng không chứng minh nguyên nhân các lượt trả lời assistant từng đổi ngôn ngữ.
- Thêm regression cho actor chết, ba counter ở max, fingerprint không đổi khi reject, và nhãn `Chờ` / `Tuyển dụng`.

## Gate chạy

| Gate | Kết quả |
|---|---|
| Release build `polyec_realm_tests` | Exit 0 |
| Release direct Realm test | 25.779 checks passed; profile A 10.000 người, 19.743 sparse edges |
| Debug build `polyec_realm_tests` | Exit 0 |
| Debug direct Realm test | 25.779 checks passed; cùng profile |
| UTF-8 label scan | Pass; không replacement character hoặc mojibake signatures trong định nghĩa nhãn |

Lần đầu test Release bắt fixture của lát này dùng actor 129 trong world 128 người; tăng fixture lên 256, chạy lại. Đây là lỗi test fixture, không phải lỗi core. Sau khi thêm nhãn UTF-8, chạy lại cả Release và Debug với kết quả nêu trên.

## Giới hạn và bước tiếp

Chỉ chạy target Realm test Release/Debug, không chạy full CTest vì thay đổi nằm trong `RealmCommand` và nhãn action. Lát này không sửa `ScheduleCommand`, establishment/work authorization, allocator record-ID, tổng bảo toàn tiền, production attendance indexing, Python, Godot hay model. Review là self-review. Next: audit ID allocator và checked money ledger trong `Realm::invariant` bằng fixture save biên, rồi xử lý chi phí quét attendance/site trước profile B. M5/core refactor vẫn mở.
