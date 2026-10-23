# M5 — model và môi trường RL cho contract Realm hiện hành

Ngày lập và hoàn tất: 2026-10-23. Đây là nhóm chuẩn bị đúng đường C++→Python cho gameplay contract hiện tại; smoke PPO không phải lượt huấn luyện candidate, chọn checkpoint, holdout hay nghiệm thu policy.

## Baseline và lý do

Audit [readiness](../reports/M5-current-contract-rl-readiness-audit-evidence-2026-10-23.md) cho biết source core hiện khai báo model schema `realm-v27`, `state=295`, `actions=220`, `features/candidate=12`, `observation=2.935`, model household v30/timeline v32. Trainer v11 cũ có model `state=128`,129 action và1.676 observation; check cũ hardcode schema v11 và một observation không khớp. Run v30 dùng contract cũ, bị selector loại cả sáu policy; Civilian v31 dừng ở update43 và chưa validation. Không được resume, đổi nhãn hoặc export các run đó.

## Mục tiêu nhìn thấy

Trainer test có thể dựng một Realm từ binding C++ hiện tại, đưa toàn bộ vùng state cần cho policy cùng 220 candidate hợp lệ vào mạng, lấy hành động chỉ trong legal mask, gọi transition/reward từ C++ và cập nhật buffer/checkpoint đúng seed. Khi đã qua gameplay gate, đây là đầu vào cho run RL phát hành. Bản test chỉ chứng minh adapter/model shape và một update smoke, không chứng minh policy học tốt hoặc có thể thay model runtime.

## Ranh giới triển khai

1. Tạo architecture version mới (`realm_model_v12.py`) với `STATE=295`, `ACTIONS=220`, `FEATURES=12`, `OBS=2935`, hidden64. State đầy đủ đi qua encoder; candidate-scoring giữ chung một scorer 12 đặc trưng/action. Version v11/checkpoints cũ tiếp tục bị từ chối.
2. Tạo wrapper PPO v12, co-play và bộ điều khiển dev có nhãn riêng. Các peer chưa có model đủ chuẩn vẫn dùng dev controller, không gọi là learned. Không viết lại luật Realm, route, settlement hoặc reward bằng Python.
3. Mọi metadata bắt buộc khóa Realm model schema v27, household/timeline contract hiện hành, observation/action, reward version, gameplay-config fingerprint, seed, số environment, horizon và quyết định mỗi update. Resume sai identity bị reject.
4. Smoke nhỏ: một family, batch nhỏ nhất hợp lệ theo PPO, đủ một update kiểm tra sampling → legal mask → `polyec_env`/C++ transition → reward → optimizer/checkpoint/resume. Artifacts được đặt trong thư mục evidence `artifacts/`, đánh dấu `smoke_only`, không tạo bundle runtime, selection lock, holdout hay candidate được chọn.
5. Mỗi smoke phải lưu raw update data/metadata và biểu đồ từ chính run. Dữ liệu smoke không được trộn vào train/validation/holdout phát hành.

## Kiểm ngay và kiểm sau

Kiểm ngay: inspection/static checks cho dimensions/imports; build target `polyec_env` nếu binding source/current binary cần rebuild; Python model shape/loss smoke và một update nhỏ qua đúng C++ binding. C++ transition và action mask phải giữ nguyên; không build Godot/native app khi không đổi adapter đó.

Kiểm sau khi gameplay M5.0–M5.4 đã ổn định: tạo ba seed train/family theo gate; validation-only selector; biểu đồ và dữ liệu gốc từng run; kiểm bundle loader; ghép cặp learned policy, baseline dev và policy ban đầu trên seed holdout riêng. Chỉ chạy training theo ngân sách local đo được từ smoke, không dùng cloud. Không lấy smoke random-initialized làm policy learned.

## Điều kiện đóng nhóm adapter

## Kết quả và bằng chứng

Nhóm đã đóng ngày 2026-10-23. Model đọc state295, 220 candidate và observation2.935; C++ mask giới hạn hành động; co-play dùng cùng C++ transition/reward. Một generic schedule fixture hết hạn sau một tick đã được version hóa để mọi smoke world bật timeline contract v32. Household/timeline model contract, fingerprint, reward, seed, curriculum và fixture version được khóa trong training identity, manifest và trạng thái môi trường.

Windows binding `build/windows/Debug/polyec_env.cp312-win_amd64.pyd` đã sẵn có từ checkout hiện tại; không build lại vì nhóm chỉ đổi Python. `py_compile`, model tensor/sequence/mask test, C++-backed legal transition, co-play save/resume, checkpoint restore, fixture mismatch rejection và legacy v11 bundle rejection đều pass. Một update Economic smoke dùng16 world ×64 bước=1.024 focal decisions; rollout69,05 giây, cả update71,27 giây, bộ nhớ đỉnh551.616.512 byte; peer baseline tạo28.042 decisions. Run chỉ có `smoke_only`; không có bundle policy, candidate selection hay kết quả đánh giá.

Đã sinh chart và JSON từ đúng history/manifest của run-v2. Manifest, history, checkpoint, ảnh và nguồn JSON có SHA-256 đầy đủ trong [báo cáo](../reports/M5-current-contract-rl-adapter-evidence-2026-10-23.md). Báo cáo giải thích seed, cách tạo 1.024 transition qua C++ binding, cách peer baseline hoạt động, buffer RAM so với file được giữ, số liệu chart, hành động có/không có cơ hội, và giới hạn không có train/validation/holdout. Một smoke sơ bộ vẫn được giữ nguyên để truy nguyên; do nhãn `trained_with_rl` ban đầu sai nghĩa so với PPO update đã thực hiện, nó không được dùng làm bằng chứng chính. Source đã sửa và run-v2 chạy lại với metadata đúng.

Sau group adapter, chưa có policy RL đủ điều kiện. Full train chỉ bắt đầu sau khi luật gameplay M5.0–M5.4 ổn định, ngân sách local được tính, và evaluator/selector/holdout đã dùng contract hiện tại. Nhóm gameplay kế tiếp được điều phối trong [CURRENT_TASK](../CURRENT_TASK.md).
