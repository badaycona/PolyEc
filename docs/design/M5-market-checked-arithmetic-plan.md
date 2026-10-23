# Kế hoạch kiểm tra tràn số trong mua bán và thuế chợ

## Vấn đề

`Realm::can()` hiện nhân giá đơn vị với lượng hàng để kiểm khả năng mua/bán; `Realm::sales_tax()` nhân tổng giao dịch với thuế suất. Các phép nhân số nguyên có dấu vượt miền `int64_t` là hành vi không hợp lệ, có thể làm sai quyết định chấp nhận trước khi transaction chạy.

## Phạm vi thay đổi

- Thêm `MarketManager` thuần cho tổng tiền `unit_price × quantity` có kiểm tra miền, giá chào mua/bán hiện tại và phép tính thuế tương đương chính xác với quy tắc hiện tại.
- Công thức thuế dùng tách thương/dư để không tạo tích trung gian lớn: vẫn tính `floor((gross × rate + prior_remainder)/100)` và remainder modulo 100. Từ chối đầu vào âm/sai miền; không bão hòa hoặc wrap số.
- `Realm::can()` dùng các giá trị đã kiểm tra; `Realm::execute()` tiếp tục dùng cùng helper trước khi trừ tiền/cập nhật hàng. Pha cư dân mua lương thực bằng tiền cá nhân hoặc quỹ tổ chức cũng kiểm tra sức chứa sổ thuế trước khi trừ tiền/hàng. Không đổi thứ tự hoặc đối tượng sở hữu ledger.
- Thêm test helper cho số âm/0, giá trị chính xác, `INT64_MAX`, rate/remainder hợp lệ và sai miền; giữ Realm market integration test hiện có để kiểm lệnh thành công, phần lẻ thuế, bảo toàn tiền và reject nguyên tử.
- Không đổi gameplay: giá mua vẫn `price+1`, giá bán vẫn `max(1, price-1)`, thuế suất hiện tại không đổi. Không đổi schema, action/observation/reward/binding.

## Kiểm tra ngay

Windows Release build/chạy target mới `polyec_market_manager_tests`, rồi build/chạy `polyec_realm_tests` và `polyec_establishment_transfer_tests` để xác nhận giao dịch, thuế cuối ngày và chuỗi xưởng/chợ qua Realm không đổi. Build target `polyec_env` trên source core mới và chạy smoke C++→Python cho chính chuỗi chợ vì binding gọi trực tiếp Realm. Không full CTest/Debug/native/Godot vì helper và core implementation không đổi ABI, save data hoặc binding; smoke Python chỉ xác nhận chuỗi này, không phải toàn bộ adapter M5 hay learned policy.

## Ranh giới

Lát này chỉ bảo vệ số học của một giao dịch tại chợ. Không thêm giá động theo tháng, nhu cầu/cạnh tranh, dự trữ hợp đồng, vận tải hoặc nguồn cung cho hàng deferred; hệ số cân bằng thị trường vẫn chưa được tài liệu xác định.
