/**
 ******************************************************************************
 * @file    actuator_task_v3.c
 * @brief   执行器任务实现 - 基于V3设计文档
 * @author  Ink Supply Control System Development Team
 * @version V3.0.0
 * @date    2025-01-23
 ******************************************************************************
 * @attention
 *
 * 本文件基于墨路控制系统详细设计文档V3实现，采用四层架构设计
 *
 * 主要功能:
 * 1. 控制电磁阀 (24V x2) - 数字输出
 * 2. 控制加热器 (继电器 x3) - 数字输出
 * 3. 控制调速泵 (PWM x2) - PWM输出
 * 4. 控制直流泵 (IO x2) - 数字输出
 * 5. 执行安全输出保护和故障检测
 * 6. 提供执行器状态反馈和统计信息
 * 7. 支持紧急停止和安全模式
 ******************************************************************************
 */

#include "actuator_task_v3.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ========================================================================== */
/* 私有宏定义 */
/* ========================================================================== */

#define RAMP_UPDATE_INTERVAL_MS     10      // 爬坡更新间隔
#define SAFETY_CHECK_INTERVAL_MS    100     // 安全检查间隔
#define OUTPUT_SETTLE_TIME_MS       5       // 输出稳定时间
#define FAULT_DEBOUNCE_COUNT        3       // 故障防抖次数

/* ========================================================================== */
/* 全局变量定义 */
/* ========================================================================== */

// 任务句柄
TaskHandle_t xTaskHandle_ActuatorV3 = NULL;

// 队列句柄
QueueHandle_t xQueue_ActuatorCmd = NULL;
QueueHandle_t xQueue_ActuatorMsg = NULL;

// 互斥体
SemaphoreHandle_t xMutex_ActuatorContext = NULL;

// 事件组
EventGroupHandle_t xEventGroup_Actuator = NULL;

/* ========================================================================== */
/* 私有变量 */
/* ========================================================================== */

// 执行器上下文
static actuator_context_t g_actuator_context = {0};

// 执行器配置
static actuator_config_t g_actuator_configs[ACTUATOR_COUNT] = {0};

// 统计信息
static actuator_task_stats_t g_actuator_stats = {0};

// 故障防抖计数器
static uint8_t g_fault_debounce[ACTUATOR_COUNT] = {0};

// 安全检查计数器
static uint32_t g_safety_check_counter = 0;

/* ========================================================================== */
/* 私有函数声明 */
/* ========================================================================== */

static void Actuator_InitializeConfigs(void);
static void Actuator_ProcessCommands(void);
static void Actuator_UpdateOutputs(void);
static void Actuator_UpdateValves(void);
static void Actuator_UpdateHeaters(void);
static void Actuator_UpdatePumps(void);
static void Actuator_ApplyRamping(actuator_type_t actuator_type);
static void Actuator_CheckSafety(void);
static void Actuator_CheckFaults(void);
static void Actuator_UpdateStatistics(void);
static void Actuator_SendStatus(void);
static BaseType_t Actuator_SetDigitalOutput(uint8_t channel, bool state);
static BaseType_t Actuator_SetPWMOutput(uint8_t channel, float duty_cycle);
static bool Actuator_ReadFaultStatus(actuator_type_t actuator_type);

/* ========================================================================== */
/* 公共函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化执行器任务系统
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t ActuatorTaskV3_Init(void)
{
    // 创建互斥体
    xMutex_ActuatorContext = xSemaphoreCreateMutex();
    if (xMutex_ActuatorContext == NULL) {
        printf("[ActuatorV3] ERROR: 创建互斥体失败\r\n");
        return pdFAIL;
    }

    // 创建命令队列
    xQueue_ActuatorCmd = xQueueCreate(ACTUATOR_CMD_QUEUE_SIZE, sizeof(actuator_command_t));
    if (xQueue_ActuatorCmd == NULL) {
        printf("[ActuatorV3] ERROR: 创建命令队列失败\r\n");
        return pdFAIL;
    }

    // 创建消息队列
    xQueue_ActuatorMsg = xQueueCreate(ACTUATOR_MSG_QUEUE_SIZE, sizeof(actuator_msg_t));
    if (xQueue_ActuatorMsg == NULL) {
        printf("[ActuatorV3] ERROR: 创建消息队列失败\r\n");
        return pdFAIL;
    }

    // 创建事件组
    xEventGroup_Actuator = xEventGroupCreate();
    if (xEventGroup_Actuator == NULL) {
        printf("[ActuatorV3] ERROR: 创建事件组失败\r\n");
        return pdFAIL;
    }

    // 初始化执行器配置
    Actuator_InitializeConfigs();

    // 初始化上下文
    memset(&g_actuator_context, 0, sizeof(actuator_context_t));
    g_actuator_context.system_ready = false;
    g_actuator_context.safety_mode = false;
    g_actuator_context.emergency_stop = false;

    // 初始化统计信息
    memset(&g_actuator_stats, 0, sizeof(actuator_task_stats_t));

    printf("[ActuatorV3] 执行器任务系统初始化成功\r\n");
    return pdPASS;
}

/**
 * @brief 创建执行器任务
 * @return pdPASS=成功, pdFAIL=失败
 */
BaseType_t ActuatorTaskV3_Create(void)
{
    BaseType_t result;

    result = xTaskCreate(
        Task_ActuatorV3,
        "ActuatorV3",
        ACTUATOR_TASK_STACK_SIZE,
        NULL,
        ACTUATOR_TASK_PRIORITY,
        &xTaskHandle_ActuatorV3
    );

    if (result != pdPASS) {
        printf("[ActuatorV3] ERROR: 创建任务失败\r\n");
        return pdFAIL;
    }

    printf("[ActuatorV3] 执行器任务创建成功\r\n");
    return pdPASS;
}

