# M4 — Đường đèo & nguồn sống

Ngày kiểm chứng: 2026-10-15. **Đã triển khai lát cắt và kiểm chứng Windows local. Linux của thay đổi này chưa chạy; không suy từ Windows sang Linux.** Đây chưa phải campaign hoàn chỉnh hoặc model production.

## Phạm vi đã làm

- Bản đồ native lấy polygon, địa hình và topology từ C++: sáu lãnh địa + đất triều đình, 10.000 Person ID. Pan, zoom tại con trỏ, picking, lớp dự trữ/hồ sơ và tuyến dự kiến dùng cùng geography với vận tải. Năng suất địa hình và thời tiết theo seed/lưu vực ảnh hưởng thu hoạch.
- Người chơi có thể xử lý thiếu hụt/nghĩa vụ Hắc Sơn bằng nhiều quyết định: thỏa thuận quyền đèo, hoặc điều tra có thời hạn rồi phân xử với chi phí bất mãn; xây đường, cấp hàng từ kho hữu hạn và trả nợ đúng chủ nợ.
- Đường có thời gian/công suất; đặt chỗ theo từng chặng dùng chung hai chiều. Chia lô không nhân công suất. Hàng đang đi thuộc manifest được đối soát cùng hàng trong Realm; giao hàng không bị gọi là sản xuất mới.
- Xưởng dệt Bạch Khâu nhận tối đa sáu lao động trưởng thành, dùng công cụ/vốn/xơ/nhiên liệu; trả lương và bán vải cho thị trường có tiền. Thiếu đầu vào/vốn thì không sản xuất.
- Hồ sơ mở/đóng từ kho, nợ quá hạn hoặc công trình vướng quyền; báo cáo/sổ việc trễ hai ngày, có ID và liên kết căn cứ. Không có lịch truyện bắt buộc ép người chơi đi theo chuỗi nhiệm vụ.
- Save `POLYEC_M4_1` giữ Realm, RNG, sequence/revision, quyền, công trình, lịch hàng/đặt chỗ, lao động và hồ sơ. Checksum/schema/count/invariant được kiểm tra trước khi thay live world. Lưu đè giữ `.bak`, thay file bằng thao tác thay thế của OS; tải luôn tạm dừng.

Điều kiện đầu kỳ là fixture M4 có nhãn: kho/nguồn sống khác nhau, quyền thu phí đèo có sẵn, khoản vay 200 đ + 8 đ lãi có nguồn tiền và ngân khố hoàng gia 60.000 đ được ghi trong opening ledger. Đây không phải quyết định giảm scope sản phẩm hoặc bộ luật xã hội cuối cùng.

## Kiến trúc và controller

`Campaign` sở hữu `Realm` và các record logistics/case của M4 trong C++20. Luật tiền, hàng, nhu cầu và sản xuất dùng cùng core; công thức thu hoạch chỉ có một hàm trong Realm. Godot nhận snapshot tách rời và gửi command có sequence/revision; lệnh bị từ chối không thay một phần world. Snapshot thường lệ không serialize/hash toàn dân; fingerprint đầy đủ là lệnh chẩn đoán riêng.

**Việc thường lệ mới dùng controller kiểm thử có nhãn**, hiện chọn Work cho cư dân đủ tuổi theo cadence 30 ngày. Controller gọi cùng validator/command core; đây không phải learned strategy. Xưởng là quy trình có nguồn lực và lao động hữu hạn. M4 không nạp model realm-v6 để quyết định các action mới, không gọi heuristic là RL đã hoàn thành. Sandbox/policy M3 vẫn giữ source và bundle riêng; R1–R3 chưa được tác vụ này giải quyết. Production training vẫn ở M7 sau feature freeze.

M4 tập trung đóng được một vấn đề, không làm mọi vùng ổn định cùng lúc. Ảnh hoàn tất cho thấy Hắc Sơn được cứu trong khi nơi khác vẫn thiếu dự trữ: can thiệp tiêu dùng nguồn lực có thật, không tự cân bằng toàn xã hội. Household động, quyền đất đầy đủ, hôn nhân/thừa kế/gia tộc và luật giáo hội chưa được triển khai; chưa tự duyệt các đề xuất của cuộc thảo luận trước.

## Kiểm tra thực sự đã chạy

Toolchain: MSVC 19.51.36260, Visual Studio 18 2026, CMake đi kèm VS; Godot **4.5.1 stable**; godot-cpp **e83fd0904c13356ed1d4c3d09f8bb9132bdc6b77**. ZIP Godot Windows SHA256 khớp metadata release: `defccc78669e644861b4247626b01ae362cd9f23975edf19c8bfd2eb1f6a1783`.

