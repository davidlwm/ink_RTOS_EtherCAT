/**
 * @file    FreeRTOSConfig.h
 * @brief   FreeRTOS configuration for STM32F407 + EtherCAT project
 * @version FreeRTOS V10.4.6 (Stable)
 * @author  EtherCAT RTOS Integration Team
 * @date    2024-01-01
 *
 * @description
 * 本配置文件针对STM32F407ZE处理器和EtherCAT应用进行了优化：
 * - 支持高实时性EtherCAT通信
 * - 优化内存使用和任务调度
 * - 兼容HAL库和现有EtherCAT协议栈
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/* Ensure stdint is only used by the compiler, and not the assembler. */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
    #include <stdint.h>
    extern uint32_t SystemCoreClock;
#endif

/* ========================================================================== */
/* 核心调度器配置 */
/* ========================================================================== */

#define configUSE_PREEMPTION                     1
#define configSUPPORT_STATIC_ALLOCATION          1
#define configSUPPORT_DYNAMIC_ALLOCATION         1
#define configUSE_IDLE_HOOK                      1
#define configUSE_TICK_HOOK                      1
#define configCPU_CLOCK_HZ                       ( SystemCoreClock )
#define configTICK_RATE_HZ                       1000
#define configMAX_PRIORITIES                     ( 7 )
#define configMINIMAL_STACK_SIZE                 ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE                    ( ( size_t ) ( 64 * 1024 ) )
#define configMAX_TASK_NAME_LEN                  ( 16 )
#define configUSE_TRACE_FACILITY                 1
#define configUSE_16_BIT_TICKS                   0
#define configUSE_MUTEXES                        1
#define configQUEUE_REGISTRY_SIZE                8
#define configUSE_RECURSIVE_MUTEXES              1
#define configUSE_COUNTING_SEMAPHORES            1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  1

/* ========================================================================== */
/* 任务优先级定义 (针对EtherCAT应用优化) */
/* ========================================================================== */

/* 优先级范围: 0 (最低) ~ 6 (最高) */
#define configKERNEL_INTERRUPT_PRIORITY         255
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    191 /* equivalent to 0xb0, or priority 11. */

/* EtherCAT特定优先级定义 */
#define ETHERCAT_ESC_ISR_PRIORITY               5   /* EtherCAT ESC中断最高优先级 */
#define ETHERCAT_SYNC_TASK_PRIORITY             4   /* EtherCAT同步任务 */
#define ETHERCAT_IO_TASK_PRIORITY               3   /* EtherCAT IO处理任务 */
#define ETHERCAT_APP_TASK_PRIORITY              2   /* 应用层任务 */
#define SYSTEM_MONITOR_TASK_PRIORITY            1   /* 系统监控任务 */

/* ========================================================================== */
/* 内存管理配置 */
/* ========================================================================== */

#define configAPPLICATION_ALLOCATED_HEAP         0
#define configSTACK_DEPTH_TYPE                   uint16_t
#define configMESSAGE_BUFFER_LENGTH_TYPE         size_t

/* ========================================================================== */
/* Hook函数配置 */
/* ========================================================================== */

#define configUSE_DAEMON_TASK_STARTUP_HOOK       1
#define configUSE_MALLOC_FAILED_HOOK             1
#define configCHECK_FOR_STACK_OVERFLOW           2
#define configUSE_STATS_FORMATTING_FUNCTIONS     1

/* ========================================================================== */
/* 运行时统计配置 */
/* ========================================================================== */

#define configGENERATE_RUN_TIME_STATS            1
#ifdef configGENERATE_RUN_TIME_STATS
    extern void ConfigureTimerForRunTimeStats(void);
    extern unsigned long GetRunTimeCounterValue(void);
    #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() ConfigureTimerForRunTimeStats()
    #define portGET_RUN_TIME_COUNTER_VALUE()         GetRunTimeCounterValue()
#endif

/* ========================================================================== */
/* 软件定时器配置 */
/* ========================================================================== */

#define configUSE_TIMERS                         1
#define configTIMER_TASK_PRIORITY                ( 3 )
#define configTIMER_QUEUE_LENGTH                 5
#define configTIMER_TASK_STACK_DEPTH             ( configMINIMAL_STACK_SIZE * 2 )

/* ========================================================================== */
/* 队列和信号量配置 */
/* ========================================================================== */

#define configUSE_QUEUE_SETS                     1
#define configUSE_TIME_SLICING                   1
#define configUSE_NEWLIB_REENTRANT               0
#define configENABLE_BACKWARD_COMPATIBILITY      0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS  5

