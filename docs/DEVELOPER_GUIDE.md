# PolyEc — mô hình bài toán, phương pháp và hệ thống thực nghiệm

Cập nhật đối chiếu: 2026-10-18. Tài liệu dành cho developer đọc để hiểu lý do thiết kế, implementation, phép đo và giới hạn. Scope sản phẩm thuộc [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md).

## Tóm tắt cập nhật (hợp nhất từ archive)
- Thêm mục "Household & Accounting": hợp đồng nghiệp vụ, escrow, inheritance, guardian, quỹ hộ và rules idempotence cho settlement.
- Thêm tóm tắt thiết kế M5 (D1–D8): attention/presence/scheduling, transaction semantics, RL policy constraints.
- Thêm phần Scaling & Performance: memory, throughput, event-driven update, bottleneck reduction.
- Thêm checklist test và parity harness cho ONNX/C++/Python/native.

Đọc đường: phần 1–3 là bài toán/domain/hạch toán; phần 4–5 giải thời gian, attention, settlement, thông tin và logistics; phần 6 giải mạng và training; phần 7 là scaling/benchmark.

---

## 1. Đặt vấn đề và tiêu chí thành công

Xét vùng mỏ thiếu lương thực khi tuyến đèo bị tranh quyền và khoản nợ sắp tới hạn. Cấp hàng chỉ có ý nghĩa khi biết hàng thuộc ai, ai được xuất kho, ai hưởng lợi, ai chịu trách nhiệm. Đó là trọng tâm của world model PolyEc.

Đầu vào gồm geography, dân số, tài sản, quyền/nghĩa vụ, luật, seed, command và bước tiến game time. Đầu ra gồm state mới, kết quả hành động, nghĩa vụ còn lại, evidence và log fact. Nhiệm vụ không phải chỉ “chọn hành động có score cao”, mà là tạo một hệ thống trong đó luật, nguồn lực, quan hệ và thông tin là đủ đáng tin và có thể kiểm chứng.

Đánh giá thành công có ba lớp:
- Luật đúng: state hợp lệ, invariant, hạch toán đúng, reject không làm hỏng world.
- Policy có ích: lựa chọn hành vi có thể gia tăng utility và thích ứng với hoàn cảnh.
- Hệ thống chạy đủ nhanh: throughput, save, sync, AI inference và UI không phá luật.

---

## 2. Mô hình thế giới, ownership và household accounting

Phần này hợp nhất các hợp đồng nghiệp vụ quan trọng về hộ và hạch toán từ archive. Là source-of-truth cho các quy tắc tài chính/hộ trong engine.

### 2.1 Household (hộ) — khái niệm và invariants
- Household là tập ID người có quan hệ cư trú/kinship với membership list và một `household_fund` (có thể bằng 0).
- Membership có role: member, dependent, guardian. Guardian có quyền đại diện hạn chế (chi tiêu escrow, consent) theo contract.
- Thay đổi membership phải update index dẫn xuất `households_` (không nhân bản). Chuyển member giữa household phải preserve provenance (ai bắt đầu, khi nào).

### 2.2 Household fund & payroll
- Household fund là tài khoản riêng; transfer giữa household và member tuân theo bút toán đối ứng: debit/credit.
- Payroll từ employer: employer debit -> worker credit. Khi ngân sách không đủ, phân bổ theo tỷ lệ đã mô tả (floor-based rounding), phần dư lưu ở residual để tránh mất tiền/thuế do làm tròn.

### 2.3 Escrow, offers và settlement idempotence
- Escrow: giữ tiền/asset tạm cho hợp đồng; bên gửi không thể tiêu khoản đã escrow.
- Offer/Consignment: tạo manifest, escrow và phân bổ tạm. Khi settlement commit, manifest -> debit/credit và manifest marked settled.
- Idempotence: mỗi settlement có identity `k`; Settle_k(Settle_k(S)) = Settle_k(S). Implement bằng counter/nonce chống lặp.

### 2.4 Inheritance & estate
- Khi một person dies: create estate case, list liabilities and assets. Priority: settle secured obligations -> payroll obligations due -> taxes -> heirs. Assets distributed by legal order; unresolved claims create pending cases.
- Di sản không tự động trả nợ bên ngoài estate; guardian cannot appropriate estate except by legal flow.

### 2.5 Contracts & checks
- All account transitions must maintain M_t = M_0 + E_t for the chosen scope; net creation/destruction only via declared endowment events.
- Unit tests: invariants for sums, manifest reconciliation, escrow semantics, payroll rounding behavior, idempotence on repeated settlement.

---

## 3. Command, transaction và bất biến

### 3.1 Validation, chuẩn bị và commit
Luồng xử lý: nhận actor/tư cách/version → validate ID/quyền/consent → kiểm tài nguyên và điều kiện của bước đang thực hiện → chuẩn bị thay đổi → commit/rollback. Reject phải là no-op đối với state, không được tiêu RNG hoặc đổi cursor không báo cáo.

