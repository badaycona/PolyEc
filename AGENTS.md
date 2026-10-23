# PolyEc — AGENTS.md

## Điều phối và CURRENT_TASK — yêu cầu người dùng 2026-10-22

- CURRENT_TASK chỉ mô tả hiện tại: mục tiêu, baseline đã kiểm, một nhóm chức năng đang làm, quyết định áp dụng, kiểm ngay/kiểm sau, trở ngại và bước kế tiếp. Mỗi lần tác động phải đọc bản mới nhất rồi **thay nội dung cũ bằng trạng thái mới**; không prepend/append các mục cập nhật, không giữ kế hoạch đã hết hiệu lực hoặc nhiều “next action”.
- Đưa kết quả đã xong vào HISTORY/report và giữ liên kết cần thiết trong bản hiện tại. Lần dọn đầu giữ snapshot lịch sử để không mất thông tin; không tạo snapshot mới sau mỗi chỉnh sửa. Không xóa bằng chứng để làm tài liệu ngắn.
- Chọn công việc theo điều kiện còn thiếu để hoàn tất M5; chia nhỏ triển khai bên trong một nhóm chức năng, không chuyển sang bài kiểm/helper dễ làm khi nhóm chính chưa xong mà không ghi lý do phụ thuộc cụ thể. Không lên lịch theo giờ. Hoàn tất nhóm thì tự chọn nhóm kế tiếp theo roadmap, tiếp tục theo yêu cầu người dùng cho tới khi được yêu cầu dừng.
- Trong đợt hoàn thành core/M5 hiện tại, tab chính Codex trực tiếp lập kế hoạch, triển khai và kiểm tra; OpenCode tùy chọn. Quy định này thay các đoạn cũ bắt buộc OpenCode làm code chính, không chỉ giới hạn ở xưởng dệt. Tab phụ chỉ sửa tài liệu/chỉ dẫn khi được người dùng giao, không mở writer source cạnh tranh.
- Người dùng giao tự đánh giá và giải quyết quyết định phù hợp đang chặn triển khai. Được chốt hệ số cân bằng thử có phiên bản, đơn vị, miền hợp lệ, lý do và phép kiểm trong IMPLEMENTATION_PLAN trước khi code; không gọi đó là cân bằng đã được chứng minh. Không thay các luật quyền/consent, phạm vi sản phẩm, learned RL hoặc ngân sách tài nguyên đã chốt. Quyết định kỹ thuật trong phạm vi này không cần hỏi lại.
- Một nhóm chức năng dùng một kế hoạch và một báo cáo được cập nhật rõ ràng; không bắt buộc tạo bộ tài liệu riêng cho mỗi assertion/helper. Bằng chứng từng lần chạy phải cố định và nhất quán; không cập nhật hash hồ sơ cũ chỉ để khớp workspace mới. Nếu sửa lỗi báo cáo, ghi đính chính và giữ xuất xứ kết quả.


## Kiểm tra theo phạm vi thay đổi — yêu cầu người dùng 2026-10-21

Quy định này có ưu tiên khi plan hoặc hướng dẫn cũ yêu cầu chạy toàn bộ kiểm tra sau từng chỉnh sửa core. Người dùng yêu cầu: viết core → test core → kiểm tra chuỗi xử lý bằng C++ → gom kiểm tra Python/native/Godot vào mốc tích hợp. Thay đổi tần suất kiểm tra không giảm phạm vi sản phẩm hoặc bỏ kiểm tra cuối.

