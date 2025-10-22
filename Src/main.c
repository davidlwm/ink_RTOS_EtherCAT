
/* 包含头文件 ----------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "led/bsp_led.h"
#include "GeneralTIM/bsp_GeneralTIM.h"
#include "usart/bsp_debug_usart.h"
#include <stdio.h>

/* FreeRTOS 头文件 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

/* EtherCAT 头文件 */
#include "ecat_def.h"
#include "applInterface.h"
#include "ecatslv.h"

/* 传感器模拟和桥接模块 */
#include "sensor_simulator.h"
#include "ethercat_sensor_bridge.h"
#include "sensor_tasks.h"



/* 私有函数原型 --------------------------------------------------------------*/
void Task_LEDBlink(void *pvParameters);
void Task_SystemMonitor(void *pvParameters);
void Task_EtherCATApplication(void *pvParameters);
void Task_EtherCATMainLoop(void *pvParameters);

/* FreeRTOS任务句柄 */
TaskHandle_t xTaskHandle_LEDBlink = NULL;
TaskHandle_t xTaskHandle_SystemMonitor = NULL;
TaskHandle_t xTaskHandle_EtherCATApp = NULL;
TaskHandle_t xTaskHandle_EtherCATMainLoop = NULL;

/* 硬件句柄 */
ADC_HandleTypeDef hadc1;
DAC_HandleTypeDef hdac;
TIM_HandleTypeDef htim3;

/* 测试变量 ------------------------------------------------------------------*/
static uint32_t system_counter = 0;



/**
  * 函数功能: 主函数 - 集成FreeRTOS  + EtherCAT.
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 使用FreeRTOS兼容API运行EtherCAT系统
  */
int main(void)
{
    /* 复位所有外设，初始化Flash接口和系统滴答定时器 */
    HAL_Init();

    /* 配置系统时钟 */
    SystemClock_Config();

    /* 初始化IO操作管脚 */
    LED_GPIO_Init();

    /* 初始化调试串口 */
    MX_DEBUG_USART_Init();

    /* 初始化EtherCAT硬件 */
    HW_Init();

    /* 初始化EtherCAT主程序 */
    MainInit();

    /* 初始化传感器模拟器 */
    if (SensorSimulator_Init(NULL) != 0) {
        //printf("ERROR: Failed to initialize sensor simulator!\r\n");
    }

    /* 初始化EtherCAT传感器桥接 */
    if (EtherCAT_SensorBridge_Init(NULL) != 0) {
        //printf("ERROR: Failed to initialize EtherCAT sensor bridge!\r\n");
    }

    /* 启动传感器模拟器 */
    SensorSimulator_Enable(true);

    /* 启动EtherCAT传感器桥接 */
    EtherCAT_SensorBridge_Start();

    /* 初始化传感器任务系统 */
    if (Sensor_Tasks_Init() != pdPASS) {
        //printf("ERROR: Failed to initialize sensor tasks!\r\n");
    }

    /* 打印启动信息 */
    // printf("\r\n=================================\r\n");
    // printf("FreeRTOS + EtherCAT Integration\r\n");
    // printf("Using Real FreeRTOS V10.4.6\r\n");
    // printf("System Starting...\r\n");
    // printf("=================================\r\n");

    /* 创建FreeRTOS任务 */
    if(xTaskCreate(Task_LEDBlink,
                   "LED_Blink",
                   configMINIMAL_STACK_SIZE,
                   NULL,
                   1,
                   &xTaskHandle_LEDBlink) != pdPASS)
    {
        //printf("ERROR: Failed to create LED Blink task!\r\n");
    }

    if(xTaskCreate(Task_SystemMonitor,
                   "Sys_Monitor",
                   configMINIMAL_STACK_SIZE * 2,
                   NULL,
                   2,
                   &xTaskHandle_SystemMonitor) != pdPASS)
    {
				configASSERT( pdFAIL );
        //printf("ERROR: Failed to create System Monitor task!\r\n");
    }

    if(xTaskCreate(Task_EtherCATApplication,
                   "EtherCAT_App",
                   configMINIMAL_STACK_SIZE * 2,
                   NULL,
                   ETHERCAT_APP_TASK_PRIORITY,
                   &xTaskHandle_EtherCATApp) != pdPASS)
    {
			configASSERT( pdFAIL );
        //printf("ERROR: Failed to create EtherCAT Application task!\r\n");
    }

    if(xTaskCreate(Task_EtherCATMainLoop,
                   "EtherCAT_Loop",
                   configMINIMAL_STACK_SIZE * 3,
                   NULL,
                   ETHERCAT_SYNC_TASK_PRIORITY,
                   &xTaskHandle_EtherCATMainLoop) != pdPASS)
    {
        //printf("ERROR: Failed to create EtherCAT MainLoop task!\r\n");
    }

    /* 创建传感器相关任务 */
    if (Sensor_Tasks_Create() != pdPASS) {
        //printf("ERROR: Failed to create Sensor tasks!\r\n");
    }

    //printf("All tasks created successfully\r\n");

    /* 启动FreeRTOS调度器 */
    //printf("Starting FreeRTOS Scheduler...\r\n");
    vTaskStartScheduler();

    /* 调度器启动失败才会到达这里 */
    //printf("CRITICAL ERROR: Scheduler failed to start!\r\n");
    while(1)
    {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
        HAL_Delay(100);
    }
}

/**
  * 函数功能: LED闪烁任务
  * 输入参数: pvParameters - 任务参数
  * 返 回 值: 无
  * 说    明: 定期闪烁LED指示系统运行状态
  */
