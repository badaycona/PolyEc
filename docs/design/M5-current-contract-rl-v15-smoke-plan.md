# M5 — một lượt PPO smoke với contract Realm v15 hiện hành

Ngày lập: 2026-10-23. Đây là bước chuẩn bị sau khi chuỗi gameplay quân sự M5.4 đã được ghép và kiểm bằng C++. Mục tiêu là đo một update nhỏ dùng đúng adapter v15 trước khi viết hoặc khởi chạy các lượt tạo candidate nhiều seed. Smoke không tạo policy có thể chọn hay đưa vào game.

## Baseline đã đọc

- `docs/CURRENT_TASK.md`: Realm model v30, household v32, timeline v34, state382, actions264, observation3550; trainer/model/co-play v15 đã có nhưng chưa có policy được chọn bằng validation hoặc holdout độc lập.
- `ai/training/realm_model_v15.py`: model đầu vào3550, encoder đủ382 trạng thái, 264 ứng viên ×12 đặc trưng, GRU64.
- `ai/training/realm_train_v15.py`: từ chối schema/action/observation lệch; `--smoke-only` yêu cầu đúng một update và không cho resume; manifest đánh dấu smoke không đủ điều kiện chọn hoặc phát hành.
- `ai/training/realm_coplay_env_v15.py`: tạo world qua binding C++ và khóa contract/config/timeline fixture trong trạng thái.
- Bài kiểm model, binding và co-play v15 đã chạy ở nhóm adapter trước; không đổi adapter trong nhóm này. Python extension Release tại `build/m5-python-ai/Release/polyec_env.cp312-win_amd64.pyd` có sẵn.
- Artifact PPO duy nhất khớp một smoke trực tiếp thuộc contract schema v27/model v12. Tìm manifest artifact hiện có chưa thấy run v15 tương thích, nên không được dùng run cũ làm smoke v15 hoặc làm policy candidate.

## Phạm vi và dữ liệu được phép tạo

Chạy đúng một PPO update Economic qua C++ binding hiện hành với 16 environment, 64 quyết định mỗi environment, Realm 6 vùng, seed mới `20261024`. Đây là workload nhỏ để đo tốc độ/RSS của trainer trên contract đúng; không gọi đó là train phát hành hoặc benchmark Profile B. Mọi transition/reward vẫn đến từ C++ Realm. Lượt này chỉ dùng peer dev baseline có nhãn nếu không cung cấp peer đã qua gate.

Ghi artifact riêng dưới `artifacts/m5-current-realm-v15-smoke-2026-10-23/run/`: manifest, history tổng hợp, checkpoint/resume state và log. Không xuất bundle `policy.pt`, không tạo selector lock, không đánh dấu peer/runtime eligible. Rollout đầy đủ được giữ trong RAM trong update, không ghi trajectory row-by-row. Thêm một script biểu đồ chỉ đọc đúng `history.jsonl`/manifest của run này, xuất PNG và JSON dữ liệu nguồn cùng thư mục. Hình phải cho thấy số quyết định, reward/return của update và phân bố Intent đã được chọn; ghi rõ một điểm không cho thấy xu hướng hoặc chất lượng policy.

Không tạo split train/validation/holdout trong smoke. Không ghép run khác vào hình; không tiếp tục từ checkpoint cũ; không sửa C++/Python binding/adapter trong bước này. Nếu binding import hoặc contract không khớp, dừng, ghi lỗi và lập kế hoạch sửa adapter riêng trước khi train.

## File và kiểm tra

- Tạo `scripts/plot_realm_v15_smoke.py`, chỉ đọc một run folder và từ chối manifest không ghi đúng schema/model v15 hoặc `smoke_only=true`.
- Tạo một artifact smoke trong thư mục riêng, không ghi đè evidence đã có.
- Chạy `py_compile` cho script; chạy trainer `--smoke-only --updates 1` bằng Release binding có sẵn; xác nhận manifest, một dòng history, checkpoint và số quyết định đủ1024; tạo PNG/JSON từ chính history đó và mở PNG để xem.
- Không chạy lại test toàn C++, build native/GDExtension/Godot hoặc full suite vì nhóm này không đổi source/runtime contract. Nếu trainer phát hiện lệch binding thì dừng trước khi cho kết quả thành công.

## Điều kiện đóng

Smoke chỉ đóng khi model v15 lấy đủ mask hợp lệ từ binding C++, hoàn tất đúng một PPO update, manifest vẫn khóa `smoke_only`/không đủ điều kiện chọn, checkpoint khôi phục được cùng contract và hình/JSON dùng đúng dữ liệu run. Báo cáo phải giải thích bằng tiếng Việt cách tạo1024 quyết định, C++ làm gì, Python làm gì, peer nào dùng, buffer nằm ở đâu, file nào được lưu và vì sao đây chưa phải đánh giá policy.

Sau smoke, bước kế tiếp là selector validation và evaluator paired hiện hành v15. Trước các lượt training candidate nhiều seed phải có plan riêng, validation seed cố định, tiêu chí chọn versioned và dự toán thời gian/RAM từ smoke; holdout giữ riêng và tuyệt đối không dùng để chọn.

## Kết quả chạy

Ngày 2026-10-23, trainer Windows/Python3.12 gọi binding Release có sẵn. Lệnh chọn Economic role1, seed20261024, Realm6 vùng,16 environment,64 rollout step, đúng một update và `--smoke-only`. Build binding không cần lặp. Trainer hoàn tất trong18,60 giây: rollout17,58 giây, optimizer0,92 giây,55,05 quyết định focal/giây, peer baseline tạo29.842 quyết định và peak RSS629.510.144 byte. Hình/reward JSON và hình/phân bố Intent JSON được tạo từ đúng `history.jsonl`/manifest của lần chạy.

Manifest giữ `smoke_only=true`, `policy_candidate=false`, `policy_selection_eligible=false`, `release_eligible=false`, `complete=false`; chỉ có1.024 decision và không có `policy.pt`. Checkpoint đọc lại thành công với Realm schema v30, model contract `realm-worksite-payroll-goods27-config-v34`, update1 và16 C++ world states. Không có train/validation/holdout split hoặc paired evaluation. Chi tiết cách sinh rollout, bảng đủ33 Intent, kết quả và hash là [report](../reports/M5-current-contract-rl-v15-smoke-evidence-2026-10-23.md); artifact nằm trong `artifacts/m5-current-realm-v15-smoke-2026-10-23/`.

Thời gian một update chỉ là ước tính ban đầu cho workload6 vùng/Economic này. Nó không đủ để giả định tốc độ Governance/Civilian, full Atlas hoặc profile B. Không chạy candidate dài ở bước smoke; selector/validation và ngân sách từng family phải được chốt riêng trước training.
