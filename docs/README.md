# PolyEc — tài liệu hiện hành

Thiết kế lại ngày 2026-10-18 theo nhu cầu người đọc. Bộ docs không bị giới hạn ở năm file: mỗi tài liệu có câu hỏi, người đọc và trách nhiệm riêng. Giữ chi tiết trong nguồn hiện hành; không thay đặc tả bằng câu “xem bản gốc”.

| Tài liệu | Người đọc và câu hỏi trả lời |
|---|---|
| [README dự án](../README.md) | Người mới: mục đích game, tech stack, folder tree, giai đoạn chung và bắt đầu ở đâu |
| [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md) | Chủ dự án/dev: toàn project cần làm gì, lựa chọn đã chốt, stack/roadmap/gates |
| [PLAYER_GUIDE](PLAYER_GUIDE.md) | Người chơi: mở game, điều khiển, tính năng có thật và điều cần hiểu, bằng ngôn ngữ đời thường |
| [TECHNICAL_PROBLEMS](TECHNICAL_PROBLEMS.md) | Dev mới/reviewer: sáu bài toán lớn → P1–P16 → bước nhỏ, input/output, phương pháp, công thức và phép kiểm |
| [DEVELOPER_GUIDE](DEVELOPER_GUIDE.md) | Dev: paper về bài toán, phương pháp, kiến trúc/thuật toán, thực nghiệm, hạn chế và build |
| [DEVELOPMENT_WORKFLOW](DEVELOPMENT_WORKFLOW.md) | Codex/OpenCode: phân vai, plan/review/code/test/acceptance và safe handoff theo AGENTS |
| [DEVELOPMENT_HISTORY](DEVELOPMENT_HISTORY.md) | Người theo dõi: quyết định, thay đổi và evidence theo ngày/revision |
| [CURRENT_TASK](CURRENT_TASK.md) | Agent/coding agent/dev bàn giao: task đang làm, files/contract/ownership, tests và next action |

## Các đường đọc

- **Người muốn chơi:** README → PLAYER_GUIDE. Không cần biết C++, PPO hoặc đọc contract để chơi thử.
- **Người muốn hiểu sản phẩm:** README → IMPLEMENTATION_PLAN phần 1–10 → phần 11–21 theo chủ đề. PLAYER_GUIDE giải thích trải nghiệm bằng lời dễ hiểu.
- **Dev mới:** README/folder tree → TECHNICAL_PROBLEMS → đúng phần DEVELOPER_GUIDE → source/tests. Cây bài toán là đường vào paper, không phải một roadmap cạnh tranh.
- **Agent đang nhận việc:** AGENTS → CURRENT_TASK → supporting contract/source liên quan; dùng DEVELOPMENT_WORKFLOW khi cần quy trình. Không đọc tuần tự toàn archive.
- **Người review tiến độ:** CURRENT_TASK → report của lát → DEVELOPMENT_HISTORY để hiểu lý do và giới hạn của mốc cũ.

## Một thông tin nên nằm ở đâu?

| Thông tin | Nơi sở hữu | Nơi khác làm gì? |
|---|---|---|
| Đích game, luật được chốt, scope, roadmap, budgets | IMPLEMENTATION_PLAN | Tóm tắt có link, không đặt thêm luật |
| Cách người chơi dùng tính năng | PLAYER_GUIDE | Tách bản thử hiện có khỏi sản phẩm dự kiến |
| Phân rã bài toán và bước nhỏ | TECHNICAL_PROBLEMS | Paper giải sâu; task chọn một lát để làm |
| Phương pháp/toán/thuật toán và giả thiết | DEVELOPER_GUIDE | Cây bài toán chỉ công thức cốt lõi và dẫn phần chi tiết |
| Quyền thực thi | AGENTS | WORKFLOW giải thích cách áp dụng, không mở rộng quyền |
| Lát active, acceptance, test gate và next action | CURRENT_TASK | README chỉ giai đoạn chung; không nhân bản status |
| Contract chi tiết một lát | docs/design, được CURRENT_TASK dẫn | Cần review rõ; file tồn tại không có nghĩa đã duyệt |
| Kết quả một run/revision | docs/reports và artifacts | History tóm tắt có nguồn, không chứng nhận HEAD |
| Vấn đề/mốc/giải pháp theo thời gian | DEVELOPMENT_HISTORY | Thêm đính chính thay vì xóa kết quả xấu |

