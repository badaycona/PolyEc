# M5 v25 — audit dòng tiền theo quyết định Economic

## Câu hỏi và cách đo

Các checkpoint Economic v23 tạo khoảng 22 xưởng mỗi world, nhưng sản lượng làm thuê tại xưởng thấp và người chơi đại diện có ít tiền, nhiều nợ hơn baseline. Audit này chạy lại ba checkpoint trên cùng 20 seed validation, mỗi world 128 ngày, với C++ Realm shared-core. Các con số bên dưới gộp quyết định của toàn bộ nhân vật Economic trong 60 lượt world (3 checkpoint ×20 world).

Mỗi lệnh hợp lệ được đo tiền mặt và nợ của nhân vật ngay trước/sau `world.step`. Số tiền là biến động tiền riêng của actor, không gồm phần chi được lấy từ quỹ hộ gia đình; khoản lãi và nhu cầu sống hằng ngày, lương, sản xuất xảy ra giữa các lượt quyết định không bị gán cho một Intent.

## Kết quả

| Lệnh đã chọn | Số lần | Thay đổi tiền mặt ngay tại lệnh | Thay đổi nợ ngay tại lệnh |
|---|---:|---:|---:|
| Đi lại | 1.482 | −11.604 | 0 |
| Tạo xưởng | 1.365 | −72.950 | 0 |
| Mua hàng | 622 | −72.160 | 0 |
| Kết hôn | 511 | −25.550 | 0 |
| Vay | 423 | +42.300 | +43.992 |
| Trả nợ | 32 | −2.720 | −2.720 |
| Bán hàng | 230 | +30.736 | 0 |
| Thuê | 198 | −3.960 | 0 |
| Chuyển hàng giữa kho và xưởng | 30 | 0 | 0 |
| Cấp quyền làm tại xưởng | 181 | 0 | 0 |
| Gửi Work tại xưởng | 144 | 0 | 0 |

Trong lệnh mua, Fiber được mua 139 lần với tổng biến động tiền actor −7.628; Iron 73 lần (−13.180), Tools 56 lần (−13.112), Fuel 144 lần (−10.948), Cloth 70 lần (−9.204), DryFood 103 lần (−8.876), Horse 37 lần (−9.212). 1.365 lần tạo xưởng gồm 464 MilitarySupply, 451 Tool và 450 Cloth. Cuối world, số xưởng trung bình 22,75, paid site work units 9,60, Fiber tồn xưởng 0,15 và Cloth tồn xưởng 0,08.

Số lệnh cho thấy chính sách thường dùng vốn vào đi lại/mở cơ sở/mua sắm và một số giao dịch xã hội; ít khi đưa hàng vào xưởng hoặc giao Work. Mô hình cũng vay nhiều hơn hẳn số lần trả: 423 lệnh Borrow so với 32 Repay. Đây là lời giải thích có bằng chứng cho chu trình sản xuất yếu trong các checkpoint đang thử, không phải kết luận về toàn bộ model hoặc mọi kịch bản.

## Kết luận và giới hạn

Nguyên nhân quan sát được nằm ở lựa chọn hành động và thứ tự ưu tiên vốn: nhiều lần mở xưởng trên ba ngành, mua vật tư/hàng khác nhau, đi lại, cùng vay thường xuyên; rất ít lệnh chuyển hàng và gửi Work so với lượng xưởng được tạo. Chưa đủ dữ liệu để xác định việc mua nào là đầu cơ hay cần thiết, hoặc phân bổ đầy đủ mọi thay đổi tiền/nợ theo ngày. Không thay reward/action/gameplay, không huấn luyện lại, không chọn hoặc nâng cấp model. Tất cả 60 lượt giữ invariant đúng.

Codex tự review, chưa có reviewer độc lập. Kiểm tra Python compile và evaluator adapter 28/28 pass.

- Kế hoạch: [M5 v25 flow audit](../design/M5-v25-economic-decision-flow-audit-plan.md)
- Dữ liệu chi tiết: [audit.json](../../build/m5-v25-economic-flow-audit-final/audit.json)
- Manifest/hash: [m5-v25 manifest](../../artifacts/m5-v25-economic-decision-flow-2026-10-21-manifest.json)
