#include "stm32f4xx.h" // 假设使用 STM32F4

// ==========================================================
// 1. 全局标志位定义 (必须使用 volatile，防止编译器过度优化)
// ==========================================================
volatile uint8_t flag_1ms_tick = 0;   // 1ms 定时器滴答标志 (用于电机PID控制)
volatile uint8_t flag_uart_rx_ok = 0; // 串口接收完成标志 (用于解析上位机指令)
volatile uint8_t flag_sensor_ready = 0; // 传感器数据就绪标志

// 全局数据缓存
uint8_t uart_rx_buffer[64];
float current_motor_speed = 0.0f;

// ==========================================================
// 2. 后台：硬件中断服务函数 (越短越好，快进快出)
// ==========================================================

// 假设配置了 TIM3 为 1ms 触发一次中断
void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // 清除中断标志
        
        flag_1ms_tick = 1; // 仅仅举起标志位，立刻退出！
    }
}

// 串口接收中断 (与 ROS 2 上位机通信)
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        // 读取硬件寄存器，存入缓存...
        // 如果检测到完整的一帧数据（例如遇到了帧尾 0xAA）：
        flag_uart_rx_ok = 1; // 举起标志位，通知主循环去处理复杂解析
        
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

// ==========================================================
// 3. 前台：主循环与状态机
// ==========================================================

int main(void) {
    // A. 硬件初始化 (RCC时钟树、GPIO、定时器、串口、看门狗等)
    Hardware_Init(); 
    
    // 配置看门狗 (假设配置为 10ms 超时)
    IWDG_Config(); 

    // B. 进入死循环 (主执行流)
    while (1) {
        
        // --------------------------------------------------
        // 任务 1：高频实时任务 (例如：电机 FOC/PID 运算)
        // --------------------------------------------------
        if (flag_1ms_tick == 1) {
            flag_1ms_tick = 0; // 进门第一件事：放下标志位
            
            // 执行极其精确的 PID 计算
            current_motor_speed = Read_Encoder();
            Run_Motor_PID_Loop(current_motor_speed); 
        }

        // --------------------------------------------------
        // 任务 2：异步通信处理 (例如：解析 Jetson 发来的速度指令)
        // --------------------------------------------------
        if (flag_uart_rx_ok == 1) {
            flag_uart_rx_ok = 0; // 放下标志位
            
            // 执行耗时的字符串解析或校验和计算
            Parse_ROS2_Command(uart_rx_buffer); 
        }

        // --------------------------------------------------
        // 任务 3：低优先级状态机 (例如：电池电量监控、UI 指示灯)
        // --------------------------------------------------
        if (flag_sensor_ready == 1) {
            flag_sensor_ready = 0;
            Update_Battery_Status_Machine(); // 状态机：不阻塞代码，分步执行
        }

        // --------------------------------------------------
        // 最终底线：喂狗
        // --------------------------------------------------
        // 只有当上方所有代码都没有死循环、没有卡死时，
        // 才能安全地执行到这里。证明整个系统这一圈是健康的。
        IWDG_ReloadCounter(); // 等同于 iwdg_feed()
    }
}