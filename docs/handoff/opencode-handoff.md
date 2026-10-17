# PolyEc — Bộ giao việc cho OpenCode

Điểm tiếp tục cập nhật 2026-10-17: [kế hoạch sau pull](next-steps-2026-10-17.md). Ưu tiên sửa tính toàn vẹn source/build của revision a5647a5 rồi nối timeline theo thiết kế 17/10; các mục tiếp tục 15/10 bên dưới giữ làm lịch sử khi mâu thuẫn. Quy trình plan → review → code/test → nghiệm thu vẫn áp dụng.

## Tiếp tục hiện tại — M5 lát cắt hộ gia đình (2026-10-15)

Spec đã duyệt: [M5 draft](../design/M5-draft.md) (APPROVED 2026-10-15). Ưu tiên Windows trước; app/package Linux hoãn, core/Python portable vẫn phải kiểm thử (Kaggle). Phân vai: OpenCode viết code chính; Codex chia tác vụ, ra chỉ dẫn, review diff/bằng chứng và phối hợp nghiệm thu — không tự thay OpenCode viết code trong tác vụ đã giao.

Đã xong và đã chạy Windows (toolchain: Zig 0.13 + Clang 18, C++20, CMake 3.31.6):

- `core/src/realm.cpp`, `core/include/polyec/realm.hpp`: thay quét `household..household+3` bằng membership index thưa dẫn xuất (`households_`), có `household_of`, `household_members`, `same_household`, `guardian`, `household_authority`; index dựng lại khi load, không thêm tiền/world thứ hai.
- `Intent::Support` nhắm tới authority thật (founder còn sống đủ tuổi → thành viên lớn tuổi nhất, không thì adult sống cao tuổi nhất); reject `c.target<=0` trước khi tra person.
- Save `realm-v6`/`POLYEC_REALM_6` giữ nguyên; reject save có household label rỗng hoặc ngoài dải người.
- `core/tests/test_realm.cpp`: fixture bằng scalar surgery trên save text thật (không friend/private), test membership không liền nhau, giám hộ chết/thiếu, authority cùng hộ, từ chối target trong hộ khác/self/0, roundtrip và corrupt load.
- Lệnh đã chạy: `& build/tooling/cmake/data/bin/cmake.exe --build build/m5-core --parallel 4`; `& build/tooling/cmake/data/bin/ctest.exe --test-dir build/m5-core --output-on-failure` → CTest 5/5, `realm_tests` 25.298 check. Chi tiết: [M5 progress](../reports/M5-progress.md).

Việc kế tiếp: M5.1 — hộ động/thừa kế/hôn nhân qua cùng command/transaction pipeline; không đổi schema save nếu giữ được semantics thật ở index dẫn xuất.

Các mục lịch sử phía dưới giữ nguyên để truy nguyên; chúng là hồ sơ M0/M3/M4, không phải việc cần làm lúc này.

## Quy trình hiện hành — người dùng yêu cầu ngày 2026-10-15

Áp dụng cho công việc triển khai tiếp theo, ưu tiên hơn các prompt/workflow lịch sử bên dưới. Người dùng đã duyệt draft M5; OpenCode là người triển khai chính, Codex điều hành và nghiệm thu. Ưu tiên Windows; giữ core/Python portable để chuẩn bị Kaggle, hoãn app/package Linux. Không khởi động lại hoặc hủy tiến trình đang chạy chỉ để áp dụng ghi chú này; tại điểm bàn giao an toàn gần nhất, review diff đang có và bổ sung plan cho phần còn lại, không viết lại code đã kiểm chứng.

### 1. Khảo sát và lập plan, chưa sửa code

Giao một tác vụ nhỏ có mục tiêu và ranh giới file. OpenCode chỉ đọc source/test liên quan; dùng thông tin toolchain đã kiểm chứng thay vì tìm lại trên toàn máy. Implementation plan thường khoảng một trang, phải có:

