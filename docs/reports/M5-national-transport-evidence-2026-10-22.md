# M5 — nền vận tải hàng hóa quốc gia (đang triển khai)

Cập nhật 2026-10-22. Báo cáo này mô tả source hiện tại, cách hàng và tiền đi qua Realm, các phép kiểm đã chạy, những giới hạn còn lại và kết quả cũ theo schema 27. Kế hoạch của cả nhóm nằm trong [M5-national-transport-plan](../design/M5-national-transport-plan.md); các thông số game thử và luật cấp cao nằm trong [IMPLEMENTATION_PLAN §15.5](../IMPLEMENTATION_PLAN.md).

## Tình trạng nhìn từ phía người chơi

Trong core hiện có thể mua hàng bằng command giao dịch của Realm rồi gửi chuyến tới tỉnh khác. Khi Realm nhận lệnh gửi, hàng rời inventory người gửi và phí tuyến được giữ từ tiền của họ. Hàng chờ hoặc đang đi được lưu trong sổ vận tải. Khi lô tải lên một cạnh atlas, ngân khố vùng khởi hành nhận phí; lô tới điểm cuối mới được cộng vào inventory người gửi. Nếu người đó chết giữa đường, hàng tới hồ sơ di sản và di sản chỉ được chia sau chuyến cuối.

Đây hiện là hành vi C++ core, chưa nối Python, ứng dụng native, Godot hay giao diện người chơi. Không thể khẳng định người chơi đã bấm được nút vận chuyển trong app. API quyết định v11 cũng chưa dùng được ở Realm Atlas 27 vùng; chi tiết và phép kiểm được nói rõ bên dưới.

## Bản đồ tuyến và luật ngày

Atlas có 27 vùng mang ID 0–26 và 31 liên kết. Realm nhận `regions=26` để tạo đủ 27 tỉnh. `NationalTransportManager` chuyển liên kết atlas thành 59 hướng tuyến có cấu hình cụ thể. Sông chỉ đi xuôi theo mũi tên atlas; planner không tự tạo chiều ngược. Đường nâng cấp, ven biển và hành lang atlas được cấu hình theo quyết định đã ghi trong IMPLEMENTATION_PLAN. Năm liên kết triều đình đang được bật thử hai chiều như đường bộ; đây là giả định gameplay có thể cấu hình, chưa phải kết luận địa lý.

Giá trị ban đầu là 1 ngày cho cạnh sông, 2 ngày cho cạnh khác, 64 đơn vị hàng mỗi ngày trên một liên kết, và 1 đồng bạc cho mỗi đơn vị hàng qua mỗi cạnh. Đây là đơn vị game trừu tượng, không đổi từ kilomet hay tốc độ ngoài đời. Chiều đi và chiều về của cùng một liên kết dùng chung mức sức tải; phí một chiều trả vào ngân khố vùng khởi hành của lô.

Command nhận sau khi ngày D đã kết thúc có thể bắt đầu tải sớm nhất ngày D+1. Nếu lô được tải ngày L và cạnh mất N ngày, nó tới cuối ngày L+N; lô ở điểm trung chuyển có thể được tải vào ngày kế tiếp. Realm xét shipment ID tăng dần rồi lô theo thứ tự ổn định. Nếu có chuyến ngược chiều dùng hết một phần sức tải, chuyến còn lại chỉ nhận phần sức tải còn trống và giữ phần dư tới hôm sau.

Planner `Fastest` hoặc `LowestFee` hiện chỉ nhìn thời gian và phí cấu hình tĩnh của cạnh. Nó chưa biết những lô đang xếp hàng, nên đường nó chọn không phải lời hứa “nhanh nhất khi có ùn hàng”. Thời gian giao thật được quyết định bởi lịch tải hằng ngày. Khi thiếu chỗ trống trong ngân khố để nhận thêm phí hoặc ký quỹ không đủ do tình trạng save/config, tải bị giới hạn theo lượng an toàn thay vì cộng tràn số.

