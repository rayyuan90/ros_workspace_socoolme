#include "main.h"

#define BUFFER_SIZE 256
uint8_t imu_raw_buffer[BUFFER_SIZE]; // 循环缓冲区

void IMU_DMA_Start(SPI_HandleTypeDef *hspi) {
    // 启动循环模式：传输完成会自动从头开始，无缝衔接
    HAL_SPI_Receive_DMA(hspi, imu_raw_buffer, BUFFER_SIZE);
}

// 核心：半传输回调，处理 buffer 的前一半
void HAL_SPI_RxHalfCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        // CPU 处理前半段，此时 DMA 正在填后半段
        Process_IMU_Data(&imu_raw_buffer[0], BUFFER_SIZE / 2);
    }
}

// 核心：传输完成回调，处理 buffer 的后一半
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        // CPU 处理后半段，此时 DMA 正在填前半段
        Process_IMU_Data(&imu_raw_buffer[BUFFER_SIZE / 2], BUFFER_SIZE / 2);
    }
}