# Kế hoạch nhóm M5: vận tải hàng hóa quốc gia

Ngày lập: 2026-10-22. Kế hoạch này thực hiện quyết định trong `docs/IMPLEMENTATION_PLAN.md` §15.5. Đây là một nhóm chức năng duy nhất; các lát bên dưới chỉ là thứ tự triển khai và kiểm tra của cùng chuỗi vận tải. Quy tắc người chơi sẽ thấy: người có hàng và tiền đặt chuyến từ vùng hiện tại tới một vùng đích; hàng được giữ khỏi kho ngay khi lệnh được nhận, đi theo các cạnh atlas hợp lệ trong giới hạn sức tải từng ngày, trả phí thật cho ngân khố được cấu hình, rồi chỉ được cộng vào kho người sở hữu khi đến đích. Save giữa đường phải tiếp tục cùng kết quả.

## Trạng thái sau khi triển khai — 2026-10-22

Các lát C++ về profile Atlas 27 vùng, cấu hình tuyến, lệnh chuyến, hàng/tiền giữ, tải theo ngày, save/load, xử lý người gửi qua đời và queue snapshot dọc tuyến ứng viên đã hoàn tất; xem [report vận tải](../reports/M5-national-transport-evidence-2026-10-22.md) và [report T3.8](../reports/M5-T3.8-visible-transport-queue-evidence-2026-10-22.md). Binding Python đã chạy chuỗi vận tải và đối chiếu với runner C++. Các mục “Lát 1–5” phía dưới là contract/kế hoạch ban đầu và bằng chứng cần đạt, không phải danh sách việc còn chưa làm. Phần còn mở gồm dự báo lot đang đi/sẽ đến sau snapshot, UI vận tải, policy/model cho Atlas và các kiểm/package M5 cấp sản phẩm. Nhóm hiện hành ở [CURRENT_TASK](../CURRENT_TASK.md).

## Phạm vi và quyết định đã review

- Dùng 27 ID vùng của atlas. Không thêm cạnh hình học hoặc vùng mới. Năm cạnh `RoyalDomains` được bật hai chiều như đường bộ theo cấu hình thử; đây là giả định game được ghi trong IMPLEMENTATION_PLAN và có thể cấu hình lại.
- Sông dùng duy nhất chiều `from → to` của atlas. Không tự coi chiều ngược là tuyến hợp lệ. Đường nâng cấp, ven biển và hành lang đông/bắc/tây là hai chiều; hành lang chưa ghi loại dùng giá trị thử đường bộ. Tuyến ven biển dùng thời gian thử 2 ngày cho mỗi cạnh vì chưa có hiệu chỉnh riêng.
- Giá trị khởi đầu theo cạnh: sông xuôi dòng 1 ngày; các tuyến khác 2 ngày; 64 món/ngày, tính chung hai chiều; phí 1 đồng bạc/món/cạnh. Hệ số ghi đè theo cạnh là dữ liệu config phiên bản, không suy từ kilômét.
- Miền cấu hình để tránh tràn khi ghi ngày đến: thời gian một cạnh từ 1 tới 365.000 ngày; sức tải 1 tới 1.000.000 đơn vị/ngày; phí trên một đơn vị không âm và mọi tổng/phép nhân phải biểu diễn được bằng `int64_t`. Lô không được tải nếu ngày tới vượt chân trời lịch 365.000 ngày của Realm.
- Mỗi chiều trả phí vào ngân khố của vùng khởi hành qua cạnh. Lệnh giữ đủ phí dự kiến từ tiền cá nhân; mỗi lô chỉ trả khi được tải lên cạnh. Hủy phần hàng đang chờ tại cùng vùng chỉ hoàn lại phí chưa chi. Người gửi phải có mặt tại vùng để nhận hàng hủy. Lô đã đi trên cạnh không bị xóa; nó đến vùng kế tiếp rồi người gửi mới có thể tiếp tục hủy tại đó.
- Lựa chọn dịch vụ của policy: nhanh nhất hoặc phí thấp nhất. Planner chỉ thực hiện lựa chọn đó và tie-break xác định; không tự chọn chiến lược hay giá trị “an toàn” khi chưa có dữ liệu rủi ro.
- T3.8 thêm queue snapshot lô đang chờ theo từng link vào ước tính ngày dọc tuyến ứng viên; queue hai chiều được cộng chung và queue của cạnh sau được giảm theo sức tải có thể dùng trong thời gian di chuyển. Lô đang đi và chuyến đến sau snapshot chưa được dự báo. Thời gian giao thật vẫn do lịch tải từng ngày quyết định và có thể muộn hơn. Chi tiết và oracle xem plan/report T3.8.
- Mọi command được preflight trước commit. Hàng, tiền, ký quỹ, công suất cạnh và phí phải cân bằng; thiếu người nhận phí, hướng không hợp lệ, thiếu tiền/hàng, sức tải hoặc phép tính tràn đều reject nguyên tử.

