# M5.2 — expose Realm market-order commands to Python, native runtime and GDExtension

Ngày: 2026-10-22  
Trạng thái: phần adapter đã được viết; Python và GDExtension đã build, kiểm tra C++/Python pass. Native runtime success/cancel đã có test nhưng chưa chạy được vì bundle model hiện có bị từ chối theo contract; runtime test chỉ xác nhận nhánh Realm chưa nạp. Chi tiết: [báo cáo](../reports/M5-market-order-python-native-adapter-evidence-2026-10-22.md).

## Lý do và phạm vi

C++ Realm đã có `MarketOrderCommand` với hai thao tác Place/Cancel, hàng giữ tại vùng, số lượng, giá trần, giữ tiền cá nhân/quỹ hộ/tổ chức, quyết toán ngày sau và giá theo tháng. Tests C++ kiểm toán các nhánh này. Python `Realm` binding chưa có wrapper cho command hoặc query các lệnh đang mở, nên trainer không gọi được cùng transaction.

Nhóm này đưa command và trạng thái cần đọc qua Python và đường gọi native runtime để GDExtension có thể gọi. Không chuyển công thức, cấp tiền, phân bổ hàng, thuế, thời điểm quyết toán hoặc điều kiện actor sang adapter. Không đổi C++ gameplay, model observation/reward/action hoặc save schema. Python xuất `MarketOrderOperation`, wrapper trả `(success, reason, order_id)`, query các trường lệnh và số dư đang giữ. Native runtime dùng sequence hiện tại của actor, hỗ trợ đối chiếu fingerprint trước khi gửi, trả ID lệnh và lý do C++ từ chối. GDExtension trả kết quả dạng Dictionary và đưa danh sách lệnh/tiền giữ vào snapshot.

## File và phép kiểm

- `bindings/python/realm_binding.cpp`: enum, wrapper `market_order_command`, `market_orders` và `market_escrow`.
- `ai/tests/market_order_binding_checks.py`: Realm mua thật qua binding, kiểm tiền bị giữ, query đúng funding/source, stale sequence, cancel/refund, lưu/nạp và từ chối tham số sai.
- `native/include/polyec/realm_runtime.hpp`, `native/realm_runtime.cpp`: thêm lệnh đặt/hủy dùng cùng `Realm::execute`, sequence tăng đúng một, từ chối Realm chưa nạp, actor ngoài miền, fingerprint cũ và sequence tràn.
- `native/realm_contracts.cpp`: kiểm Realm chưa nạp; có fixture đặt/hủy/stale cho nhánh model tương thích. Bài chạy hiện hành dừng trước fixture này do chỉ có model v6 lịch sử, không tương thích Realm hiện tại.
- `gdext/polyec_session.cpp`: thêm method `realm_market_order` và các dòng đơn mua/escrow vào snapshot.
- `docs/IMPLEMENTATION_PLAN.md`, `docs/CURRENT_TASK.md`, HISTORY/report: ghi kết quả sau kiểm.

## Ranh giới

Build target Python `polyec_env` Release và chạy riêng binding test; chạy C++ `polyec_market_order_tests`; build native `polyec_realm_contracts` và chạy duy nhất `runtime_contracts`; build GDExtension target `polyec`. Không chạy ALL_BUILD/full CTest. Máy không có Godot runtime để mở project/chạy UI. Native successful order fixture nằm sau kiểm tra bundle; hiện chỉ model v6 bị từ chối, nên không tuyên bố đặt/hủy đã chạy qua native runtime. UI màn hình mua vẫn là bước sau.

Đóng phần triển khai adapter khi Python/GDExtension chỉ gọi command C++ đã có, success/reject/cancel/save đúng ở C++/Python, tiền giữ/hoàn khớp Realm và không có luật kinh tế riêng. Native success/cancel và UI còn chờ model tương thích/runtime Godot; đây không chứng minh learned policy đã học đặt lệnh. Chưa train RL.
