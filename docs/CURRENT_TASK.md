# CURRENT_TASK — điểm dừng và tình trạng hiện tại

Cập nhật: 2026-10-23. Người dùng yêu cầu dừng phát triển tại điểm này, ghi lại tài liệu, khôi phục nội dung tài liệu bị thiếu từ worktree cũ, commit và push phần project phù hợp; sau khi xác minh push thì dừng. Không tiếp tục code gameplay, không khởi động RL, không đặt lịch làm việc. Đây là điểm bàn giao, không phải tuyên bố M5 hay bản game đã hoàn tất.

## Baseline và bức tranh hoàn thiện

- Core refactor R1–R4 và chuỗi command → transaction → ngày game → save/load có báo cáo tích hợp: [báo cáo core refactor](reports/M5-core-refactor-R1-R4-integration-evidence-2026-10-22.md). Báo cáo không chứng nhận toàn bộ feature M5.
- Save hiện hành là `POLYEC_REALM_60`; model còn giữ `realm-v31`, state382, actions282, observations3766, household contract v33, timeline contract v35 và reward `realm-v14-person-enterprise-equity-atlas-products-v1`. Lát trận đánh không thay policy inputs/actions/model/reward.
- Thế giới đã có địa đồ Atlas27, hộ gia đình, quan hệ gia đình, thừa kế, sức khỏe, lao động, sản xuất, thị trường, vận tải, ngoại thương, cứu trợ và một số hệ thống quản trị/quân vụ. Mức hoàn thiện khác nhau theo từng chuỗi; bảng mô tả phần đã có và còn thiếu nằm trong [audit mức hoàn thiện thế giới](reports/M5-world-completeness-audit-and-next-work-2026-10-23.md).
- Bản game chưa đầy đủ. Còn thiếu nhiều người tiêu thụ/nguồn hàng và hiệu chuẩn kinh tế; phản ứng ngoại giao tự chủ; nhiều cơ chế xã hội và lãnh địa; hoàn thiện giao diện cho các chuỗi gameplay; kiểm tra chơi xuyên suốt, hiệu năng quy mô mục tiêu và gói Windows offline. Chiến đấu hiện mới là phòng thủ một cảng trước lực lượng đột kích tổng hợp, không phải chiến tranh lãnh thổ.

| Phần thế giới | Đã có | Còn thiếu |
|---|---|---|
| Địa đồ | Atlas27 có 27 vùng polygon và 31 liên kết; bản đồ cho chọn hồ sơ, pan/zoom. Hình học là setting giả tưởng do PolyEc thiết kế. | Chưa hiệu chuẩn địa hình, khoảng cách, hướng, passability và điều kiện route đầy đủ; đô thị chưa là địa điểm/census độc lập. Liên kết trên bản đồ tự nó không mở tuyến vận tải. |
| Người và xã hội | Person có ID/vòng đời; có hộ, kết hôn/parentage, thừa kế, sức khỏe, học/làm, một số quyền và thông tin theo phạm vi. | Chưa có đầy đủ mọi quan hệ/tổ chức; guild, giáo hội và cơ quan đô thị chưa có cơ chế xã hội hoàn chỉnh. |
| Hàng và sản xuất | Catalog hiện 30 GoodId/16 nhóm; nhiều hàng có producer, recipe, nguồn mùa vụ hoặc nguồn hữu hạn. | Không phải mọi hàng đều có producer, consumer, tồn đầu kỳ, mùa vụ, giá và cầu hợp lý. Nguồn thử nghiệm mới chưa được hiệu chuẩn. |
| Kinh tế/vận tải | Có các chuỗi thị trường, lệnh, giá, route/capacity, shipment, ngoại thương và cứu trợ với kiểm riêng. | Một số giá/tuyến vẫn là giả định thử; partner chưa có phản ứng ngoại giao tự chủ; cần chơi live qua model hiện hành. |
| Quân sự | Có volunteer, payroll/escrow, consent, tập kết, khẩu phần, nghĩa vụ tiếp tế và core phòng thủ đột kích Nam Cảng schema60. | Chưa có lãnh địa/quân số đối địch thực, chiến tranh chiếm đất, tù binh, đình chiến hay nhiều mặt trận. Adapter/UI cho trận mới chưa được xác nhận. |
| Ứng dụng và phát hành | Godot có bản đồ/panel và nhiều core API đã chạy qua C++/Python/native. | Chưa có full feature-flow UI được kiểm trong phiên game hoàn chỉnh với policy phù hợp; còn kiểm live, benchmark/stress, và gói Windows offline. |
| RL | Có trainer/adapter và run smoke lịch sử kèm biểu đồ/dữ liệu nguồn. | Chưa có policy được chọn và qua validation/holdout; RL đang dừng theo chỉ đạo. |


