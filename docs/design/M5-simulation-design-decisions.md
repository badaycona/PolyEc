# M5 — Quyết định thiết kế mô phỏng đã duyệt

Ngày: 2026-10-17. Người dùng đã đồng ý hướng thiết kế trong báo cáo nghiên cứu và yêu cầu cập nhật docs. Đây là quyết định về mô hình, không phải chứng nhận code đã có hoặc phê duyệt mọi tham số cân bằng. Áp dụng Windows; giữ C++ core độc lập Godot. Khi phần mô tả cũ mâu thuẫn, dùng chỉ đạo người dùng/AGENTS hiện hành và bản làm rõ này; không xóa bằng chứng lịch sử.

## 1. Nói bằng ngôn ngữ đời sống

Mỗi NPC là một người có danh tính và cuộc đời: tuổi tăng theo thời gian game; có nhu cầu ăn, ngủ, nghỉ; có thể học, làm việc, di chuyển, chăm người khác, giao tiếp, bị bệnh và chết. Tuổi, sức khỏe, kinh nghiệm, hoàn cảnh và quan hệ ảnh hưởng việc họ làm được và cách họ nhìn nhận lựa chọn; không biến một chỉ số thành quyền pháp lý tự động.

Một ngày có nhiều hoạt động, không phải một lần chọn rồi bận đến hết ngày. Có việc tốn thời gian, việc chỉ tạo cam kết, việc tương thích để cùng làm và việc tranh cùng sự chú ý hoặc đòi có mặt ở hai nơi. Ngủ khi có người phụ thuộc không có nghĩa vẫn cung cấp đủ chăm sóc trực tiếp.

NPC có thể nhận ca trùng giờ, hứa chăm nhiều người hoặc cố làm quá sức. Game không từ chối chỉ vì quyết định thiếu khôn ngoan. Họ chỉ làm được phần thực sự dành thời gian, sức lực và tài nguyên; phần thiếu dẫn đến kết quả thất bại, thiếu chăm sóc, nợ nghĩa vụ hoặc hậu quả khác nếu nghiệp vụ tương ứng đã được định nghĩa. Không mặc định ai cũng làm được 16 giờ, cũng không cấm mọi người sau một mốc giờ chung.

Tính cách là khuynh hướng tương đối bền; tâm trạng, mệt mỏi và kiến thức thay đổi nhanh hơn. Hai người cùng hoàn cảnh có thể chọn khác nhau và một người cũng có thể đổi ý. Ngẫu nhiên tạo khác biệt có điều kiện, không thay tính cách bằng bốc thăm vô nghĩa. Policy được học quyết định chiến lược trong bản hoàn chỉnh; world thực thi và ghi hậu quả, không âm thầm chọn phương án tốt hộ NPC.

## 2. Mô hình kỹ thuật tương ứng

