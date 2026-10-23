# M5 — environment PPO với peer policy đóng băng

## Mục tiêu

Tạo training adapter dùng đầy đủ `Realm` C++ cho focal actor và partner decisions. Mỗi PPO step gửi candidate action của focal, rồi mô phỏng 7 ngày (civilian) hoặc 30 ngày (governance/economic). Trong từng ngày, tất cả peer tới lượt theo `Realm.due(family)` chọn action từ policy checkpoint đóng băng của family; actor luôn đọc observation/mask mới ngay trước action. Actor không có checkpoint partner dùng baseline dev có nhãn, chỉ để bootstrap.

Reward, horizon, action legality và ledger giữ nguyên contract v8 `RealmBatch`: reward là normalized focal utility delta qua khoảng giữa hai quyết định focal. Adapter không viết luật kinh tế hoặc migration Python.

## Files

- Thêm `ai/training/realm_coplay_env_v11.py`: adapter batch Python gọi cùng C++ binding và model v11.
- Expose read-only debt/loan summaries và cờ khởi tạo taxing-authority từ cùng Realm binding để outcome PPO giữ chỉ số obligation/default và actor governance đúng vai trò.
- Thêm `ai/tests/realm_coplay_env_v11_checks.py`: kiểm seed/action determinism, focal family/horizon, frozen peer isolation, mask legality, World invariant và reset.
- Thêm plan/evidence riêng; chưa thêm PPO optimizer/trainer trong lát này.

## Invariants

- Peer checkpoint phải là `Policy` v11 đúng family, contract, shape và config fingerprint. Adapter không mutate `state_dict`; `eval()` và `no_grad()`.
- Peer memory/sampling thuộc từng actor và environment; reset khi episode mới hoặc family đổi. Dùng cùng `Realm.step`, `observe`, `mask`, `due`, `advance`.
- Trong một lượt, focal actor không tự chạy lần hai giữa hai PPO action; các actor khác vẫn ra quyết định theo từng ngày. Thứ tự action tuần tự giúp candidate/mask luôn mới.
- Candidate invalid, peer command reject, seed/mask/config sai hoặc invariant fail đều fail-fast. Baseline peer giữ nhãn dev-only.
- Đã qua paired replay/restore cho economic focal actor và frozen economic peer; role governance/civilian cùng cross-family checkpoint chưa được đo trong lát này. Smoke throughput 1 env / 7 ngày cần 7–11 ms Release, 18–20 ms Debug, với 33 peer decisions. Không train dài trong lát này.

## Giới hạn

Self-review only. C++ vẫn sở hữu toàn bộ luật và command validation, nhưng Python điều phối nhiều lời gọi per actor nên có thể chậm. Đo hiện tại chỉ là một transition economic quy mô nhỏ, không đại diện batch lớn hoặc profile B. Checkpoint v11 learned chưa tồn tại; test peer policy dùng weights khởi tạo smoke không được tính learned evaluation. Trainer/resume, cross-family holdout và R1 vẫn chưa được làm.
