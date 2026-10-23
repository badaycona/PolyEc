# M5 T1 — tài khoản quỹ hộ

## Baseline và mục tiêu

Baseline là working tree hiện hành với Realm schema 19, HouseholdIndex dẫn xuất từ membership và các account tiền hiện có. Tạo tài khoản quỹ hộ riêng, ban đầu bằng 0; nộp/rút là Realm transaction có sequence, authority, event và conservation. Tiền cá nhân không tự chuyển khi membership thay đổi. Lát này không chọn khoản chi, không mở rộng luật guardian/estate/land, không đổi action/observation/reward/model.

## Working rules

- Sổ quỹ là map thưa `household_id -> cash`; chỉ lưu bản ghi dương. `household_id` tiếp tục là label của hộ hiện hành, không cấp ID mới.
- Nộp: actor còn sống, đủ 18 tuổi, thuộc hộ đích; chuyển đúng số tiền từ cash cá nhân vào quỹ.
- Rút: actor còn sống và bằng `household_authority(household_id)` tại thời điểm lệnh; chuyển từ quỹ về cash cá nhân. Guardian không được rút tiền riêng của người được giám hộ qua API này.
- Số tiền phải dương. Sai sequence/ID/quyền/nguồn/overflow reject nguyên tử, không tăng counter hoặc phát event.
- Quỹ không tự chi Study/Buy/Aid trong lát này; các rule sử dụng và quyền lợi của dependent cần lát riêng có contract.

## Files và ownership

- `core/include/polyec/realm.hpp`, `core/include/polyec/household_fund.hpp`: command API, query và account map; Realm tiếp tục sở hữu state.
- `core/include/polyec/household_fund_section_codec.hpp`: codec tail section có thứ tự ổn định.
- `core/src/realm_household.cpp`: validation, staged transfer/commit; chỉ Realm đổi account/cash/counter/fact.
- `core/src/realm.cpp`: money ledger/invariant, `balances()` slot mới, schema 20 tail section và migration 7–19 về account rỗng.
- `bindings/python/realm_binding.cpp`: command và read-only account query cùng C++ core.
- `core/tests/test_household_fund.cpp`, `core/CMakeLists.txt`: oracle nộp/rút, authority, reject atomic, save/load/migration/conservation.
- `docs/IMPLEMENTATION_PLAN.md`, `docs/CURRENT_TASK.md`, `docs/DEVELOPMENT_HISTORY.md`, evidence và manifest: ghi rõ working rule, schema/API và giới hạn.

## Save contract

Schema 20 giữ nguyên mọi token trước đó, thêm `HOUSEHOLD_1` sau `BODY_2`: count rồi các cặp `household_id cash` tăng dần. Parser staging, chỉ nhận household đang tồn tại, số dư dương, ID tăng nghiêm ngặt, không duplicate; mọi lỗi giữ nguyên Realm đích. Schema 7–19 khởi tạo quỹ bằng 0. Fingerprint bao gồm section này.

## Gates

1. Core Release/Debug build; direct household tests và full CTest theo cache hiện có.
2. Oracle tính tay: 100 nộp → fund 100/person giảm 100; 35 rút → fund 65/person tăng 35; total money và balances giữ khớp.
3. Reject wrong member/age/authority/sequence/zero/negative/shortage/overflow giữ fingerprint, cash, fund, counters và facts nguyên vẹn.
4. Round-trip giữ đúng fund; schema 19 fixture migration ra schema 20 với fund 0; malformed section không thay state đích.
5. Python Release/Debug command và query dùng cùng Realm; transition/clock replay theo gate bị ảnh hưởng.
6. `git diff --check`, repository link checker; final self-review theo contract. Review độc lập chưa có.

## Handoff

Không commit/push. Đây là nền tài khoản M5.0/1; chưa hoàn tất household lifecycle, land/rights, guardian administration, household spending, action/model agency, Godot UX hoặc các gate M5 còn lại.
