# M5.2 — kiểm hàng thay thế qua Realm và save/load

Ngày 2026-10-22. Plan: [Realm production no-substitution integration](../design/M5-realm-no-substitution-integration-plan.md). Luật đầu vào lấy từ [T2.4 Gate 4](../design/M5-T2.4-gate4-production-plan.md): recipe dùng exact `GoodId`, hàng khác cùng `GoodGroup` không tự thay thế.

## Chuỗi được chạy

Mỗi trường hợp khởi tạo Realm hai vùng với actor 129. Một xưởng đúng ngành được tạo qua `EstablishmentCommand`; worker được cấp quyền; `ScheduleCommand` nhận 32 tick Work tại đúng vùng và xưởng. Vì chưa có lệnh giao hàng tới kho xưởng, test không giả làm giao dịch gameplay: nó thêm hàng đầu vào thử trực tiếp vào bản save kiểm thử và tăng cùng số lượng ở opening goods ledger để giữ invariant trước khi tải lại.

Sau khi tải fixture, Realm chạy đủ 64 game tick để qua ranh giới ngày và pha sản xuất. Đã chạy năm trường hợp:

| Công thức xưởng | Hàng thay thế được để trong kho | Đầu vào phụ trợ | Kết quả sau ngày mô phỏng |
|---|---|---|---|
| Fiber → Cloth | 1 Beeswax | Không cần | Không có batch; Beeswax còn 1, Cloth vẫn 0 |
| Iron + Fuel → Tools | 1 Copper | 1 Fuel | Không có batch; Copper và Fuel đều còn 1, Tools vẫn 0 |
| Iron + Fuel → Tools | 1 Silver | 1 Fuel | Không có batch; Silver và Fuel đều còn 1, Tools vẫn 0 |
| Iron + Fuel → Tools | 1 Gold | 1 Fuel | Không có batch; Gold và Fuel đều còn 1, Tools vẫn 0 |
| Tools + Cloth → MilitarySupply | 1 Silk | 1 Tools | Không có batch; Silk và Tools đều còn 1, MilitarySupply vẫn 0 |

Trong cả năm trường hợp, Realm ghi nhận đủ hai work unit từ ca 32 tick và đóng dấu production day. Không có đầu vào bị tiêu thụ để tạo output không hợp lệ. Tổng tiền, invariant hàng hóa, save/load và fingerprint đều đúng; tải lại giữ cùng stock và watermark.

## Kiểm tra đã chạy

- Windows Visual Studio Release: build riêng `polyec_realm_tests` thành công.
- Chạy trực tiếp `build/m5-core/core/Release/polyec_realm_tests.exe`: **20.799 checks** pass. Runner kèm profile A 10.000 người/19.743 quan hệ thưa/90 ngày; đây không phải profile B.
- `py -3.12 scripts/check-repository-links.py`: **1.172 tham chiếu nội bộ** pass.
- Kiểm khoảng trắng giới hạn vào tệp của lát này pass.
- Không chạy full CTest, Debug, Python/native/GDExtension/Godot. Chỉ thêm fixture/test; không sửa production, API, binding, schema, action, observation, reward hoặc model.

## Giới hạn

Hàng thử được nạp trong serialized fixture để kiểm recipe; gameplay hiện chưa có transfer command tổng quát đưa mọi GoodId vào xưởng. Kết quả chứng minh ba recipe hiện hành từ chối năm loại thay thế này qua Realm và bản lưu; không chứng minh nguồn cung/market/producer cho sản phẩm deferred hoặc hoàn tất M5.2. Tự review; chưa có reviewer độc lập. Không commit, push hoặc publish.
