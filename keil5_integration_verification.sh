#!/bin/bash
# Keil5 FreeRTOS集成最终验证脚本

echo "========================================="
echo "Keil5 + FreeRTOS Stub 集成验证"
echo "========================================="

echo "1. 检查源文件..."

# 检查必需的源文件
required_files=(
    "Src/main.c"
    "Src/freertos_stub.c"
    "Inc/freertos_stub.h"
    "Src/bsp/usart/bsp_debug_usart.c"
    "Inc/bsp/usart/bsp_debug_usart.h"
)

all_files_exist=true

for file in "${required_files[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file 存在"
    else
        echo "  ✗ $file 不存在"
        all_files_exist=false
    fi
done

echo ""
echo "2. 检查main.c集成..."

# 检查main.c的FreeRTOS集成
if grep -q '#include "freertos_stub.h"' Src/main.c; then
    echo "  ✓ main.c 包含 freertos_stub.h"
else
    echo "  ✗ main.c 缺少 freertos_stub.h"
    all_files_exist=false
fi

if grep -q 'vTaskStartScheduler()' Src/main.c; then
    echo "  ✓ main.c 调用 vTaskStartScheduler()"
else
    echo "  ✗ main.c 缺少调度器启动"
    all_files_exist=false
fi

if grep -q 'xTaskCreate(' Src/main.c; then
    echo "  ✓ main.c 创建FreeRTOS任务"
else
    echo "  ✗ main.c 缺少任务创建"
    all_files_exist=false
fi

echo ""
echo "3. 检查fputc重复定义解决..."

# 检查fputc函数
if ! grep -q 'int fputc(' Src/main.c; then
    echo "  ✓ main.c 已移除fputc重定义"
else
    echo "  ✗ main.c 仍包含fputc定义"
    all_files_exist=false
fi

if grep -q 'int fputc(' Src/bsp/usart/bsp_debug_usart.c; then
    echo "  ✓ bsp_debug_usart.c 包含fputc实现"
else
    echo "  ✗ bsp_debug_usart.c 缺少fputc实现"
    all_files_exist=false
fi

echo ""
echo "4. 检查FreeRTOS Stub实现..."

# 检查FreeRTOS存根实现
if grep -q 'xTaskCreate(' Src/freertos_stub.c; then
    echo "  ✓ freertos_stub.c 实现xTaskCreate"
else
    echo "  ✗ freertos_stub.c 缺少xTaskCreate实现"
    all_files_exist=false
fi

if grep -q 'vTaskStartScheduler(' Src/freertos_stub.c; then
    echo "  ✓ freertos_stub.c 实现调度器"
else
    echo "  ✗ freertos_stub.c 缺少调度器实现"
    all_files_exist=false
fi

echo ""
echo "5. 系统架构总结..."

echo "  📋 任务架构:"
echo "     - Task_LEDBlink (优先级1): LED闪烁指示"
echo "     - Task_SystemMonitor (优先级2): 系统状态监控"
echo "     - Task_EtherCATApplication (优先级3): EtherCAT应用处理"
echo "     - EtherCAT MainLoop: 高频率轮询调度"

echo ""
echo "  🔧 通信输出:"
echo "     - printf通过UART串口输出"
echo "     - LED状态指示系统运行"
echo "     - 系统状态定期打印"

echo ""
echo "6. Keil5项目配置提醒..."

echo "  ⚠️  手动操作需求:"
echo "     1. 在Keil5中添加 Src/freertos_stub.c 到项目"
echo "     2. 确认包含路径包含 Inc/ 目录"
echo "     3. 验证预处理器定义: USE_HAL_DRIVER,STM32F407xx"
echo "     4. 编译并解决任何链接问题"

echo ""
if $all_files_exist; then
    echo "✅ 所有文件集成完成"
    echo "📝 请按照 Keil5_Project_Configuration_Guide.md 完成项目配置"
    echo "🚀 配置完成后即可编译和运行"
    exit 0
else
    echo "❌ 集成未完成，请检查上述错误"
    exit 1
fi