## Quyền sở hữu và chu trình

`Realm` sở hữu danh sách cấu hình tuyến và lệnh chuyến. Khi tạo lệnh, Realm kiểm người gửi còn sống, vùng nguồn hiện tại, vùng đích, hàng, số lượng, policy mục tiêu và đủ hàng/tiền; sau đó trừ hàng khỏi inventory cá nhân, chuyển lượng phí dự kiến từ cá nhân vào sổ ký quỹ vận tải và ghi một shipment ID duy nhất. Hàng trong shipment không thuộc kho vùng hay inventory cá nhân; Realm cộng số đó vào kiểm tra bảo toàn hàng đang giữ.

Mỗi ngày, Realm xử lý lô đang đến trước, rồi các lệnh theo ID tăng dần. Lô đích đến cuối đường được cộng vào inventory người gửi đúng một lần. Lô đến vùng trung chuyển trở thành hàng chờ ở vùng đó. Với mỗi cạnh không đóng, dùng sức tải còn lại chung của ngày cho cả hai hướng; nạp tối đa sức tải, phần dư chờ ngày sau. Phí nhân lượng lô được chuyển từ ký quỹ sang ngân khố của vùng khởi hành của cạnh. Nếu tuyến sau bị đóng, hàng dừng ở vùng trung chuyển và không dịch chuyển. Người gửi chỉ được hủy lượng đang chờ tại vùng mình đang đứng; hàng đã đi vẫn tiếp tục chặng hiện tại. Khi chuyến hoàn tất hoặc hủy xong, số dư phí ký quỹ còn lại được hoàn đúng chủ thể.

Chọn tuyến khi tạo lệnh từ cấu hình cạnh mở và có hướng. `Fastest` tối thiểu hóa ngày đến, `LowestFee` tối thiểu hóa tổng phí rồi tie-break theo ngày đến; cùng kết quả thì chọn chuỗi ID vùng/cạnh nhỏ hơn. Sức tải xác định ngày tải và tới; planner/Realm dùng chung quy ước: lệnh nhận trong ngày D bắt đầu có thể tải từ ngày D+1, lô tải ngày L tới vùng kế tiếp vào cuối ngày `L + travel_days`. Lệnh gửi trong ngày D bắt đầu tải sớm nhất ở ngày D+1; lô tải ngày L đến cuối ngày `L + travel_days`. Vì vậy tổng ngày từ lúc nhận lệnh tới lô cuối đến đích gồm một ngày chờ tới lượt tải, thời gian đi từng cạnh và các ngày chia tải. Mỗi ngày tải tối đa `capacity_per_day`; hàng không bị tính thành đủ một ngày công suất nếu lượng nhỏ hơn.

## Lát 1 — planner có hướng, giữ tương thích Campaign

- File: `core/include/polyec/logistics_manager.hpp`, `core/src/logistics_manager.cpp`, `core/tests/test_logistics_manager.cpp`.
- Thêm thuộc tính cho phép đi ngược cạnh; mặc định `true` để các route initializer và Campaign hiện hành giữ hành vi. Cạnh sông quốc gia đặt `false`. Planner không xét hàng xóm ngược khi cờ tắt; reservations vẫn tham chiếu cùng cạnh không hướng để công suất hai chiều được chia sẻ.
- Test: đường hai chiều cũ vẫn đi được cả hai hướng; route một chiều đi xuôi được và bị chặn ngược; nhiều cạnh một chiều; capacity split/tie hiện có không đổi.
- Kiểm ngay: build/run `polyec_logistics_manager_tests` Release; build/run `polyec_campaign_tests` Release và xác nhận cùng fixture route trước/sau như report Campaign hiện tại. Không chạy full core/UI.
- Ranh giới: chỉ đóng lát này khi planner test và Campaign pipeline liên quan cùng pass. Nếu Campaign thay đổi kết quả, sửa tương thích trước khi mở lớp atlas.

