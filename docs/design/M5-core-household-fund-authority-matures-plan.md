# Kế hoạch M5 — quyền rút quỹ hộ mở lại khi thành viên còn sống đủ tuổi trưởng thành

Ngày 2026-10-22. Contract nguồn: tài khoản quỹ hộ chỉ cho `household_authority` hiện tại rút; authority là người sáng lập còn sống và đủ 18 tuổi, nếu không thì thành viên trưởng thành lớn tuổi nhất còn sống, hoặc 0 khi không có người lớn. Dùng chung command Realm hiện có; không trao quyền guardian rút tiền riêng hoặc quyết định thay người phụ thuộc.

Lát trước đã xác nhận quỹ giữ nguyên và lệnh của trẻ bị từ chối khi người duy nhất còn sống chưa đủ tuổi. Lát này kiểm nối tiếp: người đó đạt đúng 18 game years sau thời gian chạy Realm, trở thành authority và có thể gửi lệnh rút hợp lệ.

## Hành vi và phép tính

1. Tạo hộ có một chủ hộ trưởng thành và một người con ở tuổi `18 * 365 - 3` ngày; chuyển 37 tiền từ Province firm vào quỹ hộ và 2 DryFood từ kho sang hành trang người con.
2. Chạy pha người để chủ hộ chết. Người con tăng một ngày nhưng vẫn dưới 18; authority là 0 và quỹ 37 vẫn bị khóa.
3. Chạy Realm đúng hai game days. Người con chạm tuổi `18 * 365`; authority query tự trả ID người đó, còn quỹ không bị dùng lén cho việc khác.
4. Người vừa trưởng thành rút 5 qua `HouseholdFundCommand`: cash cá nhân +5, quỹ 32, tổng tiền/invariant đúng.
5. Save/load giữ tuổi, authority, cash, quỹ và fingerprint.

Fixture tránh ngày mua thực phẩm bằng cách cho người con giữ hai món DryFood có nguồn từ Province và chọn ID không đến lượt ăn trong ba ngày kiểm tra. Như vậy oracle riêng cho lệnh rút không lẫn với khoản tiêu dùng thiết yếu.

## Phạm vi và kiểm tra

- Chỉ fixture/test `core/tests/test_realm.cpp` và tài liệu/mốc bằng chứng.
- Build và chạy trực tiếp Realm target Windows Release rồi Debug. Không chạy full CTest hoặc adapters: không đổi production, schema, API, binding hay model; full CTest core đã có ở mốc ổn định trước nhóm test-only gần đây.
- Tự review bằng ranh giới ngày ngay trước/đúng 18 tuổi, số dư trước/sau, tổng tiền và save/load; không có reviewer độc lập.
