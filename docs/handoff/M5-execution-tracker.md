# M5 — bảng nghiệm thu triển khai (bản lịch sử)

Tài liệu này giữ checklist và kế hoạch bàn giao cũ để tra cứu; các trạng thái/bước làm bên dưới không phải chỉ dẫn hiện hành. Hãy dùng [AGENTS.md](../../AGENTS.md) cho cách phối hợp và [CURRENT_TASK](../CURRENT_TASK.md) cho nhóm đang làm, bằng chứng gần nhất và bước kế tiếp. Những quy định cũ về OpenCode hoặc T1 bên dưới đã hết hiệu lực.

Yêu cầu ngày 2026-10-18 về OpenCode bên dưới chỉ là bối cảnh lịch sử của checklist này; không còn điều khiển cách thực hiện hiện tại. Các kết quả và phạm vi đã xác nhận vẫn được giữ làm lịch sử.

Nguồn scope: [M5 đã duyệt](../design/M5-draft.md), [quyết định 17/10](../design/M5-simulation-design-decisions.md), [change map](M5-simulation-change-map.md). Windows app/package ưu tiên; Linux app/package hoãn theo chỉ đạo, core/Python portability vẫn là gate. M6 UX và M7 production training không được đánh tráo vào trạng thái M5.

## Trạng thái đã nghiệm thu mới nhất — 2026-10-21

Bảng cũ bên dưới là lịch sử ban đầu và không còn phản ánh trạng thái hiện tại. R3 day pipeline, T2.1–T2.4 (gồm migration lương xưởng/dev smoke), nhiều lát M5.0/1, M5.2/3, core codec, gameplay configuration và learned-policy plumbing đã được code hoặc nghiệm thu theo báo cáo liên kết trong [CURRENT_TASK](../CURRENT_TASK.md). Chưa hoàn tất toàn bộ tích hợp/runtime/UI người chơi, learned policy đạt chất lượng, đủ M5.0–M5.5, profile B hoặc package Windows. Ưu tiên CURRENT_TASK và report của revision hiện hành khi tiếp tục.

## Bằng chứng đã có

- Core repair b8160f8: đã push, CTest Windows 6/6.
- Membership hộ thưa/guardian: lát đầu đã nghiệm thu, chưa có đủ hộ động/tài khoản/thừa kế.
- Activity evaluator thuần: đã review, CTest Windows 7/7; chưa nối Realm/clock/save. Xem [report](../reports/M5-activity-evaluator-2026-10-18.md). Working tree chưa commit.
- T1a Realm schedule timeline: implemented in working tree, CTest 8/8 passed in `build/m5-t1a` (core, trade, realm, court, campaign, manager, activity, schedule). Native/Python/schema parity and T1b clock adapter remain open; T1 not complete. Plan: [M5-T1a](../design/M5-T1a-realm-schedule-plan.md).

## Gate còn mở — chỉ đánh dấu sau bằng chứng thực

| Gate | Nội dung phải đạt | Trạng thái |
|---|---|---|
| T1 | Timeline nối command, world time, save/replay; nhiều phiên/ngày, overcommit, giữ job khi lệnh không liên quan, pause, model guard | T1a–T1b2d đã nghiệm thu Windows; core R1–R3 có evidence riêng. Linux parity được hoãn theo quyết định người dùng |
| T2 | Attendance/presence/progress/settlement thật; compatible và exclusive; không double credit, shortage có cause | T2.1–T2.4 đã có evidence; Gate 6b payroll/migration và dev smoke Godot pass, nhưng chưa là UI gameplay hoàn chỉnh hoặc learned policy |
| T3 | Ngủ/exertion/phục hồi, care, lifecycle, traits/knowledge; công thức và đơn vị được review, không admission gate quá tải | Một số lát body state, sleep debt, knowledge/relations đã triển khai; tích hợp đầy đủ T3 còn mở |
| M5.0/1 | Registry/schema/quyền; hộ động/quỹ/đất/quyền/địa vị; giám hộ và ownership, save/load, bảo toàn | Còn mở |
| M5.2 | Cơ sở/hợp đồng/học nghề/16 hàng/vận tải/thị trường; fixture A/B và shock vùng | Còn mở |
| M5.3 | Tổ chức/chức, consent hôn nhân/escrow, di sản/nợ/claim, giáo hội; fixture C/D/E và privacy | Còn mở |
| M5.4 | 24 chư hầu + 3 khu triều đình, thành thị/quân sự/ngoại thương và UI vua; 3 causal chain có can thiệp | Còn mở |
| M5.5 | Profile B 100k người, 512–2048 vai phức tạp, stress/long-run và báo cáo CPU/RAM/p50/p95/p99/save/FPS/input; model tương thích cho đo end-to-end | Chưa chạy |
| Portable/shared core | Python cùng core, binding/parity/schema guards và kiểm thử portable; native Windows package/run | Chưa chạy cho M5 |

## Quy tắc tiếp tục

Mỗi lượt giao một mục nhỏ có file/gate. Plan phải nêu state gốc/index, ownership, success/reject/accepted-failed, đơn vị, save/binding/action/observation/reward/model và lệnh thực. Codex review trước code; review diff/kết quả trước lát sau. Ghi kết quả/link vào bảng khi có, giữ các gate khác mở. Không giao toàn M5 trong một prompt triển khai.

Bước hiện tại không được theo dõi trong bản checklist lịch sử này. Tại lần đối chiếu 2026-10-22, [CURRENT_TASK](../CURRENT_TASK.md) ghi nhóm T3.5 — hiển thị nợ ngủ và gắng sức trong hồ sơ người chơi. M5 vẫn chưa hoàn thành; hãy mở CURRENT_TASK để xem trạng thái mới nhất trước khi làm tiếp.