| Kiểm tra | Kết quả Windows |
|---|---|
| CTest Release | 5/5: core 70, trade 36.546, realm 25.203, court 168, campaign 202 checks |
| Godot native / UI cũ | 19/19 và 18/18 |
| Godot M4 | 39/39: map/pan/zoom/layers, hai cách xử lý quyền, command reject/stale, pause/focus/input số, save/restore và đóng hồ sơ |
| Shared-core Python | Build binding Python 3.12/pybind11 3.1.0; 5/5 kiểm tra invariant, advance, serialize/restore và fingerprint của Realm |
| C++ workload M4 | 6 seed × 720 ngày ở 256 người giữ invariant; workload 10.000 người/102 ngày giữ invariant và đóng hồ sơ Hắc Sơn |
| Render | OpenGL Compatibility trên Intel UHD; đã xem ảnh 1440×900 và cửa sổ 1100×720 |
| Portable ZIP giải nén vào đường dẫn có dấu/khoảng trắng | Native 19/19, UI 18/18, M4 39/39; launcher exit 0 |

Ảnh: [hoàn tất một hồ sơ](../../artifacts/visuals/m4/Windows-completed.png), [cửa sổ nhỏ](../../artifacts/visuals/m4/Windows-1100x720.png).

[Output kiểm tra được lưu](../../artifacts/visuals/m4/Windows-checks.txt). Gói đã thử tại `dist/Đóng gói thử M4/PolyEc-M4-Windows`; binary/resource tương ứng trong `dist/PolyEc-M4-Windows.zip`. Chưa kiểm tra cài trên máy Windows sạch hoặc không có VC runtime.

OpenCode Bun 1.18.30 / `opencode/step-5-preview-free` đã đọc code và trả review trong phiên `ses_ec2ad0720ffeW4kmlv0IVEuLiq`. Đã xử lý góp ý về công thức thu hoạch dùng chung, nhãn focus/pause, lý do từ chối và ngữ nghĩa/giới hạn chỉ số chủ nợ. Nhận xét rằng slot cũ tự chặn vô hạn không đúng với so sánh khoảng ngày; đã thêm test chứng minh slot hết hạn không làm trễ lô sau và tránh quét hàng đã giao khi preview. Review model không thay cho các kết quả chạy ở trên.

## Profile tương tác

Máy: Windows 11 Home Single Language 10.0.26200, Intel Core i5-12450H, Intel UHD Graphics, RAM vật lý khoảng 15,7 GiB; viewport 1440×900. Mẫu 270 frame sau warmup, 30 ngày có lô 1.000 lương thực đang đi, đường mắc quyền, xưởng sáu thợ, payroll tháng, click chọn vùng, save/restore. **Không có ONNX inference trong phép đo này.**

| Chỉ tiêu | Số đo |
|---|---|
| Frame p50 / p95 / p99 | 16,730 / 28,504 / 38,056 ms |
| Một ngày + dựng snapshot/UI p95 | 4,910 ms |
| Click tới inspector ở frame kế p95 | 47,446 ms |
| Lưu / tải khi tạm dừng | 202,368 / 199,581 ms |
| File save | 1.471.158 byte |
| Working set cao nhất quan sát mỗi 100 ms | 737.193.984 byte, khoảng 703 MiB |

[JSON gốc](../../artifacts/visuals/m4/Windows-profile.json). Median gần 60 FPS; không tuyên bố khóa 60 FPS mọi frame (p99 còn spike). Đây là profile A/M4, chưa chứng minh B/100k hoặc C/1m, chưa đo workflow ONNX/production và chưa phải kiểm định hiệu năng dài hạn. Sampling working set không bảo đảm bắt mọi peak ngắn hơn 100 ms.

Hai tối ưu được đo: bỏ fingerprint toàn save khỏi snapshot click thường lệ; dừng render/cập nhật địa đồ cũ bị che bởi màn M4. Không giảm dân số, cadence hoặc bỏ kiểm tra transaction để đạt số đo.

## Chạy lại trên Windows

Trong shell có CMake của Visual Studio:

```powershell
git clone --depth 1 --branch godot-4.5-stable https://github.com/godotengine/godot-cpp third_party/godot-cpp
cmake -S . -B build/windows -G "Visual Studio 18 2026" -A x64 -DPOLYEC_BUILD_APP=ON -DPOLYEC_BUILD_AI=OFF
cmake --build build/windows --config Release --parallel 4
.\scripts\test-native.ps1 -Godot <Godot-console.exe> -BuildDir build/windows
.\scripts\package-windows.ps1 -Godot <Godot-console.exe> -BuildDir build/windows -OutputDir dist/PolyEc-M4-Windows -IncludeM4
```

Nếu dependency đã có thì giữ checkout đã pin; không chạy clone đè. M4 controller build không cần ONNX Runtime; muốn kiểm tra lại sandbox RL cần build AI cùng ORT và chạy các smoke tương ứng, chưa thực hiện trong tác vụ này. Package/player guide nằm ở `dist/PolyEc-M4-Windows` và [PLAYER-README-M4.txt](../guides/PLAYER-README-M4.txt). CI Linux đã thêm campaign smoke nhưng chưa được dispatch/push từ tác vụ local.

## Điểm tiếp tục

Trước M5, chốt nền xã hội từ năm ví dụ có sổ, rồi mở luật/world theo plan. Giữ quality gates RL chưa đạt trong WORK_STATUS; không biến fixture M4 hoặc controller test thành bằng chứng xã hội/RL production. Chưa commit/push/publish trong tác vụ này.
