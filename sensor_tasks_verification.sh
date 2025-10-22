#!/bin/bash
################################################################################
# 传感器任务集成验证脚本
# 用于验证sensor_tasks的实现和Keil工程集成
################################################################################

echo "=========================================="
echo "传感器任务集成验证"
echo "=========================================="
echo ""

PASS_COUNT=0
FAIL_COUNT=0

# 辅助函数
pass_test() {
    echo "[PASS] $1"
    ((PASS_COUNT++))
}

fail_test() {
    echo "[FAIL] $1"
    ((FAIL_COUNT++))
}

info() {
    echo "[INFO] $1"
}

# 1. 检查头文件
echo "1. 检查头文件..."
if [ -f "Inc/sensor_tasks.h" ]; then
    pass_test "sensor_tasks.h 存在"
else
    fail_test "sensor_tasks.h 不存在"
fi

# 2. 检查实现文件
echo ""
echo "2. 检查实现文件..."
if [ -f "Src/sensor_tasks.c" ]; then
    pass_test "sensor_tasks.c 存在"

    # 检查文件大小
    SIZE=$(wc -c < "Src/sensor_tasks.c")
    if [ $SIZE -gt 1000 ]; then
        pass_test "sensor_tasks.c 文件大小正常 (${SIZE} bytes)"
    else
        fail_test "sensor_tasks.c 文件太小 (${SIZE} bytes)"
    fi
else
    fail_test "sensor_tasks.c 不存在"
fi

# 3. 检查关键函数定义
echo ""
echo "3. 检查关键函数定义..."

if grep -q "Task_SensorDataCollection" Src/sensor_tasks.c; then
    pass_test "找到 Task_SensorDataCollection 函数"
else
    fail_test "未找到 Task_SensorDataCollection 函数"
fi

if grep -q "Task_MasterSignalReceiver" Src/sensor_tasks.c; then
    pass_test "找到 Task_MasterSignalReceiver 函数"
else
    fail_test "未找到 Task_MasterSignalReceiver 函数"
fi

if grep -q "Sensor_Tasks_Init" Src/sensor_tasks.c; then
    pass_test "找到 Sensor_Tasks_Init 函数"
else
    fail_test "未找到 Sensor_Tasks_Init 函数"
fi

if grep -q "Sensor_Tasks_Create" Src/sensor_tasks.c; then
    pass_test "找到 Sensor_Tasks_Create 函数"
else
    fail_test "未找到 Sensor_Tasks_Create 函数"
fi

# 4. 检查main.c集成
echo ""
echo "4. 检查main.c集成..."

if grep -q "#include \"sensor_tasks.h\"" Src/main.c; then
    pass_test "main.c 包含 sensor_tasks.h"
else
    fail_test "main.c 未包含 sensor_tasks.h"
fi

if grep -q "Sensor_Tasks_Init" Src/main.c; then
    pass_test "main.c 调用 Sensor_Tasks_Init"
else
    fail_test "main.c 未调用 Sensor_Tasks_Init"
fi

if grep -q "Sensor_Tasks_Create" Src/main.c; then
    pass_test "main.c 调用 Sensor_Tasks_Create"
else
    fail_test "main.c 未调用 Sensor_Tasks_Create"
fi

# 5. 检查Keil工程文件
echo ""
echo "5. 检查Keil工程文件..."

if [ -f "MDK-ARM/YS-F4STD.uvprojx" ]; then
    pass_test "Keil工程文件存在"

    if grep -q "sensor_tasks.c" MDK-ARM/YS-F4STD.uvprojx; then
        pass_test "sensor_tasks.c 已添加到Keil工程"
    else
        fail_test "sensor_tasks.c 未添加到Keil工程"
    fi

    if grep -q "sensor_simulator.c" MDK-ARM/YS-F4STD.uvprojx; then
        pass_test "sensor_simulator.c 在Keil工程中"
    else
        fail_test "sensor_simulator.c 不在Keil工程中"
    fi

    if grep -q "ethercat_sensor_bridge.c" MDK-ARM/YS-F4STD.uvprojx; then
        pass_test "ethercat_sensor_bridge.c 在Keil工程中"
    else
        fail_test "ethercat_sensor_bridge.c 不在Keil工程中"
    fi

    if grep -q "app_io_handler.c" MDK-ARM/YS-F4STD.uvprojx; then
        pass_test "app_io_handler.c 在Keil工程中"
    else
        fail_test "app_io_handler.c 不在Keil工程中"
    fi
else
    fail_test "Keil工程文件不存在"
