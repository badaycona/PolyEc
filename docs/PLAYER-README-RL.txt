POLYEC — PHÒNG THỬ NGHIỆM RL KINH TẾ
Windows x64 · Chạy offline · CPU inference

MỞ APP
Giải nén toàn bộ thư mục và mở PolyEc.exe.
Không cần cài Godot, Python, PyTorch, Node hoặc API key.
Giữ models/ và bin/ cạnh executable; model và ONNX Runtime đã được bundle.

XEM POLICY ĐÃ HỌC
1. Tại bản đồ, bấm “RL · Phòng thử nghiệm”.
2. Bấm “Một quyết định”: policy chọn mua/bán/di chuyển/chờ;
   C++ kiểm tra và thực hiện giao dịch, tiến bảy ngày mô phỏng.
3. Bấm “Chạy phiên” để xem chuỗi64 quyết định. Space chạy/dừng.
4. Xem giá sáu chợ, hàng trên xe, tiền mặt và nhật ký action/hệ quả.
5. “Phiên mới” đổi seed thế giới; Esc về địa đồ.
Rời cửa sổ sẽ dừng chạy tự động. Về địa đồ không tự resume đồng hồ.

BẰNG CHỨNG
Model là actor GRU64 được huấn luyện bằng PPO trên C++ core chung.
Không dùng imitation warm start hoặc heuristic để chọn thay policy trong app.
Checkpoint được chọn bằng validation; benchmark test tách riêng.
Xem models/evaluation.json và models/native-parity.json để đọc kết quả thật,
kể cả các baseline và các gate chưa đạt. Mất/hỏng model sẽ dừng RL, không fallback.

PHẠM VI THỬ NGHIỆM
Một learned merchant, sáu vùng, bảy hàng, tài khoản và stock thật.
Firm/household dùng hợp đồng cố định làm luật curriculum; chưa phải NPC RL.
Đây chưa phải M1/M2/M3 hoàn chỉnh, chưa có RL quản trị/cư dân/trẻ em,
chiến dịch, save, stat/augment hay một triệu NPC có agency.
Bản đồ hai vùng M0a vẫn có sẵn, tách khỏi phiên sandbox RL sáu thị trường.
Chỉ Windows đã được build/test. Linux chưa xác minh.

Công nghệ: Godot4.5.1, C++20, ONNX Runtime CPU1.31.0.
Giấy phép engine/runtime/font trong licenses/.
