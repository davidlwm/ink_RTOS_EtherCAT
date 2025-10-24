/**
  ******************************************************************************
  * @file    mcp23017_example.c
  * @brief   MCP23017 GPIO扩展芯片驱动使用示例
  * @note    这是一个MCP23017 I2C GPIO扩展器的使用示例，展示如何初始化和使用
  ******************************************************************************
  */

#include "mcp23017/MCP23017.h"
#include "usart/bsp_debug_usart.h"
#include "FreeRTOS.h"
#include "task.h"

/* MCP23017 设备句柄 */
static MCP23017_HandleTypeDef hmcp23017;

/* 外部I2C句柄声明 - 根据实际硬件修改 */
extern I2C_HandleTypeDef hi2c1;  // 根据你的I2C配置修改

/**
  * @brief  MCP23017使用示例 - 初始化
  * @note   在main函数或初始化任务中调用
  *         需要先初始化I2C外设
  */
void MCP23017_Example_Init(void)
{
    HAL_StatusTypeDef status;

    printf("MCP23017 Initialization...\r\n");

    /* 初始化MCP23017，地址为0x20 (A2=A1=A0=0) */
    status = mcp23017_init(&hmcp23017, &hi2c1, MCP23017_ADD_20);

    if (status == HAL_OK) {
        printf("MCP23017 Initialized successfully at address 0x%02X\r\n", MCP23017_ADD_20);
    } else {
        printf("Error: MCP23017 initialization failed! Check I2C connection.\r\n");
        return;
    }

    /* 配置示例：
     * Port A: 全部配置为输出 (用于控制LED或继电器等)
     * Port B: 全部配置为输入上拉 (用于读取按键或开关状态)
     */

    // 配置Port A为输出
    status = mcp23017_portMode(&hmcp23017, MCP23017Port_A,
                               MCP23017_PIN_MODE_OUTPUT,
                               MCP23017_PIN_POLARITY_NORMAL);
    if (status == HAL_OK) {
        printf("Port A configured as OUTPUT\r\n");
    }

    // 配置Port B为输入上拉
    status = mcp23017_portMode(&hmcp23017, MCP23017Port_B,
                               MCP23017_PIN_MODE_INPUT_PULLUP,
                               MCP23017_PIN_POLARITY_NORMAL);
    if (status == HAL_OK) {
        printf("Port B configured as INPUT with PULLUP\r\n");
    }

    // 初始化Port A所有输出为低电平
    mcp23017_writePort(&hmcp23017, MCP23017Port_A, 0x00);

    printf("MCP23017 configuration completed\r\n\r\n");
}

/**
  * @brief  MCP23017使用示例 - 单个引脚操作
  * @note   演示如何读写单个引脚
  */
void MCP23017_Example_PinOperation(void)
{
    HAL_StatusTypeDef status;
    uint8_t pin_value;

    printf("=== MCP23017 Pin Operation Example ===\r\n");

    /* 写单个引脚示例 - 设置GPA0为高电平 */
    status = mcp23017_digitalWrite(&hmcp23017, MCP23017_GPA0_Pin, GPIO_PIN_SET);
    if (status == HAL_OK) {
        printf("GPA0 set to HIGH\r\n");
    }

    HAL_Delay(500);

    /* 设置GPA0为低电平 */
    mcp23017_digitalWrite(&hmcp23017, MCP23017_GPA0_Pin, GPIO_PIN_RESET);
    printf("GPA0 set to LOW\r\n");

    /* 读单个引脚示例 - 读取GPB0状态 */
    status = mcp23017_digitalRead(&hmcp23017, MCP23017_GPB0_Pin, &pin_value);
    if (status == HAL_OK) {
        printf("GPB0 state: %s\r\n", pin_value ? "HIGH" : "LOW");
    }

    printf("\r\n");
}

/**
  * @brief  MCP23017使用示例 - 端口操作
  * @note   演示如何一次读写整个端口(8个引脚)
  */
