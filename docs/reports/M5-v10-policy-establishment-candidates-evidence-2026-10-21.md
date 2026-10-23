# M5 — policy tạo và cấp vốn cơ sở sản xuất

Ngày 2026-10-21. Bước này thêm action tạo xưởng vào policy contract; chưa triển khai toàn chu trình xưởng.

## Người chơi/agent nhận được gì

Action mới cho phép chọn một trong ba ngành (dệt, công cụ, quân nhu) và vốn 25, 50 hoặc 100 trong vùng actor đang đứng. Actor kinh tế/dân sự sở hữu xưởng và trả vốn từ tiền cá nhân. Quan chức có taxing authority tạo xưởng công và trả từ treasury vùng. Core kiểm tra quyền, vốn, charter và vùng; action policy gọi cùng `EstablishmentCommand` với các đường lệnh khác. Action có tên UI tiếng Việt “Tạo cơ sở sản xuất”.

## Thay đổi contract

- Chuyển từ `realm-v9` 80 action/1.088 observation sang `realm-v10` 89 action/1.196 observation; slot 80–88 là ba ngành × ba mức vốn.
- Tăng model-contract lên v4 và hệ số chuẩn hóa feature loại action; manifest v8/v9 bị từ chối kể cả dev override. Có architecture scorer v10 để khóa shape, nhưng chưa có trainer/export v10 hoặc checkpoint được huấn luyện.
- Save vẫn schema23; action mới không sửa luật save hay tự thêm site ngoài khi chọn action.
- Thêm stack 4 MiB riêng cho executable `polyec_realm_tests` trên MSVC. Debug Realm suite vượt stack test mặc định; production `deserialize` cũng dựng staging Realm trên heap.

## Đã kiểm tra

- Full core CTest: Debug **18/18**, Release **18/18**. Sau lần chỉnh nhãn action, `realm_tests` chạy lại và pass ở Debug (150,82 giây) và Release (17,26 giây).
- Python 3.12 Debug/Release: model contract 14 checks; preflight **478**; private/public workshop binding **14**; clock binding **173** và 7/7 checkpoint; config parity 49/7; household fund 11 + study 7; save migration 40; establishment transfer/production 32/7; work authorization 7 checkpoint.
- Native `runtime_contracts` Release: **1/1**. GDExtension Windows Release build thành công; Godot 4.5.1 headless xác nhận schema v10 và từ chối bundle cũ: **5/5**.
- Scoped `git diff --check` pass; log chẩn đoán tạm đã gỡ.

## Còn thiếu

Self-review; không có reviewer độc lập. Policy chưa thể chuyển nguyên liệu, nạp/rút kho xưởng, cấp Work authorization hoặc gửi ca Work có worksite. Khi timeline bắt đầu, policy candidate access hiện tại chưa chạy chu trình tiếp diễn. Chưa có learned partner/cross-play, chưa huấn luyện v10 và không đạt R1. Bước tiếp là đưa chuyển hàng, ủy quyền và Worksite Schedule vào cùng candidate/command pipeline trước khi chạy training.

Không commit/push vì source checkout còn nhiều lát M5/core khác không thể tách sạch.

SHA-256 manifest: [m5-v10-policy-establishment-candidates-2026-10-21-manifest.json](../../artifacts/m5-v10-policy-establishment-candidates-2026-10-21-manifest.json).
