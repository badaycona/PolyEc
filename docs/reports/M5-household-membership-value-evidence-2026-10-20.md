# M5 HouseholdMembership value composition — evidence (2026-10-20)

Plan đã review: [M5 HouseholdMembership value plan](../design/M5-household-membership-value-plan.md). Đây là thay đổi cấu trúc source-level cho quan hệ hộ hiện có; không tạo entity/quỹ/quyền/lifecycle hoặc gameplay rule mới.

## Kết quả

`Person::household` được thay bằng `Person::household_membership.household_id`, một value record chỉ chứa nhãn hộ hiện hành. Realm vẫn sở hữu Person và HouseholdIndex; public Realm queries, validation và index semantics giữ nguyên. Serializer/deserializer dùng cùng scalar ở đúng token vị trí cũ, không đổi header/schema/token order. GDExtension tiếp tục xuất key `household`, nên Godot view contract giữ nguyên. Python không expose field này.

## Kiểm chứng Windows

- Release `build/m5-core`: build exit 0; full CTest 10/10 pass (16.50 s).
- Debug `build/m5-core`: build exit 0; targeted `realm_tests|schedule_tests|manager_tests|realm_clock_replay` 4/4 pass (54.04 s).
- Native Release `build/m5-runtime`: `polyec_realm_benchmark` và `polyec_realm_contracts` build exit 0; CTest `runtime_contracts` 1/1 pass (1.00 s).
- Python Release và Debug `build/m5-python-ai`: `polyec_env` và `polyec_realm_transition_replay` builds exit 0; clock binding replay mỗi cấu hình 168 checks pass với 7 C++ checkpoints khớp; transition replay mỗi cấu hình 64 checkpoints.
- `git diff --check` exit 0. Git phát CRLF normalization warning cho file test Realm vốn đã dirty; không có whitespace error.
- Final source scan không còn truy cập trực tiếp `.household` field trong source hiện hành; GDScript vẫn đọc projected `household` key như trước. Lịch sử trong `artifacts/` loại trừ.

## Quy trình và giới hạn

OpenCode đã được giao lát plan-only trước implementation. Draft plan của nó sai phạm vi/đường dẫn và implementation bị dừng khi chưa hoàn tất; Codex thực hiện field-path migration cơ học theo approved plan, rồi chạy các gate trên. Không thay đổi luật Support/guardian/authority hoặc schema. Không kiểm app/Godot/Linux, không train, không commit/push.
