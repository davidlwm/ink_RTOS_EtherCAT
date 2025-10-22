#ifndef __APP_IO_HANDLER_H
#define __APP_IO_HANDLER_H

#include "stm32f4xx_hal.h"
#include "ecat_def.h"

#ifdef __cplusplus
extern "C" {
#endif

// ====================================================================
// 配置参数定义
// ====================================================================
#define MAX_DIGITAL_INPUTS    16    // 最大数字输入通道
#define MAX_DIGITAL_OUTPUTS   16    // 最大数字输出通道
#define MAX_ANALOG_INPUTS     8     // 最大模拟输入通道
#define MAX_ANALOG_OUTPUTS    4     // 最大模拟输出通道

// 调试功能开关
#define APP_IO_DEBUG_ENABLE   1     // 启用调试输出

// ADC采样配置
#define ADC_SAMPLE_TIME      ADC_SAMPLETIME_144CYCLES
#define ADC_TIMEOUT_MS       100

// DAC输出配置
#define DAC_ALIGNMENT        DAC_ALIGN_12B_R

// ====================================================================
// 数据结构定义
// ====================================================================

/**
 * @brief 数字IO引脚配置结构
 */
typedef struct {
    GPIO_TypeDef* port;      // GPIO端口
    uint16_t pin;            // GPIO引脚
    uint8_t active_level;    // 有效电平 (0=低电平有效, 1=高电平有效)
    uint8_t pull_mode;       // 上下拉配置 (0=无, 1=上拉, 2=下拉)
} digital_pin_config_t;

/**
 * @brief 模拟输入配置结构
 */
typedef struct {
    ADC_HandleTypeDef* hadc; // ADC句柄
    uint32_t channel;        // ADC通道
    float scale_factor;      // 比例因子 (工程单位转换)
    int16_t offset;          // 偏移量
    uint8_t enabled;         // 使能标志
} analog_input_config_t;

/**
 * @brief 模拟输出配置结构
 */
typedef struct {
    DAC_HandleTypeDef* hdac; // DAC句柄 (NULL表示使用PWM)
    uint32_t channel;        // DAC通道 或 PWM通道
    TIM_HandleTypeDef* htim; // PWM定时器句柄 (DAC为NULL时使用)
    uint32_t tim_channel;    // PWM定时器通道
    float scale_factor;      // 比例因子
    int16_t offset;          // 偏移量
    uint8_t enabled;         // 使能标志
} analog_output_config_t;

/**
 * @brief 扩展数字输入对象 (Object 0x6001)
 */
typedef struct {
    UINT16 u16SubIndex0;        // 子索引计数
    UINT16 digital_inputs;      // 16位数字输入状态
    UINT16 reserved;            // 预留扩展
} TOBJ6001_DigitalInputsExt;

/**
 * @brief 扩展数字输出对象 (Object 0x7011)
 */
typedef struct {
    UINT16 u16SubIndex0;        // 子索引计数
    UINT16 digital_outputs;     // 16位数字输出控制
    UINT16 reserved;            // 预留扩展
} TOBJ7011_DigitalOutputsExt;

/**
 * @brief 模拟输入对象 (Object 0x6002)
 */
typedef struct {
    UINT16 u16SubIndex0;                    // 子索引计数
    INT16 channel[MAX_ANALOG_INPUTS];       // 8通道模拟输入值
} TOBJ6002_AnalogInputs;

/**
 * @brief 模拟输出对象 (Object 0x7012)
 */
typedef struct {
    UINT16 u16SubIndex0;                    // 子索引计数
    INT16 channel[MAX_ANALOG_OUTPUTS];      // 4通道模拟输出值
} TOBJ7012_AnalogOutputs;

/**
 * @brief IO状态统计结构
 */
typedef struct {
    uint32_t digital_input_changes;         // 数字输入变化次数
    uint32_t analog_input_samples;          // 模拟输入采样次数
    uint32_t analog_conversion_errors;      // 模拟转换错误次数
    uint32_t last_update_timestamp;         // 最后更新时间戳
} io_statistics_t;

// ====================================================================
// 全局变量声明
// ====================================================================
extern TOBJ6001_DigitalInputsExt Obj0x6001;
extern TOBJ7011_DigitalOutputsExt Obj0x7011;
extern TOBJ6002_AnalogInputs Obj0x6002;
extern TOBJ7012_AnalogOutputs Obj0x7012;

// 外部ADC/DAC句柄声明 (需要在main.c中定义)
extern ADC_HandleTypeDef hadc1;
extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim3;  // PWM定时器

// ====================================================================
// 函数声明
// ====================================================================

/**
 * @brief IO处理模块初始化
 * @note 必须在EtherCAT初始化后调用
 */
void App_IO_Init(void);

/**
 * @brief 数字输入处理
 * @note 从GPIO读取状态并更新到EtherCAT对象
 */
void App_Digital_Input_Process(void);

/**
 * @brief 数字输出处理
 * @note 根据EtherCAT对象值控制GPIO输出
 */
void App_Digital_Output_Process(void);

/**
 * @brief 模拟输入处理
 * @note 从ADC读取值并转换为标准化数据
 */
void App_Analog_Input_Process(void);

/**
 * @brief 模拟输出处理
 * @note 根据EtherCAT对象值设置DAC/PWM输出
 */
void App_Analog_Output_Process(void);

/**
 * @brief IO处理主函数
 * @note 在APPL_Application()中调用，处理所有IO
 */
void App_IO_Handler(void);

/**
 * @brief 获取IO统计信息
 * @param stats 统计信息结构指针
 */
void App_IO_GetStatistics(io_statistics_t* stats);

/**
 * @brief 设置数字输出通道
 * @param channel 通道号 (0-15)
 * @param state 输出状态 (0或1)
 * @return 0=成功, -1=失败
 */
int App_Set_Digital_Output(uint8_t channel, uint8_t state);

/**
 * @brief 读取数字输入通道
 * @param channel 通道号 (0-15)
 * @return 输入状态 (0或1), -1=错误
 */
int App_Get_Digital_Input(uint8_t channel);

/**
 * @brief 设置模拟输出通道
 * @param channel 通道号 (0-3)
 * @param value 输出值 (-32768~32767)
 * @return 0=成功, -1=失败
 */
int App_Set_Analog_Output(uint8_t channel, int16_t value);

/**
 * @brief 读取模拟输入通道
 * @param channel 通道号 (0-7)
 * @return 输入值 (-32768~32767), INT16_MIN=错误
 */
int16_t App_Get_Analog_Input(uint8_t channel);

/**
 * @brief 配置数字输入引脚
 * @param channel 通道号
 * @param port GPIO端口
 * @param pin GPIO引脚
 * @param active_level 有效电平
 * @return 0=成功, -1=失败
 */
int App_Config_Digital_Input(uint8_t channel, GPIO_TypeDef* port, uint16_t pin, uint8_t active_level);

/**
 * @brief 配置数字输出引脚
 * @param channel 通道号
 * @param port GPIO端口
 * @param pin GPIO引脚
 * @param active_level 有效电平
 * @return 0=成功, -1=失败
 */
int App_Config_Digital_Output(uint8_t channel, GPIO_TypeDef* port, uint16_t pin, uint8_t active_level);

/**
 * @brief 配置模拟输入通道
 * @param channel 通道号
 * @param hadc ADC句柄
 * @param adc_channel ADC通道
 * @param scale_factor 比例因子
 * @param offset 偏移量
 * @return 0=成功, -1=失败
 */
int App_Config_Analog_Input(uint8_t channel, ADC_HandleTypeDef* hadc, uint32_t adc_channel,
                            float scale_factor, int16_t offset);

/**
 * @brief 配置模拟输出通道
 * @param channel 通道号
 * @param hdac DAC句柄 (NULL表示PWM)
 * @param dac_channel DAC通道
 * @param htim PWM定时器句柄
 * @param tim_channel PWM通道
 * @param scale_factor 比例因子
 * @param offset 偏移量
 * @return 0=成功, -1=失败
 */
int App_Config_Analog_Output(uint8_t channel, DAC_HandleTypeDef* hdac, uint32_t dac_channel,
                             TIM_HandleTypeDef* htim, uint32_t tim_channel,
                             float scale_factor, int16_t offset);

#if APP_IO_DEBUG_ENABLE
/**
 * @brief 调试输出IO状态
 */
void App_IO_Debug_Print(void);

/**
 * @brief 设置调试输出周期
 * @param period_ms 输出周期 (毫秒)
 */
void App_IO_Set_Debug_Period(uint32_t period_ms);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __APP_IO_HANDLER_H */