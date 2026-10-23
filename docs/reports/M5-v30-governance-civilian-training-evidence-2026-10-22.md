# M5 v30 — kết quả huấn luyện thăm dò Governance và Civilian

Ngày 22-10-2026. Báo cáo này mô tả hai lần huấn luyện thăm dò policy Realm v11. Đây không phải chứng nhận RL hoàn tất: không family nào có policy được selector chấp nhận, không có model được export, không chạy holdout, và chưa chạy native benchmark bằng policy học được.

## Kết quả người dùng cần biết

Governance và Civilian đều được huấn luyện trên đúng C++ Realm hiện tại, nhưng mỗi policy học được trong ngân sách 25 update lại đạt utility thấp hơn bộ điều khiển developer baseline trên tập đánh giá độc lập. Khoảng tin cậy 95% của chênh lệch utility đều nằm dưới 0 so với baseline; không seed nào qua điều kiện selector. Với Governance, hai seed còn làm tăng nợ và số người mất khả năng thanh toán so với baseline. Không có model phù hợp để thay model cũ trong runtime, vì vậy kết quả này chưa mở khóa benchmark policy native, profile B hoặc package.

Trong lần khởi tạo Civilian đầu tiên, pipeline dừng trước update 1 vì Python adapter tìm khóa `candidate` trong dictionary `person()` mà binding C++ không xuất ra. Observation Realm v11 đã có cùng cờ ở chỉ số 102. Adapter được sửa để đọc observation; environment checks đạt 25/25, cross-play checks đạt 28/28, và một smoke PPO riêng hoàn thành 1.024 quyết định cùng checkpoint. Lượt lỗi không được tính là dữ liệu huấn luyện. Runset chính thức Civilian sau đó hoàn tất đủ ba seed.

## Nguồn dữ liệu huấn luyện và cách tạo

Đây là dữ liệu tương tác được sinh trực tiếp trong lúc PPO train, không phải bảng offline có sẵn và không phải dữ liệu người chơi. Với mỗi family và seed huấn luyện 1, 2, 3, trainer tạo 16 thế giới Realm C++ độc lập. Seed khởi tạo các thế giới bắt đầu ở `2^32 + training_seed × 10.000.000`; mỗi thế giới mô phỏng 6 vùng và dùng horizon 128 ngày. Mỗi bước, binding C++ cung cấp observation hiện tại và legal-action mask. Policy chọn hành động trong số hành động hợp lệ; adapter gửi ý định vào cùng Realm command pipeline, C++ thực thi hoặc từ chối theo luật, rồi trainer lấy reward từ thay đổi utility sau transition.

Một update gom 64 bước liên tiếp trong mỗi 16 thế giới, tức `16 × 64 = 1.024` quyết định của policy đang học. PPO chia rollout thành chuỗi hồi quy dài 16 bước, dùng minibatch 64 chuỗi và 3 epoch với learning rate `0,0003`, entropy coefficient `0,01`. 25 update tạo 25.600 quyết định/seed và 76.800 quyết định/family. Cả hai family dùng cùng ba training seed, cấu hình và ngân sách. Governance là role 0; Civilian là role 2. Không dùng Economic worksite curriculum trong các lượt này.

Ở mỗi thế giới, các actor khác tiếp tục quyết định qua cùng Realm. Vì chưa có policy học được đủ điều kiện cho family đối tác, actor đối tác dùng bộ điều khiển dev baseline có nhãn; chúng không được gọi là learned policy. Policy ứng viên và các đối tác vẫn chịu legal mask và command validation của C++. Dataset không được ghi thành một tệp trajectory dài hạn riêng: các observation, mask, hành động, reward và trạng thái tuần tự được giữ trong rollout buffer khi update chạy; checkpoint lưu policy, optimizer, RNG và toàn bộ trạng thái môi trường để resume chính xác, còn history/manifest lưu số quyết định và chẩn đoán update. Do đó report chỉ khẳng định đúng loại dữ liệu trainer thực sự tạo và giữ lại.

