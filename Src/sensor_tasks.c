/**
 ******************************************************************************
 * @file    sensor_tasks.c
 * @brief   传感器数据采集和主站信号接收RTOS任务实现
 * @author  EtherCAT Development Team
 * @version V1.0.0
 * @date    2025-01-20
 ******************************************************************************
 * @attention
 *
 * 本文件实现了两个核心RTOS任务：
 * 1. Task_SensorDataCollection - 模拟采集传感器数据并上报给主站
 * 2. Task_MasterSignalReceiver - 接收主站发送的控制命令
 *
 * 任务间通过FreeRTOS队列、互斥体和事件组进行通信和同步
 ******************************************************************************
 */

#include "sensor_tasks.h"
#include "sensor_simulator.h"
#include "ethercat_sensor_bridge.h"
#include "app_io_handler.h"
#include "ethercat_output_monitor.h"
#include <string.h>
#include <stdio.h>

/* ========================================================================== */
/* 全局变量定义 */
/* ========================================================================== */

// 任务句柄
TaskHandle_t xTaskHandle_SensorData = NULL;
TaskHandle_t xTaskHandle_MasterSignal = NULL;

// 队列句柄
QueueHandle_t xQueue_SensorData = NULL;
QueueHandle_t xQueue_MasterCommands = NULL;

// 互斥体和信号量
SemaphoreHandle_t xMutex_SensorData = NULL;
SemaphoreHandle_t xMutex_MasterCommands = NULL;

// 事件组
EventGroupHandle_t xEventGroup_SensorTasks = NULL;

// 当前传感器配置
sensor_config_t current_sensor_config = {
    .enabled_digital_inputs = 0xFFFF,       // 默认使能所有数字输入
    .digital_debounce_ms = 10,
    .enabled_analog_inputs = 0xFF,          // 默认使能所有模拟输入
    .analog_sample_rate = 1000,
    .filter_enable = 1,
    .quality_check_enable = 1,
    .min_quality_threshold = DATA_QUALITY_THRESHOLD
};

// 最新的传感器数据和主站命令
sensor_data_t latest_sensor_data = {0};
master_command_t latest_master_command = {0};

// 任务统计信息
static sensor_task_stats_t task_stats = {0};

/* ========================================================================== */
/* 私有变量 */
/* ========================================================================== */

// 数字输入防抖缓冲区
static uint16_t digital_input_history[DIGITAL_DEBOUNCE_COUNT] = {0};
static uint8_t digital_history_index = 0;

// 模拟输入滤波缓冲区
static int16_t analog_filter_buffer[MAX_ANALOG_INPUTS][ANALOG_FILTER_DEPTH] = {0};
static uint8_t analog_filter_index[MAX_ANALOG_INPUTS] = {0};

// 安全状态变量
static uint8_t current_safety_mode = 0;  // 0=正常, 1=安全, 2=紧急停止

// 变化检测相关变量
static uint32_t g_no_change_counter = 0;
static uint32_t g_last_force_update = 0;
static const uint32_t FORCE_UPDATE_INTERVAL = 5000; // 5秒强制更新一次

/* ========================================================================== */
/* 私有函数声明 */
/* ========================================================================== */

static void Process_Digital_Output_Changes(void);
static void Process_Analog_Output_Changes(void);
static void Process_Control_Command_Changes(void);
static void Process_Configuration_Changes(void);

static void Sensor_ProcessDigitalInputs(sensor_data_t *data);
static void Sensor_ProcessAnalogInputs(sensor_data_t *data);
static void Sensor_UpdateFromSimulator(sensor_data_t *data);
static void Master_ProcessCommand(const master_command_t *command);
static uint16_t Sensor_CalculateChecksum(const master_command_t *command);

