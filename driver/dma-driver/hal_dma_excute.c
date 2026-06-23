// 在你的电机控制任务或 IMU 采样任务中
void Task_IMU_Sample(void *pvParameters) {
    for(;;) {
        // 发起 DMA 读取，函数会立刻返回，不占用 CPU 时间
        // MPU6050 地址 0xD0，从寄存器 0x3B 开始读取 14 字节
        HAL_I2C_Mem_Read_DMA(&hi2c1, 0xD0, 0x3B, I2C_MEMADD_SIZE_8BIT, imu_raw_buffer, IMU_DATA_SIZE);
        
        // 这里 CPU 可以继续执行 FOC 计算或其他逻辑...
        
        // 等待 DMA 完成中断回调置位
        if (imu_dma_complete_flag) {
            imu_dma_complete_flag = 0;
            // 解析 imu_raw_buffer ...
        }
        
        vTaskDelay(pdMS_TO_TICKS(2)); // 500Hz 采样
    }
}

// DMA 完成回调函数 (由 HAL 库自动调用)
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c->Instance == I2C1) {
        imu_dma_complete_flag = 1; // 通知主任务解析数据
    }
}