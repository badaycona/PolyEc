<!-- HISTORICAL SNAPSHOT ONLY: not active instructions. -->
# CURRENT_TASK — bàn giao M5

## Cập nhật mới nhất — chặn tràn số trong tiền mua/bán và thuế

Chợ hiện tính tổng mua/bán qua phép nhân đã kiểm tra miền; thuế dùng cách chia thương/dư cho ra cùng kết quả mà không nhân một tổng tiền rất lớn với thuế suất. Realm kiểm tra trước sổ biên nhận thuế và các lần cư dân mua lương thực cuối ngày, để tiền/hàng không bị ghi dở khi bộ đếm thuế hết miền. Giá mua/bán, phần lẻ thuế và hành vi hợp lệ giữ nguyên. Windows Release: helper **19 checks**, Realm **21.086 checks** (profile A 10.000 người/19.743 quan hệ thưa/90 ngày), chuỗi xưởng/chợ **217 checks**; các runner C++ pass. Binding Python được build lại và cùng C++ thực hiện chuỗi chợ **23/23 checks mỗi lượt, ba lượt liên tiếp**; fingerprint toàn Realm và sổ tiền/hàng khớp. Chỉ đổi core C++ và bài kiểm C++/Python; binding API, schema/action/observation không đổi. Đây là sửa độ bền số học và parity một chuỗi, không bổ sung thị trường theo cầu/giá tháng hoặc đóng M5.2. [Kế hoạch](design/M5-market-checked-arithmetic-plan.md), [báo cáo đầy đủ](reports/M5-market-checked-arithmetic-evidence-2026-10-22.md), [manifest](../artifacts/m5-market-checked-arithmetic-2026-10-22-manifest.json).
## Cập nhật mới nhất — nối lệnh mua hàng, làm tại xưởng và giao dịch sản phẩm qua chợ

Một test C++ duy nhất giờ chạy chuỗi bằng lệnh Realm thật: cư dân mua hai Fiber, lập và cấp vốn cho xưởng, chuyển Fiber vào xưởng, gửi lịch Work, lưu sau 16 tick rồi tải và tiếp tục sản xuất. Xưởng có một batch Cloth theo capacity hiện tại; chủ xưởng rút và bán một Cloth, cư dân trưởng thành khác mua lại, thuế và phần dư thuế khớp phép tính từ giá/thuế thật ngay trước lệnh. Test đối chiếu tồn kho, tiền, invariant, fingerprint sau nhánh save/load giữa ca và sau save/load cuối. Windows Release `polyec_establishment_transfer_tests` build/run thành công, **217 checks**. Chỉ sửa bài test, không đổi gameplay/schema/API/adapter. Điều này chứng minh chuỗi C++ dùng driver lệnh, không phải learned policy và không đóng thị trường/vận tải M5.2. [Kế hoạch](design/M5-command-driven-workshop-to-market-chain-plan.md), [báo cáo đầy đủ](reports/M5-command-driven-workshop-to-market-chain-evidence-2026-10-22.md), [manifest](../artifacts/m5-command-driven-workshop-to-market-chain-2026-10-22-manifest.json).

## Ưu tiên hiện tại — hoàn thiện thế giới M5 trước khi quay lại RL

Theo chỉ thị mới, chưa train lại cho tới khi hoàn tất T1–T3 cùng luật hoạt động và chu trình gameplay M5.0/1–M5.4 trong C++ core: (1) lịch/time và các luật hoạt động gắn với T1; (2) hộ, ownership, di sản, cơ sở, hợp đồng, đủ 16 nhóm hàng, vận tải/thị trường; (3) T3 gồm sleep/exertion/recovery, care/lifecycle, personality/knowledge theo contract; (4) tổ chức, chức vụ, hôn nhân, claims/nợ, giáo hội; (5) quốc gia 24+3 vùng, thành thị, quân sự, ngoại thương và các chuỗi nguyên nhân–kết quả. Sau đó phải đưa T1–T3 và mọi luật M5.0/1–M5.4 mà policy cần biết vào Python training adapter, rồi chạy smoke nhỏ qua C++ → Python để kiểm observation, legal actions/action slots, reward và việc gọi cùng C++ core; không train bằng adapter/model cũ hoặc thiếu luật mới. Đây chỉ là điều kiện bắt đầu training, không phải nghiệm thu M5. M5.5 — profile B đầy đủ, stress/long-run, hiệu năng toàn chu trình và Windows package — cùng UI/native presentation/full app builds được làm sau training, nhưng vẫn cần cho kết luận toàn bộ M5 hoàn tất. Không dùng việc hoãn presentation hoặc build lớn để bỏ luật core hay khả năng policy quan sát/chọn hành động. Core refactor R1–R4 đã được xác nhận trên Windows theo các bằng chứng bên dưới nhưng không thay cho hoàn tất M5. Chi tiết điều kiện và kiểm tra: [M5 integration gates](design/M5-integration-and-tuning-gates.md).

## Cập nhật mới nhất — Campaign dùng bộ lập kế hoạch tuyến C++ dùng chung

Tách phép tính đường, chia lô theo sức tải, đặt chỗ ngày tải và thời gian đến ra `LogisticsManager`; Campaign M4 chuyển dữ liệu bảy vùng và hàng chưa giao vào helper, rồi giữ nguyên công thức phí cũ. Test khóa đường 6→0→2→1, ngày đến 16/phí 345 trước khi thương lượng, và ngày đến 7/phí 165 sau khi dỡ quyền thu phí/cải thiện sức tải; tie cùng ngày giữ kết quả xác định theo ID vùng. Helper đạt **14/14 checks**, Campaign **281/281** trong Windows Release. Không đổi gameplay hay save; atlas chưa được dùng vì chưa có thời gian/sức tải/phí tuyến quốc gia. Đây là refactor thuật toán C++ để tái dùng, không đóng vận tải M5.2. [Kế hoạch](design/M5-logistics-path-planner-extraction-plan.md), [báo cáo đầy đủ](reports/M5-logistics-path-planner-extraction-evidence-2026-10-22.md), [manifest](../artifacts/m5-logistics-path-planner-extraction-2026-10-22-manifest.json).

## Cập nhật trước đó — truy vấn được đường nối đã khai báo trong atlas

Atlas C++ giờ tìm được chuỗi ID vùng qua các liên kết có hướng xác định: sông chỉ đi xuôi theo chiều đã ghi, đường hai chiều đi cả hai phía; liên kết triều đình chưa rõ hướng không bị đoán. Test xác nhận đủ 576 cặp điểm đi/đến trong 24 lãnh địa chư hầu có đường nối hợp lệ; khi tạm đóng đường 5↔4, truy vấn tìm đường vòng qua tuyến bắc; khi đóng cả 31 liên kết, không trả đường giả. Test cũng kiểm đường sông 0→2→1→8, hướng ngược sông, đường hai chiều, đích trùng nguồn và ID ngoài atlas. Windows Release target `polyec_world_atlas_tests` build và runner đều thoát mã 0. Đây là truy vấn topology của dữ liệu tĩnh, không tính đường vận tải gameplay, thời gian, phí, sức tải hay tình trạng mở tuyến; chưa nối vào Realm hoặc UI. [Kế hoạch](design/M5-atlas-explicit-topology-query-plan.md), [báo cáo đầy đủ](reports/M5-atlas-explicit-topology-query-evidence-2026-10-22.md), [manifest](../artifacts/m5-atlas-explicit-topology-query-2026-10-22-manifest.json).

## Cập nhật trước đó — cứu trợ làm đóng hồ sơ thiếu lương thực trong Campaign

Campaign seed 37, 512 người mở hồ sơ thiếu sinh kế ở vùng 1 sau thời gian công bố hai ngày. Lệnh cứu trợ gửi 500 lương thực qua tuyến; test lưu/tải khi hàng còn đang đi, tiếp tục tới ngày đến, xác nhận ghi nhận đúng 500 đơn vị và hồ sơ đóng trong ngày giao khi dự trữ vượt ngưỡng hiện có. Nhật ký được kiểm sau đủ hai ngày công bố; mã tóm tắt toàn trạng thái của bản tải lại khớp bản chạy liền. Windows Release target `polyec_campaign_tests` đạt **280/280 phép kiểm**; runner kèm workload 10.000 người/102 ngày (~429 ms, smoke chứ không phải benchmark). Chỉ thêm test, không đổi gameplay/API/save. Chỉ là chuỗi C++ của Campaign M4 sáu vùng, chưa có kiến nghị của dân hay quốc gia M5 đủ 27 vùng. [Kế hoạch](design/M5.4-relief-case-resolution-chain-plan.md), [báo cáo giải thích đầy đủ](reports/M5.4-relief-case-resolution-chain-evidence-2026-10-22.md), [manifest](../artifacts/m5.4-relief-case-resolution-chain-2026-10-22-manifest.json).

## Cập nhật trước đó — xưởng thừa kế bán hàng ra chợ và phát sinh thuế mua hàng

Test Realm C++ nối chuỗi chủ xưởng chết → trả khoản vay từ di sản → người thừa kế nhận xưởng → ca làm tiếp tục, được trả lương và tạo 2 Cloth từ 2 Fiber → chủ mới bán một Cloth vào chợ → cư dân khác mua và thuế mua hàng vào ngân khố. Test kiểm từng số dư người/xưởng/chợ/ngân khố, phần lẻ thuế, tổng tiền và lưu/tải. Windows Release `polyec_realm_tests` **21.086 phép kiểm** pass với workload profile A 10.000 người/19.743 quan hệ thưa/90 ngày; không phải profile B. Chỉ thêm test C++, không đổi luật gameplay hoặc API. Đây là một đoạn của chuỗi M5.4, chưa kiểm guild/thành viên hay policy tự chọn; M5.2/M5.4 còn mở. [Kế hoạch](design/M5.4-workshop-inheritance-market-tax-chain-plan.md), [báo cáo đầy đủ](reports/M5.4-workshop-inheritance-market-tax-chain-evidence-2026-10-22.md), [manifest](../artifacts/m5.4-workshop-inheritance-market-tax-chain-2026-10-22-manifest.json).

## Cập nhật mới nhất — chặn cưới giữa anh chị em có cha/mẹ chung

Trước đây một số anh chị em có chung cha/mẹ nhưng không có cạnh `Sibling` trực tiếp trong dữ liệu, nên action mask có thể để lọt cặp nằm hai đầu chuỗi anh chị em. Realm hiện dựng anh chị em từ mọi con của cùng cha hoặc mẹ, vẫn giữ các cạnh `Sibling` tường minh; quan hệ mới được thêm ngay khi phát sinh và được dựng lại từ save khi tải. C++ Release: `polyec_court_tests` **565 checks**, `polyec_realm_tests` **21.081 checks** (profile A 10.000 người/19.743 quan hệ/90 ngày), `polyec_manager_tests` pass. Consumer native `polyec_ai` build thành công; `polyec_realm_contracts` đạt **24 checks** và từ chối bundle pilot v6 đúng dự kiến; không có model v11 phù hợp để chạy inference. Cấu hình Windows mặc định tắt Python nhưng cấu hình `build/m5-python-ai` đã build binding và replay 64 ngày C++–Python đạt **273 checks**. GDExtension target `polyec` Release build pass; Godot 4.5.1 headless smoke đạt **5/5** kiểm tra model guard, nhưng không thao tác màn hình cưới. Không đổi save schema hay kích thước action/observation. [Kế hoạch](design/M5-core-marriage-common-parent-sibling-plan.md), [báo cáo đầy đủ](reports/M5-core-marriage-common-parent-sibling-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-marriage-common-parent-sibling-2026-10-22-manifest.json).

## Cập nhật mới nhất — cấm kết hôn giữa tổ tiên và hậu duệ nhiều thế hệ

Realm trước đây chỉ chặn một cạnh cha/mẹ–con trực tiếp nên ông/bà và cháu có thể lọt qua. Kiểm tra hôn nhân giờ lần theo chuỗi quan hệ cha/mẹ–con để chặn tổ tiên/hậu duệ ở mọi số thế hệ; luật anh chị em, đồng thuận, tuổi tối thiểu và ký quỹ không đổi. Chỉ mục quan hệ được cập nhật khi sinh con và dựng lại khi tải game. Bài kiểm C++ xác nhận action mask, lệnh gửi đề nghị và chấp thuận đều từ chối nguyên tử; Realm save/load giữ việc từ chối đúng. Windows Release `polyec_court_tests` **557 checks** và `polyec_realm_tests` **20.846 checks** pass; runner Realm dùng profile A 10.000 người/19.743 quan hệ/90 ngày, không phải profile B. Do `Realm` đổi bố cục C++ nội bộ, đã build riêng native consumer `polyec_ai` và chạy `polyec_realm_contracts`: **24 checks** pass, bundle pilot v6 bị từ chối đúng dự kiến; không có model v11 phù hợp để chạy inference. Python tắt trong cấu hình này; Python/Godot chưa chạy. Không đổi save schema, action/observation shape hoặc binding. [Kế hoạch](design/M5-core-marriage-lineal-kinship-plan.md), [báo cáo giải thích đầy đủ](reports/M5-core-marriage-lineal-kinship-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-marriage-lineal-kinship-2026-10-22-manifest.json).

## Cập nhật mới nhất — core có bộ phân giải ứng viên kế vị tước

IMPLEMENTATION_PLAN §2 quy định tước truyền cho con hợp pháp lớn tuổi nhất và chờ công nhận khi chưa có người kế vị rõ. Lõi C++ có phép tra cứu chỉ đọc để chọn con hợp pháp còn sống lớn tuổi nhất theo số ngày tuổi; con chưa thành niên vẫn được xét, chỉ cạnh cha/mẹ–con theo chiều con → cha/mẹ được xét, cạnh chưa xác nhận và người con đã chết bị bỏ qua. Nếu đồng tuổi lớn nhất, kết quả giữ đủ ứng viên và yêu cầu công nhận thay vì tự chọn. Windows Release `polyec_manager_tests` được build và chạy trực tiếp sau khi bổ sung kiểm tra ID/tuổi cư dân sai, cờ quan hệ không hợp lệ và cạnh đảo chiều; runner pass. Chưa có bản ghi tước, hồ sơ công nhận, xử lý chết hoặc lưu/tải trong Realm; vì vậy người chơi chưa có thể thấy hoặc sử dụng kế vị tước. [Kế hoạch](design/M5-peerage-successor-resolution-plan.md), [báo cáo giải thích đầy đủ](reports/M5-peerage-successor-resolution-evidence-2026-10-22.md), [manifest](../artifacts/m5-peerage-successor-resolution-2026-10-22-manifest.json).

## Cập nhật mới nhất — phân loại đặc sản cho đủ 24 lãnh địa

Atlas hiện có một phân loại cho từng đặc sản L01–L24, dựa trên các nhóm hàng ở IMPLEMENTATION_PLAN §15.2. Hai mươi mục được gán vào nhóm đã rõ, 12 trong số đó có product ID cụ thể; Tàu đi biển được ghi là tài sản vận tải. Thủy tinh, đồ sứ và da thuộc vẫn để chưa xác định vì catalog chưa đủ căn cứ cho nhóm hoặc biến thể chính xác. Windows Release `polyec_world_atlas_tests` build và direct runner thành công, test đủ 24 vùng, từng tên/nhóm/product, cấu trúc không hợp lệ và lookup khu triều đình. Đây chỉ là phân loại dữ liệu, chưa tạo nguồn cung, công thức, giá hoặc giao dịch. [Kế hoạch](design/M5-atlas-complete-specialty-classification-plan.md), [báo cáo đầy đủ có toàn bảng](reports/M5-atlas-complete-specialty-classification-evidence-2026-10-22.md), [manifest](../artifacts/m5-atlas-complete-specialty-classification-2026-10-22-manifest.json).

