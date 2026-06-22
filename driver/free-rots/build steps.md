打开终端，确保你的系统拥有必要的底层工具：

Bash
sudo apt update
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi gdb

步骤 2：硬件连接
将 ST-Link V2（通常是一个几十块钱的小 U 盘形状设备）通过 4 根杜邦线连接到 STM32 板载的 SWD 接口：

3.3V -> 3.3V (如果板子已独立供电则不要连)

GND -> GND

SWDIO -> SWDIO

SWCLK -> SWCLK

将 ST-Link 插入电脑 USB 口。输入 lsusb，如果你看到类似 STMicroelectronics ST-LINK/V2 的输出，说明硬件识别成功

步骤 3：编译你的工程
在工程根目录执行标准的 CMake 越权构建流：

Bash
mkdir build
cd build
cmake ..
make -j$(nproc)

译成功后，终端末尾会打印出 .elf 文件的大小（text段是你的代码占用 Flash 的大小，bss 和 data 是占用 RAM 的大小），并在 build 目录下生成 Robot_MainController.elf 和 .bin 文件。

步骤 4：一键烧录命令 (闪电行动)
无需打开任何沉重的图形界面软件。在终端中，仍在 build 目录下，直接执行以下命令：

Bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program Robot_MainController.elf verify reset exit"
参数拆解：

-f interface/stlink.cfg: 告诉 OpenOCD 你使用的是 ST-Link 烧录器。

-f target/stm32f4x.cfg: 告诉 OpenOCD 目标芯片是 STM32F4 系列，它会自动配置 Flash 扇区。

-c "program ...": 这是一个自动化命令序列。它会：

program: 烧录 .elf 文件（OpenOCD 能自动解析 ELF 的地址，无需手动指定起始地址）。

verify: 烧录后立刻回读 Flash，确保每一比特数据都正确写入。

reset: 软复位芯片，让你的代码立即开始运行。

exit: 烧录完成后自动关闭 OpenOCD 进程，将终端交还给你。

高阶架构师配置：VS Code 自动化整合
作为习惯于高效工具链的架构师，你可以将上述流程整合进 VS Code，实现按下一个快捷键，自动完成“编译 + 烧录 + 复位”。

在工程目录下的 .vscode/tasks.json 中配置：

JSON
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake Build",
            "type": "shell",
            "command": "cd build && cmake .. && make -j$(nproc)",
            "group": "build",
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "Flash to STM32",
            "type": "shell",
            "command": "openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c 'program build/Robot_MainController.elf verify reset exit'",
            "dependsOn": "CMake Build",
            "problemMatcher": []
        }
    ]
}
配置好后，在 VS Code 中按下 Ctrl+Shift+P，输入 Run Task，选择 Flash to STM32，VS Code 就会在后台瞬间完成多线程编译并把最新的固件推送到机器人的大脑中。