# Bằng chứng M5 — giữ hoặc chuyển title đất theo các nhánh thừa kế dự phòng

Ngày 2026-10-22. Plan: [title đất theo hàng thừa kế dự phòng](../design/M5-core-estate-fallback-land-realm-plan.md). Quy tắc title trong [plan estate land-title](../design/M5-core-estate-land-title-plan.md) chỉ chuyển holding khi khoản nợ đã được trả và có đúng một người hưởng hợp pháp. Nhiều người cùng hàng hoặc không có người hưởng thì holding và giấy phép sử dụng tiếp tục nằm trong hồ sơ; không chia đất hoặc tự chuyển đất cho quỹ công.

## Kết quả theo ba tình huống

Realm C++ tạo một title có ID ổn định và một giấy phép sử dụng đang hoạt động cho mỗi fixture. Mỗi giấy phép ghi rõ người được dùng holding; trước và sau quyết toán, test so sánh bản ghi theo ID. Không có khoản vay trong các fixture để cô lập việc chọn người hưởng.

- **Hai cha/mẹ cùng hàng còn sống:** mỗi người được hưởng tiền/hàng theo luật chia đều, nhưng title không thể tách thành hai. Người chết tiếp tục được ghi làm title-holder trong hồ sơ; giấy phép sử dụng giữ nguyên người cấp và người sử dụng; cả hai ID vẫn nằm trong EstateCase.
- **Một anh/chị/em trưởng thành là người hưởng duy nhất:** holding chuyển sang người đó. Giấy phép hiện có vẫn cho cùng người sử dụng, còn trường grantor đổi sang người thừa kế mới; hai ID được bỏ khỏi danh sách tài sản còn nằm trong hồ sơ.
- **Không còn người hưởng hợp pháp:** title tiếp tục đứng tên người chết và được hồ sơ tham chiếu. Giấy phép giữ nguyên grantor/grantee, không tự đổi chủ sang quỹ di sản công.

Mỗi tình huống save/load sang Realm mới, rồi so fingerprint, toàn bộ danh sách title/quyền dùng, hồ sơ estate và invariant. Tổng tiền giữ nguyên. Ba nhánh tiền/hàng song song đã được kiểm tra trong [report trước](M5-core-estate-parent-sibling-public-realm-evidence-2026-10-22.md).

## Fixture và kiểm tra

Fixture mượn cùng ba mối quan hệ đã tạo cho kiểm tra cash/hàng; thêm holding area 1 tại region 1, title do người sẽ chết nắm và quyền sử dụng cấp cho một người còn sống. Việc thêm title/quyền chỉ sửa Realm fixture, dùng stable ID từ bộ cấp ID chung. Test gọi pha người của Realm để xử lý tử vong, không gọi thẳng EstateManager.

- Windows Visual Studio Release: build `polyec_realm_tests` pass; chạy trực tiếp đạt **20.716 checks**. Runner kèm profile A 10.000 người, 19.743 quan hệ thưa, 90 ngày; không phải profile B.
- Windows Visual Studio Debug: build thành công; chạy trực tiếp đạt **20.716 checks**. Profile A giống Release. Toàn executable mất khoảng **219 giây** theo thời gian chờ tiến trình; phần profile 90 ngày báo 21,24 giây.
- Full CTest core Release/Debug 19/19 có trước những bổ sung test-only gần đây; lát này chỉ thêm Realm fixture/test, không chạy lại full suite hoặc Python/native/GDExt/Godot vì không đổi production, schema, API hay binding.
- Tự review đối chiếu holding ID, title-holder, right ID, grantor, grantee, EstateCase, tổng tiền và save/load. Không có reviewer độc lập.

Lát này chỉ xác nhận phần đất trong các nhánh thừa kế qua Realm; không định giá holding, không giải quyết cách chia đất cho nhiều người, và không đóng toàn bộ M5. [Manifest](../../artifacts/m5-core-estate-fallback-land-realm-2026-10-22-manifest.json) ghi hash của mã test và tài liệu. Không commit, push hoặc publish.
