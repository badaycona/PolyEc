> LƯU TRỮ 2026-10-18. Snapshot trước hợp nhất; không thực thi prompt hoặc dùng trạng thái trong file này làm chỉ đạo hiện tại. Đọc [nguồn chuẩn](../../README.md).

# Review thiết kế simulation với OpenCode local

Ngày: 2026-10-15. Đây là hồ sơ review, **không phải spec được người dùng duyệt**. Không triển khai gameplay hoặc thay quy mô sản phẩm.

## Công cụ và bằng chứng thực hiện

- Checkout GitHub tại `106f8e4`; thư mục local ban đầu trống, không có thay đổi PolyEc cần stash. Repo cha không bị sửa.
- OpenCode Bun `1.18.30`, model cấu hình repo `opencode/step-5-preview-free`.
- Phiên review: `ses_ec2c4f9d1ffeDp1pxyjE7GMnPs`. Model đã dùng `read` đọc AGENTS.md, WORK_STATUS.md, docs/handoff/opencode-handoff.md, docs/design/simulation-complexity-scope.md và docs/design/late-medieval-world-design-discussion.md, rồi trả review bằng tiếng Việt.
- Lỗi 403 trước đó xuất hiện khi phiên được đặt `bash: deny`. Cùng executable/model/credential trả lời thành công khi đổi sang `bash: ask`; xem cách chạy trong opencode-handoff.md. Không cần cài lại hoặc đổi tài khoản.
- Không chạy build/test, benchmark hoặc training trong review. Không có bằng chứng gameplay hay policy mới.

## Ba nhóm quyết định cần làm rõ

1. **Luật và quyền:** quyền nào áp dụng chung, quyền nào theo jurisdiction hoặc loại tài sản; cơ quan nào công nhận, xét tranh chấp và thi hành? Một bộ luật thống nhất và lõi chung có biến thể đều là phương án thiết kế, chưa có lựa chọn cuối.
2. **Hôn nhân và thừa kế:** tách quyền đề nghị/chấp thuận hôn nhân khỏi luật chuyển tài sản, đất, tước và chức vụ. Quyết định ở một trục không mặc định quyết định trục kia. Cần ghi quyền của cá nhân, đại diện, người phụ thuộc, góa phụ và giám hộ; liên minh là cam kết riêng.
3. **Phân quyền và thông tin:** vua có thể ra lệnh gì; người giữ chức đại diện cho ai, dùng nguồn lực nào và biết thông tin gì; địa phương tự xử lý việc thường lệ tới đâu và khi nào vụ việc lên cấp vua? Can thiệp trực tiếp và tự vận hành có thể cùng tồn tại nếu thẩm quyền rõ.

Các lựa chọn ảnh hưởng command/transaction, observation, action targets, memory và đánh giá policy. Mức tăng chi phí và độ khó học chưa được đo. Ba họ policy hiện tại không phải ba giai cấp; không suy ra cần model riêng cho từng nghề/gia tộc.

## Tác vụ thiết kế tiếp theo được đề xuất

Chuẩn bị **bản đề xuất nền xã hội và năm ví dụ có sổ**: một hộ, một xưởng, một người giữ quyền, một cuộc cưới và một vụ thừa kế. Chưa giao triển khai và chưa chốt luật thay người dùng.

Tiêu chí tối thiểu:

- Mỗi lựa chọn có trạng thái: người dùng đã chọn, đề xuất chờ duyệt, hoặc hoãn có lý do; trả lời các câu mở ở mục 11 của tài liệu nền xã hội.
- Mỗi ví dụ có actor, thẩm quyền, tài sản/quyền/nghĩa vụ trước và sau, chuyển giao có đối ứng, người biết thông tin, cơ quan thi hành và nhánh bị từ chối/thất bại.
- Phân biệt tài sản riêng/chung, chức bổ nhiệm/tước thế tập, claim/quyền kiểm soát; không nhân đôi tiền, lao động hoặc quyền độc quyền.
- Liệt kê primitive cần có và tác động dự kiến tới action/observation/reward/save/model; không đổi schema trong bước thiết kế.
- Ghi điều kiện khởi tạo và bằng chứng cần chạy sau này: test luật/transaction, đánh giá policy, benchmark. Tài liệu thiết kế không tự chứng nhận các gate đó.
- Sáu vùng có thể là fixture kiểm chứng được đề xuất; không trở thành giới hạn sản phẩm. Thứ tự A–F cũng chưa tự trở thành kế hoạch đã duyệt.

## Phản biện review ban đầu

Review đầu của OpenCode đã ghép quyền tự quyết hôn nhân với trưởng nam thừa kế thành một phương án; diễn đạt luật thống nhất như trái yêu cầu lịch sử; và coi can thiệp vi mô tất yếu phá tự vận hành. Đây là các kết luận quá mạnh, không dùng làm spec. Codex đã gửi phản biện trong cùng phiên, yêu cầu tách các trục và sửa cách diễn đạt. OpenCode đã trả lời, rút kết luận gộp, xác nhận luật chung có thể là đủ và can thiệp trực tiếp có thể cùng tồn tại với tự vận hành; đồng thời xác nhận sáu vùng/A–F chưa duyệt. Những điểm trung lập ở trên là bản tổng hợp sau trao đổi để tiếp tục thảo luận.
