# PolyEc — lịch sử phát triển

## 2026-10-18 — T1a core timeline

OpenCode triển khai Realm schedule/command/game-time/save v7; Codex review và nghiệm thu core Windows, CTest 8/8. Có continuation giữa interval, death retirement, shared command sequence, overcommit và reject/load hỏng có đúng lý do. [Bằng chứng](reports/M5-T1a-evidence-2026-10-18.md). Native/Python parity và T1b clock/pause chưa đạt; M5 chưa hoàn thành.

Đây là nhật ký theo thời điểm, không phải lệnh triển khai. Đọc [CURRENT_TASK](CURRENT_TASK.md) để tiếp tục công việc và [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md) để biết scope đích. Kết quả cũ chỉ áp dụng revision/workload được dẫn, không tự chứng nhận HEAD hiện tại.

## 2026-10-14 — hướng native/OOP/RL và roadmap v3

Hợp nhất đích một quốc gia, một đời vua; C++ core, Godot native, Python shared-core training và ONNX CPU. Roadmap M0–M7 và nghiên cứu mở rộng S phân biệt nền engine, pilot RL và production training. Mô tả cũ về prototype 4–6 vùng không là quyết định giảm scope quốc gia.

## M3 — pilot khả thi và các kết quả chưa đạt

Đã có sandbox sáu lãnh địa + đất triều đình, khoảng 10k người, ba họ policy và đường recurrent PPO→ONNX/native. Báo cáo giữ cả audit xấu: governance không cải thiện so untrained trong categorical audit; các họ chưa qua đầy đủ baseline noninferiority; default cao, goal/INT response yếu; sandbox dài chưa chứng minh xã hội cân bằng.

Chi tiết, số seed, checkpoint, CI/platform và chất lượng: [M3 evidence](reports/M3-evidence.md). Artifact realm-v1…v6 là pilot/source-locked experiments; không model release mặc định.

## 2026-10-15 — M4 native và nền xã hội M5

M4 có map, quyền đèo, logistics hữu hạn, Hắc Sơn/nợ và xưởng dệt. Core/native/UI/Python/package Windows được ghi trong [M4 evidence](reports/M4-evidence.md); việc thường lệ dùng controller có nhãn, không chứng nhận RL production. Profile M4 khác workload ONNX và chưa B/C.

Người dùng chốt luật chung toàn quốc, bình đẳng giới, consent hôn nhân, tách tài sản/tước/chức, giáo hội giả tưởng và vai trò vua. Duyệt M5 với 24+3 khu, 16 hàng và profile B; ưu tiên Windows, hoãn app/package Linux nhưng giữ portable core/Python. Production training sau feature freeze.

