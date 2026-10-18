# Artifact thí nghiệm — không phải tài liệu điều hành

Đọc [bộ docs hiện hành](../docs/README.md) trước. Artifacts giữ sản phẩm của run theo phiên bản, không đại diện tự động cho implementation/model active.

- `models/`: checkpoint, config, manifest, evaluation, SOURCE/SELECTION LOCK và source snapshots. Realm-v1…v6 là lịch sử/pilot, gồm cả kết quả chưa đạt. Không chạy code trong source snapshot để phát triển world mới.
- `visuals/`: ảnh, JSON/profile, logs và biểu đồ của workload/platform cụ thể. Một ảnh không chứng nhận toàn gameplay hoặc hiệu năng hiện tại.
- Model app thực sự nạp nằm trong `godot_project/models/` theo manifest/hash. Không giả model gần đây nhất trong artifacts là model tương thích world mới.

Không di chuyển/xóa artifact bừa: script training còn dùng các đường dẫn như `artifacts/models/realm-v3` để resume và các report dùng evidence ảnh/profile. Chưa có tác vụ kiểm kê dependency/xóa model, nên lần tổ chức docs này giữ nguyên toàn bộ binary, source lock và số liệu. Quản lý chúng theo run; muốn đọc kết luận xem [HISTORY](../docs/DEVELOPMENT_HISTORY.md) và [dev paper](../docs/DEVELOPER_GUIDE.md), muốn tiếp tục code xem [CURRENT_TASK](../docs/CURRENT_TASK.md).