## `docs/design/` bổ trợ thế nào và tài liệu nào cần thận trọng

`TECHNICAL_PROBLEMS` và `DEVELOPER_GUIDE` là tài liệu giải thích dùng lâu dài: một tài liệu phân rã câu hỏi cần giải, tài liệu kia giải thích phương pháp, thuật toán, công thức và giới hạn. Thông tin khái quát về kiến trúc core từ `core-architecture-refactor.md` đã được đưa vào DEVELOPER_GUIDE mục 2–3; phương pháp chọn truy vấn, index và thuật toán từ `core-query-algorithms.md` đã được đưa vào DEVELOPER_GUIDE mục 7.3 và TECHNICAL_PROBLEMS P14. `docs/design/` không phải phần tiếp nối bắt buộc của hai tài liệu này và không phải roadmap thứ hai. Phần còn lại của thư mục chủ yếu là kế hoạch/contract cho từng lát, cộng tài liệu tham khảo kiến trúc có mốc đối chiếu cụ thể. Chỉ dùng một plan làm căn cứ triển khai khi `CURRENT_TASK` dẫn tới nó và trạng thái review của plan đã rõ; sau khi xong, report và HISTORY giữ bằng chứng, còn plan giải thích phạm vi/ý định của lát đó.

Một số file trong `docs/design/` đã hoàn thành vai trò ban đầu hoặc chỉ mô tả một ảnh chụp code cũ. Chúng vẫn có ích để truy nguyên, nhưng không tự mô tả trạng thái hiện tại:

- `implementation-plan.md`, `M5-draft.md`, `M5-simulation-design-decisions.md`, `realm-domain.md` và `simulation-complexity-scope.md` là đường dẫn chuyển hướng; nội dung lịch sử nằm trong `docs/archive/design/`. Không dùng chúng thay `IMPLEMENTATION_PLAN`.
- `core-class-reference.md` ghi rõ baseline Git `6cc55ab` và ngày đối chiếu 2026-10-19. Đây là ảnh chụp có thay đổi chưa commit, không phải danh mục class đã đối chiếu với code hiện tại. Những nguyên tắc sở hữu, composition, record và ranh giới Realm/Campaign đã được tổng hợp ở DEVELOPER_GUIDE mục 2.3; không chép danh sách class cũ vào guide như thể nó còn khớp source. Kiểm tra source khi cần tên class/trách nhiệm hiện tại.
- `core-architecture-refactor.md` là bản giải thích thiết kế ngày 2026-10-19, không báo cáo tiến độ. Các nguyên tắc tổng quát đã được tổng hợp ở DEVELOPER_GUIDE mục 2–3; kết quả R1–R4 có report tích hợp riêng, còn trạng thái tiếp theo lấy từ `CURRENT_TASK` và `IMPLEMENTATION_PLAN`.
- `core-query-algorithms.md` là ghi chú phương pháp ngày 2026-10-19. Các hướng truy vấn, index và thuật toán dùng lâu dài đã được hợp nhất vào DEVELOPER_GUIDE mục 7.3 và TECHNICAL_PROBLEMS P14; bản trong `docs/design/` giữ xuất xứ và ví dụ cũ, không là hướng dẫn cập nhật độc lập.
- Các file `M5-*`, `M5.*-*` và `R4-*` khác thường là contract hoặc kế hoạch cho một lát cụ thể. File không được CURRENT_TASK dẫn có thể là kế hoạch đã hoàn tất, bị thay thế hoặc chỉ giữ lịch sử; xem report/HISTORY tương ứng trước khi xem là yêu cầu đang mở.

Không xóa các file này chỉ vì đã cũ: kế hoạch và ảnh chụp cũ giúp giải thích quyết định và xuất xứ kết quả. Khi phát hiện một file có chỉ dẫn mâu thuẫn, ưu tiên thứ bậc nguồn ở phần “Quy tắc đọc và cập nhật”, rồi sửa nhãn hoặc thêm liên kết thay vì âm thầm sửa nội dung lịch sử.