void Task_LEDBlink(void *pvParameters)
{
    uint32_t led_counter = 0;

    for(;;)
    {
        led_counter++;

        /* LED闪烁 */
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_11);

        printf("LED Blink Task: %lu\r\n", led_counter);

        /* 任务延时500ms */
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

/**
  * 函数功能: 系统监控任务
  * 输入参数: pvParameters - 任务参数
  * 返 回 值: 无
  * 说    明: 监控系统状态和性能
  */
void Task_SystemMonitor(void *pvParameters)
{
    uint32_t monitor_counter = 0;

    for(;;)
    {
        monitor_counter++;
        system_counter++;

        /* 打印系统状态信息 */
        if ((monitor_counter % 10) == 0) {
            // printf("\r\n=== FreeRTOS System Status ===\r\n");
            // printf("System Ticks: %lu\r\n", xTaskGetTickCount());
            // printf("Active Tasks: %d\r\n", (int)uxTaskGetNumberOfTasks());
            // printf("Free Heap: %d bytes\r\n", (int)xPortGetFreeHeapSize());
            // printf("===============================\r\n");

            /* 检查EtherCAT状态 */
            extern uint16_t nAlState;
            printf("EtherCAT AL State: 0x%04X\r\n", nAlState);
        }

        /* 任务延时1000ms */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
  * 函数功能: EtherCAT应用任务
  * 输入参数: pvParameters - 任务参数
  * 返 回 值: 无
  * 说    明: 处理EtherCAT应用层逻辑
  */
void Task_EtherCATApplication(void *pvParameters)
{
    uint32_t app_counter = 0;

    for(;;)
    {
        app_counter++;

        /* 调用EtherCAT应用函数 */
        APPL_Application();

        /* 定期打印应用状态 */
        if ((app_counter % 100) == 0) {
            printf("EtherCAT App Task: %lu cycles\r\n", app_counter);
        }

        /* 任务延时10ms - EtherCAT应用层处理频率 */
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/**
  * 函数功能: EtherCAT主循环任务
  * 输入参数: pvParameters - 任务参数
  * 返 回 值: 无
  * 说    明: 处理EtherCAT高频轮询 - 最高优先级任务
  */
void Task_EtherCATMainLoop(void *pvParameters)
{
    uint32_t loop_counter = 0;

    for(;;)
    {
        loop_counter++;

        /* 调用EtherCAT主循环 - 高频处理 */
        MainLoop();

        /* 定期打印MainLoop状态 */
        if ((loop_counter % 10000) == 0) {
            printf("EtherCAT MainLoop: %lu cycles\r\n", loop_counter);
        }

        /* 任务延时1ms - 高频率EtherCAT处理 */
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

  

/**
  * 函数功能: 系统时钟配置
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
 
  __HAL_RCC_PWR_CLK_ENABLE();                                     //使能PWR时钟

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);  //设置调压器输出电压级别1

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;      // 外部晶振，8MHz
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;                        //打开HSE 
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                    //打开PLL
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;            //PLL时钟源选择HSE
  RCC_OscInitStruct.PLL.PLLM = 8;                                 //8分频MHz
  RCC_OscInitStruct.PLL.PLLN = 336;                               //336倍频
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;                     //2分频，得到168MHz主时钟
  RCC_OscInitStruct.PLL.PLLQ = 7;                                 //USB/SDIO/随机数产生器等的主PLL分频系数
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;       // 系统时钟：168MHz
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;              // AHB时钟： 168MHz
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;               // APB1时钟：42MHz
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;               // APB2时钟：84MHz
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  HAL_RCC_EnableCSS();                                            // 使能CSS功能，优先使用外部晶振，内部时钟源为备用
  
 	// HAL_RCC_GetHCLKFreq()/1000    1ms中断一次
	// HAL_RCC_GetHCLKFreq()/100000	 10us中断一次
	// HAL_RCC_GetHCLKFreq()/1000000 1us中断一次
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);                // 配置并启动系统滴答定时器
  /* 系统滴答定时器时钟源 */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* 系统滴答定时器中断优先级配置 */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


/* ========================================================================== */
/* FreeRTOS Hook Functions Implementation */
/* ========================================================================== */

/**
 * @brief EtherCAT AL State variable
 */
uint16_t nAlState = 0;

/**
 * @brief Idle task hook
 */
void vApplicationIdleHook(void)
{
    __WFI(); /* Wait for interrupt */
}

/**
 * @brief Tick hook function
 */
void vApplicationTickHook(void)
{
    /* Can be used for runtime statistics */
}

/**
 * @brief Malloc failed hook
 */
void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    while(1)
    {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
        for(volatile uint32_t i = 0; i < 1000000; i++);
    }
}

/**
 * @brief Stack overflow hook
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    while(1)
    {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_12);
        for(volatile uint32_t i = 0; i < 500000; i++);
    }
}

/**
 * @brief Daemon task startup hook
 */
void vApplicationDaemonTaskStartupHook(void)
{
    /* Timer service task startup */
}

/* ========================================================================== */
/* Runtime Statistics Functions */
/* ========================================================================== */

static volatile uint32_t ulRunTimeCounter = 0;

/**
 * @brief Configure timer for runtime statistics
 */
void ConfigureTimerForRunTimeStats(void)
{
    ulRunTimeCounter = 0;
}

/**
 * @brief Get runtime counter value
 */
unsigned long GetRunTimeCounterValue(void)
{
    return ulRunTimeCounter++;
}

/* ========================================================================== */
/* Static Allocation Functions (for FreeRTOS static allocation) */
/* ========================================================================== */

static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

/**
 * @brief Get memory for Idle task (static allocation)
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

/**
 * @brief Get memory for Timer task (static allocation)
 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
    *ppxTimerTaskStackBuffer = &xTimerStack[0];
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

