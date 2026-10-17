# Kế hoạch tiếp tục sau đồng bộ 2026-10-17

Cập nhật sau triển khai: lát A đã được OpenCode sửa và Codex nghiệm thu core Windows; CTest độc lập 6/6 pass. Xem [bằng chứng](../reports/copilot-core-repair-2026-10-17.md). Các blocker/source markers bên dưới ghi trạng thái lúc vừa pull để truy nguyên, đã được xử lý trong working tree; chưa commit. Bước tiếp theo là plan-only lát B.

Revision đã kéo: `a5647a5`, nhánh main; fast-forward từ `d5751bb`, working tree sạch trước/sau pull. Codex chỉ làm rõ kế hoạch và review; OpenCode triển khai chính. Không commit/push, training hoặc mua tài nguyên trong tác vụ này.

Người dùng xác nhận refactor core trong commit này do GitHub Copilot thực hiện. Tác vụ tiếp tục dùng OpenCode sửa code, giữ ý định refactor và review từng thay đổi; không mặc định hoàn tác refactor về bản trước.

## Nguồn điều hành và trạng thái thật

Theo AGENTS.md và [quy trình hiện hành](opencode-handoff.md), mỗi lát phải qua plan-only → Codex review → OpenCode code/test → Codex review diff/bằng chứng. [Quyết định 17/10](../design/M5-simulation-design-decisions.md) và [change map](M5-simulation-change-map.md) thay phần thiết kế cũ mâu thuẫn; không bỏ mục tiêu M5.1–M5.5, M6 hoặc RL production M7.

Không có activity.hpp/activity.cpp/test_activity.cpp trong checkout này. Mô tả activity local trong change map không chứng nhận chúng đã có trên main. Kết quả Windows 5/5 trong báo cáo trước là lịch sử, không chứng nhận revision hiện tại.

Source hiện có nguyên văn patch markers trong core/CMakeLists.txt, core/include/polyec/realm.hpp và core/src/realm.cpp. Đây là blocker trước gameplay. Lệnh configure đã chạy:

```powershell
& build/tooling/cmake/data/bin/cmake.exe -S . -B build/review-20261017 -DPOLYEC_BUILD_APP=OFF -DPOLYEC_BUILD_AI=OFF -DPOLYEC_BUILD_PYTHON=OFF
```

Kết quả exit 1: generator mặc định NMake thiếu nmake, chưa chọn được compiler; chưa tới bước parse core hay compile. Không dùng thư mục configure này làm bằng chứng test. Tài liệu refactor nói chưa chạy tests.

## Thứ tự giao OpenCode

| Lát | Phạm vi và kết quả | Gate trước lát sau |
|---|---|---|
| A — sửa tính toàn vẹn bản pull | Plan-only đối chiếu diff d5751bb..a5647a5 trong core/CMakeLists.txt, realm.hpp/realm.cpp, sim.hpp/sim.cpp, types.hpp, command.hpp và managers mới; chỉ mở rộng file nếu chứng minh phụ thuộc. Xác định nội dung source bị thay bằng patch, API bị mất, mutable access phá ownership và snapshot/restore có thực sự nguyên tử. Đề xuất phục hồi có chọn lọc, không reset toàn repo hoặc bỏ refactor âm thầm. | Codex duyệt plan; OpenCode sửa rồi configure/build sạch bằng toolchain Windows đã ghi nhận, core CTest đầy đủ. Review diff so với trước pull; reject không thay state, finance bảo toàn, household index đúng và save roundtrip. Native/Python/runtime chạy riêng khi API bị ảnh hưởng. Chưa pass thì dừng mở gameplay. |
| B — contract thời gian và timeline | Sau A, khảo sát giới hạn clock/command/save và nộp plan cho một người có nhiều phiên trong ngày. Chốt đơn vị, ownership, requested/delivered/unmet, priority do actor cấp, tie chưa giải quyết, command ID và schema. Không mặc định module activity local tồn tại. | Codex duyệt contract trước code. Tests horizon splitting, qua ngày, pause/focus không chạy bù, command không liên quan giữ job, overcommit hợp lệ, reject nguyên tử, duplicate/stale, save/replay. Chưa sinh lương/care/health từ attention credit. |
| C — thực hiện và settlement | Một nghiệp vụ cụ thể nối presence/resource/progress/output thật, partial/shortfall và ledger; xác định cặp compatible lẫn exclusive. Allocation có provenance, không ưu tiên ID hoặc tối ưu hộ NPC. | Không double-credit giờ/tiền/hàng; nguồn lực bảo toàn; cạnh tranh tài nguyên, settlement lặp, save giữa tiến độ và reorder có test. |
| D — cơ thể và care | Plan công thức ngủ/exertion/phục hồi, units và đặc điểm cá nhân trước; sau đó care thực và thiếu dịch vụ. Tham số chưa duyệt không tự thành luật gameplay. | Nghỉ thực mới hồi phục; không chặn nhận cam kết chỉ vì quá tải; không tự chọn người chăm hoặc chuyển hộ. Thiếu care có nguyên nhân và hậu quả đã định nghĩa. |
| E — xuyên core/native/Python | Nối view, observation, reward outcome và guard model theo từng lát bị ảnh hưởng, không dồn compatibility tới cuối. Tiếp tục hộ/tài sản/lifecycle, cơ sở/tổ chức và quốc gia theo M5. | Parity cùng C++ core, version semantics/shape, bundle cũ bị từ chối rõ khi không tương thích; baseline có nhãn. Profile B, Linux/package và training là gate riêng, không suy pass từ core Windows. |

Trong mọi plan: nêu file/class/hàm, state gốc/index, invariant, success/reject/accepted-failed, edge cases, tác động save/binding/action/observation/reward/model và lệnh nghiệm thu thực. Không giao cả bảng làm một task code.

## Quỹ hộ và tài liệu cũ

[Plan quỹ hộ](../design/M5-1a-household-accounts-plan.md) chưa được duyệt code. Review thật ở [M5.1a review](../design/M5-1a-household-accounts-review.md), không phải M5-1a-handoff.md. Khi quay lại quỹ hộ, cập nhật plan theo contract timeline mới; yêu cầu review cũ về chặn accept do xung đột thời gian đã bị thiết kế overcommit 17/10 thay thế. Giữ consent, authority, reservation, bảo toàn settlement và model guard. Không coi nhãn APPROVED lịch sử là cho phép áp lại cơ chế cũ.

## Prompt tác vụ đầu tiên

> Chỉ khảo sát và lập implementation plan cho lát A trong tài liệu này, chưa sửa source. Đọc AGENTS.md, quy trình hiện hành và diff d5751bb..a5647a5 trong phạm vi đã nêu. Xác minh patch markers/source bị mất; đề xuất khôi phục source hợp lệ và kiểm chứng refactor mà giữ ownership, API và reject nguyên tử. Dùng thông tin toolchain đã có, không rà cả máy hoặc repo. Nêu file/hàm, success/reject/edge cases, save/binding/model impact, tests và lệnh Windows với generator/compiler rõ ràng. Không triển khai gameplay mới, train, commit/push hoặc reset. Dừng để Codex review plan.
