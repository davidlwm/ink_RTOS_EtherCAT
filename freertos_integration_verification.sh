#!/bin/bash
# FreeRTOS V10.4.6 Keil5集成最终验证脚本

echo "========================================="
echo "FreeRTOS V10.4.6 + Keil5 集成验证"
echo "========================================="

echo "1. 检查FreeRTOS源文件..."

# 检查FreeRTOS核心源文件
freertos_core_files=(
    "Middlewares/Third_Party/FreeRTOS/tasks.c"
    "Middlewares/Third_Party/FreeRTOS/queue.c"
    "Middlewares/Third_Party/FreeRTOS/list.c"
    "Middlewares/Third_Party/FreeRTOS/timers.c"
    "Middlewares/Third_Party/FreeRTOS/event_groups.c"
    "Middlewares/Third_Party/FreeRTOS/stream_buffer.c"
)

freertos_portable_files=(
    "Middlewares/Third_Party/FreeRTOS/portable/RVDS/ARM_CM4F/port.c"
    "Middlewares/Third_Party/FreeRTOS/portable/RVDS/ARM_CM4F/portmacro.h"
    "Middlewares/Third_Party/FreeRTOS/portable/MemMang/heap_4.c"
)

freertos_headers=(
    "Middlewares/Third_Party/FreeRTOS/include/FreeRTOS.h"
    "Middlewares/Third_Party/FreeRTOS/include/task.h"
    "Middlewares/Third_Party/FreeRTOS/include/queue.h"
    "Middlewares/Third_Party/FreeRTOS/include/semphr.h"
    "Middlewares/Third_Party/FreeRTOS/include/event_groups.h"
    "Middlewares/Third_Party/FreeRTOS/include/timers.h"
    "Middlewares/Third_Party/FreeRTOS/include/portable.h"
)

config_files=(
    "Inc/FreeRTOSConfig.h"
    "Src/main.c"
)

all_files_exist=true

echo "  📦 FreeRTOS核心源文件:"
for file in "${freertos_core_files[@]}"; do
    if [ -f "$file" ]; then
        echo "    ✓ $file 存在"
    else
        echo "    ✗ $file 不存在"
        all_files_exist=false
    fi
done

echo "  🔧 FreeRTOS端口文件:"
for file in "${freertos_portable_files[@]}"; do
    if [ -f "$file" ]; then
        echo "    ✓ $file 存在"
    else
        echo "    ✗ $file 不存在"
        all_files_exist=false
    fi
done

echo "  📄 FreeRTOS头文件:"
for file in "${freertos_headers[@]}"; do
    if [ -f "$file" ]; then
        echo "    ✓ $file 存在"
    else
        echo "    ✗ $file 不存在"
        all_files_exist=false
    fi
done

echo "  ⚙️ 配置文件:"
for file in "${config_files[@]}"; do
    if [ -f "$file" ]; then
        echo "    ✓ $file 存在"
    else
        echo "    ✗ $file 不存在"
        all_files_exist=false
    fi
done

echo ""
echo "2. 检查main.c的FreeRTOS集成..."

# 检查main.c的FreeRTOS集成
if grep -q '#include "FreeRTOS.h"' Src/main.c; then
    echo "  ✓ main.c 包含 FreeRTOS.h"
else
    echo "  ✗ main.c 缺少 FreeRTOS.h"
    all_files_exist=false
fi

if grep -q '#include "task.h"' Src/main.c; then
    echo "  ✓ main.c 包含 task.h"
else
    echo "  ✗ main.c 缺少 task.h"
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

if grep -q 'Task_EtherCATMainLoop' Src/main.c; then
    echo "  ✓ main.c 包含EtherCAT MainLoop任务"
else
    echo "  ✗ main.c 缺少EtherCAT MainLoop任务"
    all_files_exist=false
fi

echo ""
echo "3. 检查Stub文件清理..."

# 检查是否已删除stub文件
if ! [ -f "Src/freertos_stub.c" ]; then
    echo "  ✓ freertos_stub.c 已删除"
else
    echo "  ✗ freertos_stub.c 仍存在"
    all_files_exist=false
fi

if ! [ -f "Inc/freertos_stub.h" ]; then
    echo "  ✓ freertos_stub.h 已删除"
else
    echo "  ✗ freertos_stub.h 仍存在"
    all_files_exist=false
fi

