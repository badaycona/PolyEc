# M5 v17 — đồng bộ dev peer controller giữa train và đánh giá

## Mục tiêu

Khi chưa có learned checkpoint, dùng cùng labeled dev baseline cho peer governance/economic trong rollout và evaluation. Controller chỉ đọc observation/mask cùng candidate features, chọn trong legal actions, và không được gọi là learned policy. Civilian tiếp tục dùng dev baseline hành động Work/Study.

## Phạm vi

- Sửa `_baseline_peer` trong `realm_coplay_env_v11.py` để trùng baseline controller ở `realm_coplay_v11.py`.
- Version hóa baseline trong manifest và resume identity để run không trộn giữa các định nghĩa.
- Thêm checks cho mask legality, family-specific behavior, deterministic replay và invariant.
- Đánh giá lại training quality từ đầu; kết quả ba run baseline đều trước khi đổi peer controller chỉ là chẩn đoán lịch sử, không được gộp với run mới.

## Không đổi

C++ Realm vẫn sở hữu mọi luật/validation. Baseline chỉ là dev/test peer; quyết định tự chủ sản phẩm vẫn cần learned policy. Không dùng baseline làm output release.

## Điều kiện nghiệm thu

1. Mỗi family baseline trả candidate hợp pháp từ observation/mask hiện tại.
2. Replay cùng seed deterministic và Realm invariant pass.
3. Trainer manifest/resume ghi đúng baseline version; mismatch bị từ chối.
4. Smoke 25-update chưa được gọi là quality pass; holdout R1 riêng vẫn bắt buộc.

Self-review only trên shared dirty checkout.
