#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>

#define DMAC_SRC_REG     0x50001000  // DMA 控制器：源地址寄存器
#define DMAC_DEST_REG    0x50001004  // DMA 控制器：目的地址寄存器
#define DMAC_COUNT_REG   0x50001008  // DMA 控制器：计数寄存器
#define DMAC_CTRL_REG    0x5000100C  // DMA 控制器：控制寄存器

#define BUFFER_SIZE      4096

static int dma_irq_number;
static char *dma_cpu_buffer;        // CPU 访问的虚拟地址
static dma_addr_t dma_handle;       // 💥 关键：给 DMA 硬件使用的物理/总线地址
static void __iomem *dmac_base;

// 1. 当雷达有数据要发时，应用层或驱动发起传输初始化
void trigger_lidar_dma_transfer(void) {
    // ⚙️ CPU 仅执行初始化配置：指明源头、目的地和大小
    iowrite32(0x40001000, dmac_base + DMAC_SRC_REG);   // 源头：雷达硬件寄存器
    iowrite32(dma_handle, dmac_base + DMAC_DEST_REG);  // 目的地：刚刚申请的 DMA 物理物理内存
    iowrite32(BUFFER_SIZE, dmac_base + DMAC_COUNT_REG);// 大小：4KB
    
    // 🚀 启动 DMA！配置一写完，CPU 立刻释放总线，回去跑核心算法！
    // 此时硬件总线（Bus）由 DMA 控制器接管，数据自动从雷达流向 RAM
    iowrite32(0x01, dmac_base + DMAC_CTRL_REG);        // 0x01 代表 START + ENABLE IRQ
}

// 2. 💥 DMA 中断服务程序
// 只有当 4KB 数据【全部自动搬完】后，DMA 控制器才会触发这个中断通知 CPU
static irqreturn_t dma_complete_interrupt_handler(int irq, void *dev_id) {
    // 清除 DMA 硬件中断标记
    iowrite32(0x00, dmac_base + DMAC_CTRL_REG);
    
    // ✅ 此时，dma_cpu_buffer 内部已经完美静静躺着 4KB 激光点云数据了！
    // 整个过程 CPU 没有执行哪怕一次数据拷贝的 for 循环！
    pr_info("DMA Hardware transferred 4KB. CPU just came here to read the result.\n");
    
    return IRQ_HANDLED;
}

static int __init dma_driver_init(void) {
    dmac_base = ioremap(DMAC_SRC_REG, 16);
    
    // 💥 申请一块一致性 DMA 内存（Coherent DMA mapping），打通虚拟地址与物理内存
    struct device *dev = NULL; // 在真实 PCIe 驱动中为 pci_dev->dev
    dma_cpu_buffer = dma_alloc_coherent(dev, BUFFER_SIZE, &dma_handle, GFP_KERNEL);
    
    // 绑定的是 DMA 完成中断
    request_irq(dma_irq_number, dma_complete_interrupt_handler, IRQF_TRIGGER_RISING, "lidar_dma", NULL);
    return 0;
}