# PolyEc — M3 native sandbox và bằng chứng RL v6

**Bản presentable để chạy và kiểm tra cơ chế. Full R1/R2/R3 chưa được chứng nhận.**
Mốc M3 trong plan cho phép gate đạt hoặc lỗi được chẩn đoán; tài liệu này phân
biệt rõ triển khai, huấn luyện, kiểm chứng và chất lượng chiến lược. Chưa phải
campaign hoàn chỉnh hay xác nhận hoàn tất mọi chi tiết M1/M2.

## Chạy bản demo

Giải nén toàn bộ Windows ZIP, mở PolyEc.exe → “Quốc gia · Sandbox”. Chọn một thẻ
mở sổ, rồi chạy/tiến một ngày. Space chạy/dừng, Esc về địa đồ, F1 hướng dẫn.
Rời cửa sổ sẽ tạm dừng. Đổi ID xem cá thể và lệnh hợp lệ; đây là góc nhìn sandbox
inspect, chưa phải UI bảo mật của campaign. “RL · Phòng thử nghiệm” giữ merchant
v2 trước đó. Offline; người chơi không cần Python/Godot/credentials.
Windows x64 cần Microsoft VC++ v14 runtime (đã có trên máy kiểm tra); DLL import
của ORT phụ thuộc MSVCP140/MSVCP140_1/VCRUNTIME140_1.
[Trang runtime chính thức của Microsoft](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist/).

## Đã triển khai

C++20 chung cho pybind11 training và Godot native: sáu lãnh địa + đất triều đình,
bảy hàng, thị trường, mùa vụ, nghề nghiệp và giáo viên có nguồn lương, học tập,
hộ gia đình/guardian budget, quỹ tổ chức thưa, vận tải/đóng tuyến, thuế/cứu trợ,
nợ/default, dự án có vật tư, quyền địa phương, facts/điều tra và báo cáo trễ.
10.000 người ban đầu gồm128 actor chủ chốt +9.872 cư dân. ID, vòng đời,
GRU memory và sampling counters riêng; batch không gộp danh tính.

App suy luận ba ONNX PPO actor trên CPU, không fallback heuristic. Bổ nhiệm là
người có sẵn tự nộp ứng tuyển ở mở sổ; có lương, có thể từ chức, giữ tài sản/ID/
gia đình. Đổi họ policy reset memory, không reset cá thể. Quyết định AI/UI đi
cùng validator. Save giữ world, RNG, counters, model identity và memory; hỏng
checksum/sai bundle bị từ chối mà không thay một phần world.

## Kiểm chứng thực thi

- CTest: 5/5 native AI,4/4 core standalone (Windows/Linux CI); core70, trade36.546, realm24.980, court168 checks.
- Replay C++/Python64 ngày:258 checks; batch sai action và bundle mismatch atomic.
- Bộ huấn luyện476 checks:1/4 worker đồng nhất, seed reset theo thứ tự, terminal
  freeze, tổng reward bằng normalized score và GRU không rò qua episode boundary.
- Godot native/UI/merchant/realm: 19/19 native,18/18 UI,336/336 merchant,411/411 realm.
- Native runtime37checks: đổi họ/reset memory/replay, privacy hồ sơ đề nghị,
  chặn nhận điều khoản chưa quan sát trong batch và chặn lệnh UI từ snapshot cũ.
- Realm64 ngày khớp executable C++ độc lập theo fingerprint, quyết định, tiền,
  vòng đời; save/restore replay và focus/sleep pause đều được smoke kiểm tra.
  Fixture được sinh bằng executable của từng platform; chưa tuyên bố replay
  bit-identical giữa Windows và Linux.
- Torch/Python ORT/native C++ ORT: 489 checks, batch1/3/32/64,
  sai số logits tối đa3.81e-06, memory2.98e-07; invalid probability0.
