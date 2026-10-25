# Kiểm class/action trial-a2 — 2026-10-25

## Kết quả

Đã tách class thành các file ownership và triển khai30 verb có handler thật cho một lãnh thổ; danh sách/điều kiện tại CLASS_ACTION_MAP.md. model.hpp chỉ còn umbrella. ActionDefinition và handler cùng registry; Godot đọc catalog, không có danh sách engine khác hoặc luật kinh tế GDScript. Không tạo stub cho action/vùng chưa triển khai.

Cơ chế mới gồm: chuyển tiền/hàng/kho có quyền, chủ tự sản xuất, lương tùy điều khoản, thu hồi chào việc; sale giữ hàng và counter đổi lượt consent; loan escrow/phát tiền/nợ/trả một phần/quá hạn; lease trả trước/quyền nghỉ/hết hạn vẫn ở chỗ cũ; delivery nhận lời chưa nhận thưởng, phải giao hàng thật; hủy/quá hạn hoàn escrow và ghi nguyên nhân; tool12 mẻ/hao mòn/thay bằng hàng thật; lấy nước vào túi/uống/chăm sóc; decline/withdraw invitation, bổ nhiệm có accept, bãi nhiệm và công báo. Người giữ chức mới dùng tài khoản công, không được coi tiền đó là ví mình.

Save2 ghi activity.started, tool capacity, term author/awaiting, reserved source, proposal escrow/status/deadline, debt và lease liên kết. Validator kiểm tham số, nguồn chủ, consent turn, cặp talk, trạng thái nợ/thuê, tiền556 và hàng tồn+reserve+consumed so với nguồn+sản xuất (tool đầu kỳ4 gồm2 lắp sẵn). Save1 bị từ chối rõ; app dùng tên file a2 riêng. Snapshot a1/bằng chứng cũ vẫn có nhãn phiên trước.

## Đã thực thi

- MSVC build và C++ tests:191 checks regression,271 checks action/domain pass. Tests mới thực hiện27 verb;3 verb còn lại được phủ trong regression. Có success/reject/edge, reject nguyên tử so toàn serialized state, counter hai phía, public/private offers, hoàn escrow khi reject/withdraw/expire, giao hàng thiếu hàng, debt partial/overdue/no-auto-debit, lease expiry/occupancy, tool wear/replacement, tự sản xuất không fake wage, collect cancel, care có tài nguyên, appointment/permission, malformed contract linkage và save/replay. Đây là các ca kiểm cụ thể, không là chứng minh mọi tổ hợp.
- `scripts/verify-root.cmd`: configure từ C:\prc\PolyEc bằng root CMake mới, Ninja Release, build đúng4 targets core_tests/action_tests/polyec_core/PolyEc; CTest2/2 pass. Log evidence/root-build-a2.log và core-a2.log (có stdout191/271). Gói dùng binary từ build này và CRT static.
- `tests/bridge_smoke.py`:30 lượt CLI gói; Unicode file/mục tiêu, không cho player điều khiển NPC, viewer sai không ghi state, save/load, named params thật, duplicate/unknown params reject. evidence/bridge-a2.json. Có đo độ trễ process/view nhỏ làm tham khảo, không benchmark quốc gia.
- Godot headless: vòng cũ24 bước + roundtrip/player/focus/sleep checks pass; chuỗi mở rộng37 records đều ok qua các control UI (dropdown/amount/mode/named fields và tín hiệu nút Gửi lệnh), gồm sale-counter, loan-repay, delivery, appointment-grant-dismiss, lease-expiry, self-production-equip, collect, invitation và announce. Catalog core30/UI30 khớp, save/load giữ đúng toàn projection. evidence/ui-a2-final.log, ui-actions-a2-final.log và JSON tương ứng. Test trả nonzero nếu record thất bại.
- Cửa sổ native renderer OpenGL đã mở từ C:\prc\PolyEc\rebuild; bấm dropdown kiểm30 hành động và form đề nghị (loại hợp đồng/giá/hạn/nguồn). Script UI không phải learned policy. Vòng cũ đã được bấm chuột trực tiếp ở a1; không tuyên bố mọi nhánh của30 lệnh đã được bấm bằng chuột. Các nhánh mới được kiểm bằng cùng control gửi lệnh và core tests.

## Chuyển nơi làm việc và Git

Theo chỉ đạo mới từ người dùng, source đang hoạt động chuyển về C:\prc\PolyEc\rebuild trong checkout cũ. Đã copy/đối chiếu hash39 source/docs ban đầu rồi chỉ sửa tại đó. Root AGENTS/README/CMake/WORK_STATUS trỏ rewrite, root CMake không include legacy; root entrypoints trước chuyển giữ tại work/migration-previous. Documents trở thành snapshot, chỉ nhận outputs deliverable.

Origin giữ https://github.com/badaycona/PolyEc.git; nhánh codex/m5-private-housing-handoff và lịch sử không đổi. Chưa commit/push/force-push. Legacy source và dirty work cũ vẫn còn trên đĩa: thao tác xóa trước bị chặn; không thử xóa/move recursive hoặc đổi tool để né chặn. Do đó việc thay toàn bộ file legacy trên đĩa chưa hoàn thành; không dùng chúng cho build/test mới, không động worktree ngoài. CI GitHub cũ chưa chuyển và chưa được chạy; chưa có push để kích hoạt nó.

## Giới hạn

Hoàn thành catalog30 lệnh đã đặc tả cho slice A2, chưa phải toàn roadmap. Chưa liên vùng/chiến tranh/cưỡng chế/pháp luật chuyên sâu, chưa RL/binding Python. Tendency chưa được policy dùng; script chỉ kiểm luật. Lease là quyền nghỉ tại site trừu tượng, chưa mô phỏng phòng/eviction. Loan lãi cố định đồng thuận, không compound interest hay auto-enforcement. Sale chốt cả lô, không market maker/giá động. Chăm sóc chỉ cơ bản bằng food/water/rest, không mô phỏng bệnh/thuốc. History giới hạn2000 contracts/debts/leases và20000 events; bridge một writer, serialize/process mỗi lệnh nên chưa phù hợp quy mô lớn. Chưa chứng minh mọi điều kiện đúng chỉ bằng số checks. Tham số trial cần chơi/hiệu chỉnh.

Người dùng dừng computer-use bằng Escape sau khi kiểm dropdown/form; đã dừng mọi thao tác chuột/bàn phím tiếp theo. Chuỗi native renderer đã được khởi chạy trước thời điểm dừng; log/ảnh của nó được lưu riêng, không thay bằng chứng thao tác chuột.
