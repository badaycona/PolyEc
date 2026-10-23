# Báo cáo triển khai Atlas27 và bản đồ chính

Ngày: 2026-10-23  
Phạm vi: dữ liệu địa lý giả tưởng `atlas-cartography-v1`, API chỉ đọc cho giao diện, bản đồ Atlas27 ở màn hình chính và các phép kiểm trực tiếp.  
Trạng thái: core, GDExtension và Godot headless đã kiểm; các phép smoke giao diện/native đạt. Không có ảnh chụp để đánh giá thủ công bố cục ở độ phân giải cửa sổ thực.

## Người chơi nhìn thấy gì sau thay đổi

Màn hình chính không còn vẽ hai đa giác Thanh Điền và Hắc Sơn như thể đó là toàn bộ thế giới. Nó lấy dữ liệu bản đồ có phiên bản từ native extension và hiển thị 27 vùng cùng tên, ID, nhóm địa lý và các mối nối Atlas đã khai báo. Người chơi có thể chọn cả vùng ID 0, dùng cuộn chuột để phóng/thu quanh con trỏ, kéo bằng nút chuột giữa hoặc phải, và dùng nút hiện có để bật/tắt đường nối. Khung hồ sơ hiện tên vùng, nhóm địa lý, mô tả lợi thế/khó khăn và đặc sản được ghi trong Atlas.

Đường sông, đường bộ và vận tải biển chỉ nhận kiểu vẽ riêng khi Atlas đã chỉ rõ mode tương ứng. Các link chưa xác định phương thức hoặc hướng được vẽ bằng nét đứt trung tính. Đường vẽ này là minh họa topology của catalog; nó không tự mở đường vận tải, không đặt thời gian/cước/chuyên chở và không thay luật tìm đường đang chạy trong Realm.

## Setting và dữ liệu địa lý

`atlas-cartography-v1` là bản đồ hư cấu cố định dùng cho mọi campaign seed. Hệ tọa độ là `map_unit`, không phải kilômét. 27 ID hiện hành giữ nguyên: 24 lãnh địa và ba khu triều đình. Mỗi lãnh địa có lục giác đều được đặt trong lưới so le bốn hàng; hình học được sinh xác định từ ID, vì vậy cùng ID luôn có cùng polygon và anchor. Đây là lựa chọn để tạo bản đồ liền mạch có thể chọn được, không phải khảo sát lịch sử hoặc bản đồ địa hình đo đạc.

Nhóm địa lý và đặc sản lấy nguyên từ catalog Atlas hiện hữu. Ba khu triều đình không được gán dân số hoặc diện tích mới. Dân số/diện tích bối cảnh của 24 lãnh địa không được đổi thành Person, census thành thị hoặc số liệu Realm. Geometry không cấp quyền đất, tiền, hàng hóa hay năng lực sản xuất. Thành phố, nếu được thêm sau này, vẫn phải là địa điểm nằm trong lãnh địa và dân thành phố phải là tập con có thể truy về Person hiện có.

## Thay đổi source

- [atlas_cartography.hpp](../../core/include/polyec/atlas_cartography.hpp) thêm catalog polygon/anchor bất biến cho 27 ID, định nghĩa đơn vị bản đồ và kiểm tra compile-time ID liên tục, giới hạn khung bản đồ, diện tích polygon dương.
- [test_world_atlas.cpp](../../core/tests/test_world_atlas.cpp) mở rộng test Atlas: xác nhận 27 polygon khớp đúng 27 ID, anchor nằm phía trong mỗi cạnh và tâm của lãnh địa khác không nằm bên trong polygon đang kiểm. Kiểm tra hiện tại không thay các phép kiểm chuyên sâu về giao điểm mọi cặp cạnh; việc nhìn hình trong Godot vẫn chờ engine.
- [polyec_session.cpp](../../gdext/polyec_session.cpp) thêm `atlas_map_data()` chỉ đọc, trả về phiên bản, đơn vị, polygon, anchor, tên, nhóm, mô tả, đặc sản, nhãn khu triều đình và 31 link. Session tạo vùng chọn cho toàn bộ Atlas. `GameWorld` dành ID 0 làm giá trị rỗng, nên lớp giao diện chuyển Atlas ID `0–26` thành ID nội bộ `1–27` khi gửi lệnh, rồi chuyển lựa chọn về Atlas ID khi trả snapshot; người chơi và catalog vẫn dùng ID chuẩn.
- [map_canvas.gd](../../godot_project/ui/prototype/map_canvas.gd) thay hoàn toàn hình học hai vùng thử nghiệm bằng map renderer nhận dữ liệu Atlas. Picking dùng polygon thật và trả `-1` khi click ngoài bản đồ để không nhầm khoảng trống với lãnh địa 0. Pan/zoom giữ map unit nhất quán; tuyến lấy từ link source, không tự dựng đường qua hai địa danh hardcode.
- [main.gd](../../godot_project/ui/main.gd) nạp dữ liệu Atlas vào map và lấy hồ sơ vùng từ cùng catalog thay cho hai đoạn mô tả riêng của prototype. Realm vẫn là nguồn sự thật cho dữ liệu mô phỏng; màn hình chính chỉ hiện thông tin tĩnh Atlas ở nhóm chức năng này.

