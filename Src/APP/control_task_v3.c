/**
 ******************************************************************************
 * @file    control_task_v3.c
 * @brief   控制任务实现 - 基于V3设计文档
 * @author  Ink Supply Control System Development Team
 * @version V3.0.0
 * @date    2025-01-23
 ******************************************************************************
 * @attention
 *
 * 本文件基于墨路控制系统详细设计文档V3实现，采用四层架构设计
 *
 * 主要功能:
 * 1. 实现12个控制回路的PID控制算法
 * 2. 传感器数据融合和处理
 * 3. 执行器输出控制和协调
 * 4. 控制模式切换和参数管理
 * 5. 系统安全监控和故障处理
 * 6. 控制质量评估和优化
 ******************************************************************************
 */

#include "control_task_v3.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ========================================================================== */
/* 私有宏定义 */
/* ========================================================================== */

#define PID_INTEGRAL_LIMIT_FACTOR   2.0f    // 积分限制因子
#define PID_DERIVATIVE_FILTER_MIN   0.1f    // 微分滤波最小值
#define SETPOINT_FILTER_COEFF       0.95f   // 设定值滤波系数
#define PROCESS_VALUE_TIMEOUT_MS    200     // 过程值超时时间
#define CONTROL_QUALITY_SAMPLES     20      // 质量计算样本数
#define STABILITY_CHECK_CYCLES      50      // 稳定性检查周期

/* ========================================================================== */
/* 全局变量定义 */
/* ========================================================================== */

// 任务句柄
TaskHandle_t xTaskHandle_ControlV3 = NULL;

// 队列句柄
QueueHandle_t xQueue_ControlCmd = NULL;
QueueHandle_t xQueue_ControlMsg = NULL;

// 互斥体
SemaphoreHandle_t xMutex_ControlContext = NULL;

// 事件组
EventGroupHandle_t xEventGroup_Control = NULL;

/* ========================================================================== */
/* 私有变量 */
/* ========================================================================== */

// 控制系统上下文
static control_context_t g_control_context = {0};

// 控制任务统计信息
static control_task_stats_t g_control_stats = {0};

// 控制质量历史缓冲区 (用于计算平均值)
static float g_quality_history[CONTROL_LOOP_COUNT][CONTROL_QUALITY_SAMPLES] = {0};
static uint8_t g_quality_index[CONTROL_LOOP_COUNT] = {0};

// 稳定性检测缓冲区
static float g_stability_buffer[CONTROL_LOOP_COUNT][STABILITY_CHECK_CYCLES] = {0};
static uint8_t g_stability_index[CONTROL_LOOP_COUNT] = {0};

/* ========================================================================== */
/* 私有函数声明 */
/* ========================================================================== */

static void Control_InitializeLoops(void);
static void Control_ProcessCommands(void);
static void Control_UpdateSensorData(void);
static void Control_ExecuteControlLoops(void);
static void Control_UpdateActuators(void);
static void Control_CheckAlarms(void);
static void Control_UpdateQuality(void);
static void Control_CheckStability(void);
static void Control_SendStatusMessage(void);

// PID控制器相关函数
static float PID_Calculate(control_loop_t loop_id, float setpoint, float process_value);
static void PID_Reset(control_loop_t loop_id);
static void PID_SetParams(control_loop_t loop_id, const pid_params_t *params);
static float PID_AntiWindup(control_loop_t loop_id, float output);

// 传感器数据映射函数
static float Control_GetSensorValue(control_loop_t loop_id);
static bool Control_IsSensorValid(control_loop_t loop_id);

// 执行器控制映射函数
static BaseType_t Control_SetActuatorOutput(control_loop_t loop_id, float output);

// 控制质量和稳定性评估
static float Control_CalculateLoopQuality(control_loop_t loop_id);
static float Control_CalculateLoopStability(control_loop_t loop_id);

/* ========================================================================== */
/* 公共函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化控制任务系统
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t ControlTaskV3_Init(void)
{
    // 创建互斥体
    xMutex_ControlContext = xSemaphoreCreateMutex();
    if (xMutex_ControlContext == NULL) {
        printf("[ControlV3] ERROR: 创建互斥体失败\r\n");
        return pdFAIL;
    }

    // 创建命令队列
    xQueue_ControlCmd = xQueueCreate(CONTROL_CMD_QUEUE_SIZE, sizeof(control_command_t));
    if (xQueue_ControlCmd == NULL) {
        printf("[ControlV3] ERROR: 创建命令队列失败\r\n");
        return pdFAIL;
    }

    // 创建消息队列
    xQueue_ControlMsg = xQueueCreate(CONTROL_MSG_QUEUE_SIZE, sizeof(control_msg_t));
    if (xQueue_ControlMsg == NULL) {
        printf("[ControlV3] ERROR: 创建消息队列失败\r\n");
        return pdFAIL;
    }

    // 创建事件组
    xEventGroup_Control = xEventGroupCreate();
    if (xEventGroup_Control == NULL) {
        printf("[ControlV3] ERROR: 创建事件组失败\r\n");
        return pdFAIL;
    }

    // 初始化控制回路
    Control_InitializeLoops();

    // 初始化上下文
    memset(&g_control_context, 0, sizeof(control_context_t));
    g_control_context.system_mode = CONTROL_MODE_MANUAL;
    g_control_context.system_state = CONTROL_STATE_IDLE;
    g_control_context.system_enabled = false;
    g_control_context.emergency_stop = false;
    g_control_context.safety_mode = false;

    // 初始化统计信息
    memset(&g_control_stats, 0, sizeof(control_task_stats_t));

    printf("[ControlV3] 控制任务系统初始化成功\r\n");
    return pdPASS;
}

/**
 * @brief 创建控制任务
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t ControlTaskV3_Create(void)
{
    BaseType_t result;

    result = xTaskCreate(
        Task_ControlV3,
        "ControlV3",
        CONTROL_TASK_STACK_SIZE,
        NULL,
        CONTROL_TASK_PRIORITY,
        &xTaskHandle_ControlV3
    );

    if (result != pdPASS) {
        printf("[ControlV3] ERROR: 创建任务失败\r\n");
        return pdFAIL;
    }

    printf("[ControlV3] 控制任务创建成功\r\n");
    return pdPASS;
}

/**
 * @brief 控制任务主函数
 * @param pvParameters 任务参数
 */
