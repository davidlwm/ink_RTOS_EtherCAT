/**
 ******************************************************************************
 * @file    sensor_task_v3.h
 * @brief   传感器任务头文件 - 基于V3设计文档
 * @author  Ink Supply Control System Development Team
 * @version V3.0.0
 * @date    2025-01-23
 ******************************************************************************
 * @attention
 *
 * 本文件基于墨路控制系统详细设计文档V3实现，采用四层架构设计：
 * - 应用层 (Application Layer)
 * - 中间件层 (Middleware Layer)
 * - HAL层 (Hardware Abstraction Layer)
 * - 驱动层 (Driver Layer)
 *
 * 传感器任务负责:
 * 1. 采集温度传感器数据 (FTT518 Pt100 x3)
 * 2. 采集压力传感器数据 (HP10MY x4)
 * 3. 采集液位传感器数据 (FRD-8061 x3 + 模拟液位 x1)
 * 4. 采集流量传感器数据 (I2C接口 x1)
 * 5. 数据滤波和质量检查
 * 6. 通过消息队列向控制任务和通信任务发送数据
 ******************************************************************************
 */

#ifndef __SENSOR_TASK_V3_H
#define __SENSOR_TASK_V3_H

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* 任务配置参数 (参考设计文档V3 第2.1.1节) */
/* ========================================================================== */

#define SENSOR_TASK_PRIORITY           8        // 传感器任务优先级 (参考V3表格)
#define SENSOR_TASK_STACK_SIZE         1024     // 堆栈大小 (words)
#define SENSOR_TASK_PERIOD_MS          50       // 任务周期 50ms (参考V3表格)

/* ========================================================================== */
/* 传感器通道定义 (参考设计文档V3 第2.2.1节) */
/* ========================================================================== */

// 传感器类型枚举
typedef enum {
    SENSOR_TEMP_1       = 0,    // 温度传感器1 (FTT518 Pt100)
    SENSOR_TEMP_2       = 1,    // 温度传感器2
    SENSOR_TEMP_3       = 2,    // 温度传感器3
    SENSOR_PRESSURE_1   = 3,    // 压力传感器1 (HP10MY)
    SENSOR_PRESSURE_2   = 4,    // 压力传感器2
    SENSOR_PRESSURE_3   = 5,    // 压力传感器3
    SENSOR_PRESSURE_4   = 6,    // 压力传感器4
    SENSOR_LEVEL_1      = 7,    // 液位传感器1 (FRD-8061)
    SENSOR_LEVEL_2      = 8,    // 液位传感器2
    SENSOR_LEVEL_3      = 9,    // 液位传感器3
    SENSOR_LEVEL_ANALOG = 10,   // 模拟量液位
    SENSOR_FLOW         = 11,   // 流量传感器(I2C)
    SENSOR_COUNT        = 12    // 传感器总数
} sensor_type_t;

/* ========================================================================== */
/* 传感器配置结构 (参考设计文档V3 第2.2.1节) */
/* ========================================================================== */

typedef struct {
    uint8_t channel;              // ADC通道或I2C地址
    float scale_factor;           // 标定系数
    float offset;                 // 零点偏移
    float filter_coefficient;     // 滤波系数 (0.0-1.0)
    uint16_t sample_count;        // 采样次数
    bool enabled;                 // 使能标志
} sensor_config_t;

/* ========================================================================== */
/* 传感器数据结构 (参考设计文档V3 第2.2.2节) */
/* ========================================================================== */

typedef struct {
    float raw_value;              // 原始值
    float filtered_value;         // 滤波后值
    float calibrated_value;       // 标定后值
    uint32_t timestamp;           // 时间戳 (ms)
    bool valid;                   // 数据有效性
    uint16_t error_count;         // 错误计数
    uint8_t quality;              // 数据质量 (0-100)
} sensor_data_t;

/* ========================================================================== */
/* 传感器上下文结构 (参考设计文档V3 第2.2.2节) */
/* ========================================================================== */

typedef struct {
    // 所有传感器数据
    sensor_data_t sensors[SENSOR_COUNT];

    // 分类数据 (便于访问)
    float temp_values[3];         // 温度值 (°C)
    float pressure_values[4];     // 压力值 (kPa)
    float level_values[4];        // 液位值 (mm)
    float flow_value;             // 流量值 (L/min)

    // 整体状态
    uint32_t cycle_count;         // 循环计数
    uint32_t last_update_time;    // 最后更新时间
    uint8_t overall_quality;      // 整体质量 (0-100)
    bool system_ready;            // 系统就绪标志
} sensor_context_t;

/* ========================================================================== */
/* 传感器任务统计信息 */
/* ========================================================================== */

