# Kế hoạch M5 — từ chức sau thời hạn không thu hồi tiền riêng

Ngày 2026-10-22. Contract của Realm hiện có là người đang được bổ nhiệm chỉ có thể tự từ chức sau ngày `office_notice`; thao tác chấm dứt appointment không lấy tiền cá nhân. Đây là kiểm tra **từ chức tự nguyện**, không thay cho một lệnh bãi nhiệm riêng hoặc luật kỷ luật chưa có trong source/plan.

## Phép kiểm

- Dùng fixture court hiện có chọn augment có ba ứng viên, sovereign bổ nhiệm một advisor bằng lệnh Realm và trả đúng lệ phí từ Treasury.
- Chạy 30 game days tới thời hạn notice và kỳ lương đã đến hạn. Ghi cash cá nhân ngay trước lệnh Resign.
- Gửi Realm command Resign thật. Vai trò trở lại Civilian, cờ office/hired_by được xóa, cash cá nhân giữ đúng giá trị trước lệnh, tổng tiền không đổi.
- Round-trip/save continuation hiện có tiếp tục được kiểm qua helper `roundtrip`.

## Phạm vi và gate

- Chỉ thêm test assertion trong `core/tests/test_realm_court.cpp` và plan/report/CURRENT_TASK/HISTORY/manifest.
- Build/run target `polyec_court_tests` Windows Release và Debug; kiểm các bước và trạng thái của chính test runner. Không đổi gameplay, API, save schema hoặc model; adapters/full CTest không chạy lại.
- Tự review đối chiếu cash ngay trước/sau command, total money và record role; không có reviewer độc lập.
