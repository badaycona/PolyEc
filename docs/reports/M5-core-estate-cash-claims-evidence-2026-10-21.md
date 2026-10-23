# C++ — thanh toán khoản vay từ tiền hồ sơ di sản

## Hành vi

Khi người vay qua đời, Realm ghi hồ sơ tài sản trước rồi dùng tiền mặt của đúng hồ sơ để trả toàn bộ khoản vay còn dư nếu đủ. Khoản tiền chuyển về Province firm hoặc credit pool đã ghi làm chủ nợ; hồ sơ và `Loan::paid` cập nhật cùng nhau. Nếu thiếu tiền, không trả một phần và không chọn chủ nợ ưu tiên. Tiền hộ, tài sản người sống và nợ cá nhân của người hưởng không bị dùng. Hàng, đất và xưởng chưa bị bán/chuyển; phân phối chưa chạy.

Không đổi save schema: schema24 đã lưu cash trong case và số đã trả của Loan. Các field này tiếp tục roundtrip.

## Kiểm tra trên source hiện tại

- Windows Release manager tests: pass; gồm prep thuần, trả nợ Province + pool, số dư không đủ, overflow và từ chối read-set stale.
- Windows Release Realm tests: 20.627 checks pass; fixture chạy pha tử vong thật, ghi nhận tiền case giảm đúng 20, pool tăng đúng 20, invariant và tổng tiền bảo toàn, rồi save/load khớp fingerprint cùng loan/case.
- Windows Release CTest: 19/19 pass, tổng thời gian 109,42 giây.
- Python binding: 273 checks; replay C++/Python khớp 64 ngày, batch reject nguyên tử, model bundle mismatch reject. Clock binding: 173 checks và 7 checkpoint khớp.
- GDExtension Release build: pass.
- Godot 4.5.1 headless: native 19/19, UI 18/18, campaign 49/49 và historical model guard 5/5 pass.
- `realm_smoke` 64 ngày chưa pass: project không có `godot_project/models/realm-v11`, trong khi smoke cần manifest và ba model tương thích. Lệnh thử dừng ở lỗi không đọc được manifest; không dùng bundle v6/v8 thay thế.

## Giới hạn

Codex tự review, chưa có reviewer độc lập. Chưa chạy Debug cho thay đổi hiện tại. Chưa xử lý hồ sơ estate cũ chưa gắn decedent; chưa thanh lý inventory/title/xưởng để trả nợ khi tiền mặt thiếu, chưa chia tài sản cho người hưởng hoặc đưa guardian vào quản lý. Cần làm rõ provenance của cạnh `ParentChild` trước khi để query người hưởng chuyển quyền sở hữu; các cạnh khởi tạo household hiện không chứng minh quan hệ con hợp pháp.

Plan: [quyết toán khoản vay từ tiền di sản](../design/M5-core-estate-cash-claims-plan.md).
