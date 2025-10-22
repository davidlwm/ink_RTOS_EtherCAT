#!/bin/bash
# Keil5项目FreeRTOS集成验证脚本

echo "========================================="
echo "Keil5项目FreeRTOS集成验证"
echo "========================================="

PROJECT_FILE="MDK-ARM/YS-F4STD.uvprojx"

echo "1. 检查项目文件存在性..."
if [ -f "$PROJECT_FILE" ]; then
    echo "  ✓ Keil5项目文件存在: $PROJECT_FILE"
else
    echo "  ✗ Keil5项目文件不存在: $PROJECT_FILE"
    exit 1
fi

echo ""
echo "2. 检查FreeRTOS包含路径集成..."

# 检查包含路径是否包含FreeRTOS路径
if grep -q "Middlewares\\\\Third_Party\\\\FreeRTOS\\\\include" "$PROJECT_FILE"; then
    echo "  ✓ FreeRTOS include路径已添加"
else
    echo "  ✗ FreeRTOS include路径缺失"
    exit 1
fi

if grep -q "Middlewares\\\\Third_Party\\\\FreeRTOS\\\\portable\\\\RVDS\\\\ARM_CM4F" "$PROJECT_FILE"; then
    echo "  ✓ FreeRTOS portable路径已添加"
else
    echo "  ✗ FreeRTOS portable路径缺失"
    exit 1
fi

echo ""
echo "3. 检查FreeRTOS源文件组..."

# 检查FreeRTOS/Core组
if grep -q "<GroupName>FreeRTOS/Core</GroupName>" "$PROJECT_FILE"; then
    echo "  ✓ FreeRTOS/Core组已创建"
else
    echo "  ✗ FreeRTOS/Core组缺失"
    exit 1
fi

# 检查FreeRTOS/Portable组
if grep -q "<GroupName>FreeRTOS/Portable</GroupName>" "$PROJECT_FILE"; then
    echo "  ✓ FreeRTOS/Portable组已创建"
else
    echo "  ✗ FreeRTOS/Portable组缺失"
    exit 1
fi

echo ""
echo "4. 检查FreeRTOS核心源文件..."

# 检查核心源文件
freertos_core_files=(
    "tasks.c"
    "queue.c"
    "list.c"
    "timers.c"
    "event_groups.c"
    "stream_buffer.c"
)

for file in "${freertos_core_files[@]}"; do
    if grep -q "<FileName>$file</FileName>" "$PROJECT_FILE"; then
        echo "  ✓ $file 已添加到项目"
    else
        echo "  ✗ $file 缺失"
        exit 1
    fi
done

echo ""
echo "5. 检查FreeRTOS端口文件..."

# 检查端口文件
if grep -q "<FileName>port.c</FileName>" "$PROJECT_FILE"; then
    echo "  ✓ port.c 已添加到项目"
else
    echo "  ✗ port.c 缺失"
    exit 1
fi

if grep -q "<FileName>heap_4.c</FileName>" "$PROJECT_FILE"; then
    echo "  ✓ heap_4.c 已添加到项目"
else
    echo "  ✗ heap_4.c 缺失"
    exit 1
fi

echo ""
echo "6. 检查文件路径..."

# 检查关键文件路径是否正确
if grep -q "..\\\\Middlewares\\\\Third_Party\\\\FreeRTOS\\\\tasks.c" "$PROJECT_FILE"; then
    echo "  ✓ FreeRTOS核心文件路径正确"
else
    echo "  ✗ FreeRTOS核心文件路径错误"
    exit 1
fi

if grep -q "..\\\\Middlewares\\\\Third_Party\\\\FreeRTOS\\\\portable\\\\RVDS\\\\ARM_CM4F\\\\port.c" "$PROJECT_FILE"; then
    echo "  ✓ FreeRTOS端口文件路径正确"
else
    echo "  ✗ FreeRTOS端口文件路径错误"
    exit 1
fi

if grep -q "..\\\\Middlewares\\\\Third_Party\\\\FreeRTOS\\\\portable\\\\MemMang\\\\heap_4.c" "$PROJECT_FILE"; then
    echo "  ✓ FreeRTOS内存管理文件路径正确"
else
    echo "  ✗ FreeRTOS内存管理文件路径错误"
    exit 1
fi

echo ""
echo "7. 检查源文件实际存在..."

# 验证源文件确实存在
freertos_files_to_check=(
    "Middlewares/Third_Party/FreeRTOS/tasks.c"
    "Middlewares/Third_Party/FreeRTOS/queue.c"
    "Middlewares/Third_Party/FreeRTOS/list.c"
    "Middlewares/Third_Party/FreeRTOS/timers.c"
    "Middlewares/Third_Party/FreeRTOS/event_groups.c"
    "Middlewares/Third_Party/FreeRTOS/stream_buffer.c"
    "Middlewares/Third_Party/FreeRTOS/portable/RVDS/ARM_CM4F/port.c"
    "Middlewares/Third_Party/FreeRTOS/portable/MemMang/heap_4.c"
)

all_files_exist=true
for file in "${freertos_files_to_check[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file 文件存在"
    else
        echo "  ✗ $file 文件不存在"
        all_files_exist=false
    fi
done

echo ""
echo "8. 检查头文件..."

freertos_headers=(
    "Middlewares/Third_Party/FreeRTOS/include/FreeRTOS.h"
    "Middlewares/Third_Party/FreeRTOS/include/task.h"
    "Middlewares/Third_Party/FreeRTOS/include/queue.h"
    "Middlewares/Third_Party/FreeRTOS/include/semphr.h"
    "Middlewares/Third_Party/FreeRTOS/portable/RVDS/ARM_CM4F/portmacro.h"
)

for file in "${freertos_headers[@]}"; do
    if [ -f "$file" ]; then
        echo "  ✓ $file 头文件存在"
    else
        echo "  ✗ $file 头文件不存在"
        all_files_exist=false
    fi
done

echo ""
echo "9. 项目配置总结..."

echo "  📁 已添加的文件组:"
echo "     - FreeRTOS/Core: 6个核心源文件"
echo "     - FreeRTOS/Portable: 2个端口文件"
echo ""
echo "  🔧 已更新的配置:"
echo "     - C/C++ Include Paths: 已添加FreeRTOS头文件路径"
echo "     - 预处理器定义: 保持USE_HAL_DRIVER,STM32F407xx"
echo ""
echo "  📝 编译预期:"
echo "     - FreeRTOS.h 应该能够正确包含"
echo "     - 所有FreeRTOS API应该可用"
echo "     - 无符号未定义错误"

echo ""
if $all_files_exist; then
    echo "✅ Keil5项目FreeRTOS集成完成"
    echo "📝 现在可以在Keil5中打开项目并编译"
    echo "🚀 预期无FreeRTOS相关编译错误"
    echo ""
    echo "🔍 下一步操作:"
    echo "   1. 在Keil5中打开 MDK-ARM/YS-F4STD.uvprojx"
    echo "   2. 检查FreeRTOS/Core和FreeRTOS/Portable组是否显示"
    echo "   3. 编译项目验证无错误"
    echo "   4. 如有编译错误，检查路径和配置"
    exit 0
else
    echo "❌ 部分文件缺失，请检查FreeRTOS源码下载"
    exit 1
fi