## Kiểm tra adapter và cấu hình run

Preflight xác nhận binding `realm-v11`, observation 1.676 chiều, 129 action slot, model contract `realm-household-fund-goods27-v5`, config fingerprint `7186450176306598410`, PyTorch 2.5.1 CPU và ONNX 1.23.2. Environment checks trước training đạt 22/22; sau khi bổ sung regression cho Civilian và sửa lỗi adapter đạt 25/25. Cross-play adapter đạt 28/28 trước và sau sửa.

Lượt Governance seed 1 ban đầu dừng ở update 7 do cap thăm dò 300 giây. Trainer ghi checkpoint; tiếp tục từ chính checkpoint đó với cùng training identity/RNG/world đến update 25. Seed 2 và 3 hoàn tất trực tiếp. Mỗi seed có cap tối đa 1.800 giây và chỉ được đưa vào selector nếu đủ 25 update. Không thay reward, observation, action, model contract, luật gameplay hoặc optimizer. Mọi model checkpoint ghi `peer_eligible=false`.

Civilian có một lượt khởi tạo lỗi nói trên, sau đó smoke riêng chạy một update đầy đủ với 16 environment ×64 bước =1.024 quyết định, 1 epoch, và không được đưa vào training set chính. Ba seed chính thức hoàn tất 25/25 update, mỗi seed 25.600 quyết định; `TRAINING-SET.json` khóa hash manifest/policy và xác nhận ngân sách bằng nhau. Cùng giới hạn 1.800 giây/seed được dùng cho runset chính.

## Validation và tiêu chí chọn

Selector đánh giá Governance trên 20 world seed 194000–194019 và Civilian trên 20 world seed 195000–195019; hai dải tách khỏi các dải trước đó 190000–190019 và 193000–193019. Mỗi world chạy 128 ngày. So sánh ghép cặp trên cùng seed loại bỏ khác biệt do mỗi policy gặp thế giới khác nhau.

Selector chỉ chọn ứng viên nếu đạt đồng thời: cận dưới khoảng tin cậy 95% của utility so với baseline không thấp hơn `-0,05`; cận dưới của utility so với policy ban đầu lớn hơn 0; cận dưới chênh lệch survival không âm; cận trên chênh lệch insolvency/default không dương; và ít nhất 25 update. Quy tắc này được giữ nguyên cho cả hai family. Số âm của utility ở bảng dưới nghĩa là policy học được có utility trung bình thấp hơn đối thủ được nêu. “Policy ban đầu” là mạng policy tại đầu lượt train trước các update; không phải policy phát hành.

### Governance

| Training seed | Utility so với baseline, trung bình [CI 95%] | Utility so với policy ban đầu, trung bình [CI 95%] | Nợ so với baseline, trung bình [CI 95%] | Insolvent so với baseline, trung bình [CI 95%] | Kết quả |
|---|---:|---:|---:|---:|---|
| 1 | -0,092343 [-0,115436; -0,069251] | +0,005233 [-0,006772; +0,017238] | +20,8 [+2,094; +39,506] | +0,2 [+0,020; +0,380] | Loại |
| 2 | -0,089366 [-0,108060; -0,070672] | +0,008211 [-0,002409; +0,018831] | +31,2 [+9,770; +52,630] | +0,3 [+0,094; +0,506] | Loại |
| 3 | -0,094617 [-0,116429; -0,072806] | +0,002959 [-0,006959; +0,012878] | 0 [0; 0] | 0 [0; 0] | Loại |

### Civilian

| Training seed | Utility so với baseline, trung bình [CI 95%] | Utility so với policy ban đầu, trung bình [CI 95%] | Nợ so với baseline, trung bình [CI 95%] | Insolvent so với baseline, trung bình [CI 95%] | Kết quả |
|---|---:|---:|---:|---:|---|
| 1 | -0,119395 [-0,171908; -0,066881] | +0,012788 [-0,021703; +0,047279] | 0 [0; 0] | 0 [0; 0] | Loại |
| 2 | -0,134636 [-0,191805; -0,077467] | -0,005980 [-0,041849; +0,029889] | 0 [0; 0] | 0 [0; 0] | Loại |
| 3 | -0,132573 [-0,184485; -0,080661] | -0,004062 [-0,036692; +0,028569] | 0 [0; 0] | 0 [0; 0] | Loại |

