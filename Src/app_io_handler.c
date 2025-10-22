/**
 * @file    app_io_handler.c
 * @brief   EtherCAT从站扩展IO处理模块
 * @author  EtherCAT Application Developer
 * @version 1.0.0
 * @date    2024-01-01
 *
 * @description
 * 本模块实现了EtherCAT从站的扩展IO处理功能，包括：
 * - 16路数字输入/输出扩展
 * - 8路模拟输入 (ADC)
 * - 4路模拟输出 (DAC/PWM)
 * - 灵活的硬件配置
 * - 实时数据处理
 *
 * @note
 * - 保持与原有基础IO的兼容性
 * - 支持动态配置硬件映射
 * - 提供统计和调试功能
 */

#include "app_io_handler.h"
#include "main.h"
#include "SSC-Device.h"
#include <stdio.h>
#include <string.h>

// ====================================================================
// 全局变量定义
// ====================================================================

// EtherCAT对象实例
TOBJ6001_DigitalInputsExt Obj0x6001 = {2, 0, 0};
TOBJ7011_DigitalOutputsExt Obj0x7011 = {2, 0, 0};
TOBJ6002_AnalogInputs Obj0x6002 = {8, {0}};
TOBJ7012_AnalogOutputs Obj0x7012 = {4, {0}};

// IO统计信息
static io_statistics_t io_stats = {0};

// 调试配置
#if APP_IO_DEBUG_ENABLE
static uint32_t debug_period_ms = 1000;  // 默认1秒输出一次
static uint32_t debug_counter = 0;
#endif

// ====================================================================
// 硬件配置表 (可根据实际硬件修改)
// ====================================================================

// 数字输入引脚配置表
static digital_pin_config_t digital_input_pins[MAX_DIGITAL_INPUTS] = {
    // 原有配置 (保持兼容性)
    {GPIOE, GPIO_PIN_2,  1, 0},   // DI0  - Switch1 (原有)
    {GPIOE, GPIO_PIN_3,  1, 0},   // DI1  - Switch2 (原有)

    // 扩展配置 (根据实际硬件调整)
    {GPIOE, GPIO_PIN_4,  1, 1},   // DI2  - 扩展输入 (上拉)
    {GPIOE, GPIO_PIN_5,  1, 1},   // DI3  - 扩展输入
    {GPIOE, GPIO_PIN_6,  1, 1},   // DI4  - 扩展输入
    {GPIOE, GPIO_PIN_7,  1, 1},   // DI5  - 扩展输入
    {GPIOE, GPIO_PIN_8,  1, 1},   // DI6  - 扩展输入
    {GPIOE, GPIO_PIN_9,  1, 1},   // DI7  - 扩展输入
    {GPIOE, GPIO_PIN_10, 1, 1},   // DI8  - 扩展输入
    {GPIOE, GPIO_PIN_11, 1, 1},   // DI9  - 扩展输入
    {GPIOE, GPIO_PIN_12, 1, 1},   // DI10 - 扩展输入
    {GPIOE, GPIO_PIN_13, 1, 1},   // DI11 - 扩展输入
    {GPIOE, GPIO_PIN_14, 1, 1},   // DI12 - 扩展输入
    {GPIOE, GPIO_PIN_15, 1, 1},   // DI13 - 扩展输入
    {GPIOD, GPIO_PIN_8,  1, 1},   // DI14 - 扩展输入
    {GPIOD, GPIO_PIN_9,  1, 1},   // DI15 - 扩展输入
};

