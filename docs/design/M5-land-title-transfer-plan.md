# M5.0/1 — lệnh chuyển title đất không định giá

Ngày 2026-10-21. Next slice sau registry `LAND_1` schema21. Đây là working assumption được ủy quyền để tiếp tục M5, không phải luật bán đất hoặc cân bằng thị trường.

## Mục tiêu và working rule

Cho phép title-holder trưởng thành, còn sống, tự chuyển toàn bộ một `LandHolding` của mình cho một Person còn sống. Đây là quà/chuyển quyền không đối giá: không đổi tiền, diện tích, region, ID holding hoặc quyền sử dụng đất. Chỉ cập nhật title-holder trong giao dịch Realm nguyên tử.

Person dưới 18 tuổi vẫn có thể nhận title. Guardian không thể ký deed/chuyển title thay dependent; không có guardian consent/override trong lát này. Người giữ title vị thành niên không thể tự chuyển cho tới khi trưởng thành. Nếu holder chết, không tự chuyển title trong lát này; inheritance là gate riêng. Cùng holder/recipient, title ID không tồn tại, actor không phải holder, actor/recipient chết hoặc chưa thành niên, stale sequence, overflow và fact capacity đều reject không mutation.

Lệnh dùng riêng `LandTitleTransferCommand` cùng Realm ledger/fact/counter transaction; không thêm Intent/action slot, observation, reward, learned model hoặc giá đất. Không dùng UI ghi state trực tiếp. Python/GDExt adapter phải chỉ gọi command C++.

## File boundary dự kiến

- `core/include/polyec/land_holding.hpp`: command value type.
- `core/include/polyec/realm.hpp`, `core/src/realm_establishment.cpp` hoặc file domain Realm phù hợp: public execute overload và staged validation/commit.
- `core/tests/test_land_title.cpp`: success/reject, guardian denial, dead recipient, counter/fact/ID atomicity, fingerprint, save/load/continuation. Run as a separate CTest target because the existing Realm harness has a large stack frame.
- `bindings/python/realm_binding.cpp`, binding checks: test-only C++ command adapter.
- `gdext/polyec_session.cpp`, Godot smoke nếu extension hiện expose Realm session: chỉ command method, giữ PlayerView riêng tư; nếu adapter không có Realm owner phải xác định đúng boundary trước sửa.
- `docs/IMPLEMENTATION_PLAN.md`, CURRENT_TASK, HISTORY, report, manifest.

## Gate

1. Chuyển hợp lệ giữ nguyên title ID/area/region, đổi holder đúng một lần, không đổi total money, invariant/fingerprint/save-load đều khớp.
2. Holder adult alive có thể chuyển cho adult hoặc dependent sống; người thứ ba, guardian của holder, deceased/underage holder không thể chuyển.
3. Wrong holding, invalid recipient, stale sequence, dead actor, counter/fact overflow reject nguyên tử.
4. Release/Debug Realm tests và full CTest; Python command path và GDExt build. Godot thao tác thật chỉ nghiệm thu nếu có UI test access phù hợp; không lộ registry toàn cục trong PlayerView.
5. Bảo toàn schema21 bytes ngoại trừ holder field của holding; không đổi action/observation/reward/model. Scoped diff/link/hash.

## Self-review và giới hạn

Working assumption trực tiếp cho phép chuyển quyền tự nguyện giữa owner trưởng thành; cấm guardian định đoạt title của dependent. Không có giá/thuế, lien, consent recipient, đất đồng sở hữu, chia diện tích, lease, use-right, inheritance, mất năng lực, tổ chức là owner hoặc provincial jurisdiction. Các luật đó cần gate riêng. Plan này được Codex tự review; chưa có reviewer độc lập.