## Lát 2 — cấu hình cạnh quốc gia và tìm tuyến

- Tạo `NationalTransportManager` thuần để ánh xạ 31 AtlasLink sang directed edge config, kiểm version/phạm vi, xác định người nhận phí, chọn tuyến theo `Fastest`/`LowestFee` và tính phí bằng checked integer. **Bối cảnh lúc lập kế hoạch:** Realm khi đó chỉ chấp nhận profile 1, 2 hoặc 6 vùng; profile Atlas 27 vùng và các giới hạn save/codec liên quan là điều kiện trước khi nối lệnh vận tải. Điều kiện này đã được thực hiện theo report vận tải ở đầu tài liệu. Không coi route manager thuần một mình là tích hợp Realm.
- Cấu hình chứa atlas link index, directed endpoints, loại đường, open, ngày đi, capacity/day, fee/unit, recipient account type và recipient region. Bản config mặc định tạo từ atlas, sau đó ghi đè theo cạnh; dữ liệu cuối cùng được copy vào Realm. `RoyalDomains` là 5 cạnh hai chiều thử; config lỗi, trùng hoặc thiếu ngân khố hợp lệ bị từ chối.
- Files dự kiến: header/source mới `national_transport_manager.hpp/.cpp`, CMake, test manager mới. Chỉ thêm config/schema vào Realm sau khi pure manager có test oracle.
- Oracle độc lập dựng mạng nhỏ; kiểm river reverse, road/coastal, royal override, closed edge, route không tới, fast/cheap khác nhau, fee × quantity và overflow. Không đo lại Campaign planner ngoài regression đã chạy ở lát 1.

## Lát 3 — profile Realm 27 vùng

- Khảo sát giới hạn region trong `Realm::Realm`, `Realm::deserialize`, `body_state_section_codec.hpp`, `work_credit_section_codec.hpp`, invariant và các index cố định. Mở hỗ trợ đúng profile 27 vùng dùng atlas; không nới mọi số tùy ý nếu các cấu trúc khác vẫn chưa an toàn.
- Bối cảnh lúc lập kế hoạch: Atlas transport dùng ID 0–26 trong khi Realm khi đó đánh số vùng chơi 0–`regions_`; cần xác nhận mọi ID atlas 0–26 có Province và actor-region hợp lệ. Giữ dữ liệu/save profile cũ 1/2/6. Mạng `routes_` 7×7 của Campaign phải tiếp tục chỉ dùng trong profile hiện hành; không cho profile 27 truy cập mảng đó ngoài giới hạn. Các kiểm tra sau khi mở profile được nêu ở report vận tải liên kết đầu tài liệu.
- File thay đổi lấy theo kết quả khảo sát: `core/src/realm.cpp`, `core/src/realm_save.cpp`, codec có giới hạn region, và test Realm/save riêng. Chạy Realm test và một profile Campaign liên quan; thêm test tạo/lưu/tải profile 27 và reject save region ngoài miền.
- Ranh giới: trước khi pass kiểm profile 27 và các profile cũ, chưa nối lệnh quốc gia vào Realm.

## Lát 4 — lệnh vận tải, tiền ký quỹ và hàng theo lô

- `Realm` nhận public `TransportCommand` cho Create/Cancel; actor và sequence theo command convention. Command Create có source/destination, GoodId, quantity và service goal. Realm xác thực route trước mọi thay đổi rồi tạo shipment và lô; xử lý reject phải giữ fingerprint, inventory, cash, escrow và ID kế tiếp.
- Add lô mang shipment ID, route edge, quantity, depart day và arrival day. Mỗi ngày capacity dùng chung hai chiều; xử lý có thứ tự shipment/edge ổn định. Giao tới cá nhân cuối tuyến đúng một lần. Phí từng lô không vượt phần ký quỹ; ngân khố nhận phí không âm và không overflow.
- Cancel chỉ giải phóng lô chưa tải hoặc đang chờ ở vùng có người gửi hiện diện; hoàn đúng lượng tới inventory người gửi tại vùng đó và hoàn phí chưa chi. Lô đã ở giữa chặng được phép hoàn tất; không teleport hoặc xóa hàng.
- Thêm kiểm Realm goods invariant và money balances để tổng hàng/tiền không đổi ngoài chuyển tài khoản. Test success, insufficient inventory/cash, missing fee recipient, river reverse, closure, shared capacity both directions, fee overflow/recipient overflow, cancel partial và duplicate command.

