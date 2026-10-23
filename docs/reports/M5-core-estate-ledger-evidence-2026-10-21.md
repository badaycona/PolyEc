# Core — ghi hồ sơ tài sản khi nhân vật qua đời

Khi nhân vật chết trong nhịp ngày, Realm chuyển số tiền mặt và từng lượng hàng còn ở người đó vào hồ sơ theo ID decedent. Hồ sơ giữ ngày/vùng qua đời và liên kết tới xưởng cá nhân, các thửa đất đứng tên, quyền sử dụng đất do decedent cấp và khoản vay còn nghĩa vụ. Các quyền sở hữu và khoản vay vẫn nằm trong ledger gốc, hồ sơ lưu ID để quyết toán bước sau. Quỹ hộ gia đình không nhập vào hồ sơ.

Save schema24 ghi các hồ sơ theo thứ tự ID xác định. Save schema7–23 vẫn nạp được; estate aggregate cũ được ghi thành phần legacy chưa gắn với một decedent, và tiếp tục được cộng vào tổng `estate_`. Inventory trong hồ sơ tham gia bảo toàn tổng hàng; tiền hồ sơ là sổ phụ của `estate_`, không cộng hai lần. Các tham chiếu được kiểm tra lại với chủ sở hữu, người cấp quyền hoặc con nợ khi Realm xác nhận invariant.

Phần này chưa trả nợ từ estate, chia tiền/hàng cho người hưởng, chuyển title/xưởng, xử lý phần estate legacy thành đối tượng pháp lý hoặc nối guardian. Hàm phân giải người hưởng chỉ là query; chưa gọi trong phân phối.

## Kiểm chứng

- Windows Release build cho `polyec_core`, `polyec_manager_tests` và `polyec_realm_tests`: pass.
- Release manager tests: pass; gồm prepare/apply purity, stale read set, conservation, link ID cho xưởng/title/land-use/loan và roundtrip codec case.
- Release Realm tests: 20.621 checks pass; bao gồm Realm save/load cho 512 nhân vật, schema23 aggregate estate migration sang schema24 và các fixture migration hiện có.
- Tìm thấy rồi sửa giới hạn parser lịch: `SCHEDULE_5` trước đó từ chối schema trên 23 dù nội dung không đổi.
- Chưa chạy full CTest, Debug, Python binding/save replay, GDExt/Godot smoke hoặc performance profile sau thay đổi.
- Review Codex tự kiểm; chưa có review độc lập.

Plan: [hồ sơ di sản và schema24](../design/M5-core-estate-ledger-plan.md). Manifest gốc được nhắc ở phiên nghiệm thu nhưng hiện không còn trong checkout; không tạo lại hash lịch sử.

