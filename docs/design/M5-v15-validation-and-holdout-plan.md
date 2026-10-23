# M5 — đánh giá, chọn checkpoint và holdout cho Realm v15

Ngày lập: 2026-10-23. Phạm vi tiếp theo sau adapter smoke v15. Mục tiêu là thay evaluator/selector v11 đã khóa schema cũ bằng một pipeline chỉ nhận model/trainer hiện hành, tạo ba run có ngân sách bằng nhau cho từng họ actor, chọn bằng validation và chỉ sau đó mới mở đánh giá holdout độc lập.

## Baseline đã kiểm

- Realm hiện hành: schema `realm-v30`, model contract `realm-worksite-payroll-goods27-config-v34`, household v32, timeline v34, state382, actions264, observation3550, reward `realm-v13-person-enterprise-equity-v1`.
- `realm_model_v15.py`, `realm_train_v15.py`, `realm_coplay_env_v15.py` và controller dev baseline v15 là adapter hiện hành. Model/trainer/evaluator v11 và artifact v30/v31 cũ không tương thích; không sửa manifest, resume hoặc export chúng.
- Một PPO smoke v15 đúng contract vừa hoàn tất:16 world×64 bước=1.024 quyết định, một update Economic. Nó chỉ đo adapter và tài nguyên; không có run candidate.
- Selector cũ yêu cầu Realm v11; không có evaluator validation/holdout v15.
- Source of truth cho ngưỡng chất lượng là IMPLEMENTATION_PLAN §8/R1. Các báo cáo cũ cho biết cách tính paired utility, outcome và CI; plan này chuyển chúng sang contract hiện hành mà không hạ chất lượng.

## Seed split được khóa trước code và training

Đặt tên split `realm-v15-r1-seed-split-1`:

- Training seeds cho từng họ actor: `20261101`, `20261102`, `20261103`.
- Validation world seeds, chỉ dùng cho chọn: `303000`–`303019` (20 thế giới).
- Holdout world seeds, chỉ dùng sau khi validation đã ghi `selected_seed`: `404000`–`404099` (100 thế giới).

Các dải không giao nhau; script từ chối trùng seed và ghi đầy đủ split vào runset/selection/report. Trainer từ mỗi training seed phát sinh các C++ world seed riêng theo công thức `2^32 + training_seed × 10.000.000 + index`; evaluator dùng world seed validation/holdout được truyền trực tiếp và cùng seed actor cho từng mode paired. Không thay seed theo kết quả. Không chọn policy hoặc siêu tham số bằng holdout.

## Ngân sách và cố định cấu hình

R1 dùng profile sáu vùng và full focal horizon hiện hành: Economic512 quyết định×7 ngày mỗi quyết định; Governance/Civilian128 quyết định×30 ngày. Mỗi candidate có đúng25 update, 16 environment và64 quyết định/environment/update (25.600 focal decisions/run), cùng3 epoch, learning rate3e-4, entropy0,01, sequence16/minibatch64 và CPU. Ba run của cùng family phải giống cấu hình, contract, curriculum, peer labels/bundle hashes, reward/config fingerprint và budget; chỉ seed khởi tạo khác.

Economic dùng curriculum `economic-worksite-reset-v1` đã có version; Governance/Civilian dùng `none`. Peer khác family dùng dev baseline có nhãn nếu chưa có peer đã qua gate; không trộn peer thay đổi giữa các seed. Không resume smoke hoặc checkpoint cũ. Mỗi training run lưu chart reward/return theo update/quyết định và dữ liệu nguồn riêng; chart Intent/opportunity cũng cùng run. Validation/holdout lưu raw kết quả từng seed, paired differences và chart riêng đúng protocol.

Các smoke theo phiên bản đúng `ai/requirements.txt` đã đo được: Economic13,307 giây/update, Governance49,115 giây, Civilian57,432 giây; peak RSS lần lượt555.204.608,558.235.648,558.346.240 byte. Mỗi smoke dùng16 environment×64 bước=1.024 quyết định, một update. Phép nhân 9 candidate runs×25 update cho khoảng8.989 giây CPU huấn luyện theo workload smoke; đây là ước lượng compute, không phải thời hạn và chưa gồm validation/holdout. Chi tiết run, seed, số liệu, file nguồn, biểu đồ và hash: [báo cáo evaluator smoke/budget](../reports/M5-v15-r1-evaluator-smoke-and-budget-evidence-2026-10-23.md). Chạy local CPU, tuần tự; không tự mua cloud/GPU. Nếu RAM/đĩa không đủ cho checkpoint/run evidence, giảm số công việc đồng thời, không giảm sample requirement đã khóa.

## Luật paired evaluation và selector

Mỗi world seed được chạy riêng cho các mode dev baseline, policy ban đầu (initial), policy đã train, hold và random legal; từng mode bắt đầu từ cùng seed, draft, config/timeline fixture và RNG actor. Các actor ngoài family đang đo vẫn dùng peer/controller được ghi trong manifest. Thước đo gồm normalized focal utility, sống sót/sức khỏe/tiền/nợ/insolvent, nghĩa vụ tới hạn/default, và phân bố Intent/opportunity. Invariant phải đúng ở mọi world.

Validation selector cần đúng3 run distinct, cùng một budget/cấu hình, mỗi run đủ25 update và20 validation seed. Trên mỗi candidate, paired 95% CI phải đồng thời thỏa:

1. utility learned so với dev baseline: cận dưới `>= -0.05` trên normalized utility (không kém hơn baseline quá5%); 
2. utility learned so với initial: cận dưới `> 0` (có cải thiện so trọng số ban đầu);
3. alive/survival so với baseline: cận dưới `>= 0`;
4. insolvent, defaults và nghĩa vụ đến hạn so với baseline: cận trên `<= 0`.

