# M5 v28 — đối chứng cách định giá hàng trong checkpoint Economic

## Câu hỏi

Định giá hàng theo giá bán được tại vùng hiện tại có loại bỏ lợi ích Buy giả do giá chợ chạm sàn không, và nó có khiến giá trị utility đổi khi người mang hàng di chuyển vùng không?

## Phạm vi và cách tính

- Khôi phục ba checkpoint v23 và đọc candidate/mask, tồn kho và giá từng vùng từ observation của Realm C++.
- Với 15 Buy candidate trong các snapshot đã có Buy hợp lệ, so độ đổi utility giả định khi vẫn dùng giá tham chiếu với khi đánh dấu tồn kho theo giá bán hiện tại `max(1, local_price - 1)`, trừ đúng tiền mua.
- Với 7 nhóm hàng legacy có số lượng cá nhân trong observation, đo biên độ giá trị hàng nếu đánh dấu theo giá bán của mỗi vùng.
- Chỉ tính counterfactual offline; không sửa utility C++, reward version, policy, save hoặc gameplay.

## Kiểm chứng và giới hạn

Đối chiếu 15 mẫu Buy với audit v27 trên cùng checkpoint; ghi SHA từng checkpoint/policy và audit output. Giá trị theo vùng là giả lập kế toán từ observation hiện tại, không chạy lệnh Travel/Sell, không chứng minh kết quả thị trường sau khi hàng được bán. Nếu phương án định giá phụ thuộc vùng, cần tính cả chênh lệch điểm do di chuyển trước khi cân nhắc gameplay.

Review tự kiểm của Codex; không có reviewer độc lập.