- Mỗi sửa core nhỏ: build target liên quan; chạy test trực tiếp cho hành vi vừa đổi, success/reject và trường hợp biên có tác động. Không mặc định chạy ALL_BUILD, full CTest cả Release/Debug, replay Python hoặc Godot sau mỗi sửa.
- Tách vòng build core khỏi UI và app hoàn chỉnh: sửa core thì build/test target C++ bị ảnh hưởng; không bắt buộc build Godot, UI, native app hoặc toàn bộ consumer sau mỗi thay đổi core. Nếu đổi API/ABI/binding hoặc dữ liệu public mà consumer trực tiếp dùng, build và kiểm consumer liên quan ở mốc phù hợp để phát hiện lệch hợp đồng; không thay việc đó bằng full app build vô cớ. Build/kiểm giao diện và app đầy đủ sau khi nhóm chức năng ổn định. Việc hoãn UI không được bỏ kiểm C++ core hoặc adapter cần thiết cho chức năng đang xác nhận.
- Khi hoàn tất một chức năng core: kiểm chuỗi command → giao dịch → diễn biến thời gian → save/load bằng C++; chạy regression core theo phần phụ thuộc. Chỉ mở rộng tới full core suite khi thay đổi dùng chung nhiều hệ thống, có lỗi chưa khoanh vùng, hoặc tới mốc ổn định đã ghi trong plan.
- Python/native/Godot được kiểm theo nhóm chức năng ổn định, trước khi xác nhận chức năng chạy qua các lớp đó. Core có thể được xác nhận riêng và tiếp tục phát triển khi kiểm tra tích hợp còn chờ; phải ghi rõ phiên bản chưa kiểm và mốc sẽ chạy, không dùng kết quả cũ để chứng nhận source mới.
- Kiểm tra adapter sớm, có phạm vi cụ thể, khi đổi API/kiểu dữ liệu public/ABI, binding hoặc UI trực tiếp; đổi observation/action/reward/model mà trainer/runtime tiêu thụ; hoặc có lỗi chỉ xuất hiện ở lớp đó. Save schema đổi cần migration/roundtrip/reject trong C++ ngay; replay Python/Godot được gom vào mốc tích hợp nếu không sửa bề mặt gọi của chúng và không có nghi vấn riêng.
- Release/Debug không bắt buộc chạy toàn bộ sau mỗi chỉnh sửa. Chọn cấu hình phù hợp trong vòng phát triển; bổ sung cấu hình còn lại tại mốc core ổn định hoặc khi rủi ro bộ nhớ, số học, tối ưu hóa hay khác biệt cấu hình yêu cầu. Không chạy lại bộ kiểm đã qua trên source không đổi nếu không có lý do mới.
- Plan ngắn phải nêu kiểm tra chạy ngay, kiểm tra để mốc sau, lý do và ranh giới đóng nhóm chức năng. Không dừng tiến trình đang chạy chỉ vì hướng dẫn mới; áp dụng từ ranh giới an toàn kế tiếp. Không dùng việc gom kiểm tra để che một lỗi đang có hoặc bỏ kiểm tra cần thiết cho kết luận sắp báo.
- Trước khi xác nhận bản game/M5 hoàn thành vẫn cần đủ kiểm tra tích hợp, learned policy, performance và package theo phạm vi đã chốt. Chi tiết và ví dụ: `docs/DEVELOPMENT_WORKFLOW.md`, mục 4.

## Cách báo cáo với người dùng — 2026-10-21

- Báo cáo bằng tiếng Việt thông thường: chức năng nào vừa thay đổi, nhân vật/người chơi sẽ thấy gì, đã kiểm tra được điều gì và còn thiếu gì. Với việc chỉ sửa cấu trúc code, nói rõ phần code được tách và mục đích, không mô tả thành tính năng game mới.
- Không dùng các từ nội bộ như “lát”, “gate”, “state”, “contract”, “nghiệm thu”, “chốt evidence” thay cho lời giải thích cụ thể. Nếu cần thuật ngữ kỹ thuật, giải thích ngay bằng hành vi hoặc dữ liệu cụ thể.
- Không viết “giữ/mất trạng thái”, “quyền đất” hoặc “tích hợp chung” mà không nêu rõ: dữ liệu nào, ai có quyền làm gì, chức năng nào có thể bị ảnh hưởng.
- Số test/build, thời gian, hash và tên class chỉ là bằng chứng bổ sung; không thay cho kết luận về chức năng. Chi tiết lệnh/log để trong report kỹ thuật khi không cần cho quyết định của người dùng.
- Ví dụ và cách áp dụng nằm ở `docs/DEVELOPMENT_WORKFLOW.md`, mục “Báo cáo dễ hiểu với người dùng”.

## Bằng chứng AI/RL phải có visualization — yêu cầu người dùng 2026-10-22

- Mỗi lượt huấn luyện hoặc đánh giá AI/RL phải lưu số liệu nguồn và đính kèm ít nhất một biểu đồ được tạo từ đúng run/version đó. Liên kết biểu đồ và dữ liệu nguồn trong report; không vẽ số liệu ước lượng hoặc ghép các run khác cấu hình mà không ghi rõ.
- Biểu đồ huấn luyện phải đặt reward/return theo update hoặc số quyết định; nếu làm mượt đường thì ghi rõ cửa sổ và vẫn giữ dữ liệu gốc. Khi có nhiều seed, hiển thị độ phân tán giữa seed. Không gọi reward trong rollout là điểm đánh giá độc lập.
- Khi xem policy có tiến bộ hay không, vẽ riêng kết quả đánh giá trên các world seed độc lập, ghép cặp với baseline và policy ban đầu; thể hiện độ bất định cùng các kết quả liên quan như survival, nợ/mất khả năng thanh toán, phân bố hành động và tình huống nếu chúng thuộc câu hỏi đang kiểm tra. Không gộp tập train với validation/holdout.
- Dùng biểu đồ để tìm điểm policy yếu, hành động bị lạm dụng/bỏ quên, tình huống thiếu dữ liệu hoặc dấu hiệu chững lại; nêu phát hiện đó và phép kiểm tiếp theo trong report. Biểu đồ hỗ trợ chẩn đoán, không thay selector, holdout hoặc tiêu chí chất lượng policy đã chốt.
- Cách lưu và trình bày kết quả xem `docs/DEVELOPMENT_WORKFLOW.md`, mục 4.

