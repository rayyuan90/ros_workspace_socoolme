#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/gpio/consumer.h> // Added for modern GPIO descriptor mapping

#define MODULE_NAME "robot_lidar_irq"

static int lidar_irq_number;      /* Changed: Will be assigned dynamically by the kernel */
static struct mutex data_lock;    
static struct platform_device *lidar_pdev;
static struct gpio_desc *lidar_gpio; // Virtual descriptor to back our interrupt

/* =========================================================================
 * 1. 顶半部 (Top Half / Hard IRQ)
 * ========================================================================= */
static irqreturn_t lidar_hard_irq_handler(int irq, void *dev_id) {
    return IRQ_WAKE_THREAD;
}

/* =========================================================================
 * 2. 底半部 (Bottom Half / Threaded IRQ)
 * ========================================================================= */
static irqreturn_t lidar_threaded_irq_handler(int irq, void *dev_id) {
    pr_info("[%s] Bottom half: Kernel thread woken up! Context: %s (PID: %d)\n", 
            MODULE_NAME, current->comm, current->pid);

    mutex_lock(&data_lock);
    pr_info("[%s] Parsing 4KB point cloud data data...\n", MODULE_NAME);
    msleep(5); 
    mutex_unlock(&data_lock);
    
    pr_info("[%s] Bottom half: Point cloud ready. Heading back to sleep.\n", MODULE_NAME);
    return IRQ_HANDLED;
}

/* =========================================================================
 * 3. 驱动生命周期与总线匹配
 * ========================================================================= */
static int lidar_irq_probe(struct platform_device *pdev) {
    int ret;
    struct device *dev = &pdev->dev;
    
    pr_info("[%s] Device matched. Registering Dynamic Threaded IRQ...\n", MODULE_NAME);

    mutex_init(&data_lock);

    /* Look up a virtual GPIO line (using "index 0" of optional properties).
     * This registers a safe software hook that the kernel can convert into a valid IRQ line */
    lidar_gpio = gpiod_get_index_optional(dev, "lidar", 0, GPIOD_IN);
    if (IS_ERR(lidar_gpio)) {
        return PTR_ERR(lidar_gpio);
    }

    if (lidar_gpio) {
        /* Ask the framework to dynamically assign a valid system IRQ number for this line */
        lidar_irq_number = gpiod_to_irq(lidar_gpio);
    } else {
        /* Fallback: If no device tree pin is mapped on PC emulation, use an absolute dummy line 
         * that is safe on x86 architectures instead of a low reserved ISA line */
        lidar_irq_number = 64; 
    }

    /* 💥 核心：使用动态或安全分配的 IRQ 号注册线程化中断 */
    ret = request_threaded_irq(
        lidar_irq_number,
        lidar_hard_irq_handler,     
        lidar_threaded_irq_handler, 
        IRQF_TRIGGER_RISING | IRQF_ONESHOT, 
        "robot_lidar_service",      
        &data_lock                  
    );

    if (ret) {
        pr_err("[%s] Failed to request threaded IRQ %d: error %d\n", MODULE_NAME, lidar_irq_number, ret);
        if (lidar_gpio) gpiod_put(lidar_gpio);
        return ret;
    }

    pr_info("[%s] Threaded IRQ successfully requested on valid IRQ %d\n", MODULE_NAME, lidar_irq_number);
    return 0;
}

#if KERNEL_VERSION(6, 11, 0) <= LINUX_VERSION_CODE
static void lidar_irq_remove(struct platform_device *pdev)
#else
static int lidar_irq_remove(struct platform_device *pdev)
#endif
{
    free_irq(lidar_irq_number, &data_lock);
    if (lidar_gpio) {
        gpiod_put(lidar_gpio);
    }
    mutex_destroy(&data_lock);
    pr_info("[%s] Driver removed. IRQ freed.\n", MODULE_NAME);
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
    return 0;
#endif
}

static struct platform_driver lidar_irq_driver = {
    .probe = lidar_irq_probe,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 13, 0)
    .remove = lidar_irq_remove,
#else
    .remove_new = lidar_irq_remove,
#endif
    .driver = { .name = MODULE_NAME },
};

static int __init lidar_irq_init(void) {
    int ret = platform_driver_register(&lidar_irq_driver);
    if (ret) return ret;
    lidar_pdev = platform_device_register_simple(MODULE_NAME, -1, NULL, 0);
    return PTR_ERR_OR_ZERO(lidar_pdev);
}

static void __exit lidar_irq_exit(void) {
    platform_device_unregister(lidar_pdev);
    platform_driver_unregister(&lidar_irq_driver);
}

module_init(lidar_irq_init);
module_exit(lidar_irq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ray Yuan");
MODULE_DESCRIPTION("Threaded IRQ Real-Time Driver Example");