## Nhóm gần nhất: `coastal-raid-battle-v1` — đã viết core, chưa đóng nhóm

Luật v1 tạo một đột kích tại Nam Cảng (vùng 8) bởi một trong ba actor ngoại thương A/B/C. Lực lượng bên ngoài là quân số tổng hợp chứ không sinh Person giả. Một vụ chưa trả lời có thể tồn tại tại một thời điểm. Tạo vụ cần quyết định actor mới, corridor biển mở, mỗi đơn vị có 25 đồng và 1 DryFood trong sổ partner; giới hạn 1–8 đơn vị. Tiền được chuyển vào carrier, thức ăn được ghi đã tiêu thụ, vì vậy không tạo tài sản miễn phí.

Sovereign có thể đưa 1–16 volunteer đủ điều kiện tại Nam Cảng vào phòng thủ hoặc rút lui. Core tự kiểm lại người còn sống, sức khỏe, consent, kỳ phục vụ, thời hạn huy động và vị trí hiện tại; UI không thể gọi người chưa đồng ý. Sức phòng thủ của mỗi người là `1 + floor(MilitaryCapability / 50) + min(1, WeaponInventory)` (1–4 điểm); mỗi đơn vị đột kích có 1 điểm. Phòng thủ thắng khi tổng điểm lớn hơn lực đột kích; hòa được tính là đột kích thành công. Khi thất bại, casualties được đặt health=0 và đi qua xử lý tử vong/thừa kế của ngày game kế tiếp. Khi đột kích thắng hoặc Sovereign rút, đối thủ lấy tối đa một Salt mỗi đơn vị còn sống, bị giới hạn bởi hàng Salt thật ở cảng và sức chứa partner; không tạo hàng. Không đổi quyền chủ quyền đất, không có tù binh, chiếm đất hay đình chiến trong v1.

Save đổi từ schema59 sang schema60 và thêm phần `MILITARY_RAID_1`. Save cũ được đọc với danh sách trận rỗng; không đổi kích thước model, dữ liệu quan sát/hành động, reward hay luật consent. Chi tiết quyết định và source: [plan chiến đấu](design/M5.4-volunteer-battle-resolution-plan.md), [báo cáo kiểm chứng và điểm dừng](reports/M5.4-volunteer-battle-resolution-evidence-2026-10-23.md).

### Kiểm tra thực tế và giới hạn

- CTest `military_battle_tests` pass 1/1 **trước** lần sửa cuối vào query dùng chung; nó kiểm tra giới hạn lực lượng, chỉ một trận chờ, trừ đúng tiền/thức ăn, roundtrip save trận chờ, migration save59, tuyến biển đóng, quyền actor, người phòng thủ trùng, consent đã rút, casualty qua ngày game, phòng thủ thắng và chuyển Salt khi rút lui.
- CTest ngoại thương + Atlas specialty pass 2/2 khi cập nhật fixture schema60.
- Release `polyec_env` cùng core, native runtime, Python binding và query trận đánh build thành công sau lần sửa query cuối.
- Python smoke trận đánh **không pass**: dừng tại assertion test dùng `Realm.route(...)` legacy cho bản đồ Atlas 27 vùng, trong khi helper đó chỉ hỗ trợ tuyến bản đồ nhỏ. Đây là lỗi cách gọi của smoke test; cần đổi bài kiểm tra sang lệnh tra tuyến theo index công khai rồi chạy lại. Không coi smoke là pass và cũng chưa kết luận API sản phẩm hỏng.
- Chưa build lại GDExtension Windows sau thay đổi chiến đấu cuối; chưa chạy Godot editor scan/UI thật, thao tác trận đánh trong game, Python parity trọn luồng, package, benchmark hoặc RL. Vì vậy adapter/UI và trải nghiệm người chơi của trận đánh chưa được xác nhận.
- Full Realm runner vẫn dừng sau 434 checks ở checksum fixture save v14 lịch sử; không thay checksum để ép pass. Readiness runner 16/16 và CTest 1/1 là kết quả lịch sử của lát trước, không thay thế việc kiểm save60 hiện tại.

