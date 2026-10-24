# Điểm dừng và bàn giao M5 — 2026-10-23

## Phạm vi điểm dừng

Người dùng yêu cầu dừng phát triển tại thời điểm hiện tại, ghi lại tài liệu, khôi phục nội dung từ worktree tài liệu cũ, commit và push phần project phù hợp, rồi dừng. Báo cáo này cố định đúng những gì đã có trên checkout khi dừng. Nó không thay bằng chứng cho các thay đổi code và không tuyên bố M5/game hoàn chỉnh.

## Gameplay đã có ở lát cuối

`coastal-raid-battle-v1` là một bản thử nghiệm phòng thủ cảng có luật xác định. Ba partner A/B/C được phép gửi command tạo một lực lượng đột kích tổng hợp tại Nam Cảng (vùng8) trên Atlas27. Không tạo Person đại diện cho quân địch. Force có 1–8 đơn vị; partner trả 25 đồng và 1 DryFood cho mỗi đơn vị, corridor đường biển phải đang mở, sequence quyết định phải mới và không được có vụ khác đang đợi. Tiền chuyển sang carrier; DryFood ghi vào `consumed_`.

Chỉ Sovereign sống mới giải quyết raid chờ. Có thể rút lui mà không ép volunteer ra trận hoặc chọn 1–16 volunteer. Core không tin danh sách giao diện: nó tự tra consent còn bật, kỳ phục vụ đang hiệu lực, thời hạn mobilize còn hiệu lực, sức khỏe và vùng hiện tại. Mỗi volunteer có defense score `1 + floor(MilitaryCapability / 50) + min(1, WeaponInventory)`; force unit có 1 attack point. Tổng phòng thủ phải lớn hơn force còn lại mới đẩy lui được raid, nên hòa thuộc về phía đột kích. Khi phòng thủ thua, tối đa `ceil(force_units / 2)` người được chọn chịu casualty, giới hạn ở số defender; họ chuyển sang health0 và luồng ngày game xử lý tử vong/estate. Khi quân đột kích sống sót, họ lấy tối đa1 Salt mỗi người còn sống, bị chặn bởi Salt thực có ở vùng8 và sức chứa partner. Hàng chỉ chuyển sổ, không sinh mới. V1 không đổi quyền sở hữu vùng, không có tù binh, chiếm đất, lãnh địa đối phương, nhiều mặt trận hoặc đình chiến.

Mỗi trận lưu ID, partner, vùng, ngày tạo/giải quyết, quân số, điểm phòng thủ, casualty, Salt bị lấy, danh sách defender và kết quả. Save schema tăng59→60, thêm `MILITARY_RAID_1`; save cũ tải với lịch sử raid rỗng. Các invariant và decoder từ chối record sai thứ tự, ID, vùng, giới hạn, casualty/outcome hoặc defender. Model vẫn realm-v31, state382, actions282, observations3766, household v33, timeline v35, reward không đổi.

## Kiểm đã chạy và điều chúng chứng minh

| Lệnh/phạm vi | Kết quả quan sát | Kết luận được phép |
|---|---|---|
| Build + CTest `military_battle_tests` | 1/1 pass, chạy trước lần chỉnh cuối query dùng chung | Những hành vi trong test core về tạo raid, reject, consent, trận, hàng hóa và roundtrip đã pass tại revision chạy đó. Cần chạy lại sau query cuối trước khi đóng nhóm. |
| CTest ngoại thương và Atlas specialty | 2/2 pass sau cập nhật fixture schema60 | Các nhóm phụ thuộc gần đó tiếp tục qua kiểm tra được chọn. Không phải full suite. |
| Release `polyec_env` và source core/native/Python binding | build thành công sau query cuối | Core và Python adapter biên dịch với schema/query hiện tại. Đây không phải smoke hành vi hoàn chỉnh. |
| Python `military_battle_binding_checks.py` | dừng tại `assert closed.route(1, 9, 8, False)` | Smoke không pass. Test gọi helper route legacy chỉ xử lý bản đồ tối đa6 vùng, còn Atlas có27 vùng. Cần đổi test sang API/lệnh route index phù hợp và chạy lại; không được ghi kết quả này thành pass. |
| GDExtension, Godot UI, native runtime trong app | chưa build/kiểm lại sau thay đổi cuối | Chưa xác nhận adapter hoặc panel trận đánh hoạt động trong game. |
| Full Realm runner | build được, dừng sau434 checks ở checksum historical v14 | Chưa pass toàn runner. Không sửa expected hash lịch sử; chưa xác định đầy đủ tại sao dữ liệu workspace lệch checksum cũ. |
| Readiness tests từ lát trước | runner16/16 và CTest1/1; các adapter từng build khi chỉ có roster | Chứng minh roster consent/sẵn sàng trước đó, không chứng minh battle resolution/save60 tích hợp đầy đủ. |

Không chạy toàn bộ CTest, RL, package, benchmark hoặc phiên chơi live ở điểm dừng này. Giới hạn là chủ ý theo phạm vi; chưa phải kết quả pass.

## RL và nội dung chưa hoàn chỉnh

RL không được chạy tiếp tại điểm dừng. Civilian seed20261101 đã có25/25 update và25.600 quyết định trong1/3 seed, nhưng manifest ghi `complete=false`. Chưa có bộ checkpoint được tuyển chọn, validation và holdout độc lập đạt tiêu chí. Không ghi đè artifacts hiện có. Số liệu/biểu đồ chỉ thuộc run đó; rollout return không thay đánh giá holdout.

