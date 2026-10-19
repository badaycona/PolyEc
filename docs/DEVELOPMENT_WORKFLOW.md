# PolyEc — quy trình phát triển cho Codex và OpenCode

Tài liệu này giải thích cách biến một phần thiết kế thành thay đổi đã kiểm chứng. [AGENTS](../AGENTS.md) quy định quyền thực thi; [CURRENT_TASK](CURRENT_TASK.md) là nguồn duy nhất về lát đang giao, trạng thái và next gate. Tài liệu này không mở một tác vụ mới hay cấp quyền commit/push/train.

## 1. Đọc đúng thứ trước khi làm

1. Đọc AGENTS, rồi CURRENT_TASK; kiểm branch/revision và `git status --short`.
2. Xác định file của tác vụ khác phải giữ, supporting plan nào active, gate nào chưa đóng. Không mở lượt code cạnh tranh cùng file.
3. Đọc đúng phần [plan sản phẩm](IMPLEMENTATION_PLAN.md) liên quan. Muốn hiểu phương pháp dùng [cây bài toán](TECHNICAL_PROBLEMS.md) và [paper](DEVELOPER_GUIDE.md).
4. Mở source/tests liên quan để xác nhận behavior thực. Report cũ không chứng nhận HEAD.
5. Chỉ mở archive khi cần truy nguyên một quyết định cụ thể; chữ APPROVED trong archive không là quyền triển khai hiện tại.

## 2. Phân vai và vòng công việc

| Bước | Người thực hiện | Đầu ra/gate |
|---|---|---|
| Chọn lát | Codex phối hợp người dùng | Before → after hẹp, phạm vi file và điều kiện nghiệm thu |
| Khảo sát/plan-only | OpenCode | File/class/functions, ownership/invariants, flows, edge cases, compatibility và test commands; chưa sửa code |
| Review plan | Codex | Approved có điều kiện rõ hoặc yêu cầu sửa plan; không suy im lặng là duyệt |
| Triển khai/test | OpenCode | Diff trong phạm vi, test thực và evidence; thay contract lớn phải quay lại review |
| Review kết quả | Codex | Đối chiếu spec/diff/oracle, kiểm bằng chứng, chạy kiểm liên quan khi cần |
| Bàn giao | Codex/OpenCode theo phần được giao | CURRENT_TASK cập nhật; history ghi quyết định, report giữ số liệu; next action duy nhất |

Codex không tự thay OpenCode viết gameplay trong tác vụ đã giao. Công việc biên tập docs theo yêu cầu người dùng không là một lượt coding gameplay. Chỉ hỏi người dùng khi thay gameplay, scope hoặc ngân sách; các quyết định kỹ thuật đã được contract duyệt có thể xử lý trong lát.

## 3. Một lát đủ nhỏ trông như thế nào?

Ví dụ về ranh giới: nối pause/game time vào controller và chứng minh không catch-up; không đồng thời đổi reward, body và payroll. Một prompt “hoàn thành M5” không có ranh giới review hữu ích.

Mẫu plan chuẩn nằm ở CURRENT_TASK để tránh hai mẫu cạnh tranh. Khi điền phải trả lời:

- Ai sở hữu state; index nào dẫn xuất; đơn vị là gì?
- Success, reject không đổi state và nỗ lực hợp lệ thất bại khác nhau thế nào?
- ID/quyền/consent, chết, shortage, stale/duplicate, concurrency và overflow xử lý ra sao?
- Save/Campaign/Python/native/Godot/action/observation/reward/model bị ảnh hưởng gì?
- Test nào quyết định đúng/sai, lệnh nào chạy ở platform nào, phần nào chưa kiểm?

## 4. Evidence đủ để nghiệm thu

Report theo revision/run cần ghi baseline và dirty scope, supporting plan, diff liên quan, lệnh thực thi, exit code, test counts hoặc workload, platform/toolchain, kết quả và giới hạn. Không ghi một lệnh dự định chạy như đã pass.

| Loại thay đổi | Kiểm tối thiểu theo tác động |
|---|---|
| Chỉ docs | Link/path, headings/fences/công thức, coverage và consistency; không chạy CTest gameplay chỉ để tăng số check |
| Core transition | Success/reject/edge cases có oracle, invariant, save/replay và regression liên quan |
| Clock/adapter/binding | Units/pause/focus/restore, cùng transition qua bề mặt bị tác động |
| Action/obs/reward/model | Schema/semantic guards, masks/memory/reset, parity; quality test đúng phạm vi |
| UI/package | Hành vi trên app, quyền biết, input/focus/save và offline package theo platform |
| Performance | Workload/counts, full-cycle latency/throughput/RAM/backlog; không dùng constructor count làm scale evidence |

Test Release phải còn hoạt động; không dùng `assert` bị NDEBUG loại bỏ. Reject test phải chạm validator định kiểm. Kết quả build, model train, policy quality và profile benchmark là bốn kết luận riêng.

## 5. Ghi nhận quyết định và sửa sai

Thay scope/gameplay ghi vào IMPLEMENTATION_PLAN sau khi được chốt. Phương pháp/công thức ghi vào tài liệu kỹ thuật với đơn vị, giả thiết và giới hạn. Một supporting contract trong `docs/design/` chỉ active khi CURRENT_TASK dẫn đúng phiên bản và review status.

Lỗi phát hiện trong review cần ghi điều kiện gây lỗi → invariant/behavior bị vi phạm → cách sửa → test chặn tái diễn → phần còn mở. Nếu bằng chứng cũ sai, thêm đính chính có nguồn; không sửa checkpoint/source lock để giả tương thích, không xóa failure khỏi lịch sử.

## 6. Safe handoff

Kiểm lại dirty state trước bàn giao, giữ thay đổi tác vụ khác. CURRENT_TASK ghi acceptance thực, gate còn mở và next action; DEVELOPMENT_HISTORY ghi mốc/bug/giải pháp; `docs/reports/` giữ số liệu chi tiết. README chỉ tóm tắt giai đoạn lớn, không copy bảng trạng thái từng lát.

Không commit/push/publish, huấn luyện dài hoặc chi tiền cloud khi chưa được yêu cầu tương ứng. Không bỏ test, giảm scope hoặc tắt dependency/TLS checks để qua gate. Lệnh build phải dùng cấu hình phù hợp lát; core-only flags không chứng nhận app/native/Python.