### 3.2 Định luật đối soát và phạm vi tài khoản
Gọi A là tập tài khoản thực (cá nhân, cơ sở, ngân khố, escrow nếu dùng). Với nguồn/phần tiêu hủy E_t và đích Z_t, tổng tiền theo scope: M_t = sum_{a∈A} b_{a,t}; M_{t+1} - M_t = E_t - Z_t.

Chuyển nội bộ x từ u sang v phải có Δb_u = -x, Δb_v = x. Nợ là quyền/nghĩa vụ, không là một tài khoản tiền mới; escrow giữ tiền tạm cho giao dịch chưa kết thúc.

Với hàng hóa g: I_g(τ) + C_g(τ) = I_g(0) + P_g(τ), nơi I là tồn kho, C là tiêu dùng, P là sản xuất. Hệ kín này chỉ đúng khi phạm vi được khai báo đầy đủ. Không làm ngầm “sửa tổng tồn kho” để che thiếu nguồn.

Diễn giải bằng lời:
```text
Tổng tiền sau = tổng tiền trước + nguồn tiền khai báo − đích tiền khai báo
Kho sau = kho trước + sản xuất + nhập/chuyển vào − tiêu dùng/đầu vào − xuất/chuyển ra
```

### 3.3 Tiền: chuyển giao thay vì tạo/xóa ngầm
- Nếu triều đình chi 300 đồng làm đường thì phải biết tài khoản nào trả và tài khoản nào nhận.
- Gọi b_u, b_v là số dư trước; Δ là số chuyển: b'_u = b_u - Δ; b'_v = b_v + Δ.
- Điều kiện kiểm trước: Δ ≥ 0, đủ tiền / đủ hạn mức quyền.

### 3.4 Hàng: tính cả lượng đang đi đường
- Xuất kho 100 đơn vị không làm 100 đơn vị đó lập tức có mặt ở đích hoặc biến mất.
- Hàng trong cargo, shipment, inventory, sale và production đều thuộc cùng hệ kín; không xóa khỏi một phần mà không ghi nhận đích hợp lệ.

### 3.5 Sản xuất: đầu vào hữu hạn và lao động
Mỗi recipe có định mức đầu vào/đầu ra; số mẻ thực hiện bị giới hạn bởi điều kiện thiếu nhất: vật tư, lao động, tiền và công suất.

### 3.6 Giá: phản ứng đơn giản với mức tồn kho
Hoàn chỉnh với formula heuristic, chỉ là heuristic tham số hóa, không phải mô hình kinh tế tổng quát.

### 3.7 Lương thiếu ngân sách và thuế làm tròn
- Với lương, dùng floor-based proportional split.
- Với thuế, dùng carry-over remainder để tránh mất tiền do làm tròn lặp.

### 3.8 Nợ và escrow
Nợ là nghĩa vụ, không là tiền. Khi vay và trả, balance and debt phải tách rõ. Escrow giữ tiền chờ settlement/offer.

---

## 4. Thời gian, hoạt động và cơ thể

### 4.1 Evaluator thuần đã có
`polyec::activity::evaluateActivity(actor_id, Horizon{begin,end}, sessions)` nhận immutable views của một actor, với half-open intervals tuyệt đối. Tick trừu tượng, không cố định là phút.

Các đối tượng cần tách rõ: requested demand, delivered attention, unmet, output, wage, body load. Capacity và tie policy phải có mô tả rõ.

### 4.2 Mô hình mới: attention, presence, effort, body
Thời gian và sản lượng không thể chỉ là “busy_until”. Thiết kế đích tách:
- sleep pressure / debt
- exertion
- health
- actual rest / effort
- output / attendance / wage

Body update cần có state transition `B_i(τ + Δτ) = f_B(...)` nhưng chưa có hệ số hiệu chuẩn cuối cùng.

### 4.3 M5 design decisions (hợp nhất từ archive)
- D1: session theo ngày, tách requested demand và delivered attention.
- D2: body state tách khỏi stat cứng; body dynamics to be calibrated.
- D3: routine/schedule là kế hoạch, không phải ràng buộc tuyệt đối.
- D4: concurrency model: requested, delivered, output phải được tách riêng và có evidence.
- D5: ledger là source-of-truth cho tài sản; không tạo tiền/hàng ngầm.
- D6: sparse typed relations cho membership/ownership.
- D7: RL design phải có shared weights + per-agent memory + masked observation.
- D8: pipeline world: command → schedule → execution → ledger/evidence → view/save → evaluation.

---

## 5. Từ facts đến thông tin và trồi sinh

World tạo fact có ID, participants, location/time và parent cause. KnowledgeRecord xác định ai nhận được fact khi nào. Case/obligation tổng hợp các facts đang liên quan, giúp phát hiện causal chain.

Một chuỗi thử: chủ xưởng chết → di sản còn nợ → thiếu nguồn trả lương → người làm có thể tìm việc/đòi quyền → guild hoặc chính quyền nhận hồ sơ → thay đổi quyền lực. Đây là causal chain, không phải script campaign.