// 数字输出引脚配置表
static digital_pin_config_t digital_output_pins[MAX_DIGITAL_OUTPUTS] = {
    // 原有配置 (保持兼容性)
    {GPIOB, GPIO_PIN_11, 1, 0},   // DO0  - Led1 (原有)
    {GPIOB, GPIO_PIN_12, 1, 0},   // DO1  - Led2 (原有)

    // 扩展配置 (根据实际硬件调整)
    {GPIOB, GPIO_PIN_13, 1, 0},   // DO2  - 扩展输出
    {GPIOB, GPIO_PIN_14, 1, 0},   // DO3  - 扩展输出
    {GPIOB, GPIO_PIN_15, 1, 0},   // DO4  - 扩展输出
    {GPIOD, GPIO_PIN_10, 1, 0},   // DO5  - 扩展输出
    {GPIOD, GPIO_PIN_11, 1, 0},   // DO6  - 扩展输出
    {GPIOD, GPIO_PIN_12, 1, 0},   // DO7  - 扩展输出
    {GPIOD, GPIO_PIN_13, 1, 0},   // DO8  - 扩展输出
    {GPIOD, GPIO_PIN_14, 1, 0},   // DO9  - 扩展输出
    {GPIOD, GPIO_PIN_15, 1, 0},   // DO10 - 扩展输出
    {GPIOC, GPIO_PIN_6,  1, 0},   // DO11 - 扩展输出
    {GPIOC, GPIO_PIN_7,  1, 0},   // DO12 - 扩展输出
    {GPIOC, GPIO_PIN_8,  1, 0},   // DO13 - 扩展输出
    {GPIOC, GPIO_PIN_9,  1, 0},   // DO14 - 扩展输出
    {GPIOC, GPIO_PIN_10, 1, 0},   // DO15 - 扩展输出
};

// 模拟输入配置表 (需要根据实际ADC配置修改)
static analog_input_config_t analog_input_config[MAX_ANALOG_INPUTS] = {
    {&hadc1, ADC_CHANNEL_0,  1.0f, 0, 1},    // AI0 - PA0/ADC1_CH0
    {&hadc1, ADC_CHANNEL_1,  1.0f, 0, 1},    // AI1 - PA1/ADC1_CH1
    {&hadc1, ADC_CHANNEL_2,  1.0f, 0, 1},    // AI2 - PA2/ADC1_CH2
    {&hadc1, ADC_CHANNEL_3,  1.0f, 0, 1},    // AI3 - PA3/ADC1_CH3
    {&hadc1, ADC_CHANNEL_4,  1.0f, 0, 1},    // AI4 - PA4/ADC1_CH4
    {&hadc1, ADC_CHANNEL_5,  1.0f, 0, 1},    // AI5 - PA5/ADC1_CH5
    {&hadc1, ADC_CHANNEL_6,  1.0f, 0, 1},    // AI6 - PA6/ADC1_CH6
    {&hadc1, ADC_CHANNEL_7,  1.0f, 0, 1},    // AI7 - PA7/ADC1_CH7
};

// 模拟输出配置表 (需要根据实际DAC/PWM配置修改)
static analog_output_config_t analog_output_config[MAX_ANALOG_OUTPUTS] = {
    {&hdac, DAC_CHANNEL_1, NULL, 0, 1.0f, 0, 1},        // AO0 - PA4/DAC_CH1
    {&hdac, DAC_CHANNEL_2, NULL, 0, 1.0f, 0, 1},        // AO1 - PA5/DAC_CH2
    {NULL, 0, &htim3, TIM_CHANNEL_1, 1.0f, 0, 1},       // AO2 - PWM模拟输出
    {NULL, 0, &htim3, TIM_CHANNEL_2, 1.0f, 0, 1},       // AO3 - PWM模拟输出
};

// ====================================================================
// 私有函数声明
// ====================================================================
static void update_compatibility_objects(void);
static int16_t adc_to_standard_value(uint32_t adc_value, const analog_input_config_t* config);
static uint32_t standard_value_to_dac(int16_t value, const analog_output_config_t* config);
static uint32_t standard_value_to_pwm(int16_t value, const analog_output_config_t* config);

// ====================================================================
// 公共函数实现
// ====================================================================

/**
 * @brief IO处理模块初始化
 */