## Cập nhật mới nhất — atlas tham chiếu bảy đặc sản theo catalog hàng

Theo IMPLEMENTATION_PLAN §15.2, bảy ánh xạ đặc sản được ghi thành dữ liệu có cấu trúc: Len trắng → nhóm Fiber (chưa có mã sản phẩm len); Lụa nhuộm → Silk; Trà → Tea; Trầm hương → Incense; Thủy tinh và Đồ sứ → nhóm Luxury nhưng chưa chỉ định mã cụ thể vì tên vùng không xác nhận biến thể “trang trí/tốt”; Sáp ong → Beeswax. Bốn mã sản phẩm cụ thể được kiểm tra có nhóm khớp catalog. Validator từ chối vùng trùng/triều đình, mã hàng không hợp lệ và nhóm không khớp sản phẩm. Windows Release `polyec_world_atlas_tests` build và runner pass, cùng kiểm tra 27 vùng và các liên kết atlas. Đây chưa phải nguồn cung, sản xuất, giá riêng hay giao dịch; đặc sản khác chưa được tự ánh xạ. [Kế hoạch](design/M5-atlas-specialty-good-mapping-plan.md), [báo cáo đầy đủ](reports/M5-atlas-specialty-good-mapping-evidence-2026-10-22.md), [manifest](../artifacts/m5-atlas-specialty-good-mapping-2026-10-22-manifest.json).

## Cập nhật mới nhất — catalog có các liên kết vùng bắt buộc đầu tiên

Atlas C++ hiện lưu 31 liên kết theo đúng chuỗi vùng ở IMPLEMENTATION_PLAN §12.3: sông lớn 3 đoạn, đường cần cải thiện 2, ven biển 3, tuyến đông 6, bắc 5, tây 7 và 5 kết nối khu triều đình. Phương thức chỉ ghi là sông/đường/ven biển ở nơi tài liệu nói rõ; tuyến đông/bắc/tây và các kết nối triều đình để chưa xác định. Kiểm tra Release `polyec_world_atlas_tests` build và chạy trực tiếp thành công, xác nhận 27 vùng và 31 liên kết cùng số liệu catalog nền. Đây vẫn chỉ là dữ liệu atlas tĩnh; chưa có bản đồ 27 vùng trong Realm, tìm đường, thời gian/sức tải/chi phí, dòng hàng hoặc gameplay vận tải. [Kế hoạch](design/M5-world-atlas-minimum-links-plan.md), [báo cáo đầy đủ](reports/M5-world-atlas-minimum-links-evidence-2026-10-22.md), [manifest cùng mã băm và kích thước file](../artifacts/m5-world-atlas-minimum-links-2026-10-22-manifest.json).

## Cập nhật mới nhất — đủ ba công thức khởi đầu đã chạy qua Realm

Realm C++ giờ có kiểm tra tích hợp cho cả ba công thức được ghi ở IMPLEMENTATION_PLAN §15.3: Fiber → Cloth (đã có) cùng Iron + Fuel → Tools và Tools + Cloth → MilitarySupply (hai ca mới). Hai ca mới tạo xưởng đúng ngành, cấp lịch Work, lưu/tải sau 16 tick rồi tiếp tục 48 tick; kết quả khớp với bản chạy liền 64 tick. Đầu vào bị tiêu đúng một đơn vị mỗi loại, một đầu ra được tạo; chủ xưởng rút Tools/MilitarySupply sang kho cá nhân rồi bán tại chợ cùng vùng. Tiền nhận khớp quy tắc giá bán sẵn có, kho cá nhân/xưởng/chợ cùng tổng tiền đúng. Ca thiếu Fuel xác nhận Iron không bị tiêu lẻ và không tạo Tools dù công đã được xét. Windows Release `polyec_realm_tests` **20.829 checks** pass; workload kèm profile A 10.000 người/19.743 quan hệ/90 ngày, không phải profile B. Chỉ thêm test C++, không đổi luật sản xuất/thị trường. Nguyên liệu được nạp trong fixture; nguồn cung, thị trường tổng quát, vận tải, công thức khác và toàn bộ M5.2 còn mở. [Plan](design/M5-realm-all-starter-recipes-integration-plan.md), [báo cáo đầy đủ](reports/M5-realm-all-starter-recipes-integration-evidence-2026-10-22.md), [manifest](../artifacts/m5-realm-all-starter-recipes-integration-2026-10-22-manifest.json).

## Cập nhật mới nhất — phần dư nợ ngủ qua ngày và save/load

T3.2 test thêm chuỗi 16 tick thức + 48 tick nghỉ: phần dư còn `-32/64` ngày, giữ nợ ngủ ở một ngày, rồi sau save/load ngày thức kế tiếp đưa phần dư về `+32/64` mà chưa tăng nợ; ngày thức tiếp theo tăng đúng một ngày và giữ `+32/64`. Hai ca biên xác nhận ở debt 0, phần dư âm bị xóa sau ngày nghỉ; ở debt 100, phần dư dương bị xóa sau ngày thức. Realm gốc và bản tải lại có fingerprint khớp. Windows Release Realm runner **20.844 checks** pass, workload profile A 10.000 người/19.743 quan hệ/90 ngày. Chỉ thêm test, không đổi công thức hay luật sức khỏe. Chưa kiểm tra mọi chuỗi nhiều ngày tại biên, chưa là exertion/recovery và không đóng T3. [Plan](design/M5-T3.2-partial-sleep-balance-integration-plan.md), [báo cáo đầy đủ](reports/M5-T3.2-partial-sleep-balance-evidence-2026-10-22.md), [manifest](../artifacts/m5-T3.2-partial-sleep-balance-2026-10-22-manifest.json).

RL hiện chỉ còn trạng thái thí nghiệm dở dang, không phải ưu tiên hoạt động: v30 có ba seed mỗi họ Governance/Civilian nhưng selector loại cả sáu; không export model hoặc chạy holdout ([báo cáo](reports/M5-v30-governance-civilian-training-evidence-2026-10-22.md)). V31 đã hoàn tất Governance seed 1–3 ở 75 update, nhưng Civilian seed 1 dừng tại update 43/75 theo lệnh dừng trước đó; seed 2–3 và validation v31 chưa chạy. Checkpoint này được giữ lại; không resume trước khi T1–T3 và luật M5.0/1–M5.4 qua core, sau đó Python adapter/smoke C++→Python đạt yêu cầu ([manifest trạng thái dừng hiện tại](../artifacts/m5-v31-training-deferred-at-update43-2026-10-22.json), [kế hoạch v31](design/M5-v31-extended-governance-civilian-training-plan.md)).

## Cập nhật mới nhất — Realm giữ đúng recipe khi gặp biến thể cùng nhóm

Năm Realm fixture tạo xưởng, cấp quyền, gửi ca 32 tick, nạp stock kiểm thử cùng opening ledger, chạy một ngày, rồi save/load. Beeswax không thay Fiber; Copper/Silver/Gold không thay Iron khi Fuel đủ; Silk không thay Cloth khi Tools đủ. Cả năm không tạo output, không tiêu hàng thay thế/phụ trợ, ghi nhận hai work unit đã xét và giữ invariant/fingerprint sau tải lại. Windows Release Realm direct runner **20.799 checks** pass, profile A 10.000 người/19.743 quan hệ/90 ngày (không phải profile B). Chỉ đổi test, không thêm transfer/nguồn hàng hay luật sản xuất. [Plan](design/M5-realm-no-substitution-integration-plan.md), [báo cáo đầy đủ](reports/M5-realm-no-substitution-integration-evidence-2026-10-22.md), [manifest](../artifacts/m5-realm-no-substitution-integration-2026-10-22-manifest.json).

## Cập nhật mới nhất — recipe không nhận hàng thay thế cùng nhóm

Production planner đã có ba recipe dùng `GoodId` chính xác; test mới cấp đủ 16 tick tại đúng cơ sở rồi thử Beeswax thay Fiber, Copper/Silver/Gold thay Iron (giữ Fuel đủ), và Silk thay Cloth (giữ Tools đủ). Cả năm trường hợp không sản xuất, không tiêu hàng thay thế hoặc hàng phụ trợ; work unit vẫn được ghi nhận đã xét. Windows Release `polyec_production_tests` direct runner **43 checks** pass. Chỉ thêm test planner, không đổi luật sản xuất hoặc catalog. Các sản phẩm còn deferred vẫn chưa có nguồn/recipe. [Plan](design/M5-production-no-substitution-tests-plan.md), [báo cáo đầy đủ](reports/M5-production-no-substitution-tests-evidence-2026-10-22.md), [manifest](../artifacts/m5-production-no-substitution-tests-2026-10-22-manifest.json).

## Cập nhật mới nhất — validator danh mục 27 sản phẩm yêu cầu đủ 16 nhóm

`good_catalog_is_well_formed()` giờ từ chối nhóm trống, ID lệch vị trí, nhóm ngoài miền, tên rỗng/trùng, giá tham chiếu không dương và mapping bảy hàng cũ sai/trùng. Test sản xuất khóa số lượng 27 sản phẩm theo 16 nhóm như bảng hiện hành. Windows Release build `polyec_production_tests` và runner trực tiếp **28 checks** pass. Chỉ đổi validator và test catalog; không thêm nguồn hàng, recipe, giá hoặc giao dịch. Full CTest, Debug và adapter để mốc tích hợp M5.2; sản xuất đủ mặt hàng vẫn còn mở. [Plan](design/M5-good-catalog-group-invariants-plan.md), [báo cáo đầy đủ](reports/M5-good-catalog-group-invariants-evidence-2026-10-22.md), [manifest](../artifacts/m5-good-catalog-group-invariants-2026-10-22-manifest.json).

## Cập nhật mới nhất — catalog dữ liệu atlas đủ 24 lãnh địa và 3 khu triều đình

Đã thêm 27 bản ghi tham chiếu từ bảng IMPLEMENTATION_PLAN §12.2: 24 lãnh địa trong sáu cụm, dân số bối cảnh theo nghìn người, diện tích km², đặc sản và lợi thế/nút thắt; ba khu triều đình có tên/vai trò nhưng chưa có số dân/diện tích được chỉ định. Hai trường này được biểu diễn là không có giá trị thay vì số 0, tránh code downstream đọc nhầm thành số liệu thật. Dân số bối cảnh không biến thành NPC. Windows Release target atlas build và direct runner xác nhận 27/24/3, tổng dân số bối cảnh 4.185 nghìn người, tổng diện tích 212.000 km² và tổng theo từng cụm. Chỉ thêm static catalog/test, Realm mặc định vẫn 7 vùng; chưa có map, tuyến, nguồn hàng hoặc luật quốc gia hoạt động. [Plan](design/M5-world-atlas-territory-catalog-plan.md), [báo cáo đầy đủ có toàn bảng](reports/M5-world-atlas-territory-catalog-evidence-2026-10-22.md), [manifest](../artifacts/m5-world-atlas-territory-catalog-2026-10-22-manifest.json).

## Cập nhật mới nhất — hai quỹ hộ vẫn tách sau kết hôn và tử vong

C++ Realm test nối nộp 7 và 11 tiền vào hai hộ → cầu hôn/ký quỹ → lưu/tải → chấp thuận → thử rút quỹ bên kia bị từ chối → rút đúng 3 từ quỹ của mình → phối ngẫu chết → lưu/tải. Sau kết hôn hai mã hộ và số dư quỹ vẫn riêng; sau tử vong quỹ của cả hộ người sống lẫn hộ người chết vẫn ở nguyên, không vào di sản cá nhân. Tổng tiền và invariant giữ đúng. Windows Release target Court build và direct runner **546 checks** pass. Chỉ sửa test/fixture; full CTest, Debug và adapters chưa chạy lại. Đây là một kiểm tra nối luật, không đóng M5.0/1. [Kế hoạch](design/M5-household-funds-marriage-continuity-plan.md), [báo cáo đầy đủ](reports/M5-household-funds-marriage-continuity-evidence-2026-10-22.md), [manifest](../artifacts/m5-household-funds-marriage-continuity-2026-10-22-manifest.json).

## Cập nhật mới nhất — phối ngẫu nhận tiền di sản, hai quỹ hộ vẫn riêng

Realm C++ xử lý 1.000 tiền cá nhân và khoản vay 20 của người chết; phối ngẫu duy nhất nhận 980. Hai household label riêng có quỹ 37 và 23; cả hai giữ nguyên ngoài di sản. Fixture cấp sẵn một DryFood để việc mua thức ăn trong ngày không làm lệch phép đối chiếu. Windows Release Realm build và direct runner **20.744 checks** pass, profile A 10.000 người/19.743 quan hệ thưa/90 ngày. Chỉ test/fixture; full CTest, Debug và adapters chưa chạy lại. [Plan](design/M5-estate-spouse-separate-household-funds-plan.md), [báo cáo đầy đủ](reports/M5-estate-spouse-separate-household-funds-evidence-2026-10-22.md), [manifest](../artifacts/m5-estate-spouse-separate-household-funds-2026-10-22-manifest.json).

## M5.5 — đo riêng chu trình ngày C++ ở profile 100.000 người/512 vai

Thêm executable benchmark riêng để đo `Realm::advance(1)` khi chưa có bundle AI tương thích. Release build target này và chạy 31 ngày với seed 37, 100.000 người, 512 vai Governance/Economic; JSON ghi 7 vùng pilot, 198.975 quan hệ, p50/p95/lớn nhất 7,8741/10,9511/14,9784 ms, tiền được bảo toàn và invariant đúng sau từng ngày. Kiểm tiền/invariant mất thêm 473,182 ms cộng 31 ngày và được tách riêng khỏi thời gian mô phỏng. Đây chỉ là core, chưa gồm policy, giao diện, save/load, 27 vùng đích hoặc chất lượng RL; chưa thể gọi là profile B hoàn chỉnh. [Báo cáo đầy đủ](reports/M5-T5.1-core-only-100k-512-evidence-2026-10-22.md), [JSON](../artifacts/m5-t5.1-core-only-100k-512-2026-10-22.json), [manifest](../artifacts/m5-t5.1-core-only-100k-512-2026-10-22-manifest.json).

## M5.5 — benchmark 100.000 người/512 vai chưa chạy được vì model cũ không tương thích

Target native benchmark đã build Release từ source hiện tại, bật đo thời gian từng pha. Lần chạy seed 37, 31 ngày, 100.000 người và 512 vai dừng ngay khi đọc manifest: bundle v7 khai báo hợp đồng cũ, còn core hiện tại yêu cầu `realm-household-fund-goods27-v5`. Không có mô phỏng hay số đo được tạo; không dùng kết quả 100.000/128 từ ngày trước để chứng nhận source mới. Cần bundle model được train/export với hợp đồng hiện tại. Đây chưa phải kết quả profile B hoặc đánh giá chất lượng RL. [Báo cáo đầy đủ về lệnh, kết quả và giới hạn](reports/M5-T5.1-phase-profile-100k-512-blocked-2026-10-22.md), [manifest hash](../artifacts/m5-t5.1-phase-100k-512-blocked-2026-10-22-manifest.json).

## Cập nhật mới nhất — Realm từ chối kết hôn giữa cha/mẹ và con trưởng thành