/* ========================================================================== */
/* 中断嵌套配置 (关键for EtherCAT) */
/* ========================================================================== */

/*
 * 中断优先级配置说明：
 * - EtherCAT ESC中断: 最高优先级 (0-10)
 * - SysTick: 优先级11 (configMAX_SYSCALL_INTERRUPT_PRIORITY)
 * - 其他外设中断: 优先级12-15
 */
#define configASSERT( x ) if ((x) == 0) {taskDISABLE_INTERRUPTS(); for( ;; );}

/* ========================================================================== */
/* 可选功能配置 */
/* ========================================================================== */

#define INCLUDE_vTaskPrioritySet                 1
#define INCLUDE_uxTaskPriorityGet                1
#define INCLUDE_vTaskDelete                      1
#define INCLUDE_vTaskCleanUpResources            1
#define INCLUDE_vTaskSuspend                     1
#define INCLUDE_vTaskDelayUntil                  1
#define INCLUDE_vTaskDelay                       1
#define INCLUDE_xTaskGetSchedulerState           1
#define INCLUDE_xTimerPendFunctionCall           1
#define INCLUDE_xTaskGetCurrentTaskHandle        1
#define INCLUDE_uxTaskGetStackHighWaterMark      1
#define INCLUDE_xTaskGetIdleTaskHandle           1
#define INCLUDE_eTaskGetState                    1
#define INCLUDE_xEventGroupSetBitFromISR         1
#define INCLUDE_xTimerPendFunctionCall           1
#define INCLUDE_xTaskAbortDelay                  1
#define INCLUDE_xTaskGetHandle                   1
#define INCLUDE_xTaskResumeFromISR               1

/* ========================================================================== */
/* Cortex-M4 specific definitions */
/* ========================================================================== */

#ifdef __NVIC_PRIO_BITS
    /* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
    #define configPRIO_BITS         __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS         4
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY   15

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 11

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* ========================================================================== */
/* EtherCAT特定配置 */
/* ========================================================================== */

/* EtherCAT相关任务堆栈大小 */
#define ETHERCAT_SYNC_TASK_STACK_SIZE           ( configMINIMAL_STACK_SIZE * 4 )
#define ETHERCAT_IO_TASK_STACK_SIZE             ( configMINIMAL_STACK_SIZE * 3 )
#define ETHERCAT_APP_TASK_STACK_SIZE            ( configMINIMAL_STACK_SIZE * 2 )

/* EtherCAT队列大小 */
#define ETHERCAT_EVENT_QUEUE_SIZE               10
#define ETHERCAT_DATA_QUEUE_SIZE                5

/* ========================================================================== */
/* 调试和性能监控 */
/* ========================================================================== */

#if defined(DEBUG) || defined(_DEBUG)
    #define configUSE_APPLICATION_TASK_TAG       1
    #define configRECORD_STACK_HIGH_ADDRESS      1
    #define traceTASK_SWITCHED_IN()              extern void trace_task_switched_in(void); trace_task_switched_in()
    #define traceTASK_SWITCHED_OUT()             extern void trace_task_switched_out(void); trace_task_switched_out()
#endif

/* ========================================================================== */
/* 兼容性定义 */
/* ========================================================================== */

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
/* USER CODE BEGIN 1 */
#define configASSERT( x ) if ((x) == 0) {taskDISABLE_INTERRUPTS(); for( ;; );}
/* USER CODE END 1 */

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler    SVC_Handler
#define xPortPendSVHandler PendSV_Handler

/* IMPORTANT: This define is commented when used with STM32Cube firmware, to prevent overwriting SysTick_Handler defined within STM32Cube HAL */
/* #define xPortSysTickHandler SysTick_Handler */

/* ========================================================================== */
/* 用户扩展配置区域 */
/* ========================================================================== */

/* USER CODE BEGIN Defines */
/* Section where parameter definitions can be added (for instance, to override default ones in FreeRTOS.h) */

/* 用户自定义Hook函数声明 */
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
//extern void vApplicationStackOverflowHook(void *xTask, char *pcTaskName);
extern void vApplicationDaemonTaskStartupHook(void);

/* EtherCAT集成相关函数声明 */
extern void EtherCAT_RTOS_Init(void);
extern void EtherCAT_CreateTasks(void);

/* USER CODE END Defines */

#endif /* FREERTOS_CONFIG_H */