## Hàng, tiền, hủy và di sản

**Tạo chuyến:** command cần người gửi còn sống, tỉnh hiện tại, tỉnh đích khác nguồn, GoodId hợp lệ, lượng hàng dương, đủ inventory và đủ tiền cho phí của đường được chọn. Realm dựng đường trên những hướng hiện mở, tính tổng phí bằng số nguyên có kiểm tràn, rồi giữ hàng/phí và cấp shipment ID. Lệnh không hợp lệ bị từ chối trước khi đổi quyết định, tiền, hàng hay ID. Hàng trên đường không được tính đồng thời vào kho người gửi hoặc kho tỉnh.

**Tải và trả phí:** đầu ngày Realm xử lý lô đã tới trước, sau đó xét các lô đang chờ. Sức tải được cộng chung cho hai chiều của cùng cạnh. Khi lô thật sự được tải, phí của cạnh đó rời ký quỹ và vào ngân khố cấu hình; phí dự kiến cho chặng sau vẫn nằm trong ký quỹ. Hàng đã lên cạnh không bị xóa khi tuyến đóng; nó đến nút kế tiếp rồi chờ nếu cạnh tiếp theo chưa mở.

**Hủy:** người gửi chỉ có thể hủy lượng chưa tải đang chờ tại đúng tỉnh mà họ đang đứng. Hàng đã rời tỉnh tiếp tục đi tới nút kế tiếp; người gửi không thể kéo nó về từ xa. Hủy một phần hoàn lượng hàng được chọn và khoản phí dành cho các cạnh chưa tải của chính phần đó; phí cho cạnh đang đi/đã đi không được hoàn lại.

**Người gửi chết:** khi chết, tiền/hàng đang ở trong inventory đi theo quy tắc EstateCase hiện hữu; shipment và phí chưa chi vẫn nằm riêng trong sổ vận tải với người đó là người hưởng lợi. Lô cuối tới được cộng vào inventory của EstateCase. Realm trì hoãn trả nợ và chia di sản cho đến khi mọi shipment của người đó đã giao hoặc bị hủy hợp lệ; danh sách shipment đang hoạt động là nguồn xác định việc chờ, không tạo thêm một cờ có thể lệch với sổ hàng. Oracle C++ kiểm save/load, giao hàng vào di sản, hoàn tất chia tài sản và chạy tiếp giống hệt thế giới không load.

## Save, migration và observation AI

Schema 27 đã có section `NATIONAL_TRANSPORT_1` để giữ cấu hình tuyến. Schema 28 bổ sung section `TRANSPORT_SHIPMENT_1`, lưu shipment, đường đã chọn, lot theo chặng, lô đang đi hoặc chờ, phí escrow, ID lot kế tiếp. Sức tải đã dùng trong ngày được suy lại từ cạnh và thời điểm tải của các lot; không cần lưu một bộ đếm thứ hai có thể khác với sổ hàng. Schema 7–27 được đọc với ledger shipment rỗng; schema 26 của Realm Atlas vẫn được migration route mặc định như trước. Tải schema 28 kiểm các ID, người hưởng lợi, hàng, route path, lượng chờ/đã giao/đã hủy, phí còn phải chi, ngày đến và sức tải trước khi thay Realm đang mở.

API policy hiện tại là contract v11 với 129 hành động và bố cục quan sát giá dành cho tối đa 7 tỉnh. Trước khi sửa, vòng ghi giá cho 27 tỉnh vượt qua index 127 và ghi đè phần mô tả hành động bắt đầu ở index 128. **Đính chính provenance:** test profile 13/13 ban đầu chỉ gọi `candidates`, `mask` và `observation`, không so kết quả với một bộ giá đã biết; kết quả đó chỉ cho thấy lời gọi không văng, không chứng minh dữ liệu đúng. Bản sửa chặn mọi API quyết định v11 trên Realm có hơn 7 tỉnh trước khi trả dữ liệu.