/* ========================================================================== */
/* 公共函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化传感器任务系统
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t Sensor_Tasks_Init(void)
{
    // 创建队列
    xQueue_SensorData = xQueueCreate(SENSOR_QUEUE_SIZE, sizeof(sensor_data_t));
    if (xQueue_SensorData == NULL) {
        return pdFAIL;
    }

    xQueue_MasterCommands = xQueueCreate(MASTER_COMMAND_QUEUE_SIZE, sizeof(master_command_t));
    if (xQueue_MasterCommands == NULL) {
        return pdFAIL;
    }

    // 创建互斥体
    xMutex_SensorData = xSemaphoreCreateMutex();
    if (xMutex_SensorData == NULL) {
        return pdFAIL;
    }

    xMutex_MasterCommands = xSemaphoreCreateMutex();
    if (xMutex_MasterCommands == NULL) {
        return pdFAIL;
    }

    // 创建事件组
    xEventGroup_SensorTasks = xEventGroupCreate();
    if (xEventGroup_SensorTasks == NULL) {
        return pdFAIL;
    }

    // 初始化统计信息
    memset(&task_stats, 0, sizeof(sensor_task_stats_t));

    printf("Sensor Tasks Init: SUCCESS\r\n");
    return pdPASS;
}

/**
 * @brief 创建传感器相关任务
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t Sensor_Tasks_Create(void)
{
    BaseType_t result;

    // 创建传感器数据采集任务
    result = xTaskCreate(
        Task_SensorDataCollection,
        "SensorData",
        SENSOR_DATA_TASK_STACK_SIZE,
        NULL,
        SENSOR_DATA_TASK_PRIORITY,
        &xTaskHandle_SensorData
    );

    if (result != pdPASS) {
        printf("ERROR: Failed to create SensorData task\r\n");
        return pdFAIL;
    }

    // 创建主站信号接收任务
    result = xTaskCreate(
        Task_MasterSignalReceiver,
        "MasterSignal",
        MASTER_SIGNAL_TASK_STACK_SIZE,
        NULL,
        MASTER_SIGNAL_TASK_PRIORITY,
        &xTaskHandle_MasterSignal
    );

    if (result != pdPASS) {
        printf("ERROR: Failed to create MasterSignal task\r\n");
        return pdFAIL;
    }

    printf("Sensor Tasks Created: 2 tasks\r\n");
    return pdPASS;
}

/**
 * @brief 传感器数据采集任务
 * @param pvParameters 任务参数
 *
 * 此任务负责：
 * 1. 从传感器模拟器读取数据
 * 2. 对数据进行滤波和质量检查
 * 3. 将数据发送到队列供其他任务使用
 * 4. 通过EtherCAT桥接上报数据给主站
 */
void Task_SensorDataCollection(void *pvParameters)
{
    sensor_data_t sensor_data;
    TickType_t xLastWakeTime;
    uint32_t cycle_counter = 0;

    // 初始化延时基准时间
    xLastWakeTime = xTaskGetTickCount();

    printf("Task_SensorDataCollection: Started\r\n");

    for (;;)
    {
        cycle_counter++;
        task_stats.sensor_task_cycles++;

        // 清零数据结构
        memset(&sensor_data, 0, sizeof(sensor_data_t));

        // 1. 从传感器模拟器更新数据
        Sensor_UpdateFromSimulator(&sensor_data);

        // 2. 处理数字输入（防抖）
        Sensor_ProcessDigitalInputs(&sensor_data);

        // 3. 处理模拟输入（滤波）
        Sensor_ProcessAnalogInputs(&sensor_data);

        // 4. 数据质量检查
        sensor_data.overall_data_quality = Sensor_Check_Data_Quality(&sensor_data);

        // 5. 添加时间戳和序列号
        sensor_data.timestamp = xTaskGetTickCount();
        sensor_data.sequence_number = (uint16_t)(cycle_counter & 0xFFFF);
        sensor_data.system_status = (current_safety_mode << 6) | 0x01; // bit0=运行中

        // 6. 配置掩码
        sensor_data.active_digital_mask = current_sensor_config.enabled_digital_inputs;
        sensor_data.active_analog_mask = current_sensor_config.enabled_analog_inputs;

        // 7. 获取互斥体并更新全局变量
        if (xSemaphoreTake(xMutex_SensorData, pdMS_TO_TICKS(10)) == pdTRUE) {
            memcpy(&latest_sensor_data, &sensor_data, sizeof(sensor_data_t));
            xSemaphoreGive(xMutex_SensorData);
        }

        // 8. 发送到队列（非阻塞）
        if (xQueueSend(xQueue_SensorData, &sensor_data, 0) != pdPASS) {
            task_stats.sensor_data_errors++;
        }

        // 9. 通过EtherCAT桥接更新数据到PDO
        EtherCAT_SensorBridge_UpdateInputs();

        // 10. 设置事件标志
        xEventGroupSetBits(xEventGroup_SensorTasks, EVENT_NEW_SENSOR_DATA);

        // 11. 数据质量检查
        if (sensor_data.overall_data_quality < DATA_QUALITY_THRESHOLD) {
            xEventGroupSetBits(xEventGroup_SensorTasks, EVENT_DATA_QUALITY_LOW);
        }

        // 12. 定期打印调试信息
        if ((cycle_counter % 200) == 0) {
            printf("[Sensor] Cycle=%lu, Quality=%d%%, Digital=0x%04X\r\n",
                   cycle_counter,
                   sensor_data.overall_data_quality,
                   sensor_data.digital_sensors);
        }

        // 13. 按照固定周期执行 (5ms)
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SENSOR_DATA_PERIOD_MS));
    }
}