void App_IO_Init(void)
{
    // 初始化扩展对象
    Obj0x6001.u16SubIndex0 = 2;  // 配置项数量
    Obj0x6001.digital_inputs = 0;
    Obj0x6001.reserved = 0;

    Obj0x7011.u16SubIndex0 = 2;  // 配置项数量
    Obj0x7011.digital_outputs = 0;
    Obj0x7011.reserved = 0;

    Obj0x6002.u16SubIndex0 = MAX_ANALOG_INPUTS;
    for(int i = 0; i < MAX_ANALOG_INPUTS; i++) {
        Obj0x6002.channel[i] = 0;
    }

    Obj0x7012.u16SubIndex0 = MAX_ANALOG_OUTPUTS;
    for(int i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
        Obj0x7012.channel[i] = 0;
    }

    // 初始化统计信息
    memset(&io_stats, 0, sizeof(io_stats));
    io_stats.last_update_timestamp = HAL_GetTick();

#if APP_IO_DEBUG_ENABLE
    printf("[APP_IO] IO Handler initialized successfully\r\n");
    printf("[APP_IO] Digital IO: %d inputs, %d outputs\r\n",
           MAX_DIGITAL_INPUTS, MAX_DIGITAL_OUTPUTS);
    printf("[APP_IO] Analog IO: %d inputs, %d outputs\r\n",
           MAX_ANALOG_INPUTS, MAX_ANALOG_OUTPUTS);
#endif
}

/**
 * @brief 处理数字输入
 */
void App_Digital_Input_Process(void)
{
    uint16_t current_input_state = 0;
    uint16_t previous_state = Obj0x6001.digital_inputs;

    // 逐位读取数字输入状态
    for(int i = 0; i < MAX_DIGITAL_INPUTS; i++) {
        GPIO_PinState pin_state = HAL_GPIO_ReadPin(
            digital_input_pins[i].port,
            digital_input_pins[i].pin
        );

        // 根据有效电平设置位状态
        if((pin_state == GPIO_PIN_SET && digital_input_pins[i].active_level == 1) ||
           (pin_state == GPIO_PIN_RESET && digital_input_pins[i].active_level == 0)) {
            current_input_state |= (1 << i);
        }
    }

    // 更新扩展数字输入对象
    Obj0x6001.digital_inputs = current_input_state;

    // 检测状态变化
    if(current_input_state != previous_state) {
        io_stats.digital_input_changes++;
    }

    // 更新兼容性对象
    update_compatibility_objects();
}

/**
 * @brief 处理数字输出
 */
void App_Digital_Output_Process(void)
{
    uint16_t output_state = Obj0x7011.digital_outputs;

    // 逐位设置数字输出状态
    for(int i = 0; i < MAX_DIGITAL_OUTPUTS; i++) {
        GPIO_PinState pin_state;

        // 根据位状态和有效电平确定输出电平
        if(output_state & (1 << i)) {
            pin_state = digital_output_pins[i].active_level ? GPIO_PIN_SET : GPIO_PIN_RESET;
        } else {
            pin_state = digital_output_pins[i].active_level ? GPIO_PIN_RESET : GPIO_PIN_SET;
        }

        HAL_GPIO_WritePin(
            digital_output_pins[i].port,
            digital_output_pins[i].pin,
            pin_state
        );
    }

    // 更新兼容性对象
    update_compatibility_objects();
}

/**
 * @brief 处理模拟输入
 */
