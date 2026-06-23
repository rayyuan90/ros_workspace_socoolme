#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;

// 补全：实现 fops 接口
static int camera_mmap(struct file *filp, struct vm_area_struct *vma) {
    // 确保 mmap 的大小与 DMA 申请的大小一致
    return dma_mmap_coherent(NULL, vma, dma_buffer, dma_handle, FRAME_SIZE);
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .mmap  = camera_mmap, // 这里将 mmap 挂载到 fops
};

static int __init camera_driver_init(void) {
    // 1. 动态申请设备号
    alloc_chrdev_region(&dev_num, 0, 1, "camera_dma");
    
    // 2. 初始化字符设备并绑定 fops
    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev_num, 1);
    
    // 3. 自动在 /dev 下创建节点 (camera_dma)
    my_class = class_create(THIS_MODULE, "camera_class");
    device_create(my_class, NULL, dev_num, NULL, "camera_dma");
    
    // 4. DMA 内存申请 (如前所述)
    dma_buffer = dma_alloc_coherent(NULL, FRAME_SIZE, &dma_handle, GFP_KERNEL);
    
    return 0;
}
module_init(camera_driver_init);