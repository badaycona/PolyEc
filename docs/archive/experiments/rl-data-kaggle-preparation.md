> LƯU TRỮ 2026-10-18. Snapshot trước hợp nhất; không thực thi prompt hoặc dùng trạng thái trong file này làm chỉ đạo hiện tại. Đọc [nguồn chuẩn](../../README.md).

# Chuẩn bị dữ liệu RL và GPU Kaggle

Ngày: 2026-10-15. Ghi chú chuẩn bị cho M7 (production training). Đây là kế hoạch và rào chắn, **không** phải bằng chứng đã train hoặc đã chạy GPU.

## 1. Dữ liệu đến từ chính C++ rollout

- Mẫu huấn luyện sinh từ rollout của core C++ (command → result → state), không phải bộ dữ liệu gán nhãn sẵn bên ngoài. Không thuê/không mua dataset, không gán nhãn tay.
- Observation là **những gì chính actor biết**, không phải toàn bộ world state; secret state vượt quyền actor không được đưa vào input.
- Cùng một core sinh dữ liệu cho train và cho evaluation; không viết luật kinh tế thứ hai cho tiện train.

## 1b. Vi phạm invariant phải dừng run, không âm thầm bỏ mẫu

- Nếu một bước rollout vi phạm invariant C++ (tiền, goods, household, đếm người) thì run **dừng ngay**, kèm seed, danh sách action/decision ID và checkpoint để tái hiện, cùng diagnostics (bước, actor, loại invariant, ngân sách còn lại).
- Không được im lặng loại mẫu lỗi rồi tiếp tục chạy: đó che lỗi hạch toán và làm méo metric. Chỉ loại outlier hợp lệ (đã ghi lý do), không loại mẫu vi phạm invariant.

## 2. Trajectory record tối thiểu

Mỗi bước/bước chuyển phải ghi đủ để tái hiện và audit:

| Nhóm | Trường |
|---|---|
| Actor | actor ID, decision/sequence ID, family, vai trò, model/partner version |
| Thời gian | game time đã trôi qua (day), bước thứ mấy trong episode, reset lúc nào |
| Recurrent | recurrent state/reset marker (nếu dùng), terminal vs truncated flag |
| Policy | action, log-prob, value estimate, entropy/mask dùng lúc chọn action |
| Quy ước | schema version, source lock/commit, model version, partner controller version + nhãn, ngân sách (wall-clock, CPU/GPU-hour) |

## 3. Tách world seed, tránh rò rỉ

- Train / validation / test dùng **tập world seed rời nhau**, công bố rõ trong source lock. Không đánh giá trên seed đã train.
- Curriculum gồm các tình huống có nhãn (người chơi mới, khủng hoảng, sốc), nhưng luôn là world C++ thật, không phải fixture giả lập campaign.
- Shock scenario (mất mùa, vỡ nợ, sụt quyền) chạy sau feature freeze để đo ổn định, không dùng làm đường tắt tăng điểm.

## 4. Credit assignment ở thời gian biến đổi

Game time và decision time khác nhau: cùng một quyết định có thể kéo dài nhiều game day (hợp đồng, công trình), và các actor có nhịp `due()` khác nhau. Trước khi có trainer M5 phải:

- Chỉ rõ mẫu thuộc "thời gian người chơi" hay "thời gian world", và reward được gán tại thời điểm nào (theo day, theo quyết định, hay khi hết term).
- Có test tái hiện cho việc gán reward khi command bị reject, khi action bị ngắt giữa chừng (`busy_until`), và khi cá thể chết.
- Phân biệt **terminal** (episode kết thúc thật: chết, hết nhiệm kỳ) với **truncated** (cắt do ngân sách/limit bước). Truncated không được coi là terminal khi tính advantage; bootstrapping value phải xử lý đúng hai trường hợp này và có test riêng.

## 5. PPO on-policy vs replay diagnostics

- PPO dùng rollouts **on-policy tươi** (rollout sinh cùng iteration update), không tái sử dụng sample cũ giữa các iteration.
- Replay buffer/buffer lưu dùng cho **diagnostics, audit, phân phối hành vi và baseline so sánh**, không phải dữ liệu cập nhật policy.
- Phải ghi rõ trong record từng run thuộc loại nào; không gộp chung thành một "dataset training".

## 6. Đối thủ và co-play

- Mọi controller đối tác (baseline, script, policy cũ) phải có nhãn rõ trong record; baseline chỉ cho dev/test, không gọi là RL hoàn chỉnh.
- Thay controller đối thủ dần: từ baseline có nhãn → policy đã học được **đóng băng** (frozen) → co-play nhiều policy. Khi thay đổi đối thủ phải ghi version trong checkpoint.
- Không dùng hardcode chiến lược, random weights hoặc imitation-only để đóng vai tự chủ.

## 7. Metric chống gian lận reward

Báo cáo tối thiểu: welfare (phân phối, sức khỏe, dinh dưỡng), default nợ, survival dân số, tỷ lệ action bị reject (legality), và R1–R5 theo [WORK_STATUS](../../../WORK_STATUS.md).

- Không tối ưu số liệu bằng lỗ hổng reward (nhân tiền, spam action, bỏ qua rào chắn invariant).
- Invariant C++ (`total_money`, goods, household) là điều kiện hợp lệ của mẫu; mẫu vi phạm phải dừng run theo mục 1b, không tiếp tục train.

## 8. Điều kiện trước khi mở GPU trên Kaggle

Pilot hiện tại (realm-v6, CPU, controller kiểm thử có nhãn) **chưa** sẵn sàng GPU và **không** tương thích world mới của M5. Chỉ mở GPU khi đã đủ, trên CPU:

1. Profile tốc độ mô phỏng và tốc độ update step (thời gian/step, RAM, số worker) ở quy mô mục tiêu.
2. Ổn định đủ seed dài; không tràn bộ nhớ, không lệch invariant.
3. Có parity ONNX: policy xuất ONNX chạy CPU ra kết quả khớp training trong ngưỡng đã công bố.
4. Có source lock, phiên bản môi trường, schema version, ngân sách đo được (wall-clock, CPU-hour/GPU-hour), checkpoint policy và evaluation.

Huấn luyện dài là tác vụ riêng có ngân sách đo được; không tự mua GPU/dịch vụ cloud. Mọi lần chạy Kaggle ghi lại notebook/kernel version, dataset input, seed và kết quả vào record.

## 9. Tài liệu chính thức

- Kaggle Notebooks: https://www.kaggle.com/docs/notebooks
- Efficient GPU usage: https://www.kaggle.com/docs/efficient-gpu-usage