void App_Analog_Input_Process(void)
{
    for(int i = 0; i < MAX_ANALOG_INPUTS; i++) {
        if(analog_input_config[i].hadc != NULL && analog_input_config[i].enabled) {
            // 配置ADC通道
            ADC_ChannelConfTypeDef sConfig = {0};
            sConfig.Channel = analog_input_config[i].channel;
            sConfig.Rank = 1;
            sConfig.SamplingTime = ADC_SAMPLE_TIME;

            if(HAL_ADC_ConfigChannel(analog_input_config[i].hadc, &sConfig) == HAL_OK) {
                // 启动转换
                HAL_ADC_Start(analog_input_config[i].hadc);

                // 等待转换完成
                if(HAL_ADC_PollForConversion(analog_input_config[i].hadc, ADC_TIMEOUT_MS) == HAL_OK) {
                    uint32_t adc_value = HAL_ADC_GetValue(analog_input_config[i].hadc);

                    // 转换为标准化值
                    Obj0x6002.channel[i] = adc_to_standard_value(adc_value, &analog_input_config[i]);

                    io_stats.analog_input_samples++;
                } else {
                    // 转换超时，记录错误
                    io_stats.analog_conversion_errors++;
                }

                HAL_ADC_Stop(analog_input_config[i].hadc);
            } else {
                io_stats.analog_conversion_errors++;
            }
        }
    }
}

/**
 * @brief 处理模拟输出
 */
void App_Analog_Output_Process(void)
{
    for(int i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
        if(!analog_output_config[i].enabled) continue;

        int16_t target_value = Obj0x7012.channel[i];

        if(analog_output_config[i].hdac != NULL) {
            // DAC输出处理
            uint32_t dac_value = standard_value_to_dac(target_value, &analog_output_config[i]);
            HAL_DAC_SetValue(
                analog_output_config[i].hdac,
                analog_output_config[i].channel,
                DAC_ALIGNMENT,
                dac_value
            );
        } else if(analog_output_config[i].htim != NULL) {
            // PWM输出处理
            uint32_t pwm_value = standard_value_to_pwm(target_value, &analog_output_config[i]);
            __HAL_TIM_SET_COMPARE(
                analog_output_config[i].htim,
                analog_output_config[i].tim_channel,
                pwm_value
            );
        }
    }
}

/**
 * @brief IO处理主函数
 */
void App_IO_Handler(void)
{
    // 按顺序处理各种IO
    App_Digital_Input_Process();   // 处理数字输入
    App_Analog_Input_Process();    // 处理模拟输入
    App_Digital_Output_Process();  // 处理数字输出
    App_Analog_Output_Process();   // 处理模拟输出

    // 更新时间戳
    io_stats.last_update_timestamp = HAL_GetTick();

#if APP_IO_DEBUG_ENABLE
    App_IO_Debug_Print();
#endif
}

/**
 * @brief 获取IO统计信息
 */
void App_IO_GetStatistics(io_statistics_t* stats)
{
    if(stats != NULL) {
        memcpy(stats, &io_stats, sizeof(io_statistics_t));
    }
}

/**
 * @brief 设置数字输出通道
 */
int App_Set_Digital_Output(uint8_t channel, uint8_t state)
{
    if(channel >= MAX_DIGITAL_OUTPUTS) {
        return -1;  // 通道号超出范围
    }

    if(state) {
        Obj0x7011.digital_outputs |= (1 << channel);
    } else {
        Obj0x7011.digital_outputs &= ~(1 << channel);
    }

    return 0;
}

/**
 * @brief 读取数字输入通道
 */
int App_Get_Digital_Input(uint8_t channel)
{
    if(channel >= MAX_DIGITAL_INPUTS) {
        return -1;  // 通道号超出范围
    }

    return (Obj0x6001.digital_inputs & (1 << channel)) ? 1 : 0;
}

/**
 * @brief 设置模拟输出通道
 */
int App_Set_Analog_Output(uint8_t channel, int16_t value)
{
    if(channel >= MAX_ANALOG_OUTPUTS) {
        return -1;  // 通道号超出范围
    }

    Obj0x7012.channel[channel] = value;
    return 0;
}

/**
 * @brief 读取模拟输入通道
 */
int16_t App_Get_Analog_Input(uint8_t channel)
{
    if(channel >= MAX_ANALOG_INPUTS) {
        return INT16_MIN;  // 错误返回值
    }

    return Obj0x6002.channel[channel];
}

/**
 * @brief 配置数字输入引脚
 */
