# Báo cáo: kiểm tra evaluator v15 và đo ngân sách trước candidate R1

Ngày chạy: 2026-10-23. Đây là báo cáo cho phần chuẩn bị huấn luyện Realm v15 theo protocol `realm-v15-r1-seed-split-1`. Kết quả ở đây xác nhận đường chạy và giúp đo tài nguyên; **chưa có policy candidate, chưa chọn policy, chưa chạy holdout và chưa xác nhận chất lượng AI**.

## 1. Kết quả dành cho người đọc

Đã chạy một lượt PPO smoke mới cho cả ba nhóm actor — Governance, Economic và Civilian — bằng cùng phiên bản Python, PyTorch, NumPy và ONNX mà `ai/requirements.txt` quy định. Mỗi lượt tạo tương tác trong 16 thế giới C++ sáu vùng, lấy 64 quyết định của actor được huấn luyện trong mỗi thế giới, tổng cộng 1.024 quyết định, rồi thực hiện đúng một lượt cập nhật PPO. Các actor còn lại trong những thế giới đó hành động bằng controller dev baseline có nhãn; chúng không phải policy đã học.

Đã chạy thêm evaluator smoke trên một world seed riêng cho mỗi nhóm. Với từng seed, evaluator khởi tạo lại world từ cùng seed cho năm cách hành động: dev baseline, trọng số sau một update smoke, trọng số ban đầu, chỉ giữ nguyên và chọn ngẫu nhiên trong các lựa chọn C++ cho phép. Đây chỉ xác nhận rằng evaluator có thể chạy hết horizon, ghép mode theo cùng seed, kiểm invariant và ghi báo cáo/biểu đồ. Một world cho mỗi nhóm không đủ để kết luận policy tốt hơn baseline.

Mỗi training smoke có biểu đồ reward và PPO return, biểu đồ cơ hội/lựa chọn theo đủ Intent và JSON nguồn riêng lấy từ chính manifest/history của run đó. Mỗi evaluator smoke có JSON từng mode/world và biểu đồ utility có nhãn “smoke only”. Rollout từng quyết định được tạo trong C++/Python lúc chạy và giữ trong RAM; các tệp lưu trên đĩa là manifest, history tổng hợp, initial weights, checkpoint, kết quả evaluator, biểu đồ và JSON nguồn. Không dùng dữ liệu ước lượng hoặc trộn seed.

## 2. Contract và môi trường thực thi

- Core mô phỏng: binding C++ `realm-v30`; observation 3.550 số, 264 action; model contract `realm-worksite-payroll-goods27-config-v34`; reward `realm-v13-person-enterprise-equity-v1`; timeline fixture `one-tick-generic-schedule-v1`.
- Python: 3.12.6; PyTorch `2.13.0+cpu`; NumPy `2.5.1`; ONNX `1.23.2`; ONNX Runtime `1.31.0`; ONNX Script `0.7.2`; psutil `7.2.2`.
- Môi trường riêng nằm ở `build/m5-python-ai/venv-v15/`, được tạo từ bộ khóa trong `ai/requirements.txt`. Binding C++ Release hiện có được nạp qua `PYTHONPATH`; không build lại core hoặc extension vì nhóm này không đổi API/binding.
- Trước khi huấn luyện candidate, đã chạy `py_compile` trên evaluator, selector, holdout, train-set và script biểu đồ; bộ kiểm thuần logic mới có 14 assertion thành công. Phép kiểm bao gồm seed validation/holdout rời nhau, CI đủ/thiếu mẫu, ngưỡng R1, không chọn khi không candidate đạt, từ chối schema cũ và checkpoint smoke, kiểm hash/seed của selection lock, cùng việc tạo đủ ba loại biểu đồ validation.

## 3. Cách dữ liệu smoke được tạo

Đây không phải dữ liệu game được chép sẵn từ một file. Với từng run, trainer khởi tạo trọng số model mới bằng seed đã ghi dưới đây; đặt seed cho PyTorch, NumPy và Python; rồi tạo các Realm thật qua binding C++. Mỗi Realm có sáu vùng và 128 cá thể. Trainer bật fixture timeline v34, nhận observation/action mask từ C++, lấy action hợp lệ theo mask, gửi action qua transition C++ và dùng reward mà core trả về để tính đúng một update PPO. Các peer bên cạnh actor focal sử dụng `realm-v15-observation-dev-v1` vì tại thời điểm smoke chưa có learned peer nào qua validation/holdout.

