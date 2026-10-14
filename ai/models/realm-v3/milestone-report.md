# PolyEc — bản thử cơ chế và RL tại M3

Đã triển khai sandbox native có sáu lãnh địa + đất triều đình, bảy hàng, thị trường,
lao động/học tập, mùa vụ, vận tải/đóng tuyến, vay–trả/default, dự án có ngân sách
và vật tư, quyền địa phương, báo cáo trễ/điều tra, hộ gia đình và quỹ tổ chức.
10.000 cá thể ban đầu gồm128 actor chủ chốt +9.872 cư dân; ID, vòng đời,
memory/sampling riêng. App dùng ba ONNX actor PPO trên CPU; không fallback heuristic.

**Đây là bản thử M3 với gate được chẩn đoán, chưa phải xác nhận hoàn tất mọi
chi tiết M1/M2/M3 hoặc bản campaign hoàn chỉnh.** Không đồng nhất “đã train”
với “đã vượt baseline”. Các thiếu sót được ghi dưới đây.

## Kiểm chứng đã chạy

- C++ core70, trade36.546, realm14.305 kiểm tra; CTest3/3.
- C++/Python replay64 ngày:258 kiểm tra, batch lỗi và bundle sai không đổi state.
- Godot native19/19, UI18/18, merchant RL336/336, integrated realm410/410.
- Realm64 ngày khớp fingerprint/counters/tiền/vòng đời với executable C++ độc lập;
  21.784 quyết định learned. Save khôi phục cả RNG, sampling và recurrent memory;
  replay sau tải giống nhau; save hỏng giữ world đang chơi.
- PyTorch/Python ORT/native C++ ORT: 489 kiểm tra, batch1/3/32/64;
  sai số logits tối đa1.91e-06, memory2.98e-07; invalid probability0.
- Tám sandbox learned co-play x7.308 ngày (hơn20 năm), đã hoàn tất8/8;
  tất cả báo invariant và tiền bảo toàn: True.
  Đây là sandbox tiếp tục world, không giả chiến dịch vua sống20 năm.
- Một năm ở profileA: 124,228 quyết định, p95 ngày
  4.35ms gồm core+observation+inference+sampling+commit;
  peak process 45.4MiB. Chưa gồm render/input/save;
  không dùng số này thay benchmark toàn app/profileB/C.
- CPU i7-12700K,12core20thread,15.78GiB RAM; ORT1.31.0 CPU2intra/1inter.
  Render Windows: Godot4.5.1, OpenGL3.3/RX580. CI Linux native có workflow riêng.

## Model và đánh giá

Ba training seeds/họ; curriculum1/2/6 vùng trong v1/v2, v3 tiếp tục PPO từng seed
sau sửa luật. V3: 1,439,744 quyết định,
715.6s training; các lượt trước được giữ.
Checkpoint/seed chỉ chọn bằng validation10000–10031. Argmax test80000–80099
được giữ nguyên. Audit sampling dưới đây dùng test mới90000–90099,100 world
ghép cặp; không dùng để chọn checkpoint. “Untrained” là checkpoint random trước
bất kỳ PPO nào, không phải checkpoint đã train trước fine-tuning v3.

Đơn vị là thay đổi utility đã version hóa của actor, **không phải lợi nhuận tiền**.
Các họ có objective khác nhau, không so utility giữa các hàng.

| Họ | Learned sampled | So untrained,95% CI | So baseline,95% CI |
|---|---:|---:|---:|
| governance | -4.22 | +1.51 [+0.44, +2.61] | -25.68 [-27.35, -23.94] |
| economic | -3.62 | +4.01 [+3.30, +4.81] | +1.66 [+0.86, +2.50] |
| civilian | 56.45 | +114.55 [+92.33, +136.68] | -0.93 [-18.34, +15.72] |

Governance vẫn thua baseline rõ. Civilian cải thiện lớn so untrained nhưng CI
so baseline còn quá rộng để qua biên noninferiority5utility. Economic qua biên
trong audit sampling này; một merchant curriculum không chứng minh toàn nền kinh tế.
Survival, health, hold, nghĩa vụ đến hạn/default và cash–debt shortfall có trong
runtime-sampling-audit.json. Chưa có legal bankruptcy và chất lượng thỏa thuận
chính trị có điều kiện đầy đủ; full_R1_pass vẫn false.

## R2/R3 và draft

- Thử100 state x64 sampling seeds: tuổi, ngân sách, việc làm và chỗ học thay đổi
  phân bố; mask chặn child labor, trường hết chỗ và việc hết chỗ.
- Sampling riêng tạo nhiều hành động trong cùng state; batch không bắt cả nhóm
  chọn chung. Cần thêm đánh giá learned rollout từ trẻ tới trưởng thành và co-play
  trên các phân bố holdout; chưa chứng nhận R2 đầy đủ.
- Sáu stat là input và có hồ sơ. INT ảnh hưởng học, ADM dự án, INTG điều tra,
  MIL giảm unrest; ECO/DIP forecast/negotiation chưa đủ. Không có bằng chứng
  nhiệm vụ nhiều bước rằng INT cao suy xét tốt hơn; R3 chưa đạt.
- Draft có12ID/3cấp,3thẻ khác nhau cùng cấp, reroll1lần, chọn1lần và lưu nguyên
  lựa chọn. **Hiệu ứng của cả12thẻ chưa hoàn chỉnh**: A01/A03, quỹ hạn chế A09,
  hợp đồng liên lãnh A10, nhân sự/cố vấn và phạm vi modifier còn cần hoàn thiện.
  Kiểm tra draft/invariant không được gọi là kiểm tra đủ12hiệu ứng thiết kế.

## Lỗi đã tìm và xử lý

- v1 governance không hơn initial; kết quả được giữ, không sửa baseline để qua gate.
- Thuế lương làm tròn về0: thêm phần lẻ tích lũy; cứu trợ trả tới người thật.
- Hợp đồng hết hạn bị kéo dài khi chuyển sang học: regression đã fail trước sửa,
  sau sửa pass. Thêm luật dùng ngân sách guardian có nguồn và quỹ tương trợ thật;
  v3 train/evaluate lại với test mới.
- CLI từng nạp nhầm ORT1.17.1 từ PATH: bundle đúng1.31 cạnh executable và kiểm tra
  API trước tạo session. Bộ merchant cũ sau sửa vẫn98/98 parity.
- Linux cold import gặp race docs GDExtension Godot#111048; CI chờ callback bằng
  frame delay cho editor import, giữ strict exit/log checks. Không bỏ qua lỗi.

## Cách chạy và giới hạn phát hành

Giải nén Windows, mở PolyEc.exe → “Quốc gia · Sandbox”, chọn thẻ rồi chạy/tiến
một ngày. Đổi ID xem hồ sơ và lệnh; F1 hướng dẫn; lưu/tải tại user data Godot.
UI là sandbox có góc nhìn inspect từng actor, chưa là presentation secrecy của
campaign. “RL · Phòng thử nghiệm” giữ merchant v2 để đối chiếu.

Native Linux build/smoke/package có CI; GUI Linux dùng software render khi kiểm
tra, không phải benchmark iGPU. Chưa có campaign24vùng/16hàng, chiến tranh,
profileB100k/C1m hay planner. Không đánh dấu các gate đó đã đạt.

Source/checkpoint/config/selection locks/reports ở ai/models; source raw hashes
ghi compiler input Windows, canonicalLF hashes hỗ trợ đối chiếu Git checkout.
Các phiên bản môi trường cũ là lịch sử thử nghiệm, không tự đổi vào save mới.
