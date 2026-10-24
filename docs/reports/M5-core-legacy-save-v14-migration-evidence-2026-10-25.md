# Kết quả: khôi phục di trú save lịch sử v14

Ngày hoàn tất: 2026-10-25
Nhóm: `legacy-save-v14-migration-v1`
Phạm vi: đọc save v14 cũ trong C++ Realm, dựng lại dữ liệu mặc định của các hệ thống được thêm ở save mới hơn, và xác nhận load/reject nguyên tử. Không đổi định dạng save đang ghi, không đổi hash chuẩn lịch sử, không train RL.

## Vì sao lỗi ảnh hưởng đến game

Save v14 được tạo trước khi Realm lưu cấu hình gameplay, sức chứa nhà công và thành viên hội đồng đô thị. Khi mở save cũ, người chơi phải thấy cùng thế giới đã lưu với các luật mặc định của phiên bản hiện tại; những dữ liệu mà phiên bản cũ chưa từng lưu cần được khởi tạo nhất quán.

Đường đọc v14 bỏ qua toàn bộ đoạn xử lý phần đuôi cấu hình, vì phần đuôi đó chỉ tồn tại từ save v15. Việc dựng mặc định cho sức chứa nhà công và hội đồng lại đang nằm bên trong đoạn xử lý này. Kết quả là bản Realm tạm dùng để đọc save vẫn giữ mảng nhà công có kích thước của thế giới tạm một vùng. Với save Atlas, danh sách hội đồng của 27 settlement cũng chưa được tạo. Kiểm tra invariant cuối cùng từ chối cả hai trường hợp bằng `Corrupt save/invariant`.

Đây là thiếu bước di trú ở code đọc save, không phải fixture có grammar sai. Bộ test đã dựng save v14 bằng cách chuyển save mới về grammar cũ, bỏ `CONFIG_1` mà v14 không có và trả bộ cấp ID về mức trước khi save có land title. Hash lịch sử chuẩn không bị sửa.

## Thay đổi đã thực hiện

Sau khi đọc các trường save, mọi save trước v15 nay được đưa qua khởi tạo sức chứa nhà công hiện hành. Với save Atlas cũ trước v65, Realm cũng tạo 27 hồ sơ hội đồng trống, mỗi hồ sơ mang đúng ID settlement. Những bước này chạy cả khi save không có phần cấu hình. Các save v15 trở lên tiếp tục dùng đúng nhánh đọc section hiện hành; cách ghi save mới không đổi.

Kiểm thử `gameplay_config_tests` được mở rộng để nạp hai fixture v14: một Realm hai vùng và một Atlas 27 vùng. Ca Atlas xác nhận load thành công, invariant đúng và có đủ 27 hồ sơ hội đồng trống. Ca hiện có tiếp tục xác nhận luật cấu hình lịch sử được khôi phục, không tự tạo land title và Realm hợp lệ sau load.

## Bằng chứng kiểm tra

- Các lệnh chạy trên Windows Release (CMake/CTest dùng bản đi cùng Visual Studio, vì chúng không nằm trong `PATH`):

  ```powershell
  & 'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe' --build build/m5-core --target polyec_gameplay_config_tests --config Release
  & .\build\m5-core\core\Release\polyec_gameplay_config_tests.exe
  & 'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/ctest.exe' --test-dir build/m5-core -C Release -R 'gameplay_config_tests|work_credit_tests' --output-on-failure
  & 'C:/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe' --build build/m5-core --target polyec_realm_tests --config Release
  & .\build\m5-core\core\Release\polyec_realm_tests.exe
  git diff --check
  ```

- Release build target C++ `polyec_gameplay_config_tests`: thành công; executable báo **51/51 checks pass**. Những check có liên quan đến nhóm gồm: parse/chuẩn hóa cấu hình và mặc định lịch sử; cấu hình mới được giữ khi lưu rồi nạp; tính input/output và lương theo cấu hình; overflow không tiêu input; thiếu ngân quỹ giữ lại phần lương chưa đủ một đơn vị; save hỏng/remainder không hợp lệ bị từ chối mà không thay thế thế giới; chuyển v14 khôi phục mặc định, không tạo land title và giữ invariant cho Realm hai vùng lẫn Atlas27; file cấu hình đổi sau khi save không thay cấu hình đã được ghi trong save. Tổng51 là toàn bộ test executable, không phải số lượng chỉ riêng migration.
- CTest Release, hai mục `gameplay_config_tests` và `work_credit_tests`: **2/2 pass**. `work_credit_tests` chạy các quy tắc phân bổ/kiểm tra tín dụng công việc cũ; nó không thay bằng chứng load fixture v14, vốn nằm trong target 51 checks trên.
- `polyec_realm_tests` được build và chạy để kiểm các migration/roundtrip Realm rộng hơn. Output kết thúc `FAIL after 434: line 2231: default configuration preserves complete historical v14 save bytes`; process trả exit code1. Đây là phép so byte/hash độc lập, không phải lỗi invariant load v14 trong `gameplay_config_tests`. Không sửa hash, dữ liệu fixture hoặc bỏ assertion. Runner vì dừng ở đây nên các kiểm tra nằm sau assertion đó chưa được chạy trong lượt này. Kiểm nạp v14 hiện hành được chứng minh riêng bằng target 51 checks trên.
- Không chạy lại Python/GDExtension/Godot vì sửa này chỉ nằm trong đường đọc save C++ và không đổi API, binding hoặc dữ liệu mà adapter tiêu thụ. Không chạy full CTest, benchmark, package hay RL.
- Các file chẩn đoán tạm chỉ dùng để tìm nhánh invariant đầu tiên đã được khôi phục/xóa; không còn log fixture mới trong source tree. `git diff --check` chạy trước commit và không báo whitespace lỗi.

## Giới hạn và bước kế tiếp

Kết quả này xác nhận cách khởi tạo mặc định cho save v14 ở Realm hai vùng và Atlas 27 vùng, đồng thời giữ nguyên đường từ chối dữ liệu sai. Nó không xác nhận mọi phiên bản save cũ hoặc mọi kết quả của Realm runner rộng; assertion so byte lịch sử vẫn là điểm dừng riêng cần giữ nguyên để truy nguyên.

M5 còn thiếu nguồn hoặc nơi tiêu thụ rõ ràng cho Gold, Beeswax, Silk, Livestock, DecorativeGlass, FinePorcelain và GeneralLuxury. Sau nhóm migration, công việc quay lại chuỗi hàng hóa còn thiếu theo roadmap; RL tiếp tục bị hoãn cho tới khi gameplay M5 và core refactor R1–R4 hoàn tất.