void MCP23017_Example_PortOperation(void)
{
    HAL_StatusTypeDef status;
    uint8_t port_value;

    printf("=== MCP23017 Port Operation Example ===\r\n");

    /* 写整个Port A - 设置不同的位模式 */
    // 0b10101010 = 0xAA (奇数位高，偶数位低)
    status = mcp23017_writePort(&hmcp23017, MCP23017Port_A, 0xAA);
    if (status == HAL_OK) {
        printf("Port A set to 0xAA (10101010)\r\n");
    }

    HAL_Delay(500);

    // 0b01010101 = 0x55 (偶数位高，奇数位低)
    mcp23017_writePort(&hmcp23017, MCP23017Port_A, 0x55);
    printf("Port A set to 0x55 (01010101)\r\n");

    HAL_Delay(500);

    /* 读整个Port B */
    status = mcp23017_readPort(&hmcp23017, MCP23017Port_B, &port_value);
    if (status == HAL_OK) {
        printf("Port B value: 0x%02X (", port_value);
        // 打印二进制格式
        for (int i = 7; i >= 0; i--) {
            printf("%d", (port_value >> i) & 1);
        }
        printf(")\r\n");
    }

    printf("\r\n");
}

/**
  * @brief  MCP23017使用示例 - 配置单个引脚
  * @note   演示如何配置单个引脚的模式
  */
void MCP23017_Example_PinConfig(void)
{
    printf("=== MCP23017 Pin Configuration Example ===\r\n");

    /* 将GPA7配置为输出 */
    mcp23017_pinMode(&hmcp23017, MCP23017_GPA7_Pin,
                     MCP23017_PIN_MODE_OUTPUT,
                     MCP23017_PIN_POLARITY_NORMAL);
    printf("GPA7 configured as OUTPUT\r\n");

    /* 将GPB3配置为输入上拉 */
    mcp23017_pinMode(&hmcp23017, MCP23017_GPB3_Pin,
                     MCP23017_PIN_MODE_INPUT_PULLUP,
                     MCP23017_PIN_POLARITY_NORMAL);
    printf("GPB3 configured as INPUT_PULLUP\r\n");

    /* 将GPB5配置为输入(无上拉) */
    mcp23017_pinMode(&hmcp23017, MCP23017_GPB5_Pin,
                     MCP23017_PIN_MODE_INPUT,
                     MCP23017_PIN_POLARITY_NORMAL);
    printf("GPB5 configured as INPUT (no pullup)\r\n");

    printf("\r\n");
}

/**
  * @brief  MCP23017 LED闪烁示例
  * @note   在Port A连接8个LED，演示流水灯效果
  */
