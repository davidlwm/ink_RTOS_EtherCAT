/**
 ******************************************************************************
 * @file    control_task_v3.h
 * @brief   控制任务头文件 - 基于V3设计文档
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
 * 控制任务负责:
 * 1. PID控制算法实现
 * 2. 传感器数据融合和处理
 * 3. 执行器输出控制
 * 4. 控制模式切换
 * 5. 参数自适应调整
 * 6. 系统安全监控
 ******************************************************************************
 */

#ifndef __CONTROL_TASK_V3_H
#define __CONTROL_TASK_V3_H

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "sensor_task_v3.h"
#include "actuator_task_v3.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* 任务配置参数 (参考设计文档V3 第2.1.1节) */
/* ========================================================================== */

#define CONTROL_TASK_PRIORITY       12      // 控制任务优先级
#define CONTROL_TASK_STACK_SIZE     1024    // 堆栈大小 (words) - 与传感器和执行器任务保持一致
#define CONTROL_TASK_PERIOD_MS      20      // 任务周期 20ms (参考V3表格)

/* ========================================================================== */
/* 控制回路定义 (参考设计文档V3 第2.4节) */
/* ========================================================================== */

// 控制回路类型
typedef enum {
    CONTROL_LOOP_TEMP_1     = 0,    // 温度控制回路1
    CONTROL_LOOP_TEMP_2     = 1,    // 温度控制回路2
    CONTROL_LOOP_TEMP_3     = 2,    // 温度控制回路3
    CONTROL_LOOP_PRESSURE_1 = 3,    // 压力控制回路1
    CONTROL_LOOP_PRESSURE_2 = 4,    // 压力控制回路2
    CONTROL_LOOP_PRESSURE_3 = 5,    // 压力控制回路3
    CONTROL_LOOP_PRESSURE_4 = 6,    // 压力控制回路4
    CONTROL_LOOP_LEVEL_1    = 7,    // 液位控制回路1
    CONTROL_LOOP_LEVEL_2    = 8,    // 液位控制回路2
    CONTROL_LOOP_LEVEL_3    = 9,    // 液位控制回路3
    CONTROL_LOOP_LEVEL_4    = 10,   // 液位控制回路4
    CONTROL_LOOP_FLOW       = 11,   // 流量控制回路
    CONTROL_LOOP_COUNT      = 12    // 控制回路总数
} control_loop_t;

// 控制模式
typedef enum {
    CONTROL_MODE_MANUAL,            // 手动模式
    CONTROL_MODE_AUTO,              // 自动模式 (PID控制)
    CONTROL_MODE_CASCADE,           // 串级控制
    CONTROL_MODE_FEEDFORWARD,       // 前馈控制
    CONTROL_MODE_ADAPTIVE,          // 自适应控制
    CONTROL_MODE_SAFETY             // 安全模式
} control_mode_t;

// 控制状态
typedef enum {
    CONTROL_STATE_IDLE,             // 空闲
    CONTROL_STATE_RUNNING,          // 运行中
    CONTROL_STATE_TUNING,           // 参数整定中
    CONTROL_STATE_ERROR,            // 错误
    CONTROL_STATE_SAFETY            // 安全状态
} control_state_t;

/* ========================================================================== */
/* PID控制器参数结构 */
/* ========================================================================== */

typedef struct {
    // PID参数
    float kp;                       // 比例系数
    float ki;                       // 积分系数
    float kd;                       // 微分系数

    // 输出限制
    float output_min;               // 输出最小值
    float output_max;               // 输出最大值

    // 积分限制
    float integral_min;             // 积分最小值
    float integral_max;             // 积分最大值

    // 微分滤波
    float derivative_filter;        // 微分滤波系数 (0-1)

    // 死区和饱和
    float deadband;                 // 死区范围
    float saturation_limit;         // 饱和限制

    // 采样时间
    float sample_time;              // 采样时间 (秒)

    // 使能标志
    bool enabled;                   // PID使能
    bool integral_enabled;          // 积分项使能
    bool derivative_enabled;        // 微分项使能
    bool anti_windup_enabled;       // 抗积分饱和使能
} pid_params_t;

/* ========================================================================== */
/* PID控制器状态结构 */
/* ========================================================================== */