## Quyết định về tử vong của người gửi

Nếu người gửi qua đời khi shipment còn hàng chờ hoặc đang đi, shipment tiếp tục thuộc tài sản của người đã mất thông qua `EstateCase`. Hàng đang đi không được chuyển ngay về kho cá nhân/ngân khố. Lệ phí đã chi vẫn tới ngân khố; phí còn giữ trong ký quỹ tiếp tục dùng cho lô sau. Hàng được giao sau khi người mất qua đời được thêm vào inventory của EstateCase. Phân phối tài sản và trả nợ của vụ di sản đó tạm đợi cho tới khi tất cả shipment của người mất đã giao hoặc đã bị hủy hợp lệ; luật xác định người thừa kế và tỷ lệ phân phối không thay đổi, chỉ thời điểm quyết toán vật lý bị trễ.

Realm có thể nhận ra một di sản còn đang vận chuyển bằng cách kiểm tra các shipment mang người mất làm beneficiary; không cần cờ chờ riêng có thể lệch với danh sách shipment. Chỉ sau khi mọi shipment của người đó đã giao hoặc hủy hợp lệ mới chạy cash claims và distribution. Nếu tuyến đóng làm hàng đứng chờ, hàng cùng khoản phí chưa chi được giữ nguyên trong shipment và di sản; có thể mở lại tuyến bằng command rồi tiếp tục. Không teleport hoặc xóa tài sản. Test phải chứng minh death→save/load→arrival→inheritance, bảo toàn tiền/hàng và không phân phối lặp.
## Lát 5 — ngày mô phỏng, save schema và migration

- Tích hợp `process_transport_day` theo thứ tự cố định trong `RealmDayPipeline`; game pause/partial ticks không tải hàng. Update `fingerprint()` bao gồm config, shipment, batches, escrow và ngày đã xử lý.
- Nâng `POLYEC_REALM_27` lên `POLYEC_REALM_28`; thêm section shipment độc lập, không sửa lại ý nghĩa section route đã phát hành trong schema 27. Load schema 7–27 tạo shipment rỗng và giữ/migrate tuyến theo quy tắc đã có. Schema 28 reject reference, quantity, phí đang giữ, route, lô hàng, chủ hàng/di sản và mốc ngày không hợp lệ trước khi thay Realm đích.
- C++ test save/load khi một lô đang đi, trước chặng hai, khi hàng chờ vì route đóng, và sau khi đã giao. So fingerprint + tiền + goods với world chạy liền; chạy thêm ngày để chứng minh không tính phí hoặc giao hàng lần hai. Dùng fixture legacy schema 26 cho migration.

## Kiểm tra nhóm ở mốc ổn định

- Mỗi lát build đúng CMake target manager/Realm và chạy test trực tiếp; với thay đổi `advance()` chạy một Campaign hoặc Realm pipeline liên quan để bắt sai thứ tự ngày. Không chạy `ALL_BUILD`/full CTest sau mỗi lát.
- Kết thúc core group: Release `polyec_national_transport_tests`, Realm tests và các regression manager bị tác động; Debug manager + Realm do có số học/save. Kiểm trực tiếp từ purchase/inventory → command vận tải → tải theo ngày → phí → tới nơi → save/load.
- Thực hiện sensitivity 0.5×/1×/2× capacity và travel time trên cùng world seeds, ghép cặp policy/baseline nếu có AI evaluation. Lưu raw data và biểu đồ từ đúng run; không gọi reward rollout là điểm holdout. Chưa nối adapter nào thì nêu rõ và chuyển kiểm consumer tới mốc binding/native/Godot gọi API đó.
- Report phải giải thích cho người đọc: ai sở hữu hàng ở từng giai đoạn, khi nào tiền bị giữ/chi/hoàn, hàng đi chậm hoặc dừng ở đâu, ai nhận phí, các kết quả test thật và phần adapter/AI chưa được nối. Tự review trong tab chính; không tuyên bố reviewer độc lập.

