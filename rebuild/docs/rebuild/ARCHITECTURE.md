# Kiến trúc mục tiêu của bản xây lại

Tài liệu hỗ trợ [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md). Đây là ranh giới trách nhiệm, không khóa tên class, module, folder hoặc thư viện ngoài hướng công nghệ đã nêu.

## 1. Các lớp

### C++ simulation core

Là nguồn luật cho người, tài sản, cơ sở, thị trường, địa điểm, hoạt động, chính trị, thời gian và save/load. Core không phụ thuộc Godot. Nó cung cấp view chỉ đọc, danh sách lựa chọn phù hợp với dữ liệu actor và API nhận command.

### Godot app

Hiển thị bản đồ, hồ sơ, thông tin người chơi được biết, lựa chọn/chi phí/thời gian và kết quả. UI gửi command; nó không sửa trực tiếp tiền, kho, clock, quyền hoặc trạng thái NPC. Debug inspector toàn tri phải tách khỏi player view.

### Python trainer/evaluator

Gọi đúng C++ core qua binding. Python có thể chuẩn bị batch, lưu rollout và tính thuật toán RL; nó không được thay bằng bản mô phỏng kinh tế/luật thứ hai.

### Native policy runtime

Chỉ thêm sau khi game hoàn chỉnh và policy đã qua đánh giá. Shared weights, memory/sampling theo actor ID. Runtime từ chối model có contract khác và ghi rõ khi đang dùng dev baseline.

## 2. Luồng command

```text
Godot hoặc policy
  → tạo command chung: actor, verb, target, parameters, context/version
  → core nạp dữ liệu domain có chủ và kiểm điều kiện/quyền/consent
  → chuẩn bị thay đổi và kiểm số học/tài khoản/hàng/time/event limits
  → commit trọn giao dịch hoặc reject không đổi state
  → cập nhật hoạt động, thời gian và sự kiện theo luật
  → cập nhật knowledge của đúng observer
  → tạo view chỉ đọc hoặc dữ liệu save
```

Không để UI, Python hoặc runtime có đường phụ để viết trực tiếp world state. Command bên ngoài, NPC và test harness dùng cùng luật; khác nhau ở ai đề xuất lệnh và thông tin nào họ thấy.

## 3. Chia trách nhiệm mà không chia thành vô số file

Action schema/definition dùng chung mô tả verb, tham số, miền hợp lệ, điều kiện, chi phí/thời gian và kiểu kết quả. Các domain nhỏ (thị trường, sản xuất, vận tải, cơ thể, chính quyền) giữ luật của mình và cùng gọi dịch vụ giao dịch/clock/save khi phù hợp. File/class chỉ tách khi ownership hoặc hành vi khác đáng kể; không có quota một cặp file cho mỗi action/biến thể. Không dồn tất cả hành vi vào Realm lớn hoặc một switch tổng.

Data chính có owner rõ. Cache/index là dữ liệu dẫn xuất, rebuild được từ dữ liệu có chủ và phải qua phép kiểm tương đương trước/sau load. Một NPC có một record/state gọn; quan hệ và hoạt động của hàng triệu NPC không dùng ma trận dense hoặc một scene/model object chiến lược riêng cho mỗi người.

## 4. Thời gian và hoạt động

Chọn đơn vị và nhịp mô phỏng sau khi vertical slice đầu có hoạt động cần định lượng. Không mang cadence theo family/ngày hoặc tick constant cũ vào mặc định. Một hoạt động đang chạy phải có actor, mục tiêu, tiến độ/thời hạn, nguồn lực đã cam kết, điều kiện kết thúc/hủy và hậu quả. Core tiến hoạt động một cách xác định; khi thế giới đổi, NPC có cơ hội phản ứng theo quy tắc đã chọn.

Pause, mất focus hoặc máy ngủ không tạo thời gian game ngoài ý muốn. Save giữa hoạt động phải nạp lại nguồn đã cam kết và tiến độ đúng một lần; resume không lặp tiền, hàng, kinh nghiệm hoặc kết quả.

## 5. Save, version và replay

Bản xây lại có thể khởi đầu bằng schema sạch; không cần mang tất cả lịch sử migration của project cũ. Mỗi format lưu rõ version, dùng reader có kiểm tra giới hạn, staging trước khi commit, migration tường minh nếu sau này có save người dùng cần giữ. Seed/config/replay cùng phiên bản luật phải cho kết quả lặp lại trong phạm vi determinism đã chọn.

Schema đổi khi nghĩa dữ liệu đổi, không chỉ khi tên file đổi. Bất kỳ model/observation/action schema nào có nghĩa mới phải có contract mới; không nhận model cũ như tương thích chỉ vì tensor tình cờ cùng chiều.

## 6. Kiểm tra theo lớp

- Sửa luật core: build target core liên quan; test thành công, reject, biên và invariant trực tiếp.
- Đóng chuỗi gameplay: C++ command→transaction→time→save/load trong một world.
- Đổi API/ABI/adapter/observation/action/reward: build và kiểm consumer phù hợp như Python, native hoặc Godot ở mốc tích hợp.
- Trước khi gọi game playable: chạy luồng UI thật, save/load, tình huống thiếu nguồn lực, hai vùng nếu thuộc phạm vi, benchmark và package.
- Huấn luyện: chỉ sau gate gameplay; mỗi run lưu nguồn số liệu và chart riêng như [DEVELOPMENT_AND_RL](DEVELOPMENT_AND_RL.md).
