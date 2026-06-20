#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of.h>          // 包含设备树操作 API
#include <linux/of_irq.h>      // 包含设备树中断解析 API

#define MODULE_NAME "robot_lidar_dt"

static int lidar_irq_number;
static void __iomem *lidar_reg_base; /* 映射后的寄存器虚拟地址 */

static irqreturn_t lidar_hard_irq_handler(int irq, void *dev_id) {
    return IRQ_WAKE_THREAD;
}

static irqreturn_t lidar_threaded_irq_handler(int irq, void *dev_id) {
    pr_info("[%s] Bottom half: Parsing lidar data...\n", MODULE_NAME);
    return IRQ_HANDLED;
}

/* =========================================================================
 * 核心：探测函数 (当设备树节点与驱动 compatible 匹配成功时被内核调用)
 * ========================================================================= */
static int lidar_dt_probe(struct platform_device *pdev) {
    struct resource *res;
    int ret;

    pr_info("[%s] Device Tree Match Success! Probe starting...\n", MODULE_NAME);

    /* 1. 从内核自动生成的 pdev 中提取寄存器物理内存资源 (对应 dts 中的 reg) */
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        pr_err("[%s] Failed to get memory resource\n", MODULE_NAME);
        return -EINVAL;
    }
    pr_info("[%s] DTS physical memory base: %pa, size: %pa\n", 
            MODULE_NAME, &res->start, &res->end);

    /* 2. 将硬件物理地址映射为内核虚拟地址（这样你就能用 readl/writel 操控雷达寄存器了） */
    lidar_reg_base = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(lidar_reg_base)) {
        return PTR_ERR(lidar_reg_base);
    }

    /* 3. 从 pdev 中提取内核自动转换好的、完全合法的系统中断号 (对应 dts 中的 interrupts) */
    lidar_irq_number = platform_get_irq(pdev, 0);
    if (lidar_irq_number < 0) {
        pr_err("[%s] Failed to get IRQ from Device Tree\n", MODULE_NAME);
        return lidar_irq_number;
    }
    pr_info("[%s] DTS auto-translated system IRQ number: %d\n", MODULE_NAME, lidar_irq_number);

    /* 4. 注册线程化中断（这里的系统中断号是合法的，不会再报 -22 错误） */
    ret = devm_request_threaded_irq(
        &pdev->dev,
        lidar_irq_number,
        lidar_hard_irq_handler,
        lidar_threaded_irq_handler,
        IRQF_ONESHOT, // 触发触发沿通常在DTS中定义，这里保持ONESHOT即可
        "robot_lidar_dt_service",
        NULL
    );
    if (ret) {
        pr_err("[%s] Failed to request threaded IRQ\n", MODULE_NAME);
        return ret;
    }

    return 0;
}

static void lidar_dt_remove(struct platform_device *pdev) {
    pr_info("[%s] Driver removed.\n", MODULE_NAME);
}

/* =========================================================================
 * 驱动与设备树匹配表 (这是解耦的关键枢纽)
 * ========================================================================= */
static const struct of_device_id robot_lidar_dt_ids[] = {
    { .compatible = "socool,robot-lidar" }, /* 必须与 DTS 中的字符串逐字严格匹配 */
    { /* 哨兵节点，必须留空作为结束标志 */ }
};
MODULE_DEVICE_TABLE(of, robot_lidar_dt_ids);

static struct platform_driver lidar_dt_driver = {
    .probe = lidar_dt_probe,
    .remove = lidar_dt_remove,
    .driver = {
        .name = MODULE_NAME,
        .of_match_table = robot_lidar_dt_ids, /* 绑定设备树匹配表 */
    },
};

module_platform_driver(lidar_dt_driver); /* 宏包裹：自动生成 init 和 exit 函数并注册 */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ray Yuan");