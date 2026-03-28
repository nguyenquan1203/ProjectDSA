## Báo cáo dự án (Project report)

---

### 1. Đặt vấn đề
Nén dữ liệu đóng vai trò sống còn trong việc tối ưu hóa không gian lưu trữ và băng thông mạng. Thách thức lớn nhất là làm sao cân bằng được giữa **tỷ lệ nén**, **thời gian chạy** và **lượng RAM tiêu thụ**. Hơn nữa, các thuật toán nén thường bộc lộ điểm yếu chí mạng khi xử lý các "tệp siêu nhỏ" (micro-files) – nơi mà phần dữ liệu đặc tả (header/metadata) còn nặng hơn cả nội dung gốc. Dự án này đánh giá toàn diện thuật toán DEFLATE trên nhiều tập dữ liệu để làm rõ sức mạnh, giới hạn vật lý và tính ứng dụng thực tiễn của nó.

---

### 2. Dữ liệu thực nghiệm
Hệ thống được kiểm thử khắt khe trên **237 tệp**, chia làm 3 nhóm chính:
* **Độ lặp cao (100 tệp):** Các file log hệ thống (OpenStack) và dữ liệu container.
* **Dữ liệu chuẩn (27 tệp):** Các bộ test kinh điển như Canterbury và Silesia (ví dụ: `alice29.txt`, `mozilla.tar`, `kennedy.xls`).
* **Kiểm thử trường hợp biên (110 tệp):** Các file mã nguồn cực nhỏ (`.cpp`, `.h`, `.py`) nhằm đánh giá hành vi thuật toán khi gặp dữ liệu quá ngắn.

---

### 3. Thuật toán cốt lõi & cấu trúc dữ liệu 

* **LZ77 qua Hash Table & Linked List:** 
  Thay vì quét tuyến tính toàn bộ cửa sổ trượt với độ phức tạp $O(N^2)$, chúng mình tự xây dựng hàm băm 3-byte (`hash3`). Hàm này ánh xạ các chuỗi 3 ký tự vào một **bảng băm gồm 65536 phần tử**. Để giải quyết đụng độ (hash collisions) và truy vết các chuỗi lặp lại trong quá khứ, chúng mình dùng một danh sách liên kết thông qua mảng `head` và `prev`. Kỹ thuật này kéo thời gian tìm kiếm chuỗi lặp xuống mức trung bình là $O(1)$.
* **Mã hóa Huffman qua hàng đợi ưu tiên:** 
  Sau khi qua LZ77, các token tiếp tục được mã hóa Huffman. Tần suất của từng token được đếm và đẩy vào một **hàng đợi ưu tiên (Min-Heap)**. Quá trình dựng cây mất thời gian $O(K \log K)$ (với $K$ là số lượng token duy nhất), giúp tạo ra các mã bit không tiền tố tối ưu nhất để ép nhỏ dung lượng file.
* **Cơ chế phòng ngự (Heuristic Flag 0/1):** 
  Hệ thống sẽ tính toán trước dung lượng đầu ra (`totalBits`). Nếu file sau khi nén (gồm nội dung thực tế + phần header chứa cây Huffman) to hơn cả file gốc, hệ thống sẽ tự động bật cờ **Flag 0 (Lưu thô)** để bảo toàn dung lượng gốc.
* **Xử lý thao tác bit:** 
  Thư viện I/O của C++ mặc định chỉ đọc/ghi theo từng byte. Để ghi được các mã Huffman có độ dài biến thiên (lẻ bit), chúng mình tự viết các lớp `InBitStream` và `OutBitStream` dùng phép toán thao tác bit (bitwise) để dồn và ghi từng bit một xuống tệp.

---

### 4. Kết quả thực nghiệm & phân tích chuyên sâu

Chỉ số đánh giá chính là **phần trăm dung lượng tiết kiệm được ($S$)**:

$$S = \left( 1 - \frac{\text{Dung lượng sau nén}}{\text{Dung lượng gốc}} \right) \times 100\%$$

#### Bảng tóm tắt hiệu năng

| Nhóm dữ liệu | % Tiết kiệm trung bình | Tốc độ xử lý | Đỉnh RAM sử dụng |
| :--- | :--- | :--- | :--- |
| **Độ lặp cao (Log)** | **75% - 82%** | ~10.5 MB/s | ~11 MB - 391 MB |
| **Văn bản & nhị phân (Chuẩn)** | **35% - 53%** | ~8.4 MB/s | ~15 MB - 636 MB |
| **File siêu nhỏ (Source code)** | **Âm (Bị phình to)** | < 1.0 MB/s | ~6 MB |