- Windows portable/extracted checks: Windows staged và ZIP giải nén đường dẫn có dấu/khoảng trắng:19/18/336/411 đều pass.
- Linux/MSVC: Core GCC/Linux +MSVC/Windows [CI38585539729](https://github.com/badaycona/PolyEc/actions/runs/38585539729), native Linux5/5 CTest +source/packaged19/336/411 +Xvfb render [CI38585540079](https://github.com/badaycona/PolyEc/actions/runs/38585540079); revision9e76542.

## Protocol và kết quả học

Ba seed/họ, warm start mỗi seed từ đúng checkpoint v3 tương ứng, critic reset,
không imitation. Curriculum1/2/6 vùng, PPO recurrent GRU64, rollout64/BPTT16,
4 worker core và2 Torch threads. V6 thực hiện 2,625,536
quyết định, 2023.7s recorded fit time.
Có config/checkpoint/initial/history/SOURCE-LOCK/SELECTION-LOCK/manifest để đối chiếu.
Validation14000–14031 chọn model; argmax test160000–160099. Audit categorical
bên dưới dùng100 world mới170000–170099 với common sampling uniforms.
“Untrained” là checkpoint trước mọi PPO (realm-v1), không phải warm start v3.

Đơn vị: (utility cuối − utility đầu) / max(1, abs(utility đầu)); không phải tiền.
Horizon128 quyết định tháng cho governance/civilian,512 tuần cho economic;
chết kết thúc trajectory. Margin noninferiority0.05; paired bootstrap95% CI.
Các họ có objective khác nhau, không so hàng với nhau.

| Họ | Learned sampled | So untrained,95% CI | So baseline,95% CI |
|---|---:|---:|---:|
| governance | -0.047 | -0.053 [-0.104, -0.005] | -0.091 [-0.141, -0.046] |
| economic | -0.096 | +0.722 [+0.503, +0.942] | -2.194 [-2.419, -1.956] |
| civilian | 1.401 | +2.148 [+1.905, +2.383] | +0.004 [-0.275, +0.259] |

- governance: cải thiện so untrained = **False**; baseline noninferiority = **False**.
- economic: cải thiện so untrained = **True**; baseline noninferiority = **False**.
- civilian: cải thiện so untrained = **True**; baseline noninferiority = **False**.

| Họ | Sống cuối horizon | Health TB | Hold | Default / nghĩa vụ đến hạn |
|---|---:|---:|---:|---:|
| governance | 89.0% | 84.2 | 21.5% | 576 / 622 |
| economic | 94.0% | 84.7 | 55.6% | 564 / 672 |
| civilian | 93.0% | 92.8 | 2.3% | 0 / 0 |

Chẩn đoán chất lượng: checkpoint được chọn bằng argmax, trong khi app dùng
categorical; protocol đó chưa tối ưu đúng chế độ triển khai. Kết quả argmax và
sampling dùng hai tập seed khác nhau, nên không coi chênh lệch là phân rã nhân
quả riêng của sampling. Utility hiện tính dư nợ nhưng chưa có chi phí riêng cho
vi phạm deadline; tỷ lệ default cao ở bảng trên cho thấy nghĩa vụ chưa được
học tốt. Đây là lỗi chất lượng thực, không chỉ thiếu phép đo. Goal-only response
rất yếu được ghi dưới R3. Những điểm này được chẩn đoán, không chỉnh model sau
khi xem holdout.

Các phép thử này chưa đo đầy đủ chất lượng thương lượng có điều kiện, pháp lý
phá sản và cân bằng chính trị. Default/insolvency là chỉ số chẩn đoán, chưa có
thủ tục bankruptcy đầy đủ. **full_R1_pass=False** bất kể kiểm tra hẹp nào đạt.

## Tám world co-play dài hạn, giữ nguyên các kết quả xấu

V3 kết thúc0 người sống ở cả8 lần20năm, dù hạch toán đúng. Đã giữ số liệu
artifacts/models/realm-v3/long-sandboxes. V6 sửa thu nhập có nguồn, pro-rata payroll,
food/health feedback, mức hữu hạn của xây dựng/học, fiscal reward và quyền thuế;
không diễn giải thay luật thành công trạng học của model.

V6 dùng holdout mới200001–200008, mỗi world7.308 ngày, cùng3 learned policies,
có sốc đóng tuyến ngày30. Đây là world tiếp tục, không phải vua sống đủ20năm.

| Seed | Còn sống | Trẻ em | Tổng ID từng tồn tại | Tiền + invariant |
|---|---:|---:|---:|---|
| 200001 | 11,516 | 1,593 | 12,433 | True |
| 200002 | 11,525 | 1,562 | 12,381 | True |
| 200003 | 7,788 | 1,084 | 11,945 | True |
| 200004 | 11,475 | 1,543 | 12,343 | True |
| 200005 | 11,439 | 1,532 | 12,320 | True |
| 200006 | 11,395 | 1,504 | 12,371 | True |
| 200007 | 11,452 | 1,526 | 12,418 | True |
| 200008 | 11,446 | 1,513 | 12,383 | True |

Còn sống lớn hơn0 không tự chứng minh xã hội tự duy trì hoặc dân số cân bằng.
Seed200003 chỉ còn7.788/10.000 người ban đầu; đó là trường hợp mất cân bằng
được giữ nguyên. Các lượt dài có chạy đồng thời, nên không dùng timing của
chúng thay benchmark riêng365ngày dưới đây.
Một năm profileA: 123,291 quyết định; p95 ngày
5.89ms, peak native process
44.6MiB. Bao gồm core+observation+inference+
sampling+commit+invariant, **chưa gồm render/input/save**. i7-12700K,12core20thread,
15.78GiB RAM; ORT1.31 CPU2intra/1inter. Không thay benchmark toàn app/B/C.

## Draft12augment và sáu stat

Court tests168 kiểm tra gồm counterfactual cùng mở sổ, tài khoản/vật tư,
capacity, quyền từ chối, thời gian và serialize/replay. Khả năng nguyên mẫu:

| Thẻ | Cơ chế và giới hạn |
|---|---|
| A01 | Báo cáo công nhanh20%, ADM kiểm kê+10 chặn100; không mở bí mật |
| A02 | Dự án đào tạo nghề triều đình nhanh15%; cần giáo viên, người đang học, tiền/food/cloth |
| A03 | Hàng đợi hòa giải bổ sung180ngày, chuẩn bị nhanh20%; đối tác có thể từ chối |
| A04 | Kho/thóc thêm90ngày nhu cầu đầu kỳ; một lần, spoilage và bảo trì có nguồn |
| A05 | Quỹ tín dụng riêng0.5R0; vay tùy chọn5năm, nghĩa vụ và hoàn trả về đúng quỹ |
| A06 | Một đội kỹ sư cho dự án đường; productivity+20%, giữ tiền và vật tư |
| A07 | Ba cư dân hiện hữu có credential70–80 ứng tuyển; bổ nhiệm trả lương, có thể từ chức |
| A08 | Courier nhanh25%, vua điều tra2vụ đồng thời; chứng cứ tới khi hoàn tất |
| A09 | Quỹ hạn chế1R0, matching tối đa50% hạ tầng; không cứu trợ/trả nợ bằng quỹ |
| A10 | Khung ba chư hầu liền kề, ba quyết định chấp thuận; escrow tiền/iron/tools, từ chối hoàn trả |
| A11 | Học viện+8chỗ, hai dự án teacher training, tốc độ+25%; cần giáo viên thực |
| A12 | Hai ứng viên cố vấn và hai office slots bổ sung; lương thật, actor và lợi ích riêng |

R0 là reference mở sổ365ngày với thời tiết cố định, không strategy/birth/random
aging; copy không thay RNG tương lai world thật. Đây là convention cân bằng
prototype. A05 dùng simple interest15% toàn5năm (3%/năm theo reference4%/năm);
vay ngắn hiện vẫn có fee4% mỗi hợp đồng, nên cần chuẩn hóa term structure trước
khi tuyên bố giá tín dụng tổng quát. Hòa giải hiện là offer/accept/reject có
escrow, chưa có hệ thống tranh chấp/công lý nhiều giai đoạn đầy đủ. Dự án thiếu
người học/giáo viên sẽ stall, không giả completion.

INT ảnh hưởng học; ADM thời gian dự án/kiểm kê; ECO procurement preparation;
DIP chuẩn bị offer; INTG thời gian điều tra; MIL xử lý unrest. Đây là cơ chế,
không chứng minh reasoning. Agency probe100 state ×64 sampling cho tuổi,
ngân sách riêng, trường, việc, nutrition/health và goal. Child labor/capacity
masks pass=True; variation riêng=
True. Ngân sách trẻ còn có guardian;
probe tiền riêng không tương đương cắt toàn bộ ngân sách gia đình.
**R2_full_pass=False**: thiếu đánh giá learned transfer trẻ→người lớn đầy đủ.
Goal-only first-decision TV trên cùng legal set là
0.000424; phản ứng goal còn rất yếu.
Mask đúng là tính hợp lệ của luật, không tự chứng minh policy biết lập kế hoạch.
**R3_full_pass=False**: chưa có multi-step strategic INT task với paired CI
chứng minh lợi thế trí lược. Không dùng skill gain cơ học thay bằng chứng đó.

## Giới hạn và khả năng tái lập

Chưa có campaign24vùng/16hàng, chiến tranh đầy đủ, pháp lý/chính trị hoàn chỉnh,
planner hay profileB100k/C1m learned agency. Training focal giữ họ, loại applicant
để tránh reward đổi đơn vị khi thăng chức; runtime vẫn xử lý đổi họ thực tế.
Training partners là test controllers có nhãn; không lẫn vào native release.
Save schema realm-v6 từ chối bundle cũ. Merchant v2 vẫn giữ nguyên bản học trước.
Source/checkpoint/protocol và Actions ở repo badaycona/PolyEc.