void Task_ControlV3(void *pvParameters)
{
    TickType_t xLastWakeTime;
    uint32_t cycle_start_time, cycle_end_time;

    // 初始化延时基准时间
    xLastWakeTime = xTaskGetTickCount();

    printf("[ControlV3] 控制任务启动 - 周期: %d ms, 优先级: %d\r\n",
           CONTROL_TASK_PERIOD_MS, CONTROL_TASK_PRIORITY);

    // 等待系统初始化完成
    vTaskDelay(pdMS_TO_TICKS(100));

    // 设置系统就绪标志
    g_control_context.system_enabled = true;
    g_control_context.system_state = CONTROL_STATE_RUNNING;

    for (;;)
    {
        // 记录周期开始时间
        cycle_start_time = HAL_GetTick();

        // 1. 处理命令队列中的命令
        Control_ProcessCommands();

        // 2. 更新传感器数据
        Control_UpdateSensorData();

        // 3. 执行控制算法 (如果系统未紧急停止)
        if (!g_control_context.emergency_stop) {
            Control_ExecuteControlLoops();
        }

        // 4. 更新执行器输出
        Control_UpdateActuators();

        // 5. 检查报警和安全状态
        Control_CheckAlarms();

        // 6. 更新控制质量评估
        Control_UpdateQuality();

        // 7. 检查系统稳定性
        Control_CheckStability();

        // 8. 发送状态消息 (每5个周期发送一次)
        if ((g_control_context.cycle_count % 5) == 0) {
            Control_SendStatusMessage();
        }

        // 9. 更新统计信息
        cycle_end_time = HAL_GetTick();
        uint32_t cycle_time_us = (cycle_end_time - cycle_start_time) * 1000;

        g_control_stats.total_cycles++;
        if (cycle_time_us > g_control_stats.max_cycle_time_us) {
            g_control_stats.max_cycle_time_us = (uint16_t)cycle_time_us;
        }

        // 计算平均周期时间
        g_control_stats.avg_cycle_time_us = (uint16_t)(
            (g_control_stats.avg_cycle_time_us * (g_control_stats.total_cycles - 1) + cycle_time_us) /
            g_control_stats.total_cycles
        );

        // 更新上下文中的周期时间统计
        g_control_context.max_cycle_time_us = g_control_stats.max_cycle_time_us;
        g_control_context.avg_cycle_time_us = g_control_stats.avg_cycle_time_us;

        // 10. 定期打印调试信息 (每100个周期 = 2秒)
        if ((g_control_context.cycle_count % 100) == 0) {
            printf("[ControlV3] 周期=%lu, 质量=%d%%, 稳定性=%.2f, 执行时间=%luμs\r\n",
                   g_control_context.cycle_count,
                   g_control_context.overall_quality,
                   g_control_context.system_stability,
                   cycle_time_us);
        }

        // 11. 更新周期计数和时间戳
        g_control_context.cycle_count++;
        g_control_context.last_update_time = HAL_GetTick();

        // 12. 按照固定周期执行
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CONTROL_TASK_PERIOD_MS));
    }
}

/**
 * @brief 设置控制回路设定值
 * @param loop_id 控制回路ID
 * @param setpoint 设定值
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_SetSetpoint(control_loop_t loop_id, float setpoint)
{
    control_command_t command;

    if (loop_id >= CONTROL_LOOP_COUNT) {
        return pdFALSE;
    }

    command.cmd_type = CONTROL_CMD_SET_SETPOINT;
    command.loop_id = loop_id;
    command.value = setpoint;
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    return ControlTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 设置控制模式
 * @param loop_id 控制回路ID
 * @param mode 控制模式
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_SetMode(control_loop_t loop_id, control_mode_t mode)
{
    control_command_t command;

    if (loop_id >= CONTROL_LOOP_COUNT) {
        return pdFALSE;
    }

    command.cmd_type = CONTROL_CMD_SET_MODE;
    command.loop_id = loop_id;
    command.mode = mode;
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    return ControlTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 紧急停止所有控制回路
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_EmergencyStop(void)
{
    control_command_t command;

    command.cmd_type = CONTROL_CMD_EMERGENCY_STOP;
    command.loop_id = CONTROL_LOOP_TEMP_1;  // 不关心具体回路
    command.timestamp = HAL_GetTick();
    command.urgent = true;

    printf("[ControlV3] 紧急停止触发!\r\n");

    return ControlTaskV3_SendCommand(&command, 0);  // 立即发送
}

/**
 * @brief 发送控制命令
 * @param command 命令结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_SendCommand(const control_command_t *command, uint32_t timeout_ms)
{
    if (command == NULL) {
        return pdFALSE;
    }

    BaseType_t result = xQueueSend(xQueue_ControlCmd, command, pdMS_TO_TICKS(timeout_ms));

    if (result == pdPASS) {
        g_control_stats.command_count++;
        xEventGroupSetBits(xEventGroup_Control, EVENT_CONTROL_UPDATE);
    } else {
        g_control_stats.command_errors++;
    }

    return result;
}

/**
 * @brief 获取控制系统上下文
 * @param context 上下文结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_GetContext(control_context_t *context)
{
    if (context == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_ControlContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(context, &g_control_context, sizeof(control_context_t));
        xSemaphoreGive(xMutex_ControlContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 检查控制系统健康状态
 * @return 健康分数 (0-100)
 */