Oracle mới kiểm 49 giá features (7 tỉnh × 7 hàng legacy) với giá Province làm nguồn biết trước, xác nhận feature giá cuối nằm ở index 72 còn action features bắt đầu ở index 128, rồi đối chiếu kiểu hành động và bit hợp lệ ở đúng offset. `polyec_realm_profile_tests` hiện 17/17. Realm Atlas vẫn tạo, mô phỏng, đóng/mở route và save/load được, nhưng không gọi policy v11 để ra quyết định. Chưa có model/observation v27 vùng, chưa resume hoặc huấn luyện learned policy trên profile này.

## Release build và kiểm tra trực tiếp trên source hiện tại

Chỉ build các target C++ có liên quan tới API, save schema và pipeline ngày; không chạy ALL_BUILD hoặc full CTest. Kết quả sau migration schema 28 và fixture tương ứng:

| Target | Kết quả | Hành vi được đối chiếu |
|---|---:|---|
| `polyec_realm_transport_tests` | 88 checks passed (Release and Debug) | Mua hàng bằng command thật → gửi chuyến → giữ phí → chuyển phí vào đúng ngân khố (so với Realm cùng seed không gửi) → giao hàng. Hai chuyến ngược chiều dùng 64 đơn vị sức tải ngày đầu và 16 đơn vị còn lại ngày kế. Save/load giữa chặng; save sai đường, vị trí lô, ngày đến hoặc tổng phí bị từ chối mà Realm đang mở không đổi; tuyến đóng/mở lại; hủy phần chờ sau khi phần khác đã lên đường; nhiều chặng, người gửi chết, hàng vào di sản và hoàn tất chia tài sản. Thêm chín cấu hình thử thời gian/sức tải, mỗi lần giao đủ 80 đơn vị và giữ invariant. |
| `polyec_realm_profile_tests` | 17/17 | API v11 từ chối Realm27; oracle giá/action; route snapshot; schema28 round-trip; schema27 không shipment; schema26 migration; save đảo chiều sông bị từ chối nguyên tử. |
| `polyec_realm_tests` | 18.834 checks passed (Release and Debug) | Regression Realm sau save/schema/core. Lượt Release cuối nhóm: workload profile A 10.000 người/90 ngày mất 0,52118 giây; Debug mất 3,50342 giây. Đây chỉ là thời gian chạy chức năng trên máy hiện tại, không phải benchmark vận tải hay kết luận hiệu năng. Các lượt cũ ghi 0,52602 và 1,14558 giây; giữ nguyên xuất xứ từng lượt, không lấy trung bình để đặt mục tiêu hiệu năng. |
| `polyec_schedule_tests` | 3.717 checks | Cursor, attendance, production, pause/save/load/migration; oracle token count đã tính header snapshot tuyến và shipment ledger rỗng. |
| `polyec_national_transport_tests` | 16/16 (Release and Debug) | 59 hướng atlas, sông ngược, route đóng, chọn đường, phí và overflow; thời gian chặng và sức tải ngoài miền được chặn. |
| `polyec_campaign_tests` | 281/281 | Campaign vẫn giữ đường và chuỗi xử lý cũ sau thay đổi các bảng trả lương theo vùng; workload 10.000 người/102 ngày khoảng 513 ms trong lượt này, chỉ là smoke run. |
| `polyec_market_order_tests` | 33 | Load/save lệnh thị trường và ký quỹ qua fixture schema 28 → schema lịch sử. |
| `polyec_court_tests` | 565 | Regression hồ sơ tòa/di sản và migration. |
| `polyec_household_fund_tests` | 36 | Save/migration quỹ hộ. |
| `polyec_land_title_tests` | 88 | Save/migration quyền sở hữu đất. |
| `polyec_realm_clock_tests` | 301 | Save/restore đồng hồ Realm. |