| Quyết định | Luật cần giữ | Cách kiểm chứng |
|---|---|---|
| D1. Nhiều phiên hoạt động/ngày | Tách job/quan hệ dài hạn, lịch mong muốn, phiên thực hiện và lượt suy luận. Interval có thời lượng và đơn vị; xử lý theo ranh giới sự kiện, không bắt chạy policy mỗi phút. | Một người làm/học/nghỉ trong ngày; chia horizon cho kết quả tương đương khi inputs không đổi. |
| D2. Cơ thể có trạng thái riêng | Phân biệt nhu cầu/thiếu ngủ, mệt do gắng sức và sức khỏe. Tiến triển từ hoạt động thực, môi trường và đặc điểm cá nhân; không đồng nhất thành busy hoặc capacity. | Hai người có sức bền khác nhau phản ứng khác; không nghỉ thì không nhận hiệu ứng nghỉ; hậu quả có nguyên nhân truy được. |
| D3. Routine có thể sửa | Lịch thường lệ là kế hoạch có thể bị gián đoạn, không khóa quyền quyết định. Policy cung cấp ưu tiên/điều chỉnh; scheduler cung cấp cơ hội quyết định với ngân sách tính toán hữu hạn. | Có thể nhận thêm ý định khi đang làm; gián đoạn/đổi ưu tiên có chi phí và kết quả rõ; không tự mất job do lệnh không liên quan. |
| D4. Đồng thời theo ngữ nghĩa | Tách requested demand khỏi delivered attention, presence, service và output. Tương thích phụ thuộc loại/context; không nhân đôi thời gian độc chiếm hay dịch vụ chăm sóc. | Làm/học độc chiếm không cùng nhận toàn bộ credit; trường hợp tương thích có test riêng; thời gian chăm nhóm không tự biến thành chăm trực tiếp đầy đủ cho mọi người. |
| D5. Kinh tế có nguồn chuẩn | Ledger/kho/quyền sở hữu và nghĩa vụ là state chuẩn; chất lượng sống là kết quả tổng hợp, không thay tài sản thật. Mục đích chi là dữ liệu phân loại; chỉ tách nghiệp vụ khi khác quyền, settlement hoặc hậu quả. | Tiền/hàng bảo toàn qua giao dịch; thêm nhãn chi không đòi thêm action chiến lược; thiếu tiền không sinh tài sản để hoàn thành lời hứa. |
| D6. Quan hệ thưa và có loại | Quan hệ có hướng/loại/trạng thái và nguồn phát sinh; membership, sở hữu, giám hộ, tình cảm và thông tin không đồng nhất. Không ma trận N×N, không bản sao Person. | Đổi hộ không tự xóa họ hàng/nợ/giám hộ; sự kiện quan hệ có cause; policy không biết thông tin chưa nhận được. |
| D7. RL theo kết quả | Shared learned weights, memory/sampling cá nhân. Observation giới hạn theo điều biết; reward và evaluation đo kết quả/thất bại thực, không thưởng nút bấm hay ép mọi người thành một chiến lược. | Phân biệt baseline và trained policy; test lặp hành động không tạo reward vô hạn; đánh giá nhiều seed/bối cảnh/đặc điểm và hành vi không mong muốn. |
| D8. Một lát đời sống xuyên suốt trước | Nối command → lịch → thực hiện → nhu cầu/kết quả → ledger/hậu quả → view/save → observation/evaluation. Đây là thứ tự kiểm chứng, không bỏ bớt scope milestone. | Kịch bản một ngày có nhiều hoạt động và thiếu hụt chạy xuyên core/native/Python theo cùng luật trước khi mở rộng diện hệ thống. |

### Tự do quyết định không phải vô hạn thực hiện

- Ý định, đề nghị, chấp nhận cam kết, nỗ lực và settlement là các bước khác nhau. Không loại lựa chọn vì rủi ro, quá tải hoặc dự báo thất bại; cũng không cho sửa state trái quyền.
- Reject nguyên tử cho command hỏng cấu trúc, ID/quyền thao tác/consent không hợp lệ, overflow hoặc vi phạm bảo toàn tại settlement. Một nỗ lực hợp lệ nhưng thất bại có thể commit chi phí/kết quả thực. Không gọi thiếu tiền dự báo là lỗi cấu trúc.
- Không đồng nhất luật trong world với API: nếu có gameplay vi phạm/kháng lệnh, cần action/nỗ lực và hậu quả, không mở đường giả quyền hay sửa ledger trực tiếp.
- Allocation phải có provenance từ quyết định actor/player. Trường hợp hòa/chưa quyết định được ghi rõ và cần cơ hội giải quyết; không mặc định ID nhỏ thắng hay tự tối ưu hạnh phúc. Giải pháp tạm ở evaluator không phải luật cuối cho mọi tương tác.
- Điều khiển UI và AI cùng command/transaction pipeline. Training dùng cùng core; không triển khai bộ luật Python thứ hai.

### Thời gian, quy mô và tái lập

Game time điều khiển tuổi, deadline, tiến độ và phục hồi; pause không tiến lịch, không chạy bù sau ngủ máy/mất focus. Quy đổi tick/phút/ngày/năm phải có một contract chung trước khi nối module; không suy đơn vị world từ tên Tick trong helper.

Tách cadence quyết định khỏi bước cập nhật world. Hoạt động đã nhận vẫn tiến triển khi policy không chạy. Dùng sự kiện/aggregate/lazy update có last-update-time và xử lý đủ ranh giới; NPC ít hoạt động vẫn già đi và chịu nghĩa vụ. Không skip sinh lý để đạt benchmark. Tách snapshot đọc, ý định cạnh tranh và commit để thứ tự duyệt actor không âm thầm quyết định kết quả; cơ chế phân bổ tài nguyên cạnh tranh cần spec riêng, không suy từ tie-break timeline.

RNG cần seed/state hoặc quy tắc tái lập; save/load không đổi kết quả vì gọi lại may rủi. Tối ưu bằng dữ liệu gọn/shared policy/quan hệ thưa trước; chưa có bằng chứng thì không cam kết một triệu NPC trên CPU. Body chi tiết, độ trễ và kích thước event/history cần benchmark và ngân sách riêng.

## 3. Phần chưa được tự chốt

