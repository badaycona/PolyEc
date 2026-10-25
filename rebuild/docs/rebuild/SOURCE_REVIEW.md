# Kiến thức đã chọn lọc từ project cũ

Ngày2026-10-25. Bộ docs này được biên tập từ các nguyên tắc còn hữu ích của PolyEc và thảo luận scope mới. Sau đó người dùng yêu cầu xóa toàn bộ file không cần thiết, chỉ giữ docs định hướng. Vì vậy nguồn cũ được ghi bằng tên để giải thích xuất xứ, không là liên kết tới file phải tồn tại.

## Phần còn được dùng

| Nguồn trước đây | Nội dung đã chọn | Nơi giải thích trong bộ mới |
|---|---|---|
| IMPLEMENTATION_PLAN, phần nền NPC | NPC là actor; kinh tế chính trị; một policy; tiền riêng; quyền, nhu cầu và thông tin | [Scope mới](IMPLEMENTATION_PLAN.md), [mô hình NPC](NPC_MODEL.md) |
| TECHNICAL_PROBLEMS, DEVELOPER_GUIDE | Danh tính, sở hữu, giao dịch nguyên tử, tiền/hàng có đối ứng, thời gian/hoạt động | [Luật thế giới](WORLD_AND_ACTIONS.md), [kiến trúc](ARCHITECTURE.md) |
| CORE_ACTION_ARCHITECTURE, NPC_ACTION_CATALOG | Definition dùng chung, tiến trình theo người, registry rõ, lời nhờ/phản hồi và nhóm hành động | [Nhóm hành động](WORLD_AND_ACTIONS.md), [kiến trúc](ARCHITECTURE.md); bỏ quota và động từ chi tiết |
| DEVELOPMENT_WORKFLOW, tài liệu tích hợp | Kiểm core theo phạm vi; chuỗi command/giao dịch/thời gian/save; kiểm UI thật khi nhóm ổn định | [Quy trình và RL](DEVELOPMENT_AND_RL.md) |
| Tài liệu huấn luyện/đánh giá | Cùng C++ core, dữ liệu version rõ, seed tách biệt, raw metrics/biểu đồ, đánh giá độc lập | [Quy trình và RL](DEVELOPMENT_AND_RL.md) |
| Atlas catalog | Tên, vai trò, bất cân xứng nguồn hàng của24 lãnh địa và3 khu triều đình | [Bảng Atlas](WORLD_ATLAS_REFERENCE.md) |
| Thảo luận với người dùng | Mức cơ bản cho từng hệ, bữa theo giá/thời gian/độ ngon, kỹ năng qua thực hành, xu hướng/mục tiêu có thể khác | [Quyết định](DECISIONS.md), [NPC](NPC_MODEL.md), [scope](IMPLEMENTATION_PLAN.md) |

## Phần không mang sang

Roadmap M5/R1–R4, các plan theo từng mặt hàng/xưởng, ba family policy, quỹ hộ, cadence7/30 ngày, auto-consumer, quota action/file, tensor/model/save schema, fixture/hash, bảng giá và siêu tham số cũ không là yêu cầu của bản mới.

Người dùng đã yêu cầu xóa source, save, build, model/checkpoint, artifacts, report, archive và snapshot, sau đó xác nhận xóa cả Git. Tuy nhiên các lệnh xóa bị công cụ từ chối với thông báo `blocked by policy`; project cũ chưa được xóa. Không có package hoặc kết quả chạy cũ cần thiết để hiểu bộ tài liệu này. Session mới phải bắt đầu ở workspace sạch, chỉ dùng docs, không phục hồi hay tham khảo source cũ làm baseline.

## Tính độc lập của bộ docs

Nội dung cần thiết đã được viết lại trong bộ này. Mọi liên kết tài liệu trỏ tới file được giữ. Có thể mang nguyên thư mục docs sang nơi code mới; không cần khôi phục kho legacy, không có script/toolchain/binary cũ phải chạy.

Đây là định hướng, chưa phải game, API hoặc learned policy đã triển khai. Những hệ số, conviction, danh sách xu hướng và ngân sách chưa chốt phải đọc đúng nhãn tại DECISIONS; không suy ra từ thử nghiệm cũ.
