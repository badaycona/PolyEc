# M5.2 — nối Sawmill với nhà ở hội đồng trong cùng Realm

Ngày 2026-10-25. Nhóm này chỉ kiểm chứng tích hợp, chưa đề xuất luật nhà ở mới.

## Vì sao làm

`test_sawmill_housing_chain.cpp` hiện chứng minh Forestry tạo Timber, Sawmill dùng Timber tạo ConstructionMaterial và save/load giữ sản phẩm. `test_municipal_council.cpp` kiểm đúng việc cư dân tự nguyện đề cử, Sovereign bổ nhiệm/từ chối, đại diện được dùng quỹ đô thị để mở dự án nhà ở. Nhưng test council đang tự thêm ConstructionMaterial, Tools và tiền quỹ bằng fixture. Cần nối ít nhất ConstructionMaterial do Sawmill làm ra với thao tác nhà ở có quyền trong cùng Realm.

## Phạm vi và luật giữ nguyên

- Mở rộng `core/tests/test_sawmill_housing_chain.cpp` và chỉ khi cần thêm helper test-only cùng file.
- Dùng Realm Atlas seed `20261024`, region6, Forestry, Sawmill, Work authorization/schedule, thao tác nominate → Sovereign appoint → FundMunicipal → housing project.
- Không thêm ConstructionMaterial trực tiếp; tối thiểu bốn đơn vị phải được tạo bởi Sawmill theo recipe hiện hành, chuyển vào kho vùng bằng command thật và được tiêu bởi dự án.
- Dự án cần bốn ConstructionMaterial và hai Tools. Trước khi code phải xác nhận Tools có thể được tạo/mua/chuyển hợp lệ trong seed và vùng thử. Không dùng fixture để che thiếu Tools. Nếu chưa có nguồn hợp lệ, giữ test thất bại có dữ liệu và mở một nhóm nguồn Tools riêng theo M5.2.
- Quỹ đô thị phải đến từ FundMunicipal command, không cộng trực tiếp bằng helper. Đại diện phải tự ứng cử; Sovereign bổ nhiệm. Không dùng bầu cử phổ thông hay đổi quyền.
- Lưu trước khi nhà hoàn tất; tải và tiếp tục cùng số game time. So sánh công trình, tồn kho ConstructionMaterial/Tools, quỹ đô thị, fingerprint và invariant.

## Kiểm ngay và ranh giới

Xác nhận trước code nguồn Tools, funded treasury, candidate/action indices và số Work cycles cần thiết; sau đó build `polyec_sawmill_housing_chain_tests` cùng `polyec_municipal_council_tests` và chạy đúng hai CTest. Kiểm trực tiếp thành công, thiếu vật liệu bị từ chối nguyên tử, quyền của người không được bổ nhiệm và save/load. Không đổi core gameplay/API nên Python/GDExtension/Godot được gom vào mốc tích hợp UI sau.

Nhóm đóng khi cùng một Realm dùng ConstructionMaterial do Sawmill sản xuất để đại diện được bổ nhiệm tiêu quỹ công và khởi động nhà ở; load/resume giữ đúng kết quả. Nếu Tools chưa có chuỗi thật, không gọi nhóm này hoàn tất và không thêm chúng bằng fixture.
