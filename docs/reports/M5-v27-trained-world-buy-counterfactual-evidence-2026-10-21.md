# M5 v27 — Buy–Hold đối chứng trên checkpoint đã train

## Phương pháp

Khôi phục đúng 16 Realm world cuối cùng, scenario, peer hidden state và RNG từ checkpoint của mỗi seed v23. Với mỗi candidate Buy còn hợp lệ, clone đúng world đó thành hai bản, giữ cùng actor/đối tác/RNG, rồi chạy Buy hoặc Hold trong một transition 7 ngày. So cả utility ngay sau lệnh và reward sau 7 ngày.

## Kết quả

- Seed 1 có 10 cặp, seed 3 có 5 cặp, seed 2 có 0 candidate Buy hợp lệ trong 16 world checkpoint. Tổng 15 cặp, mọi invariant đúng; fingerprint Realm và hash peer RNG khớp trước từng cặp.
- Cả 15 Buy đều tăng reward so với Hold; reward chênh từ +0,04 đến +0,32. Utility tức thời tăng 0,44–3,48 điểm. Actor cash giảm đúng 12 cho mỗi lệnh; household fund và debt không đổi.
- Không cặp nào có chênh paid work units. Health/nutrition giữ nguyên trong từng cặp. Mức tăng utility khớp lượng hàng nhân giá tham chiếu trừ 12 tiền đã trả: Fiber +0,44; Iron +1,68; Fuel +0,60; Tools +2,48; Horse +3,48 (mỗi lệnh 4 món).

## Kết luận

Giá hàng ở các world đã phát triển xuống mức tối thiểu 2 mỗi món, cộng 1 phí giao dịch thì bốn món chỉ tốn 12; utility vẫn định giá chúng theo giá tham chiếu, ví dụ Horse là 90 mỗi món. Do đó Buy tạo lợi ích tài sản trên giấy ngay cả khi không có sản xuất hoặc cải thiện đời sống trong tuần đó. Đây giải thích reward Buy dương và đưa ra động cơ tích trữ khi thị trường dư hàng. Reset snapshots v26 chưa có mức giá thấp này nên Buy ở đó giảm utility.

Chưa sửa công thức reward trong lượt này. Định giá theo giá thị trường có thể thay đổi utility khi actor di chuyển hoặc giá thay đổi, nên cần khóa phương án bằng thí nghiệm đối chứng trước khi bump reward version và train lại. Self-review của Codex; chưa có reviewer độc lập.

- Kế hoạch: [M5 v27 checkpoint counterfactual](../design/M5-v27-trained-world-buy-counterfactual-plan.md)
- Dữ liệu: [audit.json](../../build/m5-v27-trained-world-buy-counterfactual-final/audit.json)
- Manifest/hash: [m5-v27 manifest](../../artifacts/m5-v27-trained-world-buy-counterfactual-2026-10-21-manifest.json)
