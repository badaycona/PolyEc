# M5 — action policy cho vật tư và ca tại xưởng

## Hành vi cần nối

Policy phải có thể tiếp tục từ action v10 tạo cơ sở: chuyển nguyên liệu/sản phẩm, cho phép actor làm ở site mình quản lý, gửi ca Work gắn đúng site và tiến mô phỏng qua production/payroll. Mọi thao tác đi qua các typed Realm command đang dùng ở Python/Godot; policy không sửa balance trực tiếp.

## Action contract

- Chuyển thành `realm-v11`: 129 actions/1.676 observations. Giữ nguyên 89 slot trước; slots 89–128 là 8 dòng site × 5 thao tác.
- Mỗi dòng site có: nạp input 1 theo recipe/config; nạp input 2 nếu recipe có; rút output; cấp quyền Work cho một worker khác đang sống và có mặt tại site trong 31 game-day; gửi ca Work một ngày game-time.
- Core chọn tối đa 8 site khác nhau, ổn định theo actor/sampling/day, không đổi state. Observation cho mỗi row thấy industry, vùng, tồn kho đầu vào/đầu ra, cash/capacity, skill/tuổi worker được chọn và trạng thái quyền; không đưa ID worker vào feature của policy.
- Các slot không có site/không dùng input 2 là mask 0, không tạo alias Hold. Legal mask kiểm tra cùng quyền, vị trí, số dư/tồn kho và thời gian như typed command; candidate execute dispatch qua typed command, nơi xác thực lại trước khi đổi state.
- Candidate/observation tiếp tục hoạt động sau `timeline_contract` bật. Model contract v10 bị từ chối; model contract hiện chuyển sang v5. Không tạo trainer/checkpoint v11.

## Invariants và kiểm thử

- Transfer chỉ chạm đúng hai account, quantity recipe/config; không mint goods/money, reject giữ fingerprint.
- Authorization actor phải quản lý site và hiện diện đúng region; worker candidate phải sống và có mặt; Work phải nằm trong window, cùng region/site, không overlap; stale/replay atomic.
- Một production day tiêu đúng input, thêm đúng output theo capacity/recipe, payroll đúng attendance; save giữa ca rồi load/tiếp tục cho cùng ledger/fingerprint với uninterrupted run.
- Core Release/Debug, Python Realm/RealmBatch, native manifest, Godot compatibility và typed command replay. Có oracle tính tay cho một lô Cloth: buy Fiber → deposit → authorize → Work 64 ticks → một ngày production → withdraw Cloth.
- Self-review theo diff/evidence; chưa huấn luyện dài. R1/cross-play vẫn giữ mở cho tới khi có policy learned và holdout đủ đại diện.
