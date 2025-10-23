/**
 ******************************************************************************
 * @file    actuator_task_v3.h
 * @brief   执行器任务头文件 - 基于V3设计文档
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
 * 执行器任务负责:
 * 1. 控制电磁阀 (24V x2)
 * 2. 控制加热器 (继电器 x3)
 * 3. 控制调速泵 (PWM x2)
 * 4. 控制直流泵 (IO x2)
 * 5. 执行安全输出保护
 * 6. 提供执行器状态反馈
 ******************************************************************************
 */

#ifndef __ACTUATOR_TASK_V3_H
#define __ACTUATOR_TASK_V3_H

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

#define ACTUATOR_TASK_PRIORITY          8       // 执行器任务优先级
#define ACTUATOR_TASK_STACK_SIZE        1024    // 堆栈大小 (words)
#define ACTUATOR_TASK_PERIOD_MS         10      // 任务周期 10ms (参考V3表格)

/* ========================================================================== */
/* 执行器类型定义 (参考设计文档V3 第2.3.1节) */
/* ========================================================================== */

// 执行器类型枚举
typedef enum {
    ACTUATOR_VALVE_1        = 0,    // 电磁阀1 (24V)
    ACTUATOR_VALVE_2        = 1,    // 电磁阀2 (24V)
    ACTUATOR_HEATER_1       = 2,    // 加热器1 (继电器)
    ACTUATOR_HEATER_2       = 3,    // 加热器2 (继电器)
    ACTUATOR_HEATER_3       = 4,    // 加热器3 (继电器)
    ACTUATOR_PUMP_SPEED_1   = 5,    // 调速泵1 (PWM)
    ACTUATOR_PUMP_SPEED_2   = 6,    // 调速泵2 (PWM)
    ACTUATOR_PUMP_DC_1      = 7,    // 直流泵1 (IO)
    ACTUATOR_PUMP_DC_2      = 8,    // 直流泵2 (IO)
    ACTUATOR_COUNT          = 9     // 执行器总数
} actuator_type_t;

// 执行器输出类型
typedef enum {
    OUTPUT_TYPE_DIGITAL,            // 数字输出 (ON/OFF)
    OUTPUT_TYPE_PWM,                // PWM输出 (0-100%)
    OUTPUT_TYPE_ANALOG              // 模拟输出 (0-100%)
} output_type_t;

// 执行器状态
typedef enum {
    ACTUATOR_STATE_IDLE,            // 空闲
    ACTUATOR_STATE_RUNNING,         // 运行中
    ACTUATOR_STATE_ERROR,           // 错误
    ACTUATOR_STATE_DISABLED         // 禁用
} actuator_state_t;

/* ========================================================================== */
/* 执行器配置结构 (参考设计文档V3 第2.3.1节) */
/* ========================================================================== */

typedef struct {
    uint8_t channel;                // 输出通道
    output_type_t output_type;      // 输出类型
    float current_output;           // 当前输出值 (0-100%)
    float target_output;            // 目标输出值 (0-100%)
    float min_output;               // 最小输出限制
    float max_output;               // 最大输出限制
    bool enabled;                   // 使能状态
    bool safety_override;           // 安全覆盖标志
    uint32_t last_update;           // 最后更新时间
    float ramp_rate;                // 爬坡速率 (%/s)
} actuator_config_t;

/* ========================================================================== */
/* 执行器状态结构 */
/* ========================================================================== */

typedef struct {
    actuator_type_t type;           // 执行器类型
    actuator_state_t state;         // 执行器状态
    float output_value;             // 输出值 (0-100%)
    float feedback_value;           // 反馈值 (如果有)
    bool fault;                     // 故障标志
    uint32_t fault_code;            // 故障代码
    uint32_t run_time;              // 运行时间 (秒)
    uint32_t switch_count;          // 开关次数
    uint32_t timestamp;             // 时间戳
} actuator_status_t;

/* ========================================================================== */
/* 执行器上下文结构 */
/* ========================================================================== */

