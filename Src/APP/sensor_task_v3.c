/**
 ******************************************************************************
 * @file    sensor_task_v3.c
 * @brief   传感器任务实现 - 基于V3设计文档
 * @author  Ink Supply Control System Development Team
 * @version V3.0.0
 * @date    2025-01-23
 ******************************************************************************
 * @attention
 *
 * 本文件基于墨路控制系统详细设计文档V3实现，采用四层架构设计
 *
 * 主要功能:
 * 1. 周期性采集传感器数据 (50ms周期)
 * 2. 数据滤波和质量检查
 * 3. 通过消息队列发送数据到控制任务和通信任务
 * 4. 支持传感器校准和配置
 * 5. 提供统计信息和健康监控
 ******************************************************************************
 */

#include "sensor_task_v3.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ========================================================================== */
/* 私有宏定义 */
/* ========================================================================== */

#define MAX_FILTER_SAMPLES      8       // 滤波缓冲区深度
#define SENSOR_TIMEOUT_MS       100     // 传感器读取超时
#define CALIBRATION_SAMPLES     10      // 校准采样次数
#define QUALITY_THRESHOLD       80      // 质量阈值

/* ========================================================================== */
/* 全局变量定义 */
/* ========================================================================== */

// 任务句柄
TaskHandle_t xTaskHandle_SensorV3 = NULL;

// 队列句柄
QueueHandle_t xQueue_SensorMsg = NULL;

// 互斥体
SemaphoreHandle_t xMutex_SensorContext = NULL;

// 事件组
EventGroupHandle_t xEventGroup_Sensor = NULL;

/* ========================================================================== */
/* 私有变量 */
/* ========================================================================== */

// 传感器上下文
static sensor_context_t g_sensor_context = {0};

// 传感器配置
static sensor_config_t g_sensor_configs[SENSOR_COUNT] = {0};

// 统计信息
static sensor_task_stats_t g_sensor_stats = {0};

// 滤波缓冲区
static float g_filter_buffer[SENSOR_COUNT][MAX_FILTER_SAMPLES] = {0};
static uint8_t g_filter_index[SENSOR_COUNT] = {0};
static uint8_t g_filter_count[SENSOR_COUNT] = {0};

/* ========================================================================== */
/* 私有函数声明 */
/* ========================================================================== */

static void Sensor_InitializeConfigs(void);
static void Sensor_ReadAllSensors(void);
static void Sensor_ReadTemperatureSensors(void);
static void Sensor_ReadPressureSensors(void);
static void Sensor_ReadLevelSensors(void);
static void Sensor_ReadFlowSensor(void);
static float Sensor_ApplyFilter(sensor_type_t sensor_type, float raw_value);
static float Sensor_ApplyCalibration(sensor_type_t sensor_type, float filtered_value);
static uint8_t Sensor_CalculateQuality(sensor_type_t sensor_type);
static void Sensor_UpdateContext(void);
static void Sensor_CheckSystemHealth(void);

/* ========================================================================== */
/* 公共函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化传感器任务系统
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t SensorTaskV3_Init(void)
{
    // 创建互斥体
    xMutex_SensorContext = xSemaphoreCreateMutex();
    if (xMutex_SensorContext == NULL) {
        printf("[SensorV3] ERROR: Failed to create mutex\r\n");
        return pdFAIL;
    }

    // 创建消息队列
    xQueue_SensorMsg = xQueueCreate(SENSOR_MSG_QUEUE_SIZE, sizeof(sensor_msg_t));
    if (xQueue_SensorMsg == NULL) {
        printf("[SensorV3] ERROR: Failed to create message queue\r\n");
        return pdFAIL;
    }

    // 创建事件组
    xEventGroup_Sensor = xEventGroupCreate();
    if (xEventGroup_Sensor == NULL) {
        printf("[SensorV3] ERROR: Failed to create event group\r\n");
        return pdFAIL;
    }

    // 初始化传感器配置
    Sensor_InitializeConfigs();

    // 初始化上下文
    memset(&g_sensor_context, 0, sizeof(sensor_context_t));
    g_sensor_context.system_ready = false;

    // 初始化统计信息
    memset(&g_sensor_stats, 0, sizeof(sensor_task_stats_t));

    printf("[SensorV3] Initialization SUCCESS\r\n");
    return pdPASS;
}

/**
 * @brief 创建传感器任务
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t SensorTaskV3_Create(void)
{
    BaseType_t result;

    result = xTaskCreate(
        Task_SensorV3,
        "SensorV3",
        SENSOR_TASK_STACK_SIZE,
        NULL,
        SENSOR_TASK_PRIORITY,
        &xTaskHandle_SensorV3
    );

    if (result != pdPASS) {
        printf("[SensorV3] ERROR: Failed to create task\r\n");
        return pdFAIL;
    }

    printf("[SensorV3] Task Created Successfully\r\n");
    return pdPASS;
}

/**
 * @brief 传感器任务主函数
 * @param pvParameters 任务参数
 */
