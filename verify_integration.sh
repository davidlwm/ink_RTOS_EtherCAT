#!/bin/bash
# FreeRTOS + EtherCAT 集成验证脚本

echo "======================================"
echo "FreeRTOS + EtherCAT 集成验证"
echo "======================================"

echo "1. 检查主要文件是否存在..."

# 检查主要文件
files_to_check=(
    "Src/main.c"
    "Src/freertos_ethercat_integration.c"
    "Inc/FreeRTOSConfig.h"
    "Inc/freertos_ethercat_integration.h"
    "Ethercat/Inc/ecat_def.h"
)

all_files_exist=true

for file in "${files_to_check[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file 存在"
    else
        echo "  ✗ $file 不存在"
        all_files_exist=false
    fi
done

echo ""
echo "2. 检查关键配置..."

# 检查 USE_DEFAULT_MAIN 设置
if grep -q "USE_DEFAULT_MAIN.*0" Ethercat/Inc/ecat_def.h; then
    echo "  ✓ USE_DEFAULT_MAIN 已设置为 0"
else
    echo "  ✗ USE_DEFAULT_MAIN 配置错误"
    all_files_exist=false
fi

# 检查 main 函数是否包含 FreeRTOS 初始化
if grep -q "FreeRTOS_EtherCAT_Init" Src/main.c; then
    echo "  ✓ main.c 包含 FreeRTOS 初始化"
else
    echo "  ✗ main.c 缺少 FreeRTOS 初始化"
    all_files_exist=false
fi

# 检查 SysTick 中断是否支持 FreeRTOS
if grep -q "xPortSysTickHandler" Src/stm32f4xx_it.c; then
    echo "  ✓ SysTick 中断已配置 FreeRTOS 支持"
else
    echo "  ✗ SysTick 中断缺少 FreeRTOS 支持"
    all_files_exist=false
fi

echo ""
echo "3. 集成状态总结..."

if $all_files_exist; then
    echo "  ✓ FreeRTOS + EtherCAT 集成完成"
    echo ""
    echo "4. 下一步操作："
    echo "  - 使用 Keil MDK 或其他 IDE 编译项目"
    echo "  - 确保添加 FreeRTOS 源文件路径到项目"
    echo "  - 检查链接器配置"
    echo "  - 烧录到硬件进行测试"
    echo ""
    echo "5. 验证功能："
    echo "  - LED 应该开始闪烁 (Task_LEDBlink)"
    echo "  - 系统监控任务应该运行 (Task_SystemMonitor)"
    echo "  - EtherCAT 通信应该正常工作"
    echo "  - 测试任务应该打印系统信息"
    exit 0
else
    echo "  ✗ 集成未完成，请检查上述错误"
    exit 1
fi