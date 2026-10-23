# Core/M5.5 — tái dùng candidate trong decision snapshot

Ngày 2026-10-21. Baseline `dff2781a41ae52a4bd476b4b1460ad0f09ef5dc3`; bước tối ưu được chọn sau đo native profile B-population-scale-only: Runtime dựng candidate trong `observation`, dựng lại khi tạo `seen`, rồi dựng lại lần nữa trong `mask` của revalidation. Benchmark phase instrumented 100k/31 ngày ghi `observation_pack` là 1,75–2,34 s/run và `action_revalidate_commit` 1,04–1,26 s/run; ONNX inference và toàn phần còn lại vẫn lớn. Đây là chẩn đoán một lần mỗi cấu hình, không phải latency acceptance.

## Contract giữ nguyên

- `Realm::observation(actor)` và `Realm::mask(actor)` hiện hữu giữ nguyên signature/hành vi.
- Thêm đường snapshot cho Runtime để một candidate array cụ thể được dùng đồng thời khi encode observation mask và giữ lại làm candidate đã quan sát.
- Revalidation vẫn dựng candidate/mask mới sau mỗi action trước commit; chỉ dùng lại candidate array vừa tạo để không gọi `candidates()` lần thứ hai bên trong `mask()`.
- Candidate order/terms, feature bytes, mask, stochastic action selection, RNG, event order, C++/Python parity và transaction behavior phải giữ nguyên.
- Các vector staging được reserve theo batch trước khi nạp observation. Không cache mask qua mutation hoặc bỏ revalidation.

## Files và gate

- `core/include/polyec/realm.hpp`, `core/src/realm.cpp`: overloads nhận candidate output/input và giữ wrappers hiện hành.
- `native/realm_runtime.cpp`: Runtime lấy candidate qua overload observation, dùng lại khi revalidate; reserve `seen`/mask batch.
- `core/tests/test_decision_snapshot.cpp`, `core/CMakeLists.txt`: đối chiếu byte observation, toàn candidate array và mask với hai overload cũ cho governance/economic/civilian, trước/sau lệnh có mutation.
- Release/Debug full CTest, Python binding parity/replay, native v7 phase benchmark lặp cùng 100k/128 và 100k/512 workload. So sánh phải ghi rõ variation; không tuyên bố đạt 80 ms chỉ từ snapshot optimization.

Self-review. Đây là tối ưu đường decision snapshot, không thay model/semantics và không đóng M5.5.