CI dùng paired differences theo world seed, Student t hai phía với95%; evaluator ghi số mẫu, trung bình, độ lệch và hai đầu CI. Với n20 validation và n100 holdout, critical values được khóa trong code theo split version; không dùng CI nếu mẫu thiếu hoặc outcome không hữu hạn. Nếu nhiều run qua, chọn paired utility mean so với baseline cao nhất. Nếu không run nào qua, ghi `selected_seed=null` và không tạo bundle. Bundle chọn từ validation chỉ có `peer_eligible=false` cho tới khi holdout đạt.

Holdout evaluator chỉ nhận đúng seed được ghi trong selection lock; không nhận danh sách candidate để chọn lại. Dùng100 seed riêng, paired với baseline/initial/hold/random; báo đủ outcome và CI. Chỉ đánh dấu `peer_eligible=true` khi selected run đồng thời qua R1 trên holdout và selection record/config/hash còn khớp. `release_eligible` vẫn false cho tới khi gate runtime/app/performance/package liên quan được xác nhận. Holdout thất bại thì giữ kết quả thất bại; không đổi model, reward, threshold hoặc split rồi gọi cùng100 seed là mới.

## Thành phần đã triển khai và lượt kiểm

- `ai/training/realm_train_set_v15.py`: đã viết để chạy 3 seed tuần tự cho mỗi family, khóa config/source/dependency phiên bản vào runset manifest, gọi trainer và vẽ dữ liệu đúng từng run. Runset kiểm output folder trống, exact seeds, budget, checkpoint và source hash giữa các run; log trainer được đặt ngoài thư mục output riêng của mỗi run.
- `ai/evaluation/realm_coplay_v15.py`: runner dùng binding C++ hiện hành, fixture timeline v34, paired actor seed, full horizon, action/Intent counts và invariant.
- `ai/evaluation/realm_policy_validation_v15.py`: loader từ chối contract/budget/checkpoint không khớp; chạy20 fixed validation seed; lưu dữ liệu từng world, paired CI95 và ba chart riêng cho utility, kết quả sinh tồn/rủi ro và phân bổ Intent.
- `ai/evaluation/realm_select_v15.py`: yêu cầu 3 run cố định và so khớp hash manifest, weights, history, chart và JSON nguồn; chỉ đọc validation; chọn mean baseline cao nhất trong candidate đạt đủ điều kiện. Không đạt thì `selected_seed=null`, không chép policy/initial vào bundle.
- `ai/evaluation/realm_policy_holdout_v15.py`: nhận một bundle duy nhất, kiểm validation report/chart/weights/seed lock; chạy đúng100 holdout seed; không có tham số nhận danh sách ứng viên; chỉ đổi `peer_eligible` sau khi chính model khóa đạt R1.
- `ai/tests/realm_validation_v15_checks.py`: 15 assertion thuần logic qua cho seed split, CI và mẫu thiếu, ngưỡng R1, serializer NumPy, không candidate, ranh giới đường dẫn output/log, reject schema cũ/smoke, charts, chọn winner, không chọn candidate thấp hơn, seed không trùng và hash weights thay đổi; thêm36 kiểm tra context curriculum trên bốn context của cả ba seed R1. `py_compile` qua cho evaluator, selector, holdout, adapter, runset, plotter và test.

Smoke train và evaluator smoke đã chạy cho cả ba family trên đúng environment requirement; mỗi family có một seed evaluator riêng. Mỗi run có biểu đồ cùng JSON nguồn, nhưng đều được đánh dấu `smoke_only` và không được dùng làm candidate hoặc bằng chứng chất lượng. Có một lỗi JSON NumPy lúc đầu đã sửa; evaluator smoke của cả ba family được chạy lại sau sửa. Toàn bộ nội dung và hash ở [báo cáo smoke/budget](../reports/M5-v15-r1-evaluator-smoke-and-budget-evidence-2026-10-23.md).

Đã xong kiểm tra trước candidate: `py_compile`, 15 assertion và36 kiểm curriculum, một update train smoke/family, evaluator smoke một world/family và biểu đồ/JSON nguồn. Hai preflight candidate thất bại trước rollout, đã giữ log/runset: một do ghi log trong thư mục trainer cần rỗng; một do curriculum giả định transfer slot89 cố định trong khi candidate hợp lệ là slot101. Train-set chuyển log ra ngoài run; adapter kiểm candidate transfer Intent qua mask C++ thay vì slot cố định. Đã chạy lại curriculum cho đủ bốn context trên từng seed 20261101/02/03 và tất cả qua; chưa có training update candidate nào được tính. Không build full C++/app vì không đổi core, binding hoặc API. Bước hiện tại: khởi chạy lại runset candidate ở thư mục mới với ba seed×25 update/family tuần tự; sau đó chạy validation20 seed và selector. Chỉ khi family có candidate đạt validation và selection lock/hash khớp mới chạy holdout100 seed. Không dùng kết quả smoke để chọn policy.

## Điều kiện đóng nhóm

Đóng phần triển khai khi test chứng minh selector từ chối đúng tất cả mismatch/thiếu run, không đọc holdout, ghép cặp cùng world/actor seed, tính đúng CI/outcome, và không tạo bundle nếu không đạt. Đóng kết quả chất lượng riêng chỉ khi đã có3 run đủ budget, validation chọn hoặc không chọn được, sau đó holdout riêng100 seed. Nếu nguồn lực không đủ hoặc các candidate bị loại, ghi rõ run đã tiêu thụ, raw data/chart và vẫn giữ M5 mở; không gắn nhãn learned-policy complete.
