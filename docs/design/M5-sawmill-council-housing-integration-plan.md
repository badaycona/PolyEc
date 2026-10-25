# M5.2 — nối Sawmill với nhà ở hội đồng trong cùng Realm

Ngày 2026-10-25. Nhóm này chỉ kiểm chứng tích hợp, chưa đề xuất luật nhà ở mới.

## Vì sao làm

`test_sawmill_housing_chain.cpp` hiện chứng minh Forestry tạo Timber, Sawmill dùng Timber tạo ConstructionMaterial và save/load giữ sản phẩm. `test_municipal_council.cpp` kiểm đúng việc cư dân tự nguyện đề cử, Sovereign bổ nhiệm/từ chối, đại diện được dùng quỹ đô thị để mở dự án nhà ở. Nhưng test council đang tự thêm ConstructionMaterial, Tools và tiền quỹ bằng fixture. Cần nối ít nhất ConstructionMaterial do Sawmill làm ra với thao tác nhà ở có quyền trong cùng Realm.

## Phạm vi và luật giữ nguyên

- Mở rộng `core/tests/test_sawmill_housing_chain.cpp` và chỉ khi cần thêm helper test-only cùng file.
- Dùng Realm Atlas seed `20261024`, region6, Forestry, Sawmill, Work authorization/schedule, thao tác nominate → Sovereign appoint → FundMunicipal → housing project.
- Không thêm ConstructionMaterial trực tiếp; tối thiểu bốn đơn vị phải được tạo bởi Sawmill theo recipe hiện hành, chuyển vào kho vùng bằng command thật và được tiêu bởi dự án.
- Dự án cần bốn ConstructionMaterial và hai Tools. Tools có thể được tạo hợp lệ bằng ToolWorkshop với một Iron, một Copper và một Fuel cho mỗi đơn vị; Iron/Fuel có sẵn trong kho chợ mở đầu legacy của region6, còn Copper có nguồn mỏ hữu hạn ở region7. Kịch bản sẽ khai thác hai Copper, mua chúng bằng thương nhân, gửi hàng qua tuyến quốc gia rồi dùng cả ba đầu vào thật tại ToolWorkshop. Không dùng fixture để che thiếu Tools.
- Vùng6 mở đầu có275 Tools legacy. Để chứng minh housing phụ thuộc vào sản lượng mới thay vì tồn cũ, test-only scenario marks toàn bộ275 đơn vị đó là đã tiêu thụ, ghi cùng số vào goods-consumed ledger và kiểm `Realm::invariant()` trước khi bắt đầu. Nhờ vậy hai Tools mới do Workshop tạo là toàn bộ tồn kho địa phương khi dự án khởi động. Báo cáo phải nêu rõ mô phỏng không tạo ra bước tiêu thụ mở đầu đó bằng luật game; đây là cách dựng tình huống khan hiếm có cân ledger.
- Nhà ở chỉ được khởi động khi cư dân đô thị vượt số giường công. Để không chờ tăng dân số tự nhiên nhiều năm, test đặt sức chứa giường công vùng6 về0 như một fixture tình huống khủng hoảng nhà ở; helper chỉ đổi mức sức chứa kịch bản và làm mới census, không tạo/xóa người hay tài sản. Ghi rõ giới hạn này trong report.
- Quỹ đô thị phải đến từ FundMunicipal command, không cộng trực tiếp bằng helper. Đại diện phải tự ứng cử; Sovereign bổ nhiệm. Không dùng bầu cử phổ thông hay đổi quyền.
- Lưu trước khi nhà hoàn tất; tải và tiếp tục cùng số game time. So sánh công trình, tồn kho ConstructionMaterial/Tools, quỹ đô thị, fingerprint và invariant.

## Kiểm ngay và ranh giới

Xác nhận trước code source chain, funded treasury, candidate/action indices và số Work cycles cần thiết; sau đó build `polyec_sawmill_housing_chain_tests` cùng `polyec_municipal_council_tests` và chạy đúng hai CTest. Kiểm trực tiếp thành công, thiếu vật liệu bị từ chối nguyên tử, quyền của người không được bổ nhiệm và save/load. Không đổi core gameplay/API nên Python/GDExtension/Godot được gom vào mốc tích hợp UI sau.

Nhóm đóng khi cùng một Realm dùng ConstructionMaterial do Sawmill sản xuất để đại diện được bổ nhiệm tiêu quỹ công và khởi động nhà ở; load/resume giữ đúng kết quả. Nếu Tools chưa có chuỗi thật, không gọi nhóm này hoàn tất và không thêm chúng bằng fixture.
