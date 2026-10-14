"""Render immutable measurements, including negative survival/quality results."""
import argparse,json
from pathlib import Path
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
p=argparse.ArgumentParser();p.add_argument('--models',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
read=lambda name:json.loads((a.models/name).read_text(encoding='utf-8-sig'))
audit=read('runtime-sampling-audit.json');parity=read('native-parity.json');agency=read('agency-stat-report.json');profile=read('profile-A-native.json');verification=read('verification.json')
long=[json.loads(f.read_text()) for f in sorted((a.models/'long-sandboxes').glob('seed-*.json'))]
assert len(long)==8 and all(x['days']==7308 for x in long)
families=('governance','economic','civilian');labels=('Governance','Economic','Civilian')
plt.rcParams.update({'figure.facecolor':'#0b191f','axes.facecolor':'#14262b','text.color':'#e7e2cd','axes.labelcolor':'#e7e2cd','xtick.color':'#91a7a4','ytick.color':'#91a7a4','axes.edgecolor':'#345057','font.size':11,'grid.alpha':.15})
fig,axes=plt.subplots(1,3,figsize=(15,5));fig.suptitle('PolyEc / categorical deployment audit — 100 untouched worlds',fontsize=18)
for ax,family,title in zip(axes,families,labels):
 e=audit['families'][family]['evaluation'];names=('original_untrained','baseline','learned');means=[e[n]['mean_utility'] for n in names]
 ax.bar(['Untrained','Dev baseline','Learned PPO'],means,color=['#627b7e','#d1b575','#74b7a5']);ax.axhline(0,color='#91a7a4',lw=.7);ax.set_title(title);ax.set_ylabel('Normalized actor utility change');ax.grid(axis='y')
fig.text(.05,.02,'Paired confidence intervals and survival are in the report. Mechanical validity does not certify full R1/R2/R3.',fontsize=10,color='#91a7a4');fig.tight_layout(rect=(0,.07,1,.91));fig.savefig(a.output/'PolyEc-M3-evidence.png',dpi=150);plt.close(fig)
fig,axes=plt.subplots(1,3,figsize=(15,5));fig.suptitle('Realm v6 / recurrent PPO / three training seeds per family',fontsize=18)
for ax,family,title in zip(axes,families,labels):
 for seed,color in zip((1,2,3),('#74b7a5','#d1b575','#93b5cb')):
  history=read(f'{family}/seed-{seed}-history.json');rows=[r for r in history if 'validation_mean_profit' in r]
  ax.plot([r['decisions']/1000 for r in rows],[r['validation_mean_profit'] for r in rows],label=f'Seed {seed}',color=color)
 ax.set_title(title);ax.set_xlabel('v6 decisions (thousands)');ax.set_ylabel('Validation normalized utility');ax.legend();ax.grid()
fig.text(.05,.02,'Warm starts continue each seed\'s own v3 checkpoint. Critic reset; no imitation. Validation selects, test never tunes.',fontsize=10,color='#91a7a4');fig.tight_layout(rect=(0,.07,1,.91));fig.savefig(a.output/'PolyEc-M3-training.png',dpi=150);plt.close(fig)
rows=[];outcomes=[];gates=[]
for family in families:
 f=audit['families'][family];d=f['paired']['original_untrained'];b=f['paired']['baseline'];e=f['evaluation']['learned'];rows.append(f"| {family} | {e['mean_utility']:.3f} | {d['mean']:+.3f} [{d['ci95'][0]:+.3f}, {d['ci95'][1]:+.3f}] | {b['mean']:+.3f} [{b['ci95'][0]:+.3f}, {b['ci95'][1]:+.3f}] |")
 outcomes.append(f"| {family} | {e['survival_rate']:.1%} | {e['mean_health']:.1f} | {e['hold_rate']:.1%} | {e['defaults']} / {e['obligations_due']} |")
 gates.append(f"- {family}: cải thiện so untrained = **{f['original_improvement']}**; baseline noninferiority = **{f['baseline_noninferiority']}**.")
training=[x for family in families for x in read(f'{family}/evaluation.json')['training']]
maxlog=max(v['max_abs_logits'] for v in parity['families'].values());maxmem=max(v['max_abs_memory'] for v in parity['families'].values())
longrows='\n'.join(f"| {x['seed']} | {x['alive_end']:,} | {x['children_end']:,} | {x['population_end']:,} | {x['invariant'] and x['money_conserved']} |" for x in long)
text=f'''# PolyEc — M3 native sandbox và bằng chứng RL v6

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

- CTest: {verification['ctest']}; core70, trade36.546, realm24.980, court168 checks.
- Replay C++/Python64 ngày:258 checks; batch sai action và bundle mismatch atomic.
- Bộ huấn luyện476 checks:1/4 worker đồng nhất, seed reset theo thứ tự, terminal
  freeze, tổng reward bằng normalized score và GRU không rò qua episode boundary.
- Godot native/UI/merchant/realm: {verification['godot_checks']}.
- Native runtime37checks: đổi họ/reset memory/replay, privacy hồ sơ đề nghị,
  chặn nhận điều khoản chưa quan sát trong batch và chặn lệnh UI từ snapshot cũ.
- Realm64 ngày khớp executable C++ độc lập theo fingerprint, quyết định, tiền,
  vòng đời; save/restore replay và focus/sleep pause đều được smoke kiểm tra.
  Fixture được sinh bằng executable của từng platform; chưa tuyên bố replay
  bit-identical giữa Windows và Linux.
- Torch/Python ORT/native C++ ORT: {parity['checks']} checks, batch1/3/32/64,
  sai số logits tối đa{maxlog:.3g}, memory{maxmem:.3g}; invalid probability0.
- Windows portable/extracted checks: {verification['portable']}.
- Linux/MSVC: {verification.get('cross_platform','Xem GitHub Actions cho đúng revision; chưa ghi nhận final revision tại thời điểm tạo báo cáo này.')}.

## Protocol và kết quả học

Ba seed/họ, warm start mỗi seed từ đúng checkpoint v3 tương ứng, critic reset,
không imitation. Curriculum1/2/6 vùng, PPO recurrent GRU64, rollout64/BPTT16,
4 worker core và2 Torch threads. V6 thực hiện {sum(x['decisions'] for x in training):,}
quyết định, {sum(x['seconds'] for x in training):.1f}s recorded fit time.
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
{chr(10).join(rows)}

{chr(10).join(gates)}

| Họ | Sống cuối horizon | Health TB | Hold | Default / nghĩa vụ đến hạn |
|---|---:|---:|---:|---:|
{chr(10).join(outcomes)}

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
{longrows}

Còn sống lớn hơn0 không tự chứng minh xã hội tự duy trì hoặc dân số cân bằng.
Seed200003 chỉ còn7.788/10.000 người ban đầu; đó là trường hợp mất cân bằng
được giữ nguyên. Các lượt dài có chạy đồng thời, nên không dùng timing của
chúng thay benchmark riêng365ngày dưới đây.
Một năm profileA: {profile['decisions']:,} quyết định; p95 ngày
{profile['full_day_p95_ms']:.2f}ms, peak native process
{profile['peak_process_bytes']/2**20:.1f}MiB. Bao gồm core+observation+inference+
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
masks pass={agency['age_and_capacity_masks_pass']}; variation riêng=
{agency['private_sampling_variation_pass']}. Ngân sách trẻ còn có guardian;
probe tiền riêng không tương đương cắt toàn bộ ngân sách gia đình.
**R2_full_pass=False**: thiếu đánh giá learned transfer trẻ→người lớn đầy đủ.
Goal-only first-decision TV trên cùng legal set là
{agency["mean_total_variation_vs_adult"]["adult_social_goal"]:.6f}; phản ứng goal còn rất yếu.
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
'''
(a.output/'PolyEc-M3-report.md').write_text(text,encoding='utf-8');(a.models/'milestone-report.md').write_text(text,encoding='utf-8')
print('Rendered recorded M3 evidence, including actual long-run population and failed gates')