/**
 * @brief 主站信号接收任务 - 集成变化检测优化
 * @param pvParameters 任务参数
 *
 * 此任务负责：
 * 1. 检测主站下发数据是否有变化
 * 2. 只在数据变化时进行处理（优化CPU使用率）
 * 3. 执行相应的输出控制
 * 4. 心跳机制确保可靠性
 */
void Task_MasterSignalReceiver(void *pvParameters)
{
    master_command_t master_command;
    uint32_t cycle_counter = 0;

    printf("Task_MasterSignalReceiver: Started with change detection\r\n");

    // 初始化输出监控模块
    EtherCAT_OutputMonitor_Init();
    g_last_force_update = HAL_GetTick();

    for (;;)
    {
        // 1. 检查主站下发数据是否有变化
        uint8_t changes = EtherCAT_OutputMonitor_CheckChanges();

        // 2. 检查是否需要强制更新（心跳机制）
        bool force_update = EtherCAT_OutputMonitor_NeedForceUpdate(FORCE_UPDATE_INTERVAL);

        // 3. 只有在数据变化或需要强制更新时才处理
        if (changes != OUTPUT_CHANGE_NONE || force_update) {

            cycle_counter++;
            task_stats.master_task_cycles++;

            if (force_update && changes == OUTPUT_CHANGE_NONE) {
                //printf("[Master] 强制心跳更新 (静默 %lu 秒)\r\n",
                //       (HAL_GetTick() - g_last_force_update) / 1000);
                g_last_force_update = HAL_GetTick();
            }

            // 4. 根据变化类型进行不同处理
            if (changes & OUTPUT_CHANGE_DIGITAL) {
                Process_Digital_Output_Changes();
            }

            if (changes & OUTPUT_CHANGE_ANALOG) {
                Process_Analog_Output_Changes();
            }

            if (changes & OUTPUT_CHANGE_COMMAND) {
                Process_Control_Command_Changes();
            }

            if (changes & OUTPUT_CHANGE_CONFIG) {
                Process_Configuration_Changes();
            }

            // 5. 更新输出数据缓存
            EtherCAT_OutputMonitor_UpdateCache(force_update);

            // 6. 设置事件标志
            xEventGroupSetBits(xEventGroup_SensorTasks, EVENT_MASTER_COMMAND);

            task_stats.commands_executed++;
            g_no_change_counter = 0;

            // 7. 调试输出（减少频率）
            if ((cycle_counter % 10) == 0) {
                printf("[Master] 变化类型: 0x%02X, 周期: %lu\r\n", changes, cycle_counter);
            }

        } else {
            // 无变化，跳过处理
            g_no_change_counter++;

            // 每1000次无变化时输出一次统计
            if ((g_no_change_counter % 1000) == 0) {
                printf("[Master] 无变化跳过: %lu 次\r\n", g_no_change_counter);
            }
        }

        // 8. 从队列检查是否有新命令（非阻塞）
        if (xQueueReceive(xQueue_MasterCommands, &master_command, pdMS_TO_TICKS(5)) == pdPASS)
        {
            task_stats.commands_received++;

            // 验证并处理队列命令
            if (Sensor_Validate_Master_Command(&master_command) == pdTRUE) {
                Master_ProcessCommand(&master_command);

                // 更新全局变量
                if (xSemaphoreTake(xMutex_MasterCommands, pdMS_TO_TICKS(10)) == pdTRUE) {
                    memcpy(&latest_master_command, &master_command, sizeof(master_command_t));
                    xSemaphoreGive(xMutex_MasterCommands);
                }
            } else {
                task_stats.command_errors++;
                printf("[Master] ERROR: Invalid command ID=%lu\r\n", master_command.command_id);
            }
        }

        // 9. 周期性统计输出
        if ((cycle_counter % 2000) == 0 && cycle_counter > 0) {
            EtherCAT_OutputMonitor_PrintStats();
        }

        // 10. 任务延时 - 根据系统负载调整
        vTaskDelay(pdMS_TO_TICKS(changes != OUTPUT_CHANGE_NONE ? 5 : 20));
    }
}