typedef struct {
    uint32_t total_cycles;        // 总循环次数
    uint32_t data_errors;         // 数据错误次数
    uint32_t queue_full_count;    // 队列满次数
    uint32_t timeout_count;       // 超时次数
    uint16_t max_cycle_time_us;   // 最大循环时间 (微秒)
    uint16_t avg_cycle_time_us;   // 平均循环时间 (微秒)
    uint32_t total_samples;       // 总采样次数
} sensor_task_stats_t;

/* ========================================================================== */
/* 消息队列数据结构 (参考设计文档V3 第2.1.2节) */
/* ========================================================================== */

// 消息类型
typedef enum {
    MSG_SENSOR_DATA,              // 传感器数据消息
    MSG_SENSOR_ERROR,             // 传感器错误消息
    MSG_SENSOR_CONFIG,            // 传感器配置消息
    MSG_SENSOR_CALIBRATE          // 传感器校准消息
} sensor_msg_type_t;

// 传感器消息结构
typedef struct {
    sensor_msg_type_t type;       // 消息类型
    uint32_t timestamp;           // 时间戳
    uint16_t data_len;            // 数据长度
    sensor_context_t context;     // 传感器上下文
} sensor_msg_t;

/* ========================================================================== */
/* 事件标志定义 */
/* ========================================================================== */

#define EVENT_SENSOR_DATA_READY      (1 << 0)   // 传感器数据就绪
#define EVENT_SENSOR_ERROR           (1 << 1)   // 传感器错误
#define EVENT_SENSOR_CALIBRATE       (1 << 2)   // 传感器校准请求
#define EVENT_SENSOR_CONFIG_UPDATE   (1 << 3)   // 传感器配置更新

/* ========================================================================== */
/* 队列大小定义 */
/* ========================================================================== */

#define SENSOR_MSG_QUEUE_SIZE        16         // 传感器消息队列大小

/* ========================================================================== */
/* 全局变量声明 */
/* ========================================================================== */

extern TaskHandle_t xTaskHandle_SensorV3;
extern QueueHandle_t xQueue_SensorMsg;
extern SemaphoreHandle_t xMutex_SensorContext;
extern EventGroupHandle_t xEventGroup_Sensor;

/* ========================================================================== */
/* 公共函数声明 */
/* ========================================================================== */

/**
 * @brief 初始化传感器任务系统
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t SensorTaskV3_Init(void);

/**
 * @brief 创建传感器任务
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t SensorTaskV3_Create(void);

/**
 * @brief 传感器任务主函数
 * @param pvParameters 任务参数
 */
void Task_SensorV3(void *pvParameters);

/**
 * @brief 获取传感器上下文数据
 * @param context 上下文结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_GetContext(sensor_context_t *context);

/**
 * @brief 配置单个传感器
 * @param sensor_type 传感器类型
 * @param config 配置结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_ConfigureSensor(sensor_type_t sensor_type, const sensor_config_t *config);

/**
 * @brief 获取单个传感器数据
 * @param sensor_type 传感器类型
 * @param data 数据结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_GetSensorData(sensor_type_t sensor_type, sensor_data_t *data);

/**
 * @brief 校准传感器
 * @param sensor_type 传感器类型
 * @param reference_value 参考值
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_CalibrateSensor(sensor_type_t sensor_type, float reference_value);

/**
 * @brief 获取传感器任务统计信息
 * @param stats 统计信息结构指针
 */
void SensorTaskV3_GetStatistics(sensor_task_stats_t *stats);

/**
 * @brief 重置传感器任务统计信息
 */
void SensorTaskV3_ResetStatistics(void);

/**
 * @brief 发送传感器消息到队列
 * @param msg 消息结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_SendMessage(const sensor_msg_t *msg, uint32_t timeout_ms);

/**
 * @brief 从队列接收传感器消息
 * @param msg 消息结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_ReceiveMessage(sensor_msg_t *msg, uint32_t timeout_ms);

/**
 * @brief 获取温度传感器数组
 * @param temp_array 温度数组指针 (至少3个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_GetTemperatures(float *temp_array);

/**
 * @brief 获取压力传感器数组
 * @param pressure_array 压力数组指针 (至少4个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_GetPressures(float *pressure_array);

/**
 * @brief 获取液位传感器数组
 * @param level_array 液位数组指针 (至少4个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t SensorTaskV3_GetLevels(float *level_array);

/**
 * @brief 获取流量值
 * @return 流量值 (L/min)
 */
float SensorTaskV3_GetFlowRate(void);

/**
 * @brief 检查传感器系统健康状态
 * @return 整体质量分数 (0-100)
 */
uint8_t SensorTaskV3_CheckHealth(void);

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_TASK_V3_H */

/************************ (C) COPYRIGHT Ink Supply Control System *****END OF FILE****/