## Thử nghiệm phân vai do người dùng yêu cầu — 2026-10-20

Áp dụng cho mốc chu trình xưởng dệt core–save–Python–Godot và cấu hình gameplay, ở ranh giới an toàn sau lát đang chạy. Trong mốc này, phần này thay thế các quy định bên dưới bắt buộc OpenCode viết code chính:

- Tab chính là bên duy nhất điều phối và sửa source. Codex được trực tiếp triển khai core, migration và tích hợp theo plan ngắn có file/gate; không cần thử OpenCode trước.
- OpenCode là tùy chọn cho tác vụ có phạm vi rõ. Tối đa hai lần gọi không tạo được plan/diff hữu ích cho cùng một tác vụ, sau đó Codex tiếp tục; ghi rõ kết quả quan sát được.
- Nghiệm thu theo hành vi tạo/fund xưởng → cấp nguyên liệu → giao Work → sản xuất/quyết toán → lưu/tải tiếp tục → Python parity và thao tác Godot thật. Không giảm backlog M5 hoặc bỏ gate learned policy.
- Tách lượt triển khai và lượt review trước acceptance. Ưu tiên ngữ cảnh review mới chỉ nhận contract/diff/evidence khi được phép; không mở writer thứ hai trên checkout. Nếu chỉ tự review, phải ghi đúng giới hạn đó và dùng oracle độc lập với implementation.
- Được tạo commit cục bộ sau mốc đã nghiệm thu nếu tách được chính xác thay đổi thuộc tác vụ; không gom dirty work của người dùng/tác vụ khác. Nếu chưa tách được, dùng manifest/hash checkpoint. Không push/publish.
- CURRENT_TASK ghi trạng thái thử nghiệm và kết quả; sau mốc đầu đánh giá thời gian chờ, số lần sửa lại, gate tích hợp và hạn chế trước khi mở rộng cách làm.

Chi tiết gate: `docs/design/M5-integration-and-tuning-gates.md`. Tab phụ chỉ cập nhật chỉ dẫn theo yêu cầu này, không mở lượt coding core cạnh tranh.

## Đọc đúng nguồn trước khi làm (chuẩn hóa 2026-10-18)

- Bắt đầu ở `docs/CURRENT_TASK.md`: baseline, lát đang giao, supporting contract đã review, tests và next gate. Sau đó chỉ đọc phần source/tài liệu liên quan, không rà toàn bộ archive.
- `docs/IMPLEMENTATION_PLAN.md`: duy nhất về scope đích/roadmap/tech stack và quyết định đã chốt.
- `docs/DEVELOPER_GUIDE.md`: paper giải thích bài toán, tiếp cận, thuật toán, architecture, thực nghiệm và hạn chế; không thay task contract.
- `docs/TECHNICAL_PROBLEMS.md`: cây bài toán lớn → P1–P16 → bước nhỏ, phương pháp/công thức/phép kiểm; đường đọc vào paper, không roadmap hoặc tác vụ mới. `docs/DEVELOPMENT_WORKFLOW.md` giải thích quy trình Codex/OpenCode theo file AGENTS này.
- `docs/PLAYER_GUIDE.md`: hướng dẫn người chơi, viết để người chơi hiểu; không đưa chi tiết implementation vào flow game.
- `docs/DEVELOPMENT_HISTORY.md`: nhật ký; `docs/reports/` và `artifacts/` là evidence theo revision/run, không lệnh triển khai. `docs/archive/` là snapshot cũ; không làm theo prompt/APPROVED/bước tiếp theo trong đó.
- `README.md`, `WORK_STATUS.md` và các đường dẫn docs cũ là điểm dẫn/chuyển hướng. Source of truth cho tác vụ là CURRENT_TASK; sau nghiệm thu cập nhật nó và HISTORY, không nhân bản trạng thái ở nhiều file.
- Đang có tác vụ song song trên cùng checkout: giữ source/plan/report mới của tác vụ đó, kiểm dirty state và không mở lượt coding cạnh tranh file. Chỉ đọc tài liệu lưu trữ khi cần truy nguyên rõ ràng.