Không đổi schema save, observation/action/reward, model contract, quyền/consent, logic vận tải hoặc sản xuất. Campaign M4 và các test riêng của nó không đổi.

## Kiểm tra đã chạy

1. Build target `polyec_world_atlas_tests` trong cấu hình Release hoàn tất. Target này kéo theo thư viện core cần để liên kết; không build `ALL_BUILD` hoặc chạy toàn bộ CTest.
2. Chạy riêng `world_atlas_tests`: **1/1 pass** sau khi cập nhật hình học so le cuối cùng.
3. Build đúng consumer `polyec` Release trong `build/windows` hoàn tất, tạo `godot_project/bin/polyec.windows.template_release.x86_64.dll`. Đây là xác nhận C++ GDExtension compile/link với API mới, không phải xác nhận UI đã chạy.
4. Godot 4.5.1 `--headless --script res://tests/ui_smoke.gd`: **49/49 pass**. Ca này nạp main scene thật, kiểm 27 polygon anchor trả đúng ID, click mở đúng hồ sơ, pan/zoom giữ picking, vị trí ngoài mọi polygon trả `-1`, và Atlas ID0 vẫn chọn được qua adapter nội bộ.
5. Godot `--headless --script res://tests/native_smoke.gd`: **19/19 pass**, gồm snapshot có27 lãnh địa với ID0–26 và tên đúng; chọn ID hợp lệ, từ chối ID ngoài miền, selection cũ không bị đổi khi reject.
6. Godot `--headless --editor --quit` quét/import project thành công sau khi thêm renderer và bài smoke. Main scene cũng đã khởi động headless không báo lỗi.
7. `git diff --check` không báo whitespace error. Có cảnh báo chuẩn hóa CRLF của Git cho hai tài liệu lịch sử/plan.

Không có screenshot để đánh giá thủ công bố cục cửa sổ ở nhiều độ phân giải. Các smoke kiểm logic chọn/render frame không thay cho kiểm mỹ thuật hoặc accessibility.

## Trạng thái RL khi ưu tiên hoàn thiện game

Theo chỉ đạo mới của người dùng, chưa chạy tiếp chọn policy, validation hoặc holdout cho đến khi hoàn thiện phiên bản game có đủ tính năng theo roadmap. Run Civilian đang hoạt động được để chạy xong seed an toàn rồi dừng trước seed kế tiếp. Seed `20261101` hoàn tất 25 PPO update, 25.600 quyết định focal, checkpoint, manifest, history, policy và hai biểu đồ có JSON nguồn. Các tệp thuộc cùng run ở [thư mục seed 20261101](../../artifacts/m5-v15-r1-candidates-2026-10-23-retry2/civilian/seed-20261101/); runset [runset.json](../../artifacts/m5-v15-r1-candidates-2026-10-23-retry2/civilian/runset.json) ghi một seed hoàn tất nhưng `complete=false`, vì vậy đây không phải candidate set đủ ba seed, không đủ điều kiện validation/holdout và không chứng minh policy tốt hơn.