typedef struct {
    // 当前值
    float setpoint;                 // 设定值
    float process_value;            // 过程值 (反馈值)
    float error;                    // 误差
    float last_error;               // 上次误差
    float integral;                 // 积分累积
    float derivative;               // 微分项
    float output;                   // 输出值

    // 滤波后的微分
    float filtered_derivative;      // 滤波后的微分

    // 状态信息
    uint32_t last_update_time;      // 上次更新时间
    bool first_run;                 // 首次运行标志
    bool in_deadband;               // 在死区内标志
    bool output_saturated;          // 输出饱和标志

    // 统计信息
    uint32_t cycle_count;           // 控制周期计数
    float max_error;                // 最大误差
    float avg_error;                // 平均误差
    float steady_state_error;       // 稳态误差
} pid_state_t;

/* ========================================================================== */
/* 控制回路配置结构 */
/* ========================================================================== */

typedef struct {
    // 回路基本信息
    control_loop_t loop_id;         // 回路ID
    control_mode_t mode;            // 控制模式
    control_state_t state;          // 控制状态

    // 传感器映射
    sensor_type_t sensor_type;      // 关联的传感器类型
    actuator_type_t actuator_type;  // 关联的执行器类型

    // PID控制器
    pid_params_t pid_params;        // PID参数
    pid_state_t pid_state;          // PID状态

    // 回路参数
    float setpoint;                 // 设定值
    float setpoint_min;             // 设定值最小限制
    float setpoint_max;             // 设定值最大限制
    float process_value;            // 当前过程值
    float output_value;             // 输出值

    // 报警限制
    float alarm_high;               // 高报警限制
    float alarm_low;                // 低报警限制
    float warning_high;             // 高警告限制
    float warning_low;              // 低警告限制

    // 使能和状态
    bool enabled;                   // 回路使能
    bool auto_mode;                 // 自动模式标志
    bool alarm_status;              // 报警状态
    bool warning_status;            // 警告状态

    // 时间信息
    uint32_t last_update_time;      // 最后更新时间
    uint32_t total_run_time;        // 总运行时间

    // 质量指标
    float control_quality;          // 控制质量分数 (0-100)
    uint32_t quality_update_count;  // 质量更新次数
} control_loop_config_t;

/* ========================================================================== */
/* 控制系统上下文结构 */
/* ========================================================================== */

typedef struct {
    // 所有控制回路
    control_loop_config_t loops[CONTROL_LOOP_COUNT];

    // 系统状态
    control_mode_t system_mode;     // 系统控制模式
    control_state_t system_state;   // 系统控制状态
    bool system_enabled;            // 系统使能
    bool emergency_stop;            // 紧急停止状态
    bool safety_mode;               // 安全模式

    // 传感器数据缓存
    sensor_context_t sensor_data;   // 传感器数据
    uint32_t sensor_data_age;       // 数据年龄 (ms)
    bool sensor_data_valid;         // 数据有效性

    // 执行器状态缓存
    bool actuator_states_valid;     // 执行器状态有效性
    uint32_t actuator_update_time;  // 执行器更新时间

    // 系统统计
    uint32_t cycle_count;           // 总控制周期数
    uint32_t error_count;           // 错误计数
    uint32_t mode_switch_count;     // 模式切换次数
    uint32_t last_update_time;      // 最后更新时间

    // 性能指标
    uint8_t overall_quality;        // 整体控制质量 (0-100)
    float system_stability;         // 系统稳定性指标
    uint16_t max_cycle_time_us;     // 最大控制周期时间 (微秒)
    uint16_t avg_cycle_time_us;     // 平均控制周期时间 (微秒)
} control_context_t;

/* ========================================================================== */
/* 控制命令结构 */
/* ========================================================================== */

typedef enum {
    CONTROL_CMD_SET_SETPOINT,       // 设置设定值
    CONTROL_CMD_SET_MODE,           // 设置控制模式
    CONTROL_CMD_ENABLE_LOOP,        // 使能控制回路
    CONTROL_CMD_DISABLE_LOOP,       // 禁用控制回路
    CONTROL_CMD_TUNE_PID,           // PID参数整定
    CONTROL_CMD_RESET_LOOP,         // 复位控制回路
    CONTROL_CMD_EMERGENCY_STOP,     // 紧急停止
    CONTROL_CMD_RESUME,             // 恢复运行
    CONTROL_CMD_UPDATE_PARAMS       // 更新参数
} control_cmd_type_t;