if ! grep -q 'freertos_stub.h' Src/main.c; then
    echo "  ✓ main.c 已移除stub头文件引用"
else
    echo "  ✗ main.c 仍引用stub头文件"
    all_files_exist=false
fi

echo ""
echo "4. 检查任务架构..."

# 检查任务函数结构
if grep -q 'for(;;)' Src/main.c; then
    echo "  ✓ 任务使用无限循环结构"
else
    echo "  ✗ 任务缺少无限循环结构"
    all_files_exist=false
fi

if grep -q 'vTaskDelay(' Src/main.c; then
    echo "  ✓ 任务使用vTaskDelay延时"
else
    echo "  ✗ 任务缺少FreeRTOS延时"
    all_files_exist=false
fi

if grep -q 'TaskHandle_t' Src/main.c; then
    echo "  ✓ 使用正确的TaskHandle_t类型"
else
    echo "  ✗ 缺少TaskHandle_t类型定义"
    all_files_exist=false
fi

echo ""
echo "5. 检查配置文件内容..."

# 检查FreeRTOSConfig.h配置
if grep -q 'configTICK_RATE_HZ' Inc/FreeRTOSConfig.h; then
    echo "  ✓ FreeRTOSConfig.h 包含时钟配置"
else
    echo "  ✗ FreeRTOSConfig.h 缺少时钟配置"
    all_files_exist=false
fi

if grep -q 'configTOTAL_HEAP_SIZE' Inc/FreeRTOSConfig.h; then
    echo "  ✓ FreeRTOSConfig.h 包含堆内存配置"
else
    echo "  ✗ FreeRTOSConfig.h 缺少堆内存配置"
    all_files_exist=false
fi

if grep -q 'ETHERCAT_SYNC_TASK_PRIORITY' Inc/FreeRTOSConfig.h; then
    echo "  ✓ FreeRTOSConfig.h 包含EtherCAT优先级定义"
else
    echo "  ✗ FreeRTOSConfig.h 缺少EtherCAT优先级定义"
    all_files_exist=false
fi

echo ""
echo "6. Keil5项目集成指导..."

echo "  📋 需要手动添加到Keil5项目的文件:"
echo "     📁 FreeRTOS Core组:"
echo "        - Middlewares/Third_Party/FreeRTOS/tasks.c"
echo "        - Middlewares/Third_Party/FreeRTOS/queue.c"
echo "        - Middlewares/Third_Party/FreeRTOS/list.c"
echo "        - Middlewares/Third_Party/FreeRTOS/timers.c"
echo "        - Middlewares/Third_Party/FreeRTOS/event_groups.c"
echo "        - Middlewares/Third_Party/FreeRTOS/stream_buffer.c"
echo ""
echo "     📁 FreeRTOS Portable组:"
echo "        - Middlewares/Third_Party/FreeRTOS/portable/RVDS/ARM_CM4F/port.c"
echo "        - Middlewares/Third_Party/FreeRTOS/portable/MemMang/heap_4.c"

echo ""
echo "  ⚙️ 包含路径设置:"
echo "     添加到C/C++ Include Paths:"
echo "        - Middlewares\\Third_Party\\FreeRTOS\\include"
echo "        - Middlewares\\Third_Party\\FreeRTOS\\portable\\RVDS\\ARM_CM4F"

echo ""
echo "  🎯 任务架构总结:"
echo "     - Task_LEDBlink (优先级1): LED闪烁，500ms周期"
echo "     - Task_SystemMonitor (优先级2): 系统监控，1000ms周期"
echo "     - Task_EtherCATApplication (优先级ETHERCAT_APP_TASK_PRIORITY): 应用处理，10ms周期"
echo "     - Task_EtherCATMainLoop (优先级ETHERCAT_SYNC_TASK_PRIORITY): 高频轮询，1ms周期"

echo ""
echo "  🔧 预期串口输出:"
echo "     ================================="
echo "     FreeRTOS + EtherCAT Integration"
echo "     Using Real FreeRTOS V10.4.6"
echo "     System Starting..."
echo "     ================================="

echo ""
if $all_files_exist; then
    echo "✅ FreeRTOS V10.4.6集成准备完成"
    echo "📝 请按照 FreeRTOS_Integration_Guide.md 完成Keil5项目配置"
    echo "🚀 配置完成后即可编译和运行"
    exit 0
else
    echo "❌ FreeRTOS集成未完成，请检查上述错误"
    exit 1
fi