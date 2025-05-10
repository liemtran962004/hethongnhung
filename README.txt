/*
Thành viên:
Nguyễn Tấn Lợi - 22146347
Trần Chí Liêm  - 22146342
Nguyễn Trần Thiện Hải - 22146300
Nguyễn Đỗ Minh Khoa - 22146334
*/

bmp180_open(): Mở thiết bị BMP180 và in ra thông báo khi thiết bị được mở thành công
/* 
bmp180_release(): Đóng thiết bị BMP180 khi không sử dụng nữa và in ra thông báo khi thiết bị được đóng
*/

*/

bmp180_shutdown(int fd);
/* shutdow bmp18 khi đưa vào giá trị địa chỉ bmp180 */

bmp180_reset(int fd);
/* reset  bmp180 khi đưa vào giá trị địa chỉ  bmp180 */

 bmp180_read_register(int fd, uint8_t reg, uint8_t *value);
/*
Đọc giá trị thanh ghi của  bmp180
Khi đưa vào giá trị địa chỉ  bmp180 giá trị địa chỉ thanh ghi, con trỏ giá trị value trả về
*/

 bmp180_write_register(int fd, uint8_t reg, uint8_t value);
/*
Ghi giá trị thanh ghi value của  bmp180vào địa chỉ thanh ghi reg
Khi đưa vào giá trị địa chỉ  bmp180, giá trị địa chỉ thanh ghi, giá trị cần ghi
*/

 bmp180_read_fifo(int fd, uint32_t *irValue, uint32_t *redValue);
/*
Đọc giá trị thanh ghi FIFO của  bmp180 và trả về giá trị ir và hr thông qua con trỏ
Khi đưa vào giá trị địa chỉ  bmp180, con trỏ giá trị ir, con trỏ giá trị red trả về
*/

calculateHeartRate(uint32_t *irBuffer, uint8_t bufferLength);
/*
Tính toán giá trị nhịp tim khi đưa vào mảng ir, và kích thước mảng
Trả về giá trị kiểu float 
*/

calculateSPO2(uint32_t *irBuffer, uint32_t *redBuffer,uint8_t bufferLength);
*/

kbhit(void);
/* Kiểm tra phím đc nhật từ bàn phím */