/**
 * @brief 获取最新传感器数据
 * @param data 数据结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t Sensor_Get_Latest_Data(sensor_data_t *data)
{
    if (data == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_SensorData, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(data, &latest_sensor_data, sizeof(sensor_data_t));
        xSemaphoreGive(xMutex_SensorData);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 发送主站命令到队列
 * @param command 命令结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t Sensor_Send_Master_Command(const master_command_t *command, uint32_t timeout_ms)
{
    if (command == NULL) {
        return pdFALSE;
    }

    return xQueueSend(xQueue_MasterCommands, command, pdMS_TO_TICKS(timeout_ms));
}

/**
 * @brief 配置传感器参数
 * @param config 配置结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t Sensor_Configure(const sensor_config_t *config)
{
    if (config == NULL) {
        return pdFALSE;
    }

    memcpy(&current_sensor_config, config, sizeof(sensor_config_t));
    printf("Sensor Config Updated\r\n");
    return pdTRUE;
}

/**
 * @brief 获取传感器任务统计信息
 * @param stats 统计信息结构指针
 */
void Sensor_Get_Task_Statistics(sensor_task_stats_t *stats)
{
    if (stats != NULL) {
        memcpy(stats, &task_stats, sizeof(sensor_task_stats_t));
    }
}

/**
 * @brief 重置传感器任务统计
 */
void Sensor_Reset_Statistics(void)
{
    memset(&task_stats, 0, sizeof(sensor_task_stats_t));
    printf("Sensor Statistics Reset\r\n");
}

/**
 * @brief 执行传感器数据质量检查
 * @param data 传感器数据指针
 * @return 整体质量分数 (0-100)
 */
uint8_t Sensor_Check_Data_Quality(sensor_data_t *data)
{
    if (data == NULL) {
        return 0;
    }

    uint32_t total_quality = 0;
    uint8_t active_channels = 0;

    // 检查模拟输入质量
    for (uint8_t i = 0; i < MAX_ANALOG_INPUTS; i++) {
        if (current_sensor_config.enabled_analog_inputs & (1 << i)) {
            total_quality += data->analog_quality[i];
            active_channels++;
        }
    }

    // 计算平均质量
    if (active_channels > 0) {
        data->overall_data_quality = (uint8_t)(total_quality / active_channels);
    } else {
        data->overall_data_quality = 100;  // 无激活通道，默认100%
    }

    return data->overall_data_quality;
}

/**
 * @brief 验证主站命令
 * @param command 命令结构指针
 * @return pdTRUE=有效, pdFALSE=无效
 */
BaseType_t Sensor_Validate_Master_Command(const master_command_t *command)
{
    if (command == NULL) {
        return pdFALSE;
    }

    // 1. 检查校验和
    uint16_t calculated_checksum = Sensor_CalculateChecksum(command);
    if (calculated_checksum != command->checksum) {
        return pdFALSE;
    }

    // 2. 检查控制模式范围
    if (command->control_mode > 2) {
        return pdFALSE;
    }

    // 3. 检查安全状态范围
    if (command->safety_state > 2) {
        return pdFALSE;
    }

    // 4. 检查时间戳（简单的超时检查）
    uint32_t current_time = xTaskGetTickCount();
    uint32_t age = current_time - command->timestamp;
    if (age > COMMAND_TIMEOUT_MS) {
        return pdFALSE;
    }

    return pdTRUE;
}

/**
 * @brief 处理系统错误事件
 * @param error_code 错误代码
 */