Mỗi tài liệu kỹ thuật phải phân biệt **đích thiết kế**, **phương án chưa chốt**, **baseline pilot** và **kết quả có evidence**. Đây là cách đọc trạng thái của phát biểu, không thêm bốn bảng tiến độ mới. Tham số kế thừa sáu stat/XP không tự thành schema capability mới.

## Quy tắc đọc và cập nhật

Agent: AGENTS → CURRENT_TASK → đúng supporting plan/source được giao. Scope hỏi IMPLEMENTATION_PLAN; cần hiểu phương pháp đọc DEVELOPER_GUIDE. Không tự đọc toàn repo/archive hoặc thực thi prompt cũ. Plan phải được review trước code; plan không thay test.

Nguồn ưu tiên: chỉ đạo trực tiếp người dùng/AGENTS → IMPLEMENTATION_PLAN cho scope → CURRENT_TASK cho tác vụ/status → DEVELOPER_GUIDE cho giải thích. Evidence theo revision dùng xác nhận kết quả, không tự sửa luật. Nếu các nguồn mâu thuẫn hoặc contract còn thiếu, ghi lại và review thay vì chọn tài liệu cũ thuận tiện hơn.

Sau nghiệm thu cập nhật CURRENT_TASK, HISTORY và report tương ứng. Thay luật/scope thì cập nhật IMPLEMENTATION_PLAN; đổi implementation/phương pháp thì cập nhật DEVELOPER_GUIDE; tính năng người chơi đã có thì cập nhật PLAYER_GUIDE. Không đồng loạt copy trạng thái sang mọi file.

## Ma trận bao phủ toàn project

Bộ docs phải giữ chi tiết cần thiết trong nguồn chuẩn, không chỉ tóm tắt rồi đẩy người đọc về archive. Các đường dẫn dưới đây trả lời câu hỏi hiện hành; bản lưu chỉ dùng đối chiếu nguồn gốc.

| Nội dung | Nguồn chi tiết hiện hành | Phần chưa thể tuyên bố hoàn thành |
|---|---|---|
| Tầm nhìn, vòng chơi, ending | IMPLEMENTATION_PLAN 1–4, 11; PLAYER_GUIDE | Campaign một đời vua/ending M6 |
| Địa lý, 24+3 khu, tuyến, bản đồ/overlay | IMPLEMENTATION_PLAN 12; PLAYER_GUIDE | Quốc gia đầy đủ; ánh xạ dân số atlas sang cá thể |
| Chính trị, quyền, cam kết, điều tra | IMPLEMENTATION_PLAN 2, 13–14 | Hệ quyền/thiết chế M5, privacy đầy đủ |
| Hộ, công việc, hôn nhân, di sản | IMPLEMENTATION_PLAN 2–3, 7; DEVELOPER_GUIDE 2–5 | Domain động và năm giao dịch nghiệm thu |
| 16 hàng, chợ, nợ, dự án, quân sự/ngoại thương | IMPLEMENTATION_PLAN 15–17; DEVELOPER_GUIDE 3, 5 | Chuyển từ pilot sang quy mô và luật đích |
| UI và trải nghiệm, F1, ending, accessibility | IMPLEMENTATION_PLAN 18; PLAYER_GUIDE | M6 và kiểm thử luồng hoàn chỉnh |
| 12 augment, draft, stat/traits/XP | IMPLEMENTATION_PLAN 19–20; DEVELOPER_GUIDE 6 | Hệ số kế thừa chưa hiệu chuẩn, chưa chứng minh agency |
| Sự kiện/knowledge/causal chain | IMPLEMENTATION_PLAN 21; DEVELOPER_GUIDE 5 | Chuỗi đủ M5, information audit và đối chứng |
| Kiến trúc, source map, toán, thuật toán | README; TECHNICAL_PROBLEMS; DEVELOPER_GUIDE 2–8, 11 | Mỗi công thức phân biệt code/đích thiết kế |
| RL, dataset, curriculum, co-play, quality | IMPLEMENTATION_PLAN 8; DEVELOPER_GUIDE 6, 9 | R1–R4 còn các phần chưa chứng minh; production sau freeze |
| Hiệu năng, thread, save, platform/package | IMPLEMENTATION_PLAN 9; DEVELOPER_GUIDE 7–8, 11 | B/S, full-cycle và platform chưa đo phải ghi riêng |
| Milestones, phụ thuộc và nghiệm thu | IMPLEMENTATION_PLAN 6–9; CURRENT_TASK | Không lấy milestone có code thay mọi gate |
| Lý do thay đổi, run và giới hạn thực tế | DEVELOPMENT_HISTORY; reports | Evidence chỉ áp revision/workload của report |

