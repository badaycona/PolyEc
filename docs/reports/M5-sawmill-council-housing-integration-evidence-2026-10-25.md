# Bằng chứng tích hợp Sawmill, Tools và nhà ở hội đồng

Ngày chạy: 2026-10-25
Phạm vi: C++ Realm trên Windows, cấu hình Debug, build tree `build/m5-python-ai`.
Mục tiêu: chứng minh vật liệu nhà ở được tạo bởi cơ sở thật và đại diện hội đồng có thể khởi động, rồi hoàn tất dự án bằng quỹ được cấp hợp lệ.

## Chuỗi đã chạy

Trong Atlas 27 vùng, seed `20261024`, dân số256:

1. Forestry vùng6 nhận bốn khoảng Work hợp lệ và tạo bốn Timber.
2. Chủ cơ sở công chuyển từng Timber từ Forestry vào Sawmill. Bốn khoảng Work tại Sawmill biến đúng bốn Timber thành bốn ConstructionMaterial; chủ cơ sở chuyển cả bốn món vào kho vùng6.
3. Authority vùng7 lập mỏ Copper hữu hạn. Hai khoảng Work tạo hai Copper, nguồn quặng giảm100→98. Người buôn mua Copper, trả phí vận chuyển tới vùng6, đi theo tuyến đang mở rồi bán hàng vào kho vùng nhận.
4. ToolWorkshop vùng6 nhận hai Iron và hai Fuel từ kho mở đầu cùng hai Copper đã vận chuyển. Hai khoảng Work tạo đúng hai Tools. Chủ xưởng công chuyển Tools vào kho vùng.
5. Cư dân đô thị trưởng thành vùng6 tự nộp hồ sơ; Sovereign bổ nhiệm. Authority cấp499 tiền vào quỹ đô thị qua lệnh FundMunicipal hiện hành. Đại diện được bổ nhiệm khởi động dự án công, dự án tiêu đúng bốn ConstructionMaterial, hai Tools và80 tiền.
6. Test lưu lúc dự án đang chạy, nạp sang Realm mới, rồi tiến cả hai thêm60 ngày. Cả hai hoàn tất cùng lúc, có cùng fingerprint, và Census cho thấy sức chứa công vùng6 tăng lên8 giường.

Cư dân đủ điều kiện đầu tiên trong seed này là Governance ID8, cũng là authority vùng6. Quy tắc hiện hành cho phép một cư dân đô thị đủ điều kiện tự ứng cử; Sovereign vẫn là người quyết định bổ nhiệm. Test kiểm đúng hành vi này, không thêm bầu cử phổ thông hoặc quyền mới.

## Fixture và cách đọc số liệu

- Dự án công chỉ mở khi cư dân đô thị vượt sức chứa giường công. Test đặt sức chứa vùng6 về0 để tạo tình trạng thiếu nhà kiểm được ngay. Đây là tình huống dựng cho test; nó không chứng minh campaign tự nhiên đã tạo quá tải.
- Kho vùng6 mở đầu có275 Tools legacy. Test đánh dấu chúng là đã tiêu thụ trong goods ledger trước khi bắt đầu, không tạo Tools mới. `Realm::invariant()` được kiểm sau bước này. Vì thế hai Tools mới do ToolWorkshop tạo là toàn bộ tồn kho Tools địa phương khi dự án khởi động và dự án tiêu hết đúng hai món đó. Đây là cách dựng tình huống khan hiếm có cân ledger, không phải hành vi game được giả làm đã xảy ra.
- Iron và Fuel là hàng legacy có sẵn trong kho mở đầu vùng6; Copper đến từ mỏ thật vùng7. Chuỗi này chưa chứng minh Iron/Fuel có nguồn địa lý tái sinh.
- Quỹ đô thị tăng499 qua lệnh chuẩn, chi80 khi mở dự án và còn419.

## Kiểm tra đã chạy

- Build `polyec_sawmill_housing_chain_tests` và `polyec_municipal_council_tests` trong Debug.
- CTest chỉ chạy `sawmill_housing_chain_tests` và `municipal_council_tests`: **2/2 pass**.
- Chạy trực tiếp: chuỗi Sawmill/Tools/council/housing **56 checks**; council regression **42 checks**.
- `git diff --check`: pass.
- SHA-256 `core/tests/test_sawmill_housing_chain.cpp`: `C69C543091A57B886072FC8F99D6F3710D659863EB963C5FE7B26355A7407E92`.

Các lượt kiểm đầu phát hiện hai giả định sai trong test: kho vùng6 vốn có275 Tools nên tổng tồn không bằng2; và cư dân đô thị đầu tiên là người Governance nên test không được loại họ khỏi danh sách ứng cử. Test cuối dựng thiếu Tools bằng sổ tiêu thụ cân bằng và dùng đúng candidate/mask hiện hành. Không sửa gameplay source.

## Giới hạn

Kết quả chứng minh các đường khai thác, sản xuất, thương mại qua tuyến, bổ nhiệm hội đồng, cấp quỹ, vật liệu xây dựng, nhà ở và save/load có thể tiếp nối trong cùng Realm. Nó chưa chứng minh thiếu nhà tự phát, campaign nhiều năm, nguồn Iron/Fuel địa lý, thao tác chuột Godot, Python/native parity, Profile B, stress/performance hay package Windows. Đây là kiểm tích hợp C++ của một tình huống được dựng rõ, không phải nghiệm thu toàn M5.

Plan: [M5 Sawmill–council housing](../design/M5-sawmill-council-housing-integration-plan.md). Source base trước nhóm: commit `ae0ba99`.