Court fixture lấy quan hệ ParentChild đã lưu giữa hai người lớn còn sống; lệnh kết hôn nhắm vào người kia bị Realm từ chối. Đây là luật kết hôn theo quan hệ trực hệ, không suy ra quan hệ đó đã được xác nhận làm người thừa kế. Release và Debug `polyec_court_tests` đều **539 checks** pass. Chỉ thêm assertion test; full Release CTest 19/19 lần gần nhất có trước assertion này nhưng đã gồm test tái hôn; direct Court ở hai cấu hình có assertion mới. [Plan](design/M5-core-marriage-parent-child-exclusion-plan.md), [báo cáo đầy đủ](reports/M5-core-marriage-parent-child-exclusion-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-marriage-parent-child-exclusion-2026-10-22-manifest.json).

## Cập nhật mới nhất — người góa có thể kết hôn lại sau khi phối ngẫu chết

Realm ghi nhận phối ngẫu cũ chết mà vẫn giữ quan hệ trong hồ sơ. Người còn sống có ứng viên mới, phải đặt 50 tiền vào escrow; không có quan hệ mới trước acceptance. Người nhận mới tự chấp thuận sau thời gian chờ, nhận đúng 50, escrow về 0, tổng tiền và invariant đúng. Hai household vẫn tách biệt; chỉ một phối ngẫu còn sống. Release/Debug `polyec_court_tests` mỗi cấu hình **537 checks** pass. Chỉ thêm test helper/scenario, không đổi production/API/schema/binding; full Release CTest 19/19 có trước test này nên không gồm nó. [Plan](design/M5-core-marriage-remarry-after-death-plan.md), [báo cáo đầy đủ](reports/M5-core-marriage-remarry-after-death-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-marriage-remarry-after-death-2026-10-22-manifest.json).

## Cập nhật mới nhất — con hợp pháp được ưu tiên trước phối ngẫu

Realm death phase chạy trường hợp có con 10 tuổi với cạnh cha-con hợp pháp và phối ngẫu cùng còn sống. Người con nhận toàn bộ 100 tiền và 5 Paper trong hồ sơ; số dư của phối ngẫu giữ nguyên. Tổng tiền, hồ sơ, invariant và save/load đúng. Windows Release/Debug Realm **20.741 checks** mỗi cấu hình, profile A 10.000 người/19.743 quan hệ thưa/90 ngày; profile đo 5,77/23,02 giây. Chỉ thêm fixture/assertion, không đổi production/API/schema/binding. Full CTest 19/19 trước assertion này nên không gồm nó; adapters không chạy lại ở lát test-only. [Plan](design/M5-core-estate-child-before-spouse-realm-plan.md), [báo cáo đầy đủ](reports/M5-core-estate-child-before-spouse-realm-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-child-before-spouse-realm-2026-10-22-manifest.json).

## Cập nhật mới nhất — phối ngẫu được ưu tiên trước cha mẹ trong Realm

Realm death test hiện có cả phối ngẫu và cha/mẹ hợp pháp còn sống, không có con hợp pháp sống sót. Sau khi trả khoản vay, phối ngẫu nhận phần di sản; tiền và Fiber của cha/mẹ không đổi. Hồ sơ, tổng tiền, invariant, title/quyền sử dụng/xưởng và save/load tiếp tục đúng. Windows Release/Debug Realm **20.736 checks** mỗi cấu hình; profile A 10.000 người/19.743 quan hệ thưa/90 ngày, profile lần lượt 5,85/21,33 giây. Chỉ fixture/assertion test, không sửa production/API/schema/binding; full Release CTest 19/19 trước assertion này nên không được tính là bao gồm nó. [Plan](design/M5-core-estate-spouse-before-parent-realm-plan.md), [báo cáo đầy đủ](reports/M5-core-estate-spouse-before-parent-realm-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-spouse-before-parent-realm-2026-10-22-manifest.json).

## Cập nhật mới nhất — quan hệ cha-con chưa hợp pháp không nhận di sản

Realm death phase xử lý người có cạnh ParentChild nhưng `legal_parentage=false`: người đó không nhận tiền/hàng. Vì không có heir hợp pháp nào khác, 100 tiền đi vào balance quỹ di sản công và 5 Paper về kho Province của vùng người chết; hồ sơ, tổng tiền, invariant và save/load đúng. Windows Release/Debug Realm **20.735 checks** mỗi cấu hình, profile A 10.000 người/19.743 quan hệ thưa/90 ngày. Full CTest Release 19/19 có trước fixture-only; sau đó chỉ chạy trực tiếp Realm cả hai cấu hình. Không đổi production/schema/API/binding. [Plan](design/M5-core-estate-unverified-parentage-realm-plan.md), [báo cáo đầy đủ](reports/M5-core-estate-unverified-parentage-realm-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-unverified-parentage-realm-2026-10-22-manifest.json).

## Mốc kiểm tra core — estate/quỹ hộ/chức bổ nhiệm

Sau hai fixture kiểm thứ tự người hưởng và test người góa tái hôn sau khi phối ngẫu chết, full Windows Release CTest đạt **19/19 trong 38,54 giây**. Realm Release/Debug trực tiếp đạt **20.741 checks**; Court Release/Debug đạt **537 checks**. Đây là cùng nhóm test C++ cho death/save/command; không build ALL_BUILD hoặc chạy adapters vì production/API/save/binding/model không đổi. Full Debug CTest gần nhất có trước fixtures mới; hai runner Debug thay đổi đã chạy trực tiếp. Link checker hiện xác nhận **1.072** tham chiếu cục bộ. Chi tiết và giới hạn: [báo cáo tích hợp đầy đủ](reports/M5-core-estate-office-lifecycle-integration-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-office-lifecycle-integration-2026-10-22-manifest.json). M5 vẫn mở.

## Cập nhật mới nhất — từ chức không lấy lại tiền cá nhân

Court test dùng Realm command: từ chối từ chức trước notice mà không đổi fingerprint; sau 30 ngày, người giữ chức chủ động Resign và trở lại Civilian, `office/hired_by` được xóa, cash đúng bằng số ngay trước command, tổng tiền giữ nguyên. Lệ phí/lương đã chuyển trước đó vẫn thuộc sổ tương ứng. Release và Debug `polyec_court_tests` đều **522 checks** pass; không đổi production/API/save/binding. Đây là từ chức, không phải lệnh bãi nhiệm. [Plan](design/M5-core-office-resignation-private-cash-plan.md), [report đầy đủ](reports/M5-core-office-resignation-private-cash-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-office-resignation-private-cash-2026-10-22-manifest.json).

## Cập nhật mới nhất — chỗ chức bổ nhiệm mở lại sau tử vong

Qua Realm command, sovereign bổ nhiệm ba người vào ba vị trí; lệnh thứ tư bị từ chối nguyên tử. Khi một người giữ chức chết ở pha cư dân, số appointment đang hoạt động giảm về hai; lệnh kế tiếp bổ nhiệm được ứng viên thứ tư, còn trường chức của người chết chỉ nằm trong hồ sơ cá nhân cũ và không chuyển cho người hưởng. Save/load và tiếp tục một ngày giữ ba vị trí hoạt động, không trả lương trùng. Release/Debug Realm **20.730 checks** mỗi cấu hình, profile A 10.000 người/19.743 quan hệ thưa/90 ngày. Full CTest 19/19 hai cấu hình có trước test-only additions gần đây; không đổi production/API/schema/binding. [Plan](design/M5-core-estate-appointed-office-vacancy-plan.md), [báo cáo đầy đủ](reports/M5-core-estate-appointed-office-vacancy-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-appointed-office-vacancy-2026-10-22-manifest.json).

## Cập nhật mới nhất — người con tự thành authority khi đủ 18 tuổi

Chuỗi Realm chạy qua cái chết của người lớn, hai ngày game và command rút tiền. Ngay sau cái chết, người con còn thiếu tuổi thành niên chưa có authority và quỹ 37 vẫn khóa. Sau đúng hai ngày, tuổi đạt 18×365 ngày; Realm tự chọn người này làm authority. Hai món DryFood từ kho đã giúp loại chi tiêu ăn khỏi phép đối chiếu; người mới trưởng thành rút 5 qua command, quỹ còn 32, tiền/sổ/save-load/fingerprint đúng. Windows Release và Debug Realm **20.721 checks** mỗi cấu hình, profile A 10.000 người/19.743 quan hệ thưa/90 ngày. Full CTest 19/19 hai cấu hình có trước test-only additions gần đây; lát này chỉ chạy Realm trực tiếp, không đổi production hoặc binding. [Plan](design/M5-core-household-fund-authority-matures-plan.md), [báo cáo đầy đủ](reports/M5-core-household-fund-authority-matures-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-household-fund-authority-matures-2026-10-22-manifest.json).

## Cập nhật mới nhất — title đất theo nhánh cha mẹ/anh chị em/không người hưởng

Test Realm nối title và quyền dùng đất vào ba nhánh thừa kế đã kiểm tiền/hàng: hai cha mẹ cùng hàng thì holding không bị chia và vẫn được hồ sơ người chết tham chiếu; một anh/chị/em trưởng thành hưởng duy nhất nhận title, trở thành grantor mới nhưng người dùng holding không đổi; không có người hưởng thì title/quyền hiện tại tiếp tục ở hồ sơ, không bị cấp cho quỹ công. Save/load, fingerprint, hồ sơ và invariant đúng. Windows Release/Debug Realm **20.716 checks** mỗi cấu hình, profile A 10.000 người/19.743 quan hệ thưa/90 ngày. Full CTest 19/19 hai cấu hình có trước các bổ sung test-only gần đây; lát này chỉ chạy trực tiếp Realm target. Không sửa luật hoặc production. [Plan](design/M5-core-estate-fallback-land-realm-plan.md), [báo cáo đầy đủ](reports/M5-core-estate-fallback-land-realm-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-fallback-land-realm-2026-10-22-manifest.json).

## Cập nhật mới nhất — cha mẹ, anh chị em và quỹ di sản công qua Realm

Ba nhánh dự phòng của luật thừa kế ở IMPLEMENTATION_PLAN §2 nay được nối qua death phase C++ Realm. Hai cha/mẹ còn sống chia 100 tiền và 5 Paper; nếu cha mẹ chết thì anh/chị/em hợp pháp còn sống nhận tài sản; nếu không còn người hưởng thì 100 tiền ở balance quỹ di sản công và 5 Paper về kho Province vùng người chết. Tổng tiền, invariant và save/load/fingerprint đúng. Windows Release và Debug Realm đều **20.707 checks** pass, profile A 10.000 người/19.743 quan hệ thưa/90 ngày. Full CTest 19/19 Release/Debug có trước các test-only additions gần đây; lát này chỉ bổ sung test Realm và không chạy lại full suite/adapters. Chỉ test/fixture, không đổi gameplay source. [Plan](design/M5-core-estate-parent-sibling-public-realm-plan.md), [báo cáo đầy đủ](reports/M5-core-estate-parent-sibling-public-realm-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-parent-sibling-public-realm-2026-10-22-manifest.json).

## Cập nhật mới nhất — quỹ hộ bị khóa khi chỉ còn trẻ vị thành niên

Realm test chuyển 37 tiền từ Province firm vào quỹ hộ, rồi làm người lớn trong hộ qua đời. Trẻ 10 tuổi nhận phần tiền cá nhân theo hồ sơ di sản, nhưng truy vấn quyền rút quỹ chung trả về 0; lệnh rút 5 bị từ chối mà không đổi tiền, fingerprint hoặc sổ cái. Save/load giữ nguyên quỹ 37 riêng với tiền cá nhân. Windows Release và Debug Realm đều **20.695 checks** pass, kèm profile A 10.000 người/19.743 quan hệ thưa/90 ngày. Full CTest 19/19 hai cấu hình đã chạy trước test-only addition; sau đó chỉ chạy trực tiếp Realm target cả hai cấu hình. Không thêm quyền guardian chi tiền và không đổi gameplay. [Plan](design/M5-core-estate-household-minor-lock-plan.md), [báo cáo đầy đủ](reports/M5-core-estate-household-minor-lock-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-household-minor-lock-2026-10-22-manifest.json).

## Cập nhật mới nhất — quỹ hộ còn nguyên khi người có quyền rút qua đời

Realm fixture nối death phase với quy tắc quỹ hộ: chuyển 37 tiền từ Province firm vào tài khoản chung, làm chủ hộ chết, để Realm chọn người lớn còn sống tiếp theo, rồi người đó rút 5 qua command. Quỹ giữ 32, tiền cá nhân người rút tăng 5, estate cash về 0 sau xử lý tiền cá nhân; tổng tiền/invariant/save-load đúng. Release và Debug Realm đều **20.689 checks** pass, profile A 10.000 người/19.743 quan hệ thưa/90 ngày, không phải profile B. Lượt đầu cho thấy fixture khởi tạo có thêm người trưởng thành lớn tuổi hơn; thu gọn fixture, không đổi authority gameplay. Full Release/Debug CTest 19/19 có trước fixture này; sau đó chỉ chạy trực tiếp target Realm ở cả hai cấu hình. Không đổi production/schema/API/binding nên adapter không chạy lại. Chi tiết: [plan](design/M5-core-estate-household-authority-continuity-plan.md), [báo cáo](reports/M5-core-estate-household-authority-continuity-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-household-authority-continuity-2026-10-22-manifest.json).

## Cập nhật mới nhất — Buy rồi Sell bằng giao dịch thật trong Realm

Audit dùng **15 trạng thái Buy hợp lệ đã lưu** từ ba checkpoint Economic v23 (update 25; seed 1: 10, seed 2: 0, seed 3: 5). Đây không phải mẫu mới để train RL. Cùng C++ Realm thực hiện Buy 4 món rồi Sell 4 món trong cùng vùng/ngày: utility tăng tạm thời khi giữ hàng, nhưng khi bán thật thì cash giảm 8 và utility ròng giảm 0,08; hàng trở lại mức đầu, quỹ hộ không đổi, tổng tiền và invariant đều giữ nguyên. Điều này xác nhận giá trị trên giấy chưa thành tiền lời, không chứng minh policy sẽ chọn chuỗi mua-bán. Không đổi reward, model hoặc gameplay. Chi tiết dataset, các số theo từng hàng, hashes và lệnh: [plan](design/M5-v29-buy-sell-roundtrip-audit-plan.md), [báo cáo đầy đủ](reports/M5-v29-buy-sell-roundtrip-audit-evidence-2026-10-22.md), [audit](../build/m5-v29-buy-sell-roundtrip/audit.json), [manifest](../artifacts/m5-v29-buy-sell-roundtrip-audit-2026-10-22-manifest.json).

## Cập nhật mới nhất — tài sản của người khác không đổi khi xử lý di sản

Realm fixture giờ có title, quyền sử dụng và xưởng thuộc người còn sống khác với người chết. Sau pha tử vong test so sánh bản ghi đầy đủ theo ID, gồm tiền/hàng, ủy quyền và sổ công/lương của xưởng; tổng tiền, invariant và save/load đều đúng. Windows Release Realm **20.680 checks** pass, workload profile A 10.000 người/19.743 quan hệ thưa/90 ngày, không phải profile B. Lượt đầu phát hiện fixture thiếu tài sản người sống để so sánh; sau khi bổ sung fixture, build và chạy lại đều pass. Mốc ổn định sau đó: full core CTest Release **19/19 trong 38,54 giây** và Debug **19/19 trong 391,31 giây**, với fixture này. Chỉ đổi test, không đổi xử lý production/schema/API/binding; Python/native/GDExt/Godot không chạy lại. Chi tiết: [plan](design/M5-core-estate-living-property-isolation-plan.md), [báo cáo](reports/M5-core-estate-living-property-isolation-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-living-property-isolation-2026-10-22-manifest.json).