/**
 * @brief 执行器任务主函数
 * @param pvParameters 任务参数
 */
void Task_ActuatorV3(void *pvParameters)
{
    TickType_t xLastWakeTime;
    uint32_t cycle_start_time, cycle_end_time;

    // 初始化延时基准时间
    xLastWakeTime = xTaskGetTickCount();

    printf("[ActuatorV3] 执行器任务启动 - 周期: %d ms, 优先级: %d\r\n",
           ACTUATOR_TASK_PERIOD_MS, ACTUATOR_TASK_PRIORITY);

    // 等待系统初始化完成
    vTaskDelay(pdMS_TO_TICKS(50));

    // 设置系统就绪标志
    g_actuator_context.system_ready = true;

    for (;;)
    {
        // 记录周期开始时间
        cycle_start_time = HAL_GetTick();

        // 1. 处理命令队列中的命令
        Actuator_ProcessCommands();

        // 2. 检查安全状态和故障
        Actuator_CheckSafety();
        Actuator_CheckFaults();

        // 3. 更新所有执行器输出
        if (!g_actuator_context.emergency_stop) {
            Actuator_UpdateOutputs();
        }

        // 4. 更新统计信息
        Actuator_UpdateStatistics();

        // 5. 发送状态消息
        if ((g_actuator_context.cycle_count % 10) == 0) {  // 每100ms发送一次状态
            Actuator_SendStatus();
        }

        // 6. 记录周期结束时间和更新统计
        cycle_end_time = HAL_GetTick();
        uint32_t cycle_time_us = (cycle_end_time - cycle_start_time) * 1000;

        g_actuator_stats.total_cycles++;
        if (cycle_time_us > g_actuator_stats.max_cycle_time_us) {
            g_actuator_stats.max_cycle_time_us = (uint16_t)cycle_time_us;
        }

        // 计算平均循环时间
        g_actuator_stats.avg_cycle_time_us = (uint16_t)(
            (g_actuator_stats.avg_cycle_time_us * (g_actuator_stats.total_cycles - 1) + cycle_time_us) /
            g_actuator_stats.total_cycles
        );

        // 7. 定期打印调试信息 (每1000个周期 = 10秒)
        if ((g_actuator_context.cycle_count % 1000) == 0) {
            // printf("[ActuatorV3] 周期=%lu, 安全模式=%s, 紧急停止=%s, 执行时间=%luμs\r\n",
            //        g_actuator_context.cycle_count,
            //        g_actuator_context.safety_mode ? "是" : "否",
            //        g_actuator_context.emergency_stop ? "是" : "否",
            //        cycle_time_us);
        }

        // 8. 更新周期计数
        g_actuator_context.cycle_count++;
        g_actuator_context.last_update_time = HAL_GetTick();

        // 9. 按照固定周期执行
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(ACTUATOR_TASK_PERIOD_MS));
    }
}