uint8_t ControlTaskV3_CheckHealth(void)
{
    uint8_t health_score = 100;
    uint8_t error_count = 0;

    // 检查各控制回路状态
    for (uint8_t i = 0; i < CONTROL_LOOP_COUNT; i++) {
        control_loop_config_t *loop = &g_control_context.loops[i];

        if (loop->enabled) {
            // 检查回路是否有报警
            if (loop->alarm_status) {
                error_count++;
            }

            // 检查控制质量
            if (loop->control_quality < CONTROL_QUALITY_THRESHOLD) {
                error_count++;
            }

            // 检查传感器数据有效性
            if (!Control_IsSensorValid((control_loop_t)i)) {
                error_count++;
            }
        }
    }

    // 根据错误数量计算健康分数
    if (error_count > 0) {
        health_score = (uint8_t)(100 - (error_count * 100) / (CONTROL_LOOP_COUNT * 3));
    }

    // 紧急停止状态影响健康分数
    if (g_control_context.emergency_stop) {
        health_score = (uint8_t)(health_score * 0.3f);
    }

    // 安全模式影响健康分数
    if (g_control_context.safety_mode) {
        health_score = (uint8_t)(health_score * 0.7f);
    }

    return health_score;
}

/**
 * @brief 获取控制任务统计信息
 * @param stats 统计信息结构指针
 */
void ControlTaskV3_GetStatistics(control_task_stats_t *stats)
{
    if (stats != NULL) {
        memcpy(stats, &g_control_stats, sizeof(control_task_stats_t));
    }
}

/* ========================================================================== */
/* 私有函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化控制回路
 */
