# M5 v30 — huấn luyện thăm dò Governance và Civilian bằng Realm v11

## Lát sửa adapter phát hiện khi khởi tạo Civilian

Lượt tạo Civilian đầu tiên dừng trước khi chạy update 1 vì `RealmCoPlayBatch._reset` đọc `world.person(actor)["candidate"]`, trong khi dictionary `person()` của binding không công khai khóa này. C++ đã đưa cờ ứng viên vào observation ở vị trí 102. Sửa đúng một phép đọc trong `ai/training/realm_coplay_env_v11.py` để kiểm tra `world.observe(actor)[102]`; không đổi actor selection, luật Realm, reward, model hay dữ liệu được quan sát bởi policy. Thêm kiểm tra tạo batch Civilian với adapter hiện có, sau đó chạy hai nhóm kiểm tra adapter v11 và một bước khởi tạo/training ngắn để xác nhận lỗi không còn. Lượt runset lỗi được giữ làm bằng chứng lỗi setup, không tính là dữ liệu huấn luyện.

Ngày 2026-10-22. Đây là bước tạo các ứng viên đúng họ cho đánh giá; chưa phải chứng nhận policy chất lượng hoặc hoàn thành RL.

## Mục tiêu

Bundle native cần một policy riêng cho Governance, Economic và Civilian. Các checkpoint có chất lượng được ghi hiện nay chỉ thuộc Economic; thư mục model Realm v11 chưa có bundle đủ ba họ. Chạy trainer đã có cho hai họ còn thiếu để xác định liệu ngân sách và môi trường hiện tại tạo được candidate đáng đưa vào bước validation hay không.

## Dataset được tạo ra như thế nào

Trainer không đọc bảng observation/action dựng sẵn. Với mỗi family và từng seed 1, 2, 3, nó khởi tạo 16 world C++ Realm, dùng seed world bắt đầu từ `2^32 + training_seed × 10.000.000`, nhận observation cùng legal-action mask trực tiếp từ binding C++ trước mỗi quyết định, lấy action theo policy đang cập nhật, gửi lệnh qua cùng Realm command pipeline, rồi lưu reward chuyển trạng thái. Rollout dài 64 bước mỗi update; mỗi update có `16 × 64 = 1.024` quyết định của policy đang học. 25 update tạo 25.600 quyết định cho mỗi seed và 76.800 cho mỗi family. RNG, trạng thái từng Realm, memory và optimizer được ghi vào checkpoint để có thể tiếp tục chính xác.

Các đối tác chưa có checkpoint đủ điều kiện dùng bộ điều khiển dev baseline đã gắn nhãn; không dùng random weights làm đối tác hoặc gọi chúng là learned policy. Bước này dùng full Realm reset hiện tại, không dùng curriculum worksite vốn chỉ dành cho Economic. Dataset là chuỗi tương tác mới sinh trong lúc học, không phải một bảng offline được sao chép hoặc dữ liệu người chơi.

## Cấu hình và ngân sách

- Mỗi family chạy ba seed độc lập: Governance (role 0) và Civilian (role 2).
- 16 environments, 64 rollout steps/update, 25 updates/seed, 64 chuỗi/mini-batch, 3 PPO epochs, learning rate `0.0003`, entropy `0.01`, 6 vùng. Horizon theo mặc định trainer cho Governance và Civilian là **128 ngày** (512 ngày chỉ dùng cho Economic).
- Quan sát preflight trên máy này: update đầu của Governance dùng 42,64 giây (1.024 quyết định), nên cap 300 giây của lần chạy đầu không thể bao phủ 25 update. Sau khi lượt đó ghi trạng thái incomplete, tiếp tục từng seed từ checkpoint bằng đúng trainer `--resume` và cap **1.800 giây mỗi seed**. Nếu seed vẫn chưa đạt 25 update trong cap, giữ nó incomplete và không đưa vào validation. Tổng tối đa là 3 giờ train cho sáu seed, không tính validation; đây là ngân sách compute của run, không phải lịch chạy theo giờ.
- Mọi output mới đặt dưới `build/m5-v30-governance-civilian-training/`; không ghi đè checkpoint/model cũ.

## Kiểm tra và tiêu chí kết luận

1. Preflight Python syntax, ONNX dependency, binding `realm-v11` (1676 observation, 129 action) và trainer/environment checks.
2. Chạy từng runset đủ fixed budget, xác nhận 3 seed/family đều 25 updates và 25.600 quyết định, invariants đúng, `peer_eligible=false`.
3. Nếu mọi seed hoàn tất, validation-only selector đánh giá Governance trên world seed **194000–194019** và Civilian trên **195000–195019**; mỗi nhóm 20 seed mới, tách khỏi các split 190000–190019 và 193000–193019 đã dùng trước. So sánh paired với baseline và initial policy trong horizon 128 ngày. Chỉ selector được quyết định có candidate hay không; training loss hoặc export thành công không đủ.
4. Không chạy 100-world R1 holdout nếu family không qua selector. Không export hoặc gọi bundle đủ chuẩn từ candidate bị loại.
5. Chỉ chạy native benchmark khi có đủ ba candidate đã chọn và cùng contract/config; nếu thiếu bất kỳ họ nào, giữ nguyên chặn tương thích đã ghi ở CURRENT_TASK.

Không sửa gameplay, reward, action, observation, model contract, save hoặc optimizer trong lượt này. Review tự thực hiện. **Kết quả:** cả sáu candidate bị selector loại; chi tiết metric, giải thích cận tin cậy và cách tạo trajectory có trong [report đầy đủ](../reports/M5-v30-governance-civilian-training-evidence-2026-10-22.md). Policy pass/peer promotion, learned cross-play, package và profile B vẫn mở. Lần tiếp theo đã được tách thành [kế hoạch v31](M5-v31-extended-governance-civilian-training-plan.md).