void Task_SensorV3(void *pvParameters)
{
    TickType_t xLastWakeTime;
    uint32_t cycle_start_time, cycle_end_time;
    sensor_msg_t sensor_msg;

    // 初始化延时基准时间
    xLastWakeTime = xTaskGetTickCount();

    printf("[SensorV3] Task Started - Period: %d ms\r\n", SENSOR_TASK_PERIOD_MS);

    // 等待系统初始化完成
    vTaskDelay(pdMS_TO_TICKS(100));

    for (;;)
    {
        // 记录周期开始时间
        cycle_start_time = HAL_GetTick();

        // 1. 读取所有传感器数据
        Sensor_ReadAllSensors();

        // 2. 更新上下文数据
        Sensor_UpdateContext();

        // 3. 检查系统健康状态
        Sensor_CheckSystemHealth();

        // 4. 准备消息并发送到队列
        if (g_sensor_context.system_ready) {
            sensor_msg.type = MSG_SENSOR_DATA;
            sensor_msg.timestamp = HAL_GetTick();
            sensor_msg.data_len = sizeof(sensor_context_t);

            // 获取互斥体并复制上下文
            if (xSemaphoreTake(xMutex_SensorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
                memcpy(&sensor_msg.context, &g_sensor_context, sizeof(sensor_context_t));
                xSemaphoreGive(xMutex_SensorContext);

                // 发送消息到队列
                if (xQueueSend(xQueue_SensorMsg, &sensor_msg, 0) != pdPASS) {
                    g_sensor_stats.queue_full_count++;
                }

                // 设置事件标志
                xEventGroupSetBits(xEventGroup_Sensor, EVENT_SENSOR_DATA_READY);
            }
        }

        // 5. 更新统计信息
        cycle_end_time = HAL_GetTick();
        uint32_t cycle_time_us = (cycle_end_time - cycle_start_time) * 1000;

        g_sensor_stats.total_cycles++;
        if (cycle_time_us > g_sensor_stats.max_cycle_time_us) {
            g_sensor_stats.max_cycle_time_us = (uint16_t)cycle_time_us;
        }

        // 计算平均循环时间
        g_sensor_stats.avg_cycle_time_us = (uint16_t)(
            (g_sensor_stats.avg_cycle_time_us * (g_sensor_stats.total_cycles - 1) + cycle_time_us) /
            g_sensor_stats.total_cycles
        );

        // 6. 定期打印调试信息
        if ((g_sensor_context.cycle_count % 100) == 0) {
            printf("[SensorV3] Cycle=%lu, Quality=%d%%, Temp1=%.1f°C, Press1=%.1fkPa, Level1=%.1fmm\r\n",
                   g_sensor_context.cycle_count,
                   g_sensor_context.overall_quality,
                   g_sensor_context.temp_values[0],
                   g_sensor_context.pressure_values[0],
                   g_sensor_context.level_values[0]);
        }

        // 7. 按照固定周期执行
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS));
    }
}