/**
 * @brief 设置执行器输出值
 * @param actuator_type 执行器类型
 * @param value 输出值 (0-100%)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SetOutput(actuator_type_t actuator_type, float value)
{
    actuator_command_t command;

    if (actuator_type >= ACTUATOR_COUNT) {
        return pdFALSE;
    }

    // 构造命令
    command.cmd_type = ACTUATOR_CMD_SET_OUTPUT;
    command.actuator_type = actuator_type;
    command.value = value;
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    return ActuatorTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 使能执行器
 * @param actuator_type 执行器类型
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_Enable(actuator_type_t actuator_type)
{
    actuator_command_t command;

    if (actuator_type >= ACTUATOR_COUNT) {
        return pdFALSE;
    }

    command.cmd_type = ACTUATOR_CMD_ENABLE;
    command.actuator_type = actuator_type;
    command.value = 0.0f;
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    return ActuatorTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 禁用执行器
 * @param actuator_type 执行器类型
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_Disable(actuator_type_t actuator_type)
{
    actuator_command_t command;

    if (actuator_type >= ACTUATOR_COUNT) {
        return pdFALSE;
    }

    command.cmd_type = ACTUATOR_CMD_DISABLE;
    command.actuator_type = actuator_type;
    command.value = 0.0f;
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    return ActuatorTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 紧急停止所有执行器
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_EmergencyStop(void)
{
    actuator_command_t command;

    command.cmd_type = ACTUATOR_CMD_EMERGENCY_STOP;
    command.actuator_type = ACTUATOR_VALVE_1;  // 不用关心具体类型
    command.value = 0.0f;
    command.timestamp = HAL_GetTick();
    command.urgent = true;

    printf("[ActuatorV3] 紧急停止触发!\r\n");

    return ActuatorTaskV3_SendCommand(&command, 0);  // 立即发送，不等待
}

/**
 * @brief 恢复执行器运行
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_Resume(void)
{
    actuator_command_t command;

    command.cmd_type = ACTUATOR_CMD_RESUME;
    command.actuator_type = ACTUATOR_VALVE_1;  // 不用关心具体类型
    command.value = 0.0f;
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    printf("[ActuatorV3] 系统恢复运行\r\n");

    return ActuatorTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 发送执行器命令
 * @param command 命令结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SendCommand(const actuator_command_t *command, uint32_t timeout_ms)
{
    if (command == NULL) {
        return pdFALSE;
    }

    BaseType_t result = xQueueSend(xQueue_ActuatorCmd, command, pdMS_TO_TICKS(timeout_ms));

    if (result == pdPASS) {
        g_actuator_stats.command_count++;

        // 设置事件标志
        xEventGroupSetBits(xEventGroup_Actuator, EVENT_ACTUATOR_UPDATE);
    } else {
        g_actuator_stats.command_errors++;
    }

    return result;
}

/**
 * @brief 获取执行器上下文
 * @param context 上下文结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_GetContext(actuator_context_t *context)
{
    if (context == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_ActuatorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(context, &g_actuator_context, sizeof(actuator_context_t));
        xSemaphoreGive(xMutex_ActuatorContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 设置电磁阀状态
 * @param valve_id 阀门ID (0 or 1)
 * @param state 状态 (true=开, false=关)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SetValve(uint8_t valve_id, bool state)
{
    if (valve_id > 1) {
        return pdFALSE;
    }

    actuator_type_t actuator_type = (valve_id == 0) ? ACTUATOR_VALVE_1 : ACTUATOR_VALVE_2;
    float value = state ? 100.0f : 0.0f;

    return ActuatorTaskV3_SetOutput(actuator_type, value);
}

/**
 * @brief 设置加热器状态
 * @param heater_id 加热器ID (0, 1, or 2)
 * @param state 状态 (true=开, false=关)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SetHeater(uint8_t heater_id, bool state)
{
    if (heater_id > 2) {
        return pdFALSE;
    }

    actuator_type_t actuator_type = ACTUATOR_HEATER_1 + heater_id;
    float value = state ? 100.0f : 0.0f;

    return ActuatorTaskV3_SetOutput(actuator_type, value);
}

/**
 * @brief 设置调速泵速度
 * @param pump_id 泵ID (0 or 1)
 * @param speed 速度 (0-100%)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SetPumpSpeed(uint8_t pump_id, float speed)
{
    if (pump_id > 1) {
        return pdFALSE;
    }

    // 限制速度范围
    if (speed < 0.0f) speed = 0.0f;
    if (speed > 100.0f) speed = 100.0f;

    actuator_type_t actuator_type = (pump_id == 0) ? ACTUATOR_PUMP_SPEED_1 : ACTUATOR_PUMP_SPEED_2;

    return ActuatorTaskV3_SetOutput(actuator_type, speed);
}

/**
 * @brief 设置直流泵状态
 * @param pump_id 泵ID (0 or 1)
 * @param state 状态 (true=开, false=关)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SetDCPump(uint8_t pump_id, bool state)
{
    if (pump_id > 1) {
        return pdFALSE;
    }

    actuator_type_t actuator_type = (pump_id == 0) ? ACTUATOR_PUMP_DC_1 : ACTUATOR_PUMP_DC_2;
    float value = state ? 100.0f : 0.0f;

    return ActuatorTaskV3_SetOutput(actuator_type, value);
}

/**
 * @brief 检查是否处于安全模式
 * @return true=安全模式, false=正常模式
 */
bool ActuatorTaskV3_IsInSafetyMode(void)
{
    return g_actuator_context.safety_mode;
}

/**
 * @brief 检查是否紧急停止
 * @return true=紧急停止, false=正常
 */
bool ActuatorTaskV3_IsEmergencyStopped(void)
{
    return g_actuator_context.emergency_stop;
}

/**
 * @brief 检查执行器系统健康状态
 * @return 健康分数 (0-100)
 */
uint8_t ActuatorTaskV3_CheckHealth(void)
{
    uint8_t health_score = 100;
    uint8_t fault_count = 0;

    // 检查各执行器故障状态
    for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
        if (g_actuator_context.status[i].fault) {
            fault_count++;
        }
    }

    // 根据故障数量计算健康分数
    if (fault_count > 0) {
        health_score = (uint8_t)(100 - (fault_count * 100) / ACTUATOR_COUNT);
    }

    // 紧急停止状态影响健康分数
    if (g_actuator_context.emergency_stop) {
        health_score = (uint8_t)(health_score * 0.5f);  // 减半
    }

    // 安全模式影响健康分数
    if (g_actuator_context.safety_mode) {
        health_score = (uint8_t)(health_score * 0.8f);  // 减少20%
    }

    return health_score;
}

/**
 * @brief 获取执行器任务统计信息
 * @param stats 统计信息结构指针
 */
void ActuatorTaskV3_GetStatistics(actuator_task_stats_t *stats)
{
    if (stats != NULL) {
        memcpy(stats, &g_actuator_stats, sizeof(actuator_task_stats_t));
    }
}

/**
 * @brief 重置执行器任务统计信息
 */
void ActuatorTaskV3_ResetStatistics(void)
{
    memset(&g_actuator_stats, 0, sizeof(actuator_task_stats_t));
    printf("[ActuatorV3] 统计信息已重置\r\n");
}

/* ========================================================================== */
/* 私有函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化执行器配置
 */