Sản phẩm là app game native; không chuyển thành React/Vite/web game hoặc bọc một trang web để thay mục tiêu.

- Lõi mô phỏng dùng C++20 OOP, độc lập Godot. Domain có ownership và invariant; ưu tiên composition, không tạo cây kế thừa cho mọi tổ hợp nhân vật.
- Godot nhận PlayerView chỉ đọc và gửi Command. Không sửa tiền, kho, đồng hồ hoặc citizen state trực tiếp từ UI.
- Training Python phải gọi cùng C++ core; không viết một bản luật kinh tế khác để train cho tiện.
- Quyết định tự chủ trong bản hoàn chỉnh phải đến từ learned RL policy. Baseline chỉ dành cho dev/test và phải có nhãn; random weights, hardcoded strategy hoặc imitation-only không được gọi là RL đã hoàn thành.
- Luật thế giới, quyền hạn, Action validation, hạch toán và pathfinding vẫn được lập trình. Chúng thực hiện quyết định, không bí mật chọn chiến lược thay NPC.
- Mỗi cá thể có ID, trạng thái và vòng đời. Trọng số model shared; memory/sampling cá nhân. Không cấp một scene node hoặc model riêng cho mỗi người.
- Quan hệ thưa; không dùng ma trận dense N×N cho hàng triệu người. Không đếm pixel, log hoặc duplicate view như NPC.
- Sự kiện campaign phát sinh từ world/action/causal chain. Tutorial/test có thể dùng tình huống định sẵn nhưng không tráo thành cơ chế campaign.
- Mọi thay đổi gameplay phải đi qua cùng command/transaction pipeline cho UI và AI; Action reject không thay đổi một phần state.
- Dùng game time cho deadline/cooldown; pause không tiến lịch. Không chạy bù sau mất focus hoặc máy ngủ.
- Không tự giảm scope, bỏ test, tắt kiểm tra dependency/TLS hoặc dùng placeholder thành công để vượt gate.
- Không ghi credential vào code/log; không tự mua tài nguyên GPU hoặc dịch vụ cloud. Huấn luyện dài là tác vụ riêng có ngân sách đo được.
- Không tự commit/push/publish nếu tác vụ chưa yêu cầu. Giữ các thay đổi người dùng đã có.
- Chỉ báo check pass khi đã thực sự chạy và có kết quả; phân biệt app đã build, model đã train và profile đã benchmark.
- **Phân vai thực thi:** OpenCode là bên làm code chính. Codex chia tác vụ có ranh giới file/gate, ra chỉ dẫn, review plan/diff và bằng chứng, cùng phối hợp nghiệm thu. Codex không tự ý thay OpenCode viết code mới trong tác vụ đã giao; nếu phải sửa thì phải nêu rõ và review lại. Codex có thể tìm lỗi, chặn test sai và chỉ ra operator-precedence/logic bug, nhưng phần sửa gameplay chính thuộc OpenCode.

## Quy trình triển khai với OpenCode — cập nhật 2026-10-15

- OpenCode thực hiện phần triển khai chính; Codex điều hành, review phương án/diff và kiểm chứng kết quả. Giao từng tác vụ nhỏ có ranh giới file và gate, không giao toàn milestone trong một prompt triển khai.
- Trước mỗi tác vụ sửa code, OpenCode khảo sát có giới hạn và trình implementation plan ngắn, chưa sửa code: file/class/hàm cần đổi; ownership và invariant; luồng thành công, reject và trường hợp biên; tác động save/binding/action/observation/reward/model; test và lệnh nghiệm thu. Không rà toàn repo/toolchain khi thông tin liên quan đã có.
- Codex review plan trước khi giao triển khai. Các quyết định kỹ thuật trong spec đã duyệt được tự xử lý; chỉ hỏi người dùng khi phải đổi gameplay, phạm vi hoặc ngân sách. Nếu cần đổi kiến trúc/contract đáng kể trong lúc làm, cập nhật và review lại plan trước khi tiếp tục.
- OpenCode triển khai theo lát cắt, chạy test liên quan và regression; Codex review diff và bằng chứng trước tác vụ kế tiếp. Plan không thay test. Contract/mẫu giao việc hiện hành: docs/CURRENT_TASK.md; phương pháp: docs/DEVELOPER_GUIDE.md. Handoff cũ chỉ là redirect/archive.