static void Control_InitializeLoops(void)
{
    // 温度控制回路初始化
    for (uint8_t i = CONTROL_LOOP_TEMP_1; i <= CONTROL_LOOP_TEMP_3; i++) {
        control_loop_config_t *loop = &g_control_context.loops[i];

        loop->loop_id = (control_loop_t)i;
        loop->mode = CONTROL_MODE_MANUAL;
        loop->state = CONTROL_STATE_IDLE;
        loop->sensor_type = SENSOR_TEMP_1 + (i - CONTROL_LOOP_TEMP_1);
        loop->actuator_type = ACTUATOR_HEATER_1 + (i - CONTROL_LOOP_TEMP_1);

        // 默认PID参数 (温度控制)
        loop->pid_params.kp = 2.0f;
        loop->pid_params.ki = 0.1f;
        loop->pid_params.kd = 0.05f;
        loop->pid_params.output_min = 0.0f;
        loop->pid_params.output_max = 100.0f;
        loop->pid_params.integral_min = -50.0f;
        loop->pid_params.integral_max = 50.0f;
        loop->pid_params.derivative_filter = 0.1f;
        loop->pid_params.deadband = 0.5f;
        loop->pid_params.sample_time = CONTROL_TASK_PERIOD_MS / 1000.0f;
        loop->pid_params.enabled = true;
        loop->pid_params.integral_enabled = true;
        loop->pid_params.derivative_enabled = true;
        loop->pid_params.anti_windup_enabled = true;

        // 设定值范围 (温度: 20-80°C)
        loop->setpoint = 25.0f;
        loop->setpoint_min = 20.0f;
        loop->setpoint_max = 80.0f;

        // 报警限制 (温度)
        loop->alarm_high = 75.0f;
        loop->alarm_low = 15.0f;
        loop->warning_high = 70.0f;
        loop->warning_low = 18.0f;

        loop->enabled = false;
        loop->auto_mode = false;

        // 初始化PID状态
        loop->pid_state.first_run = true;
        loop->pid_state.last_update_time = HAL_GetTick();
    }

    // 压力控制回路初始化
    for (uint8_t i = CONTROL_LOOP_PRESSURE_1; i <= CONTROL_LOOP_PRESSURE_4; i++) {
        control_loop_config_t *loop = &g_control_context.loops[i];

        loop->loop_id = (control_loop_t)i;
        loop->mode = CONTROL_MODE_MANUAL;
        loop->state = CONTROL_STATE_IDLE;
        loop->sensor_type = SENSOR_PRESSURE_1 + (i - CONTROL_LOOP_PRESSURE_1);
        loop->actuator_type = ACTUATOR_PUMP_SPEED_1 + ((i - CONTROL_LOOP_PRESSURE_1) % 2);

        // 默认PID参数 (压力控制)
        loop->pid_params.kp = 1.5f;
        loop->pid_params.ki = 0.2f;
        loop->pid_params.kd = 0.02f;
        loop->pid_params.output_min = 0.0f;
        loop->pid_params.output_max = 100.0f;
        loop->pid_params.integral_min = -30.0f;
        loop->pid_params.integral_max = 30.0f;
        loop->pid_params.derivative_filter = 0.2f;
        loop->pid_params.deadband = 1.0f;
        loop->pid_params.sample_time = CONTROL_TASK_PERIOD_MS / 1000.0f;
        loop->pid_params.enabled = true;
        loop->pid_params.integral_enabled = true;
        loop->pid_params.derivative_enabled = true;
        loop->pid_params.anti_windup_enabled = true;

        // 设定值范围 (压力: 50-200kPa)
        loop->setpoint = 100.0f;
        loop->setpoint_min = 50.0f;
        loop->setpoint_max = 200.0f;

        // 报警限制 (压力)
        loop->alarm_high = 180.0f;
        loop->alarm_low = 40.0f;
        loop->warning_high = 160.0f;
        loop->warning_low = 60.0f;

        loop->enabled = false;
        loop->auto_mode = false;

        // 初始化PID状态
        loop->pid_state.first_run = true;
        loop->pid_state.last_update_time = HAL_GetTick();
    }

    // 液位控制回路初始化
    for (uint8_t i = CONTROL_LOOP_LEVEL_1; i <= CONTROL_LOOP_LEVEL_4; i++) {
        control_loop_config_t *loop = &g_control_context.loops[i];

        loop->loop_id = (control_loop_t)i;
        loop->mode = CONTROL_MODE_MANUAL;
        loop->state = CONTROL_STATE_IDLE;
        loop->sensor_type = SENSOR_LEVEL_1 + (i - CONTROL_LOOP_LEVEL_1);
        loop->actuator_type = ACTUATOR_VALVE_1 + ((i - CONTROL_LOOP_LEVEL_1) % 2);

        // 默认PID参数 (液位控制)
        loop->pid_params.kp = 1.0f;
        loop->pid_params.ki = 0.05f;
        loop->pid_params.kd = 0.01f;
        loop->pid_params.output_min = 0.0f;
        loop->pid_params.output_max = 100.0f;
        loop->pid_params.integral_min = -20.0f;
        loop->pid_params.integral_max = 20.0f;
        loop->pid_params.derivative_filter = 0.3f;
        loop->pid_params.deadband = 2.0f;
        loop->pid_params.sample_time = CONTROL_TASK_PERIOD_MS / 1000.0f;
        loop->pid_params.enabled = true;
        loop->pid_params.integral_enabled = true;
        loop->pid_params.derivative_enabled = true;
        loop->pid_params.anti_windup_enabled = true;

        // 设定值范围 (液位: 10-90mm)
        loop->setpoint = 50.0f;
        loop->setpoint_min = 10.0f;
        loop->setpoint_max = 90.0f;

        // 报警限制 (液位)
        loop->alarm_high = 85.0f;
        loop->alarm_low = 5.0f;
        loop->warning_high = 80.0f;
        loop->warning_low = 15.0f;

        loop->enabled = false;
        loop->auto_mode = false;

        // 初始化PID状态
        loop->pid_state.first_run = true;
        loop->pid_state.last_update_time = HAL_GetTick();
    }

    // 流量控制回路初始化
    control_loop_config_t *flow_loop = &g_control_context.loops[CONTROL_LOOP_FLOW];
    flow_loop->loop_id = CONTROL_LOOP_FLOW;
    flow_loop->mode = CONTROL_MODE_MANUAL;
    flow_loop->state = CONTROL_STATE_IDLE;
    flow_loop->sensor_type = SENSOR_FLOW;
    flow_loop->actuator_type = ACTUATOR_PUMP_SPEED_1;

    // 默认PID参数 (流量控制)
    flow_loop->pid_params.kp = 3.0f;
    flow_loop->pid_params.ki = 0.3f;
    flow_loop->pid_params.kd = 0.1f;
    flow_loop->pid_params.output_min = 0.0f;
    flow_loop->pid_params.output_max = 100.0f;
    flow_loop->pid_params.integral_min = -40.0f;
    flow_loop->pid_params.integral_max = 40.0f;
    flow_loop->pid_params.derivative_filter = 0.15f;
    flow_loop->pid_params.deadband = 0.2f;
    flow_loop->pid_params.sample_time = CONTROL_TASK_PERIOD_MS / 1000.0f;
    flow_loop->pid_params.enabled = true;
    flow_loop->pid_params.integral_enabled = true;
    flow_loop->pid_params.derivative_enabled = true;
    flow_loop->pid_params.anti_windup_enabled = true;

    // 设定值范围 (流量: 0.5-10 L/min)
    flow_loop->setpoint = 2.0f;
    flow_loop->setpoint_min = 0.5f;
    flow_loop->setpoint_max = 10.0f;

    // 报警限制 (流量)
    flow_loop->alarm_high = 9.0f;
    flow_loop->alarm_low = 0.2f;
    flow_loop->warning_high = 8.0f;
    flow_loop->warning_low = 0.5f;

    flow_loop->enabled = false;
    flow_loop->auto_mode = false;

    // 初始化PID状态
    flow_loop->pid_state.first_run = true;
    flow_loop->pid_state.last_update_time = HAL_GetTick();

    printf("[ControlV3] 控制回路配置初始化完成\r\n");
}

/**
 * @brief 处理命令队列中的命令
 */