- Hành vi trước/sau, file/class/hàm cần đổi và trách nhiệm từng phần.
- State thuộc ai, dữ liệu gốc/index dẫn xuất, invariant và tính nguyên tử của command.
- Luồng thành công/reject; ID rỗng hoặc sai, thiếu tài nguyên, actor chết, thiếu người có quyền, stale/duplicate command, xung đột tài nguyên và lifecycle liên quan.
- Tác động tới save/load, binding, action/observation/reward/model; nêu rõ phần không tương thích nếu có.
- Test cần thêm/sửa, regression và lệnh nghiệm thu; điều chưa xác minh và ranh giới chưa làm.

Mẫu giao bước plan: “Chỉ khảo sát và lập implementation plan cho [tác vụ], trong [file/module]. Chưa sửa source hoặc chạy training. Đọc AGENTS.md và contract liên quan; trình ownership, invariant, luồng lỗi/trường hợp biên, tương thích và test. Dừng để Codex review trước khi triển khai.”

### 2. Codex review rồi giao triển khai

Codex kiểm tra plan có tuân spec và bỏ sót hệ quả/edge case không; chỉnh plan nếu cần. Không yêu cầu người dùng duyệt lại các quyết định kỹ thuật thông thường. Chỉ hỏi khi thay gameplay, phạm vi hoặc ngân sách đã duyệt. Ghi lại plan được review trong tài liệu tác vụ hoặc hồ sơ bàn giao đủ để phiên sau tiếp tục.

Mẫu giao bước code: “Triển khai đúng plan [đường dẫn/mục hoặc bản đã review]. Giữ thay đổi có sẵn; không mở rộng sang module ngoài phạm vi. Chạy các test/gate đã định, báo command và kết quả thực tế. Nếu cần đổi contract/kiến trúc đáng kể, trình plan sửa trước khi tiếp tục. Không commit/push/publish hoặc train dài.”

### 3. Nghiệm thu trước tác vụ kế tiếp

OpenCode sửa theo từng lát cắt có thể kiểm tra, chạy test liên quan và regression. Codex review diff và bằng chứng, trả lỗi cụ thể về OpenCode để sửa; chỉ ghi pass sau khi thực chạy. Cập nhật WORK_STATUS với kết quả, giới hạn và bước tiếp theo. Có plan vẫn phải test; không dùng plan hay lời báo hoàn tất của agent thay cho kết quả.

## Điểm tiếp tục hiện tại — 2026-10-15

### Sau review: đã triển khai M4 theo yêu cầu tiếp theo

Người dùng đã yêu cầu tiếp tục thực hiện đến M4. Lát cắt native đã có và được kiểm chứng Windows; xem `docs/reports/M4-evidence.md`, `docs/guides/PLAYER-README-M4.txt` và WORK_STATUS.md mới nhất. M4 dùng controller kiểm thử có nhãn, không chứng nhận RL production. Linux cần đọc kết quả CI của commit mới. Phần yêu cầu chỉ review bên dưới ghi ngữ cảnh trước khi người dùng giao triển khai M4, không phải lệnh dừng triển khai hiện tại. Chưa tự mở M5 hoặc duyệt các luật xã hội đầy đủ.

### OpenCode Windows local: lỗi 403 và cách chạy đã kiểm chứng

Máy local có `C:\Users\Hi\.bun\bin\opencode.exe`, phiên bản `1.18.30`. Ngày 2026-10-15, cả CLI `run` và TUI trả `403 FreeTierError: OpenCode's free tier can only be used from within OpenCode` khi phiên được đặt `OPENCODE_PERMISSION` với `bash: deny`. Đổi sang `bash: ask` (cùng executable, model `opencode/step-5-preview-free` và credential) đã nhận được câu trả lời model; phiên review sau đó đọc được năm tài liệu qua công cụ `read`. Không cần cài lại hoặc đổi credential. Báo cáo upstream khớp triệu chứng: https://github.com/anomalyco/opencode/issues/50627.

Đối với review, dùng quyền yêu cầu xác nhận thay vì loại bỏ công cụ shell khỏi yêu cầu model:

