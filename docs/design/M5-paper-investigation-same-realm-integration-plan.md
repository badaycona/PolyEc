# M5.2 — nối Paper Mill với Investigation trong cùng Realm

Ngày 2026-10-25. Đây là plan kiểm chứng tích hợp, không thêm luật chơi mới.

## Mục tiêu và lý do

Test Paper Mill hiện chứng minh Forestry, vận chuyển Timber, sản xuất Paper, bán và save/load. Test Investigation chứng minh tiêu Paper, nhưng tạo Paper bằng fixture riêng. Cần một kịch bản C++ duy nhất để xác nhận hàng do chuỗi sản xuất thật tạo ra có thể đi tới người làm chính quyền, được mua qua chợ, dùng mở Investigation, tiếp tục sau save/load và tạo Knowledge có nguồn.

## Phạm vi source

- Chỉ mở rộng `core/tests/test_paper_mill.cpp`; không sửa luật hoặc API của game.
- Giữ seed 49201, Atlas 27 vùng, dân số 256 và các thao tác Forestry → shipment → Paper Mill đang có.
- Dùng lệnh mua hàng thường (`RealmCommand`, Intent::Buy) của Governance actor với lượng 1. Trong code hiện tại, lệnh mua thường dùng tiền cá nhân/quỹ hộ cho mọi actor; MarketOrderCommand cũng chỉ giữ các nguồn đó. Sau chuyến đi và vận hành xưởng, Governance actor của seed này chỉ còn 10 tiền trong khi Paper có giá90, và game chưa có lệnh chuyển ngân khố sang tài khoản cá nhân. Test vì vậy cộng thêm tiền vào tài khoản mở đầu của đúng actor bằng fixture có cân ledger; không tạo Paper hoặc sửa giá. Report phải ghi rõ khoản vốn fixture và giới hạn kết luận này. Giao dịch mua, thuế, trừ kho chợ và mọi chuỗi Paper vẫn chạy qua Realm thật.
- Test có thể tạo một secret Fact qua `RealmObservationTestAccess`, vì Investigations cần một sự thật bí mật để chứng minh kết quả. Report phải nói rõ đây là fixture; Paper, tiền, vận chuyển, sản xuất, mua hàng, Investigation và Knowledge đều do thao tác Realm thật tạo.
- Lưu Realm sau khi Paper được mua nhưng trước khi Investigation hoàn tất; tải sang Realm mới, tiến cả hai cùng số ngày và so fingerprint, tồn kho, sổ tiêu thụ Paper, Investigation và Knowledge.

## Kiểm ngay

- Build `polyec_paper_mill_tests` và `polyec_investigation_paper_tests` trong build tree hiện có.
- Chạy CTest đúng hai test trên; thêm test tích hợp mới trong paper-mill executable.
- Kiểm success, reject khi thiếu Paper/không đủ giá nếu có thể dựng bằng lệnh công khai, số dư trước/sau, Paper được tiêu đúng một, Investigation hoàn tất đúng hạn, Knowledge trỏ đúng Investigation và fingerprint live/resumed giống nhau.
- Kiểm invariant Realm ở các điểm trước/sau save/load.

## Để mốc sau

Không chạy Python/GDExtension/Godot vì không thay binding hoặc hành vi UI; đưa Python paper-mill smoke và thao tác Godot thật vào mốc tích hợp gameplay ổn định theo `DEVELOPMENT_WORKFLOW.md` mục 4. Không chạy RL và không coi kịch bản đơn lẻ này là hoàn tất M5. Nếu giá/nguồn hàng Atlas khiến Governance không thể mua Paper, ghi số liệu và mở lát gameplay riêng thay vì nới assertion hoặc dùng fixture Paper.

## Đóng nhóm

Nhóm này đóng khi đúng một Realm hoàn thành chuỗi thật Forestry → Timber → shipment → Paper Mill → Paper → market order → Investigation → Knowledge, và save/load giữa Investigation cho kết quả tương đương. Sau đó chuyển sang khoảng trống M5 kế tiếp trong CURRENT_TASK.