Một quyết định Governance hoặc Civilian tiến 30 ngày game; một quyết định Economic tiến 7 ngày. Vì vậy cùng số 1.024 quyết định không có nghĩa là cùng lượng thời gian mô phỏng hoặc cùng runtime CPU. Curriculum xưởng kinh tế đã khóa trong R1 chỉ dùng khi chạy candidate Economic; các smoke ban đầu dưới đây đo adapter PPO một update và không được dùng để chọn policy.

Evaluator smoke dùng các seed `303000` (Governance), `303001` (Civilian), `303002` (Economic). Mỗi mode bắt đầu từ world mới cùng seed, cùng contract, fixture và actor identity; learned mode dùng weights sau đúng một update smoke tương ứng; initial mode dùng `initial.pt`. Vì checkpoint smoke không đủ 25 update và có `policy_candidate=false`, evaluator chỉ phát report `smoke_only=true`; selector chính thức từ chối loại checkpoint này.

## 4. Thời gian và bộ nhớ của đúng các training smoke

| Nhóm actor | Seed smoke | Horizon đầy đủ theo contract | Quyết định focal trong smoke | Rollout | Toàn update | Quyết định peer | Peak RSS | Reward trung bình trong rollout | PPO return target trung bình |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Economic | 20261024 | 512 quyết định × 7 ngày | 1.024 | 12,445 giây | 13,307 giây | 29.842 | 555.204.608 byte (529,4 MiB) | −0,007858 | −0,281620 |
| Governance | 20261025 | 128 quyết định × 30 ngày | 1.024 | 48,226 giây | 49,115 giây | 80.495 | 558.235.648 byte (532,3 MiB) | −0,012234 | −0,481334 |
| Civilian | 20261026 | 128 quyết định × 30 ngày | 1.024 | 56,471 giây | 57,432 giây | 111.088 | 558.346.240 byte (532,4 MiB) | −0,031768 | −0,597887 |

Reward và return trong bảng là số liệu rollout/update, không phải điểm kiểm tra độc lập. Một điểm trên biểu đồ không biểu thị xu hướng. Chênh lệch thời gian giữa nhóm actor phản ánh cả lịch mô phỏng 7/30 ngày và số hành động của peer; không thể coi các family có cùng tốc độ.

Theo số update đo được, phần huấn luyện candidate có workload dự kiến khoảng 998 giây cho Economic, 3.684 giây cho Governance và 4.307 giây cho Civilian: mỗi family gồm ba seed × 25 update. Tổng là khoảng 8.989 giây CPU cho 9 run và 230.400 quyết định focal. Đây chỉ là phép nhân từ một smoke-update/family, không phải thời hạn bảo đảm; curriculum, số ngày mô phỏng và biến động world có thể làm thời gian thực khác đi. Con số này chưa tính evaluator validation 20 seed, holdout 100 seed, ghi biểu đồ hay lần chạy lại lỗi. Các run sẽ chạy tuần tự trên CPU; không giảm số seed/update và không mua GPU/cloud.

## 5. Thời gian của evaluator smoke và giới hạn suy luận

| Nhóm actor | Seed evaluator smoke | Horizon mỗi mode | Năm mode ghép cặp | Tổng thời gian mô phỏng evaluator trong JSON nguồn |
|---|---:|---:|---|---:|
| Economic | 303002 | 512 quyết định × 7 ngày | baseline, learned-smoke, initial, hold, random legal | 20,722 giây |
| Governance | 303000 | 128 quyết định × 30 ngày | baseline, learned-smoke, initial, hold, random legal | 18,045 giây |
| Civilian | 303001 | 128 quyết định × 30 ngày | baseline, learned-smoke, initial, hold, random legal | 9,228 giây |

Các khoảng thời gian trên là tổng `elapsed_seconds` từng mode trong report, không phải thời gian chất lượng policy hay hiệu năng release. Dữ liệu chỉ có một seed/family; không có CI, selector không được đọc các file này, và mọi kết quả đều mang cờ smoke. Cần chạy đủ 20 world seed paired để chọn; sau đó mới kiểm đúng policy đã khóa trên 100 seed holdout. Không gộp train, validation và holdout.

## 6. Artifact và biểu đồ có thể đối chiếu

