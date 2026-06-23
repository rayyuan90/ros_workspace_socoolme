#include "stm32f4xx_hal.h"

// 声明句柄
I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_rx;

// 存储 IMU 数据的缓冲区 (必须是全局或静态，确保 DMA 能访问到)
#define IMU_DATA_SIZE 14
uint8_t imu_raw_buffer[IMU_DATA_SIZE]; 
volatile uint8_t imu_dma_complete_flag = 0;

// 硬件初始化 (在 main.c 中调用)
void MX_I2C1_Init(void) {
    hi2c1.Instance = I2C1;
    // ... 配置速率 400kHz ...
    HAL_I2C_Init(&hi2c1);
}

// DMA 初始化 (通常由 CubeMX 生成，手动修改需注意)
void HAL_I2C_MspInit(I2C_HandleTypeDef* i2cHandle) {
    if(i2cHandle->Instance == I2C1) {
        __HAL_RCC_DMA1_CLK_ENABLE();
        
        // 配置 DMA 通道 (以 STM32F4 为例)
        hdma_i2c1_rx.Instance = DMA1_Stream0;
        hdma_i2c1_rx.Init.Channel = DMA_CHANNEL_1;
        hdma_i2c1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_i2c1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_i2c1_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_i2c1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_i2c1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_i2c1_rx.Init.Mode = DMA_NORMAL;
        hdma_i2c1_rx.Init.Priority = DMA_PRIORITY_HIGH;
        HAL_DMA_Init(&hdma_i2c1_rx);

        // 【关键】将 DMA 句柄链接到 I2C 句柄
        __HAL_LINKDMA(i2cHandle, hdmarx, hdma_i2c1_rx);
        
        // 配置中断 (必须)
        HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
    }
}