int App_Config_Digital_Input(uint8_t channel, GPIO_TypeDef* port, uint16_t pin, uint8_t active_level)
{
    if(channel >= MAX_DIGITAL_INPUTS || port == NULL) {
        return -1;
    }

    digital_input_pins[channel].port = port;
    digital_input_pins[channel].pin = pin;
    digital_input_pins[channel].active_level = active_level;

    return 0;
}

/**
 * @brief 配置数字输出引脚
 */
int App_Config_Digital_Output(uint8_t channel, GPIO_TypeDef* port, uint16_t pin, uint8_t active_level)
{
    if(channel >= MAX_DIGITAL_OUTPUTS || port == NULL) {
        return -1;
    }

    digital_output_pins[channel].port = port;
    digital_output_pins[channel].pin = pin;
    digital_output_pins[channel].active_level = active_level;

    return 0;
}

/**
 * @brief 配置模拟输入通道
 */
int App_Config_Analog_Input(uint8_t channel, ADC_HandleTypeDef* hadc, uint32_t adc_channel,
                            float scale_factor, int16_t offset)
{
    if(channel >= MAX_ANALOG_INPUTS) {
        return -1;
    }

    analog_input_config[channel].hadc = hadc;
    analog_input_config[channel].channel = adc_channel;
    analog_input_config[channel].scale_factor = scale_factor;
    analog_input_config[channel].offset = offset;
    analog_input_config[channel].enabled = (hadc != NULL) ? 1 : 0;

    return 0;
}

/**
 * @brief 配置模拟输出通道
 */
int App_Config_Analog_Output(uint8_t channel, DAC_HandleTypeDef* hdac, uint32_t dac_channel,
                             TIM_HandleTypeDef* htim, uint32_t tim_channel,
                             float scale_factor, int16_t offset)
{
    if(channel >= MAX_ANALOG_OUTPUTS) {
        return -1;
    }

    analog_output_config[channel].hdac = hdac;
    analog_output_config[channel].channel = dac_channel;
    analog_output_config[channel].htim = htim;
    analog_output_config[channel].tim_channel = tim_channel;
    analog_output_config[channel].scale_factor = scale_factor;
    analog_output_config[channel].offset = offset;
    analog_output_config[channel].enabled = (hdac != NULL || htim != NULL) ? 1 : 0;

    return 0;
}

// ====================================================================
// 调试功能实现
// ====================================================================

#if APP_IO_DEBUG_ENABLE

/**
 * @brief 调试输出IO状态
 */
void App_IO_Debug_Print(void)
{
    debug_counter++;
    if(debug_counter >= debug_period_ms) {  // 根据设定周期输出
        debug_counter = 0;

        printf("\r\n=== EtherCAT IO Status Debug ===\r\n");
        printf("Timestamp: %lu ms\r\n", HAL_GetTick());

        // 数字IO状态
        printf("Digital Inputs:  0x%04X (", Obj0x6001.digital_inputs);
        for(int i = 0; i < 8; i++) {
            printf("%d", (Obj0x6001.digital_inputs & (1 << i)) ? 1 : 0);
        }
        printf(")\r\n");

        printf("Digital Outputs: 0x%04X (", Obj0x7011.digital_outputs);
        for(int i = 0; i < 8; i++) {
            printf("%d", (Obj0x7011.digital_outputs & (1 << i)) ? 1 : 0);
        }
        printf(")\r\n");

        // 模拟量状态
        printf("Analog Inputs:  ");
        for(int i = 0; i < 4; i++) {  // 只显示前4个通道
            printf("AI%d=%d ", i, Obj0x6002.channel[i]);
        }
        printf("\r\n");

        printf("Analog Outputs: ");
        for(int i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
            printf("AO%d=%d ", i, Obj0x7012.channel[i]);
        }
        printf("\r\n");

        // 兼容性对象状态
        printf("Legacy Objects: Switch1=%d, Switch2=%d, Led1=%d, Led2=%d\r\n",
               Obj0x6000.Switch1, Obj0x6000.Switch2, Obj0x7010.Led1, Obj0x7010.Led2);

        // 统计信息
        printf("Statistics: DI_Changes=%lu, AI_Samples=%lu, Errors=%lu\r\n",
               io_stats.digital_input_changes, io_stats.analog_input_samples,
               io_stats.analog_conversion_errors);

        // EtherCAT状态
        extern UINT8 nAlStatus;  // 来自ecatslv.c
        const char* state_str;
        switch(nAlStatus & 0x0F) {
            case 0x01: state_str = "INIT"; break;
            case 0x02: state_str = "PREOP"; break;
            case 0x04: state_str = "SAFEOP"; break;
            case 0x08: state_str = "OP"; break;
            default:   state_str = "UNKNOWN"; break;
        }
        printf("EtherCAT State: %s (0x%02X)\r\n", state_str, nAlStatus);
        printf("================================\r\n");
    }
}

