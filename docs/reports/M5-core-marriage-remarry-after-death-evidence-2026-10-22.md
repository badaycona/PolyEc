# Bằng chứng M5.3 — người góa kết hôn lại sau khi phối ngẫu qua đời

Ngày 2026-10-22. Luật hôn nhân tại [IMPLEMENTATION_PLAN.md §2](../IMPLEMENTATION_PLAN.md) yêu cầu không có phối ngẫu còn sống khác; plan consent/escrow nói tính độc quyền áp dụng khi cả hai đang sống. Test trước đó đã chặn người có vợ/chồng còn sống kết hôn lần hai, nhưng chưa nối điều kiện này với death phase. Lát này kiểm toàn bộ đường từ cái chết đến đề nghị mới và sự đồng ý mới.

## Tình huống và kết quả

Test Court tạo đề nghị thứ nhất giữa người 1 và người 2. Người 2 chấp thuận sau thời gian chờ; Realm ghi quan hệ phối ngẫu đối xứng, chuyển đúng 50 tiền từ ký quỹ sang người nhận và giữ nguyên hai household riêng biệt. Test xác nhận trong lúc người 2 còn sống, đề nghị kết hôn tiếp theo của người 1 bị chặn.

Sau đó fixture đặt health của người 2 dưới ngưỡng chết và gọi đúng pha cư dân của Realm. Người 2 được ghi nhận đã chết; cạnh phối ngẫu cũ vẫn ở dữ liệu lịch sử, và người 1 còn sống. Realm đưa cho người 1 một ứng viên mới đủ điều kiện. Người 1 mở đề nghị mới; 50 tiền nằm trong escrow, và quan hệ mới chưa xuất hiện trước khi người nhận đồng ý. Save/load khi đang chờ giữ fingerprint và khoản ký quỹ.

Khi hết thời gian chờ, chỉ người nhận mới được chấp thuận đề nghị. Realm chuyển 50 tiền đúng một lần, xóa ký quỹ, không đổi tổng tiền, không gộp household. Dữ liệu quan hệ vẫn ghi nhận người phối ngẫu cũ đã chết và người phối ngẫu mới; trong đó chỉ có một người còn sống. Save/load sau khi chấp thuận giữ nguyên cả hai quan hệ và số dư.

Với người chơi, cái chết kết thúc điều kiện “đang có phối ngẫu sống” nhưng không xóa lịch sử quan hệ cũ. Hôn nhân mới vẫn cần đề nghị, thời gian chờ và chính người được đề nghị chấp thuận. Lát này không tạo luật ly hôn, chia tài sản hoặc nhập hộ.

## Thay đổi và kiểm tra

- Chỉ thêm friend test helper và scenario trong `core/tests/test_realm_court.cpp`; không đổi gameplay source, action/observation/model, API hay save schema.
- Windows Visual Studio Release: build `polyec_court_tests` thành công; trực tiếp **537 checks passed**.
- Windows Visual Studio Debug: build cùng target thành công; trực tiếp **537 checks passed**.
- Full core Release CTest 19/19 gần nhất chạy trước test Court này; không tính kết quả đó là có chứa scenario mới. Không chạy Python/native/GDExtension/Godot: các mặt API, schema, binding và model không đổi.
- Trong lượt đầu, oracle so tiền sau thời gian chờ với số tiền trước đó và thất bại vì lịch Realm vẫn chạy thu nhập/chi tiêu trong thời gian chờ. Oracle được sửa để chụp lại tiền ngay trước lúc chấp thuận; kiểm tra cuối pass. Đây là sửa phép đối chiếu, không đổi luật.
- Tự review chuỗi sống/chết, eligibility, candidate mask, escrow, consent, số dư trước/sau, household riêng, quan hệ lịch sử, invariant và save/load. Chưa có reviewer độc lập.

Đây là kiểm tra vòng đời kết hôn trong Realm, không phải nghiệm thu toàn bộ M5. [Kế hoạch](../design/M5-core-marriage-remarry-after-death-plan.md) và [manifest](../../artifacts/m5-core-marriage-remarry-after-death-2026-10-22-manifest.json) lưu nguồn, phạm vi và hash file. Không commit, push hoặc publish.
