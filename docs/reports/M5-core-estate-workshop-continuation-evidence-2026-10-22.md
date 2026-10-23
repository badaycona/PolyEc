# Bằng chứng M5 — tiếp tục sản xuất khi người thừa kế nhận xưởng

Ngày 2026-10-22. Kế hoạch: [nối ca làm với chuyển giao xưởng](../design/M5-core-estate-workshop-continuation-plan.md). Đây là kiểm thử tích hợp C++ cho luật thừa kế, lịch Work, lương xưởng và sản xuất đã có. Không sửa luật gameplay, source chạy, save schema, binding, action/observation, reward hoặc model.

## Chuỗi được kiểm tra

1. Fixture Realm có một chủ xưởng sắp chết, khoản vay 20, một người phối ngẫu trưởng thành là người hưởng duy nhất, và người này đã được phép làm tại xưởng. Chủ xưởng dùng `EstablishmentTransferCommand` thật để chuyển 2 Fiber từ kho cá nhân vào kho xưởng. Người hưởng dùng `ScheduleCommand` thật gửi ca Work 64 tick tại xưởng trước khi nhận thừa kế.
2. Test chạy pha cư dân Realm. Chủ xưởng chết; khoản vay 20 được trả từ hồ sơ di sản; người hưởng nhận xưởng. Bài kiểm xác nhận cùng ID xưởng, số dư 10, 2 Fiber và danh sách quyền làm việc được giữ nguyên, còn người hưởng trở thành chủ.
3. Test tiến Realm đúng 64 tick qua `advance_ticks`. Attendance của người hưởng tại xưởng là 64 tick, tương đương bốn đơn vị công theo snapshot 16 tick/đơn vị. Sổ lương ghi bốn đơn vị; tài khoản xưởng giảm từ 10 xuống 6; tiền mặt người làm tăng đúng 4. Hai Fiber được tiêu thụ và hai Cloth được tạo theo recipe/capacity hiện hành. Tổng tiền và `invariant()` vẫn đúng.
4. Save rồi load vào Realm mới. Fingerprint khớp; chủ xưởng, cash, inventory, lượng công đã xử lý, sổ lương và attendance vẫn khớp.

Bài kiểm xưởng payroll độc lập cũng được chạy: **138 checks pass**, gồm năm oracle migration save v16. Nó kiểm sâu hơn các nguồn trả lương, shortage/retry và không tính lại công đã trả. Hai phép kiểm bổ sung cho nhau: Realm test chứng minh kế vị thật không làm đứt ca; payroll test chứng minh riêng quy tắc hạch toán của xưởng.

## Lệnh và kết quả

- Build Windows Visual Studio Release: `cmake --build build/m5-core --config Release --target polyec_realm_tests polyec_worksite_payroll_tests` — exit 0.
- Chạy `build/m5-core/core/Release/polyec_realm_tests.exe` — **20.669 checks pass**; runner kèm profile A 10.000 người, 19.743 quan hệ thưa, 90 ngày. Đây không phải profile B.
- Chạy `build/m5-core/core/Release/polyec_worksite_payroll_tests.exe` — **138 checks pass**, năm oracle save v16.
- Full core Release CTest **19/19** đã pass ngay trước phần bổ sung test-only này. Sau khi thêm fixture, full core Debug CTest chạy **19/19 pass trong 429,58 giây**, nên Realm suite chứa chính test mới cũng đã chạy trong Debug. Không lặp full Release CTest chỉ vì thay fixture; hai executable Release liên quan đã build và chạy trực tiếp.
- Python/native/GDExt/Godot không chạy lại cho test-only change này: không có source, schema, API hay binding nào thay đổi kể từ mốc tích hợp R1–R4. Kết quả trước đó được ghi riêng ở [báo cáo core refactor](M5-core-refactor-R1-R4-integration-evidence-2026-10-22.md); chúng không chứng nhận bài fixture mới chạy trên các adapter.
- Review do Codex tự thực hiện, không có reviewer độc lập.

Trong lúc viết bài kiểm, lần assert ban đầu so tiền người hưởng trước cả khi nhận di sản với tiền sau payroll, nên trộn tiền thừa kế vào oracle lương. Oracle được sửa để chụp tiền sau pha tử vong và trước ngày làm; test cuối xác nhận chênh lệch do Work là đúng 4. Đây là lỗi trong phép so sánh của test, không phải lỗi gameplay.

## Giới hạn

Bài này nối tới ca sản xuất sau kế vị, chưa có contract đồng sở hữu xưởng khi nhiều người hưởng. Nó không chứng minh guardian được chi tiền hoặc quản lý xưởng của trẻ; các quyền đó vẫn chưa được quy định chi tiết. Không có learned model được huấn luyện hay đánh giá, không có dataset RL mới, Godot gameplay playtest, profile B, benchmark toàn chu trình hoặc package Windows. M5 vẫn mở.

[Manifest SHA-256](../../artifacts/m5-core-estate-workshop-continuation-2026-10-22-manifest.json) ghi hash của test, plan, report và trạng thái bàn giao trên working tree chung. Không commit, push hoặc publish.
