# M5.2 — nối ba công thức khởi đầu qua Realm

Ngày 2026-10-22. Hợp đồng công thức ở [IMPLEMENTATION_PLAN §15.3](../IMPLEMENTATION_PLAN.md#153-sản-xuất-và-nhu-cầu); danh mục hàng ở §15.2.

## Mục tiêu

Realm đã có kiểm tra sản xuất qua ranh giới ngày, save/load giữa ca và chống tạo hàng lặp cho công thức Fiber → Cloth. Lát này bổ sung cùng mức kiểm tra cho hai công thức đã được quy định:

- 1 Iron + 1 Fuel → 1 Tools.
- 1 Tools + 1 Cloth → 1 MilitarySupply.

Mỗi batch lấy đúng một đơn vị của từng đầu vào và tạo một đơn vị đầu ra. Chỉ kiểm tra ba công thức khởi đầu; không thêm nguồn tự nhiên, mùa vụ, giá, producer mới hoặc công thức khác.

## Thay đổi và quyền sở hữu

- Chỉ sửa `core/tests/test_realm.cpp`.
- Test tạo xưởng đúng ngành thuộc về người làm, cấp quyền làm việc, gửi Work qua ScheduleCommand và nạp nguyên liệu bằng fixture save có ledger Province cân bằng.
- Realm vẫn sở hữu quyết định lệnh, thời gian, công đã xử lý, kho xưởng, lưu/tải và kiểm tra toàn thế giới. Fixture chỉ cung cấp nguyên liệu ban đầu để không giả lập nguồn cung.
- Không đổi gameplay, schema, public API, binding, observation/action/reward hoặc model contract.

## Điều kiện thành công và từ chối

- Chạy 64 tick để hoàn tất ngày sản xuất; đối chiếu với bản sao lưu sau 16 tick rồi tiếp tục 48 tick. Fingerprint cuối phải trùng.
- Xưởng tiêu thụ đúng hai hàng đầu vào, tạo đúng một đầu ra; có hai work unit đã xử lý, watermark ngày sản xuất đúng và invariant toàn Realm đúng.
- Save/load sau sản xuất phải byte-for-byte ổn định và giữ đầu ra.
- Chủ xưởng rút đầu ra vào kho cá nhân rồi bán qua Realm tại cùng vùng. Giá thu về phải khớp quy tắc đang có: tối đa một đồng chiết khấu so với giá tham chiếu, tối thiểu một đồng. Hàng chuyển vào chợ, tiền chuyển từ Province firm sang người bán và tổng tiền không đổi.
- Nếu chỉ có Iron mà thiếu Fuel cho recipe Tools, xưởng không tiêu Iron một phần và không tạo Tools. Work unit vẫn được đánh dấu đã xét theo quy tắc shortage hiện hành; invariant Realm vẫn đúng.
- Trường hợp không tạo được xưởng hoặc fixture không tải được làm test thất bại ngay. Không thay đổi quy tắc từ chối gameplay.

## Kiểm tra

- Chạy ngay: Windows Release build target `polyec_realm_tests` và chạy executable trực tiếp; đây là target chứa pipeline command → Schedule/Work → day settlement → production → save/load → rút hàng → bán tại chợ.
- Để mốc tích hợp M5.2: full CTest/Debug và Python/native/Godot, vì chỉ thêm test C++ và không đổi public surface. Chạy lại khi đóng M5.2 cùng các test khác bị ảnh hưởng ở thờiนั้น.

## Giới hạn

Kiểm tra này chứng minh Realm thi hành hai công thức đã nêu với đầu vào được cấp sẵn và tiếp tục đúng sau save/load. Nó không chứng minh các hàng được sản xuất tự nhiên, mua bán có chiều sâu, hay toàn bộ 16 nhóm có nguồn cung. Những phần đó cần contract producer/market riêng.