/**
 * @brief 获取传感器上下文数据
 * @param context 上下文结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_GetContext(sensor_context_t *context)
{
    if (context == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_SensorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(context, &g_sensor_context, sizeof(sensor_context_t));
        xSemaphoreGive(xMutex_SensorContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 配置单个传感器
 * @param sensor_type 传感器类型
 * @param config 配置结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_ConfigureSensor(sensor_type_t sensor_type, const sensor_config_t *config)
{
    if (sensor_type >= SENSOR_COUNT || config == NULL) {
        return pdFALSE;
    }

    memcpy(&g_sensor_configs[sensor_type], config, sizeof(sensor_config_t));

    printf("[SensorV3] Sensor %d configured: scale=%.3f, offset=%.3f\r\n",
           sensor_type, config->scale_factor, config->offset);

    return pdTRUE;
}

/**
 * @brief 获取单个传感器数据
 * @param sensor_type 传感器类型
 * @param data 数据结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_GetSensorData(sensor_type_t sensor_type, sensor_data_t *data)
{
    if (sensor_type >= SENSOR_COUNT || data == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_SensorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(data, &g_sensor_context.sensors[sensor_type], sizeof(sensor_data_t));
        xSemaphoreGive(xMutex_SensorContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 校准传感器
 * @param sensor_type 传感器类型
 * @param reference_value 参考值
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_CalibrateSensor(sensor_type_t sensor_type, float reference_value)
{
    if (sensor_type >= SENSOR_COUNT) {
        return pdFALSE;
    }

    // 采集多个样本计算平均值
    float sum = 0.0f;
    for (uint8_t i = 0; i < CALIBRATION_SAMPLES; i++) {
        sum += g_sensor_context.sensors[sensor_type].filtered_value;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    float average = sum / CALIBRATION_SAMPLES;

    // 计算新的偏移量
    g_sensor_configs[sensor_type].offset = reference_value -
        (average * g_sensor_configs[sensor_type].scale_factor);

    printf("[SensorV3] Sensor %d calibrated: reference=%.3f, offset=%.3f\r\n",
           sensor_type, reference_value, g_sensor_configs[sensor_type].offset);

    // 设置事件标志
    xEventGroupSetBits(xEventGroup_Sensor, EVENT_SENSOR_CALIBRATE);

    return pdTRUE;
}

/**
 * @brief 获取传感器任务统计信息
 * @param stats 统计信息结构指针
 */
void SensorTaskV3_GetStatistics(sensor_task_stats_t *stats)
{
    if (stats != NULL) {
        memcpy(stats, &g_sensor_stats, sizeof(sensor_task_stats_t));
    }
}

/**
 * @brief 重置传感器任务统计信息
 */
void SensorTaskV3_ResetStatistics(void)
{
    memset(&g_sensor_stats, 0, sizeof(sensor_task_stats_t));
    printf("[SensorV3] Statistics Reset\r\n");
}

/**
 * @brief 发送传感器消息到队列
 * @param msg 消息结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_SendMessage(const sensor_msg_t *msg, uint32_t timeout_ms)
{
    if (msg == NULL) {
        return pdFALSE;
    }

    return xQueueSend(xQueue_SensorMsg, msg, pdMS_TO_TICKS(timeout_ms));
}

/**
 * @brief 从队列接收传感器消息
 * @param msg 消息结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_ReceiveMessage(sensor_msg_t *msg, uint32_t timeout_ms)
{
    if (msg == NULL) {
        return pdFALSE;
    }

    return xQueueReceive(xQueue_SensorMsg, msg, pdMS_TO_TICKS(timeout_ms));
}

/**
 * @brief 获取温度传感器数组
 * @param temp_array 温度数组指针 (至少3个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_GetTemperatures(float *temp_array)
{
    if (temp_array == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_SensorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(temp_array, g_sensor_context.temp_values, sizeof(float) * 3);
        xSemaphoreGive(xMutex_SensorContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 获取压力传感器数组
 * @param pressure_array 压力数组指针 (至少4个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_GetPressures(float *pressure_array)
{
    if (pressure_array == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_SensorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(pressure_array, g_sensor_context.pressure_values, sizeof(float) * 4);
        xSemaphoreGive(xMutex_SensorContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 获取液位传感器数组
 * @param level_array 液位数组指针 (至少4个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_GetLevels(float *level_array)
{
    if (level_array == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_SensorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(level_array, g_sensor_context.level_values, sizeof(float) * 4);
        xSemaphoreGive(xMutex_SensorContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 获取流量值
 * @return 流量值 (L/min)
 */