/**
 * @brief 设置调试输出周期
 */
void App_IO_Set_Debug_Period(uint32_t period_ms)
{
    debug_period_ms = period_ms;
    debug_counter = 0;  // 重置计数器
}

#endif

// ====================================================================
// 私有函数实现
// ====================================================================

/**
 * @brief 更新兼容性对象
 */
static void update_compatibility_objects(void)
{
    // 保持原有对象的兼容性
    Obj0x6000.Switch1 = (Obj0x6001.digital_inputs & 0x01) ? 1 : 0;  // DI0
    Obj0x6000.Switch2 = (Obj0x6001.digital_inputs & 0x02) ? 1 : 0;  // DI1

    Obj0x7010.Led1 = (Obj0x7011.digital_outputs & 0x01) ? 1 : 0;    // DO0
    Obj0x7010.Led2 = (Obj0x7011.digital_outputs & 0x02) ? 1 : 0;    // DO1
}

/**
 * @brief ADC值转换为标准化值
 */
static int16_t adc_to_standard_value(uint32_t adc_value, const analog_input_config_t* config)
{
    // ADC值范围: 0-4095 (12bit)
    // 标准化值范围: -32768~+32767 (16bit signed)

    // 归一化到 -1.0~+1.0
    float normalized = (adc_value / 4095.0f) * 2.0f - 1.0f;

    // 应用比例因子和偏移
    float scaled = normalized * 32767.0f * config->scale_factor + config->offset;

    // 限幅并转换为整型
    if(scaled > 32767.0f) scaled = 32767.0f;
    if(scaled < -32768.0f) scaled = -32768.0f;

    return (int16_t)scaled;
}

/**
 * @brief 标准化值转换为DAC值
 */
static uint32_t standard_value_to_dac(int16_t value, const analog_output_config_t* config)
{
    // 标准化值范围: -32768~+32767
    // DAC值范围: 0-4095 (12bit)

    // 应用偏移和比例因子
    float adjusted = (value - config->offset) / (32767.0f * config->scale_factor);

    // 归一化到 0~1.0
    float normalized = (adjusted + 1.0f) / 2.0f;

    // 限幅
    if(normalized < 0.0f) normalized = 0.0f;
    if(normalized > 1.0f) normalized = 1.0f;

    return (uint32_t)(normalized * 4095.0f);
}

/**
 * @brief 标准化值转换为PWM值
 */
static uint32_t standard_value_to_pwm(int16_t value, const analog_output_config_t* config)
{
    // 假设PWM定时器ARR=1000 (可根据实际配置调整)
    const uint32_t PWM_MAX_VALUE = 1000;

    // 应用偏移和比例因子
    float adjusted = (value - config->offset) / (32767.0f * config->scale_factor);

    // 归一化到 0~1.0
    float normalized = (adjusted + 1.0f) / 2.0f;

    // 限幅
    if(normalized < 0.0f) normalized = 0.0f;
    if(normalized > 1.0f) normalized = 1.0f;

    return (uint32_t)(normalized * PWM_MAX_VALUE);
}