static void Control_ProcessCommands(void)
{
    control_command_t command;

    // 处理所有待处理的命令
    while (xQueueReceive(xQueue_ControlCmd, &command, 0) == pdPASS) {
        // 验证命令参数
        if (command.loop_id >= CONTROL_LOOP_COUNT) {
            g_control_stats.command_errors++;
            continue;
        }

        control_loop_config_t *loop = &g_control_context.loops[command.loop_id];

        // 处理不同类型的命令
        switch (command.cmd_type) {
            case CONTROL_CMD_SET_SETPOINT:
                // 限制设定值范围
                if (command.value < loop->setpoint_min) {
                    command.value = loop->setpoint_min;
                }
                if (command.value > loop->setpoint_max) {
                    command.value = loop->setpoint_max;
                }
                loop->setpoint = command.value;
                loop->pid_state.setpoint = command.value;
                break;

            case CONTROL_CMD_SET_MODE:
                loop->mode = command.mode;
                loop->auto_mode = (command.mode == CONTROL_MODE_AUTO);
                g_control_stats.mode_switches++;
                xEventGroupSetBits(xEventGroup_Control, EVENT_CONTROL_MODE_SWITCH);
                break;

            case CONTROL_CMD_ENABLE_LOOP:
                loop->enabled = true;
                loop->state = CONTROL_STATE_RUNNING;
                PID_Reset(command.loop_id);
                break;

            case CONTROL_CMD_DISABLE_LOOP:
                loop->enabled = false;
                loop->state = CONTROL_STATE_IDLE;
                loop->output_value = 0.0f;
                break;

            case CONTROL_CMD_RESET_LOOP:
                PID_Reset(command.loop_id);
                loop->state = CONTROL_STATE_IDLE;
                break;

            case CONTROL_CMD_EMERGENCY_STOP:
                g_control_context.emergency_stop = true;
                g_control_context.safety_mode = true;
                for (uint8_t i = 0; i < CONTROL_LOOP_COUNT; i++) {
                    g_control_context.loops[i].enabled = false;
                    g_control_context.loops[i].state = CONTROL_STATE_SAFETY;
                    g_control_context.loops[i].output_value = 0.0f;
                }
                g_control_stats.emergency_stops++;
                xEventGroupSetBits(xEventGroup_Control, EVENT_CONTROL_EMERGENCY);
                break;

            case CONTROL_CMD_RESUME:
                g_control_context.emergency_stop = false;
                g_control_context.safety_mode = false;
                break;

            case CONTROL_CMD_UPDATE_PARAMS:
                PID_SetParams(command.loop_id, &command.pid_params);
                break;

            default:
                g_control_stats.command_errors++;
                break;
        }
    }
}

/**
 * @brief 更新传感器数据
 */
static void Control_UpdateSensorData(void)
{
    // 从传感器任务获取最新数据
    BaseType_t result = SensorTaskV3_GetContext(&g_control_context.sensor_data);

    if (result == pdTRUE) {
        g_control_context.sensor_data_valid = true;
        g_control_context.sensor_data_age = 0;
    } else {
        g_control_context.sensor_data_age += CONTROL_TASK_PERIOD_MS;

        // 检查数据超时
        if (g_control_context.sensor_data_age > PROCESS_VALUE_TIMEOUT_MS) {
            g_control_context.sensor_data_valid = false;
            g_control_stats.sensor_timeouts++;
        }
    }
}

/**
 * @brief 执行所有控制回路
 */
static void Control_ExecuteControlLoops(void)
{
    for (uint8_t i = 0; i < CONTROL_LOOP_COUNT; i++) {
        control_loop_config_t *loop = &g_control_context.loops[i];

        // 跳过未使能或非自动模式的回路
        if (!loop->enabled || !loop->auto_mode) {
            continue;
        }

        // 获取过程值
        float process_value = Control_GetSensorValue((control_loop_t)i);
        loop->process_value = process_value;

        // 检查传感器数据有效性
        if (!Control_IsSensorValid((control_loop_t)i)) {
            loop->state = CONTROL_STATE_ERROR;
            continue;
        }

        // 执行PID控制
        float output = PID_Calculate((control_loop_t)i, loop->setpoint, process_value);
        loop->output_value = output;
        loop->state = CONTROL_STATE_RUNNING;

        // 更新回路统计
        loop->total_run_time += CONTROL_TASK_PERIOD_MS;
        loop->last_update_time = HAL_GetTick();
    }
}

/**
 * @brief 更新执行器输出
 */
static void Control_UpdateActuators(void)
{
    for (uint8_t i = 0; i < CONTROL_LOOP_COUNT; i++) {
        control_loop_config_t *loop = &g_control_context.loops[i];

        // 只更新使能且自动模式的回路
        if (!loop->enabled || !loop->auto_mode) {
            continue;
        }

        // 设置执行器输出
        BaseType_t result = Control_SetActuatorOutput((control_loop_t)i, loop->output_value);

        if (result != pdTRUE) {
            g_control_stats.actuator_errors++;
        }
    }

    // 更新执行器状态时间戳
    g_control_context.actuator_update_time = HAL_GetTick();
    g_control_context.actuator_states_valid = true;
}

/**
 * @brief 检查报警和安全状态
 */
static void Control_CheckAlarms(void)
{
    for (uint8_t i = 0; i < CONTROL_LOOP_COUNT; i++) {
        control_loop_config_t *loop = &g_control_context.loops[i];

        if (!loop->enabled) {
            continue;
        }

        float pv = loop->process_value;

        // 检查高报警
        if (pv > loop->alarm_high) {
            if (!loop->alarm_status) {
                loop->alarm_status = true;
                xEventGroupSetBits(xEventGroup_Control, EVENT_CONTROL_ALARM);
            }
        }
        // 检查低报警
        else if (pv < loop->alarm_low) {
            if (!loop->alarm_status) {
                loop->alarm_status = true;
                xEventGroupSetBits(xEventGroup_Control, EVENT_CONTROL_ALARM);
            }
        }
        // 检查警告限制
        else if (pv > loop->warning_high || pv < loop->warning_low) {
            loop->warning_status = true;
        }
        else {
            loop->alarm_status = false;
            loop->warning_status = false;
        }
    }
}

/**
 * @brief 更新控制质量评估
 */
