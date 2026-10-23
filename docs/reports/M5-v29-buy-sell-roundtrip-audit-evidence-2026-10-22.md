# M5 v29 — đối chiếu Buy–Sell bằng giao dịch Realm thật

Ngày 2026-10-22. Kế hoạch: [Buy–Sell round trip](../design/M5-v29-buy-sell-roundtrip-audit-plan.md). Câu hỏi là giá trị utility tăng ngay khi mua có đổi thành tiền lời thực tế hay không nếu cùng lượng hàng được bán lại ở cùng vùng. Đây là một phép đo chẩn đoán; không đổi C++ gameplay, công thức utility/reward, policy, model, save hoặc binding.

## Dataset và cách lấy mẫu

Đây **không phải dữ liệu train RL mới** và không phát sinh rollout mới. Bộ audit gồm đúng 15 dòng được chọn từ dữ liệu đã lưu:

1. Nguồn là ba checkpoint Economic v23, mỗi checkpoint ở update 25, tương ứng training seed 1, 2 và 3. Mỗi checkpoint chứa 16 Realm world cuối run, nhãn scenario, actor, trạng thái C++ đã serialize, và trạng thái đối tác/RNG.
2. Script khôi phục các world bằng `RealmCoPlayBatch`, tức cùng binding C++ Realm dùng bởi trainer. Nó duyệt candidate mask đã lưu và chọn candidate Buy hợp lệ đầu tiên cho từng cặp `(scenario, GoodId)` theo thứ tự world/action hiện có. Checkpoint seed 1 cho 10 cặp, seed 2 không có Buy hợp lệ, seed 3 cho 5 cặp. Kết quả khớp 15 mẫu của audit v27.
3. Với từng dòng, script khôi phục lại chính world ban đầu, ghi fingerprint, cash, quỹ hộ, lượng bảy nhóm hàng legacy quan sát được, utility, Province firm và tổng tiền. Sau đó nó gửi hai Realm command thật: Buy 4 món, rồi Sell 4 món cùng GoodId và vùng, không tiến game time hoặc cho đối tác hành động xen giữa.
4. Script ghi checkpoint/policy SHA-256, world index/scenario/fingerprint, GoodId, giá mua/bán, tiền trước và sau mỗi lệnh, utility trước/mua/sau bán, hàng, Province firm, quỹ hộ, tổng tiền và invariant. Nó dừng lỗi nếu số mẫu khác 15, lệnh bị từ chối, tiền/hàng không khớp công thức hoặc invariant hỏng.

Do đó mỗi dòng là một cặp giao dịch đối chứng trên một trạng thái checkpoint thật, không phải một observation tổng hợp, hàng replay giả lập trong Python, hay mẫu được dùng để cập nhật trọng số policy. Checkpoint gốc không bị sửa.

## Kết quả

| Hàng | Mẫu | Utility tăng ngay sau Buy | Tiền cá nhân sau Buy rồi Sell | Utility sau khi bán so với đầu |
|---|---:|---:|---:|---:|
| DryFood | 2 | +0,68 | −8 | −0,08 |
| Fiber | 3 | +0,44 | −8 | −0,08 |
| Fuel | 3 | +0,60 | −8 | −0,08 |
| Iron | 3 | +1,68 | −8 | −0,08 |
| Tools | 1 | +2,48 | −8 | −0,08 |
| Horse | 3 | +3,48 | −8 | −0,08 |
| **Tổng** | **15** | — | — | — |

Trong các snapshot này, giá Buy là giá vùng cộng 1 tiền phí, còn giá Sell là `max(1, giá vùng − 1)`. Bốn món mua tốn tổng cộng 12 tiền và bán lại thu 4, nên số dư cash cá nhân giảm 8. Khi còn giữ hàng, utility vẫn cộng giá tham chiếu của bốn món và ghi nhận mức tăng tạm thời; sau khi thực hiện lệnh Sell thật, lượng hàng về đúng số ban đầu và phần utility còn lại chính là khoản cash đã mất 8 chia cho hệ số 100, tức −0,08.

Tất cả 15 mẫu giữ nguyên tồn kho hàng sau vòng giao dịch; quỹ hộ không đổi; tổng tiền toàn Realm không đổi; mọi invariant đều đúng trước Buy, sau Buy và sau Sell. Kết quả xác nhận chênh lệch trong v27 là giá trị tồn kho chưa hiện thực thành tiền. Nó không chứng minh model luôn mua để bán lại, vì audit không cho policy chọn hành động và không chạy horizon nhiều ngày.

## Lệnh và giới hạn

- `py -3.12 -m py_compile ai/evaluation/realm_buy_sell_roundtrip_audit_v11.py`: pass.
- Chạy script với ba thư mục `build/m5-v23-economic-curriculum-r1-runset/economic/seed-{1,2,3}` và Python binding Release tại `build/m5-python-ai/Release`: **15/15** vòng Buy–Sell được chấp nhận; tồn kho trở lại đúng, tiền được đối chiếu đúng, invariant và bảo toàn tổng tiền pass.
- `audit.json` có toàn bộ 15 dòng và hash đầu vào; [manifest](../../artifacts/m5-v29-buy-sell-roundtrip-audit-2026-10-22-manifest.json) ghi hash source/checkpoint/policy/output. Đây là lượt tự review của Codex; không có reviewer độc lập.
- Không chạy CTest, native, GDExtension, Godot hoặc huấn luyện: chỉ thêm một script audit Python offline, không sửa source production hay adapter. Audit diễn ra trong cùng vùng và ngày, không đo biến động giá theo thời gian/vùng, phí vận tải, tác động của đối tác, giá vốn hàng được sản xuất/nhận chuyển hoặc cách ghi nhận hàng còn tồn. Không chọn một công thức inventory cost basis và không tuyên bố learned policy hoàn tất.
