/**
 * @file    freertos_ethercat_integration.c
 * @brief   FreeRTOS + EtherCAT集成实现文件
 * @author  EtherCAT RTOS Integration Team
 * @version 1.0.0
 * @date    2024-01-01
 *
 * @description
 * 本文件实现了FreeRTOS与EtherCAT系统的集成功能：
 * - 任务创建和管理
 * - 同步对象创建
 * - 系统监控和性能统计
 * - Hook函数实现
 */

#include "freertos_ethercat_integration.h"
#include "SSC-Device.h"
#include "ecatslv.h"
#include "applInterface.h"

/* ========================================================================== */
/* 全局变量定义 */
/* ========================================================================== */

/* 任务句柄 */
TaskHandle_t xTaskHandle_LEDBlink = NULL;
TaskHandle_t xTaskHandle_SystemMonitor = NULL;
TaskHandle_t xTaskHandle_UserInterface = NULL;
TaskHandle_t xTaskHandle_EtherCATApp = NULL;
TaskHandle_t xTaskHandle_EtherCATIO = NULL;
TaskHandle_t xTaskHandle_EtherCATSync = NULL;

/* 队列句柄 */
QueueHandle_t xQueue_EtherCATEvents = NULL;
QueueHandle_t xQueue_SystemCommands = NULL;
QueueHandle_t xQueue_IOData = NULL;
QueueHandle_t xQueue_DebugMessages = NULL;

/* 信号量句柄 */
SemaphoreHandle_t xMutex_EtherCATData = NULL;
SemaphoreHandle_t xMutex_SystemResources = NULL;
SemaphoreHandle_t xSemaphore_ESCInterrupt = NULL;
SemaphoreHandle_t xSemaphore_Sync0Event = NULL;

/* 事件组句柄 */
EventGroupHandle_t xEventGroup_System = NULL;
EventGroupHandle_t xEventGroup_EtherCAT = NULL;

/* 软件定时器句柄 */
TimerHandle_t xTimer_SystemWatchdog = NULL;
TimerHandle_t xTimer_PerformanceMonitor = NULL;

/* 消息缓冲区句柄 */
MessageBufferHandle_t xMessageBuffer_EtherCAT = NULL;
MessageBufferHandle_t xMessageBuffer_Debug = NULL;

/* 私有变量 */
static uint32_t ulRunTimeStatsCounterValue = 0;
static system_stats_t SystemStats = {0};

/* ========================================================================== */
/* 任务实现 */
/* ========================================================================== */

/**
 * @brief LED闪烁任务 - 用于验证RTOS基本功能
 */
void Task_LEDBlink(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK_PERIOD_LED_BLINK);

    /* 初始化延迟时间 */
    xLastWakeTime = xTaskGetTickCount();

    RTOS_DEBUG_PRINTF("LED Blink Task Started\r\n");

    for(;;)
    {
        /* LED切换逻辑 */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);

        /* 等待下一个周期 */
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief 系统监控任务 - 监控系统性能和状态
 */
