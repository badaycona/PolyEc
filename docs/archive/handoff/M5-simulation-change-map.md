> LƯU TRỮ 2026-10-18. Snapshot trước hợp nhất; không thực thi prompt hoặc dùng trạng thái trong file này làm chỉ đạo hiện tại. Đọc [nguồn chuẩn](../../README.md).

# M5 — Bảng thay đổi và bàn giao thiết kế đã duyệt

Ngày: 2026-10-17. Áp dụng [quyết định mô phỏng](../design/M5-simulation-design-decisions.md). Bảng này là backlog có ranh giới, không phải lệnh triển khai toàn milestone. OpenCode viết code chính; Codex duyệt plan, diff và bằng chứng từng lát. Chỉ Windows.

## 1. Trạng thái cần hiểu trước

Tại lúc rà soát có nhiều thay đổi gameplay **local chưa commit**. Module activity local có evaluator demand/delivered/unmet và ưu tiên explicit; chưa nối Realm, sinh lý, hiện diện, settlement hoặc save. Các bằng chứng test local được lưu riêng trong báo cáo tiến độ, không được coi là bằng chứng cho revision docs-only này. Khi tiếp tục, xác nhận source/version thực tế; không checkout/reset để khớp bản ghi cũ.

## 2. Phần giữ, sửa và build mới

Đường dẫn bên dưới là vị trí khảo sát/ownership đề xuất, không khẳng định mọi file đã có trên remote hoặc cho phép sửa tất cả cùng lúc.

| Phần | Hành động | Điểm phải thay đổi và gate |
|---|---|---|
| Domain/UI/training boundary | GIỮ | C++ giữ luật, Godot view chỉ đọc + command, Python gọi cùng core. UI/AI không sửa tiền/state trực tiếp; parity test cùng kết quả. |
| Busy và job | SỬA | Khảo sát core/src/realm.cpp, core/include/polyec/realm.hpp và scheduler native. Busy phải nói rõ đang giữ resource/attention gì, trong interval nào; job không phải cả ngày chỉ một action. Lệnh không liên quan không xóa job; tạo cơ hội quyết định không tự ghi attendance. |
| Activity evaluator local | GIỮ + NỐI | core/include/polyec/activity.hpp, core/src/activity.cpp, core/tests/test_activity.cpp: giữ reject cấu trúc và overcommit hợp lệ. Nối command/save/preferences; thống nhất units. Hiện delivered chỉ là attention-time, không tự biến thành lương/học/care. Hòa ưu tiên phải lộ conflict/cơ hội xử lý, không thành policy ngầm. |
| Thực hiện và tiến độ | BUILD MỚI / ĐỐI CHIẾU | Tách requested plan, actual presence, attention, service, progress và output. Activity tương thích có contract riêng; travel chiếm thời gian và không teleport. Test nhiều hoạt động/ngày, partials, ranh giới và không double-credit. |
| Ngủ, mệt, sức khỏe | BUILD MỚI / ĐỐI CHIẾU | State thuộc từng người; tách sleep pressure/debt, exertion và health. Actual effort/rest mới cập nhật; đặc điểm cá nhân khác nhau. Chốt đơn vị/công thức qua plan, không bịa 8/16h hoặc vô hiệu hóa lựa chọn vì fatigue. |
| Tuổi, bệnh và chết | BUILD MỚI / ĐỐI CHIẾU | Clock và lifecycle chung, cause/provenance, xử lý membership/tài sản/nợ/care khi lifecycle đổi. Tuổi có ảnh hưởng nhưng không tự cấp/cấm quyền giám hộ. Chưa chọn bệnh học hoặc các luật tuổi nghiệp vụ khác. |
| Giám hộ, hộ, care | SỬA + BUILD | Khảo sát guardianship/household và Realm adapters local. Tách nhận trách nhiệm khỏi lượng care thực. Không gate năng lực/tổng tải nhận cam kết; thiếu care có cause và hậu quả riêng, không tự chuyển hộ/chọn người chăm hộ NPC. Consent/quyền API vẫn kiểm tra nguyên tử. |
| Tiền và mục đích chi | HỢP NHẤT CÓ ĐIỀU KIỆN | Khảo sát funding/purpose-payment/loan local. Dùng chung đường chuyển tiền + mục đích/dữ liệu nghiệp vụ; ăn/học/viện phí không cần action riêng chỉ vì khác nhãn. Giữ contract riêng nếu khác người nhận, hàng/dịch vụ, consent hoặc settlement. Không nhập nhằng chi tiền và đã được chữa/ăn/học. |
| Quan hệ và kiến thức | GIỮ + BỔ SUNG | ID stable, quan hệ thưa/typed/directed, membership riêng ownership/care/tình cảm. Nguồn thông tin và ký ức giới hạn observation; đổi hộ không xóa nghĩa vụ khác, không cho NPC biết toàn world. |
| Tính cách và autonomy | BUILD + NỐI | Traits tương đối bền, state/memory cá nhân, policy weights shared. Routine là plan có thể đổi, không lịch viết tay bí mật tối ưu. Test lựa chọn theo context/traits và thay đổi vì hậu quả; sampling có seed, không random vô nghĩa. |
| Binding/action/observation/reward | SỬA THEO LÁT | bindings/python/realm_binding.cpp, ai/tests, ai/training, native/ và gdext/: đổi version/shape khi semantics đổi; giữ guard artifact cũ. Không gọi RL M5 hoàn thành từ baseline hoặc từ evaluator. Reward theo outcome, elapsed-time nhất quán. |
| Save/replay và performance | BỔ SUNG THEO LÁT | Save nghĩa vụ/lịch/progress/body/RNG đúng ownership, load không resolve lại giao dịch hoặc đổi may rủi. Event/lazy update không bỏ lỡ aging/deadline. Test thứ tự duyệt, roundoff, replay; benchmark Windows trước tuyên bố quy mô. |

