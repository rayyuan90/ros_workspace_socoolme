#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/irq_sim.h> // Linux kernel interrupt simulation subsystem

#define MODULE_NAME "robot_lidar_irq"

static int lidar_irq_number;
static struct mutex data_lock;    
static struct platform_device *lidar_pdev;
static struct irq_domain *sim_domain; // Virtual interrupt allocator

/* =========================================================================
 * 1. Top Half (Hard IRQ Context)
 * ========================================================================= */
static irqreturn_t lidar_hard_irq_handler(int irq, void *dev_id) {
    /* Fast pathway: fire the bottom half thread immediately */
    return IRQ_WAKE_THREAD;
}

/* =========================================================================
 * 2. Bottom Half (Threaded Process Context)
 * ========================================================================= */
static irqreturn_t lidar_threaded_irq_handler(int irq, void *dev_id) {
    pr_info("[%s] Bottom half active! Handled by thread: %s (PID: %d)\n", 
            MODULE_NAME, current->comm, current->pid);

    mutex_lock(&data_lock);
    pr_info("[%s] Simulating 4KB Lidar data decompression...\n", MODULE_NAME);
    msleep(5); // Safe to sleep here!
    mutex_unlock(&data_lock);
    
    pr_info("[%s] Processing complete. Thread returning to sleep.\n", MODULE_NAME);
    return IRQ_HANDLED;
}

/* =========================================================================
 * 3. Driver Probe & Initialization Lifecycle
 * ========================================================================= */
static int lidar_irq_probe(struct platform_device *pdev) {
    int ret;
    struct device *dev = &pdev->dev;
    
    pr_info("[%s] Device matched. Allocating software IRQ simulation context...\n", MODULE_NAME);
    mutex_init(&data_lock);

    /* Allocate a managed software interrupt domain with 1 available interrupt line */
    sim_domain = devm_irq_domain_create_sim(dev, NULL, 1);
    if (IS_ERR(sim_domain)) {
        pr_err("[%s] Failed to allocate software IRQ domain\n", MODULE_NAME);
        return PTR_ERR(sim_domain);
    }

    /* Retrieve the dynamically assigned, safe system IRQ number for line index 0 */
    lidar_irq_number = irq_create_mapping(sim_domain, 0);
    if (lidar_irq_number <= 0) {
        pr_err("[%s] Failed to map simulation IRQ descriptor\n", MODULE_NAME);
        return -EINVAL;
    }

    /* Register the Threaded IRQ on our safe, simulated line */
    ret = request_threaded_irq(
        lidar_irq_number,
        lidar_hard_irq_handler,     
        lidar_threaded_irq_handler, 
        IRQF_TRIGGER_RISING | IRQF_ONESHOT, 
        "robot_lidar_service",      
        &data_lock                  
    );

    if (ret) {
        pr_err("[%s] Threaded IRQ allocation failed on IRQ %d: error %d\n", MODULE_NAME, lidar_irq_number, ret);
        irq_dispose_mapping(lidar_irq_number);
        return ret;
    }

    pr_info("[%s] Threaded IRQ successfully requested on SAFE SYSTEM IRQ %d\n", MODULE_NAME, lidar_irq_number);
    return 0;
}

#if KERNEL_VERSION(6, 11, 0) <= LINUX_VERSION_CODE
static void lidar_irq_remove(struct platform_device *pdev)
#else
static int lidar_irq_remove(struct platform_device *pdev)
#endif
{
    free_irq(lidar_irq_number, &data_lock);
    irq_dispose_mapping(lidar_irq_number);
    mutex_destroy(&data_lock);
    pr_info("[%s] Driver unloaded clean. IRQ resources freed.\n", MODULE_NAME);
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
MODULE_DESCRIPTION("Threaded IRQ Real-Time Driver Emulation");