OpenCode là bên triển khai chính; Codex giao lát nhỏ, review plan rồi diff/evidence. Membership hộ thưa/guardian được nghiệm thu, giữ realm-v6 save vì index dẫn xuất. Có test hộ không liền ID, guardian chết/thiếu, wrong-target/stale và corrupt load. [M5 progress](reports/M5-progress.md), commit [d5751bb](https://github.com/badaycona/PolyEc/commit/d5751bb).

Plan quỹ hộ đầu tiên chưa được duyệt code vì consent/reservation/model compatibility chưa đủ. Các yêu cầu cũ chặn nhận việc vì xung đột thời gian sau đó bị thiết kế 17/10 thay thế; không quay lại plan đó chỉ vì nó có sẵn trong repo.

## 2026-10-17 — làm rõ mô phỏng và sửa core refactor

Duyệt hướng nhiều phiên/ngày; tách intention, attention, presence, service và output; sleep/exertion/health riêng; cho nhận quá tải, thực hiện có giới hạn và hậu quả. Không hard-gate lựa chọn kém khôn ngoan; world không tự chọn chiến lược. Hệ số cơ thể/cadence mới chưa được chốt.

Người dùng xác nhận các commit refactor core tới a5647a5 do GitHub Copilot thực hiện. Sau pull phát hiện patch text thay source và sim/command sai vị trí, cùng payroll extraction sai thuế/rounding. OpenCode sửa giữ intent manager/WorldState, bỏ public mutable access, khôi phục behavior và thêm test manager. Codex review/rerun Windows CTest 6/6. Đã push [b8160f8](https://github.com/badaycona/PolyEc/commit/b8160f88379418c168f53fd7f951e0772075fb0f). [Evidence](reports/copilot-core-repair-2026-10-17.md).

## 2026-10-18 — attention evaluator và tác vụ tiếp theo

OpenCode triển khai evaluator stateless một actor, immutable sessions, absolute half-open horizon, rate capacity 1000, explicit priorities, overcommit hợp lệ và counters phân biệt capacity/unresolved. Tick trừu tượng, không sinh output/care/health hoặc tự chọn thắng tie theo ID.

[Report](reports/M5-activity-evaluator-2026-10-18.md) ghi full CTest 7/7 Codex chạy độc lập, rồi targeted 1/1 sau sửa size gate của split-additivity test. Không dùng số test này chứng nhận clock/save/binding hoặc profile B. Lát tiếp theo T1 nối Realm/command/time/save/version; công việc tab chính tiến tiếp theo [CURRENT_TASK](CURRENT_TASK.md). Không cố định trạng thái dirty/commit của tác vụ đang chạy vào nhật ký này.

## 2026-10-18 — chuẩn hóa tài liệu theo yêu cầu người dùng

Tạo năm nguồn chuẩn: IMPLEMENTATION_PLAN, PLAYER_GUIDE, DEVELOPER_GUIDE theo dạng paper, DEVELOPMENT_HISTORY và CURRENT_TASK. README/WORK_STATUS/AGENTS dẫn tới chúng. Nội dung design/developer/experiments/handoff cũ vào archive với đường dẫn cũ làm redirect; giữ nguyên file tác vụ mới mà tab chính đang làm.

Reports giữ evidence theo run; guides `.txt` giữ input đóng gói theo phiên bản. Checkpoint/source-lock/manifest/artifact không xóa hoặc sửa để giả tương thích. Bản lưu còn truy nguyên được nhưng không là nguồn lệnh hiện hành. Công việc docs không chứng nhận build/train/benchmark mới và không tự commit/push.

Kiểm tra cho lần tổ chức này: checker repo qua 322 tham chiếu local tài liệu/resource; diff whitespace và JSON OpenCode hợp lệ; 27 bản lưu giữ nguyên prose ngoài banner/đích link; sáu input đóng gói còn nguyên đường dẫn/nội dung; runtime models và artifact dữ liệu không đổi. Không chạy lại CTest gameplay cho thay đổi docs-only, không lấy test do tab chính đang chạy làm bằng chứng riêng của việc tổ chức docs.

## 2026-10-18 — bổ sung lại chiều sâu và độ bao phủ của bộ docs

Người dùng phản hồi Developer Guide thiếu tính technical và toàn bộ docs còn thiếu nhiều nội dung. Nguyên nhân của bản hợp nhất đầu là rút gọn quá mạnh, để chi tiết sản phẩm/phương pháp chỉ còn trong archive. Bản sửa đưa nội dung cần dùng trở lại nguồn chuẩn; archive vẫn là bản lưu truy nguyên.

- IMPLEMENTATION_PLAN thêm đặc tả chi tiết vòng chơi/ending, danh mục 24+3 khu và tuyến, chính trị, thông tin/điều tra, kinh tế, chính sách, quân sự/ngoại thương, UI, đủ 12 augment, stat/traits/XP và sự kiện trồi sinh.
- DEVELOPER_GUIDE phân rã P1–P16, ghi đơn vị/ký hiệu; phục hồi toán hạch toán/lương/thuế, graph logistics, GRU, reward/GAE/PPO/BPTT; bổ sung attention, time splitting, settlement, memory/throughput và phương pháp đánh giá. Công thức đề xuất không được trình bày như implementation đã nghiệm thu.
- PLAYER_GUIDE giải thích hướng chơi toàn bộ bằng ngôn ngữ người chơi, tách bản M4 hiện có khỏi sản phẩm dự kiến. CURRENT_TASK bổ sung checklist tác động domain/clock/save/adapter/RL/test, giữ tác vụ đang làm ở tab chính. docs/README có ma trận bao phủ theo chủ đề.

Giữ rõ hai khoảng trống: dân số bối cảnh atlas v3 chưa có contract ánh xạ sang cá thể profile B; hệ số cân bằng kế thừa chưa được hiệu chuẩn và không tự tương thích timeline subday. Các quality gates, body/reward mới và quy mô chưa đo tiếp tục để mở. Lần sửa này chỉ là docs; không tạo bằng chứng build/train/gameplay mới, không commit/push.

Kiểm tra tài liệu lần bổ sung: 339 tham chiếu local tài liệu/resource hợp lệ; cấu trúc heading/code fence hợp lệ; Developer Guide có 48 khối công thức LaTeX với delimiter ghép cặp. Có đủ 12 định nghĩa augment và 24 lãnh địa trong plan chuẩn. Đây là kiểm tra source Markdown, chưa phải nghiệm thu render PDF hay thẩm định khoa học mọi công thức. Không chạy lại CTest gameplay cho sửa docs.

## Cách cập nhật nhật ký

Thêm sự kiện khi có quyết định, nghiệm thu hoặc thay contract đáng kể. Ghi ngày, revision/source context, điều đã làm, evidence và phần chưa đạt. Không ghi mọi tool call. Nếu phát hiện kết quả cũ sai, thêm đính chính có căn cứ; không xóa dữ liệu xấu hoặc sửa artifact khóa nguồn.
