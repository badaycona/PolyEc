# CURRENT_TASK — hoàn thiện gameplay M5 trước core refactor và RL

Cập nhật: 2026-10-25. Đích do người dùng giao: hoàn thiện toàn bộ base game M5; sau đó hoàn tất core refactor R1–R4; chỉ khi hai phần này đã đủ bằng chứng mới tạo dữ liệu, huấn luyện và đánh giá RL. Không lên lịch theo giờ. Sau mỗi nhóm tự chọn điều kiện thiếu có tác động lớn tiếp theo và tiếp tục cho đến khi người dùng yêu cầu dừng. Được tự chốt setting gameplay thử trong phạm vi đã cho phép, ghi phiên bản/đơn vị/giới hạn/lý do/phép kiểm. Giữ nguyên artifact, model và run chưa được theo dõi nếu không thuộc nhóm hiện tại.

## Baseline source hiện tại

- Save writer: `POLYEC_REALM_75`; model contract: `realm-v43`; household/timeline config: `v54`. Tensor giữ `382` giá trị trạng thái, `400` candidate actions, `5.182` giá trị quan sát, reward14. Các model có contract cũ không đại diện cho luật mới.
- Bản đồ Atlas có27 vùng/27 settlement. Đã có cư trú Urban/Rural/Unknown, census giới hạn theo quyền, đường Travel/Migrate, sức chứa nhà công, một đại diện hội đồng mỗi settlement do Sovereign bổ nhiệm và các chuỗi sinh kế/market/transport được kiểm riêng.
- Học nghề hiện dùng `formal-apprenticeship-v2`: học phí10 đồng thay cho20 của v1, vì ca Atlas sau30 ngày Study có người học16 đồng và v1 làm offer không hợp lệ. C++ command→đồng ý/chuyển tiền/quyền Work→lịch/số ngày→kết quả→save/load nối cùng C++ Realm; Python/native snapshot và Godot action smoke đọc/gửi đúng dữ liệu đó. C++ test riêng82/82; Python model contract21 và gameplay config51; Godot action smoke19/19, smoke tổng32/32. Godot smoke dùng candidate/fingerprint/`realm_command` thật, không thao tác chuột cửa sổ. [Plan v2](design/M5.2-formal-apprenticeship-tuition-v2-plan.md), [bằng chứng v2](reports/M5.2-formal-apprenticeship-tuition-v2-evidence-2026-10-25.md), [evidence v1 giữ nguyên](reports/M5.2-formal-apprenticeship-evidence-2026-10-25.md).
- Full Release CTest ở contract v42:65/66. Bài còn lỗi là `realm_tests`, dừng ở check434 khi so byte toàn bộ save lịch sử v14; hash oracle chưa sửa. Sau cập nhật v43, đã build lại và chạy Realm target, vẫn dừng đúng assertion này. Đây là giới hạn có trước học nghề; không dùng 65/66 để tuyên bố suite hiện tại pass hết. V2 kiểm trực tiếp target liên quan. Báo cáo migration cũ: [save v14](reports/M5-core-legacy-save-v14-migration-evidence-2026-10-25.md).
- Chưa có learned policy với contract hiện tại. Chưa mở dataset/training/selection/validation/holdout. Không dùng run smoke cũ làm policy của game.

## Mức hoàn thiện và câu hỏi các phần đã được ghép với nhau chưa

| Mảng | Đã kết nối và có bằng chứng riêng | Còn thiếu trước khi gọi M5/game đủ tính năng |
|---|---|---|
| Người, hộ, xã hội | ID/vòng đời, hộ/tiền hộ, quan hệ thưa, cư trú, lao động, consent, hôn nhân, xét xử/di sản và các consumer xã hội đều chạy qua Realm command. Học nghề nối Work, consent, tiền, lịch và save/load; v1 đang mở/có tuition20 được giữ nguyên khi chuyển sang v2. | Rà game-wide các ca đồng sở hữu, người phụ thuộc, thừa kế, kiện tụng và chính trị; kiểm save/replay sau các chuỗi dài; thủ công xem bố cục Godot ở mốc app rộng. |
| Sản xuất, hàng, thị trường | Catalog30; Work, quyền làm việc, kho xưởng/vùng, mua bán, logistics và một số producer→consumer cụ thể đã được ghép; có các test cặp như ranch→DryFood, Glassworks→DecorativeGlass, Pottery→FinePorcelain, specialty→consumer. | Chưa chứng minh mọi nguồn/consumer/nhà sản xuất/vận tải hoạt động cùng trong các thế giới nhiều vùng. GeneralLuxury test đặt Tea/Incense sẵn trong vùng recipe; phải kiểm tuyến hàng xuyên vùng và các nhóm hàng còn thiếu theo audit M5.2. Cần giá/thiếu hàng/sinh kế trong cùng run, không chỉ target riêng lẻ. |
| Bản đồ, nhà, settlement | Atlas27, route và thời gian đi lại, migration, census, xây/chuyển nhượng/cho thuê nhà tư đã nối với hàng, tiền, ngày đến hạn và save/load ở test chức năng. | Terrain/passability chưa làm đổi đường đi; thiếu khu phố/dịch vụ đô thị, thành viên/bầu cử hội đồng. Còn lượt chơi Godot toàn màn hình sau khi nhóm chức năng ổn định. |
| Quân sự | Consent phục vụ/tham chiến, roster/tập kết, lương, khẩu phần, kho và shipment, nghĩa vụ tiếp tế, raid/phòng thủ/rút lui; WarHorse ảnh hưởng travel, Leather ảnh hưởng defense. | Chưa có campaign nhiều vùng có đối thủ và lãnh thổ riêng, chiếm/thu hồi vùng, tù binh, đầu hàng, chiến lợi phẩm hoặc đình chiến. Raid hiện khởi tạo bằng command, chưa phải campaign tự vận hành. |
| Chính quyền và ngoại thương | Thuế/quỹ, hội đồng một đại diện được bổ nhiệm, đối tác thương mại có tiền/hàng hữu hạn, order/offer/accept/reject/expiry. | Thiếu bầu cử/hội đồng thành viên, chính sách đối tác có quyết định riêng, rủi ro và chính sách thương mại đầy đủ, chuỗi cảng nhiều vùng. |
| Game/app và phát hành | Godot gọi GDExtension; Python gọi cùng C++ core; panel có thao tác cho nhiều lệnh và báo cáo snapshot. Các chức năng phần lớn đã ghép theo adapter nhưng chưa có một lượt chơi xác nhận toàn bộ hệ thống cùng tác động. | Click-through liên lớp có kịch bản chuẩn bị; kiểm Profile B, stress/long-run, performance cả chu trình, offline Windows package và test cấu hình còn lại theo mốc ổn định. |
| RL | Trainer/evaluator và model lịch sử có trong repo để truy nguyên. | Chưa tạo dữ liệu hoặc chạy RL cho contract43. Chỉ bắt đầu sau M5 + R1–R4; khi chạy phải lưu dữ liệu nguồn, manifest cấu hình, biểu đồ đúng run, split seed train/validation/holdout, đối chứng độc lập và đánh giá ONNX/native. |

