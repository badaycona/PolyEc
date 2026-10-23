# M5 v13 — môi trường train có peer policy đóng băng

## Thay đổi

`ai/training/realm_coplay_env_v11.py` thêm adapter PPO cho một actor trọng tâm trong mỗi Realm C++. Sau candidate action của actor, Realm tiến 7 ngày với vai economic hoặc 30 ngày với governance/civilian. Mỗi peer tới lượt được đọc observation và legal mask mới, rồi chọn candidate bằng policy family đã đóng băng. Peer giữ hidden memory và bộ sinh số riêng theo actor; checkpoint được kiểm tra family, schema, số observation/action, hidden size, model contract và config fingerprint. Khi thiếu checkpoint, adapter dùng baseline dev có nhãn.

Python chỉ phối hợp lượt actor, tính utility delta và thu outcome; rule, legality, giao dịch, nợ, vật tư và thay đổi world vẫn chạy trong C++ Realm. Binding bổ sung `debt()`/`loans()` read-only và cho phép bật `taxing_authority` khi dựng Realm, đồng thời giữ `config_path` hiện có.

## Kết quả chạy

- Build `polyec_env` Windows Release và Debug đều exit 0.
- `ai/tests/realm_coplay_env_v11_checks.py`: Release 14/14 và Debug 14/14.
- Kiểm đã chạy: schema/action/observation v11; focal economic family; legal Hold; replay deterministic giữa hai Realm; frozen policy tham gia thật vào lượt peers; replay peer đồng nhất; serialize/restore rồi chạy tiếp cho kết quả giống; mask legality và Realm invariant; weights nguồn không đổi; tham số frozen `requires_grad=False`; sai family metadata bị từ chối.
- Một transition economic, 2 vùng, 128 người, 7 ngày: 33 peer decisions; lần chạy cuối 7 ms Release, 18 ms Debug (lượt trước đo 11/20 ms). Con số này không gồm khởi động Python/Torch và không đại diện throughput batch lớn hoặc profile B.

## Giới hạn và việc tiếp theo

Đây là self-review trên shared dirty tree. Peer model trong smoke là trọng số khởi tạo ngẫu nhiên để xác nhận đường inference, không phải learned checkpoint. Chưa có checkpoint v11 được train, trainer v11, resume optimizer/RNG/progress, cross-family holdout, sampled R1 hay benchmark profile B. Chưa bắt đầu training dài. Tiếp theo cần hoàn tất trainer có đường nạp checkpoint đúng contract và resume đầy đủ, sau đó đo throughput theo batch/role trước khi phân bổ budget training.
