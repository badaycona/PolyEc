# GitHub replacement — 2026-10-25

Đã publish commit17c052912208627bf481de27a2c6f64f349b3ff2 lên origin/main và kiểm bằng ls-remote. Commit nối tiếp HEAD cũ4abc6ed (main cũf282f05 là ancestor), không force-push hoặc xóa lịch sử. Tree hiện hành chỉ gồm rewrite trong rebuild/ và entrypoints/CI mới; không source/build/model legacy. Baseline root CMake/Ninja và2 CTest pass trước publish.

Đã đồng bộ HEAD/index local sang main bằng ref/index, không checkout/xóa file legacy trên đĩa. Legacy local được giữ và ghi rõ trong .gitignore, không thuộc tree publish. Không thay các worktree bên ngoài. Chỉ có snapshot Documents bất hoạt và deliverables, code được ghi tại C:\prc\PolyEc\rebuild.