static void Control_UpdateQuality(void)
{
    uint32_t total_quality = 0;
    uint8_t enabled_loops = 0;

    for (uint8_t i = 0; i < CONTROL_LOOP_COUNT; i++) {
        control_loop_config_t *loop = &g_control_context.loops[i];

        if (loop->enabled && loop->auto_mode) {
            float quality = Control_CalculateLoopQuality((control_loop_t)i);
            loop->control_quality = quality;

            // 存储质量历史
            uint8_t idx = g_quality_index[i];
            g_quality_history[i][idx] = quality;
            g_quality_index[i] = (idx + 1) % CONTROL_QUALITY_SAMPLES;

            total_quality += (uint32_t)quality;
            enabled_loops++;
            loop->quality_update_count++;
        }
    }

    // 计算整体控制质量
    if (enabled_loops > 0) {
        g_control_context.overall_quality = (uint8_t)(total_quality / enabled_loops);
    }

    // 更新统计信息
    g_control_stats.avg_control_quality = (float)g_control_context.overall_quality;

    // 设置质量更新事件
    xEventGroupSetBits(xEventGroup_Control, EVENT_CONTROL_QUALITY_UPDATE);
}

/**
 * @brief 检查系统稳定性
 */
static void Control_CheckStability(void)
{
    float total_stability = 0.0f;
    uint8_t enabled_loops = 0;

    for (uint8_t i = 0; i < CONTROL_LOOP_COUNT; i++) {
        control_loop_config_t *loop = &g_control_context.loops[i];

        if (loop->enabled && loop->auto_mode) {
            float stability = Control_CalculateLoopStability((control_loop_t)i);

            // 存储稳定性历史
            uint8_t idx = g_stability_index[i];
            g_stability_buffer[i][idx] = stability;
            g_stability_index[i] = (idx + 1) % STABILITY_CHECK_CYCLES;

            total_stability += stability;
            enabled_loops++;
        }
    }

    // 计算整体系统稳定性
    if (enabled_loops > 0) {
        g_control_context.system_stability = total_stability / enabled_loops;
    }
}

/**
 * @brief 发送状态消息
 */
static void Control_SendStatusMessage(void)
{
    control_msg_t status_msg;

    status_msg.type = MSG_CONTROL_STATUS;
    status_msg.timestamp = HAL_GetTick();
    status_msg.data_len = sizeof(control_context_t);

    // 获取互斥体并复制上下文
    if (xSemaphoreTake(xMutex_ControlContext, pdMS_TO_TICKS(5)) == pdTRUE) {
        memcpy(&status_msg.data.context, &g_control_context, sizeof(control_context_t));
        xSemaphoreGive(xMutex_ControlContext);

        // 发送消息到队列
        xQueueSend(xQueue_ControlMsg, &status_msg, 0);
    }
}

/**
 * @brief PID控制器计算
 * @param loop_id 控制回路ID
 * @param setpoint 设定值
 * @param process_value 过程值
 * @return 控制输出
 */
static float PID_Calculate(control_loop_t loop_id, float setpoint, float process_value)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return 0.0f;
    }

    control_loop_config_t *loop = &g_control_context.loops[loop_id];
    pid_params_t *params = &loop->pid_params;
    pid_state_t *state = &loop->pid_state;

    if (!params->enabled) {
        return 0.0f;
    }

    // 计算误差
    float error = setpoint - process_value;
    state->error = error;

    // 死区处理
    if (fabsf(error) < params->deadband) {
        state->in_deadband = true;
        error = 0.0f;
    } else {
        state->in_deadband = false;
    }

    // 比例项
    float p_term = params->kp * error;

    // 积分项
    float i_term = 0.0f;
    if (params->integral_enabled && !state->in_deadband) {
        state->integral += error * params->sample_time;

        // 积分限幅
        if (state->integral > params->integral_max) {
            state->integral = params->integral_max;
        }
        if (state->integral < params->integral_min) {
            state->integral = params->integral_min;
        }

        i_term = params->ki * state->integral;
    }

    // 微分项
    float d_term = 0.0f;
    if (params->derivative_enabled && !state->first_run) {
        float derivative = (error - state->last_error) / params->sample_time;

        // 微分滤波
        state->filtered_derivative = params->derivative_filter * derivative +
                                     (1.0f - params->derivative_filter) * state->filtered_derivative;

        d_term = params->kd * state->filtered_derivative;
        state->derivative = state->filtered_derivative;
    }

    // 计算总输出
    float output = p_term + i_term + d_term;

    // 输出限幅
    if (output > params->output_max) {
        output = params->output_max;
        state->output_saturated = true;

        // 抗积分饱和
        if (params->anti_windup_enabled && params->integral_enabled) {
            state->integral -= error * params->sample_time;
        }
    } else if (output < params->output_min) {
        output = params->output_min;
        state->output_saturated = true;

        // 抗积分饱和
        if (params->anti_windup_enabled && params->integral_enabled) {
            state->integral -= error * params->sample_time;
        }
    } else {
        state->output_saturated = false;
    }

    // 更新状态
    state->last_error = error;
    state->output = output;
    state->process_value = process_value;
    state->setpoint = setpoint;
    state->last_update_time = HAL_GetTick();
    state->cycle_count++;
    state->first_run = false;

    // 更新统计信息
    if (fabsf(error) > state->max_error) {
        state->max_error = fabsf(error);
    }
    state->avg_error = (state->avg_error * (state->cycle_count - 1) + fabsf(error)) / state->cycle_count;

    return output;
}

/**
 * @brief 复位PID控制器
 * @param loop_id 控制回路ID
 */
