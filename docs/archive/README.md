# Kho lưu lịch sử — không phải nguồn điều hành

Từ 2026-10-18, điểm vào hiện hành là [docs/README](../README.md), [plan](../IMPLEMENTATION_PLAN.md), [dev paper](../DEVELOPER_GUIDE.md) và [CURRENT_TASK](../CURRENT_TASK.md).

Kho này giữ nội dung trước khi tổ chức lại, gồm design v3, draft/research M5, paper kỹ thuật 15/10, protocol RL theo phiên bản và các prompt/review/handoff cũ. Các file có thể ghi APPROVED, “hiện tại”, “bước tiếp theo”, hoặc “đã pass” theo thời điểm cũ. **Không chạy lệnh hoặc tiếp tục code từ các câu đó.**

Các quyết định còn hiệu lực đã được hợp nhất vào IMPLEMENTATION_PLAN. Bản cũ dùng truy nguyên chi tiết/nguồn và artifact của run tương ứng; không làm luật mới, không phủ định evidence hiện hành. Link tương đối được sửa theo vị trí lưu trữ, không thay số liệu/checkpoint hoặc sửa kết quả xấu.

| Nhóm | Mục đích |
|---|---|
| `design/` | Plan v3, M5 draft và nghiên cứu/quyết định trước hợp nhất |
| `developer/` | Paper/sơ đồ source và refactor notes theo snapshot |
| `experiments/` | Protocol/schema/history RL đã khóa theo phiên bản |
| `handoff/` | Prompt, review và chỉ dẫn cũ đã được thay bằng CURRENT_TASK |

Evidence đang được dẫn vẫn ở `docs/reports/`; `.txt` trong `docs/guides/` là đầu vào package theo bản build, không được di chuyển làm hỏng script. Không xóa model/checkpoint/source-lock/manifest nếu chưa xác minh dependency và quyền xóa. Trạng thái archive không chứng nhận một thí nghiệm đạt gate.