Lượt Release cuối nhóm dựng lại ba target trực tiếp bị ảnh hưởng (`polyec_realm_transport_tests`, `polyec_realm_tests`, `polyec_realm_profile_tests`) và chạy cả ba: lần lượt 88 checks, 18.834 checks và 17/17 đều đạt. Sau đó bộ transport/manager/Realm cũng được build và chạy trong Debug: 88 checks, 16/16, 18.834 checks đều đạt. Thay đổi ngày đến ảnh hưởng invariant chung của Realm nên đã chạy Realm và profile; không chạy lại các target save-consumer không bị thay đổi ở lượt này. Bộ regression schema28 rộng hơn được ghi riêng trong mục provenance bên dưới và trong CURRENT_TASK.

### Phép thử độ nhạy cấu hình — trial-1

Trước khi viết fixture, IMPLEMENTATION_PLAN §15.5 đã ghi rõ thử nghiệm dùng cùng seed 805, cùng một liên kết đường bộ hai chiều, chỉ mở liên kết đó, gửi một lô 80 đơn vị, cùng loại hàng và phí 1 đồng/đơn vị/chặng. Chỉ đổi thời gian đi và sức tải. Vì mặc định đường bộ là 2 ngày và 64 đơn vị/ngày, các mức lần lượt là 1/2/4 ngày và 32/64/128 đơn vị/ngày. Thời điểm giao ghi dưới đây là ngày mà sau xử lý đầu ngày toàn bộ 80 đơn vị đã trở lại inventory người gửi; ngày bắt đầu tải sớm nhất là ngày 1.

| Thời gian chặng (ngày) | Sức tải (đơn vị/ngày) | Số ngày có tải | Ngày người gửi nhận đủ 80 đơn vị | Kết quả |
|---:|---:|---:|---:|---|
| 1 | 32 | 3 | 5 | Giao đủ, invariant hợp lệ |
| 1 | 64 | 2 | 4 | Giao đủ, invariant hợp lệ |
| 1 | 128 | 1 | 3 | Giao đủ, invariant hợp lệ |
| 2 | 32 | 3 | 6 | Giao đủ, invariant hợp lệ |
| 2 | 64 | 2 | 5 | Giao đủ, invariant hợp lệ |
| 2 | 128 | 1 | 4 | Giao đủ, invariant hợp lệ |
| 4 | 32 | 3 | 8 | Giao đủ, invariant hợp lệ |
| 4 | 64 | 2 | 7 | Giao đủ, invariant hợp lệ |
| 4 | 128 | 1 | 6 | Giao đủ, invariant hợp lệ |

Dữ liệu gốc với cấu hình, seed, lô hàng, đơn vị và cả chín dòng kết quả nằm trong [CSV trial-1](../../artifacts/m5-transport-sensitivity-trial1-2026-10-22.csv); test C++ phát lại cùng bộ và in ra bảng tương tự. Kết quả cho thấy riêng fixture này: tăng thời gian từ 1 lên 2 rồi 4 ngày làm ngày nhận đủ chậm thêm đúng 1 rồi 2 ngày; tăng sức tải từ 32 lên 64 rồi 128 giảm số ngày tải từ 3 xuống 2 rồi 1. Đây chỉ xác nhận lịch tải phản ứng theo cấu hình đã đặt. Một seed, một cặp vùng, một lượng hàng không đủ kết luận cấu hình nào cân bằng tốt cho toàn quốc gia; mặc định vẫn là giá trị thử 2 ngày/64 đơn vị.

Lượt thử phát hiện invariant reject lô có `arrival_day == day` dù luồng ngày có chủ ý giao lô đó vào đầu ngày kế tiếp. Điều kiện được sửa để giữ trạng thái “đến cuối hôm nay, sẽ xử lý ngày mai”; đồng thời vẫn từ chối arrival trước ngày hiện tại và arrival xa hơn thời lượng chặng đang đi. Cả Release và Debug đều chạy lại sau khi sửa.