typedef struct {
    control_cmd_type_t cmd_type;    // 命令类型
    control_loop_t loop_id;         // 目标控制回路
    float value;                    // 命令值
    control_mode_t mode;            // 控制模式 (用于模式切换命令)
    pid_params_t pid_params;        // PID参数 (用于参数更新命令)
    uint32_t timestamp;             // 时间戳
    bool urgent;                    // 紧急标志
} control_command_t;

/* ========================================================================== */
/* 控制任务统计信息 */
/* ========================================================================== */

typedef struct {
    uint32_t total_cycles;          // 总控制周期数
    uint32_t command_count;         // 接收命令数
    uint32_t command_errors;        // 命令错误数
    uint32_t sensor_timeouts;       // 传感器超时次数
    uint32_t actuator_errors;       // 执行器错误次数
    uint32_t mode_switches;         // 模式切换次数
    uint32_t emergency_stops;       // 紧急停止次数
    uint16_t max_cycle_time_us;     // 最大周期时间 (微秒)
    uint16_t avg_cycle_time_us;     // 平均周期时间 (微秒)
    float avg_control_quality;     // 平均控制质量
    uint32_t quality_degradation;   // 质量下降次数
} control_task_stats_t;

/* ========================================================================== */
/* 消息队列数据结构 */
/* ========================================================================== */

// 消息类型
typedef enum {
    MSG_CONTROL_STATUS,             // 控制状态消息
    MSG_CONTROL_COMMAND,            // 控制命令消息
    MSG_CONTROL_ALARM,              // 控制报警消息
    MSG_CONTROL_QUALITY             // 控制质量消息
} control_msg_type_t;

// 控制消息结构
typedef struct {
    control_msg_type_t type;        // 消息类型
    uint32_t timestamp;             // 时间戳
    uint16_t data_len;              // 数据长度
    union {
        control_context_t context;  // 控制上下文
        control_command_t command;  // 控制命令
        control_loop_config_t loop; // 回路配置
        struct {
            control_loop_t loop_id;
            uint8_t alarm_type;
            float alarm_value;
        } alarm;                    // 报警信息
    } data;
} control_msg_t;

/* ========================================================================== */
/* 事件标志定义 */
/* ========================================================================== */

#define EVENT_CONTROL_UPDATE            (1 << 0)    // 控制更新
#define EVENT_CONTROL_ALARM             (1 << 1)    // 控制报警
#define EVENT_CONTROL_MODE_SWITCH       (1 << 2)    // 模式切换
#define EVENT_CONTROL_EMERGENCY         (1 << 3)    // 紧急事件
#define EVENT_CONTROL_TUNING            (1 << 4)    // 参数整定
#define EVENT_CONTROL_QUALITY_UPDATE    (1 << 5)    // 质量更新

/* ========================================================================== */
/* 队列大小定义 */
/* ========================================================================== */

#define CONTROL_CMD_QUEUE_SIZE          16          // 命令队列大小
#define CONTROL_MSG_QUEUE_SIZE          16          // 消息队列大小

/* ========================================================================== */
/* 控制参数定义 */
/* ========================================================================== */

#define CONTROL_QUALITY_THRESHOLD       80          // 控制质量阈值
#define CONTROL_SETPOINT_CHANGE_RATE    10.0f       // 设定值变化速率限制 (%/s)
#define CONTROL_OUTPUT_FILTER_COEFF     0.9f        // 输出滤波系数
#define CONTROL_AUTO_TUNE_CYCLES        50          // 自整定周期数
#define CONTROL_STABILITY_WINDOW        100         // 稳定性检测窗口

/* ========================================================================== */
/* 全局变量声明 */
/* ========================================================================== */

extern TaskHandle_t xTaskHandle_ControlV3;
extern QueueHandle_t xQueue_ControlCmd;
extern QueueHandle_t xQueue_ControlMsg;
extern SemaphoreHandle_t xMutex_ControlContext;
extern EventGroupHandle_t xEventGroup_Control;

/* ========================================================================== */
/* 公共函数声明 */
/* ========================================================================== */

/**
 * @brief 初始化控制任务系统
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t ControlTaskV3_Init(void);

/**
 * @brief 创建控制任务
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t ControlTaskV3_Create(void);

/**
 * @brief 控制任务主函数
 * @param pvParameters 任务参数
 */
void Task_ControlV3(void *pvParameters);

