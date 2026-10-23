# M5.0/1 — di sản của người chết không nhập quỹ hộ của người phối ngẫu

## Mục tiêu và căn cứ

Nối ba luật hiện hành qua một pha tử vong Realm: khoản vay đủ tiền được trả từ hồ sơ của người chết; tiền riêng còn lại chuyển cho người hưởng ưu tiên; quỹ hộ là tài khoản chung, không thuộc di sản cá nhân. Hai người phối ngẫu trong fixture ở hai household label khác nhau để mỗi bên có một quỹ độc lập.

## Phạm vi

- Chỉ mở rộng fixture và test `test_estate_keeps_household_fund_outside_personal_claim()` trong `core/tests/test_realm.cpp`.
- Nạp 37 vào quỹ hộ của người chết và 23 vào quỹ của người phối ngẫu bằng điều chỉnh fixture có đối ứng với Province firm. Cấp một DryFood từ kho Province cho người hưởng để nhu cầu ăn trong ngày không làm thay đổi quỹ trong lúc kiểm đúng riêng phần di sản.
- Sau death phase, kiểm khoản vay, số tiền cá nhân người hưởng nhận, hai số dư quỹ, tổng tiền, invariant và save/load.
- Không sửa Realm production, luật người hưởng, điều kiện trả nợ, quyền rút quỹ hoặc schema.

## Kiểm tra ngay

Build target `polyec_realm_tests` Release và chạy trực tiếp executable, vì đây là test Realm nối death phase, loan, người hưởng, quỹ hộ và save/load. Không chạy full CTest hoặc adapter cho thay đổi chỉ ở fixture/test. Nhóm M5.0/1 estate sẽ cần mốc tích hợp rộng hơn sau khi các lát ownership được nối.

## Kết quả chạy và hiệu chỉnh fixture

Lần chạy đầu thất bại vì người phối ngẫu đến lượt ăn trong ngày và Realm đã dùng tiền quỹ hộ để mua DryFood theo luật hiện có. Đây là hành vi hợp lệ, không phải lỗi chia di sản. Fixture được sửa bằng cách chuyển một DryFood có sẵn từ kho Province vào inventory người phối ngẫu; sau đó phép so sánh hai số dư quỹ trở nên độc lập với chi phí ăn trong ngày.

## Giới hạn

Kết quả chỉ chứng minh hai quỹ cụ thể được giữ ngoài tiền cá nhân khi phối ngẫu duy nhất nhận di sản. Không thêm quyền quản lý tài khoản của hộ khác, không thay luật đồng sở hữu đất/xưởng và không khẳng định M5.0/1 đã hoàn tất. Tự review; không có reviewer độc lập.
