# Bằng chứng M5 — giữ nguyên tài sản của những người còn sống

Ngày 2026-10-22. Kế hoạch: [kiểm tra tài sản người sống khi xử lý di sản](../design/M5-core-estate-living-property-isolation-plan.md). Lát này bổ sung một phép thử Realm C++ cho nguyên tắc đã ghi trong thiết kế sổ di sản: xử lý tài sản của người chết không được sửa tài sản của người khác. Không thay đổi luật chơi, mã xử lý sản xuất, định dạng save, API hay binding.

## Cách dựng tình huống và kết quả mong đợi

- Fixture dùng một người chết có khoản vay, title, quyền sử dụng và xưởng riêng như các test di sản trước. Nó bổ sung một title đứng tên một người sống khác, một quyền sử dụng do người đó cấp cho người thứ ba, và một xưởng vải do người đó sở hữu. Tiền để lập xưởng được chuyển giữa cash cá nhân và Province firm, nên fixture không tạo tiền.
- Test chụp toàn bộ title không đứng tên người chết, mọi quyền sử dụng không do người chết cấp, và mọi xưởng không thuộc người chết. Sau khi Realm chạy pha xử lý tử vong, nó tìm lại từng bản ghi bằng ID và so sánh trường dữ liệu.
- Với title, so sánh ID, vùng, diện tích và người đứng tên. Với quyền sử dụng, so sánh ID, holding, người được cấp và người cấp. Với xưởng, so sánh chủ sở hữu, vùng, ngành, công suất, tiền, hàng, tiến độ sản xuất, công đã ghi, cửa sổ ủy quyền, quyền làm việc và sổ trả công.
- Oracle còn kiểm tra tổng tiền, invariant của Realm, rồi serialize/deserialize để xác nhận fingerprint và invariant vẫn đúng.

## Kiểm tra đã chạy

- Windows Visual Studio Release: build target `polyec_realm_tests` thành công.
- Chạy trực tiếp `build/m5-core/core/Release/polyec_realm_tests.exe`: **20.680 checks passed**. Runner kèm profile A gồm 10.000 người, 19.743 quan hệ thưa và 90 ngày. Đây không phải profile B.
- Lượt chạy đầu dừng tại điều kiện chuẩn bị fixture vì fixture gốc chỉ có tài sản của người chết, chưa có tài sản riêng của người sống để đối chiếu. Bổ sung title/quyền/xưởng cho người còn sống rồi build và chạy lại; lượt cuối qua toàn bộ 20.680 checks. Lỗi đầu là dữ liệu kiểm thử thiếu, không phải lỗi xử lý di sản.
- Sau test này, chạy lại toàn bộ core CTest Release: **19/19 pass trong 38,54 giây**. Build Realm Debug với fixture hiện tại rồi chạy toàn bộ core CTest Debug: **19/19 pass trong 391,31 giây**. Vì chỉ đổi test/fixture, không chạy Python replay, native contract, GDExtension hoặc Godot; không dùng kết quả adapter cũ để chứng nhận một UI gameplay mới.
- Codex tự xem lại thay đổi; không có review độc lập.

Test này không quy định cách chia đất/xưởng khi có nhiều người hưởng, không trao quyền tài sản cho guardian và không làm M5 hoàn tất. Nó chỉ chứng minh các bản ghi thuộc người sống khác không bị pha di sản này sửa đổi.

[Bản kê hash](../../artifacts/m5-core-estate-living-property-isolation-2026-10-22-manifest.json) ghi các file của lát test tại thời điểm tạo bằng chứng. Không commit, push hoặc publish.