```powershell
$env:OPENCODE_PERMISSION='{"edit":"ask","bash":"ask","task":"deny","webfetch":"ask","websearch":"ask"}'
.\start-opencode.ps1 --prompt "Đọc AGENTS.md, WORK_STATUS.md, docs/design/simulation-complexity-scope.md và docs/design/late-medieval-world-design-discussion.md bằng read tool. Chỉ review thiết kế. Không sửa file, dùng shell, tạo subagent, commit, push, triển khai hoặc training. Không tự giảm số vùng/dân. Trả lời tiếng Việt."
```

Biến trên chỉ áp dụng cho shell hiện tại; không ghi credential hoặc thay cấu hình toàn máy. Khi cần, lưu và phục hồi giá trị `OPENCODE_PERMISSION` trước đó sau phiên review. Quyền `ask` vẫn yêu cầu xét duyệt trước thao tác; không dùng `--auto`. Việc đọc/đánh giá tài liệu không chứng nhận gameplay, model quality hoặc hiệu năng.

**Các mục M0a bên dưới là hướng dẫn lịch sử. Repo hiện có M3 và lát cắt M4; không chạy lại M0 theo các prompt cũ.** Đọc `WORK_STATUS.md`, `docs/design/simulation-complexity-scope.md` và `docs/design/late-medieval-world-design-discussion.md` trước khi giao việc mới. Người dùng yêu cầu làm rõ độ sâu simulation trước khi triển khai thêm; đã chọn bối cảnh cuối Trung cổ. Các đề xuất còn lại chưa tự trở thành spec đã duyệt.

Docs về nền xã hội và ranh giới simulation đã push ở commit `4cb7969`. Cloud Linux đã cài và khởi động OpenCode 1.19.0; lần gọi model `opencode/step-5-preview-free` của cấu hình repo bị chặn với `Forbidden: Domain forbidden`. Chưa có câu trả lời model hoặc kết quả triển khai từ lần thử đó. Không suy ra OpenCode trên Windows local có cùng lỗi.

Chat cloud chỉ chạy lệnh trong `/workspace/PolyEc`, không điều khiển shell Windows của người dùng. Để Codex trực tiếp chạy và tương tác OpenCode trên máy người dùng, mở chat trong checkout PolyEc ở môi trường Local. Nếu checkout chưa có trong danh sách project local, mở/thêm thư mục đó trước. Không cần tạo phiên OpenCode mới chỉ để lấy lại ngữ cảnh; docs trong Git là hồ sơ bàn giao.

Tại checkout trên Windows, sau khi cập nhật Git mà giữ các thay đổi local, có thể mở OpenCode bằng script sẵn có:

```powershell
.\start-opencode.ps1 --prompt "Đọc AGENTS.md, WORK_STATUS.md, docs/design/simulation-complexity-scope.md và docs/design/late-medieval-world-design-discussion.md. Chỉ review thiết kế: nêu ba quyết định còn mở và đề xuất một tác vụ thiết kế tiếp theo kèm tiêu chí nghiệm thu. Không sửa file, chạy triển khai, tạo subagent, commit, push hoặc training. Không bắt đầu lại M0 và không tự giảm số vùng/dân. Trả lời tiếng Việt."
```

Lệnh trên giữ prompt bàn giao ban đầu. Windows local đã chạy script và thử provider ngày 2026-10-15; xem mục lỗi 403 và cấu hình quyền đã kiểm chứng ở đầu tài liệu. Không sao chép credential vào repo hoặc chat.

---

Ngày: 14/10/2026. Công cụ coding agent đã chọn: **OpenCode**.

Đây là tài liệu để bắt đầu triển khai theo từng mốc, chưa phải mã game hoặc báo cáo build thành công. OpenCode chưa được cài/chạy trong môi trường đang dùng để sửa tài liệu. Không có provider, API key hoặc model được cấu hình trong bước này.

## 1. Cách dùng

