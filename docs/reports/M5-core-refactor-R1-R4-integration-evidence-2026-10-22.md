# Tích hợp core refactor R1–R4 — Windows Release

Ngày 2026-10-22. Báo cáo này gom kiểm tra sau khi các phần R1–R4 đã được triển khai riêng: trả lương theo prepare/apply, chuyển trạng thái việc làm, điều phối ngày, cùng tách save/load và điểm vào RealmView. Đây là hoàn tất phạm vi **core refactor R1–R4** theo bảng bước trong [core architecture refactor](../design/core-architecture-refactor.md). Nó không nghiệm thu toàn bộ M5 hoặc chính sách RL.

## Điều gì đã được kiểm tra

- Core C++ Release: CTest chạy **19/19** nhóm thành công trên working tree hiện tại, tổng **37,74 giây**. Sau khi thêm Realm fixture-only cho chuỗi thừa kế/xưởng, core Debug CTest cũng đạt **19/19** trong **429,58 giây** (Realm 245,67 giây; Campaign 94,58 giây). Nhóm này có Realm, manager, lịch, đồng hồ, production, gameplay config, work credit, payroll xưởng, chuyển tài sản xưởng, quỹ hộ và clock replay. Realm tests chứa các tích hợp di sản/xưởng mới nhất và ghi **20.669 checks** trong lượt Release độc lập, với workload profile A 10.000 người, 19.743 quan hệ thưa, 90 ngày. Profile A không phải profile B.
- Python binding/replay Release được build lại cùng C++ replay driver. Đối chiếu Realm đạt **273 checks qua 64 ngày**; kiểm clock đạt **173 checks qua 7 checkpoint**. Các giá trị chuyển trạng thái và checkpoint trùng giữa Python với driver C++ trong phạm vi các phép kiểm này. Phần này không chạy neural inference.
- Native Release: `polyec_realm_contracts` và `polyec_realm_replay` được build; CTest `runtime_contracts` đạt **1/1**. Đây là kiểm hợp đồng và model guard, không phải chứng nhận chất lượng model.
- Godot Extension Windows Release được build thành công. Godot 4.5.1 headless chạy `realm_worksite_smoke.gd`, đạt **5/5** kiểm tra model guard. Smoke này xác nhận extension nhận đúng cấu trúc save hiện tại và từ chối model pilot cũ; nó không chạy chu trình sản xuất trong giao diện và không gọi learned policy.

## Kết luận theo phạm vi

R1, R2, R3 và R4 đều đã có báo cáo triển khai từng phần; lượt này bổ sung full core regression và kiểm adapter/native/Godot trên source hiện tại. Vì vậy core refactor R1–R4 được ghi là hoàn tất ở phạm vi Windows đã chọn. Không có thay đổi source gameplay trong lượt xác nhận tích hợp này.

## Việc còn mở

- Core Debug CTest đã pass 19/19 sau fixture-only; Python/native/GDExt/Godot Debug chưa chạy.
- Không có model tương thích với schema/contract mới để chạy trọn 64 ngày trong Godot, suy luận mạng hoặc đánh giá chính sách đã học. Kiểm tra 273 ngày Python là replay binding, không phải đánh giá chất lượng RL.
- Chưa chạy profile B, campaign dài, hiệu năng toàn chu trình, stress test hay package Windows offline.
- M5 vẫn mở: các phần còn thiếu trong hộ/giám hộ/di sản, đủ nền kinh tế và logistics, tổ chức/giáo hội, quốc gia 24+3 và giao diện, chuỗi chơi thực tế, cùng learned-policy quality. Các luật quản lý tiền/xưởng của trẻ và cách đồng sở hữu tài sản nhiều người hưởng chưa được tự đặt trong báo cáo này.
- Review là Codex tự đối chiếu; không có reviewer độc lập.

Các kết quả core C++ áp dụng cho Windows Release và Debug; các adapter Python/native/GDExt/Godot được kiểm ở Windows Release như ghi riêng phía trên. Các kết luận chỉ áp dụng cho những đường được nêu cụ thể. [Manifest SHA-256](../../artifacts/m5-core-refactor-R1-R4-integration-2026-10-22-manifest.json) ghi HEAD nền cùng các file nguồn, test và tài liệu đã đối chiếu; một số file nằm trong working tree chung và có thay đổi cũ không thuộc riêng mốc này. Không commit, push hoặc publish.