void Sensor_Handle_System_Error(uint32_t error_code)
{
    xEventGroupSetBits(xEventGroup_SensorTasks, EVENT_SYSTEM_ERROR);
    printf("SYSTEM ERROR: Code=0x%08lX\r\n", error_code);

    // 进入安全模式
    Sensor_Set_Safety_Mode(2);
}

/**
 * @brief 设置传感器安全模式
 * @param safety_mode 安全模式 (0=正常, 1=安全, 2=紧急停止)
 */
void Sensor_Set_Safety_Mode(uint8_t safety_mode)
{
    if (safety_mode > 2) {
        safety_mode = 2;
    }

    current_safety_mode = safety_mode;

    // 执行安全输出
    Sensor_Execute_Safety_Outputs(safety_mode);

    printf("Safety Mode Changed: %d\r\n", safety_mode);
}

/**
 * @brief 获取传感器任务状态
 * @return 状态位掩码
 */
uint32_t Sensor_Get_Task_Status(void)
{
    uint32_t status = 0;

    if (xTaskHandle_SensorData != NULL) {
        status |= (1 << 0);  // 传感器任务已创建
    }

    if (xTaskHandle_MasterSignal != NULL) {
        status |= (1 << 1);  // 主站信号任务已创建
    }

    status |= (current_safety_mode << 8);  // 安全模式

    return status;
}

/* ========================================================================== */
/* 内部函数实现 */
/* ========================================================================== */

/**
 * @brief 数字输入滤波和防抖处理
 * @param raw_inputs 原始输入值
 * @return 滤波后的输入值
 */
uint16_t Sensor_Filter_Digital_Inputs(uint16_t raw_inputs)
{
    // 存储到历史缓冲区
    digital_input_history[digital_history_index] = raw_inputs;
    digital_history_index = (digital_history_index + 1) % DIGITAL_DEBOUNCE_COUNT;

    // 多数表决滤波
    uint16_t filtered = 0;
    for (uint8_t bit = 0; bit < 16; bit++) {
        uint8_t count = 0;
        for (uint8_t i = 0; i < DIGITAL_DEBOUNCE_COUNT; i++) {
            if (digital_input_history[i] & (1 << bit)) {
                count++;
            }
        }
        // 超过半数则认为该位有效
        if (count > (DIGITAL_DEBOUNCE_COUNT / 2)) {
            filtered |= (1 << bit);
        }
    }

    return filtered;
}

/**
 * @brief 模拟输入滤波处理
 * @param channel 通道号
 * @param raw_value 原始值
 * @return 滤波后的值
 */
int16_t Sensor_Filter_Analog_Input(uint8_t channel, int16_t raw_value)
{
    if (channel >= MAX_ANALOG_INPUTS) {
        return raw_value;
    }

    // 存储到滤波缓冲区
    analog_filter_buffer[channel][analog_filter_index[channel]] = raw_value;
    analog_filter_index[channel] = (analog_filter_index[channel] + 1) % ANALOG_FILTER_DEPTH;

    // 计算移动平均值
    int32_t sum = 0;
    for (uint8_t i = 0; i < ANALOG_FILTER_DEPTH; i++) {
        sum += analog_filter_buffer[channel][i];
    }

    return (int16_t)(sum / ANALOG_FILTER_DEPTH);
}

/**
 * @brief 计算模拟输入质量分数
 * @param channel 通道号
 * @param current_value 当前值
 * @param previous_value 前一个值
 * @return 质量分数 (0-100)
 */
uint8_t Sensor_Calculate_Analog_Quality(uint8_t channel, int16_t current_value, int16_t previous_value)
{
    (void)channel;

    // 基于值的变化率计算质量
    int16_t delta = current_value - previous_value;
    if (delta < 0) delta = -delta;

    // 变化越小质量越高
    if (delta < 10) {
        return 100;
    } else if (delta < 50) {
        return 95;
    } else if (delta < 100) {
        return 90;
    } else if (delta < 500) {
        return 80;
    } else {
        return 70;
    }
}

/**
 * @brief 执行安全输出控制
 * @param safety_state 安全状态
 */
