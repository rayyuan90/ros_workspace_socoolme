#include "stm32f4xx.h"  // 以经典的 STM32F4 工业级芯片为例

/* =========================================================================
 * 1. 外设时钟总线（AHB/APB）约束调优
 * ========================================================================= */
void sys_clock_bus_tuning(void) {
    // 启用 RCC（时钟控制）模块
    
    /* 🧠 调优策略：
     * 假设系统主频 (SYSCLK) 为 168MHz。
     * 1. HCLK (AHB总线) 主管核心和DMA，配置为 168MHz (不分频) -> 保证高速数据吞吐
     * 2. PCLK2 (APB2高速总线) 主管电机高级定时器，配置为 84MHz (2分频) -> 保证电机PWM高精度
     * 3. PCLK1 (APB1低速总线) 主管普通 UART/I2C，配置为 42MHz (4分频) -> 降频压低功耗与 EMI
     */
    
    // 修改 RCC_CFGR 寄存器进行分频约束
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE2 | RCC_CFGR_PPRE1);
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;  // AHB Prescaler = 1  (168MHz)
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV2; // APB2 Prescaler = 2 (84MHz)
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV4; // APB1 Prescaler = 4 (42MHz)
    
    /* 💥 极客降功耗：严格关闭未使用的外设时钟 */
    RCC->AHB1ENR &= ~(RCC_AHB1ENR_ETHMACEN); // 关闭以太网时钟（如果机器人底盘不插网线）
    RCC->APB2ENR &= ~(RCC_APB2ENR_TIM10EN);  // 关闭不需要的定时器10
    
    // 开启高频电机控制和传感器通信所必需的时钟门控
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;      // 开启高级电机定时器1时钟
    RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;      // 开启IMU惯导通信用的SPI2时钟
}

/* =========================================================================
 * 2. 独立看门狗 (IWDG) 驱动架构实现
 * ========================================================================= */
void iwdg_safeguard_init(uint16_t timeout_ms) {
    /* LSI 内部低速时钟通常为 32kHz */
    // 1. 写入特殊密钥 0x5555，解锁控制寄存器写保护
    IWDG->KR = 0x5555;
    
    // 2. 设置分频系数。选择 64 分频：32kHz / 64 = 500Hz (计数器每秒减500下，即1步=2ms)
    IWDG->PR = IWDG_PR_PR_2; 
    
    // 3. 设置重装载值。根据传入的目标毫秒数计算步数
    // 步数 = timeout_ms / 2
    IWDG->RLR = (timeout_ms / 2) & 0x0FFF; // IWDG为12位寄存器，最大4095
    
    // 4. 写入 0xAAAA，让硬件立刻将 RLR 的值加载进计数器（第一次喂狗）
    IWDG->KR = 0xAAAA;
    
    // 5. 写入 0xCCCC，正式激活独立看门狗。从此一旦代码卡死，天王老子也挡不住硬件复位
    IWDG->KR = 0xCCCC;
}

inline void iwdg_feed(void) {
    IWDG->KR = 0xAAAA; // 刷新计数器
}

/* =========================================================================
 * 3. 窗口看门狗 (WWDG) 周期时序互锁实现
 * ========================================================================= */
void wwdg_safeguard_init(uint8_t tr, uint8_t window) {
    // WWDG 挂载在 APB1 上（42MHz），进一步预分频到时钟基准
    RCC->APB1ENR |= RCC_APB1ENR_WWDGEN; // 必须先开时钟门控
    
    // 设置预分频器（WDGTB），使其计数速度适合机器人的控制周期（如 10ms 核心环路）
    WWDG->CFR = WWDG_CFR_WDGTB_1 | WWDG_CFR_WDGTB_0; // 8分频
    
    // 设置窗口上限值（Window value）
    WWDG->CFR &= ~WWDG_CFR_W;
    WWDG->CFR |= (window & 0x7F);
    
    // 激活 WWDG 并写入初始计数器值（T[6:0] 必须保持第7位为1，防止其立刻复位）
    WWDG->CR = WWDG_CR_WDGA | (tr & 0x7F);
}

void wwdg_feed(uint8_t feed_val) {
    // 💥 窗口看门狗防作弊核心：
    // 读取当前计数器的值，必须小于配置的窗口上限值，才允许喂狗
    if ((WWDG->CR & 0x7F) < (WWDG->CFR & 0x7F)) {
        WWDG->CR = feed_val & 0x7F; // 在安全窗口期内，顺利喂狗
    } else {
        // 如果代码执行得太快（或者发生了诡异的跳转导致提前执行到这里）
        // 这里绝对不喂狗！直接等待 WWDG 硬件将其强行复位，暴露软件时序 Bug
    }
}

/* =========================================================================
 * 4. 机器人核心控制主循环：双狗协同安全策略
 * ========================================================================= */
int main(void) {
    // 初始化系统总线约束调优
    sys_clock_bus_tuning();
    
    // 启动独立看门狗：容忍上限 50ms（防死锁、防硬件烧毁）
    iwdg_safeguard_init(50);
    
    // 启动窗口看门狗：初始值 0x7F，窗口上限 0x5F（严格约束时序）
    wwdg_safeguard_init(0x7F, 0x5F);
    
    while(1) {
        // 执行机器人高频核心控制算法（如无刷电机 FOC 矢量计算）
        // ... 执行业务逻辑 ...
        
        // 模拟精确的延时或者等待 RTOS 的 10ms 节拍通知
        // delay_ms(10); 
        
        /* 💥 双狗协同喂养艺术 */
        iwdg_feed(); // 随时可以喂独立看门狗，保证底线不复位
        
        // 严格遵循窗口时序喂窗口狗。
        // 如果因为别的突发中断导致主循环被拖延、或者由于未知原因提前执行，wwdg_feed 内部都会触发防御
        wwdg_feed(0x7F); 
    }
}