Biểu đồ reward/return của đúng seed này: [training.png](../../artifacts/m5-v15-r1-candidates-2026-10-23-retry2/civilian/seed-20261101/training.png), dữ liệu nguồn [training.json](../../artifacts/m5-v15-r1-candidates-2026-10-23-retry2/civilian/seed-20261101/training.json). Biểu đồ phân bố quyết định và số cơ hội theo loại lệnh: [training-intents.png](../../artifacts/m5-v15-r1-candidates-2026-10-23-retry2/civilian/seed-20261101/training-intents.png), dữ liệu nguồn [training-intents.json](../../artifacts/m5-v15-r1-candidates-2026-10-23-retry2/civilian/seed-20261101/training-intents.json). Đây là số liệu huấn luyện một run, không phải điểm đánh giá độc lập. Không có dataset trajectory tái sử dụng trên đĩa: rollout được dùng trực tiếp trong bộ đệm của PPO; tệp JSON ghi lại số liệu tổng hợp theo update và Intent. Khi RL được mở lại, báo cáo phải giải thích rõ nguồn world seed, trạng thái ban đầu, quyết định/transition dùng để cập nhật, phần nào giữ trong RAM hay lưu ra đĩa, và cách tách train/validation/holdout.

## Mức hoàn thiện thế giới và phần còn thiếu

Audit trước triển khai ở [báo cáo hoàn thiện thế giới](M5-world-completeness-audit-and-next-work-2026-10-23.md) đối chiếu plan và source, không phải playtest. Atlas27 nay có geometry và đường chọn trong giao diện source, nhưng vì chưa chạy Godot, phần hiển thị/picking thực tế chưa được xác nhận. Các hệ thống khác vẫn có mức độ hoàn thiện khác nhau:

| Hệ thống | Đã có | Còn thiếu để gọi game M5 hoàn chỉnh |
|---|---|---|
| Thế giới Atlas | 27 lãnh địa, 31 link, nhóm/đặc sản; nay thêm polygon giả tưởng cố định và API đọc bản đồ | Xác nhận Godot runtime; địa hình chi tiết, thành phố/census/đại diện và quan hệ địa lý với đường đi cần các hợp đồng riêng |
| Kinh tế/sản xuất | nhiều catalog hàng, xưởng, lệnh, quỹ, thị trường, vận tải nội địa/ngoại thương và chuỗi quyết toán | 3 nhóm đặc sản chưa khớp hàng hóa chính xác (thủy tinh thường, đồ gốm, da thuộc); không được giả vờ chúng đã có công thức hàng tương đương |
| Quản trị và đời sống | person/household, quyền sở hữu/sử dụng đất, thừa kế, chức vụ, lao động, học, sức khỏe, hôn nhân, cứu trợ và thông tin quan sát | Cần kiểm tra qua trải nghiệm game tích hợp; chưa chứng minh đủ người dùng có thể tự hiểu và hoàn tất campaign |
| Quân sự | tự nguyện, trả công, huy động, roster có Sovereign chấp thuận, di chuyển thật, nghĩa vụ tiếp tế và shipment | Chưa có luật tác chiến, kết quả trận, thương vong, bắt giữ, kiểm soát đất hoặc đình chiến; phải thiết kế và kiểm quyền/consent trước khi code |
| AI | C++ core chung với trainer; có PPO và bộ chạy đánh giá/selector/holdout theo kế hoạch | Chưa có policy đạt lựa chọn validation/holdout và chạy cùng source trong Godot; RL đang hoãn theo chỉ đạo |
| Quy mô và phát hành | có benchmark lõi từng profile và các phép đo từng phase | Profile B hoàn chỉnh, stress/long-run, benchmark toàn chu trình ở quy mô chốt, tích hợp app/model và gói Windows offline còn thiếu |

## Việc kế tiếp được chọn

Trước hết hoàn thiện kiểm tra Godot của bản đồ ngay khi có engine runtime dùng được; không coi build native extension là bằng chứng UI chạy. Sau đó tiếp tục backlog gameplay theo IMPLEMENTATION_PLAN; các thiếu hụt chiến đấu, city/census và nhóm hàng unresolved phải được thiết kế riêng, giữ quyền/consent và không nhập nhằng dữ liệu bối cảnh thành số liệu game. Chỉ quay lại training/selection RL sau khi phiên bản game feature-complete chạy qua tích hợp, quy mô và package đã chốt. M5 chỉ được báo hoàn tất sau khi có đầy đủ bằng chứng tích hợp, policy đã học, hiệu năng và package theo các gate đã ghi.
