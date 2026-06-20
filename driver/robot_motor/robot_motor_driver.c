#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>

#define MODULE_NAME "robot_motor"
#define CLASS_NAME  "robot_chassis_class"

/* 模拟的机器人电机硬件数据结构 */
struct robot_motor_dev {
    dev_t dev_num;              /* 动态分配的设备号 */
    struct cdev cdev;           /* 字符设备核心结构 */
    struct class *class;        /* 用于自动创建 /dev 节点的类 */
    struct device *device;      /* 设备结构体 */
    struct mutex lock;          /* 互斥锁，防止多进程并发冲突 */
    int current_rpm;            /* 模拟的电机当前转速 */
};

static struct robot_motor_dev motor_dev;

/* =========================================================================
 * VFS 文件操作接口实现
 * ========================================================================= */

static int robot_motor_open(struct inode *inode, struct file *file) {
    /* 尝试加锁，若已被霸占则返回忙错误，防止多个节点同时控制同一个电机 */
    if (!mutex_trylock(&motor_dev.lock)) {
        pr_warn("%s: Device is busy!\n", MODULE_NAME);
        return -EBUSY;
    }
    pr_info("%s: Motor device opened successfully\n", MODULE_NAME);
    return 0;
}

static int robot_motor_release(struct inode *inode, struct file *file) {
    mutex_unlock(&motor_dev.lock); /* 释放硬件互斥锁 */
    pr_info("%s: Motor device closed\n", MODULE_NAME);
    return 0;
}

/* 读接口：将内核中的电机转速数据安全拷贝到用户态 */
static ssize_t robot_motor_read(struct file *file, char __user *buf, size_t count, loff_t *offset) {
    char k_buf[32];
    int len;

    /* 模拟从底盘硬件寄存器读取最新的转速 */
    motor_dev.current_rpm = 3000; 
    len = snprintf(k_buf, sizeof(k_buf), "RPM:%d\n", motor_dev.current_rpm);

    if (*offset >= len) return 0; /* 已经读完 */
    if (count < len) return -EINVAL;

    /* 💥 核心安全防线：使用 copy_to_user 防止缺页中断引起的内核崩溃 */
    if (copy_to_user(buf, k_buf, len)) {
        return -EFAULT;
    }

    *offset += len;
    return len;
}

/* 写接口：接收应用层下发的控制指令（如设置目标转速） */
static ssize_t robot_motor_write(struct file *file, const char __user *buf, size_t count, loff_t *offset) {
    char k_buf[32];

    if (count >= sizeof(k_buf)) return -EINVAL;

    /* 安全接收用户态下发的数据 */
    if (copy_from_user(k_buf, buf, count)) {
        return -EFAULT;
    }
    k_buf[count] = '\0';

    /* 解析指令并模拟写入硬件寄存器 */
    pr_info("%s: Received control command: %s", MODULE_NAME, k_buf);
    return count;
}

/* 绑定文件操作方法集 */
static const struct file_operations motor_fops = {
    .owner   = THIS_MODULE,
    .open    = robot_motor_open,
    .read    = robot_motor_read,
    .write   = robot_motor_write,
    .release = robot_motor_release,
};

/* =========================================================================
 * 现代 Linux 标准：Platform 平台驱动总线匹配
 * ========================================================================= */

