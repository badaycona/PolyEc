# Kế hoạch nối hàng mua ở chợ qua xưởng đến giao dịch thị trường

## Mục tiêu

Nối các lệnh Realm đã có thành một chuỗi C++ duy nhất để kiểm tra người chơi/driver có thể mua nguyên liệu, mở và cấp vốn cho xưởng, chuyển nguyên liệu vào xưởng, làm việc, lưu giữa ca, tiếp tục sản xuất sau khi tải, rồi đưa sản phẩm trở lại chợ để một cư dân khác mua.

## Quy tắc và giới hạn

- Chỉ dùng quy tắc hiện có: giá mua hiện hành cộng một đơn vị; giá bán là `max(1, giá hiện hành − 1)`; sản xuất Cloth theo recipe Fiber → Cloth; tiền công và tick lấy từ snapshot GameplayConfig; giới hạn sức mang hàng và quyền quản lý xưởng giữ nguyên.
- Test phải phát lệnh C++ thật cho Buy, CreateEstablishment, chuyển hàng, Submit Work, WithdrawGoods, Sell và Buy. Không sửa save để cấp hàng hoặc tiền giữa chuỗi.
- Lưu rồi tải sau 16 tick của ca; bản tiếp tục phải có fingerprint bằng bản chạy liên tục tới cùng mốc.
- Tính oracle bán hàng và thuế mua từ Province ngay trước lệnh: `sale = max(1, price − 1)`, `gross = price + 1`, `tax = floor((gross × tax_rate + prior_remainder) / 100)`, `remainder = (...) mod 100`.
- Không đổi luật giá, nhu cầu, nguồn cung, recipe, quyền, observation, action slots, save schema, Python/native/Godot hoặc model.

## Phạm vi file

- `core/tests/test_establishment_transfers.cpp`: nối bán/mua thị trường và roundtrip cuối vào bài kiểm command → sản xuất → save/load hiện có.
- `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`: ghi đúng phạm vi và điều kiện còn mở.
- `docs/reports/M5-command-driven-workshop-to-market-chain-evidence-2026-10-22.md` và manifest: báo cáo, lệnh kiểm, hash nguồn/bằng chứng.

## Kiểm tra

Chạy Windows Release build và runner trực tiếp `polyec_establishment_transfer_tests`. Không chạy toàn CTest, Debug, adapter hoặc app vì chỉ thêm assertion tích hợp ở test C++; không đổi gameplay source, schema hay public consumer.

## Điều kiện kết thúc

Runner xác nhận đủ hàng mua bằng lệnh, tạo xưởng/cấp đầu vào bằng lệnh, có mặt làm việc, sản xuất đúng một batch theo capacity hiện hành, lương và sổ hàng khớp, bán/mua/thuế khớp phép tính độc lập, invariant và save/load giữ nguyên. Đây là kiểm tra một chuỗi M5.2 bằng driver điều khiển; không chứng nhận NPC tự học cách chọn lệnh hoặc hoàn tất M5.