static void PID_Reset(control_loop_t loop_id)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return;
    }

    pid_state_t *state = &g_control_context.loops[loop_id].pid_state;

    state->error = 0.0f;
    state->last_error = 0.0f;
    state->integral = 0.0f;
    state->derivative = 0.0f;
    state->filtered_derivative = 0.0f;
    state->output = 0.0f;
    state->first_run = true;
    state->in_deadband = false;
    state->output_saturated = false;
    state->cycle_count = 0;
    state->max_error = 0.0f;
    state->avg_error = 0.0f;
}

/**
 * @brief 设置PID参数
 * @param loop_id 控制回路ID
 * @param params PID参数结构指针
 */
static void PID_SetParams(control_loop_t loop_id, const pid_params_t *params)
{
    if (loop_id >= CONTROL_LOOP_COUNT || params == NULL) {
        return;
    }

    memcpy(&g_control_context.loops[loop_id].pid_params, params, sizeof(pid_params_t));
}

/**
 * @brief 抗积分饱和处理
 * @param loop_id 控制回路ID
 * @param output 输出值
 * @return 处理后的输出值
 */
static float PID_AntiWindup(control_loop_t loop_id, float output)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return output;
    }

    control_loop_config_t *loop = &g_control_context.loops[loop_id];
    pid_params_t *params = &loop->pid_params;

    // 限制输出范围
    if (output > params->output_max) {
        return params->output_max;
    }
    if (output < params->output_min) {
        return params->output_min;
    }

    return output;
}

/**
 * @brief 获取传感器值
 * @param loop_id 控制回路ID
 * @return 传感器值
 */
static float Control_GetSensorValue(control_loop_t loop_id)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return 0.0f;
    }

    control_loop_config_t *loop = &g_control_context.loops[loop_id];
    sensor_type_t sensor_type = loop->sensor_type;

    // 从传感器上下文中获取对应的传感器数据
    if (!g_control_context.sensor_data_valid) {
        return 0.0f;
    }

    return g_control_context.sensor_data.sensors[sensor_type].calibrated_value;
}

/**
 * @brief 检查传感器数据有效性
 * @param loop_id 控制回路ID
 * @return true=有效, false=无效
 */
static bool Control_IsSensorValid(control_loop_t loop_id)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return false;
    }

    if (!g_control_context.sensor_data_valid) {
        return false;
    }

    control_loop_config_t *loop = &g_control_context.loops[loop_id];
    sensor_type_t sensor_type = loop->sensor_type;

    return g_control_context.sensor_data.sensors[sensor_type].valid;
}

/**
 * @brief 设置执行器输出
 * @param loop_id 控制回路ID
 * @param output 输出值
 * @return pdTRUE=成功, pdFALSE=失败
 */
static BaseType_t Control_SetActuatorOutput(control_loop_t loop_id, float output)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return pdFALSE;
    }

    control_loop_config_t *loop = &g_control_context.loops[loop_id];
    actuator_type_t actuator_type = loop->actuator_type;

    // 调用执行器任务API设置输出
    return ActuatorTaskV3_SetOutput(actuator_type, output);
}

/**
 * @brief 计算控制回路质量
 * @param loop_id 控制回路ID
 * @return 质量分数 (0-100)
 */
static float Control_CalculateLoopQuality(control_loop_t loop_id)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return 0.0f;
    }

    control_loop_config_t *loop = &g_control_context.loops[loop_id];
    pid_state_t *state = &loop->pid_state;

    // 基于误差的质量评估
    float error_ratio = fabsf(state->error) / (loop->setpoint_max - loop->setpoint_min);
    float error_quality = (1.0f - error_ratio) * 100.0f;

    // 限制范围
    if (error_quality < 0.0f) error_quality = 0.0f;
    if (error_quality > 100.0f) error_quality = 100.0f;

    // 检查报警状态
    if (loop->alarm_status) {
        error_quality *= 0.5f;  // 报警时质量减半
    } else if (loop->warning_status) {
        error_quality *= 0.8f;  // 警告时质量减少20%
    }

    return error_quality;
}

/**
 * @brief 计算控制回路稳定性
 * @param loop_id 控制回路ID
 * @return 稳定性指标 (0.0-1.0)
 */
static float Control_CalculateLoopStability(control_loop_t loop_id)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return 0.0f;
    }

    control_loop_config_t *loop = &g_control_context.loops[loop_id];
    pid_state_t *state = &loop->pid_state;

    // 基于误差变化率的稳定性评估
    float error_change = fabsf(state->error - state->last_error);
    float stability = 1.0f - (error_change / (loop->setpoint_max - loop->setpoint_min));

    // 限制范围
    if (stability < 0.0f) stability = 0.0f;
    if (stability > 1.0f) stability = 1.0f;

    return stability;
}

