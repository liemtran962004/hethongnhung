#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/init.h>

#define DRIVER_NAME "bmp180_driver"
#define DEVICE_NAME "bmp180"
#define CLASS_NAME  "bmp180"

#define BMP180_I2C_ADDR      0x77

// Register addresses for BMP180
#define BMP180_REG_TEMP        0xF6
#define BMP180_REG_PRESSURE    0xF6
#define BMP180_REG_CONTROL     0xF4
#define BMP180_REG_CALIBRATION 0xAA

#define IOCTL_CONFIG           0x01
#define IOCTL_SHUTDOWN         0x02
#define IOCTL_RESET            0x03
#define IOCTL_READ_REGISTER    0x04
#define IOCTL_WRITE_REGISTER   0x05

// Khai báo các biến toàn cục
static int major_number;
static struct class *bmp180_class = NULL;
static struct device *bmp180_device = NULL;
static struct i2c_client *bmp180_client;

static int bmp180_write_register(struct i2c_client *client, u8 reg, u8 value) {
    uint8_t data[2] = {reg, value};
    if (i2c_master_send(client, data, 2) != 2) {
        dev_err(&client->dev, "Failed to write register\n");
        return -EIO;
    }
    return 0;
}

static int bmp180_read_register(struct i2c_client *client, u8 reg, u8 *value) {
    int ret = i2c_master_send(client, &reg, 1);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to send register address\n");
        return ret;
    }

    ret = i2c_master_recv(client, value, 1);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read register\n");
        return ret;
    }
    return 0;
}

static int bmp180_read_temperature(struct i2c_client *client) {
    u8 value;
    int ret = bmp180_read_register(client, BMP180_REG_TEMP, &value);
    if (ret < 0) {
        return ret;
    }
    return value;
}

static int bmp180_read_pressure(struct i2c_client *client) {
    u8 value;
    int ret = bmp180_read_register(client, BMP180_REG_PRESSURE, &value);
    if (ret < 0) {
        return ret;
    }
    return value;
}

static void bmp180_reset(struct i2c_client *client) {
    bmp180_write_register(client, BMP180_REG_CONTROL, 0xB6);
}

static void bmp180_shutdown(struct i2c_client *client) {
    bmp180_write_register(client, BMP180_REG_CONTROL, 0x34);
}

static void bmp180_configure(struct i2c_client *client) {
    // Cấu hình BMP180 để đo nhiệt độ và áp suất
    bmp180_write_register(client, BMP180_REG_CONTROL, 0x2E); // Start temperature measurement
    bmp180_write_register(client, BMP180_REG_CONTROL, 0x34); // Start pressure measurement
}

static int bmp180_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "BMP180 device opened\n");
    return 0;
}

static int bmp180_release(struct inode *inode, struct file *file) {
    bmp180_shutdown(bmp180_client);
    printk(KERN_INFO "BMP180 device closed\n");
    return 0;
}

static long bmp180_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    u8 reg;
    int ret;

    switch (cmd) {
        case IOCTL_CONFIG: // configure
            bmp180_configure(bmp180_client);
            printk(KERN_INFO "BMP180 configured\n");
            break;
        case IOCTL_SHUTDOWN: // shutdown
            bmp180_shutdown(bmp180_client);
            printk(KERN_INFO "BMP180 shutdown\n");
            break;
        case IOCTL_RESET: // reset
            bmp180_reset(bmp180_client);
            printk(KERN_INFO "BMP180 reset\n");
            break;
        case IOCTL_READ_REGISTER: // read register
            if (copy_from_user(&reg, (uint8_t *)arg, 1)) {
                return -EFAULT;
            }
            ret = bmp180_read_register(bmp180_client, reg, &reg);
            if (ret < 0) {
                return ret;
            }
            if (copy_to_user((uint8_t *)arg, &reg, 1)) {
                return -EFAULT;
            }
            break;
        case IOCTL_WRITE_REGISTER: // write register
            if (copy_from_user(&reg, (uint8_t *)arg, 1)) {
                return -EFAULT;
            }
            ret = bmp180_write_register(bmp180_client, reg, 0x00);
            if (ret < 0) {
                return ret;
            }
            printk(KERN_INFO "Register 0x%02x written with 0x00\n", reg);
            break;
        default:
            return -ENOTTY;
    }
    return 0;
}

static ssize_t bmp180_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    u8 data[6];
    int ret;

    // Đảm bảo bộ đệm truyền vào đủ lớn
    if (len < 6) {
        printk(KERN_ERR "Buffer too small\n");
        return -EINVAL;
    }

    ret = bmp180_read_temperature(bmp180_client);
    if (ret < 0) {
        return ret;
    }
    data[0] = ret; // Store temperature data

    ret = bmp180_read_pressure(bmp180_client);
    if (ret < 0) {
        return ret;
    }
    data[1] = ret; // Store pressure data

    if (copy_to_user(buf, data, 6)) {
        return -EFAULT;
    }

    return 6;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = bmp180_open,
    .release = bmp180_release,
    .unlocked_ioctl = bmp180_ioctl,
    .read = bmp180_read,
};

static int bmp180_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    bmp180_client = client;

    major_number = register_chrdev(0, DEVICE_NAME, &fops);  // đăng ký major number
    if (major_number < 0) {
        printk(KERN_ALERT "BMP180 failed to register a major number\n");
        return major_number;
    }

    bmp180_class = class_create(THIS_MODULE, CLASS_NAME); // Đăng ký lớp thiết bị
    if (IS_ERR(bmp180_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);   // unregister
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(bmp180_class);
    }

    bmp180_device = device_create(bmp180_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(bmp180_device)) {
        class_destroy(bmp180_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(bmp180_device);
    }

    printk(KERN_INFO "BMP180: device class registered correctly\n");
    return 0;
}

static void bmp180_remove(struct i2c_client *client) {
    device_destroy(bmp180_class, MKDEV(major_number, 0));
    class_unregister(bmp180_class);
    class_destroy(bmp180_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "BMP180: Goodbye from the LKM!\n");
}

static const struct i2c_device_id bmp180_id[] = {
    { "bmp180", 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, bmp180_id);

static struct i2c_driver bmp180_driver = {
    .driver = {
        .name = DRIVER_NAME,
    },
    .probe = bmp180_probe,
    .remove = bmp180_remove,
    .id_table = bmp180_id,
};

static int __init bmp180_init(void) {
    printk(KERN_INFO "Initializing BMP180 driver\n");
    return i2c_add_driver(&bmp180_driver);
}

static void __exit bmp180_exit(void) {
    printk(KERN_INFO "Exiting BMP180 driver\n");
    i2c_del_driver(&bmp180_driver);
}

module_init(bmp180_init);
module_exit(bmp180_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A driver for the BMP180 sensor");
MODULE_VERSION("1.0");
