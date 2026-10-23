# Bằng chứng — nguồn gốc quan hệ cha–con cho truy vấn di sản

Ngày 2026-10-21. Plan: [M5-core-legal-parentage-provenance](../design/M5-core-legal-parentage-provenance-plan.md).

## Thay đổi

- `Edge` mang cờ `legal_parentage`. Cạnh `ParentChild` được tạo bởi sự kiện sinh thật có cờ này; cạnh gia đình tổng hợp khi khởi tạo không tự được xem là con hợp pháp.
- Truy vấn người hưởng chỉ dùng cạnh đã xác nhận để tìm con hoặc cha/mẹ. Anh chị em được suy ra từ cha/mẹ chung đã xác nhận thay vì tin cạnh `Sibling` tổng hợp.
- Save hiện tại dùng `POLYEC_REALM_25` và section sparse `PARENTAGE_1`. Khi đọc schema cũ, cạnh chỉ được xác nhận nếu có childbirth fact kind 105 khớp đúng `actor=child`, `target=parent`.
- Không đổi luồng trả nợ/di tặng; resolver vẫn là query thuần và chưa được nối vào chuyển tiền, hàng, quyền sử dụng đất, land title hay xưởng.

## Kiểm tra đã chạy

- Windows Release build cho `polyec_manager_tests` và `polyec_realm_tests`: pass.
- `polyec_manager_tests.exe`: pass; fixture xác nhận cạnh cha–con/anh chị em chưa xác minh không tạo người hưởng.
- `polyec_realm_tests.exe`: 20.634 checks pass. Có kiểm tra cạnh sinh thật tồn tại sau save/load, schema24 khôi phục cờ từ childbirth fact, và chỉ số provenance bằng 0 bị từ chối mà không thay Realm đích.
- Realm run kèm profile A: 10.000 người, 19.743 cạnh thưa, 90 ngày; invariant được kiểm trong từng ngày.

## Giới hạn và phần còn lại

- Chưa chạy full Release CTest, Debug, Python replay, GDExt build hay Godot trên schema25. Chưa có smoke Realm 64 ngày vì bundle `realm-v11` vẫn thiếu trong project, theo trạng thái trước đó ở CURRENT_TASK.
- Migration parentage được kiểm trực tiếp trên schema24; Realm suite còn có migration các schema cũ, nhưng chưa có fixture riêng chứng minh từng phiên bản 7–23 giữ đúng cờ cha–con.
- Codex tự review; chưa có reviewer độc lập.
- Đây là phần hoàn tất để bàn giao, không phải hoàn tất M5. Chia tiền/tài sản theo quan hệ này, claim còn thiếu tiền, chuyển đất/xưởng và giám hộ chưa làm.

## Lệnh kiểm tra

```powershell
cmake --build build/m5-core --config Release --target polyec_manager_tests polyec_realm_tests --parallel 4
build/m5-core/core/Release/polyec_manager_tests.exe
build/m5-core/core/Release/polyec_realm_tests.exe
```

Working tree có nhiều thay đổi M5 từ các phần việc khác; không tạo commit chứa các thay đổi trộn lẫn.

Manifest SHA-256 và kết quả: [m5-core-legal-parentage-provenance](../../artifacts/m5-core-legal-parentage-provenance-2026-10-21-manifest.json). Các hash là toàn file trong checkout dùng chung; chúng không khẳng định mọi thay đổi trong file đều thuộc riêng phần việc này.
