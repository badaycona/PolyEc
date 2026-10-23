# Realm v7 inference plumbing smoke — 2026-10-21

## Kết quả

Chạy model `Policy` v7 với trọng số khởi tạo ngẫu nhiên, export sang ONNX trong `build/`, rồi so sánh kết quả PyTorch, Python ONNX Runtime và C++ `RealmNativePolicy`. Đây chỉ là kiểm tra đường inference và batch động; **không phải learned RL, không đánh giá chất lượng policy, không nghiệm thu model hay M5**. Không tạo manifest và không chép model vào `godot_project/models/realm-v7`.

| Batch | Bước mỗi batch | Sai số logits lớn nhất | Sai số memory lớn nhất |
|---:|---:|---:|---:|
| 1 | 8 | 2,79e-9 | 1,79e-7 |
| 3 | 8 | 3,73e-9 | 1,79e-7 |
| 32 | 8 | 4,66e-9 | 1,79e-7 |
| 64 | 8 | 4,19e-9 | 1,79e-7 |

Các đầu ra khớp với `atol=1e-5`, `rtol=1e-3`. Runtime: Python ONNX Runtime 1.31.0; C++ binding dùng ONNX Runtime của build hiện tại. Smoke lấy observation từ `RealmBatch`, giữ action trong mask và truyền recurrent memory qua 8 bước. Exporter phát cảnh báo upstream thông thường về GRU batch động; chạy thực tế ở cả bốn batch trên không tái hiện lỗi shape.

## Cách tái lập và giới hạn

Script chỉ phục vụ lần kiểm tra này nằm trong `build/realm-v7-plumbing-smoke.py`; ONNX tạm nằm trong `build/realm-v7-plumbing-random.onnx`; gói Python cô lập nằm trong `build/validation-python-deps`. Không sửa dependency khai báo của dự án. Mô hình được khởi tạo mới mỗi lần nên sai số cụ thể phụ thuộc seed khởi tạo, nhưng chỉ tiêu pass là parity số học giữa ba runtime.

Để nghiệm thu learned policy vẫn cần ba checkpoint v7 thật, manifest tương thích, parity script chính thức, sampled evaluator, holdout đa seed và các gate R1. Chưa chạy train dài hoặc profile B trong smoke này.