/**
 * @brief 使能控制回路
 * @param loop_id 控制回路ID
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_EnableLoop(control_loop_t loop_id)
{
    control_command_t command;

    if (loop_id >= CONTROL_LOOP_COUNT) {
        return pdFALSE;
    }

    command.cmd_type = CONTROL_CMD_ENABLE_LOOP;
    command.loop_id = loop_id;
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    return ControlTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 禁用控制回路
 * @param loop_id 控制回路ID
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_DisableLoop(control_loop_t loop_id)
{
    control_command_t command;

    if (loop_id >= CONTROL_LOOP_COUNT) {
        return pdFALSE;
    }

    command.cmd_type = CONTROL_CMD_DISABLE_LOOP;
    command.loop_id = loop_id;
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    return ControlTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 设置PID参数
 * @param loop_id 控制回路ID
 * @param params PID参数结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_SetPIDParams(control_loop_t loop_id, const pid_params_t *params)
{
    control_command_t command;

    if (loop_id >= CONTROL_LOOP_COUNT || params == NULL) {
        return pdFALSE;
    }

    command.cmd_type = CONTROL_CMD_UPDATE_PARAMS;
    command.loop_id = loop_id;
    memcpy(&command.pid_params, params, sizeof(pid_params_t));
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    return ControlTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 启动PID自整定
 * @param loop_id 控制回路ID
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_StartAutoTune(control_loop_t loop_id)
{
    control_command_t command;

    if (loop_id >= CONTROL_LOOP_COUNT) {
        return pdFALSE;
    }

    command.cmd_type = CONTROL_CMD_TUNE_PID;
    command.loop_id = loop_id;
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    // TODO: 实现自整定算法
    return ControlTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 停止PID自整定
 * @param loop_id 控制回路ID
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_StopAutoTune(control_loop_t loop_id)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return pdFALSE;
    }

    // TODO: 实现停止自整定逻辑
    return pdTRUE;
}

/**
 * @brief 恢复控制系统运行
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_Resume(void)
{
    control_command_t command;

    command.cmd_type = CONTROL_CMD_RESUME;
    command.loop_id = CONTROL_LOOP_TEMP_1;  // 不关心具体回路
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    printf("[ControlV3] 系统恢复运行\r\n");

    return ControlTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 复位控制回路
 * @param loop_id 控制回路ID
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_ResetLoop(control_loop_t loop_id)
{
    control_command_t command;

    if (loop_id >= CONTROL_LOOP_COUNT) {
        return pdFALSE;
    }

    command.cmd_type = CONTROL_CMD_RESET_LOOP;
    command.loop_id = loop_id;
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    return ControlTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 获取控制回路配置
 * @param loop_id 控制回路ID
 * @param config 配置结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_GetLoopConfig(control_loop_t loop_id, control_loop_config_t *config)
{
    if (loop_id >= CONTROL_LOOP_COUNT || config == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_ControlContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(config, &g_control_context.loops[loop_id], sizeof(control_loop_config_t));
        xSemaphoreGive(xMutex_ControlContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 获取PID状态
 * @param loop_id 控制回路ID
 * @param state PID状态结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_GetPIDState(control_loop_t loop_id, pid_state_t *state)
{
    if (loop_id >= CONTROL_LOOP_COUNT || state == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_ControlContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(state, &g_control_context.loops[loop_id].pid_state, sizeof(pid_state_t));
        xSemaphoreGive(xMutex_ControlContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 发送控制消息到队列
 * @param msg 消息结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_SendMessage(const control_msg_t *msg, uint32_t timeout_ms)
{
    if (msg == NULL) {
        return pdFALSE;
    }

    return xQueueSend(xQueue_ControlMsg, msg, pdMS_TO_TICKS(timeout_ms));
}

/**
 * @brief 从队列接收控制消息
 * @param msg 消息结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ControlTaskV3_ReceiveMessage(control_msg_t *msg, uint32_t timeout_ms)
{
    if (msg == NULL) {
        return pdFALSE;
    }

    return xQueueReceive(xQueue_ControlMsg, msg, pdMS_TO_TICKS(timeout_ms));
}

/**
 * @brief 重置控制任务统计信息
 */
void ControlTaskV3_ResetStatistics(void)
{
    memset(&g_control_stats, 0, sizeof(control_task_stats_t));
    printf("[ControlV3] 统计信息已重置\r\n");
}

/**
 * @brief 计算控制质量分数
 * @param loop_id 控制回路ID
 * @return 质量分数 (0-100)
 */
uint8_t ControlTaskV3_CalculateQuality(control_loop_t loop_id)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return 0;
    }

    return (uint8_t)g_control_context.loops[loop_id].control_quality;
}

/**
 * @brief 检查系统稳定性
 * @return 稳定性指标 (0.0-1.0)
 */
float ControlTaskV3_CheckStability(void)
{
    return g_control_context.system_stability;
}

/**
 * @brief 获取控制回路过程值
 * @param loop_id 控制回路ID
 * @return 过程值
 */
float ControlTaskV3_GetProcessValue(control_loop_t loop_id)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return 0.0f;
    }

    return g_control_context.loops[loop_id].process_value;
}

/**
 * @brief 获取控制回路输出值
 * @param loop_id 控制回路ID
 * @return 输出值
 */
float ControlTaskV3_GetOutputValue(control_loop_t loop_id)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return 0.0f;
    }

    return g_control_context.loops[loop_id].output_value;
}

/**
 * @brief 检查控制回路是否在自动模式
 * @param loop_id 控制回路ID
 * @return true=自动模式, false=手动模式
 */
bool ControlTaskV3_IsAutoMode(control_loop_t loop_id)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return false;
    }

    return g_control_context.loops[loop_id].auto_mode;
}

/**
 * @brief 检查控制回路是否有报警
 * @param loop_id 控制回路ID
 * @return true=有报警, false=无报警
 */
bool ControlTaskV3_HasAlarm(control_loop_t loop_id)
{
    if (loop_id >= CONTROL_LOOP_COUNT) {
        return false;
    }

    return g_control_context.loops[loop_id].alarm_status;
}

/**
 * @brief 检查系统是否处于紧急停止状态
 * @return true=紧急停止, false=正常
 */
bool ControlTaskV3_IsEmergencyStopped(void)
{
    return g_control_context.emergency_stop;
}

/**
 * @brief 检查系统是否处于安全模式
 * @return true=安全模式, false=正常模式
 */
bool ControlTaskV3_IsInSafetyMode(void)
{
    return g_control_context.safety_mode;
}

/************************ (C) COPYRIGHT Ink Supply Control System *****END OF FILE****/