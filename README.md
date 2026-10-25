# PolyEc — native C++20 / Godot rewrite

Source chính: [rebuild/](rebuild/). [Việc hiện hành](rebuild/docs/CURRENT_TASK.md), [roadmap](rebuild/docs/rebuild/IMPLEMENTATION_PLAN.md), [class/action](rebuild/docs/CLASS_ACTION_MAP.md).

Build portable core: `cmake -S . -B build`, `cmake --build build --config Release --target core_tests action_tests polyec_core`, `ctest --test-dir build -C Release --output-on-failure`. Godot4.5 project ở rebuild/game, cần binary polyec_core.exe cạnh main.gd. Binaries/cache/models không nằm trong repo; game native, không web. Script Windows và packaging ở rebuild/scripts.

Repo giữ lịch sử nhưng nội dung hiện hành thay bằng rewrite. Các file legacy còn trên máy không thuộc tree source này và không được build. Chưa có learned policy; script demo chỉ kiểm luật. Quy mô kế tiếp: hai lãnh thổ theo workflow, không tự mở training/chiến tranh.