Thư mục gốc: [các smoke theo bộ phiên bản khóa](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/). JSON nguồn chứa toàn bộ update metrics và bảng đủ 33 Intent, gồm số lần Intent có ứng viên hợp lệ và số lần được chọn.

### Economic

- [Biểu đồ reward/return](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/economic/economic-smoke.png) · [JSON nguồn](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/economic/economic-smoke.json)
- [Biểu đồ đủ Intent](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/economic/economic-smoke-intents.png) · [JSON nguồn Intent](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/economic/economic-smoke-intents.json)
- [Evaluator smoke chart](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/economic/evaluator-smoke/validation-smoke.png) · [JSON năm mode](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/economic/evaluator-smoke/validation.json)

### Governance

- [Biểu đồ reward/return](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/governance/governance-smoke.png) · [JSON nguồn](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/governance/governance-smoke.json)
- [Biểu đồ đủ Intent](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/governance/governance-smoke-intents.png) · [JSON nguồn Intent](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/governance/governance-smoke-intents.json)
- [Evaluator smoke chart](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/governance/evaluator-smoke/validation-smoke.png) · [JSON năm mode](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/governance/evaluator-smoke/validation.json)

### Civilian

- [Biểu đồ reward/return](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/civilian/civilian-smoke.png) · [JSON nguồn](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/civilian/civilian-smoke.json)
- [Biểu đồ đủ Intent](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/civilian/civilian-smoke-intents.png) · [JSON nguồn Intent](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/civilian/civilian-smoke-intents.json)
- [Evaluator smoke chart](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/civilian/evaluator-smoke/validation-smoke.png) · [JSON năm mode](../../artifacts/m5-current-realm-v15-locked-smoke-2026-10-23/civilian/evaluator-smoke/validation.json)

Các SHA-256 sau áp dụng đúng artifact vừa chạy:

| Nhóm | Tệp | SHA-256 |
|---|---|---|
| Economic | `economic-smoke.png` | `D697DFF2B7AE9E4E71E5532D7DA6B66E13EF37DA97A86E97846BF98123CD2AA0` |
| Economic | `economic-smoke.json` | `5A7E1FF68E51724594040E7C1A7C20E744FC5A4F5D560EB8975DC122E1F4D2B2` |
| Economic | `economic-smoke-intents.png` | `A00F9938DE4CFC6BC27D01281D39A942EA4EE69A624B8000353D19119E4E859D` |
| Economic | `economic-smoke-intents.json` | `3386AC8E0559E077F44A66BFB460E7FF960455147351FFD72697B8D322E681CC` |
| Economic | evaluator `validation-smoke.png` | `0DD2DCBB9A331558323059A33AA1A1013978B895F0EEF6F019FCB4D305FE08AC` |
| Economic | evaluator `validation.json` | `ABCBBE2E8C1E4593C49022A9066EE4BBDFC6ED61C293CDB1CAC452C6B06FF420` |
| Governance | `governance-smoke.png` | `710B18B3C317955102E553DC2ABFCC4018B2B63E6722B7941A51DC8EC7BC2258` |
| Governance | `governance-smoke.json` | `5B51E5574AF54B656C97562D10B5EDE15A04A612B0B7549F108BD9571595696F` |
| Governance | `governance-smoke-intents.png` | `1CA984B21295BF8AF99CF94433A023F85428C21F2EC40533584240774D2BB3F2` |
| Governance | `governance-smoke-intents.json` | `0861BB00A1EFFAF1B40AB4F92CA0FFEEE7A90E939612D3D29DA40D8ADCFB3425` |
| Governance | evaluator `validation-smoke.png` | `5B82545859400244DADF55CE37D2AF71910FB066D38CAD1960F6BFC5CFFA884B` |
| Governance | evaluator `validation.json` | `ABA11A95FAB639FF1D094AF94B05DF6AA15ACD639E6AD81BF5A3364CD61552FC` |
| Civilian | `civilian-smoke.png` | `F5C002B7D0A980B31CE1DD52ADD5167A1EF3183C17A7836D3385146324082EB7` |
| Civilian | `civilian-smoke.json` | `69AA62B14F9726F3A452897A5310174FC00D233A2B432D371B0AD2794BCC385E` |
| Civilian | `civilian-smoke-intents.png` | `383E657D029033859A12061EDDDEA4851537CDD4DB874AF339C9291C6A77FDFE` |
| Civilian | `civilian-smoke-intents.json` | `6590841F02E52980CF84095C18FEE6798C8AF5662C83D6A807501EC48B4D4C96` |
| Civilian | evaluator `validation-smoke.png` | `4227B1EF93AF8B35943FB2C980B302B737513320AD087292A0911A73A41CF7DD` |
| Civilian | evaluator `validation.json` | `B4264C5A6F1744353CB9DEC1F158566B669C5E3B37D5F47E21EDC1C397C7C4CC` |