typedef struct {
    // 所有执行器配置
    actuator_config_t configs[ACTUATOR_COUNT];

    // 所有执行器状态
    actuator_status_t status[ACTUATOR_COUNT];

    // 分类状态 (便于访问)
    bool valve_states[2];           // 电磁阀状态 (ON/OFF)
    bool heater_states[3];          // 加热器状态 (ON/OFF)
    float pump_speed[2];            // 调速泵速度 (0-100%)
    bool pump_dc_states[2];         // 直流泵状态 (ON/OFF)

    // 整体状态
    uint32_t cycle_count;           // 循环计数
    uint32_t last_update_time;      // 最后更新时间
    bool safety_mode;               // 安全模式标志
    bool emergency_stop;            // 紧急停止标志
    bool system_ready;              // 系统就绪标志
} actuator_context_t;

/* ========================================================================== */
/* 执行器命令结构 */
/* ========================================================================== */

typedef enum {
    ACTUATOR_CMD_SET_OUTPUT,        // 设置输出值
    ACTUATOR_CMD_ENABLE,            // 使能执行器
    ACTUATOR_CMD_DISABLE,           // 禁用执行器
    ACTUATOR_CMD_RESET_FAULT,       // 复位故障
    ACTUATOR_CMD_EMERGENCY_STOP,    // 紧急停止
    ACTUATOR_CMD_RESUME             // 恢复运行
} actuator_cmd_type_t;

typedef struct {
    actuator_cmd_type_t cmd_type;   // 命令类型
    actuator_type_t actuator_type;  // 执行器类型
    float value;                    // 命令值
    uint32_t timestamp;             // 时间戳
    bool urgent;                    // 紧急标志
} actuator_command_t;

/* ========================================================================== */
/* 执行器任务统计信息 */
/* ========================================================================== */

typedef struct {
    uint32_t total_cycles;          // 总循环次数
    uint32_t command_count;         // 接收命令数
    uint32_t command_errors;        // 命令错误数
    uint32_t safety_triggers;       // 安全触发次数
    uint32_t emergency_stops;       // 紧急停止次数
    uint16_t max_cycle_time_us;     // 最大循环时间 (微秒)
    uint16_t avg_cycle_time_us;     // 平均循环时间 (微秒)
} actuator_task_stats_t;

/* ========================================================================== */
/* 消息队列数据结构 */
/* ========================================================================== */

// 消息类型
typedef enum {
    MSG_ACTUATOR_STATUS,            // 执行器状态消息
    MSG_ACTUATOR_COMMAND,           // 执行器命令消息
    MSG_ACTUATOR_FAULT,             // 执行器故障消息
    MSG_ACTUATOR_SAFETY             // 执行器安全消息
} actuator_msg_type_t;

// 执行器消息结构
typedef struct {
    actuator_msg_type_t type;       // 消息类型
    uint32_t timestamp;             // 时间戳
    uint16_t data_len;              // 数据长度
    union {
        actuator_context_t context; // 上下文数据
        actuator_command_t command; // 命令数据
        actuator_status_t status;   // 状态数据
        uint32_t fault_info;        // 故障信息
    } data;
} actuator_msg_t;

/* ========================================================================== */
/* 事件标志定义 */
/* ========================================================================== */

#define EVENT_ACTUATOR_UPDATE           (1 << 0)    // 执行器更新
#define EVENT_ACTUATOR_FAULT            (1 << 1)    // 执行器故障
#define EVENT_ACTUATOR_SAFETY           (1 << 2)    // 安全事件
#define EVENT_ACTUATOR_EMERGENCY        (1 << 3)    // 紧急停止

/* ========================================================================== */
/* 队列大小定义 */
/* ========================================================================== */

#define ACTUATOR_CMD_QUEUE_SIZE         16          // 命令队列大小
#define ACTUATOR_MSG_QUEUE_SIZE         16          // 消息队列大小

/* ========================================================================== */
/* 安全参数定义 */
/* ========================================================================== */

#define ACTUATOR_WATCHDOG_TIMEOUT_MS    500         // 看门狗超时
#define ACTUATOR_FAULT_RETRY_COUNT      3           // 故障重试次数
#define ACTUATOR_RAMP_DEFAULT_RATE      10.0f       // 默认爬坡速率 (%/s)

/* ========================================================================== */
/* 全局变量声明 */
/* ========================================================================== */

