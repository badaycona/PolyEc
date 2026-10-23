# Bằng chứng M5.3 — Realm từ chối đề nghị giữa cha/mẹ và con

Ngày 2026-10-22. [IMPLEMENTATION_PLAN.md §2](../IMPLEMENTATION_PLAN.md) cấm hôn nhân giữa người có quan hệ cha-con trực hệ. Test Court đã bao phủ người chưa đủ tuổi và anh chị em trưởng thành; lát này bổ sung cặp cha/mẹ–con trưởng thành bằng chính danh sách quan hệ đang có trong Realm.

## Cách kiểm tra và kết quả

Fixture tìm một cạnh `ParentChild` mà cả hai người còn sống và đã đủ 18 tuổi. Cạnh này là quan hệ typed trong dữ liệu khởi tạo; test không coi nó là bằng chứng parentage hợp pháp. Với người ở đầu quan hệ cha-con, test tạo candidate kết hôn nhắm tới đầu còn lại và hỏi Realm action mask có cho thực hiện hay không. Kết quả là Realm từ chối. Đây là hành vi đúng với contract: luật hôn nhân cấm quan hệ cha-con trực hệ; việc thừa kế lại có tiêu chuẩn riêng, cần parentage hợp pháp.

Người chơi sẽ không thể gửi đề nghị kết hôn khi hai người có quan hệ cha-con trực hệ đã được ghi trong đồ thị quan hệ. Test này không thêm quy tắc kết hôn hoặc quy tắc parentage mới.

## Thay đổi và kiểm tra

- Chỉ thêm fixture assertion trong `core/tests/test_realm_court.cpp`; production source, API, schema, binding, action/observation/model và reward không đổi.
- Windows Visual Studio Release: build `polyec_court_tests` pass; chạy trực tiếp **539 checks passed**.
- Windows Visual Studio Debug: build cùng target pass; chạy trực tiếp **539 checks passed**.
- Full core Release CTest 19/19 có trước assertion này. Nó bao gồm remarriage test trước đó nhưng không chứa phép kiểm parent-child mới; assertion mới đã chạy qua Court target ở cả hai cấu hình. Không chạy Python/native/GDExtension/Godot vì không đổi mã production hoặc bề mặt gọi.
- Tự review relation endpoints, tuổi và tình trạng sống trong fixture, candidate chọn đúng người kia, kết quả action mask và test sibling/minor kế bên. Chưa có reviewer độc lập.

Đây là bổ sung một điều kiện eligibility trong lớp Realm, không phải nghiệm thu M5.3 hoặc toàn bộ M5. [Plan](../design/M5-core-marriage-parent-child-exclusion-plan.md) và [manifest](../../artifacts/m5-core-marriage-parent-child-exclusion-2026-10-22-manifest.json) lưu phạm vi và hash file. Không commit, push hoặc publish.
