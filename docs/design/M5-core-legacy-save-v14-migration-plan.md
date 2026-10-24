# Kế hoạch và kết quả: di trú save lịch sử v14 — `legacy-save-v14-migration-v1`

Cập nhật 2026-10-25. Nhóm đã hoàn tất. Báo cáo đầy đủ về nguyên nhân, thay đổi, từng kiểm tra và giới hạn: [M5 core legacy save v14 migration evidence](../reports/M5-core-legacy-save-v14-migration-evidence-2026-10-25.md).

## Mục tiêu đã đạt

Đọc save v14 cũ mà không bỏ invariant, không sửa fixture/hash lịch sử, vẫn từ chối corruption nguyên tử và dựng các phần mặc định được thêm sau schema v14.

## Kết quả điều tra và quyết định

Fixture của `gameplay_config_tests` được chuyển về grammar v15 rồi bỏ `CONFIG_1`, phần v15 không tồn tại trong v14, đồng thời hoàn lại bộ cấp ID trước khi có land title. Hash của oracle lịch sử độc lập được giữ nguyên.

Reader chỉ khởi tạo sức chứa nhà công và hội đồng khi xử lý phần đuôi cấu hình từ v15 trở lên. Save v14 không có phần này, vì vậy nhánh đó bị bỏ qua và mảng nhà công vẫn có kích thước từ Realm tạm một vùng; Atlas còn thiếu danh sách hồ sơ hội đồng. Sửa code reader để mọi schema trước v15 đều tạo sức chứa mặc định theo số vùng đã nạp và tạo hồ sơ hội đồng trống cho 27 settlement Atlas khi cần.

## Kiểm tra đã chạy

- Build Release target `polyec_gameplay_config_tests` và chạy trực tiếp: **51/51 checks pass**.
- CTest Release, `gameplay_config_tests` và `work_credit_tests`: **2/2 pass**.
- Kiểm mới nạp save v14 cho Realm hai vùng và Atlas27; Atlas có đúng27 hồ sơ hội đồng trống và cả hai Realm qua invariant.
- Build và chạy `polyec_realm_tests`: runner dừng sau434 checks tại phép so byte của oracle save lịch sử v14. Hash/fixture không bị sửa. Các ca phía sau điểm dừng không được tính là đã chạy.
- Không đổi binding/API hoặc giao diện tiêu thụ save, nên Python/GDExtension/Godot và package được để cho mốc tích hợp; không chạy full CTest, performance, package hoặc RL.
- `git diff --check` pass trước commit.

## Lần thử chưa thành công được giữ xuất xứ

Lần đầu nhóm được mở vì target gameplay-config báo `Corrupt save/invariant` ở save v14. Trong quá trình khoanh vùng, một diagnostic riêng cũng bắt được save cố ý đặt phần lương lẻ bằng toàn bộ giá công; đó là reject đúng theo test corruption, không phải nguyên nhân của ca v14. Không giữ instrumentation chẩn đoán trong code.

## Trạng thái sau nhóm

Đã đóng trên bằng chứng mục tiêu của C++ config/save. Realm runner rộng vẫn có điểm dừng historical-byte oracle như được mô tả trong report và giữ nguyên để điều tra riêng nếu cần. Tiếp theo chọn một hàng hóa GoodId còn thiếu nguồn/consumer theo roadmap M5.2; sau khi đóng nhóm hàng hóa còn lại sẽ tiếp tục M5.3/M5.4. Chưa chạy hoặc tạo dữ liệu RL.