## 7. Lỗi gặp phải và cách xử lý

1. Python mặc định ban đầu thiếu ONNX/ONNX Runtime. Cài trực tiếp vào Python dùng chung làm pip nâng protobuf và xung đột metadata của một số thư viện vốn có; tôi đã gỡ các gói vừa thêm khỏi Python đó, rồi tạo môi trường riêng cho PolyEc và cài đủ `ai/requirements.txt`. Python dùng chung không bị giữ ở protobuf mới.
2. Evaluator smoke đầu tiên chạy xong mô phỏng nhưng dừng khi ghi JSON vì một giá trị số nguyên từ NumPy chưa đổi sang kiểu Python. Serializer báo cáo nay chuyển rõ `numpy.generic`/mảng sang số/mảng JSON; test bao phủ trường hợp này; evaluator smoke Economic, Governance và Civilian đều đã chạy lại thành công trên bộ dependency khóa.
3. Các kết quả cũ chạy Torch 2.5.1 được giữ nguyên tại thư mục cũ và được phân biệt với bằng chứng mới. Candidate sẽ được tạo bằng venv đúng `ai/requirements.txt`; không dùng artifact cũ làm candidate.
4. Preflight candidate đầu tiên dừng trước rollout vì train-set ghi `trainer.log` bên trong thư mục trainer yêu cầu phải rỗng. Log và runset không hoàn tất được giữ tại `artifacts/m5-v15-r1-candidates-2026-10-23/economic/`; script đã chuyển log ra thư mục `logs/` cạnh các run và có test riêng cho ranh giới đường dẫn.
5. Preflight tiếp theo dừng ở Economic curriculum trước rollout. Curriculum kiểm tra mask tại action slot cố định89; với model v15, action slots thay đổi theo vị trí site và đúng candidate Fiber transfer nằm ở slot101. Observation của lệnh chuyển hàng còn dùng feature thứ ba để biểu thị vùng xưởng, không phải loại hàng. Tôi sửa adapter để kiểm danh sách candidate Intent `TransferEstablishmentGoods` qua mask C++ thay vì suy đoán theo slot hoặc feature vùng. Bộ test tạo đủ bốn context cho cả ba seed training đã khóa; trong mỗi context kiểm transfer bị từ chối khi chưa có Fiber, được phép khi đã mua Fiber và Realm invariant còn đúng sau khi cấp phép. Run lỗi được giữ nguyên tại `artifacts/m5-v15-r1-candidates-2026-10-23-retry1/economic/`; chưa có update PPO hay candidate weights trong hai lần dừng này.

## 8. Bước kế tiếp và giới hạn

Các tệp `realm_train_set_v15.py`, `realm_policy_validation_v15.py`, `realm_select_v15.py` và `realm_policy_holdout_v15.py` đã được viết cùng bộ test. Selector kiểm 3 training seed đã khóa, identity/cấu hình, hash source/run/biểu đồ và chỉ chọn trên validation. Holdout chỉ nhận một bundle đã chọn, 100 seed cố định và không có đường chọn ứng viên thay thế. Khi không policy nào qua, selector ghi `selected_seed=null` và không chép weights vào bundle. Bộ kiểm hiện có 15 assertion thường cùng 36 phép so context curriculum trên ba seed R1; cả hai lần preflight candidate thất bại đều dừng trước rollout, nên không tính là run training.

Bước kế tiếp là chạy lại runset trong thư mục mới sau khi hai lỗi preflight đã được sửa, ba seed candidate × 25 update cho mỗi family, với 16 world × 64 bước/update (25.600 quyết định focal/run); sau đó chạy validation20 seed. Chỉ family có ứng viên qua validation mới chạy holdout100 seed. Biểu đồ training, action Intent, paired utility, outcome và dữ liệu nguồn sẽ được lưu cho đúng run. Chưa có kết quả learned policy; các gate chạy model cùng source trong game/app, Profile B, stress/performance và Windows package cũng còn mở. Không thể gọi M5 hoàn thành dựa trên smoke này.