static void Actuator_InitializeConfigs(void)
{
    // 初始化电磁阀配置 (24V数字输出)
    for (uint8_t i = ACTUATOR_VALVE_1; i <= ACTUATOR_VALVE_2; i++) {
        g_actuator_configs[i].channel = i;
        g_actuator_configs[i].output_type = OUTPUT_TYPE_DIGITAL;
        g_actuator_configs[i].current_output = 0.0f;
        g_actuator_configs[i].target_output = 0.0f;
        g_actuator_configs[i].min_output = 0.0f;
        g_actuator_configs[i].max_output = 100.0f;
        g_actuator_configs[i].enabled = true;
        g_actuator_configs[i].safety_override = false;
        g_actuator_configs[i].ramp_rate = 0.0f;  // 数字输出无需爬坡
    }

    // 初始化加热器配置 (继电器数字输出)
    for (uint8_t i = ACTUATOR_HEATER_1; i <= ACTUATOR_HEATER_3; i++) {
        g_actuator_configs[i].channel = i;
        g_actuator_configs[i].output_type = OUTPUT_TYPE_DIGITAL;
        g_actuator_configs[i].current_output = 0.0f;
        g_actuator_configs[i].target_output = 0.0f;
        g_actuator_configs[i].min_output = 0.0f;
        g_actuator_configs[i].max_output = 100.0f;
        g_actuator_configs[i].enabled = true;
        g_actuator_configs[i].safety_override = false;
        g_actuator_configs[i].ramp_rate = 0.0f;  // 数字输出无需爬坡
    }

    // 初始化调速泵配置 (PWM输出)
    for (uint8_t i = ACTUATOR_PUMP_SPEED_1; i <= ACTUATOR_PUMP_SPEED_2; i++) {
        g_actuator_configs[i].channel = i;
        g_actuator_configs[i].output_type = OUTPUT_TYPE_PWM;
        g_actuator_configs[i].current_output = 0.0f;
        g_actuator_configs[i].target_output = 0.0f;
        g_actuator_configs[i].min_output = 0.0f;
        g_actuator_configs[i].max_output = 100.0f;
        g_actuator_configs[i].enabled = true;
        g_actuator_configs[i].safety_override = false;
        g_actuator_configs[i].ramp_rate = ACTUATOR_RAMP_DEFAULT_RATE;  // 10%/s默认爬坡速率
    }

    // 初始化直流泵配置 (数字输出)
    for (uint8_t i = ACTUATOR_PUMP_DC_1; i <= ACTUATOR_PUMP_DC_2; i++) {
        g_actuator_configs[i].channel = i;
        g_actuator_configs[i].output_type = OUTPUT_TYPE_DIGITAL;
        g_actuator_configs[i].current_output = 0.0f;
        g_actuator_configs[i].target_output = 0.0f;
        g_actuator_configs[i].min_output = 0.0f;
        g_actuator_configs[i].max_output = 100.0f;
        g_actuator_configs[i].enabled = true;
        g_actuator_configs[i].safety_override = false;
        g_actuator_configs[i].ramp_rate = 0.0f;  // 数字输出无需爬坡
    }

    // 初始化执行器状态
    for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
        g_actuator_context.status[i].type = (actuator_type_t)i;
        g_actuator_context.status[i].state = ACTUATOR_STATE_IDLE;
        g_actuator_context.status[i].output_value = 0.0f;
        g_actuator_context.status[i].feedback_value = 0.0f;
        g_actuator_context.status[i].fault = false;
        g_actuator_context.status[i].fault_code = 0;
        g_actuator_context.status[i].run_time = 0;
        g_actuator_context.status[i].switch_count = 0;
        g_actuator_context.status[i].timestamp = HAL_GetTick();
    }

    printf("[ActuatorV3] 执行器配置初始化完成\r\n");
}

/**
 * @brief 处理命令队列中的命令
 */
static void Actuator_ProcessCommands(void)
{
    actuator_command_t command;
    BaseType_t result;

    // 处理所有待处理的命令
    while ((result = xQueueReceive(xQueue_ActuatorCmd, &command, 0)) == pdPASS) {
        // 验证命令参数
        if (command.actuator_type >= ACTUATOR_COUNT) {
            g_actuator_stats.command_errors++;
            continue;
        }

        // 处理不同类型的命令
        switch (command.cmd_type) {
            case ACTUATOR_CMD_SET_OUTPUT:
                // 限制输出值范围
                if (command.value < g_actuator_configs[command.actuator_type].min_output) {
                    command.value = g_actuator_configs[command.actuator_type].min_output;
                }
                if (command.value > g_actuator_configs[command.actuator_type].max_output) {
                    command.value = g_actuator_configs[command.actuator_type].max_output;
                }

                // 设置目标输出值
                g_actuator_configs[command.actuator_type].target_output = command.value;
                g_actuator_configs[command.actuator_type].last_update = HAL_GetTick();

                // 更新执行器状态
                if (g_actuator_context.status[command.actuator_type].state == ACTUATOR_STATE_IDLE) {
                    g_actuator_context.status[command.actuator_type].state = ACTUATOR_STATE_RUNNING;
                }
                break;

            case ACTUATOR_CMD_ENABLE:
                g_actuator_configs[command.actuator_type].enabled = true;
                g_actuator_context.status[command.actuator_type].state = ACTUATOR_STATE_IDLE;
                printf("[ActuatorV3] 执行器 %d 已使能\r\n", command.actuator_type);
                break;

            case ACTUATOR_CMD_DISABLE:
                g_actuator_configs[command.actuator_type].enabled = false;
                g_actuator_configs[command.actuator_type].target_output = 0.0f;
                g_actuator_configs[command.actuator_type].current_output = 0.0f;
                g_actuator_context.status[command.actuator_type].state = ACTUATOR_STATE_DISABLED;
                printf("[ActuatorV3] 执行器 %d 已禁用\r\n", command.actuator_type);
                break;

            case ACTUATOR_CMD_RESET_FAULT:
                g_actuator_context.status[command.actuator_type].fault = false;
                g_actuator_context.status[command.actuator_type].fault_code = 0;
                g_fault_debounce[command.actuator_type] = 0;
                printf("[ActuatorV3] 执行器 %d 故障已复位\r\n", command.actuator_type);
                break;

            case ACTUATOR_CMD_EMERGENCY_STOP:
                // 紧急停止所有执行器
                g_actuator_context.emergency_stop = true;
                g_actuator_context.safety_mode = true;

                // 立即停止所有输出
                for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
                    g_actuator_configs[i].target_output = 0.0f;
                    g_actuator_configs[i].current_output = 0.0f;
                    g_actuator_context.status[i].state = ACTUATOR_STATE_DISABLED;
                }

                // 设置事件标志
                xEventGroupSetBits(xEventGroup_Actuator, EVENT_ACTUATOR_EMERGENCY);
                g_actuator_stats.emergency_stops++;
                printf("[ActuatorV3] 紧急停止执行完成\r\n");
                break;

            case ACTUATOR_CMD_RESUME:
                // 恢复系统运行
                g_actuator_context.emergency_stop = false;
                g_actuator_context.safety_mode = false;

                // 恢复所有执行器为空闲状态
                for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
                    if (g_actuator_configs[i].enabled) {
                        g_actuator_context.status[i].state = ACTUATOR_STATE_IDLE;
                    }
                }
                printf("[ActuatorV3] 系统恢复运行完成\r\n");
                break;

            default:
                g_actuator_stats.command_errors++;
                break;
        }
    }
}