Những chỗ chưa có contract hoặc evidence là khoảng trống của project được nêu rõ, không được lấp bằng việc viết một công thức mới như thể đã duyệt. Khi thêm yêu cầu mới, phải đưa vào một hàng/phần tương ứng, cập nhật trạng thái và nguồn chi tiết; không chỉ tăng số trang.

## Tài liệu hỗ trợ và kho lịch sử

- [Reports](reports/README.md): output/evidence theo revision/platform/workload. M3/M4 cũ không chứng nhận M5 hoặc HEAD mới.
- [Task contracts hiện còn dùng](CURRENT_TASK.md): plan activity/T1 và execution ledger của tab chính giữ đường dẫn để tránh ảnh hưởng tác vụ đang chạy. Chỉ active khi CURRENT_TASK dẫn đúng bản đã review.
- [Guides theo gói](guides/README.md): file .txt cố định theo build, còn dùng cho package; hướng dẫn người chơi mới đọc PLAYER_GUIDE.
- [Archive](archive/README.md): docs design/developer/experiments/handoff trước hợp nhất, gồm cả proposal/APPROVED/prompt cũ. Không là lệnh active. File ở đường dẫn cũ chỉ chuyển hướng.
- [Artifacts](../artifacts/README.md): checkpoint/source-lock/evaluation/visual; giữ nguyên vì còn dependency và provenance. Source snapshot không là source làm việc.

Việc tổ chức docs không thay đổi gameplay, không chạy training hoặc chứng nhận test/benchmark mới.

## Khi cần thêm tài liệu

Công thức dùng LaTeX phù hợp Preview Codex hiện tại: `$$...$$` trên cùng dòng với văn bản cho ký hiệu/công thức inline; `$$` trên dòng riêng cho phương trình display. Renderer hiện cấu hình `singleDollarTextMath: false`, nên không dùng `$...$` cho inline. Nếu một đoạn bắt đầu bằng công thức inline, thêm lời dẫn trước nó để parser không hiểu thành mở khối display. Dùng phân số, chỉ số, tổng và hàm toán đúng cú pháp; giải thích ký hiệu/đơn vị cạnh công thức. Backtick dành cho tên API, file và mã, không dùng để trình bày phương trình. Công thức dài dùng `aligned` để dễ đọc; giữ rõ giả thiết và trạng thái đề xuất/đã kiểm chứng.

Thêm file khi có người đọc/câu hỏi hoặc supporting contract đủ riêng để cần một nơi rõ ràng. Ghi vai trò, nguồn ưu tiên và link từ đây hoặc CURRENT_TASK phù hợp. Không thêm bản sao roadmap/status hoặc tách một phần quan trọng rồi để docs chính chỉ còn placeholder. File dài có mục lục/đường đọc; file ngắn vẫn phải đủ nội dung để trả lời câu hỏi của nó.

Kiểm tài liệu bằng `python scripts/check-repository-links.py` từ root, `git diff --check`, kiểm heading/code fence/math delimiter, parse bằng remark-math với `singleDollarTextMath: false` và render KaTeX, rồi kiểm coverage theo bảng trên. Link hợp lệ chỉ chứng minh đường dẫn tồn tại; cần review riêng nội dung, trạng thái phát biểu và công thức. Không coi kiểm Markdown là bằng chứng build hoặc gameplay.
