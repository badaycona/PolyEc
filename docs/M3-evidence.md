# PolyEc M3 v6 — bằng chứng đang hoàn tất

Bản native local đã build và kiểm tra: CTest5/5 (core70, trade36546,
realm24980, court168, runtime37); Python replay258 và training preflight476;
Godot native19, UI18, merchant336, realm411; ONNX parity489.

Ba họ PPO v6 đã học xong với3seeds/họ, tổng2.625.536 quyết định. Checkpoint
chỉ chọn bằng validation; giữ lịch sử/initial/config/selection/source hashes.
Audit categorical100world mới đã hoàn tất. Governance vẫn thua baseline;
economic cải thiện so untrained nhưng thua baseline; civilian CI so baseline
còn rộng. **Full R1/R2/R3 chưa được chứng nhận.**

Tám world20năm đang chạy; hai world đầu còn11.516 và11.525 người sống,
hạch toán đúng. V3 trước đó cả8world kết thúc0người sống. Không gán toàn bộ
thay đổi do sửa luật cho công trạng học RL. Báo cáo đầy đủ sẽ ghi đủ8kết quả,
confidence intervals, survival, profile và các hạn chế.

Có12typed augment primitives: quỹ tín dụng/matching hạn chế, escrow3bên và
quyền từ chối, applicant/bổ nhiệm có lương/từ chức, engineer, course có giáo
viên/người học thực, reports/điều tra với độ trễ/capacity. UI cho xem điều khoản
đề nghị; native chặn commit điều khoản khác snapshot quan sát.

Đây là sandbox presentable để kiểm tra cơ chế, chưa phải campaign hoàn chỉnh.
Protocol: realm-rl-experiment-v6.md; số liệu máy đọc được: ai/models/realm-v6.
Ảnh native1440x900 và1100x720 đã được xem để kiểm tra layout. Windows portable
đã qua native19/UI18/merchant336/realm411; bản ZIP cuối sẽ kèm báo cáo đầy đủ.