#### Ý nghĩa đằng sau các con số:
* **Sự hiệu trên dữ liệu log:** Tỷ lệ tiết kiệm hơn 80% trên các file log OpenStack minh chứng cho sức mạnh của bảng băm LZ77. File log chứa các mẫu chuỗi lặp đi lặp lại rất dễ đoán (thời gian, mã trạng thái). Cửa sổ trượt đã gom các đoạn lặp dài này cực kỳ hiệu quả, cho thấy DEFLATE là lựa chọn hoàn hảo để sao lưu máy chủ.
* **Tốc độ bất đối xứng:** Trên các bộ dữ liệu chuẩn, chúng mình nhận thấy **tốc độ giải nén luôn nhanh gấp 2 đến 3 lần tốc độ nén** (ví dụ: `webster` nén mất 6449 ms nhưng giải nén chỉ mất 2777 ms). Lý do là quá trình nén đòi hỏi các phép toán nặng như tra cứu bảng băm và sắp xếp Min-Heap, trong khi giải nén chỉ cần duyệt cây đơn giản và truy xuất mảng.
* **Giới hạn xử lí file siêu nhỏ:** Nếu không có cơ chế chặn bằng Flag 0, các file dưới 1KB sẽ bị nén âm (tới -600%). **Ý nghĩa sâu xa:** Để giải mã Huffman, cấu trúc cây tần số bắt buộc phải được gắn vào phần đầu (header) của file nén. Đối với các file quá nhỏ, chính cái header này lại có dung lượng vật lý lớn hơn cả dữ liệu gốc.
* **Nút thắt về RAM:** Dù file nhỏ tiêu tốn RAM rất ít, nhưng khi xử lý các tệp lớn như `mozilla.tar` (51 MB), hệ thống ngốn tới **636 MB RAM**. Điều này phơi bày một sự đánh đổi: để đạt tốc độ cao, chúng mình đang lưu toàn bộ mảng token LZ77 trên RAM trước khi chuyển sang bộ mã hóa Huffman, thay vì xử lý cuốn chiếu (streaming).

---

### 5. Thảo luận & tầm quan trọng 
Đồ án này không chỉ dừng lại ở việc triển khai thuật toán học thuật, mà còn minh chứng cho các tư duy cốt lõi trong thiết kế hệ thống thực tế:

* **Sự đánh đổi không gian - thời gian :** Động cơ nén của chúng mình chủ động hy sinh dung lượng RAM để đổi lấy tốc độ thực thi. Bằng cách cấp phát không gian $O(N)$ để lưu toàn bộ mảng và bảng băm trên RAM, chúng mình đạt được tốc độ truy xuất gần mức $O(1)$. Hiểu được sự đánh đổi này là mấu chốt để thiết kế các hệ thống chịu tải cao.
* **Tư duy thiết kế thực tiễn :** Một công cụ nén ngây ngô sẽ nhắm mắt nén mọi thứ nó nhận được. Bằng cách thiết kế cơ chế đánh giá `totalBits` (cờ Flag 0 lưu thô), hệ thống của chúng mình cho thấy sự "nhạy bén với môi trường thực tế" — nhận ra rằng việc biết *khi nào không nên nén* cũng quan trọng hệt như việc biết *cách nén*.
* **Độ tương thích với hạ tầng web:** Trong các môi trường web hiện đại (như CDN hay API), dữ liệu chỉ được server nén một lần nhưng lại được hàng triệu trình duyệt giải nén. Đặc tính bất đối xứng của hệ thống (nén thì nặng, giải nén thì siêu tốc) chính là lý do mô hình kết hợp LZ77+Huffman trở thành tiêu chuẩn công nghiệp cho giao thức HTTP.

---

### 6. Mở rộng
Để nâng cấp hệ thống chúng mình đề xuất các hướng mở rộng sau:

* **Độ phức tạp không gian $O(1)$ thông qua chunking:** Điểm nghẽn 636 MB RAM hiện tại có thể được giải quyết triệt để bằng cách xử lý luồng theo khối (block-level streaming). Bằng cách đọc, nén và dồn bit theo từng khối **64 KB** độc lập, mức tiêu thụ RAM của hệ thống sẽ trở thành hằng số $O(1)$. Điều này cho phép nén các file khổng lồ vài GB (video 4K, bản sao lưu database) ngay cả trên các thiết bị IoT yếu kém về bộ nhớ.
* **Tối ưu hóa cho API & IoT (Cây Huffman tĩnh):** Để giải quyết triệt để lỗi nén âm trên file siêu nhỏ, chúng mình có thể thiết kế thêm tùy chọn sử dụng cây Huffman tĩnh (được tính toán trước và fix cứng trong code). Việc này xóa bỏ hoàn toàn phần header dư thừa, biến thuật toán trở nên cực kỳ tối ưu khi nén các gói tin JSON bé xíu trong các REST API.
* **Xử lý đa luồng (Multi-threading):** Vì các khối nén 64 KB độc lập không phụ thuộc vào dữ liệu lịch sử của nhau, chúng ta hoàn toàn có thể phân chia công việc băm LZ77 và mã hóa Huffman cho nhiều nhân CPU chạy song song, giúp tăng tốc độ MB/s lên gấp nhiều lần trên các dòng máy tính đa nhân hiện đại.