/**
 * @brief 更新所有执行器输出
 */
static void Actuator_UpdateOutputs(void)
{
    // 应用爬坡控制
    for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
        if (g_actuator_configs[i].enabled && !g_actuator_context.status[i].fault) {
            Actuator_ApplyRamping((actuator_type_t)i);
        }
    }

    // 更新各类执行器
    Actuator_UpdateValves();
    Actuator_UpdateHeaters();
    Actuator_UpdatePumps();
}

/**
 * @brief 更新电磁阀输出
 */
static void Actuator_UpdateValves(void)
{
    for (uint8_t i = ACTUATOR_VALVE_1; i <= ACTUATOR_VALVE_2; i++) {
        if (!g_actuator_configs[i].enabled || g_actuator_context.status[i].fault) {
            continue;
        }

        // 数字输出: >50% 为开，<=50% 为关
        bool valve_state = (g_actuator_configs[i].current_output > 50.0f);

        // 调用HAL层接口输出
        if (Actuator_SetDigitalOutput(g_actuator_configs[i].channel, valve_state) == pdTRUE) {
            // 更新状态
            g_actuator_context.valve_states[i - ACTUATOR_VALVE_1] = valve_state;
            g_actuator_context.status[i].output_value = valve_state ? 100.0f : 0.0f;
            g_actuator_context.status[i].timestamp = HAL_GetTick();

            // 统计开关次数
            static bool prev_valve_states[2] = {false, false};
            if (valve_state != prev_valve_states[i - ACTUATOR_VALVE_1]) {
                g_actuator_context.status[i].switch_count++;
                prev_valve_states[i - ACTUATOR_VALVE_1] = valve_state;
            }
        }
    }
}

/**
 * @brief 更新加热器输出
 */
static void Actuator_UpdateHeaters(void)
{
    for (uint8_t i = ACTUATOR_HEATER_1; i <= ACTUATOR_HEATER_3; i++) {
        if (!g_actuator_configs[i].enabled || g_actuator_context.status[i].fault) {
            continue;
        }

        // 数字输出: >50% 为开，<=50% 为关
        bool heater_state = (g_actuator_configs[i].current_output > 50.0f);

        // 调用HAL层接口输出
        if (Actuator_SetDigitalOutput(g_actuator_configs[i].channel, heater_state) == pdTRUE) {
            // 更新状态
            g_actuator_context.heater_states[i - ACTUATOR_HEATER_1] = heater_state;
            g_actuator_context.status[i].output_value = heater_state ? 100.0f : 0.0f;
            g_actuator_context.status[i].timestamp = HAL_GetTick();

            // 统计开关次数和运行时间
            static bool prev_heater_states[3] = {false, false, false};
            if (heater_state != prev_heater_states[i - ACTUATOR_HEATER_1]) {
                g_actuator_context.status[i].switch_count++;
                prev_heater_states[i - ACTUATOR_HEATER_1] = heater_state;
            }

            // 更新运行时间 (如果加热器开启)
            if (heater_state) {
                g_actuator_context.status[i].run_time += ACTUATOR_TASK_PERIOD_MS;
            }
        }
    }
}

/**
 * @brief 更新泵输出
 */