Cả ba seed Governance và cả ba seed Civilian có `validation_eligible=false`; `selected_seed=null` cho cả hai family. Governance seed 1–2 còn làm tăng nợ trung bình 20,8/31,2 và số người mất khả năng thanh toán trung bình 0,2/0,3 mỗi world so với baseline. CI của utility so với policy ban đầu đều cắt qua 0 ở cả hai family, vì vậy chưa có bằng chứng cải thiện ổn định trên chính sách khởi đầu. Civilian không có chênh lệch nợ hoặc mất khả năng thanh toán trong 20 world này, nhưng vẫn trượt rõ ràng về utility so với baseline.

## Phạm vi kết luận và việc tiếp theo

Validation-only exit code 2 là cách selector báo không tìm thấy candidate; JSON đầu ra xác nhận không dùng holdout để chọn. Không export policy, không tạo bundle model, không chạy 100-world holdout và không chạy `polyec_realm_benchmark` bằng policy. Không dùng kết quả core-only benchmark trước đó để thay cho kiểm tra policy. Runtime native vẫn bị chặn vì thiếu bundle Governance/Civilian đủ điều kiện.

Lần thử kế tiếp được ghi riêng trong [kế hoạch v31](../design/M5-v31-extended-governance-civilian-training-plan.md): resume từ checkpoint đủ 25 update, kéo dài đến tổng 75 update/seed, giữ nguyên luật, reward và optimizer, rồi dùng hai dải validation mới. Không tái dùng hai dải validation v30 để quyết định kết quả mới. Việc tăng số update là một thí nghiệm mới; nếu vẫn không có candidate, không tuyên bố RL hoàn tất và phải phân tích lỗi theo dữ liệu rollout trước khi đổi reward hay gameplay.

## Tệp bằng chứng

- [Validation Governance và số liệu ghép cặp](../../build/m5-v30-governance-civilian-training/governance/validation/selection.json)
- [Validation Civilian và số liệu ghép cặp](../../build/m5-v30-governance-civilian-training/civilian-fixed/validation/selection.json)
- [Training set lock Governance](../../build/m5-v30-governance-civilian-training/governance/TRAINING-SET.json)
- [Training set lock Civilian](../../build/m5-v30-governance-civilian-training/civilian-fixed/TRAINING-SET.json)
- [Lệnh và output runset Civilian](../../build/m5-v30-governance-civilian-training/civilian-fixed-runset.log)
- [Lệnh và kết quả selector Civilian](../../build/m5-v30-governance-civilian-training/civilian-fixed-validation.log)
- [Kế hoạch/cấu hình ban đầu](../design/M5-v30-governance-civilian-training-plan.md)
- [Manifest hash cho source, policy, training lock và kết quả validation](../../artifacts/m5-v30-governance-civilian-training-2026-10-22-manifest.json)

## Bổ sung biểu đồ từ đúng dữ liệu đã lưu

Ngày 22-10-2026, T5.6 tạo biểu đồ huấn luyện từ sáu `history.jsonl` và hai biểu đồ đánh giá từ hai `selection.json` của chính run v30. Hình tách rõ return rollout huấn luyện với utility/outcome validation trên seed độc lập; không chạy lại trainer hoặc validation. Kết quả nhìn thấy vẫn đúng với bảng phía trên: cả sáu seed có utility dưới baseline, Governance seed 1–2 có nợ và mất khả năng thanh toán tăng. Phần mô tả nguồn, cách đọc, giới hạn dữ liệu history và hashes nằm trong [báo cáo T5.6](M5-T5.6-rl-run-visual-evidence-2026-10-22.md); đây là bổ sung, không thay bảng hay kết luận ban đầu.

