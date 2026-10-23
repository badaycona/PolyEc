# Plan — Atlas27 và dữ liệu địa lý giả tưởng v1

Ngày lập: 2026-10-23  
Trạng thái: đã triển khai và kiểm core/GDExtension/Godot headless theo chỉ đạo mới ngày 2026-10-23; hoàn thiện phiên bản game đầy đủ trước khi tiếp tục huấn luyện/đánh giá RL.  
Audit nền: [Báo cáo mức hoàn thiện thế giới M5](../reports/M5-world-completeness-audit-and-next-work-2026-10-23.md).  
Roadmap: [IMPLEMENTATION_PLAN](../IMPLEMENTATION_PLAN.md) §4, §12, §27.2, §29–30.

## Hành vi cần đạt

Người chơi mở màn bản đồ chiến dịch và thấy đúng 27 vùng mà Realm sử dụng, có thể chọn một vùng sau khi kéo/phóng bản đồ, nhìn các tuyến Atlas đã khai báo và phân biệt đất liền, lưu vực, cao nguyên, duyên hải, cửa ải và khu triều đình. Tên/ID địa lý trong map và số liệu từ PlayerView phải cùng trỏ tới một lãnh địa; map không tự tạo tiền, dân, hàng, quyền hoặc thông tin bí mật.

## Luật và setting đã quyết định

- Phiên bản setting: `atlas-cartography-v1`.
- Atlas là thiết kế giả tưởng do nhóm sản phẩm tạo. Không gắn nhãn dữ kiện lịch sử/đo đạc.
- Dữ liệu địa hình nền cố định giữa campaign seeds.
- Hệ tọa độ bản đồ dùng `map_unit`; không gọi là km và không suy diện tích từ polygon trước khi hiệu chuẩn.
- Các ID `0–26` từ `kWorldAtlas` là chuẩn: 24 lãnh địa và 3 khu triều đình. Mỗi geometry có một ID, không thêm vùng hành chính thứ 28.
- Hình dạng/picking chỉ phục vụ bản đồ và định vị. Chúng không đổi logic vận tải/sản lượng cho tới khi có contract riêng về phương tiện, chiều, thời gian, capacity, phí, mùa và passability.
- Dữ liệu population/area hiện tại vẫn là thông tin bối cảnh của 24 lãnh địa; không chuyển tự động thành số Person hoặc city census. Ba khu triều đình không có population/area đầu vào.
- Thành thị sau này là địa điểm nằm trong lãnh địa. City population phải là tập con có thể truy ngược của Person trong Realm, không phải con số cộng thêm vào total quốc gia.

## Khảo sát có giới hạn đã làm

- `core/include/polyec/world_atlas.hpp` khai báo 27 lãnh địa/31 link nhưng không có tọa độ hay polygon; một phần link không chỉ định route mode.
- `core/include/polyec/realm.hpp` public projection được dẫn qua `RealmView`; `core/src/realm_view_projector.cpp` chiếu ngày, số vùng, số người, province, fact/knowledge và trạng thái draft, chưa có Atlas geometry.
- `godot_project/ui/campaign/campaign_map.gd` vẽ từ `Campaign` snapshot; UI chính `godot_project/ui/main.gd` đang nạp `map_canvas.gd`, vốn chỉ có polygon cho prototype 2 vùng.
- Không thấy nguồn geometry Atlas27, đường biên thành phố hay census đô thị trong các file đã khảo sát. Đây là căn cứ để tự thiết kế **geometry giả tưởng**, không phải căn cứ để tự gán dân số.

## Phạm vi code dự kiến và ownership

Một nhóm chức năng, không mở writer cạnh tranh:

