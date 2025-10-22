#ifndef __SENSOR_TASKS_H
#define __SENSOR_TASKS_H

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "app_io_handler.h"
#include "ecat_def.h"

#ifdef __cplusplus
extern "C" {
#endif

// ====================================================================
// 任务配置参数
// ====================================================================
#define SENSOR_DATA_TASK_PRIORITY      3           // 传感器任务优先级
#define MASTER_SIGNAL_TASK_PRIORITY    3           // 主站信号任务优先级
#define SENSOR_DATA_TASK_STACK_SIZE    512         // 传感器任务堆栈大小 (words)
#define MASTER_SIGNAL_TASK_STACK_SIZE  384         // 主站信号任务堆栈大小 (words)

#define SENSOR_DATA_PERIOD_MS          5           // 传感器数据采集周期 (5ms)
#define SENSOR_QUEUE_SIZE              10          // 传感器数据队列大小
#define MASTER_COMMAND_QUEUE_SIZE      8           // 主站命令队列大小

// 数据滤波配置
#define ANALOG_FILTER_DEPTH            4           // 模拟数据滤波深度
#define DIGITAL_DEBOUNCE_COUNT         3           // 数字输入防抖计数

// 安全配置
#define COMMAND_TIMEOUT_MS             100         // 命令超时时间
#define DATA_QUALITY_THRESHOLD         95          // 数据质量阈值 (百分比)

// 事件标志定义
#define EVENT_NEW_SENSOR_DATA          (1 << 0)    // 新传感器数据事件
#define EVENT_MASTER_COMMAND           (1 << 1)    // 主站命令事件
#define EVENT_SYSTEM_ERROR             (1 << 2)    // 系统错误事件
#define EVENT_DATA_QUALITY_LOW         (1 << 3)    // 数据质量低事件

// ====================================================================
// 数据结构定义
// ====================================================================

/**
 * @brief 传感器数据结构 (发送到主站)
 */
typedef struct {
    // 数字传感器数据 (开关量)
    uint16_t digital_sensors;          // 16位数字传感器状态
    uint8_t digital_quality_flags;     // 数字输入质量标志 (每位对应一个输入)

    // 模拟传感器数据 (模拟量)
    int16_t analog_sensors[MAX_ANALOG_INPUTS];   // 8路模拟传感器值 (已滤波)
    int16_t analog_raw[MAX_ANALOG_INPUTS];       // 8路原始ADC值 (调试用)
    uint8_t analog_quality[MAX_ANALOG_INPUTS];   // 模拟输入质量评分 (0-100)

    // 数据状态和时间戳
    uint8_t overall_data_quality;       // 整体数据质量 (0-100)
    uint32_t timestamp;                 // 时间戳 (ms)
    uint16_t sequence_number;           // 序列号 (循环计数)
    uint8_t system_status;              // 系统状态标志

    // 传感器配置状态
    uint16_t active_digital_mask;       // 激活的数字输入掩码
    uint8_t active_analog_mask;         // 激活的模拟输入掩码
} sensor_data_t;

/**
 * @brief 主站命令结构 (从主站接收)
 */
typedef struct {
    // 数字控制信号 (开关量)
    uint16_t digital_outputs;          // 16位数字输出控制
    uint16_t digital_output_mask;      // 数字输出有效掩码

    // 模拟控制信号 (模拟量)
    int16_t analog_outputs[MAX_ANALOG_OUTPUTS];  // 4路模拟输出值
    uint8_t analog_output_mask;        // 模拟输出有效掩码

    // 控制参数
    uint8_t control_mode;              // 控制模式 (0=手动, 1=自动, 2=安全模式)
    uint8_t safety_state;              // 安全状态 (0=正常, 1=警告, 2=紧急停止)
    uint32_t command_id;               // 命令ID (用于跟踪)
    uint32_t timestamp;                // 命令时间戳
    uint16_t checksum;                 // 校验和
} master_command_t;

/**
 * @brief 传感器配置结构
 */
typedef struct {
    // 数字传感器配置
    uint16_t enabled_digital_inputs;   // 使能的数字输入掩码
    uint8_t digital_debounce_ms;       // 数字输入防抖时间

    // 模拟传感器配置
    uint8_t enabled_analog_inputs;     // 使能的模拟输入掩码
    uint16_t analog_sample_rate;       // 模拟输入采样率
    uint8_t filter_enable;             // 滤波使能

    // 质量控制
    uint8_t quality_check_enable;      // 质量检查使能
    uint8_t min_quality_threshold;     // 最小质量阈值
} sensor_config_t;

/**
 * @brief 任务统计信息
 */
typedef struct {
    // 传感器任务统计
    uint32_t sensor_task_cycles;       // 传感器任务执行次数
    uint32_t sensor_data_errors;       // 传感器数据错误次数
    uint32_t analog_conversion_time_us; // 模拟转换时间 (微秒)

    // 主站命令任务统计
    uint32_t master_task_cycles;       // 主站任务执行次数
    uint32_t commands_received;        // 接收到的命令数
    uint32_t commands_executed;        // 执行的命令数
    uint32_t command_errors;           // 命令错误次数

    // 队列统计
    uint8_t sensor_queue_usage;        // 传感器队列使用率 (%)
    uint8_t command_queue_usage;       // 命令队列使用率 (%)

    // 性能统计
    uint16_t max_sensor_task_time_us;  // 传感器任务最大执行时间
    uint16_t max_master_task_time_us;  // 主站任务最大执行时间
    uint16_t avg_loop_time_us;         // 平均循环时间
} sensor_task_stats_t;

// ====================================================================
// 全局变量声明
// ====================================================================

// 任务句柄
extern TaskHandle_t xTaskHandle_SensorData;
extern TaskHandle_t xTaskHandle_MasterSignal;

// 队列句柄
extern QueueHandle_t xQueue_SensorData;
extern QueueHandle_t xQueue_MasterCommands;

// 互斥体和信号量
extern SemaphoreHandle_t xMutex_SensorData;
extern SemaphoreHandle_t xMutex_MasterCommands;

// 事件组
extern EventGroupHandle_t xEventGroup_SensorTasks;

// 当前传感器配置
extern sensor_config_t current_sensor_config;

// 最新的传感器数据和主站命令
extern sensor_data_t latest_sensor_data;
extern master_command_t latest_master_command;

// ====================================================================
// 函数声明
// ====================================================================

/**
 * @brief 初始化传感器任务系统
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t Sensor_Tasks_Init(void);

/**
 * @brief 创建传感器相关任务
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t Sensor_Tasks_Create(void);

/**
 * @brief 传感器数据采集任务
 * @param pvParameters 任务参数
 */
void Task_SensorDataCollection(void *pvParameters);

/**
 * @brief 主站信号接收任务
 * @param pvParameters 任务参数
 */
void Task_MasterSignalReceiver(void *pvParameters);

/**
 * @brief 获取最新传感器数据
 * @param data 数据结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t Sensor_Get_Latest_Data(sensor_data_t *data);

/**
 * @brief 发送主站命令到队列
 * @param command 命令结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t Sensor_Send_Master_Command(const master_command_t *command, uint32_t timeout_ms);

/**
 * @brief 配置传感器参数
 * @param config 配置结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t Sensor_Configure(const sensor_config_t *config);

/**
 * @brief 获取传感器任务统计信息
 * @param stats 统计信息结构指针
 */
void Sensor_Get_Task_Statistics(sensor_task_stats_t *stats);

/**
 * @brief 重置传感器任务统计
 */
void Sensor_Reset_Statistics(void);

/**
 * @brief 执行传感器数据质量检查
 * @param data 传感器数据指针
 * @return 整体质量分数 (0-100)
 */
uint8_t Sensor_Check_Data_Quality(sensor_data_t *data);

/**
 * @brief 验证主站命令
 * @param command 命令结构指针
 * @return pdTRUE=有效, pdFALSE=无效
 */
BaseType_t Sensor_Validate_Master_Command(const master_command_t *command);

/**
 * @brief 处理系统错误事件
 * @param error_code 错误代码
 */
void Sensor_Handle_System_Error(uint32_t error_code);

/**
 * @brief 设置传感器安全模式
 * @param safety_mode 安全模式 (0=正常, 1=安全, 2=紧急停止)
 */
void Sensor_Set_Safety_Mode(uint8_t safety_mode);

/**
 * @brief 获取传感器任务状态
 * @return 状态位掩码
 */
uint32_t Sensor_Get_Task_Status(void);

// ====================================================================
// 内部函数声明 (可选暴露给调试)
// ====================================================================

/**
 * @brief 数字输入滤波和防抖处理
 * @param raw_inputs 原始输入值
 * @return 滤波后的输入值
 */
uint16_t Sensor_Filter_Digital_Inputs(uint16_t raw_inputs);

/**
 * @brief 模拟输入滤波处理
 * @param channel 通道号
 * @param raw_value 原始值
 * @return 滤波后的值
 */
int16_t Sensor_Filter_Analog_Input(uint8_t channel, int16_t raw_value);

/**
 * @brief 计算模拟输入质量分数
 * @param channel 通道号
 * @param current_value 当前值
 * @param previous_value 前一个值
 * @return 质量分数 (0-100)
 */
uint8_t Sensor_Calculate_Analog_Quality(uint8_t channel, int16_t current_value, int16_t previous_value);

/**
 * @brief 执行安全输出控制
 * @param safety_state 安全状态
 */
void Sensor_Execute_Safety_Outputs(uint8_t safety_state);

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_TASKS_H */