# Bằng chứng M5 — cha mẹ, anh chị em và quỹ di sản công qua pha tử vong Realm

Ngày 2026-10-22. Kế hoạch: [các nhánh thừa kế dự phòng qua Realm](../design/M5-core-estate-parent-sibling-public-realm-plan.md). [IMPLEMENTATION_PLAN.md §2](../IMPLEMENTATION_PLAN.md) đặt thứ tự: con hợp pháp, phối ngẫu, cha mẹ, anh chị em; nếu không có người hưởng thì tiền vào quỹ di sản công và hàng về kho công ở vùng của người chết. Manager unit tests đã chọn đúng thứ tự; lát này bổ sung kiểm tra qua chính pha tử vong và quyết toán của Realm.

## Fixture và cách bảo toàn tài sản trong phép thử

Mỗi nhánh bắt đầu với một Realm riêng. Fixture chọn bốn người trưởng thành còn sống, không đến lượt mua thực phẩm trong ngày kiểm tra, không đứng tên holding và không sở hữu xưởng. Các quan hệ của người sẽ chết được làm sạch để không vô tình giữ lại vợ/chồng hay con mặc định từ thế giới khởi tạo. Người chết có **100 tiền cá nhân** và **5 Paper**; 100 tiền được chuyển từ Province firm, còn 5 Paper được ghi nhận như hàng đầu kỳ ở kho Province rồi chuyển từ kho sang người. Fixture kiểm invariant trước khi gọi pha tử vong, nên không tạo tiền/hàng ngoài sổ.

## Ba nhánh được chạy trong Realm

**Cha mẹ còn sống:** hai cha/mẹ có quan hệ ParentChild hợp pháp với người chết. Sau khi người chết được xử lý, mỗi cha/mẹ nhận 50 tiền. Năm Paper chia 3 cho ID thấp hơn và 2 cho ID cao hơn, theo quy tắc phân bổ phần dư đã có. Hồ sơ người chết không còn tiền; balance quỹ di sản trở lại số ban đầu; tổng tiền và invariant không đổi.

**Cha mẹ đã chết, còn anh/chị/em:** một người cha/mẹ chung đã chết, có quan hệ pháp lý được lưu với cả người chết và một người con khác. Không có con hoặc phối ngẫu còn sống của người chết. Người anh/chị/em nhận 100 tiền cùng 5 Paper; hồ sơ người chết không còn tiền, tổng tiền và invariant giữ nguyên.

**Không còn người hưởng hợp pháp:** fixture xóa các cạnh quan hệ thừa kế nối người chết với người khác. Sau pha tử vong, 100 tiền vẫn được ghi trong balance estate tổng hợp nhưng không còn là tiền mặt trong hồ sơ cá nhân đó; 5 Paper vào kho Province đúng vùng của người chết. Test save/load đối chiếu fingerprint.

Mỗi nhánh đều được serialize rồi load vào Realm mới; fingerprint, tổng tiền và invariant phải khớp. Đây là bài kiểm tra kết nối luật đã có, không thêm luật mới hay thay đổi resolver.

## Lượt sửa oracle

Lần chạy đầu dự đoán balance estate giảm 100 khi phân cho cha mẹ. Đối chiếu implementation cho thấy balance này bao gồm cả quỹ di sản công: khi tiền được chia cho người thân, hồ sơ cá nhân giảm tương ứng và tổng estate trở về mức trước lúc người chết; chỉ khi không có người hưởng, 100 được chuyển vào quỹ công và tiếp tục nằm trong balance estate. Oracle được sửa để so riêng hồ sơ người chết, người nhận và balance tổng hợp. Không có lỗi gameplay hoặc thay đổi production.

## Kiểm tra và giới hạn

- Windows Visual Studio Release: build `polyec_realm_tests` thành công; chạy trực tiếp đạt **20.707 checks**. Runner kèm profile A 10.000 người, 19.743 quan hệ thưa, 90 ngày; đây không phải profile B.
- Windows Visual Studio Debug: build target thành công; chạy trực tiếp đạt **20.707 checks**. Profile A giống Release. Toàn executable mất khoảng **144 giây** theo thời gian chờ tiến trình; phần profile 90 ngày báo 23,31 giây.
- Full core CTest Release/Debug **19/19** đã pass ở mốc ổn định trước những bổ sung test-only gần đây. Không lặp full suite cho lát này vì chỉ thêm Realm test/fixture. Chưa chạy adapter vì production, schema, API, binding và model không đổi.
- Tự review bằng phép tính chia 100 tiền và 5 Paper, đối chiếu vùng/kho, tổng tiền, balance estate, invariant và save/load. Chưa có reviewer độc lập.

Lát này kết nối ba hàng thừa kế dự phòng ở C++ Realm; không chứng minh policy learned tự chọn các quyết định, không hoàn tất toàn bộ luật tài sản sau chết và không đóng M5. [Manifest hash](../../artifacts/m5-core-estate-parent-sibling-public-realm-2026-10-22-manifest.json) ghi hash của mã test và tài liệu. Không commit, push hoặc publish.
