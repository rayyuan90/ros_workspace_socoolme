cmake_minimum_required(VERSION 3.22)

# ==============================================================================
# 1. 交叉编译工具链强制设定 (必须在 project() 之前)
# ==============================================================================
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_SIZE arm-none-eabi-size)

# 定义工程名称
project(Robot_MainController C CXX ASM)

# ==============================================================================
# 2. 芯片架构与编译优化参数 (以 STM32F4 硬件浮点为例)
# ==============================================================================
set(MCU_FLAGS "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")

# C 编译标志 (开启全警告，-O2 优化，丢弃未使用的函数以节省 Flash)
set(CMAKE_C_FLAGS "${MCU_FLAGS} -Wall -Wextra -O2 -ffunction-sections -fdata-sections")
# 汇编标志
set(CMAKE_ASM_FLAGS "${MCU_FLAGS} -x assembler-with-cpp")
# 链接标志 (指定链接脚本，触发垃圾回收去除死代码，启用 C 标准库的浮点格式化)
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/STM32F407VGTX_FLASH.ld")
set(CMAKE_EXE_LINKER_FLAGS "${MCU_FLAGS} -T ${LINKER_SCRIPT} -Wl,--gc-sections -u _printf_float")

# ==============================================================================
# 3. 包含头文件目录 (Include Directories)
# ==============================================================================
include_directories(
    Core/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
    Middlewares/Third_Party/FreeRTOS/Source/include
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
)

# 全局宏定义 (配置 HAL 库和芯片型号)
add_definitions(-DUSE_HAL_DRIVER -DSTM32F407xx)

# ==============================================================================
# 4. 收集源文件 (Source Files)
# ==============================================================================
# 收集业务代码与底层初始化代码
file(GLOB_RECURSE USER_SOURCES "Core/Src/*.c")

# 收集 HAL 库代码 (根据需求裁剪，不要全部编译，太慢)
file(GLOB_RECURSE HAL_SOURCES 
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c"
    "Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_can.c"
)

# 收集 FreeRTOS 源码
file(GLOB_RECURSE FREERTOS_SOURCES 
    "Middlewares/Third_Party/FreeRTOS/Source/*.c"
)
# FreeRTOS 的移植层 (特定于 GCC Cortex-M4F)
set(FREERTOS_PORT "Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c")
set(FREERTOS_MEM  "Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c")

# 启动汇编文件 (必须有，否则无法启动主频和中断向量表)
set(STARTUP_FILE "Core/Startup/startup_stm32f407vgtx.s")

# ==============================================================================
# 5. 生成可执行文件
# ==============================================================================
add_executable(${PROJECT_NAME}.elf 
    ${USER_SOURCES} 
    ${HAL_SOURCES} 
    ${FREERTOS_SOURCES}
    ${FREERTOS_PORT}
    ${FREERTOS_MEM}
    ${STARTUP_FILE}
)

# ==============================================================================
# 6. 后处理：生成 Bin/Hex，并打印内存占用
# ==============================================================================
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_SIZE} ${PROJECT_NAME}.elf
    COMMENT "Generating .hex and .bin files, printing size..."
)