Build command thực tế của lượt schema28 regression rộng trước đây:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/windows --config Release --target polyec_realm_transport_tests polyec_realm_profile_tests polyec_realm_tests polyec_schedule_tests polyec_national_transport_tests polyec_campaign_tests polyec_market_order_tests polyec_court_tests polyec_household_fund_tests polyec_land_title_tests polyec_realm_clock_tests
```

Trong quá trình chạy oracle shipment, test phát hiện ba lỗi và đã được sửa: phí bị nhân số lượng hai lần; lô tải hết bị giữ record lượng 0; trả lương dùng mảng 7 vùng khi chạy profile 27 vùng. Lượt hardening phát hiện và sửa điều kiện ngày đến nêu trên. Một lần thử nghiệm sai giữa chừng thất bại ở cấu hình 1 ngày do invariant loại nhầm ngày đến; lỗi được giữ làm bằng chứng, sửa theo ngữ nghĩa lịch, rồi chạy lại cả Release và Debug thành công. Không dùng lần fail này làm kết quả pass. Tất cả kiểm tra trực tiếp trong nhóm là C++ Release và Debug; native, Python, Godot và app đầy đủ chưa được kiểm.

## Bằng chứng cũ theo schema 27 và xuất xứ

Bảng sau giữ lại kết quả đã ghi lúc Realm mới lưu snapshot route ở schema 27. Nó là bằng chứng cho đúng source/run đó, không được dùng làm chứng nhận schema 28 hoặc observation trên 27 vùng:

| Target tại lượt schema 27 | Kết quả lịch sử |
|---|---:|
| `polyec_logistics_manager_tests` | 16/16 |
| `polyec_national_transport_tests` | 14/14 |
| `polyec_realm_profile_tests` | 13/13; lời gọi observation khi đó thiếu oracle nội dung, đã đính chính ở mục trên |
| `polyec_realm_tests` | 18.834 |
| `polyec_campaign_tests` | 281/281 |
| `polyec_schedule_tests` | 3.717 sau khi sửa token oracle bị thiếu header route |
| `polyec_court_tests`, `polyec_land_title_tests`, `polyec_household_fund_tests`, `polyec_market_order_tests`, `polyec_realm_clock_tests` | 565, 88, 36, 33, 301 |

Mốc này dùng Release; không chạy toàn CTest, Python, native hoặc Godot. Thời gian 10.000 người/90 ngày và Campaign 10.000 người/102 ngày ở lượt cũ là smoke workload, không phải benchmark chuẩn. Nội dung cũ được giữ để truy nguyên; không sửa hash hay dữ liệu trong artifact của run trước chỉ nhằm khớp workspace hiện tại.

## Ranh giới còn mở sau khi kiểm vận tải

- C++ Release và Debug đã kiểm save sai tham chiếu route, vị trí lô, ngày đến và tổng escrow; tất cả bị từ chối mà không thay Realm đang mở. Đã kiểm tuyến đóng/mở lại và hủy sau khi một phần đã lên đường. Phần chặn tràn ở ngân khố giới hạn lượng tải theo `INT64_MAX - treasury`; một trạng thái Realm hợp lệ không thể có tổng tiền lớn hơn `INT64_MAX`, nên không tạo test tiền giả có tổng sổ không hợp lệ chỉ để kích hoạt nhánh này. Manager có kiểm tràn phí và cấu hình; kiểm thử Realm giữ nguyên bảo toàn tiền trong giao dịch thực.
- Release và Debug đã build/run manager, Realm transport và Realm tổng hợp; độ nhạy trial-1 đã hoàn tất như một kiểm tra fixture, không hiệu chỉnh mặc định.

## Python binding và đối chiếu cùng C++ core

Sau khi core ổn định, binding `polyec_env` có thêm lệnh tạo/hủy shipment và đóng/mở tuyến, cùng cách đọc cấu hình tuyến, shipment, lô hàng, ký quỹ phí và inventory cá thể. Python chỉ gọi `Realm::execute` của C++; không tự chọn tuyến, trừ tiền, tải hàng, giao hàng hay quyết định chiến lược. Các trường inventory/ledger được trả về thành bản sao đọc được, không có setter để Python sửa trực tiếp sổ của Realm. Observation, legal-action mask, reward, số action và model đều chưa thay đổi.

Python test và chế độ `--binding-replay` của C++ runner dùng cùng seed 804, Realm 27 vùng, cùng cặp route hai chiều đầu tiên, cùng actor Economic, cùng lệnh mua 5 Iron, cùng lệnh gửi và cùng 10 ngày mô phỏng. Python lưu/tải sau ngày đầu rồi tiếp tục; C++ chạy liền. Cả hai cho cùng fingerprint **3514088912245713629** và cùng checkpoint: người gửi còn **203 đồng**, inventory Iron **5**, ngân khố nhận phí **10.047 đồng**, tổng tiền Realm **1.103.600 đồng**, không còn lô đang đi, shipment giao **5**, hủy **0**, escrow **0**, invariant đúng. Điều này đối chiếu đúng một chuỗi/seed đã định, không chứng nhận toàn bộ trainer hoặc policy.

Smoke Python đạt **28 checks**: mua hàng thật; tạo chuyến qua binding; inventory giảm khi gửi; lô xuất hiện sau ngày tải; round-trip save/load giữa đường; tiếp tục đến nơi; tổng tiền được bảo toàn; hàng và phí kết thúc đúng chỗ; checkpoint khớp C++ runner; từ chối sequence cũ, thiếu hàng và operation/goal không hợp lệ mà fingerprint không đổi. Extension Release được build từ source hiện tại tại `build/m5-python-ai/Release/polyec_env.cp312-win_amd64.pyd`.

Lệnh build và chạy thực tế:

```powershell
& 'C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe' --build build/m5-python-ai --config Release --target polyec_env
$env:PYTHONPATH = (Resolve-Path 'build/m5-python-ai/Release').Path
& 'C:\Users\Hi\AppData\Local\Programs\Python\Python312\python.exe' ai/tests/transport_binding_checks.py --replay build/windows/core/Release/polyec_realm_transport_tests.exe
```

Một lần smoke đầu thất bại vì test cố gửi thêm một đơn vị sau chuyến giao, trong khi người gửi vừa nhận lại đủ 5 đơn vị; Realm chấp nhận lệnh hợp lệ. Oracle được sửa thành gửi 6 đơn vị khi chỉ có 5, rồi chạy lại đạt 28 checks. Không thay luật C++ vì lần đầu không phải lỗi sản phẩm. Python binding hiện gọi được vận tải qua cùng C++ core; vẫn chưa có policy action/observation cho vận tải Atlas, Godot/native UI hoặc app build.

- Python đã đối chiếu một chuỗi mua/gửi/giao với C++ runner. Chưa tích hợp vận tải vào action, observation, reward hoặc model của policy; observation v11 vẫn không hỗ trợ Atlas 27 vùng.
- Native/Godot/UI chưa cho người chơi phát lệnh vận tải, và app build đầy đủ chưa được kiểm. Không thể coi chức năng này đã xuất hiện trong game.
- Planner chưa xét số chuyến đang chờ. Đủ 16 nhóm hàng, hợp đồng, ngoại thương, dân số 24+3 và các gate M5 khác vẫn còn trong backlog.
- Có guard giới hạn phí theo khoảng còn trống của ngân khố, nhưng không có test Realm trực tiếp làm ngân khố gần `INT64_MAX`: với invariant tiền đang được bảo toàn và tổng Realm tối đa `INT64_MAX`, không thể tạo tình huống hợp lệ mà riêng một ngân khố cộng một khoản phí vượt tổng đó. Tránh dựng fixture làm sai tổng tiền chỉ để ép nhánh bảo vệ; phần này được ghi rõ là giới hạn kiểm chứng.

Tự review source và test trong tab chính; không có reviewer độc lập cho nhóm này. Không commit, push hoặc publish.
