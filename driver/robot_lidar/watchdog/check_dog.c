void check_reset_source(void) {
    if (RCC->CSR & RCC_CSR_IWDGRSTF) {
        // 🚨 惊天警报：上一次系统重启是因为独立看门狗超时！
        // 诊断结论：代码发生了绝对死循环，或者硬件被强电磁干扰卡死！
        // 应急措施：自动进入安全跛行模式（Limp Mode），机械臂脱机。
    }
    else if (RCC->CSR & RCC_CSR_WWDGRSTF) {
        // ⚠️ 警报：上一次系统重启是因为窗口看门狗判定时序错乱！
        // 诊断结论：软件执行周期发生了严重的抖动（Jitter），或者有人提前喂狗！
    }
    
    // 💥 极其重要：读取完后必须清除复位标志位，否则下一次无法准确判定
    RCC->CSR |= RCC_CSR_RMVF;
}