1. Cài OpenCode trên máy phát triển Windows/Linux theo [tài liệu chính thức](https://opencode.ai/docs/). Có thể dùng WSL cho phần CLI trên Windows; vẫn cần kiểm tra bản app Windows native ở mốc đóng gói.
2. Cấu hình provider/model bằng cơ chế kết nối của OpenCode. Không ghi API key vào prompt, repository hoặc tài liệu.
3. Mở checkout PolyEc. Chép bản kế hoạch v3 vào `docs/design/implementation-plan.md` và tài liệu này vào `docs/handoff/opencode-handoff.md`; việc chép thực hiện khi bắt đầu bước code, không tự ghi đè file đã có.
4. Tạo/ghép quy tắc mục 3 vào `AGENTS.md` của dự án, giữ các quy tắc hiện có nếu có. Không cần tạo Git worktree: cloud task hiện có đã cô lập; chỉ tạo nếu người dùng yêu cầu.
5. Chạy OpenCode tại root repository. Đầu tiên yêu cầu lập phương án cho **M0a** bằng prompt ở mục 5, sau đó giao triển khai theo prompt mục 6.
6. Hoàn tất nghiệm thu một mốc trước khi mở rộng. Nếu phải đổi hợp đồng hoặc phạm vi, cập nhật spec và nêu rõ tác động.

OpenCode là agent điều phối việc đọc/sửa code và chạy công cụ. Khả năng và quota phụ thuộc provider/model đã kết nối; cài OpenCode không đồng nghĩa có inference miễn phí hoặc không giới hạn. Không cần dùng thêm coding agent thứ hai để bắt đầu dự án.

## 2. Tài liệu nền và thứ tự thực hiện

- Kế hoạch nguồn: `PolyEc-implementation-plan-v3-native-rl.md`; dùng bản v3, không dùng plan cũ game web 4–6 vùng.
- Hướng sản phẩm: app desktop Windows/Linux, RAM 16 GB, RL inference bằng CPU.
- Stack đề xuất: Godot native, C++20 OOP, CMake; Python/PyTorch training qua pybind11; ONNX Runtime native.
- Tài liệu giải thích thuật ngữ/gameplay phải viết dễ hiểu và có bảng Hướng dẫn F1 trong game.

M0 của plan được chia thành hai phần để kiểm tra nền trước:

| Phần | Mục tiêu |
|---|---|
| M0a | App native tối thiểu, core C++ OOP, headless test, hai vùng chọn được, clock/pause |
| M0b | Python binding, parity cùng core, ONNX CPU shape/memory test, spike citizen/sparse graph 10k/100k/1m |

M0a chưa đạt toàn bộ M0. M0b không được bỏ đi; chưa có policy đã train trong cả hai phần. Sau M0 mới tiếp tục kinh tế, chính trị/cá thể, RL, vertical slice và quốc gia đầy đủ theo plan.

## 3. Quy tắc dự án để đưa vào AGENTS.md

- Sản phẩm là app game native; không chuyển thành React/Vite/web game hoặc bọc một trang web để thay mục tiêu.
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

## 4. Đặc tả M0a

### Phạm vi

Tạo nền dự án native đủ chứng minh core và app giao tiếp được. Hai vùng thử là Thanh Điền và Hắc Sơn. Map prototype có đường nối và hình vùng dễ chọn; art địa hình đầy đủ, kinh tế và RL thuộc mốc sau.

### Hợp đồng tối thiểu

| Đối tượng | Trách nhiệm |
|---|---|
| EntityId | Định danh ổn định, không dùng tên vùng làm khóa |
| GameDate | Ngày nguyên; lịch giả tưởng 360 ngày/năm |
| Territory | ID, tên, thông tin nền và vị trí; không phụ thuộc Node2D |
| GameWorld | Sở hữu vùng và ngày game; nhận Command và trả Result |
| SimulationController | Sở hữu clock, pause, tốc độ; UI không tự tăng ngày |
| PlayerView | Dữ liệu vùng/ngày đã lọc, chỉ đọc từ phía UI |
| GodotAdapter | Chuyển input thành Command, đưa snapshot lên UI |

Domain advance một ngày phải chạy headless được. Scheduler thực dùng đồng hồ đơn điệu; test dùng fake time. Pause là trạng thái khởi đầu. Resume ở 1× tương đương một ngày game/giây thực; dùng đơn vị ngày nguyên với accumulator, không nhân delta render thẳng vào tuổi hoặc lịch.

### Luồng người thử

Mở app → thấy hai vùng và ngày → click vùng để xem panel → resume cho ngày tăng → pause để ngày đứng → mất focus/thu nhỏ cửa sổ khiến pause → trở lại không chạy bù.

### Tiêu chí nghiệm thu

- C++ core configure/build được bằng CMake và không include Godot trong domain.
- Test headless thực sự kiểm tra advance, pause/resume và ID vùng; không chỉ có test luôn pass.
- Native app nạp được GDExtension và hiện dữ liệu từ core, không phải hai bộ state giả độc lập.
- Picking chọn đúng vùng; UI input không phát một Command hai lần.
- GameDate không thay đổi khi pause; pause/resume không tạo hai scheduler.
- Có README với version/toolchain, lệnh build/test/run đúng với implementation.
- Build Windows/Linux nào chưa chạy được phải báo rõ; không suy rằng pass Linux nghĩa là pass Windows.
- Không có dependency Python/Node trên máy người chơi cho app M0a.

### Chưa làm trong M0a

Kinh tế 16 hàng, 24 vùng, RL training, map art đầy đủ, save campaign dài và một triệu người. Không tạo nút giả cho các chức năng này. Tách rõ prototype hai vùng khỏi cam kết phạm vi bản hoàn chỉnh.

## 5. Prompt giao OpenCode lập phương án M0a

> Đọc AGENTS.md, docs/design/implementation-plan.md và docs/handoff/opencode-handoff.md. Dùng kế hoạch v3 native/OOP/RL, không dùng hướng web cũ. Chỉ chuẩn bị phương án M0a trước khi sửa code: kiểm tra repository và toolchain, đề xuất file/class/interface, Godot/GDExtension version tương thích, CMake build và headless test. Nêu dependency cần cài, điều kiện chưa xác minh và cách nghiệm thu. Không mở rộng sang kinh tế/RL training/triệu người. Không bỏ M0b khỏi roadmap.

## 6. Prompt giao OpenCode triển khai M0a

> Triển khai M0a theo docs/handoff/opencode-handoff.md và phương án vừa chốt. Tạo app Godot native với lõi C++20 OOP độc lập Godot, hai vùng Thanh Điền/Hắc Sơn chọn được, panel lấy dữ liệu từ core, đồng hồ ngày, pause/resume và pause khi mất focus. Tạo CMake/headless tests, chạy build/test phù hợp và thử native app khi có khả năng. Sửa lỗi setup/build trong phạm vi M0a, không thay sản phẩm thành web. Không làm RL hoặc kinh tế đầy đủ. Cuối tác vụ báo file thay đổi, command đã chạy, test count/result, hệ điều hành đã kiểm chứng và blocker còn lại. Không tự commit/push/publish.

## 7. Cách giữ quota và tránh agent làm lại

- Mỗi task có spec nhỏ, ranh giới file và gate. Không đưa lại cả plan dài vào mọi prompt; yêu cầu đọc mục cần thiết.
- Dùng model mạnh cho kiến trúc, lỗi khó và review; tác vụ đã rõ có thể dùng model ít tốn hơn nếu chất lượng đáp ứng. Model nào phụ thuộc provider thực tế, chưa khóa trong tài liệu này.
- Giữ WORK_STATUS.md ngắn: mốc đã đạt, commands/results, vấn đề mở, version đã pin và task tiếp theo. Đây là trạng thái làm việc, không thay test report.
- Cho agent tiếp tục từ artifact và code hiện có; không yêu cầu “viết lại toàn bộ” khi chỉ một module lỗi.
- Đánh giá trên build/test và hành vi thực, không dựa vào số dòng code sinh ra.
- Khi bắt đầu RL, giao riêng environment/reward/parity, rồi training experiment, rồi evaluation/export; không gộp “viết mạng + train + thông minh + triệu NPC” vào một task.

## 8. Trạng thái bàn giao hiện tại

Đã chọn OpenCode và chuẩn bị hướng dẫn/prompt/spec M0a. Chưa cài OpenCode, kết nối provider, tạo source, build app hoặc train model. File kế hoạch v3 vẫn là nguồn scope; tài liệu này là hướng dẫn thực hiện.
