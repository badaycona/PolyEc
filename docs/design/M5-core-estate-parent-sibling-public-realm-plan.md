# Kế hoạch M5 — các nhánh cha mẹ, anh chị em và quỹ di sản công qua Realm

Ngày 2026-10-22. Nguồn luật: [IMPLEMENTATION_PLAN.md §2](../IMPLEMENTATION_PLAN.md), mục “Tài sản sau chết”. Luật yêu cầu thanh toán nghĩa vụ từ di sản trước, rồi chia phần còn lại cho hàng thân nhân đầu tiên còn sống theo thứ tự: con hợp pháp, phối ngẫu, cha mẹ, anh chị em. Nếu không có người hưởng, tiền vào quỹ di sản công và hàng về kho công tại vùng của người chết.

Manager unit tests đã kiểm tra việc chọn các hàng thân nhân. Lát này chỉ xác nhận Realm death phase thực sự phân phối cho cha/mẹ, rơi tiếp xuống anh chị em khi cha/mẹ đã chết, và chuyển về tài khoản/kho công nếu không còn người hưởng.

## Phạm vi và phép kiểm

- Chỉ thêm fixture và Realm integration test trong `core/tests/test_realm.cpp`, cùng report/CURRENT_TASK/HISTORY/manifest.
- Mỗi trường hợp dùng người chết có 100 tiền cá nhân và 5 Fiber chuyển từ Province firm/kho vùng để không tạo tài sản mới. Người được đặt trong quan hệ thân nhân hợp pháp. Không tạo nợ; khoản nợ và các hàng ưu tiên cao hơn đã có test Realm riêng.
- Trường hợp cha mẹ: hai cha/mẹ còn sống nhận chia đều; đơn vị tiền/hàng dư theo ID nhỏ hơn, như quy định trong estate manager.
- Trường hợp anh chị em: cha/mẹ chung đã chết, anh/chị/em còn sống nhận di sản; không có con hoặc phối ngẫu còn sống của người chết.
- Trường hợp không có người hưởng: không có quan hệ thừa kế hợp pháp còn sống; tiền được ghi vào balance quỹ di sản công và Fiber trở lại kho Province đúng vùng.
- Mỗi nhánh kiểm tra mất trạng thái của người chết, tổng tiền, hàng, Realm invariant và save/load. Không sửa production, luật, save schema hoặc binding.

## Gate

- Build và chạy trực tiếp `polyec_realm_tests` Windows Release và Debug; mỗi cấu hình đạt **20.707 checks**, profile A 10.000 người/19.743 quan hệ thưa/90 ngày.
- Không chạy full CTest hay adapter: chỉ thêm test C++ Realm, không đổi implementation, API, schema hoặc binding. CTest full hai cấu hình của core đã có ở mốc ổn định trước; ghi rõ khoảng cách revision.
- Tự review bằng đối chiếu số dư trước/sau và save fingerprint; không có reviewer độc lập.
