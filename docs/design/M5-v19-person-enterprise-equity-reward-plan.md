# M5 v19 — reward tính tài sản ròng của xưởng cá nhân

## Mục tiêu

Đối chiếu review PPO 2026-10-21 với cặp Realm sampled holdout: policy Economic đã tạo xưởng nhưng reward cá nhân chưa tính tiền và hàng của xưởng do chính cá nhân sở hữu. Bổ sung equity của xưởng vào wealth, trừ nghĩa vụ lương site đã phát sinh, rồi khóa rõ phiên bản reward trong trainer và mọi loader model.

## Phạm vi

- `core/include/polyec/realm.hpp`, `core/src/realm.cpp`, `core/src/realm_establishment.cpp`: phiên bản reward, chỉ mục dẫn xuất các xưởng Person-owned, tính equity cho Economic và wealth goal của Civilian. Public/Organization asset không phải tài sản cá nhân.
- `core/tests/test_realm.cpp`: kiểm chuyển tiền/hàng qua command, phân biệt tài sản chủ sở hữu, payroll liability, load rebuild và invariant chỉ mục.
- `bindings/python/realm_binding.cpp`: expose phiên bản reward của C++.
- `ai/training/realm_train_v11.py`, `ai/training/realm_coplay_env_v11.py`, `ai/evaluation/realm_policy_holdout_v11.py`, `ai/evaluation/realm_select_v11.py`, `ai/evaluation/realm_coplay_v11.py`: ghi reward version vào resume identity/manifests/bundle; reject run/peer không cùng phiên bản.
- Không đổi actions, observation, model tensor, lệnh, save schema, công thức sản xuất, phân bổ payroll hoặc luật sở hữu.

## Tính toán và bất biến

- Equity mỗi xưởng cá nhân = cash + canonical stock nhân reference price − nghĩa vụ lương site chưa trả theo attendance, work-credit, paid-unit và wage remainder ledger hiện hành.
- Chỉ số `owner -> establishment indices` là dữ liệu dẫn xuất: tạo xưởng cập nhật trước khi publish mutation; deserialize dựng lại từ bản tạm trước invariant/swap; invariant xác nhận chỉ mục khớp chính xác với vector xưởng.
- Utility chỉ thăm các xưởng của actor. Không quét toàn vector xưởng cho mỗi quyết định.
- Economic luôn tính equity; Civilian chỉ cộng equity vào wealth thành phần goal hiện hành. Governance giữ nguyên.
- Reward constant `realm-v11-person-enterprise-equity-v1`; cũ/mất trường bị từ chối khỏi selection, peer training và cross-play. Resume đòi identity khớp.

## Kiểm chứng

- Core Release test kiểm số học thủ công và ownership bằng command thật: vốn chuyển vào site bảo toàn equity; chuyển Fiber từ inventory sang site bảo toàn equity; owner khác, Public và Organization không nhận equity cá nhân; tiền lương chưa trả làm equity giảm; deserialize tái lập chỉ mục.
- Build binding rồi kiểm Python export và fail-closed loader trên manifest không có/sai reward version.
- Chạy paired economic 3-seed, fixed decision budget, holdout mới khác tập đã xem để chẩn đoán tín hiệu; nếu còn kém thì giữ rõ kết quả và không cấp peer eligibility. Đây không thay R1: R1 vẫn cần 3 training seeds, full horizon và 100 thế giới độc lập.

## Giới hạn review

Plan dựa vào review PPO được người dùng chỉ định và evidence paired full-Realm đã chạy. Codex sẽ tự triển khai và self-review theo gate tích hợp; đây không phải review độc lập OpenCode. Không thay thế audit các thiếu sót khác của PPO: partner diversity, curriculum transfer và quyết định marriage target vẫn mở.