1. Thêm catalog geometry bất biến của 27 lãnh địa trong core. Mỗi vùng phải có ID Atlas hiện tại, polygon không tự giao, label anchor nằm trong polygon, và loại feature có nguồn từ setting v1.
2. Bổ sung phép kiểm hình học thuần cho bounds, diện tích khác 0, polygon tự giao/đỉnh lặp không hợp lệ, điểm label nằm trong vùng, uniqueness/coverage đủ 27 ID. Link Atlas vẽ cùng ID endpoint; không tự đặt mode unspecified thành route gameplay.
3. Mở dữ liệu bản đồ qua view chỉ đọc của native/GDExtension; Realm vẫn sở hữu số liệu simulation, geometry không được sửa qua UI.
4. Nối UI campaign map hiện có với Realm view Atlas27 theo ID. Giữ M4 map/test fixture cũ cho Campaign; không đổi map prototype 2 vùng thành bản đồ chính của M5.
5. Chỉ sau khi đường hiển thị/picking ổn định mới thêm overlay số liệu PlayerView. Filter observer áp dụng trước khi tạo overlay/tooltip; hidden fact và cargo không được đọc thẳng từ Realm để vẽ.

File thực hiện: `core/include/polyec/atlas_cartography.hpp`, `core/tests/test_world_atlas.cpp`, `gdext/polyec_session.cpp`, `godot_project/ui/prototype/map_canvas.gd`, `godot_project/ui/main.gd`. API `PolyEcSession.atlas_map_data()` chỉ đọc, trả version, đơn vị, 27 polygon/anchor và 31 link đã có. UI dùng riêng dữ liệu này; click vùng gửi ID Atlas tới chọn lãnh địa trong shell, không coi population/area bối cảnh là số Person. Không sửa model/binding Python/trainer; bản đồ không đổi model contract/save schema. Do người dùng yêu cầu hoãn RL và ưu tiên tính năng game, giữ artifact training đang có, dừng Civilian sau seed đang chạy tại ranh giới an toàn, và không chạy selection/validation/holdout trong giai đoạn hoàn thiện gameplay. Build target test hình học trước, sau đó build đúng GDExtension consumer và kiểm Godot nếu engine cài sẵn.

## Lượt kiểm

### Trước và trong triển khai

- Test core geometry: đủ/duy nhất 27 ID; polygon finite/bounds/không tự giao; anchor ở trong; link endpoint hợp lệ; thay đổi thứ tự danh sách không đổi identity; lỗi dữ liệu bị reject.
- Godot smoke `tests/ui_smoke.gd`: tất cả27 anchor/polygon trả đúng ID, chọn vùng và tên hồ sơ khớp, ID0 qua adapter, ngoài vùng trả−1, pan/zoom không đổi vùng đang trỏ; cùng test giao diện tổng 49/49.
- Test binding/view: Realm 27 vùng và dữ liệu geometry có cùng index/ID; build đúng consumer trực tiếp do API mới.
- Không chạy lại full Realm suite hoặc full app khi chỉ sửa helper core. Sau khi nhóm geometry ổn định chạy chuỗi map-session C++→GDExtension→Godot; nếu engine không sẵn có, report nêu rõ chưa nghiệm thu session thật.

### Mốc sau nhóm

Đã qua: build riêng `polyec_world_atlas_tests` Release, CTest `world_atlas_tests` 1/1, build `polyec` Windows Release, Godot 4.5.1 headless editor scan, UI smoke49/49 và native smoke19/19. Một lần capture PNG qua SceneTree headless bị treo ở chờ khung hình và đã dừng; không có ảnh để kiểm thẩm mỹ cửa sổ. Geometry không chứng minh city census, policy/route gameplay, profile B, performance, hay package. Các mục đó vẫn theo roadmap và riêng gate M5.5. Không đổi save schema vì geometry là static versioned content ngoài save.

## Điều kiện đóng

Đóng nhóm khi map chính dùng 27 Atlas ID và cùng view chỉ đọc với Realm; picking/zoom/pan đúng sau thao tác; topology hiển thị không mâu thuẫn 31 Atlas links; không phát sinh người/hàng/tiền/quyền; observer filter giữ kín dữ liệu; core/binding/UI đã được kiểm theo target liên quan. Chưa có session Godot thật thì chỉ báo phần core/view đã kiểm, không ghi nhóm UI hoàn tất.