static void Actuator_UpdatePumps(void)
{
    // 更新调速泵 (PWM输出)
    for (uint8_t i = ACTUATOR_PUMP_SPEED_1; i <= ACTUATOR_PUMP_SPEED_2; i++) {
        if (!g_actuator_configs[i].enabled || g_actuator_context.status[i].fault) {
            continue;
        }

        // PWM输出
        float duty_cycle = g_actuator_configs[i].current_output;

        // 调用HAL层接口输出PWM
        if (Actuator_SetPWMOutput(g_actuator_configs[i].channel, duty_cycle) == pdTRUE) {
            // 更新状态
            g_actuator_context.pump_speed[i - ACTUATOR_PUMP_SPEED_1] = duty_cycle;
            g_actuator_context.status[i].output_value = duty_cycle;
            g_actuator_context.status[i].timestamp = HAL_GetTick();

            // 更新运行时间 (如果泵运行)
            if (duty_cycle > 5.0f) {  // 5%以上认为在运行
                g_actuator_context.status[i].run_time += ACTUATOR_TASK_PERIOD_MS;
            }
        }
    }

    // 更新直流泵 (数字输出)
    for (uint8_t i = ACTUATOR_PUMP_DC_1; i <= ACTUATOR_PUMP_DC_2; i++) {
        if (!g_actuator_configs[i].enabled || g_actuator_context.status[i].fault) {
            continue;
        }

        // 数字输出: >50% 为开，<=50% 为关
        bool pump_state = (g_actuator_configs[i].current_output > 50.0f);

        // 调用HAL层接口输出
        if (Actuator_SetDigitalOutput(g_actuator_configs[i].channel, pump_state) == pdTRUE) {
            // 更新状态
            g_actuator_context.pump_dc_states[i - ACTUATOR_PUMP_DC_1] = pump_state;
            g_actuator_context.status[i].output_value = pump_state ? 100.0f : 0.0f;
            g_actuator_context.status[i].timestamp = HAL_GetTick();

            // 统计开关次数和运行时间
            static bool prev_dc_pump_states[2] = {false, false};
            if (pump_state != prev_dc_pump_states[i - ACTUATOR_PUMP_DC_1]) {
                g_actuator_context.status[i].switch_count++;
                prev_dc_pump_states[i - ACTUATOR_PUMP_DC_1] = pump_state;
            }

            // 更新运行时间 (如果泵运行)
            if (pump_state) {
                g_actuator_context.status[i].run_time += ACTUATOR_TASK_PERIOD_MS;
            }
        }
    }
}

/**
 * @brief 应用爬坡控制
 * @param actuator_type 执行器类型
 */
static void Actuator_ApplyRamping(actuator_type_t actuator_type)
{
    if (actuator_type >= ACTUATOR_COUNT) {
        return;
    }

    actuator_config_t *config = &g_actuator_configs[actuator_type];

    // 数字输出类型无需爬坡
    if (config->output_type == OUTPUT_TYPE_DIGITAL) {
        config->current_output = config->target_output;
        return;
    }

    // PWM输出需要爬坡控制
    if (config->ramp_rate <= 0.0f) {
        // 无爬坡限制，直接设置
        config->current_output = config->target_output;
        return;
    }

    // 计算爬坡增量 (爬坡速率 * 周期时间)
    float ramp_delta = config->ramp_rate * (ACTUATOR_TASK_PERIOD_MS / 1000.0f);
    float output_diff = config->target_output - config->current_output;

    if (fabsf(output_diff) <= ramp_delta) {
        // 差值小于爬坡增量，直接到达目标
        config->current_output = config->target_output;
    } else {
        // 按爬坡速率调整
        if (output_diff > 0) {
            config->current_output += ramp_delta;
        } else {
            config->current_output -= ramp_delta;
        }
    }

    // 限制输出范围
    if (config->current_output < config->min_output) {
        config->current_output = config->min_output;
    }
    if (config->current_output > config->max_output) {
        config->current_output = config->max_output;
    }
}

/**
 * @brief 检查安全状态
 */
static void Actuator_CheckSafety(void)
{
    // 每100ms进行一次安全检查
    g_safety_check_counter++;
    if ((g_safety_check_counter % (SAFETY_CHECK_INTERVAL_MS / ACTUATOR_TASK_PERIOD_MS)) != 0) {
        return;
    }

    bool safety_trigger = false;

    // 检查各执行器是否有安全威胁
    for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
        actuator_status_t *status = &g_actuator_context.status[i];

        // 检查执行器是否长时间运行
        uint32_t current_time = HAL_GetTick();
        uint32_t max_run_time = 0;

        // 根据执行器类型设置最大运行时间限制
        switch (status->type) {
            case ACTUATOR_VALVE_1:
            case ACTUATOR_VALVE_2:
                max_run_time = 60000; // 电磁阀连续运行1分钟检查
                break;
            case ACTUATOR_HEATER_1:
            case ACTUATOR_HEATER_2:
            case ACTUATOR_HEATER_3:
                max_run_time = 300000; // 加热器连续运行5分钟检查
                break;
            case ACTUATOR_PUMP_SPEED_1:
            case ACTUATOR_PUMP_SPEED_2:
            case ACTUATOR_PUMP_DC_1:
            case ACTUATOR_PUMP_DC_2:
                max_run_time = 120000; // 泵连续运行2分钟检查
                break;
            default:
                max_run_time = 60000;
                break;
        }

        // 检查运行时间
        if (status->run_time > max_run_time && status->output_value > 0) {
            printf("[ActuatorV3] 警告: 执行器 %d 长时间运行 (%lu ms)\r\n", i, status->run_time);
            // 可以选择进入安全模式或发出警告
        }

        // 检查开关次数是否过于频繁
        if (status->switch_count > 1000) {  // 开关次数超过1000次
            printf("[ActuatorV3] 警告: 执行器 %d 开关次数过多 (%lu 次)\r\n", i, status->switch_count);
        }

        // 检查故障状态
        if (status->fault) {
            safety_trigger = true;
            printf("[ActuatorV3] 安全检查: 执行器 %d 存在故障 (代码: 0x%lx)\r\n", i, status->fault_code);
        }
    }

    // 更新安全模式状态
    if (safety_trigger && !g_actuator_context.safety_mode) {
        g_actuator_context.safety_mode = true;
        g_actuator_stats.safety_triggers++;
        xEventGroupSetBits(xEventGroup_Actuator, EVENT_ACTUATOR_SAFETY);
        printf("[ActuatorV3] 进入安全模式\r\n");
    }
}

