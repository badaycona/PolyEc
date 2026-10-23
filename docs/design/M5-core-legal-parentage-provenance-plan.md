# Core — provenance cha–con hợp pháp cho người hưởng di sản

Ngày 2026-10-21. `IMPLEMENTATION_PLAN.md` §2 chỉ trao thừa kế theo con hợp pháp. Hiện relation graph dùng `ParentChild` cho cả quan hệ khởi tạo hộ không đủ căn cứ pháp lý và con sinh trong Realm; resolver chưa phân biệt hai nguồn.

## Contract

- Thêm cờ provenance riêng trên `Edge`; chỉ cạnh ParentChild được xác nhận mới được dùng làm cha/mẹ/con khi phân giải người hưởng.
- Các cạnh ParentChild/Sibling khởi tạo trong population chỉ giữ vai trò quan hệ hạn chế kết hôn hiện hành; chúng không chứng minh quyền thừa kế.
- Cạnh được tạo cùng sự kiện sinh con của Realm được đánh dấu hợp pháp. Khi migrate save schema7–24, chỉ nâng cạnh ParentChild nếu có birth fact cùng hướng `child -> parent`, kind 105, actor=child, target=parent. Quan hệ không chứng minh được vẫn tồn tại cho luật cũ nhưng không được dùng để chuyển tài sản.
- Anh chị em để thừa kế được suy ra từ ít nhất một cha/mẹ hợp pháp chung; không dùng các cạnh Sibling khởi tạo tổng hợp.
- Save schema25 ghi danh sách chỉ số cạnh cha–con hợp pháp trong section mới. Schema cũ vẫn đọc nguyên Edge record; migration/re-serialize giữ provenance nào có chứng cứ.
- Chưa chia tiền/tài sản; chưa thêm nhận con nuôi, con ngoài hôn nhân hoặc registry hộ tịch tùy ý.

## File và invariants

- `core/include/polyec/realm.hpp`: cờ provenance ở Edge.
- `core/include/polyec/parentage_section_codec.hpp`: section sparse, chỉ chấp nhận index tăng dần, edge ParentChild không tự thân, giới hạn trong danh sách.
- `core/src/realm.cpp`: schema25, migration dựa birth fact, invariant section và roundtrip.
- `core/include/polyec/managers/estate_manager.hpp`: beneficiary query chỉ đọc ParentChild có provenance; sibling suy ra từ shared legal parent.
- `core/tests/test_managers.cpp`, `core/tests/test_realm.cpp`: nhóm quan hệ tổng hợp bị loại; child sinh thật được giữ sau save/load; schema24 birth-fact migration; corrupt/missing provenance reject/legacy fallback.
- Luật hôn nhân tiếp tục dùng cả relation cũ như trước; không đổi action, observation, reward hoặc model.

## Kết quả và mốc tích hợp

Hoàn tất 2026-10-21: Release manager + Realm tests pass, gồm beneficiary query loại cạnh tổng hợp, save/load, migration schema24 theo childbirth fact và từ chối parentage index sai mà không thay Realm đích. Chi tiết ở [report](../reports/M5-core-legal-parentage-provenance-evidence-2026-10-21.md). Khi nhóm ownership/estate cash distribution ổn định: full Release CTest, Python save migration/replay, GDExt build và Godot smoke. Không cần đổi bindings vì public Realm API không đổi.

Review Codex tự thực hiện; chưa có reviewer độc lập.
