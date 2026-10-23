# M5 v17 — đồng bộ đối tác dev và đánh giá lại PPO economic

## Thay đổi

Trong khi chưa có checkpoint đã qua đánh giá, adapter huấn luyện và evaluator dùng cùng bộ điều khiển dev đọc observation, candidate features và legal mask hiện tại. Manifest và thông tin resume khóa phiên bản `realm-v11-observation-dev-v2`, nên không thể tiếp tục checkpoint tạo từ luật đối tác khác.

## Kết quả huấn luyện và lựa chọn

Runset economic có 3 seed, 6 vùng, 25 cập nhật và 25.600 quyết định mỗi seed; manifest xác nhận ngân sách bằng nhau và `peer_eligible=false`. Dùng chung 20 validation worlds (seed 190000–190019), 128 ngày và cùng baseline đối tác cho cả ba policy.

Không seed nào đủ điều kiện chọn. So với dev baseline, utility trung bình ghép cặp thấp hơn lần lượt 0,583 / 0,593 / 0,592; CI95 tương ứng [-0,629, -0,537], [-0,650, -0,536], [-0,653, -0,530]. Tiền cuối kỳ thấp hơn khoảng 370–385; nợ cao hơn 84–99; số quan sát bị mất khả năng thanh toán cao hơn 0,5–0,6 cá thể mỗi world. Utility so với initial cũng không cho thấy cải thiện có ý nghĩa. `selected_seed=null`, không tạo bundle peer. Do đó không chạy holdout 100 world cho các ứng viên đã bị validation loại.

## Chẩn đoán huấn luyện

Log run 25 cập nhật cũ đo approximate KL, clip fraction và ratio trước optimizer step; các giá trị đó không mô tả thay đổi vừa cập nhật nên không dùng để chẩn đoán mức học. Đã sửa evaluator log để tính lại policy sau mỗi optimizer step, đồng thời ghi policy/value loss, gradient norm và phân vị reward, advantage thô, advantage chuẩn hóa và return. Smoke một cập nhật / 1.024 quyết định ghi đủ trường mới. Sau đó chạy lại seed 1 đủ 25 cập nhật / 25.600 quyết định; SHA policy trùng đúng với policy seed 1 trong validation, nên kết quả chọn model không đổi. KL sau optimizer của run này có median 1,97×10⁻⁶, min 1,35×10⁻⁷, max 1,07×10⁻⁵; clip fraction luôn 0; critic explained variance từ -0,94 đến 0,55. Các action histogram cũng ghi nhận create/fund 428 lần và worksite thao tác 115 lần trong 20 validation worlds. Dữ liệu chỉ ra policy có thử các hành động xưởng nhưng chưa cho kết quả kinh tế cạnh tranh; không dùng làm learned peer.

## Phạm vi còn mở

Đối tác train/eval đã đồng bộ, nhưng PPO chưa học policy kinh tế đạt baseline và không được dùng làm learned peer. 20 world × 128 ngày là bước loại ứng viên, không thay thế R1 (100 world độc lập, đủ horizon). R2/R3 interventions, learned-policy cross-play, ONNX/runtime parity, profile B và phần M5/core còn lại vẫn chưa hoàn tất. Kết quả chỉ là self-review trên checkout đang có thay đổi song song.