## Tình trạng review

Kế hoạch này đã được tự kiểm trong tab chính dựa trên `CURRENT_TASK`, IMPLEMENTATION_PLAN §15.5, `LogisticsManager`, `world_atlas.hpp`, `Realm` inventory/money và save schema hiện tại. Phạm vi kế hoạch không đổi ownership/policy, không thêm chiều ngược sông và không giả định có người thu phí ngoài ngân khố vùng. Lát đầu tiên có phạm vi file nhỏ, sẽ được kiểm trước khi tiến đến Atlas/Realm.

## Kế hoạch và kết quả — Python binding cho lệnh vận tải

Lát này đã hoàn tất; xem kết quả và lệnh kiểm tra thực tế trong [report vận tải](../reports/M5-national-transport-evidence-2026-10-22.md). Nội dung bên dưới giữ lại phạm vi, fixture và tiêu chí đã dùng để đối chiếu.

### Mục tiêu và ranh giới

Expose lệnh vận tải Realm đã có trong `bindings/python/realm_binding.cpp` để Python gọi thẳng `Realm::execute(TransportShipmentCommand)`. Python chỉ truyền dữ liệu lệnh và đọc kết quả do C++ core quyết định; không lập route, trừ tiền, chuyển hàng hoặc mô phỏng tải trong Python. Lát này không thay observation/action slots, reward, version model, policy, gameplay hoặc UI. Realm 27 vùng vẫn không dùng policy v11 do observation v11 chưa hỗ trợ profile này.

### File và cách gọi

- `bindings/python/realm_binding.cpp`: thêm binding cho `transport_shipment_command`, đọc shipment ledger, lô hàng, tuyến cấu hình, escrow, và inventory cá thể để caller kiểm tra kết quả. Các giá trị enum truyền bằng số phải được ép về kiểu C++ rồi validation thật vẫn do `Realm::execute` thực hiện; tuple kết quả giữ nguyên thành công/lý do/fact như binding command hiện hành. Không expose con trỏ mutable hoặc cho Python sửa sổ.
- `core/tests/test_realm_transport.cpp`: thêm chế độ runner chỉ chạy fixture tích hợp Python: cùng seed, cùng actor Economic, cùng lệnh Buy Fiber → gửi 5 Fiber → advance 10 ngày; xuất checkpoint số nguyên để Python so fingerprint, tiền cá nhân, hàng, ngân khố, tổng tiền, số lô, trạng thái giao/hủy, escrow và invariant. Chế độ test thường vẫn chạy toàn bộ oracle transport.
- `ai/tests/transport_binding_checks.py`: dựng cùng fixture qua `polyec_env`, so checkpoint Python với runner C++ cùng source/config/seed, kiểm inventory rỗng khi đang đi và được trả đúng 5 khi hoàn tất, save/load fingerprint, lệnh không đủ hàng bị reject mà fingerprint/sequence không đổi.

### Ownership, lỗi và kiểm tra

Lệnh thành công vẫn đi qua C++ preflight/transaction; C++ giữ ownership hàng/fee escrow và chỉ chuyển hàng/phí theo lịch Realm. Caller Python không có quyền thay trực tiếp money, inventory hay shipment state. Lệnh thiếu hàng, sequence cũ hoặc enum/đích sai phải trả reject từ Realm và không làm đổi fingerprint. Trước khi triển khai, API C++ vận tải đã qua Release/Debug 88 checks; bước này chỉ đổi adapter Python.

Kiểm ngay: build `polyec_env` Release trong cấu hình `build/m5-python-ai`; chạy script mới với extension Release và executable C++ test runner Release. Không chạy full CTest, huấn luyện, Godot/native hoặc app. Nếu binding/header làm thay đổi ABI, smoke này dùng đúng extension vừa build trên source hiện tại. Ranh giới đóng lát là checkpoint Python và C++ trùng, round-trip cùng fingerprint, hàng/phí/invariant đúng, reject không mutation. Sau đó chọn backlog M5 còn thiếu tiếp theo; các gate learned policy, native/UI và packaging tiếp tục mở theo plan lớn.



