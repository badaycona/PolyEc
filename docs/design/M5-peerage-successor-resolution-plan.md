# M5.3 — phân giải người kế vị tước theo con hợp pháp lớn tuổi nhất

## Contract và baseline

`docs/IMPLEMENTATION_PLAN.md` §2 quy định tước truyền cho người con hợp pháp lớn tuổi nhất, không phân biệt giới; nếu không có người kế tiếp thì mở hồ sơ chờ công nhận. Realm hiện có cạnh `ParentChild` với hướng child → parent và cờ `legal_parentage`, nhưng chưa có record tước. `EstateManager` chọn tất cả người hưởng theo hàng thừa kế tài sản, nên không thể dùng danh sách đồng thừa kế đó để tự chọn người giữ tước.

## Lát triển khai

- Thêm `PeerageSuccessionManager` làm query thuần trên ID người chết, Person và relation edges.
- Chỉ cạnh `ParentChild` được đánh dấu legal, lưu theo chiều child → parent, và trỏ trực tiếp tới người chết mới xét con. Cạnh đảo chiều không được dùng. Con phải còn sống. Chọn duy nhất người có `age_days` lớn nhất; không xét giới tính.
- Nếu không có con hợp pháp còn sống, trả `RecognitionRequired` với danh sách trống. Nếu hai hay nhiều con có cùng tuổi lớn nhất tính theo `age_days`, trả `RecognitionRequired` cùng các ứng viên đã sắp ID; không phá hòa theo ID.
- Từ chối người giữ tước còn sống, ID cư dân không khớp vị trí chuẩn, tuổi âm, ID/đầu mút quan hệ ngoài miền, và cờ cha/mẹ hợp pháp gắn với loại cạnh không phải cha/mẹ–con trước khi trả kết quả. Bản ghi quan hệ trùng không được biến một ứng viên thành hòa.
- Thêm manager oracle trong `core/tests/test_managers.cpp`.

## Ngoài phạm vi của lát này

Không thêm `PeerageTitle` record, dữ liệu tước khởi đầu, hồ sơ công nhận, command bổ nhiệm/kế vị, save field, event, observation/action hay xử lý tử vong trong Realm. Các phần đó cần được xây trên luật phân giải này và yêu cầu quyền/công nhận tương ứng; việc hiện thực pure resolver không được báo cáo là đã có tước hoạt động trong game.

## Kiểm tra

Build và chạy Release target `polyec_manager_tests` trực tiếp. Kiểm người con lớn tuổi nhất, chỉ nhận cạnh theo chiều child → parent, bỏ qua cạnh chưa được xác nhận và con chết, giữ nhiều ứng viên khi đồng tuổi lớn nhất, không có ứng viên, người giữ tước còn sống, ID cư dân/quan hệ sai, tuổi âm và cờ quan hệ hợp pháp không khớp loại cạnh. Không chạy full CTest vì manager chỉ đọc dữ liệu đầu vào và không sửa Realm, save, binding hoặc consumer.

## Review

Đối chiếu quy tắc với §2, hướng quan hệ với `M5-core-estate-beneficiary-resolution-plan.md`, và không đổi tie thành ID tùy tiện. Tự review; chưa có reviewer độc lập.
