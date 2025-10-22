/**
 * @file    freertos_ethercat_integration.h
 * @brief   FreeRTOS + EtherCAT集成头文件
 * @author  EtherCAT RTOS Integration Team
 * @version 1.0.0
 * @date    2024-01-01
 *
 * @description
 * 本文件定义了FreeRTOS与EtherCAT系统集成所需的：
 * - 任务定义和优先级
 * - 队列和信号量
 * - 同步机制
 * - 性能监控接口
 */

#ifndef __FREERTOS_ETHERCAT_INTEGRATION_H
#define __FREERTOS_ETHERCAT_INTEGRATION_H

#ifdef __cplusplus
extern "C" {
#endif

/* 包含必要的头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"
#include "stream_buffer.h"
#include "message_buffer.h"

#include "stm32f4xx_hal.h"
#include "main.h"

/* ========================================================================== */
/* 任务配置定义 */
/* ========================================================================== */

/* 任务优先级 (0 = 最低, 6 = 最高) */
#define TASK_PRIORITY_IDLE                  0   /* 空闲任务 */
#define TASK_PRIORITY_LED_BLINK             1   /* LED闪烁任务 */
#define TASK_PRIORITY_SYSTEM_MONITOR        1   /* 系统监控任务 */
#define TASK_PRIORITY_USER_INTERFACE        2   /* 用户界面任务 */
#define TASK_PRIORITY_ETHERCAT_APP          3   /* EtherCAT应用任务 */
#define TASK_PRIORITY_ETHERCAT_IO           4   /* EtherCAT IO处理任务 */
#define TASK_PRIORITY_ETHERCAT_SYNC         5   /* EtherCAT同步任务 */
#define TASK_PRIORITY_ESC_ISR               6   /* ESC中断服务任务 */

/* 任务堆栈大小 (单位: words) */
#define TASK_STACK_SIZE_LED_BLINK           128   /* LED闪烁任务 */
#define TASK_STACK_SIZE_SYSTEM_MONITOR      256   /* 系统监控任务 */
#define TASK_STACK_SIZE_USER_INTERFACE      256   /* 用户界面任务 */
#define TASK_STACK_SIZE_ETHERCAT_APP        512   /* EtherCAT应用任务 */
#define TASK_STACK_SIZE_ETHERCAT_IO         512   /* EtherCAT IO处理任务 */
#define TASK_STACK_SIZE_ETHERCAT_SYNC       512   /* EtherCAT同步任务 */

/* 任务周期定义 (单位: ms) */
#define TASK_PERIOD_LED_BLINK               500   /* LED闪烁周期 */
#define TASK_PERIOD_SYSTEM_MONITOR         1000   /* 系统监控周期 */
#define TASK_PERIOD_USER_INTERFACE          100   /* 用户界面周期 */
#define TASK_PERIOD_ETHERCAT_APP             10   /* EtherCAT应用周期 */
#define TASK_PERIOD_ETHERCAT_IO               1   /* EtherCAT IO周期 */

/* ========================================================================== */
/* 队列和信号量配置 */
/* ========================================================================== */

/* 队列大小定义 */
#define QUEUE_SIZE_ETHERCAT_EVENTS          10   /* EtherCAT事件队列 */
#define QUEUE_SIZE_SYSTEM_COMMANDS           5   /* 系统命令队列 */
#define QUEUE_SIZE_IO_DATA                   8   /* IO数据队列 */
#define QUEUE_SIZE_DEBUG_MESSAGES           20   /* 调试消息队列 */

/* 消息缓冲区大小 */
#define MESSAGE_BUFFER_SIZE_ETHERCAT        512  /* EtherCAT消息缓冲区 */
#define MESSAGE_BUFFER_SIZE_DEBUG           256  /* 调试消息缓冲区 */

/* ========================================================================== */
/* 事件标志定义 */
/* ========================================================================== */

/* 系统事件标志 */
#define EVENT_FLAG_SYSTEM_STARTUP           (1 << 0)   /* 系统启动完成 */
#define EVENT_FLAG_ETHERCAT_INIT             (1 << 1)   /* EtherCAT初始化完成 */
#define EVENT_FLAG_ETHERCAT_OP_STATE         (1 << 2)   /* EtherCAT OP状态 */
#define EVENT_FLAG_IO_READY                  (1 << 3)   /* IO就绪 */
#define EVENT_FLAG_ERROR_OCCURRED            (1 << 4)   /* 错误发生 */
#define EVENT_FLAG_SHUTDOWN_REQUEST          (1 << 5)   /* 关机请求 */

/* EtherCAT特定事件标志 */
#define EVENT_FLAG_ESC_INTERRUPT             (1 << 8)   /* ESC中断 */
#define EVENT_FLAG_SYNC0_EVENT               (1 << 9)   /* Sync0事件 */
#define EVENT_FLAG_SYNC1_EVENT               (1 << 10)  /* Sync1事件 */
#define EVENT_FLAG_MAILBOX_EVENT             (1 << 11)  /* 邮箱事件 */
#define EVENT_FLAG_STATE_CHANGE              (1 << 12)  /* 状态改变 */
#define EVENT_FLAG_WATCHDOG_EXPIRE           (1 << 13)  /* 看门狗超时 */

/* ========================================================================== */
/* 数据结构定义 */
/* ========================================================================== */

/**
 * @brief 系统统计信息结构
 */
typedef struct {
    uint32_t system_uptime_ms;           /* 系统运行时间 */
    uint32_t cpu_usage_percent;          /* CPU使用率 */
    uint32_t free_heap_size;             /* 剩余堆大小 */
    uint32_t min_free_heap_size;         /* 最小剩余堆大小 */
    uint32_t task_switches;              /* 任务切换次数 */
    uint32_t ethercat_cycle_count;       /* EtherCAT周期计数 */
    uint16_t ethercat_state;             /* EtherCAT状态 */
    uint8_t  active_task_count;          /* 活动任务数量 */
} system_stats_t;

/**
 * @brief EtherCAT事件消息结构
 */
typedef struct {
    uint16_t event_type;                 /* 事件类型 */
    uint16_t event_data;                 /* 事件数据 */
    uint32_t timestamp;                  /* 时间戳 */
    uint8_t  priority;                   /* 优先级 */
} ethercat_event_msg_t;

/**
 * @brief 系统命令消息结构
 */
typedef struct {
    uint8_t  command_type;               /* 命令类型 */
    uint8_t  parameter_count;            /* 参数数量 */
    uint32_t parameters[4];              /* 参数数组 */
    uint32_t timestamp;                  /* 时间戳 */
} system_command_msg_t;

/**
 * @brief 任务性能统计结构
 */
typedef struct {
    char     task_name[16];              /* 任务名称 */
    uint32_t run_time_counter;           /* 运行时间计数器 */
    uint32_t run_time_percentage;        /* 运行时间百分比 */
    uint16_t stack_high_water_mark;      /* 堆栈高水位标记 */
    uint8_t  current_priority;           /* 当前优先级 */
    uint8_t  task_state;                 /* 任务状态 */
} task_stats_t;

/* ========================================================================== */
/* 全局变量声明 */
/* ========================================================================== */

/* 任务句柄 */
extern TaskHandle_t xTaskHandle_LEDBlink;
extern TaskHandle_t xTaskHandle_SystemMonitor;
extern TaskHandle_t xTaskHandle_UserInterface;
extern TaskHandle_t xTaskHandle_EtherCATApp;
extern TaskHandle_t xTaskHandle_EtherCATIO;
extern TaskHandle_t xTaskHandle_EtherCATSync;

/* 队列句柄 */
extern QueueHandle_t xQueue_EtherCATEvents;
extern QueueHandle_t xQueue_SystemCommands;
extern QueueHandle_t xQueue_IOData;
extern QueueHandle_t xQueue_DebugMessages;

/* 信号量句柄 */
extern SemaphoreHandle_t xMutex_EtherCATData;
extern SemaphoreHandle_t xMutex_SystemResources;
extern SemaphoreHandle_t xSemaphore_ESCInterrupt;
extern SemaphoreHandle_t xSemaphore_Sync0Event;

/* 事件组句柄 */
extern EventGroupHandle_t xEventGroup_System;
extern EventGroupHandle_t xEventGroup_EtherCAT;

/* 软件定时器句柄 */
extern TimerHandle_t xTimer_SystemWatchdog;
extern TimerHandle_t xTimer_PerformanceMonitor;

/* 消息缓冲区句柄 */
extern MessageBufferHandle_t xMessageBuffer_EtherCAT;
extern MessageBufferHandle_t xMessageBuffer_Debug;

/* ========================================================================== */
/* 函数声明 */
/* ========================================================================== */

/**
 * @brief 初始化FreeRTOS + EtherCAT集成系统
 * @return pdPASS: 成功, pdFAIL: 失败
 */
BaseType_t FreeRTOS_EtherCAT_Init(void);

/**
 * @brief 创建所有RTOS任务
 * @return pdPASS: 成功, pdFAIL: 失败
 */
BaseType_t FreeRTOS_CreateAllTasks(void);

/**
 * @brief 创建所有RTOS同步对象
 * @return pdPASS: 成功, pdFAIL: 失败
 */
BaseType_t FreeRTOS_CreateSyncObjects(void);

/**
 * @brief 启动RTOS调度器前的最后配置
 */
void FreeRTOS_PreSchedulerConfig(void);

/**
 * @brief 验证RTOS功能
 * @return pdPASS: 验证成功, pdFAIL: 验证失败
 */
BaseType_t FreeRTOS_ValidateSystem(void);

/* ========================================================================== */
/* 任务函数声明 */
/* ========================================================================== */

/**
 * @brief LED闪烁任务 - 用于验证RTOS基本功能
 * @param pvParameters 任务参数
 */
void Task_LEDBlink(void *pvParameters);

/**
 * @brief 系统监控任务 - 监控系统性能和状态
 * @param pvParameters 任务参数
 */
void Task_SystemMonitor(void *pvParameters);

/**
 * @brief 用户界面任务 - 处理用户输入和显示
 * @param pvParameters 任务参数
 */
void Task_UserInterface(void *pvParameters);

/**
 * @brief EtherCAT应用任务 - 处理EtherCAT应用逻辑
 * @param pvParameters 任务参数
 */
void Task_EtherCATApplication(void *pvParameters);

/**
 * @brief EtherCAT IO任务 - 处理实时IO数据
 * @param pvParameters 任务参数
 */
void Task_EtherCATIO(void *pvParameters);

/**
 * @brief EtherCAT同步任务 - 处理分布式时钟同步
 * @param pvParameters 任务参数
 */
void Task_EtherCATSync(void *pvParameters);

/* ========================================================================== */
/* Hook函数声明 */
/* ========================================================================== */

/**
 * @brief 空闲任务Hook函数
 */
void vApplicationIdleHook(void);

/**
 * @brief 系统Tick Hook函数
 */
void vApplicationTickHook(void);

/**
 * @brief 内存分配失败Hook函数
 */
void vApplicationMallocFailedHook(void);

/**
 * @brief 堆栈溢出Hook函数
 * @param xTask 发生溢出的任务句柄
 * @param pcTaskName 任务名称
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);

/**
 * @brief 守护任务启动Hook函数
 */
void vApplicationDaemonTaskStartupHook(void);

/* ========================================================================== */
/* 辅助函数声明 */
/* ========================================================================== */

/**
 * @brief 获取系统统计信息
 * @param pStats 统计信息结构指针
 */
void FreeRTOS_GetSystemStats(system_stats_t *pStats);

/**
 * @brief 获取任务性能统计
 * @param pTaskStats 任务统计数组指针
 * @param max_tasks 最大任务数量
 * @return 实际任务数量
 */
uint8_t FreeRTOS_GetTaskStats(task_stats_t *pTaskStats, uint8_t max_tasks);

/**
 * @brief 打印系统信息到调试串口
 */
void FreeRTOS_PrintSystemInfo(void);

/**
 * @brief 运行时间统计定时器配置
 */
void ConfigureTimerForRunTimeStats(void);

/**
 * @brief 获取运行时间计数器值
 * @return 计数器值
 */
unsigned long GetRunTimeCounterValue(void);

/* ========================================================================== */
/* 软件定时器回调函数声明 */
/* ========================================================================== */

/**
 * @brief 系统看门狗定时器回调
 * @param xTimer 定时器句柄
 */
void Timer_SystemWatchdog_Callback(TimerHandle_t xTimer);

/**
 * @brief 性能监控定时器回调
 * @param xTimer 定时器句柄
 */
void Timer_PerformanceMonitor_Callback(TimerHandle_t xTimer);

/* ========================================================================== */
/* 错误处理函数声明 */
/* ========================================================================== */

/**
 * @brief RTOS错误处理函数
 * @param error_code 错误码
 * @param file_name 文件名
 * @param line_number 行号
 */
void FreeRTOS_ErrorHandler(uint32_t error_code, const char *file_name, uint32_t line_number);

/**
 * @brief 断言失败处理函数
 * @param file_name 文件名
 * @param line_number 行号
 */
void FreeRTOS_AssertFailed(const char *file_name, uint32_t line_number);

/* ========================================================================== */
/* 性能优化宏定义 */
/* ========================================================================== */

/* 进入/退出临界区的优化宏 */
#define RTOS_ENTER_CRITICAL()       taskENTER_CRITICAL()
#define RTOS_EXIT_CRITICAL()        taskEXIT_CRITICAL()

/* 从ISR中安全调用的宏 */
#define RTOS_YIELD_FROM_ISR(flag)   portYIELD_FROM_ISR(flag)

/* 任务通知的快速宏 */
#define RTOS_NOTIFY_TASK(handle, value, action) \
    xTaskNotify(handle, value, action)

#define RTOS_NOTIFY_TASK_FROM_ISR(handle, value, action, yield) \
    xTaskNotifyFromISR(handle, value, action, yield)

/* ========================================================================== */
/* 调试和诊断宏 */
/* ========================================================================== */

#ifdef DEBUG
    #define RTOS_DEBUG_PRINTF(...)      printf(__VA_ARGS__)
    #define RTOS_ASSERT(condition)      configASSERT(condition)
    #define RTOS_TRACE_ENTER_TASK(name) /* 可添加任务跟踪代码 */
    #define RTOS_TRACE_EXIT_TASK(name)  /* 可添加任务跟踪代码 */
#else
    #define RTOS_DEBUG_PRINTF(...)      /* 空操作 */
    #define RTOS_ASSERT(condition)      /* 空操作 */
    #define RTOS_TRACE_ENTER_TASK(name) /* 空操作 */
    #define RTOS_TRACE_EXIT_TASK(name)  /* 空操作 */
#endif

#ifdef __cplusplus
}
#endif

#endif /* __FREERTOS_ETHERCAT_INTEGRATION_H */