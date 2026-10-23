# M5 v24 — bộ đếm hành động xưởng theo ý nghĩa lệnh

Đã sửa bộ đánh giá co-play để nhận diện lệnh được chọn bằng trường `Intent` của đúng observation, thay vì suy ra loại lệnh từ số thứ tự slot. Khi đối chiếu với candidate builder C++ v11, giả thuyết ban đầu rằng slot cao có thể chứa lệnh kết hôn là sai: slot 72–79 dành cho kết hôn, còn slot 89–128 dành cho tám nhóm, mỗi nhóm năm thao tác xưởng. Vì vậy bộ đếm slot cũ đúng với layout hiện tại; thay đổi này giúp bộ đếm theo loại lệnh chịu được thay đổi slot về sau.

## Kết quả kiểm tra

- Python compile pass; `realm_coplay_v11_checks.py`: 28 checks pass, gồm kiểm tra layout candidate thật, transfer/authorize/work, GoodId và Intent ngoài miền.
- Chạy lại đúng 3 checkpoint Economic v23 trên cùng 20 thế giới validation (193000–193019), 6 vùng, 128 ngày. Selector hoàn tất với exit code 2 vì không seed nào đạt điều kiện; `selected_seed` rỗng và `peer_eligible=false`.
- Utility, cash, debt, insolvency, paid site work units, và kết luận không chọn model trùng hoàn toàn kết quả v23. Với 20 thế giới này, các số đếm theo Intent trùng bộ đếm theo slot cũ: worksite actions trung bình 6,00 / 6,70 / 5,05 mỗi thế giới; authorize 3,05 / 3,50 / 2,50; gửi Work 2,35 / 2,80 / 2,05. Đối chiếu nguồn C++ xác nhận phép đếm cũ hợp lệ theo layout v11 hiện tại.
- Trung bình paid site work units vẫn là 9,4 / 11,2 / 8,2; Fiber và Cloth cuối kỳ gần như bằng không. Kết quả này tiếp tục cho thấy chưa có checkpoint Economic đủ điều kiện.

## Giới hạn và dấu vết

Đây là cải thiện độ bền của instrumentation evaluator, không đổi policy, reward, chọn action, Realm transition hoặc kết quả gameplay. Test tổng hợp trước đó gán Intent kết hôn cho slot 100, nhưng candidate builder thật không thể tạo tổ hợp đó; test đã thay bằng kiểm tra layout thật. Codex tự review; chưa có reviewer độc lập.

- Kế hoạch: [M5 v24 semantic worksite counts](../design/M5-v24-semantic-worksite-counts-plan.md)
- Kết quả: [selection.json](../../build/m5-v24-corrected-economic-selection-final/selection.json)
- Hash evaluator: `C69F61DB25C6701C3C9A26EA90782115FF42548F92B62BB7DACA260B1B3C5482`
- Hash selector: `A2A53D9BAB6A35A78C05860023931FC5087C9135F4DD7489987E7C9629800A5F`
- Hash kết quả: `5CAF0F285EE67098128DB943D615AFF034D221C5FEBE33F1FA512E8850821FD6`
- Manifest: [m5-v24 manifest](../../artifacts/m5-v24-semantic-worksite-counts-2026-10-21-manifest.json)