## RL

RL đang tạm dừng theo yêu cầu: không train, chọn candidate, validation, holdout hoặc gắn policy. Civilian run seed20261101 chỉ hoàn tất 25/25 update, 25.600 quyết định trên 1/3 seed; manifest `complete=false`, chưa có policy được tuyển chọn và đánh giá độc lập đạt yêu cầu. Giữ nguyên run tại `artifacts/m5-v15-r1-candidates-2026-10-23-retry2/civilian/seed-20261101/`. Biểu đồ và số liệu của run vẫn là evidence của đúng phiên bản đó, không phải đánh giá chất lượng policy hoàn chỉnh. Khi được tiếp tục RL, báo cáo phải giải thích đầy đủ việc tạo world và transition bằng C++ core, seed và actor, dữ liệu trong RAM/file, cách tách train/validation/holdout, chọn checkpoint và biểu đồ của đúng run.

## Khôi phục tài liệu và worktree

- Worktree `C:\Users\Hi\.local\share\kilo\worktree-pool\polyec-e5c68ef74532\tide-double` ở commit `d5751bbfc4ff801950c45118069f589318a86382` là tổ tiên của `main`; thay đổi của nó đã có trong lịch sử hiện tại.
- Worktree tài liệu `C:\Users\Hi\.codex\worktrees\docs-reorganization\PolyEc` đang ở nhánh `codex/docs-reorganization`, cùng commit gốc hiện tại nhưng còn thay đổi chưa commit. Không sửa, di chuyển, xóa hoặc archive worktree này.
- Đã sao chép bốn tài liệu còn thiếu vào checkout chính: `docs/IMPLEMENTED_BRANCH_CATALOG.md`, `docs/MILESTONE_BLUEPRINTS.md`, `docs/PRODUCTION_BLUEPRINT.md`, `docs/PROJECT_MAP.md`; README chính đã thêm liên kết tới bản đồ tài liệu. 372/380 file untracked bên worktree tài liệu trùng nội dung ở checkout hiện tại. Bốn file cùng đường dẫn nhưng khác nội dung được giữ bản checkout chính vì có các cập nhật M5 mới hơn. Chi tiết nguồn gốc và danh sách hash file nhị phân không đưa vào Git nằm tại [manifest file ngoài Git](handoff/non-git-artifacts-manifest-2026-10-23.csv).
- Checkpoint, ONNX, ZIP source, object sinh ra và ghi chú cá nhân `thoughts/`, `findings.txt` được giữ trên đĩa nhưng không đưa vào Git. Thư mục build/cache cũng không đưa vào Git. Manifest CSV có đường dẫn, dung lượng, SHA-256 và lý do loại trừ; không có lệnh xóa.

## Bước tiếp theo khi người dùng yêu cầu tiếp tục

1. Sửa riêng Python battle smoke để dùng route-index query/lệnh đúng cho Atlas27; chạy lại smoke và CTest chiến đấu sau query cuối.
2. Build GDExtension bị ảnh hưởng, chạy Godot editor scan và UI/native smoke liên quan; sau đó thử chuỗi raid → Sovereign response → ngày game → save/load với UI thật nếu có profile chạy được.
3. Chỉ sau khi nhóm chiến đấu đạt các kiểm tra đó mới đóng nhóm; tiếp tục nhóm thiếu kế tiếp trong roadmap M5. RL vẫn chờ tới khi chức năng gameplay đầy đủ và gate tích hợp/hiệu năng/package đã đạt.

Lịch sử và báo cáo chi tiết: [nhật ký phát triển](DEVELOPMENT_HISTORY.md), [báo cáo điểm dừng](reports/M5-checkpoint-handoff-2026-10-23.md). Không gọi M5, game hoàn chỉnh, battle UI hoặc learned RL là hoàn tất tại điểm dừng này.