Chưa chốt các hệ số mệt/phục hồi, độ dài ngày/năm, tuổi thọ, xác suất bệnh/chết, ngưỡng nguy hiểm, reward weights hoặc cadence suy luận. Không tự suy ra giới hạn làm 8/16 giờ, tuổi cấp quyền, bảng bệnh học hay mô hình y khoa đầy đủ. Bệnh/chết cần chuỗi nguyên nhân, vòng đời quyền/nghĩa vụ và test trước khi trở thành gameplay; không thêm phạt ngẫu nhiên để buộc NPC hành xử tốt.

Không thay RL bằng utility/GOAP/routine viết tay trong bản hoàn chỉnh. Có thể dùng baseline có nhãn để kiểm thử đường ống. Không coi tài liệu này là cho phép train dài, mua GPU hoặc mở dịch vụ cloud.

## 4. Nguồn tham khảo và điều chỉnh khi áp dụng

Các nguồn dưới đây hỗ trợ lựa chọn thiết kế; D1–D8 là quyết định riêng của PolyEc, không phải cơ chế được sao chép nguyên bản hoặc bảo đảm hiệu năng từ tác giả.

| Nguồn | Nên lấy | Không sao chép máy móc |
|---|---|---|
| Tarn Adams, [Simulation Principles from Dwarf Fortress](https://www.gameaipro.com/GameAIPro2/GameAIPro2_Chapter41_Simulation_Principles_from_Dwarf_Fortress.pdf) | Xây lặp, phân rã cơ chế, dùng tương tự đời thực và tránh chi tiết không tạo giá trị. | Không bắt đầu bằng mô phỏng mọi thứ; không suy độ sâu lớn đồng nghĩa gameplay tốt. |
| Ingebretson/Rebuschatis, [Concurrent Interactions in The Sims 4, GDC 2014](https://www.gdcvault.com/play/1020413/Concurrent-Interactions-in-The-Sims) | Tư duy nhiều tương tác, tập hoạt động đang diễn ra và ràng buộc tương thích theo dữ liệu. | Không bê cơ chế ưu tiên người chơi/hủy tự chủ thành quyền chọn hộ mọi NPC; concurrency hoạt động không phải đa luồng CPU. |
| Grimm et al., [ODD protocol 2020](https://www.jasss.org/23/2/7.html) | Mô tả purpose/patterns, entities/state/scales, process/scheduling, design concepts, initialization, input và submodels. Phân biệt state, tham số và mẫu hành vi cần kiểm chứng. | ODD không phải engine, bằng chứng cân bằng hoặc chứng nhận performance. Viết đủ nghĩa, không áp thêm thủ tục cho mọi helper nhỏ. |
| Paradox, [Victoria 3 Dev Diary 13: Standard of Living, 2021](https://steamcommunity.com/games/529340/announcements/detail/2965045886379003737) | Liên hệ thu nhập, tiêu dùng, của cải và đời sống; mặt hàng thay thế và feedback kinh tế. | Đây là thiết kế lịch sử theo nhóm dân, không chứng minh mô phỏng từng người hay hành vi hiện tại. Không dùng thuật toán mua tối ưu thay lựa chọn RL. |
| Wube, [Factorio Friday Facts 421](https://www.factorio.com/blog/post/fff-421) | Giảm cập nhật vô ích, dữ liệu và đăng ký xử lý dùng chung; đo bottleneck trước khi song song hóa. | Không cho NPC ngừng già/đói/trả nợ khi không được update. Entity nhà máy không tương đương một người có đời sống. |
| Suárez et al., [Neural MMO 2.0](https://arxiv.org/abs/2311.03736) | Task/predicate/outcome, đánh giá tổng quát hóa và tránh đồng nhất survival reward với học đủ hệ thống. | Quy mô benchmark của nguồn không chứng nhận triệu NPC; không bê luật Python hoặc client web vào sản phẩm native. |
| Borbély et al., [Two-process model of sleep regulation: a reappraisal](https://pubmed.ncbi.nlm.nih.gov/26762182/) | Phân biệt áp lực ngủ tích lũy và ảnh hưởng nhịp ngày đêm để tránh một thanh fatigue giải thích mọi thứ. | Chỉ là cơ sở khái niệm về ngủ, không phải công thức cơ bắp/bệnh hay ngưỡng y khoa cho game. |

## 5. Trạng thái và bước tiếp tục

Thiết kế đã duyệt không đồng nghĩa code đã đồng bộ. Bản thay đổi đi kèm liệt kê phần cần giữ, sửa, bổ sung và gate: [M5 simulation change map](../handoff/M5-simulation-change-map.md). Không dùng commit tài liệu này làm bằng chứng build/train/benchmark hoặc công bố các module local chưa commit.