/**
 * @brief 设置控制回路设定值
 * @param loop_id 控制回路ID
 * @param setpoint 设定值
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_SetSetpoint(control_loop_t loop_id, float setpoint);

/**
 * @brief 设置控制模式
 * @param loop_id 控制回路ID
 * @param mode 控制模式
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_SetMode(control_loop_t loop_id, control_mode_t mode);

/**
 * @brief 使能控制回路
 * @param loop_id 控制回路ID
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_EnableLoop(control_loop_t loop_id);

/**
 * @brief 禁用控制回路
 * @param loop_id 控制回路ID
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_DisableLoop(control_loop_t loop_id);

/**
 * @brief 设置PID参数
 * @param loop_id 控制回路ID
 * @param params PID参数结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_SetPIDParams(control_loop_t loop_id, const pid_params_t *params);

/**
 * @brief 启动PID自整定
 * @param loop_id 控制回路ID
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_StartAutoTune(control_loop_t loop_id);

/**
 * @brief 停止PID自整定
 * @param loop_id 控制回路ID
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_StopAutoTune(control_loop_t loop_id);

/**
 * @brief 紧急停止所有控制回路
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_EmergencyStop(void);

/**
 * @brief 恢复控制系统运行
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_Resume(void);

/**
 * @brief 复位控制回路
 * @param loop_id 控制回路ID
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_ResetLoop(control_loop_t loop_id);

/**
 * @brief 发送控制命令
 * @param command 命令结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_SendCommand(const control_command_t *command, uint32_t timeout_ms);

/**
 * @brief 获取控制系统上下文
 * @param context 上下文结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_GetContext(control_context_t *context);

/**
 * @brief 获取控制回路配置
 * @param loop_id 控制回路ID
 * @param config 配置结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_GetLoopConfig(control_loop_t loop_id, control_loop_config_t *config);

/**
 * @brief 获取PID状态
 * @param loop_id 控制回路ID
 * @param state PID状态结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_GetPIDState(control_loop_t loop_id, pid_state_t *state);

/**
 * @brief 发送控制消息到队列
 * @param msg 消息结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_SendMessage(const control_msg_t *msg, uint32_t timeout_ms);

/**
 * @brief 从队列接收控制消息
 * @param msg 消息结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_ReceiveMessage(control_msg_t *msg, uint32_t timeout_ms);

/**
 * @brief 获取控制任务统计信息
 * @param stats 统计信息结构指针
 */
void ControlTaskV3_GetStatistics(control_task_stats_t *stats);

/**
 * @brief 重置控制任务统计信息
 */
void ControlTaskV3_ResetStatistics(void);

/**
 * @brief 检查控制系统健康状态
 * @return 健康分数 (0-100)
 */
uint8_t ControlTaskV3_CheckHealth(void);

/**
 * @brief 计算控制质量分数
 * @param loop_id 控制回路ID
 * @return 质量分数 (0-100)
 */
uint8_t ControlTaskV3_CalculateQuality(control_loop_t loop_id);

/**
 * @brief 检查系统稳定性
 * @return 稳定性指标 (0.0-1.0)
 */
float ControlTaskV3_CheckStability(void);

/**
 * @brief 获取控制回路过程值
 * @param loop_id 控制回路ID
 * @return 过程值
 */
float ControlTaskV3_GetProcessValue(control_loop_t loop_id);

/**
 * @brief 获取控制回路输出值
 * @param loop_id 控制回路ID
 * @return 输出值
 */
float ControlTaskV3_GetOutputValue(control_loop_t loop_id);

/**
 * @brief 检查控制回路是否在自动模式
 * @param loop_id 控制回路ID
 * @return true=自动模式, false=手动模式
 */
bool ControlTaskV3_IsAutoMode(control_loop_t loop_id);

/**
 * @brief 检查控制回路是否有报警
 * @param loop_id 控制回路ID
 * @return true=有报警, false=无报警
 */
bool ControlTaskV3_HasAlarm(control_loop_t loop_id);

/**
 * @brief 检查系统是否处于紧急停止状态
 * @return true=紧急停止, false=正常
 */
bool ControlTaskV3_IsEmergencyStopped(void);

/**
 * @brief 检查系统是否处于安全模式
 * @return true=安全模式, false=正常模式
 */
bool ControlTaskV3_IsInSafetyMode(void);

#ifdef __cplusplus
}
#endif

#endif /* __CONTROL_TASK_V3_H */

/************************ (C) COPYRIGHT Ink Supply Control System *****END OF FILE****/