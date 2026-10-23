# Bằng chứng M5 — quỹ hộ không thể bị trẻ vị thành niên rút sau khi người lớn qua đời

Ngày 2026-10-22. Kế hoạch: [kiểm tra quỹ hộ không có người lớn có thẩm quyền](../design/M5-core-estate-household-minor-lock-plan.md). Test C++ kiểm tra tình huống tài khoản chung còn tiền nhưng không còn người lớn sống trong hộ.

## Tình huống và kết quả

Fixture ban đầu có một chủ hộ 40 tuổi và một người con 10 tuổi. Người con là thành viên duy nhất còn lại trong hộ; các cư dân khác được tách sang hộ riêng để không ảnh hưởng kết quả. Quan hệ cha/mẹ-con hợp pháp được lưu trong Realm. Trước khi bắt đầu, **37 tiền** được chuyển từ Province firm vào quỹ hộ; 1.000 tiền cá nhân của chủ hộ được chuyển từ Province firm sang người đó. Các chuyển khoản này giữ nguyên tổng tiền trong thế giới.

Realm chạy pha cư dân, làm người chủ hộ chết. Người con sống sót vẫn chưa đủ 18 tuổi và nhận tiền cá nhân qua hồ sơ di sản theo quy tắc đã có. Realm không đưa quỹ hộ vào hồ sơ di sản: hồ sơ có 0 tiền mặt sau khi phân phối, quỹ hộ vẫn có 37, tổng tiền và sổ cái vẫn cân bằng. Vì không có người lớn sống trong hộ, truy vấn người được quyền rút trả về 0.

Người con thử gửi lệnh rút 5 tiền. Realm từ chối lệnh; ảnh chụp trạng thái, tiền cá nhân của người con, quỹ 37 và tổng tiền đều không đổi. Save/load tiếp tục giữ đúng quỹ hộ, không có người có quyền rút, hồ sơ di sản, tiền cá nhân và fingerprint.

## Fixture và giới hạn của kết luận

Test chọn một cặp thành viên từ household tạo sẵn, tách những người còn lại sang household riêng, đặt tuổi và quan hệ hợp pháp cho đúng tình huống, rồi chuyển tiền thật giữa Province firm và household fund. Một người có thể đến lượt xử lý nhu cầu ăn trong ngày, vì vậy fixture chọn người con không đến lượt ở ngày kiểm tra; việc đó tránh làm quỹ thay đổi do một cơ chế khác trước khi kiểm tra lệnh rút. Fixture không sửa quy tắc gameplay.

Kết quả chứng minh rằng người chưa thành niên không thể tự rút quỹ chung khi không có người lớn còn sống được Realm công nhận. Nó không thiết kế người giám hộ tài chính, khoản trợ cấp, thanh toán thay trẻ hay quy tắc đóng hộ. Những quyền đó cần contract riêng; không được suy ra từ quyền quản lý đất của guardian.

## Kiểm tra đã chạy

- Windows Visual Studio Release: build target `polyec_realm_tests` thành công; chạy `build/m5-core/core/Release/polyec_realm_tests.exe` đạt **20.695 checks**. Runner kèm profile A gồm 10.000 người, 19.743 quan hệ thưa và 90 ngày; đây không phải profile B.
- Windows Visual Studio Debug: build cùng target thành công; chạy `build/m5-core/core/Debug/polyec_realm_tests.exe` đạt **20.695 checks**. Profile A giống Release.
- Full CTest core Release **19/19** và Debug **19/19** đã pass trước lần thêm fixture này. Vì thay đổi chỉ bổ sung test C++, sau đó chạy trực tiếp Realm target ở cả hai cấu hình; các target khác không đổi.
- Không chạy lại Python/native/GDExt/Godot: không đổi mã sản phẩm, định dạng save, API, binding hoặc dữ liệu model.
- Tự review bằng các phép đối chiếu độc lập trong test: tiền chuyển vào có nguồn, tổng tiền trước/sau, quyền rút được truy vấn lại từ Realm, lệnh bị từ chối phải giữ nguyên fingerprint, và save/load phải tạo lại fingerprint. Không có reviewer độc lập.

Đây là một trường hợp bổ sung trong nhóm estate/quỹ hộ, không đóng toàn bộ M5. [Manifest hash](../../artifacts/m5-core-estate-household-minor-lock-2026-10-22-manifest.json) ghi các file thuộc lát này. Không commit, push hoặc publish.