static int robot_motor_probe(struct platform_device *pdev) {
    int ret;
    pr_info("%s: Device matched! Commencing probe initialization...\n", MODULE_NAME);

    mutex_init(&motor_dev.lock);

    /* 1. 动态申请设备号（避开系统已有硬件的设备号冲突） */
    ret = alloc_chrdev_region(&motor_dev.dev_num, 0, 1, MODULE_NAME);
    if (ret < 0) {
        pr_err("%s: Failed to allocate char device region\n", MODULE_NAME);
        return ret;
    }

    /* 2. 初始化 cdev 并绑定 fops */
    cdev_init(&motor_dev.cdev, &motor_fops);
    motor_dev.cdev.owner = THIS_MODULE;

    /* 3. 将字符设备注册进内核 */
    ret = cdev_add(&motor_dev.cdev, motor_dev.dev_num, 1);
    if (ret < 0) {
        goto unregister_region;
    }

    /* 4. 创建系统类（/sys/class/robot_chassis_class） */
    motor_dev.class = class_create(CLASS_NAME);
    if (IS_ERR(motor_dev.class)) {
        ret = PTR_ERR(motor_dev.class);
        goto delete_cdev;
    }

    /* 5. 自动创建 /dev/robot_motor 设备节点（无需手动 mknod） */
    motor_dev.device = device_create(motor_dev.class, NULL, motor_dev.dev_num, NULL, MODULE_NAME);
    if (IS_ERR(motor_dev.device)) {
        ret = PTR_ERR(motor_dev.device);
        goto destroy_class;
    }

    pr_info("%s: Driver initialized successfully. /dev/%s created.\n", MODULE_NAME, MODULE_NAME);
    return 0;

destroy_class:
    class_destroy(motor_dev.class);
delete_cdev:
    cdev_del(&motor_dev.cdev);
unregister_region:
    unregister_chrdev_region(motor_dev.dev_num, 1);
    return ret;
}

/* 🛠️ MODIFIED: Return type changed from 'int' to 'void' for modern Linux Kernel API (>= 6.11) */
static void robot_motor_remove(struct platform_device *pdev) {
    /* 严格按照逆序释放资源，防止内核残留僵尸元数据 */
    device_destroy(motor_dev.class, motor_dev.dev_num);
    class_destroy(motor_dev.class);
    cdev_del(&motor_dev.cdev);
    unregister_chrdev_region(motor_dev.dev_num, 1);
    mutex_destroy(&motor_dev.lock);
    
    pr_info("%s: Driver removed cleanly\n", MODULE_NAME);
    /* 🛠️ MODIFIED: Removed 'return 0;' */
}

/* 匹配设备树的 compatible 标签 */
static const struct of_device_id robot_motor_of_match[] = {
    { .compatible = "rayyuan,robot-motor-1.0" },
    { /* 哨兵节点，不可删除 */ }
};
MODULE_DEVICE_TABLE(of, robot_motor_of_match);

static struct platform_driver robot_motor_driver = {
    .probe  = robot_motor_probe,
    .remove = robot_motor_remove,
    .driver = {
        .name           = MODULE_NAME, /* "robot_motor" */
        .of_match_table = robot_motor_of_match,
    },
};

/* Global pointer to hold our manually created device */
static struct platform_device *robot_motor_pdev;

static int __init robot_motor_init(void)
{
    int ret;

    /* 1. Register the platform driver with the kernel */
    ret = platform_driver_register(&robot_motor_driver);
    if (ret) {
        pr_err("robot_motor: Failed to register platform driver\n");
        return ret;
    }

    /* 2. Manually allocate and register a matching platform device.
     * This forces the platform bus to match with our driver and trigger probe()! */
    robot_motor_pdev = platform_device_register_simple(MODULE_NAME, -1, NULL, 0);
    if (IS_ERR(robot_motor_pdev)) {
        pr_err("robot_motor: Failed to register platform device\n");
        platform_driver_unregister(&robot_motor_driver);
        return PTR_ERR(robot_motor_pdev);
    }

    return 0;
}

static void __exit robot_motor_exit(void)
{
    /* Unregister the device first (triggers robot_motor_remove) */
    if (robot_motor_pdev) {
        platform_device_unregister(robot_motor_pdev);
    }
    /* Finally unregister the driver */
    platform_driver_unregister(&robot_motor_driver);
}

/* Replace the old macro with standard init/exit entry points */
module_init(robot_motor_init);
module_exit(robot_motor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ray Yuan");
MODULE_DESCRIPTION("Industrial Robot Motor Character Device Driver");