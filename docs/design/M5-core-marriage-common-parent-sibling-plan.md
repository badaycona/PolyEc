# M5.3 — nhận diện mọi anh chị em cùng cha/mẹ khi kiểm tra hôn nhân

## Vấn đề và căn cứ

`docs/IMPLEMENTATION_PLAN.md` §2 cấm hôn nhân giữa anh chị em. Realm lưu cả cạnh `Sibling` tường minh và cạnh `ParentChild`. Dữ liệu khởi tạo có thể nối cạnh `Sibling` theo chuỗi, nên hai người cùng cha/mẹ ở hai đầu chuỗi không có cạnh `Sibling` trực tiếp. `Realm::marriage_related` hiện tra cạnh tường minh trực tiếp và có thể để cặp đó lọt qua.

## Thay đổi dự kiến

- Giữ cạnh `Sibling` hiện có theo cả hai chiều.
- Khi dựng chỉ mục `ParentChild`, gom con theo từng cha/mẹ và đánh dấu mọi cặp con khác nhau cùng cha/mẹ là anh chị em. Không suy luận anh chị em từ cạnh tự thân; bỏ quan hệ con==cha/mẹ khi tạo cặp.
- Duy trì chỉ mục con theo cha/mẹ cùng lúc Realm thêm cạnh cha/mẹ–con; save không đổi vì chỉ mục được dựng lại từ các quan hệ đã lưu.
- `core/tests/test_realm_court.cpp`: kiểm action mask và Realm command từ chối nguyên tử cho hai anh chị em có chung cha/mẹ nhưng không có cạnh `Sibling` trực tiếp; thêm một người con bằng phương thức cập nhật chỉ mục tăng dần; round-trip xác nhận load dựng lại chỉ mục; giữ ca cạnh `Sibling` tường minh.
- `core/tests/test_realm.cpp`: giữ bài kiểm hiện có rằng sinh người thêm cạnh cha/mẹ–con vào chỉ mục trực hệ và save/load dựng lại chỉ mục đó.

## Phạm vi và lệnh kiểm

- Source: `core/include/polyec/realm.hpp`, `core/src/realm.cpp`.
- Tests: `core/tests/test_realm_court.cpp` cho luật/action/command, cập nhật anh chị em tăng dần và save/load; `core/tests/test_realm.cpp` cho chỉ mục trực hệ khi sinh con.
- Build/chạy Windows Release `polyec_court_tests` và `polyec_realm_tests`; do thay đổi private class layout, build lại `polyec_manager_tests` và `polyec_ai` cùng native `polyec_realm_contracts` ở ranh giới tích hợp C++ phù hợp.
- Không đổi save schema hoặc action/observation dimensions. `build/windows` tắt Python, nhưng cấu hình riêng `build/m5-python-ai` bật binding; rebuild và chạy replay binding khi nhóm quan hệ này ổn định. Build target GDExtension `polyec` sau thay đổi class layout; Godot scene smoke vẫn để mốc gameplay tích hợp vì UI/API không đổi.

## Hoàn thành khi

Cặp có cùng cha hoặc mẹ bị từ chối dù thiếu cạnh `Sibling` trực tiếp; cạnh `Sibling` rời vẫn tiếp tục bị từ chối; action mask và command dùng cùng kết quả; chỉ mục đúng sau khởi tạo, sinh con và tải lại; command bị từ chối không đổi fingerprint, escrow hay quan hệ. Tự review, không có reviewer độc lập.