void Sensor_Execute_Safety_Outputs(uint8_t safety_state)
{
    switch (safety_state) {
        case 0:  // 正常模式 - 无特殊动作
            break;

        case 1:  // 安全模式 - 关闭部分输出
            // 可以通过IO Handler关闭某些输出
            break;

        case 2:  // 紧急停止 - 关闭所有输出
            // 关闭所有数字输出
            for (uint8_t i = 0; i < MAX_DIGITAL_OUTPUTS; i++) {
                App_Set_Digital_Output(i, 0);
            }
            // 清零所有模拟输出
            for (uint8_t i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
                App_Set_Analog_Output(i, 0);
            }
            break;

        default:
            break;
    }
}

/**
 * @brief 处理数字输入
 * @param data 传感器数据指针
 */
static void Sensor_ProcessDigitalInputs(sensor_data_t *data)
{
    // 读取原始数字输入（从IO Handler或模拟器）
    uint16_t raw_digital = 0;
    for (uint8_t i = 0; i < MAX_DIGITAL_INPUTS; i++) {
        if (App_Get_Digital_Input(i)) {
            raw_digital |= (1 << i);
        }
    }

    // 应用防抖滤波
    if (current_sensor_config.filter_enable) {
        data->digital_sensors = Sensor_Filter_Digital_Inputs(raw_digital);
    } else {
        data->digital_sensors = raw_digital;
    }

    // 设置质量标志（所有位默认有效）
    data->digital_quality_flags = 0xFF;
}

/**
 * @brief 处理模拟输入
 * @param data 传感器数据指针
 */
static void Sensor_ProcessAnalogInputs(sensor_data_t *data)
{
    static int16_t previous_values[MAX_ANALOG_INPUTS] = {0};

    for (uint8_t i = 0; i < MAX_ANALOG_INPUTS; i++) {
        // 读取原始模拟输入
        int16_t raw_value = App_Get_Analog_Input(i);
        data->analog_raw[i] = raw_value;

        // 应用滤波
        if (current_sensor_config.filter_enable) {
            data->analog_sensors[i] = Sensor_Filter_Analog_Input(i, raw_value);
        } else {
            data->analog_sensors[i] = raw_value;
        }

        // 计算质量分数
        data->analog_quality[i] = Sensor_Calculate_Analog_Quality(
            i,
            data->analog_sensors[i],
            previous_values[i]
        );

        // 更新历史值
        previous_values[i] = data->analog_sensors[i];
    }
}

/**
 * @brief 从传感器模拟器更新数据
 * @param data 传感器数据指针
 */
static void Sensor_UpdateFromSimulator(sensor_data_t *data)
{
    // 更新传感器模拟器
    SensorSimulator_Update();

    // 获取模拟器数据
    const SensorData_t *sim_data = SensorSimulator_GetData();
    if (sim_data != NULL) {
        // 可以选择性地使用模拟器数据覆盖或混合实际硬件数据
        // 这里作为示例，我们不直接使用，而是让ProcessDigitalInputs/ProcessAnalogInputs处理
    }
}

/**
 * @brief 处理主站命令
 * @param command 命令结构指针
 */
static void Master_ProcessCommand(const master_command_t *command)
{
    // 1. 处理安全状态
    if (command->safety_state != current_safety_mode) {
        Sensor_Set_Safety_Mode(command->safety_state);
    }

    // 2. 根据控制模式处理命令
    switch (command->control_mode) {
        case 0:  // 手动模式
            // 直接应用输出
            for (uint8_t i = 0; i < MAX_DIGITAL_OUTPUTS; i++) {
                if (command->digital_output_mask & (1 << i)) {
                    uint8_t value = (command->digital_outputs & (1 << i)) ? 1 : 0;
                    App_Set_Digital_Output(i, value);
                }
            }

            for (uint8_t i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
                if (command->analog_output_mask & (1 << i)) {
                    App_Set_Analog_Output(i, command->analog_outputs[i]);
                }
            }
            break;

        case 1:  // 自动模式
            // 可以实现自动控制逻辑
            break;

        case 2:  // 安全模式
            Sensor_Execute_Safety_Outputs(2);
            break;

        default:
            break;
    }
}

/**
 * @brief 计算命令校验和
 * @param command 命令结构指针
 * @return 校验和
 */
static uint16_t Sensor_CalculateChecksum(const master_command_t *command)
{
    uint16_t checksum = 0;
    const uint8_t *data = (const uint8_t *)command;

    // 计算除校验和字段外的所有字节
    size_t len = sizeof(master_command_t) - sizeof(uint16_t);
    for (size_t i = 0; i < len; i++) {
        checksum += data[i];
    }

    return checksum;
}

