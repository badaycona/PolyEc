# Báo cáo: liên kết đặc sản atlas với catalog hàng

## Thay đổi

Atlas giữ tên đặc sản dễ đọc cho người chơi và nay có thêm bảng liên kết có cấu trúc tới nhóm hoặc mã sản phẩm mà kế hoạch đã chỉ định. Bảy ánh xạ được lưu như sau:

| ID và vùng | Tên đặc sản trong atlas | Nhóm hàng | Mã sản phẩm cụ thể |
|---|---|---|---|
| L05 — Bạch Khâu | Len trắng | Xơ sợi (`Fiber`) | Chưa có mã riêng cho len trong catalog 27 sản phẩm |
| L11 — Bạch Sa | Thủy tinh | Hàng xa xỉ (`Luxury`) | Chưa chỉ định |
| L12 — Trầm Hải | Trầm hương | Hàng xa xỉ (`Luxury`) | `Incense` |
| L13 — Vân Lụa | Lụa nhuộm | Vải (`Cloth`) | `Silk` |
| L15 — Hồng Diệp | Trà | Hàng xa xỉ (`Luxury`) | `Tea` |
| L16 — Bích Lò | Đồ sứ | Hàng xa xỉ (`Luxury`) | Chưa chỉ định |
| L22 — Lâm Mật | Sáp ong | Xơ sợi (`Fiber`) | `Beeswax` |

“Len” chỉ trỏ tới nhóm xơ sợi vì catalog hiện tại không có `GoodId` tên len. Thủy tinh và đồ sứ trong bảng atlas không ghi rõ là hàng trang trí hoặc đồ sứ tốt, nên chỉ được gán nhóm xa xỉ, không gán nhầm `DecorativeGlass` hay `FinePorcelain`. Bốn mặt hàng còn lại có mã cụ thể theo tên biến thể trong catalog (`Silk`, `Tea`, `Incense`, `Beeswax`); kiểm tra tự động xác nhận nhóm của mã sản phẩm khớp nhóm đã ghi cho vùng. Lookup theo ID vùng trả về đúng dòng hoặc giá trị rỗng nếu không có ánh xạ. Khu triều đình không nhận ánh xạ đặc sản.

## Kiểm tra đã chạy

Windows Release target `polyec_world_atlas_tests` được build và runner chạy trực tiếp. Runner xác nhận toàn bộ bảy dòng ánh xạ gắn đúng ID, tên vùng và tên đặc sản; nhóm/mã sản phẩm; trường hợp len không có mã hàng riêng; lookup vùng không được ánh xạ; tính duy nhất của ID vùng; và tính hợp lệ của enum nhóm/sản phẩm. Các fixture lỗi bị từ chối gồm trùng vùng, gán vùng triều đình, mã sản phẩm ngoài catalog và nhóm hàng khác với nhóm của sản phẩm. Cùng target cũng kiểm tra đủ 27 vùng và 31 liên kết atlas hiện có. Không chạy toàn bộ CTest, Debug hoặc adapter vì thay đổi chỉ thêm dữ liệu tra cứu C++ cho atlas.

## Ảnh hưởng và giới hạn

Atlas nay có thể dùng nhóm hàng hoặc mã sản phẩm chung để tham chiếu bảy đặc sản đã được kế hoạch phân loại. Việc này chưa tạo kho, nguồn cung, xưởng, giá vùng, độc quyền sản xuất hoặc chuyển hàng; thị trường và Realm không thay đổi. Mười bảy đặc sản còn lại không được ánh xạ trong bảng mới vì §15.2 không chỉ rõ mapping của chúng. Đây không phải bảng mapping đầy đủ cho mọi sản phẩm của quốc gia.

## Tài liệu đối chiếu

- Quy tắc nhóm/biến thể: `docs/IMPLEMENTATION_PLAN.md` §15.2.
- Định danh sản phẩm hiện hành: `core/include/polyec/good_catalog.hpp`.
- Phạm vi triển khai và điều không được suy diễn: [kế hoạch](../design/M5-atlas-specialty-good-mapping-plan.md).
- Mã băm và kích thước source, test, report, executable và tracker: [manifest](../../artifacts/m5-atlas-specialty-good-mapping-2026-10-22-manifest.json).