Toàn thế giới chưa hoàn chỉnh. Các phần còn thiếu bao gồm nguồn và người tiêu thụ cho một số hàng hóa; hiệu chuẩn giá, sản lượng, mùa vụ và nguồn hữu hạn; cơ chế xã hội/tổ chức/cơ quan đô thị; ngoại giao do partner tự quyết; đối thủ và quyền lực lãnh thổ thực; chiến tranh ngoài một vụ đột kích giả lập; giao diện hoàn chỉnh và kiểm thử chơi thật cho các chuỗi; hiệu năng ở quy mô mục tiêu; package Windows offline. Chi tiết hiện trạng từng nhóm đã được ghi ở `docs/CURRENT_TASK.md` và audit thế giới được liên kết tại đó.

Khi mở lại RL, phải ghi rõ: cách world/transition được tạo qua cùng C++ core, seed, actor/peer, dữ liệu được giữ trong RAM và file nào được xuất, train/validation/holdout được tách ra sao, tiêu chí chọn checkpoint, và biểu đồ reward theo update/quyết định cùng đánh giá độc lập trên world seed ghép cặp với baseline/policy ban đầu. Không ghép train với holdout.

## Khôi phục worktree và xuất xứ

1. `C:\Users\Hi\.local\share\kilo\worktree-pool\polyec-e5c68ef74532\tide-double`, detached `d5751bbfc4ff801950c45118069f589318a86382`, là tổ tiên của `main` hiện tại; phần commit cũ đã nằm trong lịch sử `main`.
2. `C:\Users\Hi\.codex\worktrees\docs-reorganization\PolyEc`, branch `codex/docs-reorganization`, cùng commit gốc `dff2781a41ae52a4bd476b4b1460ad0f09ef5dc3`, chứa tài liệu chưa được tích hợp. Worktree này được giữ nguyên.
3. Từ worktree tài liệu, bốn file bị thiếu được sao chép: `docs/IMPLEMENTED_BRANCH_CATALOG.md`, `docs/MILESTONE_BLUEPRINTS.md`, `docs/PRODUCTION_BLUEPRINT.md`, `docs/PROJECT_MAP.md`. README được thêm liên kết tới chúng. 372/380 file untracked đã trùng nội dung ở checkout chính; trong bốn file cùng đường dẫn có nội dung khác, giữ bản hiện có trong checkout chính vì bản đó có lịch sử M5 mới hơn. Không ghi đè những thay đổi tracked mới ở checkout chính.
4. Không xóa, đổi tên, di chuyển hoặc archive worktree cũ. Checkpoint `.pt`, ONNX, source ZIP, object và ghi chú cá nhân vẫn giữ nguyên trong workspace. [CSV manifest](handoff/non-git-artifacts-manifest-2026-10-23.csv) ghi path, byte size, SHA-256 và lý do loại khỏi commit. `build/` là thư mục sinh và cache, được Git ignore. `thoughts/` và `findings.txt` là ghi chú cục bộ, không đưa vào repository.

## Hướng tiếp tục sau này

Khi người dùng yêu cầu tiếp tục, trước hết sửa test route Atlas để dùng route index API, chạy lại test Python và CTest battle sau query cuối. Tiếp theo build GDExtension liên quan, Godot editor scan và smoke UI/native; kiểm chuỗi command tạo raid → Sovereign phòng thủ/rút lui → ngày game xử lý casualty → save/load. Chỉ khi đã qua các bước này mới đóng nhóm battle và đi tới nhóm M5 thiếu kế tiếp. Không bắt đầu RL cho tới khi gameplay, tích hợp, policy, hiệu năng và package đạt các tiêu chí roadmap.

Việc commit/push được thực hiện theo yêu cầu hiện tại của người dùng và chỉ sau khi rà soát staged files. Không force push. Nếu remote từ chối push, báo rõ lỗi và giữ commit local; không tìm cách vượt qua bảo vệ branch.


Mã commit nội dung tại điểm dừng: `918323d008c7eee7ed82c787f162a839e936d096` trên nhánh `main`; nhánh đích là `origin/main`. Tài liệu này được cập nhật ngay sau commit để giữ lại mã nhận dạng; lần push được xác minh sau commit tài liệu.

## Đính chính sau khi người dùng yêu cầu tiếp tục phát triển — 2026-10-23

Bản handoff ở phần trên lưu trạng thái tại thời điểm dừng trước đó; yêu cầu mới hơn của người dùng là tiếp tục code cho tới khi hoàn thiện gameplay trước RL. Do đó checkpoint commit không phải trạng thái kết thúc nhiệm vụ.

Sau khi sửa Python smoke dùng đúng `transport_route_command` cho Atlas27, đã build `polyec_env` Release và smoke pass với schema realm-v31; CTest `military_battle_tests` build lại và pass1/1 sau query candidate cuối. GDExtension Windows Release `polyec` build pass. Godot4.5.1 headless editor scan pass; UI formatter smoke 41 checks (thêm các kết quả chờ, đẩy lui, vượt phòng thủ và rút lui); native smoke19/19; app UI smoke51/51, gồm kiểm tra hai command battle được đăng ký qua GDExtension. Các kết quả này kiểm API, formatter và app shell, chưa phải một trận trong Realm đang chạy với model phù hợp. Chi tiết xuất lệnh/target và giới hạn sẽ nằm trong mục cập nhật cùng nhóm bên dưới; các kết quả của checkpoint cũ không bị sửa ngược.