Trả lời câu hỏi ghép hệ thống: một số chuỗi cụ thể đã kết hợp và kiểm được (ví dụ sinh sản phẩm→kho→market→người mua; tuyến→thực phẩm→nutrition; xưởng→Work→lương; quân vụ→consent→roster→lương/khẩu phần). Các nhóm này vẫn có mức chứng minh khác nhau; test riêng của từng cơ chế không chứng minh toàn bộ thế giới hoạt động phối hợp trong một playthrough. Hạng mục tích hợp game-wide còn mở như bảng trên.

## Công việc đang làm và quyết định

Nhóm học nghề v2 đã qua C++86/86, Python model21/gameplay config51, Godot action smoke19/19 và worksite/UI smoke32/32. Bổ sung tương thích ngược: lời mời v1 đang mở vẫn tính20 và save khóa v1 tải được; candidate mới dùng10. Build Release core, GDExtension và Python binding đã chạy. Full CTest chưa chạy lại ở v43; Realm runner còn cùng lỗi historical hash check434 đã ghi. Chưa click chuột thủ công (đây là usability review), nhưng headless test đi qua candidate/mask/snapshot và `realm_command` thật. Commit/push v1: `2a952fc`; v2 đang chờ commit/push.

## Kế tiếp sau nhóm đang làm

1. Hoàn tất review tài liệu/compatibility v2, commit/push đúng file và giữ report v1 immutable.
2. Lập contract rồi ghép chuỗi hàng xuyên vùng còn thiếu: Tea/Incense/Silk/GeneralLuxury từ producer qua transfer/route/stock/market tới consumer; cùng run phải quan sát shortage/giá/kết quả người mua. Test GeneralLuxury trước đây nạp Tea/Incense sẵn ở vùng xưởng nên chưa chứng minh tuyến thực.
3. Rà source/consumer cho các hàng còn thiếu; sau đó tiếp tục tính năng gameplay M5.3/M5.4 còn thiếu: terrain/passability, chính trị/hội đồng, campaign nhiều vùng, kiểm cân bằng và tích hợp app. Mỗi nhóm có một plan và một evidence report; không hạ scope.
4. Khi toàn bộ M5 qua kiểm tích hợp, hoàn thành core refactor R1–R4 theo `IMPLEMENTATION_PLAN.md`.
5. Chỉ sau M5 và R1–R4 mới lập dataset từ C++ core hiện hành, huấn luyện và đánh giá RL. Mỗi run phải có dữ liệu nguồn, manifest và biểu đồ đúng run theo `DEVELOPMENT_WORKFLOW.md` mục4.

## Phạm vi kiểm tra

Ngay sau chỉnh sửa core: build target bị ảnh hưởng, chạy test trực tiếp và reject/biên tương ứng. Học nghề v2 đã qua command/transaction/ngày/attendance/save-load trong C++, Python binding/parity, GDExt build và Godot candidate/offer/accept action flow. Chưa có lượt click chuột trực quan; không nhầm headless action smoke với usability review. Full CTest đã chạy ở v42 và còn lỗi hash lịch sử; không chạy lại rộng sau đổi học phí vì target liên quan đã kiểm. Khi các nhóm tích hợp ổn định mới chạy kiểm app rộng, performance/profile/package và cấu hình còn lại. Không bundle/build cả app sau mỗi sửa core.

Nguồn điều phối: `IMPLEMENTATION_PLAN.md`; cách kiểm: `DEVELOPMENT_WORKFLOW.md` mục4; giải thích bài toán/architecture: `DEVELOPER_GUIDE.md`; phân tích: `TECHNICAL_PROBLEMS.md`; nhật ký: `DEVELOPMENT_HISTORY.md`; setting và kết quả: `docs/design/` và `docs/reports/`.
