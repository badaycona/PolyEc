# M5 v19 — tài sản ròng của xưởng cá nhân và PPO Economic

## Kết quả với người chơi và NPC

NPC Economic giờ tính tiền và hàng trong xưởng do chính họ sở hữu vào tài sản cá nhân; nghĩa vụ lương xưởng chưa trả được trừ khỏi phần đó. Chuyển vốn hoặc Fiber giữa ví cá nhân và xưởng không tự làm thay đổi mức giàu tính cho reward. Xưởng công hoặc của tổ chức không trở thành tài sản riêng của NPC. Không đổi lệnh, luật giao dịch, tiền trong sổ, save schema, observation hay kích thước model.

Đây là sửa cách chấm reward cho quyết định PPO, không phải tính năng UI/gameplay mới. Checkpoint dùng reward cũ bị từ chối khỏi train resume, selection, validation và co-play.

## Bằng chứng triển khai

- Kế hoạch: [M5 v19 plan](../design/M5-v19-person-enterprise-equity-reward-plan.md). Phạm vi code tự review bởi Codex; không có review độc lập từ OpenCode.
- Source/run SHA-256 và kết quả gate: [manifest](../../artifacts/m5-v19-person-enterprise-equity-reward-2026-10-21-manifest.json).
- Chỉ mục dẫn xuất ánh xạ chủ sở hữu cá nhân sang các xưởng được cập nhật khi tạo, mở rộng khi có người mới sinh, dựng lại khi đọc save và đối chiếu trong invariant.
- Core Release build thành công với Visual Studio 18, cấu hình `build/m5-core`.
- CTest Release: 18/18 pass; sau khi thêm fixture kiểm nghĩa vụ lương, `realm_tests` chạy lại riêng và pass. Fixture dùng lệnh tạo xưởng, cấp quyền lao động, giao ca một ngày, payroll thật và kiểm utility theo lượng lương còn nợ.
- Python binding Release build thành công tại `build/m5-python-ai`; export `REALM_REWARD_VERSION=realm-v11-person-enterprise-equity-v1`.
- Co-play/resume Python: 22 checks pass; kiểm metadata peer, resume cùng reward version và reject reward cũ. Python syntax check cho trainer, adapter và evaluator pass.
- Checkpoint smoke mới (1 update, 1.024 quyết định) được holdout loader nhận; loader từ chối run cũ không có reward version. Paired sampled smoke 2 thế giới/1 ngày giữ `r1_pass=false`, đúng vì thiếu ngân sách nghiệm thu.

## Thí nghiệm Economic cố định

Ba seed 1/2/3, mỗi seed 25 update × 1.024 quyết định = 25.600 quyết định; 16 Realm environment, 6 vùng, rollout 64, 3 epoch, learning rate 0,0003. Đối tác là dev baseline có nhãn; không có learned peer đạt chuẩn.

Sampled paired validation dùng 20 thế giới mới 191000–191019, 128 ngày, cùng world seed và sampling seed giữa baseline/learned. Cả ba run giữ invariant. Kết quả:

| Seed | Utility so baseline (CI95) | Utility so initial (CI95) | Tiền so baseline | Nợ so baseline | Mất khả năng thanh toán / thế giới | Xưởng bình quân | Ca công được trả | Cloth cuối kỳ |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | -0,439 [-0,479; -0,398] | +0,068 [+0,020; +0,115] | -381 | +88,8 | +0,50 | 20,8 | 11,8 | 0,25 |
| 2 | -0,460 [-0,503; -0,417] | +0,046 [+0,001; +0,091] | -380 | +93,8 | +0,65 | 21,4 | 11,0 | 0,25 |
| 3 | -0,470 [-0,501; -0,438] | +0,030 [-0,004; +0,064] | -385 | +109,2 | +0,70 | 20,0 | 14,2 | 0,25 |

Các policy tạo xưởng và có thao tác cấp quyền/giao ca, nhưng lượng công được trả thấp và tồn Cloth gần bằng 0. Reward mới cải thiện so với initial ở seed 1–2 và làm điểm so baseline bớt âm so với thí nghiệm trước, nhưng vẫn kém baseline rõ rệt; tiền, nợ và mất khả năng thanh toán đều xấu hơn. Selector không chọn seed nào; `peer_eligible=false`. Chưa chạy R1 holdout dài vì không có ứng viên đủ điều kiện.

Đây là bằng chứng chẩn đoán trên 20 thế giới validation, không phải R1 hoặc nghiệm thu learned policy. R1 còn yêu cầu đủ horizon quy định và ít nhất 100 thế giới độc lập; selection cần vượt điều kiện non-inferiority so baseline, cải thiện initial và an toàn kết quả.

## Giới hạn và bước sau

Reward version mới đã đưa giá trị xưởng vào mục tiêu học, nhưng chỉ số KL sau optimizer vẫn rất thấp (thường 10⁻⁶), clip fraction 0 và critic explained variance dao động; reward change chưa tạo policy Economic đủ tốt. Các hạn chế khác trong [PPO review](../handoff/PPO-training-review-2026-10-21.md)—đối tác học đa dạng, curriculum có can thiệp, chuyển giao sang tổ hợp holdout—vẫn mở. Bước kỹ thuật tiếp theo là thử riêng độ chia mini-batch PPO với sample budget cố định, đo trực tiếp KL/coverage/xưởng và đánh giá paired trên thế giới chưa xem. Không tăng ngân sách mặc định hoặc cho phép checkpoint này làm peer.