### 5.1 Logistics: đưa hệ quả qua không gian và thời gian
M4 có graph các tuyến, mỗi cạnh có thời gian, công suất, slot và trạng thái. Dijkstra-like routing theo thời điểm đến sớm nhất; manifest và đối soát phải giữ lịch di chuyển và lưu lượng thực tế.

---

## 6. Bài toán học hành vi và RL

### 6.1 Policy, actor, critic và GRU
Actor chọn action; critic ước lượng value. Mạng pilot hiện có encoder 128 → GRU64 → candidate scorer; candidate hiện là 32 hành động với 12 feature/candidate.

### 6.2 Các họ policy
Runtime sandbox có ba họ: Governance, Economic, Civilian. Một bộ trọng số chung cho mỗi họ; từng actor có hidden state và sampling counter riêng.

### 6.3 Reward, PPO, GAE, BPTT
- Reward không dùng “giá trị tiền” thuần túy; cần đo utility theo actor.
- Optimizer: PPO with GAE, clipped objective, entropy regularization, value loss.
- BPTT truncated theo rollout window; không khẳng định ai đó chỉ nhớ 16 quyết định.

### 6.4 Ngoại lệ và kiểm định cá nhân hóa
- Có thể condition policy theo mục tiêu/tính cách/năng lực/body, nhưng phải có paired intervention và ablation để chứng minh.
- Đảm bảo reward, observation, và model bundle versioned; không so sánh checkpoint từ hai contract khác nhau như nhau.

---

## 7. Scaling & Performance (trích từ archive)

### 7.1 Memory and hidden state
Với N cá nhân, hidden size H=64, float32, memory policy cần M_h = 4NH bytes. Ví dụ N = 1.000.000 => ~256 MB cho hidden state riêng, chưa tính world/graph/history.

### 7.2 Decisions per second
Một actor quyết định mỗi c ngày, với v ngày game/giây thực: Λ = N v / c decisions/s.

### 7.3 Critical path & cycle decomposition
T_cycle = T_due + T_retrieve + T_obs + T_infer + T_validate/commit + T_processes + T_view + T_sync.
Không cộng các thời gian chồng lấp như thể tuần tự; cần đo critical path riêng.

### 7.4 Engineering guidance
- Sparse storage O(N+E) thay vì dense O(N²).
- Event queue heap O(log Q) cho time-ordered tasks.
- Bounded batch ONNX inference; do profiling before scaling.
- Reduce queries, compact hot/cold data, index event queue và chỉ parallelize phần độc lập.

### 7.5 Target and gates
- Đo p50/p95/p99 cho operations chính.
- Không thay đổi luật để “giải” perf; performance phải là support layer cho engine, không làm sai luật.

---

## 8. Tests & parity harness
- Unit/integration: invariant, numerical fixtures, wrong ID/authority/consent, stale/duplicate, shortage and atomic reject.
- Replay/parity: same command/seed/input across C++/Python/ONNX/native; tolerance must be declared.
- ONNX parity: compare Torch outputs -> exported ONNX -> native ORT results; same masks, same memory semantics.
- Save/reload: check snapshot compatibility, malformed load rejection, semantic guards.

---

## 9. Cách đọc/archive/nguồn chuẩn
- Nguồn chuẩn: `IMPLEMENTATION_PLAN.md`, `DEVELOPER_GUIDE.md`, `CURRENT_TASK.md`, `DEVELOPMENT_HISTORY.md`, `PLAYER_GUIDE.md`.
- Archive snapshot: `docs/archive_snapshots/2026-10-18/README.md`.
- Evidence: `reports/` and `artifacts/`.
- Không dùng archive cũ làm source of truth khi có xung đột với bộ docs chính.

---

## 10. Vị trí code & build
- `core/include/polyec`, `core/src`, `core/tests`
- `bindings/python/realm_binding.cpp`
- `ai/training`, `ai/evaluation`, `ai/tests`
- `native/realm_runtime.cpp`, `native/policy.cpp`
- `gdext/polyec_session.cpp`
- `godot_project/ui`, `godot_project/tests`
- `scripts/`, `.github/workflows/`

---

## 11. Ghi chú cuối
- Một số phần như `f_B` body dynamics, household fund / inheritance details, và hệ số khảo sát triển khai vẫn là “contract cần hiệu chuẩn”, không phải kết luận chất lượng final.
- Mọi thay đổi lớn cần cập nhật `CURRENT_TASK.md` và `DEVELOPMENT_HISTORY.md`, đồng thời lưu evidence trong `reports/`.
- Tài liệu archive có giá trị lịch sử, không được dùng làm nguồn quyết định cho state hiện tại.

Tôi đã hợp nhất phần quan trọng từ archive vào DEVELOPER_GUIDE theo dạng tóm tắt nhưng đủ để maintain engineering decisions và bug rationale. Nếu bạn muốn, tiếp bước cuối cùng là move toàn bộ archive cũ vào `docs/archive_snapshots/2026-10-18/` và commit patch cuối cùng.