extern TaskHandle_t xTaskHandle_ActuatorV3;
extern QueueHandle_t xQueue_ActuatorCmd;
extern QueueHandle_t xQueue_ActuatorMsg;
extern SemaphoreHandle_t xMutex_ActuatorContext;
extern EventGroupHandle_t xEventGroup_Actuator;

/* ========================================================================== */
/* 公共函数声明 */
/* ========================================================================== */

/**
 * @brief 初始化执行器任务系统
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t ActuatorTaskV3_Init(void);

/**
 * @brief 创建执行器任务
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t ActuatorTaskV3_Create(void);

/**
 * @brief 执行器任务主函数
 * @param pvParameters 任务参数
 */
void Task_ActuatorV3(void *pvParameters);

/**
 * @brief 设置执行器输出值
 * @param actuator_type 执行器类型
 * @param value 输出值 (0-100%)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SetOutput(actuator_type_t actuator_type, float value);

/**
 * @brief 使能执行器
 * @param actuator_type 执行器类型
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_Enable(actuator_type_t actuator_type);

/**
 * @brief 禁用执行器
 * @param actuator_type 执行器类型
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_Disable(actuator_type_t actuator_type);

/**
 * @brief 紧急停止所有执行器
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_EmergencyStop(void);

/**
 * @brief 恢复执行器运行
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_Resume(void);

/**
 * @brief 获取执行器上下文
 * @param context 上下文结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_GetContext(actuator_context_t *context);

/**
 * @brief 获取执行器状态
 * @param actuator_type 执行器类型
 * @param status 状态结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_GetStatus(actuator_type_t actuator_type, actuator_status_t *status);

/**
 * @brief 配置执行器
 * @param actuator_type 执行器类型
 * @param config 配置结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_Configure(actuator_type_t actuator_type, const actuator_config_t *config);

/**
 * @brief 复位执行器故障
 * @param actuator_type 执行器类型
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_ResetFault(actuator_type_t actuator_type);

/**
 * @brief 发送执行器命令
 * @param command 命令结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SendCommand(const actuator_command_t *command, uint32_t timeout_ms);

/**
 * @brief 获取执行器任务统计信息
 * @param stats 统计信息结构指针
 */
void ActuatorTaskV3_GetStatistics(actuator_task_stats_t *stats);

/**
 * @brief 重置执行器任务统计信息
 */
void ActuatorTaskV3_ResetStatistics(void);

/**
 * @brief 设置电磁阀状态
 * @param valve_id 阀门ID (0 or 1)
 * @param state 状态 (true=开, false=关)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SetValve(uint8_t valve_id, bool state);

/**
 * @brief 设置加热器状态
 * @param heater_id 加热器ID (0, 1, or 2)
 * @param state 状态 (true=开, false=关)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SetHeater(uint8_t heater_id, bool state);

/**
 * @brief 设置调速泵速度
 * @param pump_id 泵ID (0 or 1)
 * @param speed 速度 (0-100%)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SetPumpSpeed(uint8_t pump_id, float speed);

/**
 * @brief 设置直流泵状态
 * @param pump_id 泵ID (0 or 1)
 * @param state 状态 (true=开, false=关)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SetDCPump(uint8_t pump_id, bool state);

/**
 * @brief 获取所有电磁阀状态
 * @param valve_states 阀门状态数组指针 (至少2个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_GetValveStates(bool *valve_states);

/**
 * @brief 获取所有加热器状态
 * @param heater_states 加热器状态数组指针 (至少3个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_GetHeaterStates(bool *heater_states);

/**
 * @brief 获取所有调速泵速度
 * @param pump_speeds 泵速度数组指针 (至少2个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_GetPumpSpeeds(float *pump_speeds);

/**
 * @brief 检查是否处于安全模式
 * @return true=安全模式, false=正常模式
 */
bool ActuatorTaskV3_IsInSafetyMode(void);

/**
 * @brief 检查是否紧急停止
 * @return true=紧急停止, false=正常
 */
bool ActuatorTaskV3_IsEmergencyStopped(void);

/**
 * @brief 检查执行器系统健康状态
 * @return 健康分数 (0-100)
 */
uint8_t ActuatorTaskV3_CheckHealth(void);

#ifdef __cplusplus
}
#endif

#endif /* __ACTUATOR_TASK_V3_H */

/************************ (C) COPYRIGHT Ink Supply Control System *****END OF FILE****/