/**
 * @brief 检查执行器故障
 */
static void Actuator_CheckFaults(void)
{
    for (uint8_t i = 0; i < ACTUATOR_COUNT; i++) {
        // 调用HAL层接口读取故障状态
        bool fault_detected = Actuator_ReadFaultStatus((actuator_type_t)i);

        if (fault_detected) {
            // 故障防抖处理
            g_fault_debounce[i]++;
            if (g_fault_debounce[i] >= FAULT_DEBOUNCE_COUNT) {
                // 确认故障
                if (!g_actuator_context.status[i].fault) {
                    g_actuator_context.status[i].fault = true;
                    g_actuator_context.status[i].fault_code = 0x1000 + i; // 简单的故障代码
                    g_actuator_context.status[i].state = ACTUATOR_STATE_ERROR;

                    // 立即停止故障执行器
                    g_actuator_configs[i].target_output = 0.0f;
                    g_actuator_configs[i].current_output = 0.0f;

                    // 设置事件标志
                    xEventGroupSetBits(xEventGroup_Actuator, EVENT_ACTUATOR_FAULT);

                    printf("[ActuatorV3] 执行器 %d 故障检测: 代码 0x%lx\r\n",
                           i, g_actuator_context.status[i].fault_code);
                }
            }
        } else {
            // 无故障，重置防抖计数器
            g_fault_debounce[i] = 0;
        }
    }
}

/**
 * @brief 更新统计信息
 */
static void Actuator_UpdateStatistics(void)
{
    // 统计信息在任务主循环中已经更新
    // 这里可以添加其他统计逻辑
}

/**
 * @brief 发送状态消息
 */
static void Actuator_SendStatus(void)
{
    actuator_msg_t status_msg;

    status_msg.type = MSG_ACTUATOR_STATUS;
    status_msg.timestamp = HAL_GetTick();
    status_msg.data_len = sizeof(actuator_context_t);

    // 获取互斥体并复制上下文
    if (xSemaphoreTake(xMutex_ActuatorContext, pdMS_TO_TICKS(5)) == pdTRUE) {
        memcpy(&status_msg.data.context, &g_actuator_context, sizeof(actuator_context_t));
        xSemaphoreGive(xMutex_ActuatorContext);

        // 发送消息到队列
        if (xQueueSend(xQueue_ActuatorMsg, &status_msg, 0) != pdPASS) {
            // 队列满，可能需要增加队列大小或增加消费者
        }
    }
}

/**
 * @brief 设置数字输出
 * @param channel 输出通道
 * @param state 输出状态
 * @return pdTRUE=成功, pdFALSE=失败
 */
static BaseType_t Actuator_SetDigitalOutput(uint8_t channel, bool state)
{
    // TODO: 实现实际的硬件输出接口
    // 这里应该调用HAL层的GPIO输出函数

    // 示例代码 - 需要根据实际硬件配置修改
    /*
    GPIO_TypeDef* gpio_port;
    uint16_t gpio_pin;

    // 根据通道号选择GPIO端口和引脚
    switch (channel) {
        case ACTUATOR_VALVE_1:
            gpio_port = GPIOA;
            gpio_pin = GPIO_PIN_0;
            break;
        case ACTUATOR_VALVE_2:
            gpio_port = GPIOA;
            gpio_pin = GPIO_PIN_1;
            break;
        // ... 其他通道
        default:
            return pdFALSE;
    }

    // 设置GPIO输出
    HAL_GPIO_WritePin(gpio_port, gpio_pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    */

    // 目前返回成功，实际实现时需要根据硬件接口结果返回
    return pdTRUE;
}

/**
 * @brief 设置PWM输出
 * @param channel PWM通道
 * @param duty_cycle 占空比 (0-100%)
 * @return pdTRUE=成功, pdFALSE=失败
 */
static BaseType_t Actuator_SetPWMOutput(uint8_t channel, float duty_cycle)
{
    // TODO: 实现实际的PWM输出接口
    // 这里应该调用HAL层的PWM设置函数

    // 限制占空比范围
    if (duty_cycle < 0.0f) duty_cycle = 0.0f;
    if (duty_cycle > 100.0f) duty_cycle = 100.0f;

    // 示例代码 - 需要根据实际硬件配置修改
    /*
    TIM_HandleTypeDef* htim;
    uint32_t tim_channel;

    // 根据通道号选择定时器和通道
    switch (channel) {
        case ACTUATOR_PUMP_SPEED_1:
            htim = &htim3;
            tim_channel = TIM_CHANNEL_1;
            break;
        case ACTUATOR_PUMP_SPEED_2:
            htim = &htim3;
            tim_channel = TIM_CHANNEL_2;
            break;
        default:
            return pdFALSE;
    }

    // 计算PWM比较值
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(htim);
    uint32_t pulse = (uint32_t)((duty_cycle / 100.0f) * period);

    // 设置PWM占空比
    __HAL_TIM_SET_COMPARE(htim, tim_channel, pulse);
    */

    // 目前返回成功，实际实现时需要根据硬件接口结果返回
    return pdTRUE;
}

/**
 * @brief 读取故障状态
 * @param actuator_type 执行器类型
 * @return true=有故障, false=无故障
 */
