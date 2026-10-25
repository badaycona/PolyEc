# Thế giới, quyền sở hữu và hành động

Tài liệu hỗ trợ [IMPLEMENTATION_PLAN](IMPLEMENTATION_PLAN.md). Đây là hướng thiết kế, chưa phải code hay hành vi đã được kiểm. Khi có luật/gameplay mới, cập nhật Implementation Plan trước.

## 1. Thành phần của thế giới

- **NPC** là cá nhân có ID, cơ thể, tiền/tài sản, kinh nghiệm, xu hướng, mục tiêu, thông tin đã nhận và hoạt động đang làm.
- **Cơ sở** tạo hàng/dịch vụ bằng người, thời gian, vật tư, công cụ và quyền cụ thể; sản phẩm thuộc tài khoản/kho có chủ.
- **Tổ chức** có thành viên, quyền đại diện và tài sản riêng. Người giữ chức không sở hữu tiền tổ chức.
- **Hộ/quan hệ** có thể mô tả thành viên hoặc cư trú nhưng không là policy riêng hay ví chung.
- **Địa điểm/tuyến** có hàng, người, hoạt động, chi phí, thời gian và sức chứa. Vị trí ảnh hưởng điều ai làm được.
- **Người chơi** ra quyết định trong quyền đã xác định; điều khiển debug một NPC không phải năng lực trong game.

Đây là các loại dữ liệu/đối tượng cần thiết để mô tả vòng chơi; không yêu cầu tạo một class/file riêng cho từng hàng, người, giá hoặc biến thể.

## 2. Một yêu cầu hành động chung

Mỗi lệnh có actor ID, action/verb, mục tiêu, bộ tham số có tên và thông tin version/context để từ chối lựa chọn cũ khi thế giới thay đổi. Ví dụ:

```text
actor: người mua
verb: mua
target: hàng và nơi giao dịch
parameters: lượng, nguồn thanh toán
context: world/action version cần thiết để kiểm tra lại
```

Đi bộ/đi xe, đổi nơi đến hoặc lượng mua là khác tham số nếu cơ chế và luật giống nhau. Chỉ tách phần xử lý khi điều kiện, tiến trình, chủ thể/quyền, loại phí hoặc hậu quả khác thật sự. Không tạo vô số action class/file để biểu diễn từng tổ hợp; tránh một bộ xử lý duy nhất chứa switch mọi domain.

## 3. Ba phép hỏi khi actor chọn hành động

1. **Có thể thử không?** Actor có đúng tài sản, phương tiện, vị trí, thời gian và kiến thức để bắt đầu hoặc cố làm không?
2. **Có quyền/hợp pháp không?** Điều lệ, quyền sở hữu, consent và tư cách đại diện cho phép không?
3. **Nếu thử thì chuyện gì xảy ra?** Lệnh hoàn tất, bị cản, thất bại có chi phí hoặc tạo thông tin/hậu quả nào?

Không gộp mọi điều vào một cờ `legal` rồi bỏ khỏi lựa chọn. Nếu vi phạm là phần game được mô phỏng, phải có actor, phương tiện, chi phí, người bị ảnh hưởng, thông tin và phản ứng thật. Từ chối do lệnh sai định dạng/actor/sequence là khác với thất bại trong một hành động có thể thử.

## 4. Giao dịch và hậu quả

Trước khi ghi state, tính đủ nguồn/đích tiền và hàng, phí, giới hạn số nguyên, quyền, consent và thời gian. Nếu lệnh không thể bắt đầu vì request sai hoặc điều kiện đã đổi, không sửa một phần state. Nếu lệnh bắt đầu thành công nhưng sau đó thất bại theo luật game, ghi rõ phần chi phí/ảnh hưởng đã phát sinh.

Mỗi kết quả quan trọng cần có nguyên nhân mà người chơi/NPC có thể biết theo quyền của họ. Core có thể giữ sự thật toàn thế giới cho kiểm định; view của mỗi actor chỉ đưa điều đã nhận được.

## 5. Nhóm hành động cho vòng chơi đầu

Không khóa danh mục cuối từ brainstorm cũ. Với một lãnh địa, bắt đầu bằng một nhóm nhỏ đủ chạy chuỗi:

- sản xuất/làm việc có site, người, đầu vào và đầu ra;
- mua/bán/chuyển hàng có nơi, chủ sở hữu và tài khoản;
- ăn, uống, nghỉ và chăm sóc cơ thể bằng lựa chọn có nguồn thật;
- đi tới một địa điểm nếu vòng đầu cần nhiều nơi;
- yêu cầu/nhận lời/hoàn thành lời nhờ nếu có một bên thứ hai thật;
- quyết định quản trị có chủ thể, nguồn công và đối tượng bị ảnh hưởng.

Bữa ăn là lựa chọn trừu tượng theo chi phí, thời gian chuẩn bị và mức dễ chịu. Một lệnh chọn cả cách chuẩn bị; không chọn từng gia vị, tên món hoặc dụng cụ. Những đơn vị/giá/lượng cụ thể được chọn trong config trước khi code, ghi là trial nếu chưa hiệu chuẩn.

## 6. Kiểm chéo

Trong một Realm/world, chạy producer→sản phẩm trong kho có chủ→market/giao dịch→người mua nhận đúng hàng→dùng hàng→thay đổi cơ thể/tài sản→save/load→tiếp tục. Cùng kiểm actor sai, hàng thiếu, quyền/consent thiếu, target stale, lệnh lặp, tranh tài nguyên, pause và save giữa hoạt động. UI và AI phải gọi đúng cùng pipeline này.
