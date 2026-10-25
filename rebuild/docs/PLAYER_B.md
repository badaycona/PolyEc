# PolyEc trial-b1 — hai lãnh thổ

Mở **PolyEc.exe**, giữ game/ và runtime/ cạnh launcher. Native Godot4.5 dùng cùng C++20 core, chạy offline; không cần IDE/Python để chơi. Đây là script kiểm luật, chưa có learned policy.

Bấm **Hai lãnh thổ**, rồi **Bước kịch bản →** từng lần.46 bước minh họa: Lưu mua3 food tại Thanh Điền, mang hàng qua tuyến120 phút/phí2/tải12, tự dừng ở camp để ăn/uống/nghỉ và lưu; đến Tùng Lâm bán hàng, tự nhận thuê trọ, mua gỗ rồi quay về. Vua đóng đường giữa chuyến khiến hoạt động dừng tại tọa độ thật; mở lại vẫn phải tự chọn tiếp tục. Kết thúc người buôn có64 đồng từ50, quỹ100=110/quỹ200=66; đây là kết quả giao dịch của fixture, không phải tiền thưởng AI.

**Vòng cơ bản** chạy chuỗi sinh hoạt/lao động cũ trong cùng world hai vùng. Muốn tự thử: bật DEBUG, chọn người6, chọn Khởi hành tại chợ13 hoặc23; các lệnh Dừng/Tiếp tục/Quay về/Mở đóng tuyến gửi command thật. Chọn người/site/tuyến trên bản đồ để xem. Chỉ controller1 được mở/đóng; debug không bỏ quyền. Direct move chỉ trong vùng.

Tải: Food/Seed/Water1,Wood2,Tool3; hành lý tối đa12 trên tuyến. Chuẩn bị đồ ăn/nước là lựa chọn của người đi, engine không mua/ăn/thuê thay. Pause/cancel khi đi giữ Journey, hàng, mục tiêu và phí đã trả. Quay về phải đi đủ quãng ngược; không teleport/refund. Hàng chào bán đã ký gửi ở site không đi theo chủ; hết hạn khi chủ đi xa, dùng Nhận lại hàng ký gửi sau khi quay về đúng site.

Trọ: chủ8 ở site25 chào thuê, khách tự accept/rent; expiry không xóa người. Thuế bán hàng về quỹ vùng nơi bán. Grant chọn đúng organization100/200 và đúng Office holder. Player view chỉ thấy dữ liệu đã biết/hiện diện; DEBUG toàn tri và điều khiển NPC được ghi nhãn.

**Chạy** tiến10 phút mỗi giây; **+10 phút** tiến một bước. Mất focus hoặc gián đoạn dài sẽ pause, không chạy bù. Lưu version3 ở `%APPDATA%/PolyEcRebuild/live-b1.pe`, `checkpoint-b1.pe`; không ghi đè save-a2. Không chạy hai cửa sổ ghi cùng session.

Luật: `docs/TRAVEL_B.md` trong docs/. Bản đồ class/action: docs/CLASS_ACTION_MAP.md. Bằng chứng: docs/REPORT_B.md, evidence/. Source chính tại C:\prc\PolyEc\rebuild; GitHub https://github.com/badaycona/PolyEc giữ lịch sử, main đã thay bằng rewrite. Legacy local vẫn trên đĩa, không thuộc build/tree GitHub.

Build từ repo: `cmake -S . -B build`, `cmake --build build --config Release --target core_tests action_tests journey_tests polyec_core polyec_launcher`, `ctest --test-dir build -C Release --output-on-failure`. Nếu mở sourceZIP độc lập, CMakeLists.txt trong ZIP cũng build được core; đặt binary polyec_core.exe vào game/ rồi chạy Godot với --path game. Các scripts Windows ghi đường toolchain local, còn CMake/CI là đường build portable.

Chưa huấn luyện RL, chưa chiến tranh/27 vùng, chưa cân bằng kinh tế lâu dài. Runtime Godot được tải mới ở session rebuild, license/copyright tại runtime/.