## Cập nhật mới nhất — quỹ hộ vẫn tách khỏi di sản cá nhân

Thêm Realm test: 37 tiền được chuyển thật từ Province firm vào quỹ hộ trước cái chết, không tạo tiền mới. Sau tử vong, tiền cá nhân trả khoản vay rồi mới chia cho người hưởng; người hưởng nhận đúng số tiền riêng của người chết, còn quỹ hộ giữ 37. Save/load giữ nguyên hai sổ và fingerprint. Windows Release Realm build/test pass: **20.675 checks**, runner profile A 10.000 người/19.743 quan hệ thưa/90 ngày, không phải profile B. CTest Release 19/19 và Debug 19/19 đã chạy trước fixture-only này; adapter không chạy lại vì không đổi production source/schema/API. Chi tiết: [plan](design/M5-core-estate-household-fund-isolation-plan.md), [báo cáo](reports/M5-core-estate-household-fund-isolation-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-household-fund-isolation-2026-10-22-manifest.json).

## Cập nhật mới nhất — ca làm tiếp tục sau khi người hưởng nhận xưởng

Realm C++ đã kiểm chuỗi chủ xưởng chết → trả khoản vay 20 từ di sản → chuyển xưởng cho người phối ngẫu trưởng thành hưởng duy nhất → chạy ca 64 tick đã lên lịch → trả bốn công từ xưởng → dùng 2 Fiber làm 2 Cloth → save/load giữ nguyên. Người làm được ủy quyền và gửi ca trước khi kế vị; không có quyền mới cho guardian hoặc đồng sở hữu. Windows Release build Realm/worksite-payroll targets pass; Realm **20.669 checks**, payroll xưởng **138 checks** với năm oracle save v16. Release CTest 19/19 đã qua trước fixture-only; sau khi thêm fixture, full core Debug CTest **19/19** pass trong **429,58 giây**. Python/native/GDExt/Godot không chạy lại vì không đổi production source/API. Chi tiết: [plan](design/M5-core-estate-workshop-continuation-plan.md), [báo cáo đầy đủ](reports/M5-core-estate-workshop-continuation-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-workshop-continuation-2026-10-22-manifest.json).
## Cập nhật mới nhất — đã tích hợp core refactor R1–R4 trên Windows

Các phần refactor đã tách riêng nay được kiểm lại cùng nhau sau những thay đổi test di sản. Toàn bộ core Release CTest đạt **19/19**; Python Realm replay **273 checks/64 ngày**, clock binding **173 checks/7 checkpoint**; native runtime contract **1/1**; GDExtension Release build thành công; Godot headless guard **5/5**. Realm runner đạt 20.669 checks với workload profile A 10.000 người/19.743 quan hệ thưa/90 ngày, không phải profile B. Vì vậy phạm vi core refactor R1–R4 được ghi là hoàn tất trên Windows Release. Core Debug CTest sau fixture-only cũng đạt 19/19 trong 429,58 giây. Không đồng nghĩa M5, policy đã học, profile B hay package đã hoàn tất. Chưa chạy Python/native/GDExt/Godot Debug, Godot gameplay với model tương thích, learned-policy evaluation, profile B hoặc package. Chi tiết/lệnh/giới hạn: [báo cáo tích hợp đầy đủ](reports/M5-core-refactor-R1-R4-integration-evidence-2026-10-22.md).

Bước phát triển kế tiếp vẫn thuộc backlog M5: hoàn thiện các nhóm luật còn thiếu theo contract và nối chúng qua gameplay thật. Guardian chỉ có quyền quản lý đất đã được định nghĩa; chưa có luật cụ thể để guardian chi tiền hoặc quản lý xưởng của trẻ. Không tự đặt cách đồng sở hữu đất/xưởng cho nhiều người hưởng. M5.0/1–M5.5, learned-policy quality, profile B, hiệu năng toàn chu trình và Windows package vẫn mở.

## Chỉ dẫn mới từ người dùng — gom kiểm tra tích hợp, 2026-10-21