void Task_SystemMonitor(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK_PERIOD_SYSTEM_MONITOR);

    xLastWakeTime = xTaskGetTickCount();

    RTOS_DEBUG_PRINTF("System Monitor Task Started\r\n");

    for(;;)
    {
        /* 更新系统统计信息 */
        FreeRTOS_GetSystemStats(&SystemStats);

        /* 检查系统健康状况 */
        if(SystemStats.free_heap_size < 1024)
        {
            RTOS_DEBUG_PRINTF("Warning: Low heap memory: %lu bytes\r\n", SystemStats.free_heap_size);
        }

        /* 设置系统就绪标志 */
        xEventGroupSetBits(xEventGroup_System, EVENT_FLAG_SYSTEM_STARTUP);

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief EtherCAT应用任务 - 处理EtherCAT应用逻辑
 */
void Task_EtherCATApplication(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK_PERIOD_ETHERCAT_APP);

    xLastWakeTime = xTaskGetTickCount();

    RTOS_DEBUG_PRINTF("EtherCAT Application Task Started\r\n");

    /* 等待系统初始化完成 */
    xEventGroupWaitBits(xEventGroup_System,
                        EVENT_FLAG_SYSTEM_STARTUP,
                        pdFALSE,
                        pdTRUE,
                        portMAX_DELAY);

    for(;;)
    {
        /* 调用EtherCAT应用函数 */
        if(xSemaphoreTake(xMutex_EtherCATData, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            APPL_Application();
            xSemaphoreGive(xMutex_EtherCATData);
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief EtherCAT IO任务 - 处理实时IO数据
 */
void Task_EtherCATIO(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(TASK_PERIOD_ETHERCAT_IO);

    xLastWakeTime = xTaskGetTickCount();

    RTOS_DEBUG_PRINTF("EtherCAT IO Task Started\r\n");

    for(;;)
    {
        /* 等待ESC中断信号 */
        if(xSemaphoreTake(xSemaphore_ESCInterrupt, xFrequency) == pdTRUE)
        {
            /* 处理EtherCAT主循环 */
            if(xSemaphoreTake(xMutex_EtherCATData, pdMS_TO_TICKS(5)) == pdTRUE)
            {
                MainLoop();
                xSemaphoreGive(xMutex_EtherCATData);
            }
        }
        else
        {
            /* 超时处理 - 仍然调用MainLoop确保系统正常运行 */
            if(xSemaphoreTake(xMutex_EtherCATData, pdMS_TO_TICKS(1)) == pdTRUE)
            {
                MainLoop();
                xSemaphoreGive(xMutex_EtherCATData);
            }
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/**
 * @brief EtherCAT同步任务 - 处理分布式时钟同步
 */
void Task_EtherCATSync(void *pvParameters)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(10); // 高频率同步任务

    xLastWakeTime = xTaskGetTickCount();

    RTOS_DEBUG_PRINTF("EtherCAT Sync Task Started\r\n");

    for(;;)
    {
        /* 等待Sync0事件 */
        if(xSemaphoreTake(xSemaphore_Sync0Event, xFrequency) == pdTRUE)
        {
            /* 处理同步事件 */
            xEventGroupSetBits(xEventGroup_EtherCAT, EVENT_FLAG_SYNC0_EVENT);
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/* ========================================================================== */
/* 初始化函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化FreeRTOS + EtherCAT集成系统
 */
BaseType_t FreeRTOS_EtherCAT_Init(void)
{
    BaseType_t xResult = pdPASS;

    RTOS_DEBUG_PRINTF("Initializing FreeRTOS + EtherCAT Integration...\r\n");

    /* 创建同步对象 */
    if(FreeRTOS_CreateSyncObjects() != pdPASS)
    {
        RTOS_DEBUG_PRINTF("Failed to create sync objects\r\n");
        return pdFAIL;
    }

    /* 创建所有任务 */
    if(FreeRTOS_CreateAllTasks() != pdPASS)
    {
        RTOS_DEBUG_PRINTF("Failed to create tasks\r\n");
        return pdFAIL;
    }

    /* 预调度器配置 */
    FreeRTOS_PreSchedulerConfig();

    RTOS_DEBUG_PRINTF("FreeRTOS + EtherCAT Integration Initialized Successfully\r\n");

    return xResult;
}

/**
 * @brief 创建所有RTOS任务
 */
BaseType_t FreeRTOS_CreateAllTasks(void)
{
    BaseType_t xResult = pdPASS;

    /* 创建LED闪烁任务 */
    if(xTaskCreate(Task_LEDBlink,
                   "LED_Blink",
                   TASK_STACK_SIZE_LED_BLINK,
                   NULL,
                   TASK_PRIORITY_LED_BLINK,
                   &xTaskHandle_LEDBlink) != pdPASS)
    {
        RTOS_DEBUG_PRINTF("Failed to create LED Blink task\r\n");
        xResult = pdFAIL;
    }

    /* 创建系统监控任务 */
    if(xTaskCreate(Task_SystemMonitor,
                   "Sys_Monitor",
                   TASK_STACK_SIZE_SYSTEM_MONITOR,
                   NULL,
                   TASK_PRIORITY_SYSTEM_MONITOR,
                   &xTaskHandle_SystemMonitor) != pdPASS)
    {
        RTOS_DEBUG_PRINTF("Failed to create System Monitor task\r\n");
        xResult = pdFAIL;
    }

    /* 创建EtherCAT应用任务 */
    if(xTaskCreate(Task_EtherCATApplication,
                   "EtherCAT_App",
                   TASK_STACK_SIZE_ETHERCAT_APP,
                   NULL,
                   TASK_PRIORITY_ETHERCAT_APP,
                   &xTaskHandle_EtherCATApp) != pdPASS)
    {
        RTOS_DEBUG_PRINTF("Failed to create EtherCAT App task\r\n");
        xResult = pdFAIL;
    }

    /* 创建EtherCAT IO任务 */
    if(xTaskCreate(Task_EtherCATIO,
                   "EtherCAT_IO",
                   TASK_STACK_SIZE_ETHERCAT_IO,
                   NULL,
                   TASK_PRIORITY_ETHERCAT_IO,
                   &xTaskHandle_EtherCATIO) != pdPASS)
    {
        RTOS_DEBUG_PRINTF("Failed to create EtherCAT IO task\r\n");
        xResult = pdFAIL;
    }

    /* 创建EtherCAT同步任务 */
    if(xTaskCreate(Task_EtherCATSync,
                   "EtherCAT_Sync",
                   TASK_STACK_SIZE_ETHERCAT_SYNC,
                   NULL,
                   TASK_PRIORITY_ETHERCAT_SYNC,
                   &xTaskHandle_EtherCATSync) != pdPASS)
    {
        RTOS_DEBUG_PRINTF("Failed to create EtherCAT Sync task\r\n");
        xResult = pdFAIL;
    }

    return xResult;
}

/**
 * @brief 创建所有RTOS同步对象
 */
BaseType_t FreeRTOS_CreateSyncObjects(void)
{
    BaseType_t xResult = pdPASS;

    /* 创建互斥量 */
    xMutex_EtherCATData = xSemaphoreCreateMutex();
    xMutex_SystemResources = xSemaphoreCreateMutex();

    /* 创建二进制信号量 */
    xSemaphore_ESCInterrupt = xSemaphoreCreateBinary();
    xSemaphore_Sync0Event = xSemaphoreCreateBinary();

    /* 创建事件组 */
    xEventGroup_System = xEventGroupCreate();
    xEventGroup_EtherCAT = xEventGroupCreate();

    /* 创建队列 */
    xQueue_EtherCATEvents = xQueueCreate(QUEUE_SIZE_ETHERCAT_EVENTS, sizeof(ethercat_event_msg_t));
    xQueue_SystemCommands = xQueueCreate(QUEUE_SIZE_SYSTEM_COMMANDS, sizeof(system_command_msg_t));
    xQueue_IOData = xQueueCreate(QUEUE_SIZE_IO_DATA, sizeof(uint32_t));
    xQueue_DebugMessages = xQueueCreate(QUEUE_SIZE_DEBUG_MESSAGES, sizeof(char*));

    /* 创建消息缓冲区 */
    xMessageBuffer_EtherCAT = xMessageBufferCreate(MESSAGE_BUFFER_SIZE_ETHERCAT);
    xMessageBuffer_Debug = xMessageBufferCreate(MESSAGE_BUFFER_SIZE_DEBUG);

    /* 创建软件定时器 */
    xTimer_SystemWatchdog = xTimerCreate("SysWatchdog",
                                         pdMS_TO_TICKS(5000),
                                         pdTRUE,
                                         NULL,
                                         Timer_SystemWatchdog_Callback);

    xTimer_PerformanceMonitor = xTimerCreate("PerfMonitor",
                                             pdMS_TO_TICKS(10000),
                                             pdTRUE,
                                             NULL,
                                             Timer_PerformanceMonitor_Callback);

    /* 检查是否所有对象都创建成功 */
    if(!xMutex_EtherCATData || !xMutex_SystemResources ||
       !xSemaphore_ESCInterrupt || !xSemaphore_Sync0Event ||
       !xEventGroup_System || !xEventGroup_EtherCAT ||
       !xQueue_EtherCATEvents || !xQueue_SystemCommands ||
       !xQueue_IOData || !xQueue_DebugMessages ||
       !xMessageBuffer_EtherCAT || !xMessageBuffer_Debug ||
       !xTimer_SystemWatchdog || !xTimer_PerformanceMonitor)
    {
        xResult = pdFAIL;
    }

    return xResult;
}

/**
 * @brief 启动RTOS调度器前的最后配置
 */
void FreeRTOS_PreSchedulerConfig(void)
{
    /* 启动软件定时器 */
    if(xTimer_SystemWatchdog != NULL)
    {
        xTimerStart(xTimer_SystemWatchdog, 0);
    }

    if(xTimer_PerformanceMonitor != NULL)
    {
        xTimerStart(xTimer_PerformanceMonitor, 0);
    }

    RTOS_DEBUG_PRINTF("RTOS Pre-scheduler configuration completed\r\n");
}

/**
 * @brief 验证RTOS功能
 */
BaseType_t FreeRTOS_ValidateSystem(void)
{
    /* 简单的系统验证 */
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        RTOS_DEBUG_PRINTF("RTOS System Validation: PASSED\r\n");
        return pdPASS;
    }
    else
    {
        RTOS_DEBUG_PRINTF("RTOS System Validation: FAILED\r\n");
        return pdFAIL;
    }
}

/* ========================================================================== */
/* Hook函数实现 */
/* ========================================================================== */

/**
 * @brief 空闲任务Hook函数
 */
void vApplicationIdleHook(void)
{
    /* 在空闲时进入低功耗模式或执行后台任务 */
    __WFI(); /* 等待中断 */
}

/**
 * @brief 系统Tick Hook函数
 */
void vApplicationTickHook(void)
{
    /* 运行时间统计计数器递增 */
    ulRunTimeStatsCounterValue++;
}

/**
 * @brief 内存分配失败Hook函数
 */
void vApplicationMallocFailedHook(void)
{
    RTOS_DEBUG_PRINTF("CRITICAL: Memory allocation failed!\r\n");
    /* 进入错误处理 */
    taskDISABLE_INTERRUPTS();
    for(;;)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // 点亮错误LED
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
        HAL_Delay(100);
    }
}

/**
 * @brief 堆栈溢出Hook函数
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    RTOS_DEBUG_PRINTF("CRITICAL: Stack overflow in task: %s\r\n", pcTaskName);
    /* 进入错误处理 */
    taskDISABLE_INTERRUPTS();
    for(;;)
    {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // 点亮错误LED
        HAL_Delay(200);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
        HAL_Delay(200);
    }
}

/**
 * @brief 守护任务启动Hook函数
 */
void vApplicationDaemonTaskStartupHook(void)
{
    RTOS_DEBUG_PRINTF("Daemon task startup hook called\r\n");
}

/* ========================================================================== */
/* 辅助函数实现 */
/* ========================================================================== */

/**
 * @brief 获取系统统计信息
 */
void FreeRTOS_GetSystemStats(system_stats_t *pStats)
{
    if(pStats != NULL)
    {
        pStats->system_uptime_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        pStats->free_heap_size = xPortGetFreeHeapSize();
        pStats->min_free_heap_size = xPortGetMinimumEverFreeHeapSize();
        pStats->active_task_count = uxTaskGetNumberOfTasks();
        pStats->task_switches = ulRunTimeStatsCounterValue;

        /* CPU使用率计算 (简化版本) */
        static uint32_t lastTickCount = 0;
        static uint32_t lastRunTimeCounter = 0;
        uint32_t currentTick = xTaskGetTickCount();
        uint32_t currentRunTime = ulRunTimeStatsCounterValue;

        if(currentTick > lastTickCount)
        {
            uint32_t tickDiff = currentTick - lastTickCount;
            uint32_t runTimeDiff = currentRunTime - lastRunTimeCounter;
            pStats->cpu_usage_percent = (runTimeDiff * 100) / (tickDiff * 1000);
        }

        lastTickCount = currentTick;
        lastRunTimeCounter = currentRunTime;
    }
}

/**
 * @brief 运行时间统计定时器配置
 */
void ConfigureTimerForRunTimeStats(void)
{
    /* 使用SysTick作为运行时间统计的时基 */
    /* 已经在vApplicationTickHook中处理 */
}

/**
 * @brief 获取运行时间计数器值
 */
unsigned long GetRunTimeCounterValue(void)
{
    return ulRunTimeStatsCounterValue;
}

/* ========================================================================== */
/* 软件定时器回调函数实现 */
/* ========================================================================== */

/**
 * @brief 系统看门狗定时器回调
 */
void Timer_SystemWatchdog_Callback(TimerHandle_t xTimer)
{
    /* 系统看门狗功能 */
    static uint32_t watchdog_counter = 0;
    watchdog_counter++;

    RTOS_DEBUG_PRINTF("System Watchdog: %lu\r\n", watchdog_counter);

    /* 检查系统健康状况 */
    if(xEventGroupGetBits(xEventGroup_System) & EVENT_FLAG_ERROR_OCCURRED)
    {
        RTOS_DEBUG_PRINTF("System error detected by watchdog\r\n");
    }
}

/**
 * @brief 性能监控定时器回调
 */
void Timer_PerformanceMonitor_Callback(TimerHandle_t xTimer)
{
    /* 性能监控功能 */
    FreeRTOS_GetSystemStats(&SystemStats);

    RTOS_DEBUG_PRINTF("Performance Monitor - CPU: %lu%%, Heap: %lu bytes\r\n",
                       SystemStats.cpu_usage_percent,
                       SystemStats.free_heap_size);
}

/* ========================================================================== */
/* 错误处理函数实现 */
/* ========================================================================== */

/**
 * @brief RTOS错误处理函数
 */
void FreeRTOS_ErrorHandler(uint32_t error_code, const char *file_name, uint32_t line_number)
{
    RTOS_DEBUG_PRINTF("RTOS Error: Code=0x%08lX, File=%s, Line=%lu\r\n",
                       error_code, file_name, line_number);

    /* 设置错误标志 */
    xEventGroupSetBits(xEventGroup_System, EVENT_FLAG_ERROR_OCCURRED);

    /* 进入错误处理模式 */
    taskDISABLE_INTERRUPTS();
    for(;;)
    {
        /* 错误指示 */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
        HAL_Delay(500);
    }
}

/**
 * @brief 断言失败处理函数
 */
void FreeRTOS_AssertFailed(const char *file_name, uint32_t line_number)
{
    RTOS_DEBUG_PRINTF("RTOS Assert Failed: File=%s, Line=%lu\r\n",
                       file_name, line_number);

    FreeRTOS_ErrorHandler(0xDEADBEEF, file_name, line_number);
}

/* ========================================================================== */
/* Static Allocation Hook Functions (Required for configSUPPORT_STATIC_ALLOCATION) */
/* ========================================================================== */

/* Static memory allocation for Idle Task */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

/**
 * @brief Hook function to provide static memory for Idle task
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* Static memory allocation for Timer Task */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

/**
 * @brief Hook function to provide static memory for Timer task
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
    *ppxTimerTaskStackBuffer = &xTimerStack[0];
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

/* ========================================================================== */
/* EtherCAT Global Variables */
/* ========================================================================== */

/**
 * @brief EtherCAT AL State variable
 * This variable tracks the current EtherCAT Application Layer state
 */
uint16_t nAlState = 0;