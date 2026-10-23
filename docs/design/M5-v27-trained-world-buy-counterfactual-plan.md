# M5 v27 — Buy–Hold counterfactual trên Realm checkpoint đã train

## Câu hỏi

v23 log reward dương cho Buy trên rollout đầy đủ, nhưng Buy thấp hơn Hold trong reset snapshots. Kiểm tra cùng một lệnh Buy và Hold trên chính Realm states đã lưu ở cuối ba v23 checkpoint, giữ actor, đối tác, RNG và thời điểm giống nhau.

## Phạm vi

- Đọc `checkpoint.pt` của ba run Economic v23; khôi phục 16 Realm snapshots, scenario, peer RNG/hidden state và rollout progress qua `RealmCoPlayBatch.load_state_dict`.
- Trong mỗi context, lấy các GoodId Buy legal; clone snapshot actor đó thành hai bản và chạy Buy/Hold lần lượt trong đúng một lượt chuyển 7 ngày.
- So reward, utility, cash/debt, health/nutrition và paid site work sau transition. Giữ invariants; không dùng policy để chọn hành động trong counterfactual.
- Không thay reward/training/gameplay hoặc chọn model.

## Kiểm chứng

- Compile Python; chạy trên đủ ba checkpoint/16 world.
- Mỗi cặp phải bắt đầu cùng Realm fingerprint và peer RNG; Buy phải legal ở bản gốc; Hold và Buy đều phải hoàn tất transition invariant.
- So phân bố Buy−Hold theo bối cảnh và GoodId với reset-state audit v26, đồng thời ghi rõ giới hạn chỉ là một bước counterfactual.