float SensorTaskV3_GetFlowRate(void)
{
    float flow_value = 0.0f;

    if (xSemaphoreTake(xMutex_SensorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        flow_value = g_sensor_context.flow_value;
        xSemaphoreGive(xMutex_SensorContext);
    }

    return flow_value;
}

/**
 * @brief 检查传感器系统健康状态
 * @return 整体质量分数 (0-100)
 */
uint8_t SensorTaskV3_CheckHealth(void)
{
    uint8_t quality = 0;

    if (xSemaphoreTake(xMutex_SensorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        quality = g_sensor_context.overall_quality;
        xSemaphoreGive(xMutex_SensorContext);
    }

    return quality;
}

/* ========================================================================== */
/* 私有函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化传感器配置
 */
static void Sensor_InitializeConfigs(void)
{
    // 温度传感器配置 (FTT518 Pt100)
    for (uint8_t i = SENSOR_TEMP_1; i <= SENSOR_TEMP_2; i++) {
        g_sensor_configs[i].channel = i;
        g_sensor_configs[i].scale_factor = 0.1f;      // 0.1°C per unit
        g_sensor_configs[i].offset = 0.0f;
        g_sensor_configs[i].filter_coefficient = 0.8f;
        g_sensor_configs[i].sample_count = 10;
        g_sensor_configs[i].enabled = true;
    }

    // 压力传感器配置 (HP10MY)
    for (uint8_t i = SENSOR_PRESSURE_1; i <= SENSOR_PRESSURE_4; i++) {
        g_sensor_configs[i].channel = i;
        g_sensor_configs[i].scale_factor = 0.01f;     // 0.01kPa per unit
        g_sensor_configs[i].offset = 0.0f;
        g_sensor_configs[i].filter_coefficient = 0.7f;
        g_sensor_configs[i].sample_count = 5;
        g_sensor_configs[i].enabled = true;
    }

    // 液位传感器配置 (FRD-8061)
    for (uint8_t i = SENSOR_LEVEL_1; i <= SENSOR_LEVEL_ANALOG; i++) {
        g_sensor_configs[i].channel = i;
        g_sensor_configs[i].scale_factor = 0.1f;      // 0.1mm per unit
        g_sensor_configs[i].offset = 0.0f;
        g_sensor_configs[i].filter_coefficient = 0.9f;
        g_sensor_configs[i].sample_count = 8;
        g_sensor_configs[i].enabled = true;
    }

    // 流量传感器配置 (I2C)
    g_sensor_configs[SENSOR_FLOW].channel = 0x40;    // I2C地址
    g_sensor_configs[SENSOR_FLOW].scale_factor = 0.01f;  // 0.01 L/min per unit
    g_sensor_configs[SENSOR_FLOW].offset = 0.0f;
    g_sensor_configs[SENSOR_FLOW].filter_coefficient = 0.85f;
    g_sensor_configs[SENSOR_FLOW].sample_count = 5;
    g_sensor_configs[SENSOR_FLOW].enabled = true;

    printf("[SensorV3] Sensor configurations initialized\r\n");
}

/**
 * @brief 读取所有传感器数据
 */
static void Sensor_ReadAllSensors(void)
{
    // 读取温度传感器
    Sensor_ReadTemperatureSensors();

    // 读取压力传感器
    Sensor_ReadPressureSensors();

    // 读取液位传感器
    Sensor_ReadLevelSensors();

    // 读取流量传感器
    Sensor_ReadFlowSensor();

    // 更新循环计数
    g_sensor_context.cycle_count++;
    g_sensor_context.last_update_time = HAL_GetTick();
}

/**
 * @brief 读取温度传感器 (FTT518 Pt100)
 */
static void Sensor_ReadTemperatureSensors(void)
{
    for (uint8_t i = SENSOR_TEMP_1; i <= SENSOR_TEMP_3; i++) {
        if (!g_sensor_configs[i].enabled) {
            continue;
        }

        // TODO: 调用驱动层接口读取实际硬件数据
        // 这里使用模拟数据作为示例
        float raw_value = 20.0f + (i * 5.0f) + (sinf(HAL_GetTick() / 1000.0f) * 2.0f);

        // 应用滤波
        float filtered_value = Sensor_ApplyFilter(i, raw_value);

        // 应用标定
        float calibrated_value = Sensor_ApplyCalibration(i, filtered_value);

        // 计算质量
        uint8_t quality = Sensor_CalculateQuality(i);

        // 更新传感器数据
        g_sensor_context.sensors[i].raw_value = raw_value;
        g_sensor_context.sensors[i].filtered_value = filtered_value;
        g_sensor_context.sensors[i].calibrated_value = calibrated_value;
        g_sensor_context.sensors[i].timestamp = HAL_GetTick();
        g_sensor_context.sensors[i].valid = true;
        g_sensor_context.sensors[i].quality = quality;

        // 更新分类数据
        g_sensor_context.temp_values[i - SENSOR_TEMP_1] = calibrated_value;

        g_sensor_stats.total_samples++;
    }
}

/**
 * @brief 读取压力传感器 (HP10MY)
 */
static void Sensor_ReadPressureSensors(void)
{
    for (uint8_t i = SENSOR_PRESSURE_1; i <= SENSOR_PRESSURE_4; i++) {
        if (!g_sensor_configs[i].enabled) {
            continue;
        }

        // TODO: 调用驱动层接口读取实际硬件数据
        // 这里使用模拟数据作为示例
        float raw_value = 100.0f + ((i - SENSOR_PRESSURE_1) * 10.0f) +
                          (sinf(HAL_GetTick() / 500.0f) * 5.0f);

        // 应用滤波
        float filtered_value = Sensor_ApplyFilter(i, raw_value);

        // 应用标定
        float calibrated_value = Sensor_ApplyCalibration(i, filtered_value);

        // 计算质量
        uint8_t quality = Sensor_CalculateQuality(i);

        // 更新传感器数据
        g_sensor_context.sensors[i].raw_value = raw_value;
        g_sensor_context.sensors[i].filtered_value = filtered_value;
        g_sensor_context.sensors[i].calibrated_value = calibrated_value;
        g_sensor_context.sensors[i].timestamp = HAL_GetTick();
        g_sensor_context.sensors[i].valid = true;
        g_sensor_context.sensors[i].quality = quality;

        // 更新分类数据
        g_sensor_context.pressure_values[i - SENSOR_PRESSURE_1] = calibrated_value;

        g_sensor_stats.total_samples++;
    }
}

/**
 * @brief 读取液位传感器 (FRD-8061)
 */
static void Sensor_ReadLevelSensors(void)
{
    for (uint8_t i = SENSOR_LEVEL_1; i <= SENSOR_LEVEL_ANALOG; i++) {
        if (!g_sensor_configs[i].enabled) {
            continue;
        }

        // TODO: 调用驱动层接口读取实际硬件数据
        // 这里使用模拟数据作为示例
        float raw_value = 50.0f + ((i - SENSOR_LEVEL_1) * 20.0f) +
                          (sinf(HAL_GetTick() / 2000.0f) * 10.0f);

        // 应用滤波
        float filtered_value = Sensor_ApplyFilter(i, raw_value);

        // 应用标定
        float calibrated_value = Sensor_ApplyCalibration(i, filtered_value);

        // 计算质量
        uint8_t quality = Sensor_CalculateQuality(i);

        // 更新传感器数据
        g_sensor_context.sensors[i].raw_value = raw_value;
        g_sensor_context.sensors[i].filtered_value = filtered_value;
        g_sensor_context.sensors[i].calibrated_value = calibrated_value;
        g_sensor_context.sensors[i].timestamp = HAL_GetTick();
        g_sensor_context.sensors[i].valid = true;
        g_sensor_context.sensors[i].quality = quality;

        // 更新分类数据
        g_sensor_context.level_values[i - SENSOR_LEVEL_1] = calibrated_value;

        g_sensor_stats.total_samples++;
    }
}

/**
 * @brief 读取流量传感器 (I2C)
 */
static void Sensor_ReadFlowSensor(void)
{
    sensor_type_t i = SENSOR_FLOW;

    if (!g_sensor_configs[i].enabled) {
        return;
    }

    // TODO: 调用驱动层接口读取实际硬件数据 (I2C通信)
    // 这里使用模拟数据作为示例
    float raw_value = 5.0f + (sinf(HAL_GetTick() / 3000.0f) * 2.0f);

    // 应用滤波
    float filtered_value = Sensor_ApplyFilter(i, raw_value);

    // 应用标定
    float calibrated_value = Sensor_ApplyCalibration(i, filtered_value);

    // 计算质量
    uint8_t quality = Sensor_CalculateQuality(i);

    // 更新传感器数据
    g_sensor_context.sensors[i].raw_value = raw_value;
    g_sensor_context.sensors[i].filtered_value = filtered_value;
    g_sensor_context.sensors[i].calibrated_value = calibrated_value;
    g_sensor_context.sensors[i].timestamp = HAL_GetTick();
    g_sensor_context.sensors[i].valid = true;
    g_sensor_context.sensors[i].quality = quality;

    // 更新分类数据
    g_sensor_context.flow_value = calibrated_value;

    g_sensor_stats.total_samples++;
}

/**
 * @brief 应用滤波算法 (移动平均滤波)
 * @param sensor_type 传感器类型
 * @param raw_value 原始值
 * @return 滤波后的值
 */
static float Sensor_ApplyFilter(sensor_type_t sensor_type, float raw_value)
{
    if (sensor_type >= SENSOR_COUNT) {
        return raw_value;
    }

    // 存储到滤波缓冲区
    uint8_t index = g_filter_index[sensor_type];
    g_filter_buffer[sensor_type][index] = raw_value;

    // 更新索引
    g_filter_index[sensor_type] = (index + 1) % MAX_FILTER_SAMPLES;

    // 更新计数
    if (g_filter_count[sensor_type] < MAX_FILTER_SAMPLES) {
        g_filter_count[sensor_type]++;
    }

    // 计算移动平均值
    float sum = 0.0f;
    uint8_t count = g_filter_count[sensor_type];
    for (uint8_t i = 0; i < count; i++) {
        sum += g_filter_buffer[sensor_type][i];
    }

    return sum / count;
}

/**
 * @brief 应用标定系数
 * @param sensor_type 传感器类型
 * @param filtered_value 滤波后的值
 * @return 标定后的值
 */
static float Sensor_ApplyCalibration(sensor_type_t sensor_type, float filtered_value)
{
    if (sensor_type >= SENSOR_COUNT) {
        return filtered_value;
    }

    sensor_config_t *config = &g_sensor_configs[sensor_type];
    return (filtered_value * config->scale_factor) + config->offset;
}

/**
 * @brief 计算数据质量分数
 * @param sensor_type 传感器类型
 * @return 质量分数 (0-100)
 */
static uint8_t Sensor_CalculateQuality(sensor_type_t sensor_type)
{
    if (sensor_type >= SENSOR_COUNT) {
        return 0;
    }

    // 基于滤波样本数量和变化率计算质量
    uint8_t sample_quality = (g_filter_count[sensor_type] * 100) / MAX_FILTER_SAMPLES;

    // TODO: 可以添加更多的质量检查逻辑
    // 例如: 检查值的变化率、范围检查等

    return sample_quality;
}

/**
 * @brief 更新传感器上下文
 */
static void Sensor_UpdateContext(void)
{
    if (xSemaphoreTake(xMutex_SensorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        // 上下文数据已经在读取传感器时更新
        // 这里只需要设置系统就绪标志
        g_sensor_context.system_ready = true;

        xSemaphoreGive(xMutex_SensorContext);
    }
}

/**
 * @brief 检查系统健康状态
 */
static void Sensor_CheckSystemHealth(void)
{
    uint32_t total_quality = 0;
    uint8_t valid_sensors = 0;

    // 计算所有传感器的平均质量
    for (uint8_t i = 0; i < SENSOR_COUNT; i++) {
        if (g_sensor_configs[i].enabled && g_sensor_context.sensors[i].valid) {
            total_quality += g_sensor_context.sensors[i].quality;
            valid_sensors++;
        }
    }

    if (valid_sensors > 0) {
        g_sensor_context.overall_quality = (uint8_t)(total_quality / valid_sensors);
    } else {
        g_sensor_context.overall_quality = 0;
    }

    // 检查质量是否低于阈值
    if (g_sensor_context.overall_quality < QUALITY_THRESHOLD) {
        xEventGroupSetBits(xEventGroup_Sensor, EVENT_SENSOR_ERROR);
        g_sensor_stats.data_errors++;
    }
}

/************************ (C) COPYRIGHT Ink Supply Control System *****END OF FILE****/
