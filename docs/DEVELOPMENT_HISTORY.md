# PolyEc — lịch sử phát triển

Cập nhật: 2026-10-18. Đây là nhật ký quyết định, lỗi quan trọng và cách khắc phục. Lịch sử này không thay thế `CURRENT_TASK.md` và `reports/`.

## 2026-10-18 — chuẩn hóa docs và tách archive snapshot

- Bộ docs chính được giữ tối giản: `IMPLEMENTATION_PLAN.md`, `DEVELOPER_GUIDE.md`, `PLAYER_GUIDE.md`, `CURRENT_TASK.md`, `DEVELOPMENT_HISTORY.md`, `docs/README.md`.
- `docs/archive/` cũ không còn là source of truth; toàn bộ snapshot trước khi hợp nhất được lưu dưới `docs/archive_snapshots/2026-10-18/`.
- Mục tiêu: loại bỏ trùng lặp, giữ quyết định kỹ thuật quan trọng, giữ evidence và report tách biệt khỏi sản phẩm.
- Yếu tố đã được hợp nhất từ archive vào docs chính: M5 design decisions, household accounts/ledger, scaling complexity, rationale, và historical bug context.

## 2026-10-18 — agent-centric NPC redesign

- Xóa hướng suy nghĩ kiểu “NPC = 6 stat cố định + XP đơn lẻ” khỏi contract source; thay bằng `capability vector + policy + memory + attention budget + knowledge`.
- Duy trì gameplay rõ ràng: luật, quyền, action validation, transaction và causal chain là engine; policy chỉ lựa chọn hành động trong phạm vi đó.
- Hướng này phù hợp với thiết kế hiện tại: tính cách, body state, knowledge và attention được tách biệt; không gán mọi hành vi cho 6 chỉ số cứng.

## 2026-10-18 — tích hợp nguồn cảm hứng nhưng không sao chép nguyên bản

- Factorio được dùng làm mốc về bottleneck, batching và giảm update vô ích.
- Victoria 3 được dùng làm mốc về aggregate economic feedback và đời sống ở cấp toàn quốc.
- ODD protocol, Neural MMO và nghiên cứu sleep regulation chỉ là nền tư duy về state/process/scheduling và scale, không phải blueprint kỹ thuật để cứng hóa vào bản turn-based simulation.
- Kết luận: PolyEc có agency riêng cho từng NPC, sparse relation, transaction-safe world, và causal trace; không áp nguyên xi các mechanics từ các game khác.

## 2026-10-18 — attention evaluator và repair vòng đời

- Duyệt và triển khai evaluator attention stateless cho một actor, immutable session, half-open interval, và explicit priority.
- Xử lý bug quan trọng: overcommit, bad split-additivity, capacity overflow, ordering drift và invalid partial updates.
- Giải pháp: rõ ràng tách `requested`, `delivered`, `unmet`, `capacity_unmet`, `unresolved_unmet`; reject/partial không được sửa live state.
- Report nghiệm thu: `reports/M5-activity-evaluator-2026-10-18.md`.

## 2026-10-17 — core refactor và sửa lỗi nghiêm trọng

- Duyệt hướng thiết kế mới: tách intention, attention, presence, service và output; sleep/exertion/health được tách riêng.
- Bug nghiêm trọng phát hiện sau refactor:
  - patch text thay source
  - sim/command sai vị trí trong quá trình apply
  - payroll extraction sai do cập nhật không đúng thứ tự
- Cách giải quyết:
  - revert/refactor nhỏ theo nhóm
  - đưa test replay và invariant check
  - yêu cầu kiểm tra CTest và regression trước khi merge
- Đây là bài học chính về việc refactor lớn không được thực hiện trong một patch lớn.

## 2026-10-15 — M4 native và hợp nhất luật nền M5

- M4 hoàn thiện thêm map, logistics, xưởng, nợ và quyền đèo.
- Có trước đó issue về thiết kế các quỹ hộ, quyền sở hữu, cư trú và consent; mô hình này vừa được hợp nhất vào luật nền quốc gia.
- Một số kế hoạch cũ bị chặn vì không đủ consent/reservation/model compatibility.
- Kết quả: quy tắc chung toàn quốc, quyền, hôn nhân, thừa kế và cơ cấu tổ chức trở thành luật nền có thể chạy trong simulation.

## 2026-10-14 — roadmap và nền tảng native/RL

- Hợp nhất thiết kế sản phẩm: quốc gia, đời vua, one-campaign loop, C++ core + Godot UI + Python training + ONNX runtime.
- Roadmap được định nghĩa rõ: M0–M7 và mục tiêu scale S (nghiên cứu quy mô, không phải release).
- Tách biệt đặc điểm của engine với học tập RL: engine phải giữ state chuẩn, không phát triển rule riêng trong Python để “đỡ” việc học.

## M3 — pilot RL và các kết quả chưa đạt

- Có sandbox 6 lãnh địa + đất triều đình, khoảng 10k người, ba họ policy và pipeline recurrent PPO → ONNX/native.
- Một số kết quả pilot cho thấy sự cần thiết của validation và bảo thủ trong training.
- Lưu ý: artifact `realm-v1…v6` là lịch sử pilot, không phải model release mới.
- Bất kỳ checkpoint nào muốn dùng hiện tại phải có manifest, source lock và kiểm chứng parity.

## 2026-10-18 — hệ thống docs và quyết định ghi lại

- Tạo bộ docs chuẩn, chăm sóc source-of-truth và ràng buộc việc đọc đúng nguồn.
- Lần này là bản hợp nhất cuối cùng về docs: không chép bảng trạng thái vào nhiều file; không để document cũ cạnh tranh với document hiện hành.
- `reports/` lưu evidence theo run; `docs/archive_snapshots/2026-10-18/` lưu lịch sử để truy nguyên.

## Đã biết và còn đang mở

- Atlas v3 → individual mapping chưa có contract rõ ràng và vẫn là gap M5/M6.
- Hệ số kế thừa / calibration chưa được hiệu chuẩn;
- Một số field như sleep pressure, body deterioration, policy weights và cadence chưa đóng hoàn toàn
- Mọi thay đổi lớn cần đi kèm test, evidence và cập nhật `CURRENT_TASK.md`.

## Hướng dẫn cập nhật nhật ký

- Ghi ngày, revision, quyết định, evidence, và phần còn thiếu.
- Không ghi mọi lệnh chạy; chỉ ghi xét nghiệm và quyết định có giá trị.
- Mỗi sự kiện phải có liên hệ tới một source hoặc report nếu có thể.