static bool Actuator_ReadFaultStatus(actuator_type_t actuator_type)
{
    // TODO: 实现实际的故障检测接口
    // 这里应该读取硬件故障信号或通过反馈检测故障

    // 示例故障检测逻辑
    switch (actuator_type) {
        case ACTUATOR_VALVE_1:
        case ACTUATOR_VALVE_2:
            // 电磁阀故障检测：检查电流反馈或位置反馈
            // 如果设置了输出但没有预期的反馈，可能是故障
            break;

        case ACTUATOR_HEATER_1:
        case ACTUATOR_HEATER_2:
        case ACTUATOR_HEATER_3:
            // 加热器故障检测：检查温度上升情况
            // 如果长时间开启但温度不上升，可能是故障
            break;

        case ACTUATOR_PUMP_SPEED_1:
        case ACTUATOR_PUMP_SPEED_2:
        case ACTUATOR_PUMP_DC_1:
        case ACTUATOR_PUMP_DC_2:
            // 泵故障检测：检查流量反馈或电流反馈
            // 如果设置了速度但没有流量输出，可能是故障
            break;

        default:
            break;
    }

    // 目前返回无故障，实际实现时需要根据具体检测结果返回
    return false;
}

/**
 * @brief 获取执行器状态
 * @param actuator_type 执行器类型
 * @param status 状态结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_GetStatus(actuator_type_t actuator_type, actuator_status_t *status)
{
    if (actuator_type >= ACTUATOR_COUNT || status == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_ActuatorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(status, &g_actuator_context.status[actuator_type], sizeof(actuator_status_t));
        xSemaphoreGive(xMutex_ActuatorContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 配置执行器
 * @param actuator_type 执行器类型
 * @param config 配置结构指针
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_Configure(actuator_type_t actuator_type, const actuator_config_t *config)
{
    if (actuator_type >= ACTUATOR_COUNT || config == NULL) {
        return pdFALSE;
    }

    // 验证配置参数
    if (config->min_output < 0.0f || config->max_output > 100.0f ||
        config->min_output > config->max_output) {
        return pdFALSE;
    }

    // 更新配置
    memcpy(&g_actuator_configs[actuator_type], config, sizeof(actuator_config_t));

    // printf("[ActuatorV3] 执行器 %d 配置已更新: 最小=%.1f%%, 最大=%.1f%%, 爬坡速率=%.1f%%/s\r\n",
    //        actuator_type, config->min_output, config->max_output, config->ramp_rate);

    return pdTRUE;
}

/**
 * @brief 复位执行器故障
 * @param actuator_type 执行器类型
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_ResetFault(actuator_type_t actuator_type)
{
    actuator_command_t command;

    if (actuator_type >= ACTUATOR_COUNT) {
        return pdFALSE;
    }

    command.cmd_type = ACTUATOR_CMD_RESET_FAULT;
    command.actuator_type = actuator_type;
    command.value = 0.0f;
    command.timestamp = HAL_GetTick();
    command.urgent = false;

    return ActuatorTaskV3_SendCommand(&command, 10);
}

/**
 * @brief 获取所有电磁阀状态
 * @param valve_states 阀门状态数组指针 (至少2个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_GetValveStates(bool *valve_states)
{
    if (valve_states == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_ActuatorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(valve_states, g_actuator_context.valve_states, sizeof(bool) * 2);
        xSemaphoreGive(xMutex_ActuatorContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 获取所有加热器状态
 * @param heater_states 加热器状态数组指针 (至少3个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_GetHeaterStates(bool *heater_states)
{
    if (heater_states == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_ActuatorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(heater_states, g_actuator_context.heater_states, sizeof(bool) * 3);
        xSemaphoreGive(xMutex_ActuatorContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 获取所有调速泵速度
 * @param pump_speeds 泵速度数组指针 (至少2个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_GetPumpSpeeds(float *pump_speeds)
{
    if (pump_speeds == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_ActuatorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(pump_speeds, g_actuator_context.pump_speed, sizeof(float) * 2);
        xSemaphoreGive(xMutex_ActuatorContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 获取所有直流泵状态
 * @param dc_pump_states 直流泵状态数组指针 (至少2个元素)
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_GetDCPumpStates(bool *dc_pump_states)
{
    if (dc_pump_states == NULL) {
        return pdFALSE;
    }

    if (xSemaphoreTake(xMutex_ActuatorContext, pdMS_TO_TICKS(10)) == pdTRUE) {
        memcpy(dc_pump_states, g_actuator_context.pump_dc_states, sizeof(bool) * 2);
        xSemaphoreGive(xMutex_ActuatorContext);
        return pdTRUE;
    }

    return pdFALSE;
}

/**
 * @brief 从队列接收执行器消息
 * @param msg 消息结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_ReceiveMessage(actuator_msg_t *msg, uint32_t timeout_ms)
{
    if (msg == NULL) {
        return pdFALSE;
    }

    return xQueueReceive(xQueue_ActuatorMsg, msg, pdMS_TO_TICKS(timeout_ms));
}

/**
 * @brief 发送执行器消息到队列
 * @param msg 消息结构指针
 * @param timeout_ms 超时时间
 * @return pdTRUE=成功, pdFALSE=失败
 */
BaseType_t ActuatorTaskV3_SendMessage(const actuator_msg_t *msg, uint32_t timeout_ms)
{
    if (msg == NULL) {
        return pdFALSE;
    }

    return xQueueSend(xQueue_ActuatorMsg, msg, pdMS_TO_TICKS(timeout_ms));
}

/************************ (C) COPYRIGHT Ink Supply Control System *****END OF FILE****/