fi

# 6. 检查依赖的头文件
echo ""
echo "6. 检查依赖的头文件..."

REQUIRED_HEADERS=(
    "Inc/sensor_simulator.h"
    "Inc/ethercat_sensor_bridge.h"
    "Inc/app_io_handler.h"
    "Inc/FreeRTOSConfig.h"
)

for header in "${REQUIRED_HEADERS[@]}"; do
    if [ -f "$header" ]; then
        pass_test "$header 存在"
    else
        fail_test "$header 不存在"
    fi
done

# 7. 检查FreeRTOS相关定义
echo ""
echo "7. 检查FreeRTOS相关定义..."

if grep -q "xTaskCreate" Src/sensor_tasks.c; then
    pass_test "使用 xTaskCreate 创建任务"
else
    fail_test "未找到 xTaskCreate"
fi

if grep -q "xQueueCreate" Src/sensor_tasks.c; then
    pass_test "使用 xQueueCreate 创建队列"
else
    fail_test "未找到 xQueueCreate"
fi

if grep -q "xSemaphoreCreateMutex" Src/sensor_tasks.c; then
    pass_test "使用 xSemaphoreCreateMutex 创建互斥体"
else
    fail_test "未找到 xSemaphoreCreateMutex"
fi

if grep -q "xEventGroupCreate" Src/sensor_tasks.c; then
    pass_test "使用 xEventGroupCreate 创建事件组"
else
    fail_test "未找到 xEventGroupCreate"
fi

# 8. 检查数据结构定义
echo ""
echo "8. 检查数据结构定义..."

if grep -q "sensor_data_t" Inc/sensor_tasks.h; then
    pass_test "定义了 sensor_data_t 结构"
else
    fail_test "未定义 sensor_data_t 结构"
fi

if grep -q "master_command_t" Inc/sensor_tasks.h; then
    pass_test "定义了 master_command_t 结构"
else
    fail_test "未定义 master_command_t 结构"
fi

if grep -q "sensor_config_t" Inc/sensor_tasks.h; then
    pass_test "定义了 sensor_config_t 结构"
else
    fail_test "未定义 sensor_config_t 结构"
fi

# 9. 检查任务优先级配置
echo ""
echo "9. 检查任务优先级配置..."

if grep -q "SENSOR_DATA_TASK_PRIORITY" Inc/sensor_tasks.h; then
    pass_test "定义了 SENSOR_DATA_TASK_PRIORITY"
else
    fail_test "未定义 SENSOR_DATA_TASK_PRIORITY"
fi

if grep -q "MASTER_SIGNAL_TASK_PRIORITY" Inc/sensor_tasks.h; then
    pass_test "定义了 MASTER_SIGNAL_TASK_PRIORITY"
else
    fail_test "未定义 MASTER_SIGNAL_TASK_PRIORITY"
fi

# 10. 代码质量检查
echo ""
echo "10. 代码质量检查..."

# 检查注释
COMMENT_COUNT=$(grep -c "/\*\|//" Src/sensor_tasks.c || echo 0)
if [ $COMMENT_COUNT -gt 50 ]; then
    pass_test "代码包含充足的注释 ($COMMENT_COUNT 行)"
else
    fail_test "代码注释不足 ($COMMENT_COUNT 行)"
fi

# 检查错误处理
if grep -q "pdFAIL" Src/sensor_tasks.c; then
    pass_test "包含错误处理代码"
else
    fail_test "缺少错误处理代码"
fi

# 检查调试输出
if grep -q "printf" Src/sensor_tasks.c; then
    pass_test "包含调试输出代码"
else
    info "未使用printf调试输出（可选）"
fi

# 总结
echo ""
echo "=========================================="
echo "验证总结"
echo "=========================================="
echo "通过: $PASS_COUNT"
echo "失败: $FAIL_COUNT"
echo "总计: $((PASS_COUNT + FAIL_COUNT))"
echo ""

if [ $FAIL_COUNT -eq 0 ]; then
    echo "✓ 所有验证通过！传感器任务已成功集成到Keil工程。"
    echo ""
    echo "后续步骤:"
    echo "1. 在Keil MDK中打开工程 MDK-ARM/YS-F4STD.uvprojx"
    echo "2. 编译工程 (F7)"
    echo "3. 确保没有编译错误"
    echo "4. 下载程序到目标板"
    echo "5. 运行并观察传感器任务输出"
    echo ""
    exit 0
else
    echo "✗ 发现 $FAIL_COUNT 个问题，请检查并修复。"
    exit 1
fi