Tab chính áp dụng ngay từ ranh giới an toàn kế tiếp [AGENTS](../AGENTS.md), mục “Kiểm tra theo phạm vi thay đổi”, và [workflow mục 4](DEVELOPMENT_WORKFLOW.md#4-evidence-đủ-để-nghiệm-thu): viết core → test core → kiểm chuỗi xử lý bằng C++ → gom Python/native/Godot theo nhóm chức năng ổn định. Không mặc định ALL_BUILD/full Release+Debug CTest/Python replay/Godot sau mỗi sửa nhỏ. API/ABI/binding hoặc observation/action/reward/model đổi cần kiểm sớm đúng lớp; save schema đổi cần migration/roundtrip/reject core ngay. Plan tiếp theo ghi kiểm ngay, kiểm để mốc nào và ranh giới đóng nhóm chức năng; báo rõ core đã kiểm và adapter chưa kiểm trên phiên bản mới. Không dừng lệnh đang chạy, không giảm yêu cầu tích hợp cuối, policy quality hoặc phạm vi M5. Tab phụ chỉ sửa các chỉ dẫn này theo yêu cầu trực tiếp của người dùng; không sửa source hoặc đổi tác vụ core đang chạy.

## Cập nhật trước đó — Realm thật: thừa kế holding cho trẻ rồi guardian thu hồi quyền dùng

Đã thêm test tích hợp C++ nối hai luật đã có. Một cha/mẹ chết để lại holding cho người con hợp pháp duy nhất còn sống, kể cả khi người con chưa đủ 18 tuổi. Quyền sử dụng hiện có không bị hủy; grantor cập nhật theo title-holder mới. Guardian còn sống trong household dùng lệnh Realm thu hồi permission; title vẫn thuộc trẻ, không chuyển tiền và save/load tiếp tục đúng. Đây chỉ thêm fixture test, không đổi gameplay source.

Windows Release Realm tests **20.645 checks** pass, land-title tests **88 checks** pass. Realm runner có profile A 10.000 người/19.743 quan hệ/90 ngày, không phải profile B. Full core CTest 19/19 đã pass ngay trước lát fixture này; không lặp cả suite vì chỉ đổi test Realm, và hai executable bị ảnh hưởng đã chạy trực tiếp. Chưa chạy Debug, Python/native/GDExt/Godot, learned policy, profile B hoặc package trên source mới. Tự review, chưa có reviewer độc lập. Chi tiết: [plan](design/M5-core-estate-minor-guardian-land-plan.md), [báo cáo đầy đủ](reports/M5-core-estate-minor-guardian-land-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-minor-guardian-land-2026-10-22-manifest.json).

Tiếp tục M5 theo contract: nhiều người hưởng, quản lý tài sản của trẻ và các lớp gameplay còn thiếu. Nhánh nợ thiếu tiền và nhánh guardian/đất đơn người hưởng đã có C++ Realm coverage. Không tự đặt luật chia holding hay quyền guardian quản lý tiền/xưởng. Nhóm ownership/di sản vẫn chưa đủ ổn định để đóng tích hợp Python/native/Godot. Core refactor R1–R4 đã qua Windows Release C++ và adapter gates được ghi ở dưới; M5, learned policy, cross-play/R1, profile B và package Windows còn mở.

## Cập nhật trước đó — di sản khi khoản vay lớn hơn cash

Realm integration test xác nhận khoản vay không bị trả một phần khi hồ sơ thiếu tiền. Khoản vay còn nguyên trong hồ sơ; tiền và hàng chia cho phối ngẫu duy nhất theo luật hiện hành; nợ không ghi vào người hưởng; title/quyền dùng/xưởng vẫn đứng tên người chết và còn được hồ sơ tham chiếu. Save/load giữ nguyên các dữ liệu. Chỉ mở rộng fixture/test, không sửa gameplay.

Windows Release Realm tests **20.652 checks** và manager tests pass. Runner kèm profile A 10.000 người/19.743 quan hệ thưa/90 ngày, không phải profile B. CTest core 19/19 đã pass trước các test-only addition; chạy trực tiếp các target Realm/manager bị ảnh hưởng. Chưa chạy Debug, Python/native/GDExt/Godot, learned policy, profile B hoặc package trên source mới. Chi tiết: [plan](design/M5-core-estate-underfunded-claim-plan.md), [báo cáo](reports/M5-core-estate-underfunded-claim-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-underfunded-claim-2026-10-22-manifest.json).

## Cập nhật mới nhất — nhiều người hưởng qua death phase Realm

Hai con hợp pháp nhận cash và Fiber chia đều; phần lẻ về người ID nhỏ hơn. Nợ 20 được trả đủ trước chia. Vì chưa có luật đồng sở hữu, title/quyền dùng/xưởng giữ nguyên trong hồ sơ thay vì tự chọn một người. Save/load đúng.

Windows Release Realm tests **20.659 checks** và manager tests pass; profile A 10.000 người/19.743 quan hệ thưa/90 ngày, không phải profile B. Full core CTest 19/19 đã pass trước fixture-only change; direct Realm/manager binaries đã build và chạy. Chưa chạy Debug, Python/native/GDExt/Godot cho nhóm estate, learned policy, profile B hoặc package. Chi tiết: [plan](design/M5-core-estate-multiple-heirs-realm-plan.md), [báo cáo](reports/M5-core-estate-multiple-heirs-realm-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-multiple-heirs-realm-2026-10-22-manifest.json).

## Cập nhật mới nhất — R4: tách điểm vào RealmView và điều phối save/load

Hai thay đổi cấu trúc đã chuyển `Realm::view()` sang `core/src/realm_view.cpp`, và các hàm `serialize`, `deserialize`, `save`, `load`, `fingerprint` sang `core/src/realm_save.cpp`. Realm vẫn lọc sự kiện theo quyền biết; các codec vẫn giữ quy tắc section, schema/migration và kiểm invariant. Không đổi hành vi UI, save bytes/schema, binding hoặc gameplay.

Windows Release build toàn core pass; projector **5/5**, manager pass, Realm **20.639 checks**, full core CTest **19/19** pass. CTest ban đầu phát hiện ba oracle token count chưa tính header parentage rỗng; đã sửa và chạy lại. Sau đó build/replay adapters từ core hiện tại: Python Realm **273 checks/64 ngày**, clock **173 checks/7 checkpoint**, native runtime `1/1`, GDExtension Release build pass, Godot headless model guard **5/5**. Hai test Python còn kỳ vọng schema24; đã cập nhật lên schema25 rồi chạy lại. Godot chỉ kiểm extension/model guard, không phải UI playtest hay policy run. Chưa chạy Debug, full Realm smoke cần model tương thích, learned-policy quality, profile B hoặc package. Realm runner có workload profile A 10.000 người/19.743 quan hệ/90 ngày, không phải profile B. Tự review, không có reviewer độc lập. Chi tiết: [plan save/load](design/M5-core-R4-save-orchestration-plan.md), [bằng chứng save/load](reports/M5-core-R4-save-orchestration-evidence-2026-10-22.md), [plan RealmView](design/M5-core-R4-view-method-plan.md), [bằng chứng RealmView](reports/M5-core-R4-view-method-evidence-2026-10-22.md).

Tiếp tục M5 ownership/di sản với các trường hợp nhiều người hưởng, trẻ vị thành niên và khoản nợ thiếu tiền; chưa tự đặt luật đồng sở hữu hoặc quyền giám hộ tài sản chưa có trong contract. Sau khi nhóm này ổn định, chạy tích hợp Python/native/Godot. M5, core refactor tích hợp, learned policy, cross-play/R1, profile B và package Windows vẫn mở.

## Cập nhật mới nhất — xưởng trong di sản cho một người hưởng trưởng thành

Sau khi khoản vay trong hồ sơ trả hết, nếu chỉ có một người hưởng và người đó đủ 18 game years, xưởng cá nhân của người chết chuyển cho người ấy. Xưởng giữ nguyên vùng, ngành, công suất, tiền, hàng, quyền làm việc của công nhân, số công đã ghi và tiến độ sản xuất; chỉ ID chủ sở hữu và chỉ mục nội bộ thay đổi. Nếu người hưởng còn nhỏ, có nhiều người hưởng, không có người hưởng hoặc còn nợ, xưởng vẫn nằm trong hồ sơ. Người thừa kế trưởng thành đã dùng lệnh Realm hiện có để rút tiền từ xưởng sau khi nhận.

Windows Release manager/Realm targets build pass; manager tests pass; Realm tests **20.639 checks** pass, gồm death → claim → tiền/hàng/title/xưởng → command rút tiền → save/load. Runner chạy profile A 10.000 người/19.743 quan hệ thưa/90 ngày, không phải profile B. Chưa chạy Debug, full CTest, Python/native/Godot, learned policy, profile B hoặc package trên source mới. Tự review; chưa có reviewer độc lập. Chi tiết: [plan](design/M5-core-estate-establishment-plan.md), [báo cáo](reports/M5-core-estate-establishment-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-establishment-2026-10-22-manifest.json).

Tiếp tục ownership/di sản với người hưởng chưa thành niên, nhiều người hưởng và khoản nợ thiếu tiền; sau khi nhóm này ổn định chạy kiểm tích hợp Python/native/Godot. M5, core refactor tích hợp, learned policy đạt chuẩn, cross-play/R1, profile B và package Windows còn mở.

## Cập nhật 2026-10-22 — title đất trong di sản khi chỉ có một người hưởng

Khi các khoản vay của người chết đã được trả và chỉ còn một người thân hợp pháp đủ điều kiện hưởng di sản, title từng holding chuyển sang người đó, kể cả khi người nhận chưa đủ 18 tuổi. Giấy phép sử dụng đất đang có không bị hủy; người hưởng trở thành grantor hiện tại, còn người dùng đất và holding giữ nguyên. Nếu có nhiều người cùng hàng, không có người hưởng hoặc còn nợ chưa trả, title và quyền sử dụng tiếp tục được tham chiếu trong hồ sơ, không chia holding hay chọn người tùy tiện.

Windows Release manager/Realm targets build pass; `polyec_manager_tests.exe` pass; Realm tests **20.636 checks** pass. Kiểm tra C++ bao gồm người hưởng vị thành niên, giữ title khi nhiều con, không có người hưởng hoặc còn nợ, từ chối nguyên tử khi title đã cũ, death → trả loan → chuyển title/quyền sử dụng → save/load. Runner chạy profile A 10.000 người/19.743 quan hệ/90 ngày; đây không phải profile B. Chưa chạy Debug, full CTest, Python/native/Godot, learned policy, profile B hoặc package trên source mới. Không đổi schema/binding; adapter để mốc estate/ownership ổn định. Tự review, chưa có reviewer độc lập. Chi tiết: [plan](design/M5-core-estate-land-title-plan.md), [báo cáo](reports/M5-core-estate-land-title-evidence-2026-10-22.md), [manifest](../artifacts/m5-core-estate-land-title-2026-10-22-manifest.json).

Lát title được kiểm độc lập trước khi chuyển xưởng; chi tiết và giới hạn nằm trong report liên kết bên trên. Không tuyên bố chuyển title này hoàn tất estate/M5.

## Tiếp tục theo yêu cầu người dùng — chia tiền mặt di sản sau khi trả nợ

Đã hoàn thành lát kế tiếp trong chu trình di sản. Khi cư dân chết, Realm trả đầy đủ khoản vay từ hồ sơ nếu đủ tiền; sau đó chia tiền còn lại bằng nhau cho nhóm người thân còn sống đầu tiên theo thứ tự con hợp pháp → phối ngẫu → cha mẹ → anh chị em. Đơn vị lẻ theo ID nhỏ nhất. Không có người hưởng thì tiền vào quỹ di sản công tổng hợp. Tiền người hưởng đi vào tài khoản cá nhân; quỹ hộ và tiền người khác không đổi. Hồ sơ vẫn tồn tại cho hàng, đất/quyền sử dụng đất, khoản vay và xưởng; trường cash của hồ sơ về 0. Giám hộ chưa có quyền chi/ký thay.

Windows Release build manager/Realm targets pass; manager tests pass; Realm tests 20.634 checks pass. Realm fixture kiểm chuỗi chết → trả khoản vay → chuyển 980 tiền còn lại cho phối ngẫu → save/load; tổng tiền và invariant giữ nguyên. Không đổi save schema, nên Python/GDExt/Godot chưa chạy ở lát này; chúng được để mốc tích hợp nhóm estate/ownership. Chưa chạy Debug/full CTest/learned policy/benchmark/package. Codex tự review, chưa có reviewer độc lập. Chi tiết đầy đủ và lệnh ở [plan](design/M5-core-estate-cash-distribution-plan.md) và [evidence](reports/M5-core-estate-cash-distribution-evidence-2026-10-21.md).

Tiếp theo phải tiếp tục ownership/di sản qua hàng hóa, đất, quyền sử dụng đất và xưởng; giải quyết thiếu tiền trả nợ và xử lý tiền/tài sản người chưa thành niên theo contract rõ ràng. Sau khi nhóm ownership ổn định sẽ chạy full core Release CTest và gom Python/native/Godot migration/replay/smoke. Core refactor toàn phần, M5, learned policy đạt chuẩn, cross-play/R1, profile B và package Windows vẫn mở. Working tree chứa nhiều thay đổi của các phiên trước; không commit gộp.

## Cập nhật mới nhất — phân phối hàng trong hồ sơ di sản

Lát ngay sau phân phối tiền đã nối cả hàng hóa vào cùng giao dịch tử vong. Sau trả nợ, tiền và từng loại hàng trong hồ sơ được chia đều cho cùng nhóm người thân hợp pháp còn sống; đơn vị dư đi theo ID tăng dần. Nếu không có người hưởng, tiền vào quỹ di sản công tổng hợp và hàng vào kho công của vùng đã lưu trong hồ sơ. Hồ sơ vẫn giữ ID cho đất/quyền sử dụng đất/khoản vay/xưởng, nhưng số dư cash và inventory về 0 sau chuyển. Không đổi save schema hoặc binding.

Windows Release manager/Realm targets và direct tests pass. Realm tests 20.634 checks xác nhận: cư dân chết tiêu 1 tiền mua thức ăn trước; sau đó nợ 20 được trả, còn 979 tiền và 5 Fiber chuyển cho phối ngẫu, invariant và save/load vẫn khớp. Phân phối được hoãn tới cuối pha cư dân trong ngày: người nhận không thể tiêu khoản thừa kế cùng ngày và mọi người chết trong ngày đều bị loại khỏi danh sách người hưởng. Manager tests kiểm chia đều từng GoodId, remainder theo ID, quỹ/kho công fallback, overflow và stale read nguyên tử. Profile A trong Realm runner chạy kèm; đó không phải profile B. Chưa chạy Debug, full CTest, Python/native/Godot, realm-v11 64-day, learned policy, profile B hoặc package. Review tự thực hiện. Chi tiết: [plan chuyển hàng](design/M5-core-estate-goods-distribution-plan.md), [báo cáo](reports/M5-core-estate-goods-distribution-evidence-2026-10-22.md), [manifest hash](../artifacts/m5-core-estate-goods-distribution-2026-10-22-manifest.json).

Tiếp theo: xử lý quyền sở hữu đất, quyền sử dụng đất và xưởng trong thừa kế theo từng contract có quy tắc rõ cho đồng sở hữu, thiếu tiền và người chưa thành niên. Không gộp các phần này vào phân phối hàng đã kiểm. M5/core refactor, learned policy đủ chuẩn, cross-play/R1, profile B và package Windows vẫn mở.

## Đã hoàn tất phần đang dở — provenance cha–con cho truy vấn người thừa kế

Khoản vay còn dư hiện được trả toàn bộ từ tiền của hồ sơ nếu đủ; Release CTest 19/19, Python parity, GDExt và các smoke Godot không cần model đều pass. [Bằng chứng](reports/M5-core-estate-cash-claims-evidence-2026-10-21.md). `realm_smoke` 64 ngày chưa chạy vì thiếu bundle realm-v11; chưa có model tương thích trong project.

Trước khi dùng beneficiary resolver để chuyển tiền hay tài sản, đã phân biệt quan hệ cha–con hợp pháp với cạnh tổng hợp tạo khi khởi tạo hộ. Cạnh sinh thật được đánh dấu; save schema25 lưu các cạnh này, còn migration save cũ chỉ xác nhận quan hệ khi có sự kiện sinh con khớp đúng chiều. Truy vấn người hưởng chỉ tính con/cha mẹ đã xác nhận và suy ra anh chị em qua cha/mẹ chung. Manager Release tests pass; Realm Release tests 20.634 checks pass, gồm roundtrip, migration schema24 và từ chối chỉ số section sai mà không thay Realm đích. Chưa nối resolver vào chia tiền, hàng, quyền đất hoặc xưởng. Python, GDExt/Godot và full CTest chưa chạy lại sau schema25; review Codex tự thực hiện, chưa có reviewer độc lập. [Plan](design/M5-core-legal-parentage-provenance-plan.md), [báo cáo](reports/M5-core-legal-parentage-provenance-evidence-2026-10-21.md), [bàn giao phiên sau](handoff/M5-session-handoff-2026-10-21.md).

Ghi chú tại thời điểm bàn giao trước yêu cầu tiếp tục của người dùng: phần provenance đã được khép lại, bước kế tiếp là nối phân phối tiền di sản. Bước đó hiện đã được thực hiện và ghi ở phần cập nhật mới nhất phía trên. M5, learned policy đủ chuẩn, cross-play/R1, profile B, full Realm smoke và package còn mở. Working tree có nhiều thay đổi từ các tác vụ trước; không commit/push gộp khi chưa tách được file thuộc phạm vi.

## Cập nhật 2026-10-21 — trả khoản vay từ tiền di sản

Sau khi người vay chết, hồ sơ dùng tiền của chính người đó để trả đầy đủ các khoản vay còn dư nếu đủ tiền; chủ nợ nhận đúng khoản vào Province firm hoặc credit pool. Nếu hồ sơ thiếu, không trả một phần hay ưu tiên chủ nợ. Không lấy quỹ hộ hoặc tiền người sống; khoản nợ không chuyển sang người hưởng. Hàng, title, xưởng chưa bị thanh lý/chuyển và chưa chia di sản. Save schema24 không đổi. [Plan](design/M5-core-estate-cash-claims-plan.md), [report](reports/M5-core-estate-cash-claims-evidence-2026-10-21.md), [manifest](../artifacts/m5-core-estate-cash-claims-2026-10-21-manifest.json).

Release C++ manager/Realm checks và full CTest 19/19 pass. Python parity 273 checks/64 ngày; clock 173 checks/7 checkpoint. GDExt Release build và Godot native/UI/campaign/model guard smoke 4 nhóm pass. Smoke Realm 64 ngày chưa pass vì thiếu bundle tương thích `models/realm-v11`; không dùng model cũ thay thế. Chưa chạy Debug cho thay đổi này. Codex tự review; chưa có review độc lập.

Tiếp theo: sửa provenance quan hệ để chỉ cạnh cha–con được xác nhận mới đủ điều kiện làm người thừa kế. Dữ liệu khởi tạo hiện tạo một số cạnh `ParentChild` từ gom hộ và có cạnh tự thân, nên resolver hiện có chưa an toàn để chuyển tiền/title/xưởng. Sau đó mới nối phân phối tiền có ghi sổ quỹ di sản công và xử lý giám hộ tài sản phụ thuộc. M5, learned policy đủ chuẩn, cross-play/R1, profile B và package còn mở.

## Cập nhật 2026-10-21 — hồ sơ di sản được lưu qua schema 24

Đã thêm hồ sơ riêng theo người đã chết. Khi Realm xử lý tử vong, tiền và hàng còn ở cá nhân được đưa vào hồ sơ; hồ sơ ghi ngày/vùng và ID xưởng cá nhân, land title, quyền sử dụng đất họ cấp, cùng khoản vay còn dư. Tiền hộ gia đình vẫn là tài khoản riêng. Save schema24 ghi hồ sơ; save schema7–23 chuyển estate aggregate cũ thành phần legacy chưa gắn hồ sơ. Release manager tests pass; Release Realm tests 20.621 checks pass, gồm save/load 512 nhân vật và migration schema23. Chưa chạy full CTest, Debug, binding Python, GDExt/Godot trong lượt mới. Chưa trả nợ, chia tài sản, chuyển title/xưởng hoặc đưa guardian vào thay người giám hộ; beneficiary resolver chưa áp dụng phân phối. [Plan](design/M5-core-estate-ledger-plan.md), [report](reports/M5-core-estate-ledger-evidence-2026-10-21.md), (manifest lịch sử này hiện không còn trong checkout).

Tiếp theo chạy full Release CTest, binding/save replay Python và GDExt/Godot smoke; sau đó nối settlement và phân phối theo backlog IMPLEMENTATION_PLAN. M5, learned policy đạt chuẩn, cross-play/R1 và profile B còn mở.

## Cập nhật 2026-10-21 — xác định nhóm người hưởng di sản theo thứ tự luật

Thêm query thuần vào `EstateManager`: với người đã chết, chọn nhóm đầu tiên còn sống theo thứ tự con → phối ngẫu → cha mẹ → anh chị em → quỹ di sản công; danh sách người cùng hàng được sắp ID ổn định, quan hệ anh chị em tính qua cha/mẹ chung. ID sai và người còn sống bị từ chối. Release/Debug manager tests pass; full Windows Release CTest 19/19 pass. Query chưa nối vào tử vong hoặc chuyển tài sản; Realm vẫn gom tiền vào `estate_`, đưa hàng về vùng và giữ khoản vay ở người chết. [Plan](design/M5-core-estate-beneficiary-resolution-plan.md), [report](reports/M5-core-estate-beneficiary-resolution-evidence-2026-10-21.md), [manifest](../artifacts/m5-core-estate-beneficiary-resolution-2026-10-21-manifest.json).

Phần cần làm tiếp: sổ di sản và quyết toán nghĩa vụ từ tài sản của người chết trước khi chia; chuyển quyền đất/xưởng và xử lý người chưa thành niên; thêm migration/save replay. Cần xác định quan hệ `ParentChild` từ khởi tạo household và sinh con là quan hệ pháp lý nào trước khi cho query này ghi quyền sở hữu. M5 estate/ownership, policy đạt chuẩn, cross-play/R1 và profile B vẫn mở.

## Cập nhật 2026-10-21 — tách pha xử lý cư dân khỏi translation unit Realm lớn

`Realm::day_people` được chuyển nguyên thân sang `core/src/realm_lifecycle.cpp`; Realm vẫn sở hữu state, RNG và thứ tự gọi pha. Trả lương chức vụ, di chuyển, ghi nhận công, học, ăn, tử vong và event không đổi. Windows Release `ALL_BUILD` pass, CTest 19/19 pass (674,66 giây), diff check pass; chưa chạy Debug/native/Godot hay profile. Đây là cấu trúc code, không phải tính năng người chơi. [Plan](design/M5-core-person-day-phase-extraction-plan.md), [report](reports/M5-core-person-day-phase-extraction-evidence-2026-10-21.md), [manifest](../artifacts/m5-core-person-day-phase-extraction-2026-10-21-manifest.json).

Tiếp theo: thiết kế lát di sản theo quy định §2 của `IMPLEMENTATION_PLAN`, khóa cách xác định legal child/claim priority và bảo toàn cash, hàng, cơ sở, title, household fund, khoản vay trước khi nối vào death transition. Không coi `estate_` aggregate hiện tại là probate ledger; M5 estate/ownership và learned-policy R1 vẫn mở.

## Cập nhật 2026-10-21 — đối chứng định giá hàng theo giá bán địa phương

Trên 15 Buy hợp lệ từ ba checkpoint v23, định giá hàng theo giá tham chiếu tạo utility dương; đánh dấu theo giá bán hiện tại làm cả 15 thành −0,08. Nhưng trong 52 snapshot/hàng đang có tồn, giá trị theo vùng đổi ở 11 trường hợp (tổng biên độ max–min 585 tiền; lớn nhất 192). Vậy giá thanh lý địa phương loại bỏ lợi ích mua ở các mẫu đã train, song có thể thưởng/phạt việc di chuyển trước khi bán. Không đổi reward hoặc model; cần giải quyết giá vốn ổn định cho hàng mua, sản xuất và chuyển kho trước khi sửa utility. [Plan](design/M5-v28-inventory-marking-counterfactual-plan.md), [evidence](reports/M5-v28-inventory-marking-counterfactual-evidence-2026-10-21.md), [data](../build/m5-v28-inventory-marking-counterfactual/audit.json), [manifest](../artifacts/m5-v28-inventory-marking-counterfactual-2026-10-21-manifest.json).

Tiếp tục core refactor với lát kế tiếp trong M5 backlog: hoàn tất rà soát các chuyển trạng thái quyền sở hữu/di sản qua death settlement và ranh giới save/load, sau đó kiểm tra từng thiếu hụt bằng fixture C++/binding. M5 gameplay, learned policy đủ chuẩn, cross-play/R1 và profile B vẫn mở.

Nguồn điều hành tác vụ hiện tại. Cập nhật khởi tạo: 2026-10-18. Dành cho agent: đọc file này sau AGENTS, rồi chỉ mở linked contract/source cần cho phần đang làm. Scope toàn dự án: [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md). Lý do/phương pháp: [DEVELOPER_GUIDE](DEVELOPER_GUIDE.md). Kết quả cũ: [HISTORY](DEVELOPMENT_HISTORY.md).

## Cập nhật 2026-10-21 — xác định lệch giá tài sản khiến PPO tích trữ

Counterfactual trên checkpoint v23 đã lưu: 15 cặp Buy–Hold giữ nguyên world fingerprint và peer RNG, 15/15 lệnh Buy tăng reward trong 7 ngày. Lệnh mua bốn món chỉ mất 12 tiền actor vì giá thị trường chạm đáy 2/món, nhưng utility cộng giá tham chiếu đầy đủ; ví dụ mua Horse tăng ngay 3,48 điểm dù paid work, sức khỏe và dinh dưỡng không đổi. Seed 2 không còn Buy hợp lệ trong 16 world checkpoint. Đây là động cơ tích trữ do lệch giữa giá địa phương và định giá inventory cố định, giải thích reward Buy dương và một phần hành vi mua nhiều nhưng ít sản xuất. Chưa đổi utility/reward vì cần so công thức định giá thanh lý và tác động lên gameplay/learned-policy; không promotion. [Plan](design/M5-v27-trained-world-buy-counterfactual-plan.md), [evidence](reports/M5-v27-trained-world-buy-counterfactual-evidence-2026-10-21.md), [data](../build/m5-v27-trained-world-buy-counterfactual-final/audit.json), [manifest](../artifacts/m5-v27-trained-world-buy-counterfactual-2026-10-21-manifest.json).

Audit v26 trước đó dùng 20 seed ban đầu và reset snapshots, nên mọi Buy giảm utility; đối chứng thêm trên 321 reset-context pair cùng base seed train cũng không có Buy nào vượt Hold. Chênh với v27 xác nhận vấn đề phát sinh khi thị trường đã phát triển và giá chạm đáy, không phải ngay đầu game. [Bằng chứng v26](reports/M5-v26-economic-inventory-valuation-evidence-2026-10-21.md), [dữ liệu](../build/m5-v26-economic-inventory-valuation-training-seeds/audit.json).

## Cập nhật 2026-10-21 — audit quyết định chi tiền và vay của Economic

Đã đo ngay trước/sau từng lệnh C++ trong ba checkpoint v23 ×20 holdout world ×128 ngày (60 lần chạy; mọi invariant pass). Nhân vật Economic ra lệnh tạo xưởng 1.365 lần, mua 622, đi lại 1.482, vay 423 nhưng trả nợ 32; chỉ có 30 lần chuyển hàng, 181 lần cấp quyền và 144 lần gửi Work tại xưởng. Trong lệnh mua có 139 lượt Fiber; xưởng cuối kỳ gần như không còn Fiber/Cloth, paid site work units trung bình 9,6. Kết quả cho thấy policy dùng vốn nhiều cho mở xưởng, mua hàng và di chuyển trong khi ít hoàn tất bước cấp vật tư và gửi việc; tiền mặt cá nhân giảm trực tiếp tổng 127.158 và nợ tăng 41.272 qua 60 world, không gồm quỹ hộ/chi phí ngày thường. Không đổi model hoặc gameplay, không thăng cấp model. [Plan](design/M5-v25-economic-decision-flow-audit-plan.md), [evidence](reports/M5-v25-economic-decision-flow-evidence-2026-10-21.md), [data](../build/m5-v25-economic-flow-audit-final/audit.json), [manifest](../artifacts/m5-v25-economic-decision-flow-2026-10-21-manifest.json).

## Cập nhật 2026-10-21 — tăng độ bền cách đếm hành động xưởng trong đánh giá

Evaluator giờ giải mã Intent ở đúng candidate đã chọn. Đối chiếu source C++ cho thấy cách đếm slot cũ đúng trong Realm v11 hiện tại vì slot 72–79 dành cho kết hôn, còn slot 89–128 dành cho thao tác xưởng; nhận định ban đầu rằng có kết hôn ở slot cao là sai. Kiểm tra adapter 28/28 pass. Chạy lại ba checkpoint v23 trên cùng 20 world ×128 ngày cho kết quả y hệt, không seed nào đạt điều kiện. Đây là cải thiện độ bền nếu slot thay đổi, không phải sửa số liệu gameplay. Codex tự review. [Plan](design/M5-v24-semantic-worksite-counts-plan.md), [evidence](reports/M5-v24-semantic-worksite-counts-evidence-2026-10-21.md), [kết quả](../build/m5-v24-corrected-economic-selection-final/selection.json).

## Cập nhật 2026-10-21 — PPO curriculum kinh tế cho chu trình xưởng

Curriculum `economic-worksite-reset-v1` khởi tạo đều 4 bối cảnh shared-core cho Economic: tự nhiên, xưởng thiếu Fiber, xưởng có Fiber cá nhân, và chủ xưởng có công nhân đã được phép làm. Mặc định `none` không đổi run cũ; version khóa trong resume/runset/selection. Ba seed ×25 cập nhật ×25.600 quyết định; 20 world mới ×128 ngày, invariants đúng. Selector không chọn model: utility vs baseline -0,427…-0,463; so initial dương nhỏ nhưng CI cắt 0; tiền thấp 345–373, nợ cao 94–120, thêm 0,50–0,75 insolvencies. Dù khoảng 22 xưởng, chỉ 8–11 paid site work units và gần như không có Fiber/Cloth cuối kỳ. Không chạy R1, không peer promotion. Audit v25 cho thấy mở xưởng, mua hàng và đi lại chiếm nhiều quyết định trong khi chuyển hàng/cấp quyền/gửi Work ít; phân tích tiếp theo phải kiểm tra thay đổi ưu tiên vốn trong tình huống có đánh đổi, trước một lượt train. Gameplay M5, R1, cross-play và profile B vẫn mở. [Plan](design/M5-v23-economic-worksite-curriculum-plan.md), [evidence](reports/M5-v23-economic-worksite-curriculum-evidence-2026-10-21.md), [manifest](../artifacts/m5-v23-economic-worksite-curriculum-2026-10-21-manifest.json).

## Cập nhật 2026-10-21 — kiểm tra lựa chọn policy ở tình huống xưởng

Ba seed Economic v20 đã bị selector loại được chạy qua cùng ba snapshot shared-core: đầu game, có xưởng Cloth nhưng chưa có Fiber, và sau khi mua Fiber. Khi Fiber chưa có thì lệnh chuyển bị mask; sau mua, mỗi model cấp 2,61–2,72% xác suất cho chuyển hàng (5–7 lần/256 mẫu categorical). Trong cùng snapshot, tạo thêm xưởng chiếm 23,5–24,1%, mua hàng 17,9–18,1%; authorize/làm tại xưởng khoảng 2,7% mỗi loại. Điều này cho thấy sở thích tiếp tục của model trong snapshot, không phải phép đo outcome hoặc bằng chứng nguyên nhân. Không chọn/nâng cấp model. [Plan](design/M5-v22-worksite-context-policy-audit-plan.md), [evidence](reports/M5-v22-worksite-context-policy-audit-evidence-2026-10-21.md), [data/hash](../artifacts/m5-v22-worksite-context-policy-audit-2026-10-21.json) / [manifest](../artifacts/m5-v22-worksite-context-policy-audit-2026-10-21-manifest.json).

## Cập nhật 2026-10-21 — đo độ phủ hành động PPO theo loại quyết định

History mỗi update giờ đếm số lựa chọn hợp lệ và số lần policy chọn theo từng loại `Intent`, kèm reward ngay sau quyết định và advantage. Diagnostic version nằm trong manifest/resume identity; policy, reward, observation, rollout và optimizer không đổi. Python syntax, 9 kiểm tra tổng hợp, smoke 1.024 quyết định và resume sang update 2 đều pass. Trong kịch bản shared-core, chuyển Fiber chỉ hợp lệ sau khi có xưởng Cloth và Fiber trong kho cá nhân; observation báo đúng loại hành động, ngành xưởng và lượng hàng. C++ Realm fixture kiểm tiếp ủy quyền, ca làm, save/load, trả lương/sản xuất và rút Cloth; thêm oracle xác nhận chưa có Fiber thì không có lựa chọn nạp hàng. Chưa chứng minh policy học được chuỗi này. Kế tiếp mở rộng controlled-context coverage với nhiều thành phần đối tác trước PPO screen dài. [Plan](design/M5-v21-action-intent-coverage-plan.md), [evidence](reports/M5-v21-action-intent-coverage-evidence-2026-10-21.md).

## Cập nhật 2026-10-21 — tách phép chiếu RealmView

`Realm::view(observer)` tiếp tục lọc sự kiện theo quyền biết hiện có; việc cộng số người/sống/trẻ em/quyết định và sao chép summary sang `RealmView` được chuyển sang projector thuần. Không đổi luật chơi, save, action, observation hoặc giao diện. Windows Release build và full CTest 19/19 pass; Python binding replay 273 checks/64 ngày, clock replay 173 checks/7 checkpoints; GDExtension build và Godot headless 5/5 pass. Đây là self-review, không đóng gameplay hoặc M5. [Plan](design/M5-core-R4-view-projection-plan.md), [evidence](reports/M5-core-R4-view-projection-evidence-2026-10-21.md).

## Cập nhật 2026-10-21 — screen PPO mini-batch với reward v19

Trainer v11 hỗ trợ độ dài mini-batch recurrent được khóa trong manifest và resume identity; mặc định 64 giữ đường hiện có. Screen 16 chuỗi dùng cùng sample budget (3 seed ×25 update ×25.600 quyết định), làm số optimizer step tăng 4 lần mỗi epoch. Validation sampled 20 thế giới mới ×128 ngày; cả ba seed vẫn kém baseline và CI so initial cắt 0, không chọn được model. KL tăng không đáng kể, clip fraction vẫn 0; xưởng cuối kỳ hầu như không có Cloth. [Plan](design/M5-v20-ppo-minibatch-screen-plan.md), [evidence](reports/M5-v20-ppo-minibatch-screen-evidence-2026-10-21.md), [manifest/hash](../artifacts/m5-v20-ppo-minibatch-screen-2026-10-21-manifest.json).

Kế tiếp: theo dõi credit/advantage theo tình huống kinh tế và kiểm quyết định mua/nguyên liệu/giao ca/sản xuất trước screen học tiếp. PPO review vẫn còn partner diversity, curriculum/transfer, full horizon/100-world R1. M5/core refactor vẫn mở; không giảm backlog hoặc cấp peer eligibility.

## Cập nhật 2026-10-21 — reward tài sản ròng xưởng cá nhân và PPO Economic

Đã sửa reward Economic/Civilian để tính tiền và hàng trong xưởng Person-owned, trừ lương site đã phát sinh mà chưa trả; Public/Organization không tính vào tài sản riêng. Chỉ mục chủ sở hữu được cập nhật khi tạo/sinh người và dựng lại khi load. C++/Python cùng expose khóa `realm-v11-person-enterprise-equity-v1`; run hoặc peer thiếu/sai version bị từ chối. Action, observation, model tensor và save schema không đổi. [Plan](design/M5-v19-person-enterprise-equity-reward-plan.md), [evidence](reports/M5-v19-person-enterprise-equity-reward-evidence-2026-10-21.md).

Core Release build pass; CTest 18/18 pass, sau đó fixture lương mới `realm_tests` pass riêng; Python binding build pass; co-play/resume 22 checks và syntax pass. Ba run Economic đủ 25 update/seed, cùng 25.600 quyết định; sampled validation 20 thế giới mới ×128 ngày. Utility so baseline còn -0,439…-0,470, nhưng so initial dương ở seed 1–2; tiền thấp hơn 380–385, nợ cao hơn 89–109, mất khả năng thanh toán thêm 0,50–0,70 người/thế giới. Tạo trung bình 20–21 xưởng nhưng chỉ 11–14 paid site work units, cuối kỳ Cloth trung bình 0,25. Không seed nào qua selector; không có learned peer đủ điều kiện, không chạy R1 holdout 100-world.

Review PPO đã được đọc và đối chiếu: sampled evaluation ghép đúng RNG; chu trình xưởng có đo action/authorization/submission/payroll/output; các thiếu sót partner diversity, curriculum/transfer và update nhỏ vẫn mở. Kế tiếp cô lập hiệu quả mini-batch PPO trên cùng sample budget, sau đó tiếp tục M5/core backlog; chưa giảm scope hoặc gọi milestone hoàn tất. [Manifest/hash](../artifacts/m5-v19-person-enterprise-equity-reward-2026-10-21-manifest.json).

## Cập nhật 2026-10-21 — PPO diagnostics sau optimizer và sàng lọc learning rate

Đã sửa log KL/ratio/clip để đo sau optimizer step; thêm loss policy/critic, gradient norm và phân bố reward/advantage/return. Seed 1 chạy lại đủ 25 cập nhật cho SHA policy trùng run đã validation. Một lượt thử duy nhất tăng LR 4× trên cùng ngân sách làm KL lớn hơn nhưng policy vẫn kém baseline utility 0,554; CI so initial cắt 0, cash thấp hơn và nợ/insolvency cao hơn. Không mở rộng trainset với cấu hình này; 20 thế giới đã xem chỉ là tập thăm dò, `r1_pass=false`. [Plan](design/M5-v18-ppo-update-diagnostics-and-lr-screen-plan.md), [evidence](reports/M5-v18-ppo-update-diagnostics-and-lr-screen-evidence-2026-10-21.md), [manifest](../artifacts/m5-v18-ppo-update-diagnostics-and-lr-screen-2026-10-21-manifest.json).

## Cập nhật 2026-10-21 — PPO economic sau khi đồng bộ đối tác train/eval

Ba run economic (6 vùng, mỗi run 25 cập nhật / 25.600 quyết định) dùng cùng bộ điều khiển dev cho đối tác train và đánh giá. Trên 20 thế giới validation × 128 ngày, cả ba policy đều kém baseline về utility 0,583–0,593; tiền thấp hơn, nợ và mất khả năng thanh toán cao hơn. Không chọn model, không mở quyền dùng làm learned peer và không chạy holdout dài cho ứng viên đã bị loại. Diagnostics đã thêm phân bố reward/advantage/return, loss riêng policy và critic, tỷ lệ cập nhật và gradient norm. Run lại seed 1 đủ ngân sách cho policy SHA trùng validation; KL sau optimizer median 1,97e-6, clip fraction luôn 0 và critic explained variance dao động -0,94 đến 0,55. Đây chưa phải R1 hoặc policy sản phẩm. [Kế hoạch](design/M5-v17-aligned-dev-peer-baseline-plan.md), [bằng chứng](reports/M5-v17-aligned-dev-peer-baseline-evidence-2026-10-21.md), [manifest](../artifacts/m5-v17-aligned-dev-peer-baseline-2026-10-21-manifest.json).

Kế tiếp: lập thí nghiệm có kịch bản kinh tế kiểm soát để cô lập lựa chọn worksite, hàng đầu vào và reward theo kết quả sản xuất; không tăng sample budget tổng quát trước khi có tín hiệu cải thiện. Tiếp tục các tiêu chí R1–R5 và M5/core còn mở.

## Cập nhật 2026-10-21 — validation-only checkpoint selection

Đã thêm runner ba seed cùng budget, validation-only selector và holdout riêng. Runset economic đủ 25 update/seed, nhưng runset uniform-peer cũ không hợp lệ để gộp với baseline v2. Evaluator từ chối dữ liệu thiếu peer version; selector cũ không chọn model do chưa đủ 25 updates hoặc không cải thiện initial. Holdout R1 vẫn yêu cầu 100 thế giới độc lập và đúng horizon. [Plan](design/M5-v16-validation-model-selection-plan.md) / [evidence](reports/M5-v16-validation-model-selection-evidence-2026-10-21.md).

## Cập nhật 2026-10-21 — sampled holdout cho checkpoint PPO v11

Evaluator so sánh learned/initial với baseline/hold/random trong cùng C++ Realm, đo utility cá thể, survival, health, cash, debt, nghĩa vụ/default và hoạt động xưởng. Smoke ngắn chỉ kiểm tra pipeline và invariants; không đại diện đánh giá chất lượng. R1 vẫn yêu cầu đủ training seeds, horizon và 100 holdouts. [Plan](design/M5-v15-sampled-v11-policy-holdout-plan.md) / [evidence](reports/M5-v15-sampled-v11-policy-holdout-evidence-2026-10-21.md).

## Cập nhật 2026-10-21 — PPO trainer v11 có resume chính xác

Đã thêm masked recurrent PPO trainer cho `realm-v11`, frozen peer loader và checkpoint lưu policy, optimizer, RNG, C++ Realm worlds, memory focal/peer và tiến độ. Exact-resume smoke so run ngắt/resume với run liền mạch. Đây là smoke, chưa phải policy đạt chất lượng hoặc profile B. [Plan](design/M5-v14-resumable-ppo-v11-trainer-plan.md) / [evidence](reports/M5-v14-resumable-ppo-v11-trainer-evidence-2026-10-21.md).

## Cập nhật 2026-10-21 — adapter train với peer policy đóng băng

Adapter v11 chạy focal action và peer decisions qua cùng C++ Realm; checkpoint peer được kiểm tra family/schema/model/config. Python binding expose debt/loans read-only và tùy chọn taxing authority. Đây là plumbing, không chứng minh policy đã học. [Plan](design/M5-v13-frozen-peer-training-environment-plan.md) / [evidence](reports/M5-v13-frozen-peer-training-environment-evidence-2026-10-21.md).

## Cập nhật 2026-10-21 — evaluator cross-play nhiều họ policy

Evaluator chạy governance, economic và civilian trong Realm C++ với baseline dev, hold, random legal, initial, learned-all và từng family với peer baseline. Mọi actor lấy observation/mask mới trước command; paired runs dùng world seed và actor RNG chung. Smoke 16 checks, 2 seed × 2 ngày × 8 mode giữ invariant và replay ổn định; đây chỉ là plumbing, không đánh giá policy đã học. [Plan](design/M5-v12-full-realm-crossplay-evaluation-plan.md), [evidence](reports/M5-v12-full-realm-crossplay-evaluation-evidence-2026-10-21.md), [manifest](../artifacts/m5-v12-full-realm-crossplay-evaluation-2026-10-21-manifest.json).

## Cập nhật 2026-10-21 — policy điều khiển vật tư và ca xưởng

Policy contract `realm-v11` có 129 action / 1.676 observation: mua/nạp đầu vào, rút hàng, mở Work timeline, cấp quyền worker và gửi ca một ngày vào xưởng đã chọn. C++ oracle tạo đúng Cloth rồi lưu/tải giữa ca để so fingerprint; Python/Release/Debug, native và Godot compatibility smoke pass theo report. Chưa có learned checkpoint v11 hoặc policy quality, nên không nghiệm thu gameplay tự chủ. [Plan](design/M5-v11-policy-worksite-actions-plan.md), [evidence](reports/M5-v11-policy-worksite-actions-evidence-2026-10-21.md), [manifest](../artifacts/m5-v11-policy-worksite-actions-2026-10-21-manifest.json).

## Handoff docs — thiết kế lại theo người đọc, 2026-10-18

Theo yêu cầu người dùng, ưu tiên thiết kế lại docs trước khi tiếp tục triển khai. Baseline checkout `6cc55ab`; giữ thay đổi phục hồi từ `730d341` đang có trong working tree. README giải thích mục đích/stack/folder tree/giai đoạn; [bản đồ docs](README.md) phân người đọc và nơi sở hữu thông tin; [cây bài toán](TECHNICAL_PROBLEMS.md) nối sáu nhóm lớn → P1–P16 → bước nhỏ/phương pháp/công thức/phép kiểm; [workflow](DEVELOPMENT_WORKFLOW.md) phục vụ Codex/OpenCode theo AGENTS. Player Guide bổ sung đường chơi thử dễ hiểu; History ghi mốc–vấn đề–giải pháp–evidence.

Giữ hướng agent-centric; sáu stat/XP kế thừa không là schema bắt buộc. Handoff docs-only này đã hoàn tất; nó không nghiệm thu thêm model hoặc performance. Trạng thái/gate hiện tại và next action được ghi trong các mục T1 bên dưới.

Quy ước trình bày theo yêu cầu người dùng: công thức dùng LaTeX inline/display, giải thích ký hiệu và đơn vị; không viết phương trình trong backtick. Preview Codex hiện tắt single-dollar math: dùng `$$...$$` cùng dòng văn bản cho inline và cặp `$$` trên dòng riêng cho display; kiểm bằng parser/render thay vì chỉ đếm ngoặc. Xem quy tắc biên tập ở docs/README.

## Quyền thực thi

- OpenCode triển khai chính; Codex review plan, diff và bằng chứng. Mỗi lần một lát có file/gate.
- Plan-only → Codex review → code/test → Codex acceptance. Contract thay đổi lớn phải review lại trước code.
- Không commit/push/publish, train dài hoặc chi phí cloud nếu chưa có yêu cầu tương ứng. Giữ thay đổi người dùng và tác vụ khác.
- Các lời giải thích/đề xuất trong tab thảo luận không tự thành gameplay/spec đã duyệt. M5 chưa hoàn thành.

## R1 — payroll prepare/apply (accepted 2026-10-19)

Refactor kiến trúc core theo [core architecture contract](design/core-architecture-refactor.md) §7.1/§9 đã được Codex nghiệm thu sau final diff review. Baseline `6cc55ab`; toàn bộ dirty T1/T2/docs giữ nguyên. Contract/evidence: [R1 evidence](reports/M5-R1-payroll-prepare-apply-evidence-2026-10-19.md).

- Files: chỉ `core/include/polyec/managers/finance_manager.hpp` và `core/tests/test_managers.cpp`; không đổi `core/src/realm.cpp`, CMake, save schema, native/Godot/Python, action/observation/reward/model hoặc gameplay.
- API: `prepare(day, region, public, claims, people, provinces)` pure trả `PayrollPlan` opaque (private default ctor; chỉ `FinanceManager` friend) và `apply(plan, current_day, people, provinces)` validate read-set rồi commit scalar. Wrapper `payroll(...)` giữ nguyên chữ ký cho `Realm::payroll`; không còn thuật toán payroll song song.
- Behavior giữ nguyên trên input hợp lệ: thứ tự claim và claim trùng, pro-rata integer truncating, rotation theo ngày, budget-before-tax, `tax_remainder` từng người, `tax_receipts` chỉ là counter, treasury vừa trả lương vừa nhận thuế; alias treasury công khai chỉ ghi một lần.
- Boundary/atomicity: validation state-domain và checked arithmetic (due sum, budget*claim, cash+paid, assessed, account/treasury/receipts/finals) chỉ chạy trên due>0 và reject nguyên tử (`out_of_range`/`logic_error`/`overflow_error`). Apply kiểm day, region, account/treasury/tax/receipts, rồi index/id/cash/remainder của từng recipient; lần đọc recipient đầu yêu cầu canonical `Person::id == claim.first`; lookup recipient sparse average-O(1) bằng `unordered_map`, không iterate map để tính/commit.
- Windows gates: Release, Clang 18.1.6 qua Zig, Ninja; configure `build/m5-r1-payroll` thành công ở lát này, final source rebuild exit 0; manager tests pass (101 runtime checks, 19 test functions, cộng compile-time assertion `!is_default_constructible_v<PayrollPlan>`); full CTest 10/10 (9.05 s); scoped `git diff --check` exit 0. Không có gate Linux theo quyết định Windows-only 2026-10-19.
- Không commit/push/train. R1 là refactor kiến trúc, không thêm persistent settle ID, không đổi save format, không phải T2 settlement/gameplay và không đóng M5.

## R2 — Employment transitions (accepted 2026-10-19)

OpenCode triển khai theo plan R2 đã review, giới hạn ở EmploymentManager, Realm transition/counter wiring và test manager/Realm/court. Codex review diff và nghiệm thu sau khi bổ sung preflight Work/Appoint để mọi reject xảy ra trước clear assignment; không đổi gameplay đã chốt, save format, binding, observation, reward hay policy. Chi tiết: [R2 evidence](reports/M5-R2-employment-transitions-evidence-2026-10-19.md).

- Release Windows build pass; manager tests pass; Realm 25,349 checks; Court 196 checks; full CTest 10/10 (9.29 s); scoped `git diff --check` pass, có cảnh báo line endings.
- Các hành vi cũ được giữ: counters và cách rebuild mỗi ngày, thứ tự referral khi bổ nhiệm, lương/notice/từ chức, job fields/timer stale theo đường cũ và save schema. `EmploymentManager` không sở hữu state; Realm vẫn sở hữu người/địa bàn và là command boundary.
- Preflight từ chối overflow ngày/cash trước mutation. Những biên ngày/tiền này không thể tạo thành fixture Realm hợp lệ do giới hạn save/day và invariant tổng tiền; test trực tiếp pure preflight. Integration Realm xác nhận reject giữ nguyên fingerprint, assignment và counters.
- Chỉ Windows được nghiệm thu theo phạm vi người dùng đã chọn; không có gate Linux chặn.

R3 điều phối tiến ngày đã được trích xuất và nghiệm thu trên Windows; `RealmDayPipeline` giữ đúng thứ tự pha còn `Realm` tiếp tục sở hữu dữ liệu và phép tính. Xem [bằng chứng R3](reports/M5-R3-day-pipeline-evidence-2026-10-20.md). Đây là refactor riêng, không tự đóng T2 hoặc toàn bộ core refactor.

## Trạng thái đã được ghi nhận

| Phần | Bằng chứng | Phạm vi |
|---|---|---|
| Membership/guardian | [M5-progress](reports/M5-progress.md), Windows 5/5 tại revision báo cáo | Index dẫn xuất/load/reject; không quỹ hộ/thừa kế hoàn chỉnh |
| Copilot core refactor repair | b8160f8 đã push; [report](reports/copilot-core-repair-2026-10-17.md), Windows 6/6 | Manager/WorldState/API/payroll repair; không app/AI build mới |
| Attention evaluator thuần | [plan được review](design/M5-activity-evaluator-plan.md), [report](reports/M5-activity-evaluator-2026-10-18.md): full 7/7, targeted 1/1 sau test-gate fix | Một actor, abstract ticks, checked arithmetic; chưa Realm/clock/save/body/settlement |

Phiên này tạo một handoff checkpoint gồm các lát M5/T1/T2/docs hiện có; commit/remote được xác định từ `git log` và `git status` khi phiên sau tiếp tục. Kiểm status và report/evidence trước thao tác; không suy report là code đã commit hoặc giữ mãi cùng trạng thái. Không chạy lại cả suite cho thay đổi docs-only nếu không có lỗi code mới.

## Lát đã nghiệm thu: T1 — timeline Realm (Windows accepted; Linux deferred)

T1a–T1b2d đã được nghiệm thu core/adapter/native/Campaign/Godot trên Windows (CTest và các suite tương ứng theo evidence từng lát bên dưới); Linux build/runtime parity được hoãn theo quyết định người dùng 2026-10-19 và không chặn M5. Mục dưới giữ contract, gates và evidence lịch sử của T1. R1–R3 đã được nghiệm thu theo từng lát. T2 attendance, bookkeeping, lương vùng và payroll xưởng/migration credit đã được triển khai theo các hợp đồng đã review; chu trình xưởng chạy trong dev smoke Godot. T3, phần tích hợp người chơi hoàn chỉnh và các nhóm M5 khác còn mở. File có chữ “implemented” không thay diff/test/acceptance.

### T1a: contract được ghi trong task record, cần kiểm chứng theo gate (snapshot 2026-10-18)

- Files: `core/include/polyec/schedule.hpp`, `core/src/schedule.cpp`, `realm.hpp/realm.cpp`, `core/tests/test_schedule.cpp`, `core/CMakeLists.txt`; `native/realm_runtime.cpp` chỉ phần guard khi bị tác động.
- Đơn vị đề xuất trong T1a record: 64 subday units/day, absolute int64 cursor. Đây là fixed point, không phút thật hoặc công thức cơ thể; không đổi ngầm tuổi 18 hay lịch nghiệp vụ hiện có.
- Sparse ScheduleBook theo actor; session có ID, owner, interval, priority/demand, counters và retirement cause/time. Không bản sao money/world hoặc mutable state cho UI.
- Submit/Cancel/Reprioritize qua command/sequence có ownership; record hiện dùng cùng `Person::decisions`. Submit không nhận interval hồi tố. Giới hạn sửa session đã xử lý phải có lý do và được review; không chặn overcommit vì “xấu”.
- `advance_ticks` tích hợp attention theo window, xử lý crossed day boundaries đúng một lần; `advance(days)` đi qua clock domain tương ứng. Không tăng payroll/học/care từ attention trong T1a.
- Save session/cursor/sequence/counters/retirement và sticky contract; load tạm validate trước swap. Record đề xuất save `POLYEC_REALM_7`, pilot policy contract vẫn realm-v6: phải kiểm bằng guard thực, không chỉ đổi tên.
- Khi world đã có timeline semantics mới, old pilot policy không tự chạy. Revalidate native restore/load trước thay live state; phải inventory Python/adapter/Campaign wrapper và test phần bị ảnh hưởng.

Trên đây là bàn giao từ task record, không một lần duyệt gameplay/balance mới. Nếu implementation lệch, cập nhật plan và review; không tự chọn archive có contract dễ hơn.

### Gate cần cho T1a trước acceptance

1. Nhiều phiên một actor, compatible/exclusive, overcommit, priority/ties/counters; input order không chọn thắng.
2. Reject wrong owner/ID/dead actor, invalid range/rate, stale/duplicate, overflow; fingerprint hoặc toàn state liên quan không đổi, lý do đúng validator cần kiểm.
3. Advance qua ranh giới ngày và horizon splitting tại non-event boundary; không double daily/settlement, không replay attention đã tích lũy. Save/load giữa interval khớp continuation.
4. Session owner chết/retired và canceled/reprioritized có quy tắc audit; không tự xóa chứng cứ hoặc nhận credit sau chết.
5. Pilot model guard thực trên inference/restore; save cũ/new và Campaign/binding có behavior rõ. Không silent compatibility từ shape giống nhau.
6. Unrelated commands giữ job theo contract được review; nếu sửa cổng busy/candidate/observation thì mở lại version/adapter tests đúng phạm vi, không giữ behavior sai chỉ để giữ test count.
7. Windows related suite + regression thực; Python/native/clock tests khi tác động. T1 chưa đóng nếu T1b pause/focus/units path trên Windows còn mở. Linux parity nằm ngoài phạm vi build/nghiệm thu hiện tại theo quyết định người dùng.

### Ngoài scope T1a

Không nhận attention như wage/learning/care/restoration. Không tự thiết kế fatigue/illness/death rates, reward hoặc chiến lược. Không train/model export/profile B khi chưa có tác vụ tương ứng. Chưa gọi nhiều hoạt động gameplay hoàn chỉnh chỉ từ schedule record.

## Backlog M5 giữ nguyên scope

| Gate | Kết quả cần có | Trạng thái bàn giao |
|---|---|---|
| T1a | Realm schedule/command/time/save + guards | Core Windows nghiệm thu 8/8 CTest; xem [evidence](reports/M5-T1a-evidence-2026-10-18.md). Native/Python actual parity tiếp tục ở T1b |
| T1b | Native clock/pause/focus, explicit units và adapter/binding parity trên Windows | Windows gates đã nghiệm thu; theo quyết định 2026-10-19, Linux build/runtime parity được hoãn và không chặn M5 |
| T2 | Attendance, ghi nhận công, sản xuất/trả lương theo công và không tính trùng | T2.1–T2.4 đã có evidence Windows; Gate 6b gồm migration save, payroll cash xưởng và dev smoke Godot. Xem [T2.4 Gate 6b](reports/M5-T2.4-gate6b-evidence-2026-10-20.md). Chưa là UI sản phẩm hay learned policy chọn xưởng |
| T3 | Sleep/exertion/recovery, care/lifecycle và personality/knowledge theo contract | Một số lát body state, sleep debt (gồm phần dư ngày lẻ và clamp 0/100), knowledge và quan hệ đã có report; effort/exertion, care/lifecycle và tích hợp đầy đủ T3 còn mở |
| M5.0/1 | Registry/schema/quyền; hộ động/quỹ/đất/địa vị/guardian, bảo toàn/save | Các lát quỹ, title/transfer/use, guardian, hôn nhân và nhiều nhánh di sản đã có; lần nối hai quỹ hộ qua hôn nhân/tử vong pass. Household lifecycle, quyền guardian còn thiếu contract và cách xử lý nhiều người hưởng với đất/xưởng vẫn mở |
| M5.2 | Cơ sở/hợp đồng/học nghề, đủ 16 nhóm hàng, vận tải/thị trường, A/B | Catalog 27 sản phẩm/16 nhóm có validator; pure planner và Realm-day/save tests xác nhận ba recipe không nhận biến thể thay thế đã nêu. Hành động mua/bán canonical, ba công thức đầu, cơ sở, chuyển hàng và chuỗi lệnh mua nguyên liệu → sản xuất → bán/mua sản phẩm qua chợ đã có C++ coverage. Helper thị trường mới chặn tràn số mà giữ giá/thuế hiện hành. Campaign path planner đã tách ra nhưng chỉ Campaign M4 gọi. Nguồn/nhà sản xuất cho các hàng còn lại, hợp đồng/học nghề, thị trường theo cầu và điều chỉnh giá tháng, vận tải quốc gia, policy và kiểm tra A/B còn mở |
| M5.3 | Tổ chức/chức, cưới/escrow/consent, estate/claim/nợ, giáo hội, C/D/E | Đã có đồng thuận kết hôn/ký quỹ và một số ranh giới về thẩm quyền/di sản; tổ chức, chức vụ, thừa kế đầy đủ, yêu cầu/nợ, giáo hội và bài kiểm tra C/D/E còn mở |
| M5.4 | Quốc gia 24+3, thành thị/quân sự/ngoại thương/UI vua, 3 chuỗi nhân quả | Đã có catalog tham chiếu 27 địa danh và truy vấn chuỗi nối địa lý có hướng xác định; Realm/Campaign vẫn là mô phỏng mẫu 7 vùng. Hai test Campaign/Realm nối một số hậu quả hiện có (cứu trợ đến nơi làm đóng hồ sơ; xưởng thừa kế tiếp tục sản xuất rồi bán/thu thuế), nhưng không phải ba chuỗi của quốc gia M5. Chưa có ánh xạ dân số atlas→NPC, tuyến vận tải hoạt động, thành thị, quân sự, ngoại thương hoặc UI vua |
| M5.5 | B/100k, stress/long-run, full-cycle perf và Windows package | Chưa có nghiệm thu M5 |
| Shared core/Python | Python gọi cùng C++ core; binding/parity/guards được kiểm trên Windows theo revision | Một số parity/runtime/replay đã pass; full M5 command/save/runtime matrix còn mở. Linux build/runtime không thuộc phạm vi hiện tại |

[Execution tracker của tab chính](handoff/M5-execution-tracker.md) được giữ tại đường dẫn cũ để tránh đụng tác vụ đang chạy. Đây là working ledger hỗ trợ; sau safe handoff đưa trạng thái/next action vào file CURRENT_TASK này, không duy trì hai nguồn chỉ đạo cạnh tranh.

## Mẫu plan trước code

```text
Task / baseline revision / dirty files cần giữ:
Before → after:
Files + class/functions; owner/state/index; units:
Success / reject nguyên tử / accepted-but-failed:
IDs, quyền/consent, chết, shortage, stale/duplicate, cạnh tranh, overflow:
Save / Campaign / binding / action / observation / reward / model impact:
Tests + commands + platform; chưa xác minh; ngoài scope:
Review: pending | approved-by-Codex + ngày + điều kiện
```

## Commands Windows core đã có

```powershell
& build/tooling/cmake/data/bin/cmake.exe -S . -B build/m5-activity-evaluator -G Ninja -DCMAKE_MAKE_PROGRAM=D:/Code/PolyEc/build/tooling/bin/ninja.exe -DCMAKE_TOOLCHAIN_FILE=D:/Code/PolyEc/build/tooling/zig-toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=OFF -DPOLYEC_BUILD_PYTHON=OFF
& build/tooling/cmake/data/bin/cmake.exe --build build/m5-activity-evaluator --parallel 4
& build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-activity-evaluator --output-on-failure
```

Giữ build dir/compiler đã cấu hình trừ khi task cần dir riêng. Flags OFF chỉ kiểm core; nếu thay native/binding phải thêm lệnh/gate cụ thể trước code. Test không dùng `assert` bị Release/NDEBUG vô hiệu; test reject không được pass nhờ stale sequence sai fixture.

## T1b1 — core review accepted 2026-10-19

- Codex tiếp tục qua OpenCode tại baseline `6cc55ab`, giữ nguyên dirty docs của người dùng. OpenCode sửa nhánh long-gap bỏ qua Pause failure; Codex duyệt plan rồi review diff. Contract: [T1b1](design/M5-T1b1-realm-clock-plan.md); [evidence mới](reports/M5-T1b1-review-2026-10-19.md).
- Codex rebuild toàn bộ `build/m5-t1b` và chạy CTest độc lập: exit 0, 9/9 pass; chạy trực tiếp clock test: 231 checks, exit 0. Diff check trong phạm vi sửa pass. Đây là acceptance core T1b1; không chứng nhận native/Python, T1b2 hoặc M5.
- Lỗi Pause do tràn revision được throw trước khi đổi controller state; không có cơ chế latch hoặc ép Realm thành paused khi command reject.
- **Follow-up at that time (completed):** OpenCode surveyed T1b2 and presented the native/Campaign/Godot clock and Python parity plans; subsequent accepted slices are recorded below.
- OpenCode model cấu hình `step-5-preview-free` không còn được CLI nhận diện; lượt này dùng `start-opencode.ps1 run -m opencode/beam-free`, không đổi cấu hình. Không commit/push/train.

### Bản bàn giao trước review (giữ để đối chiếu)

- Build dir: `build/m5-t1b`, Release, Clang/Zig, Ninja.
- New files: `core/include/polyec/realm_clock.hpp`, `core/src/realm_clock.cpp`, `core/tests/test_realm_clock.cpp`.
- Modified: `core/include/polyec/realm.hpp`, `core/src/realm.cpp`, `core/CMakeLists.txt`.
- Contract doc: `docs/design/M5-T1b1-realm-clock-plan.md`.
- CTest: 9/9 passed (`core_tests`, `trade_tests`, `realm_tests`, `court_tests`, `campaign_tests`, `manager_tests`, `activity_tests`, `schedule_tests`, `realm_clock_tests`); `realm_clock_tests` reports 170 checks, and `check()` now throws `std::runtime_error` on failure.
- Scope delivered: Realm-owned paused flag/revision (default bare Realm unpaused), `RealmClockCommand` Pause/Resume with stale/overflow/unknown guards, paused-aware `advance_ticks`/`advance`, `RealmClockController` wall-clock adapter, CLOCK_1 save/load before SCHEDULE_1, reset_after_restore.
- Not delivered: native/Python adapter changes, T1b2, T2, M5 completion. No commit/push/train.

## T1b2a — Python shared-core clock/schedule adapter accepted 2026-10-19

- OpenCode implemented only the approved Python binding and deterministic clock/schedule replay. Contract and run evidence: [T1b2a plan](design/M5-T1b2a-python-clock-plan.md), [report](reports/M5-T1b2a-evidence-2026-10-19.md).
- Codex independently rebuilt core (Ninja reported no work), ran full CTest: 10/10 passed in 7.64s; rebuilt Python binding and transition replay; existing Python binding replay passed 258 checks (2048 partner decisions, no neural inference); clock binding passed 103 counted checks and matched seven checkpoints against exact C++ serialization and sorted session records. Scoped diff check passed.
- **Follow-up at that time (completed):** OpenCode planned and delivered the native RealmRuntime clock integration while preserving learned-decision ordering; later Godot/Campaign slices are recorded below.
- T1b2a does not establish native learned-policy parity, Godot behavior, Campaign pause, portable-platform parity, T2 or M5 completion. No commit/push/train.

## T1b2b — native RealmRuntime pause bridge accepted 2026-10-19

- OpenCode added pause/resume delegates and read-only clock state to `RealmRuntime`; day `advance` rejects pause and maximum-horizon overflow before policy inference. It keeps load unpaused and retains Governance → Economic → Civilian inference before each Realm day. Contract and evidence: [T1b2b plan](design/M5-T1b2b-native-runtime-clock-plan.md), [report](reports/M5-T1b2b-evidence-2026-10-19.md).
- Codex independently ran the native runtime contracts with the three local realm-v6 ONNX models: 75 checks passed; native `runtime_contracts` CTest 1/1 passed; core CTest 10/10 passed; scoped diff check passed. Horizon preflight is code-reviewed but not exercised in a native test fixture.
- **Next action:** OpenCode plan-only for Godot Realm UI/session integration: synchronize UI pause/run/focus/save/restore with RealmRuntime clock, preserving explicit day-step semantics and inference order. Campaign's separate clock and portable Python parity remain open. T1/T2 and M5 remain incomplete.
- No commit/push/train/cloud.

## T1b2c — Godot Realm manual day/clock synchronization accepted 2026-10-19

- Codex reviewed and accepted the Godot Realm slice after the final fail-closed fixes. Contract/evidence: [T1b2c plan](design/M5-T1b2c-godot-clock-plan.md), [report](reports/M5-T1b2c-evidence-2026-10-19.md).
- Delivered: core `StepOneDay`; native `RealmRuntime::step_one_day`; GDExt clock bindings; interactive load paused; UI run/focus/long-frame/guide/save/load/manual-day/close/back-to-map synchronization. Failed long-frame, close and back-to-map pauses keep the panel visible with the pause error.
- Final Windows evidence: CTest **11/11**, Native **19/19**, UI **18/18**, Campaign **39/39**, Realm **471/471**; scoped `git diff --check` exit 0. No commit/push/train.
- T1b2c không đóng T1b (Campaign clock, portable parity), T2 hoặc M5. Giữ dirty/user files; không commit/push/train.

## T1b2d — Campaign clock/manual day accepted 2026-10-19

- Codex reviewed the OpenCode implementation against the user-approved Campaign semantics and supporting contract: [T1b2d plan](design/M5-T1b2d-campaign-clock-plan.md), [evidence](reports/M5-T1b2d-evidence-2026-10-19.md).
- Delivered: Campaign delegates pause ownership to Realm; automatic advance rejects paused state; manual day runs the labeled M4 controller and shared daily settle path once; interactive start/load pause; Campaign UI and app focus synchronize with core pause. Save schema and gameplay rules are unchanged.
- Windows verification: CTest 11/11, Native 19/19, UI 18/18, Campaign 49/49, Realm 471/471; independent targeted Campaign CTest 1/1 with 214/214 checks; profile workload restored with invariant true; scoped diff check passed. The Campaign wrapper horizon guard was reviewed in code but lacks a dedicated boundary fixture.
- T1b2d không đóng T2 hoặc M5. Tại thời điểm nghiệm thu, Linux parity còn được xem là mở; quyết định phạm vi Windows-only ngày 2026-10-19 hoãn gate đó. Giữ dirty/user files; không commit/push/train.

## Cập nhật phạm vi nền tảng — 2026-10-19 (historical snapshot)

- Người dùng xác nhận chỉ cần build và kiểm chứng Windows; Linux build, runtime parity và package Linux được hoãn, không là gate chặn T1b/M5 hiện tại.
- Giữ kiến trúc Python dùng cùng C++ core qua pybind11; các binding và luật chung vẫn phải được kiểm trên Windows. Quyết định này chỉ thay phạm vi nền tảng nghiệm thu, không thay gameplay, model hay chất lượng M5.
- Helper Linux-only T1b2e chưa được nghiệm thu và đã được gỡ cùng golden fixture của nó; các kiểm Windows đã có ở T1b2a–T1b2d vẫn là evidence riêng theo revision.
- Người dùng chốt cả ba vai trò (lao động, giáo viên, quan chức được bổ nhiệm) trong hierarchy; Codex review và nghiệm thu foundation Employment/Job classification trên Windows (realm tests 25,322 checks, full CTest 10/10, Python smoke pass). Contract và evidence: [T2 employment plan](design/M5-T2-employment-classification-plan.md), [report](reports/M5-T2-employment-classification-evidence-2026-10-19.md). Read-only: `Job` query-only với singleton dispatch, `describe_job` pure resolver, `job_descriptor` trả `const Job*` (nullptr = Idle/None), Python person dict thêm `employment_state`/`job_type` đọc không đổi. Không schema/lifecycle/payroll/counter/RL change.
- **Snapshot trạng thái cũ ngày 2026-10-19 (đã superseded):** dòng dưới mô tả đúng tình hình tại thời điểm đó nhưng không còn là trạng thái hiện tại. T2.1–T2.4 và R3 đã có evidence Windows; xem bảng M5 cập nhật ở đầu CURRENT_TASK và các báo cáo ngày 2026-10-20.

### Kiểm kê contract trước mỗi lát có tác động xuyên hệ thống

Bảng này là checklist bàn giao, không là tác vụ triển khai mới song song với tab chính. File/gate chính xác lấy từ supporting plan đang active; không mở rộng lát đang làm để sửa mọi hàng cùng lúc.

| Bề mặt | Phải ghi rõ trong plan/evidence |
|---|---|
| Domain và quyền | Ai sở hữu record/tài khoản; ai được sửa; nguồn truth và index dẫn xuất |
| Clock và hoạt động | Tick/ngày, phase boundaries, pause, pending và đã xử lý; split continuation |
| Transaction | Phạm vi mutate; reject so với nỗ lực thất bại; sequence/settlement ID; overflow |
| Save/load | Field/schema đổi; save cũ/new; parse tạm; corrupted reference/count; continuation |
| Campaign/Godot | Wrapper và controller có gọi đúng clock/command; UI chỉ nhận view theo quyền biết |
| Python/native | Binding có gọi cùng transition; restore không vượt guard; GIL/thread ownership |
| RL | Action/observation/reward/cadence/partner đổi hay giữ; bundle nào được phép; mask/hidden/reset |
| Chất lượng test | Fixture chạm đúng validator; numerical oracle; Release checks còn hoạt động; regression liên quan |
| Bằng chứng | Revision/dirty scope, lệnh/exit/platform, counts/workload, phần không chạy và gate còn mở |
| Docs | Scope trong plan; phương pháp trong dev guide; tính năng chơi trong player guide; acceptance vào history/report |

Phân rã kỹ thuật theo P1–P16 và công thức/giới hạn xem [DEVELOPER_GUIDE](DEVELOPER_GUIDE.md); chi tiết vùng, thẻ, thông tin, chính trị, kinh tế và UI xem phần II [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md). Không lấy tham số cân bằng v3 làm lựa chọn đã review cho timeline mới.

## Cập nhật khi bàn giao

Ghi source/revision + dirty scope, plan được review, diff, commands/exit/test counts, evidence, giới hạn và một next action rõ. Đánh dấu accepted chỉ sau review và test thật. Đưa việc đã xong vào HISTORY, số liệu vào reports. Xem archive chỉ khi cần provenance; không thực hiện prompt lịch sử.