## 3. Thứ tự giao việc, không giảm scope

1. OpenCode khảo sát giới hạn phần timeline/clock/command/save và nộp **plan-only**: file/class/hàm, units/ownership/invariant, success/reject/accepted-failed, version và tests. Codex review trước triển khai; không giao cả bảng trong một prompt.
2. Nối một người có nhiều phiên trong ngày và ưu tiên do actor cấp; giữ job khi có command không liên quan. Kiểm chứng horizon splitting, pause và save/reload. Chưa tự thêm health/ledger effects.
3. Một lát thực hiện có attendance/resource/progress thật, kết quả thiếu và settlement tương ứng. Mở rộng các cặp compatible có nhu cầu thực, không biến toàn bộ hoạt động thành exclusive vĩnh viễn.
4. Nối exertion/ngủ/phục hồi tối thiểu với cùng lát sau khi duyệt công thức; không gate nhận việc theo sức bền. Sau đó nối care và hậu quả thiếu dịch vụ, không quay lại care-admission gate đã bị thay thế.
5. Nối view/observation/reward và đánh giá baseline có nhãn qua cùng core. Tiếp tục lifecycle/quan hệ/personality và các hệ thống còn lại theo backlog M5; RL production/training và profile vẫn là gate riêng có ngân sách.

## 4. Tiêu chí nghiệm thu bắt buộc

- Một ngày có nhiều hoạt động; được nhận lịch không thể làm hết. Demand không bị giấu, delivered không vượt tài nguyên thực, shortfall có nguyên nhân và không sinh thêm giờ/tiền/hàng.
- Phân biệt reject API nguyên tử với nỗ lực hợp lệ thất bại. Audit từng mask/candidate/validator/scheduler/UI gate: bảo vệ cấu trúc/quyền/bảo toàn, hay đang chọn hộ NPC? Loại sau không được hard-gate.
- Kiểm tra phân đoạn thời gian, qua ngày, pause, save/replay, reorder, tie/unresolved và lặp command/settlement. Không để actor ID vô tình quyết định ai thắng; đặc tả phân bổ cạnh tranh riêng.
- Cặp exclusive và compatible đều có test. Nghỉ chỉ hiệu quả khi thật sự nghỉ; output không suy từ nhãn hoạt động hay attention credit đơn thuần.
- Windows tests liên quan rồi regression, binding/parity và native khi bị ảnh hưởng. Báo lệnh/exit/count/source thực; không lấy test của working tree khác làm bằng chứng cho commit docs.
- Trước mỗi lát cập nhật tác động save/binding/action/observation/reward/model. Plan không thay test; không train dài hoặc chi tiền chỉ để vượt gate.

## 5. Docs cần theo khi triển khai

[Implementation plan](../design/implementation-plan.md), [M5 draft](../design/M5-draft.md) và [simulation scope](../design/simulation-complexity-scope.md) đọc cùng quyết định đã duyệt. Các kế hoạch lịch sử giữ nguyên provenance; phần mâu thuẫn bị thay thế, không resume từ nhãn APPROVED cũ. Mỗi cơ chế mới phải có ba lớp đồng nghĩa: **đời sống → state/process/units/ownership → ví dụ/test/trạng thái**. Khi code còn thiếu ghi rõ thiếu, không đổi lời mô tả thành đã hoàn thành.
