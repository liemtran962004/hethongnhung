#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#define DEVICE_PATH "/dev/bmp180"

// Các lệnh ioctl
#define IOCTL_CONFIG        0x01
#define IOCTL_SHUTDOWN      0x02
#define IOCTL_RESET         0x03
#define IOCTL_READ_REGISTER 0x04
#define IOCTL_WRITE_REGISTER 0x05

// Hàm để đọc dữ liệu từ cảm biến
void read_sensor_data(int fd) {
    unsigned char buffer[6];
    ssize_t len;

    len = read(fd, buffer, sizeof(buffer));
    if (len < 0) {
        perror("Failed to read data from sensor");
        return;
    }

    printf("Temperature: %d\n", buffer[0]);
    printf("Pressure: %d\n", buffer[1]);
}

// Hàm để cấu hình cảm biến
void configure_sensor(int fd) {
    if (ioctl(fd, IOCTL_CONFIG, 0) < 0) {
        perror("Failed to configure sensor");
    } else {
        printf("Sensor configured successfully.\n");
    }
}

// Hàm để reset cảm biến
void reset_sensor(int fd) {
    if (ioctl(fd, IOCTL_RESET, 0) < 0) {
        perror("Failed to reset sensor");
    } else {
        printf("Sensor reset successfully.\n");
    }
}

// Hàm để tắt cảm biến
void shutdown_sensor(int fd) {
    if (ioctl(fd, IOCTL_SHUTDOWN, 0) < 0) {
        perror("Failed to shutdown sensor");
    } else {
        printf("Sensor shutdown successfully.\n");
    }
}

// Hàm đọc và ghi thanh ghi cảm biến
void read_write_register(int fd) {
    unsigned char reg, value;

    // Đọc thanh ghi
    reg = 0xF6; // Ví dụ: Đọc giá trị từ thanh ghi 0xF6
    if (ioctl(fd, IOCTL_READ_REGISTER, &reg) < 0) {
        perror("Failed to read register");
        return;
    }
    printf("Read value from register 0x%02x: 0x%02x\n", reg, value);

    // Ghi thanh ghi
    reg = 0xF4; // Ví dụ: Ghi giá trị vào thanh ghi 0xF4
    value = 0x34; // Dữ liệu muốn ghi
    if (ioctl(fd, IOCTL_WRITE_REGISTER, &reg, &value) < 0) {
        perror("Failed to write register");
        return;
    }
    printf("Wrote value 0x%02x to register 0x%02x\n", value, reg);
}

int main() {
    int fd;

    // Mở thiết bị BMP180
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    // Cấu hình cảm biến
    configure_sensor(fd);

    // Đọc dữ liệu từ cảm biến
    read_sensor_data(fd);

    // Reset cảm biến
    reset_sensor(fd);

    // Tắt cảm biến
    shutdown_sensor(fd);

    // Đọc và ghi thanh ghi
    read_write_register(fd);

    // Đóng thiết bị
    close(fd);

    return 0;
}