void MCP23017_Example_LEDChase(void)
{
    printf("=== MCP23017 LED Chase Example ===\r\n");
    printf("Starting LED chase on Port A...\r\n");

    // 流水灯效果
    for (uint8_t i = 0; i < 8; i++) {
        uint8_t pattern = 1 << i;  // 依次点亮每个LED
        mcp23017_writePort(&hmcp23017, MCP23017Port_A, pattern);
        printf("LED pattern: 0x%02X\r\n", pattern);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // 反向流水
    for (int8_t i = 7; i >= 0; i--) {
        uint8_t pattern = 1 << i;
        mcp23017_writePort(&hmcp23017, MCP23017Port_A, pattern);
        printf("LED pattern: 0x%02X\r\n", pattern);
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // 全部关闭
    mcp23017_writePort(&hmcp23017, MCP23017Port_A, 0x00);
    printf("All LEDs OFF\r\n\r\n");
}

/**
  * @brief  MCP23017 按键扫描示例
  * @note   在Port B连接按键，扫描并显示状态
  */
void MCP23017_Example_KeyScan(void)
{
    uint8_t port_b_value;
    static uint8_t last_value = 0xFF;

    /* 读取Port B状态 */
    if (mcp23017_readPort(&hmcp23017, MCP23017Port_B, &port_b_value) == HAL_OK) {
        /* 检测状态变化 */
        if (port_b_value != last_value) {
            printf("Port B changed: 0x%02X -> 0x%02X\r\n", last_value, port_b_value);

            /* 检查每个位的变化 */
            for (uint8_t i = 0; i < 8; i++) {
                uint8_t current_bit = (port_b_value >> i) & 1;
                uint8_t last_bit = (last_value >> i) & 1;

                if (current_bit != last_bit) {
                    printf("  GPB%d: %s -> %s\r\n", i,
                           last_bit ? "HIGH" : "LOW",
                           current_bit ? "HIGH" : "LOW");
                }
            }

            last_value = port_b_value;
        }
    }
}

/**
  * @brief  MCP23017控制任务示例
  * @note   这是一个FreeRTOS任务示例，演示MCP23017的周期性操作
  */
void MCP23017_Task_Example(void *pvParameters)
{
    uint8_t counter = 0;

    printf("MCP23017 Task Started\r\n");

    while (1) {
        /* 在Port A上显示计数器值 */
        mcp23017_writePort(&hmcp23017, MCP23017Port_A, counter);

        /* 读取Port B并显示 */
        uint8_t port_b_value;
        if (mcp23017_readPort(&hmcp23017, MCP23017Port_B, &port_b_value) == HAL_OK) {
            printf("Counter: %3d, Port A: 0x%02X, Port B: 0x%02X\r\n",
                   counter, counter, port_b_value);
        }

        counter++;

        /* 延时1秒后再次更新 */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/**
  * @brief  创建MCP23017控制任务
  * @note   在main函数中调用此函数创建任务
  */
void MCP23017_CreateTask(void)
{
    BaseType_t xReturn = pdPASS;

    xReturn = xTaskCreate(MCP23017_Task_Example,    /* 任务入口函数 */
                         "MCP23017_Task",           /* 任务名称 */
                         512,                       /* 任务栈大小 */
                         NULL,                      /* 任务参数 */
                         5,                         /* 任务优先级 */
                         NULL);                     /* 任务句柄 */

    if (xReturn != pdPASS) {
        printf("Failed to create MCP23017 task\r\n");
    }
}

/**
  * @brief  MCP23017完整测试序列
  * @note   按顺序执行所有测试示例
  */
void MCP23017_Example_FullTest(void)
{
    printf("\r\n");
    printf("========================================\r\n");
    printf("  MCP23017 Full Test Sequence\r\n");
    printf("========================================\r\n\r\n");

    /* 1. 初始化 */
    MCP23017_Example_Init();
    vTaskDelay(pdMS_TO_TICKS(500));

    /* 2. 单引脚操作测试 */
    MCP23017_Example_PinOperation();
    vTaskDelay(pdMS_TO_TICKS(500));

    /* 3. 端口操作测试 */
    MCP23017_Example_PortOperation();
    vTaskDelay(pdMS_TO_TICKS(500));

    /* 4. 引脚配置测试 */
    MCP23017_Example_PinConfig();
    vTaskDelay(pdMS_TO_TICKS(500));

    /* 5. LED流水灯测试 */
    MCP23017_Example_LEDChase();
    vTaskDelay(pdMS_TO_TICKS(500));

    printf("========================================\r\n");
    printf("  Test Sequence Completed\r\n");
    printf("========================================\r\n\r\n");
}

/* 使用方法:
 *
 * 硬件连接：
 * - MCP23017 SDA -> STM32 I2C_SDA
 * - MCP23017 SCL -> STM32 I2C_SCL
 * - MCP23017 VDD -> 3.3V 或 5V
 * - MCP23017 VSS -> GND
 * - MCP23017 A0/A1/A2 -> GND (地址0x20)
 * - Port A: 连接LED或其他输出设备
 * - Port B: 连接按键或开关(使用内部上拉)
 *
 * 软件使用：
 *
 * 1. 基本初始化 (在main函数中):
 *    MCP23017_Example_Init();
 *
 * 2. 运行完整测试:
 *    MCP23017_Example_FullTest();
 *
 * 3. 创建周期性控制任务:
 *    MCP23017_CreateTask();
 *
 * 4. 在应用程序中使用:
 *    // 初始化
 *    mcp23017_init(&hmcp23017, &hi2c1, MCP23017_ADD_20);
 *
 *    // 配置引脚
 *    mcp23017_pinMode(&hmcp23017, MCP23017_GPA0_Pin,
 *                     MCP23017_PIN_MODE_OUTPUT,
 *                     MCP23017_PIN_POLARITY_NORMAL);
 *
 *    // 写引脚
 *    mcp23017_digitalWrite(&hmcp23017, MCP23017_GPA0_Pin, GPIO_PIN_SET);
 *
 *    // 读引脚
 *    uint8_t value;
 *    mcp23017_digitalRead(&hmcp23017, MCP23017_GPB0_Pin, &value);
 *
 *    // 写整个端口
 *    mcp23017_writePort(&hmcp23017, MCP23017Port_A, 0xFF);
 *
 *    // 读整个端口
 *    uint8_t port_value;
 *    mcp23017_readPort(&hmcp23017, MCP23017Port_B, &port_value);
 *
 * 注意事项：
 * - 确保I2C外设已正确初始化
 * - 根据实际使用的I2C端口修改 hi2c1
 * - 根据硬件地址设置修改 MCP23017_ADD_XX
 * - Port A和Port B各有8个引脚，可独立配置
 * - 支持输入/输出/输入上拉模式
 * - 支持整体端口读写，提高效率
 */