/* ========================================================================== */
/* 变化检测处理函数实现 */
/* ========================================================================== */

/**
 * @brief 处理数字输出变化
 */
static void Process_Digital_Output_Changes(void)
{
    //printf("[Master] 处理数字输出变化: 0x%04X (掩码: 0x%04X)\r\n",
    //       Obj0x7011.digital_outputs, Obj0x7011.digital_output_mask);

    // 应用数字输出到硬件
    for (uint8_t i = 0; i < MAX_DIGITAL_OUTPUTS; i++) {
        // if (Obj0x7011.digital_output_mask & (1 << i)) {
        //     uint8_t value = (Obj0x7011.digital_outputs & (1 << i)) ? 1 : 0;
        //     App_Set_Digital_Output(i, value);
        // }
    }

    // 更新统计
    task_stats.commands_executed++;
}

/**
 * @brief 处理模拟输出变化
 */
static void Process_Analog_Output_Changes(void)
{
    printf("[Master] 处理模拟输出变化\r\n");

    // 应用模拟输出到硬件
    for (uint8_t i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
        // if (Obj0x7012.analog_output_mask & (1 << i)) {
        //     App_Set_Analog_Output(i, Obj0x7012.channel[i]);
        // }
    }

    // 更新统计
    task_stats.commands_executed++;
}

/**
 * @brief 处理控制命令变化
 */
static void Process_Control_Command_Changes(void)
{
    //printf("[Master] 处理控制命令变化: 传感器=%d, 系统=%d\r\n",
    //       Obj0x7020.sensor_config_cmd, Obj0x7020.system_control_cmd);

    // 处理传感器配置命令
    // switch (Obj0x7020.sensor_config_cmd) {
    //     case SENSOR_CMD_RESET:
    //         SensorSimulator_Reset();
    //         printf("[Master] 执行传感器复位\r\n");
    //         break;

    //     case SENSOR_CMD_CALIBRATE:
    //         // 执行校准 - 调用传感器桥接模块的校准功能
    //         for (uint8_t i = 0; i < 7; i++) {
    //             EtherCAT_SensorBridge_CalibrateSensor(i);
    //         }
    //         printf("[Master] 执行传感器校准\r\n");
    //         break;

    //     case SENSOR_CMD_INJECT_FAULT:
    //         SensorSimulator_InjectFault(0, SENSOR_STATUS_ERROR);
    //         printf("[Master] 注入传感器故障\r\n");
    //         break;

    //     default:
    //         break;
    // }

    // 处理系统控制命令
    // switch (Obj0x7020.system_control_cmd) {
    //     case 1:  // 正常运行
    //         current_safety_mode = 0;
    //         printf("[Master] 切换到正常运行模式\r\n");
    //         break;

    //     case 2:  // 紧急停止
    //         current_safety_mode = 2;
    //         printf("[Master] 执行紧急停止\r\n");
    //         // 关闭所有输出
    //         for (uint8_t i = 0; i < MAX_DIGITAL_OUTPUTS; i++) {
    //             App_Set_Digital_Output(i, 0);
    //         }
    //         for (uint8_t i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
    //             App_Set_Analog_Output(i, 0);
    //         }
    //         break;

    //     default:
    //         break;
    // }
}

/**
 * @brief 处理配置参数变化
 */
static void Process_Configuration_Changes(void)
{
    // printf("[Master] 处理配置变化: 采样率=%d, 滤波=%d\r\n",
    //        Obj0x7030.sampling_rate, Obj0x7030.filter_enable);

    // 更新传感器配置
    // if (Obj0x7030.sampling_rate > 0 && Obj0x7030.sampling_rate <= 1000) {
    //     current_sensor_config.sampling_interval_ms = 1000 / Obj0x7030.sampling_rate;
    // }

    // current_sensor_config.filter_enable = Obj0x7030.filter_enable;

    // printf("[Master] 配置已更新: 采样间隔=%lu ms, 滤波=%s\r\n",
    //        current_sensor_config.sampling_interval_ms,
    //        current_sensor_config.filter_enable ? "开启" : "关闭");
}

/************************ (C) COPYRIGHT EtherCAT Development Team *****END OF FILE****/
