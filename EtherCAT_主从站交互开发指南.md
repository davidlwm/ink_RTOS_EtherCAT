# EtherCAT 主从站交互开发指南

## 📋 目录
- [1. 项目概述](#1-项目概述)
- [2. 当前PDO配置分析](#2-当前pdo配置分析)
- [3. 主从站数据交互原理](#3-主从站数据交互原理)
- [4. 现有代码分析](#4-现有代码分析)
- [5. 扩展开关量和模拟量交互](#5-扩展开关量和模拟量交互)
- [6. 应用层交互处理代码](#6-应用层交互处理代码)
- [7. 上位机配置示例](#7-上位机配置示例)
- [8. 调试和验证方法](#8-调试和验证方法)

---

## 1. 项目概述

### 1.1 硬件架构
```
┌──────────────┐    SPI(10.5MHz)    ┌─────────────┐    EtherCAT    ┌─────────────┐
│  STM32F407   │ ◄─────────────────► │   LAN9252   │ ◄─────────────► │ EtherCAT主站 │
│   (应用层)    │                    │   (ESC)     │                │   (PLC/PC)   │
└──────────────┘                    └─────────────┘                └─────────────┘
       ▲                                   ▲
       │                                   │
   ┌───▼───┐                          ┌───▼───┐
   │ GPIO  │                          │ DPRAM │
   │传感器/│                          │ 16KB  │
   │执行器 │                          │       │
   └───────┘                          └───────┘
```

### 1.2 软件架构
```
┌─────────────────────────────────────────────────────────────┐
│                    应用层 (SSC-Device.c)                    │
├─────────────────────────────────────────────────────────────┤
│              EtherCAT协议栈 (Beckhoff SSC v5.12)            │
├─────────────────────────────────────────────────────────────┤
│           硬件抽象层 (el9800hw.c) - SPI通信                 │
├─────────────────────────────────────────────────────────────┤
│              板级支持包 (BSP) - STM32 HAL                   │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. 当前PDO配置分析

### 2.1 对象字典结构
根据`SSC-DeviceObjects.h`分析，当前配置包含：

#### **输入数据 (TxPDO) - 从站→主站**
```c
// Object 0x6000: 数字输入
typedef struct {
    UINT16 u16SubIndex0;    // 子索引计数
    BOOLEAN Switch1;        // 开关1状态 (1bit)
    BOOLEAN Switch2;        // 开关2状态 (1bit)
} TOBJ6000;

// Object 0x1A00: TxPDO映射
// 映射到 0x6000.1 (Switch1) 和 0x6000.2 (Switch2)
```

#### **输出数据 (RxPDO) - 主站→从站**
```c
// Object 0x7010: 数字输出
typedef struct {
    UINT16 u16SubIndex0;    // 子索引计数
    BOOLEAN Led1;           // LED1控制 (1bit)
    BOOLEAN Led2;           // LED2控制 (1bit)
} TOBJ7010;

// Object 0x1601: RxPDO映射
// 映射到 0x7010.1 (Led1) 和 0x7010.2 (Led2)
```

### 2.2 同步管理器配置
```c
// Object 0x1C12: SM2分配 (接收PDO)
TOBJ1C12 sRxPDOassign = {1, {0x1601}};

// Object 0x1C13: SM3分配 (发送PDO)
TOBJ1C13 sTxPDOassign = {1, {0x1A00}};
```

---

## 3. 主从站数据交互原理

### 3.1 PDO数据流向
```
主站(Master)                           从站(Slave)
     │                                      │
     ▼ 输出数据(RxPDO)                       ▼
┌─────────┐ ──────── EtherCAT ──────── ┌─────────┐
│ 控制命令 │                           │ LED控制  │
│ Led1=1  │ ────────────────────────► │ Obj0x7010│ ──► GPIO输出
│ Led2=0  │                           │         │
└─────────┘                           └─────────┘
     ▲                                      ▲
     │ 输入数据(TxPDO)                       │
┌─────────┐ ◄─────── EtherCAT ◄─────── ┌─────────┐
│ 状态反馈 │                           │ 开关状态 │
│Switch1=1│ ◄────────────────────────  │ Obj0x6000│ ◄── GPIO输入
│Switch2=0│                           │         │
└─────────┘                           └─────────┘
```

### 3.2 数据交换时序
```
主站发送周期 (典型1ms):
┌─┐   ┌─┐   ┌─┐   ┌─┐
│1│   │2│   │3│   │4│  ...
└─┘   └─┘   └─┘   └─┘
 │     │     │     │
 ▼     ▼     ▼     ▼
输出   输入   输出   输入
数据   数据   数据   数据
```

---

## 4. 现有代码分析

### 4.1 输入数据处理 (`APPL_InputMapping`)
**位置:** `Src/SSC-Device.c:266`

```c
void APPL_InputMapping(UINT16* pData)
{
    UINT16 j = 0;
    UINT16 *pTmpData = (UINT16 *)pData;

    /* 遍历所有TxPDO分配对象 */
    for (j = 0; j < sTxPDOassign.u16SubIndex0; j++)
    {
        switch (sTxPDOassign.aEntries[j])
        {
        case 0x1A00:  // TxPDO 1
            // 将对象0x6000数据复制到ESC内存
            *pTmpData++ = SWAPWORD(((UINT16 *) &Obj0x6000)[1]);
            break;
        }
    }
}
```

### 4.2 输出数据处理 (`APPL_OutputMapping`)
**位置:** `Src/SSC-Device.c:292`

```c
void APPL_OutputMapping(UINT16* pData)
{
    UINT16 j = 0;
    UINT16 *pTmpData = (UINT16 *)pData;

    /* 遍历所有RxPDO分配对象 */
    for (j = 0; j < sRxPDOassign.u16SubIndex0; j++)
    {
        switch (sRxPDOassign.aEntries[j])
        {
        case 0x1601:  // RxPDO 1
            // 从ESC内存复制数据到对象0x7010
            ((UINT16 *) &Obj0x7010)[1] = SWAPWORD(*pTmpData++);
            break;
        }
    }
}
```

### 4.3 应用程序主循环 (`APPL_Application`)
**位置:** `Src/SSC-Device.c:315`

```c
void APPL_Application(void)
{
    // 🔴 输出处理: 根据从主站接收的数据控制LED
    if(Obj0x7010.Led1) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
    }

    if(Obj0x7010.Led2) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    }

    // 🔵 输入处理: 读取GPIO状态并更新到对象
    Obj0x6000.Switch1 = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2);
    Obj0x6000.Switch2 = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3);
}
```

### 4.4 当前GPIO配置
```c
// 输出引脚 (LED控制)
GPIOB, GPIO_PIN_11  →  Led1
GPIOB, GPIO_PIN_12  →  Led2

// 输入引脚 (开关检测)
GPIOE, GPIO_PIN_2   →  Switch1
GPIOE, GPIO_PIN_3   →  Switch2
```

---

## 5. 扩展开关量和模拟量交互

### 5.1 问题分析
当前实现存在以下限制：
1. **数据类型单一**: 只支持布尔型开关量
2. **数量有限**: 仅2个输入+2个输出
3. **无模拟量**: 不支持ADC/DAC数据
4. **缺乏扩展性**: 硬编码GPIO引脚

### 5.2 扩展方案设计

#### **5.2.1 新增对象字典条目**
```c
// 扩展数字输入 (16通道)
Object 0x6001: Digital Inputs Extended
├── SubIndex 0: UINT8 (通道数量)
├── SubIndex 1: UINT16 (通道1-16状态位图)
└── SubIndex 2: UINT16 (通道17-32状态位图，预留)

// 扩展数字输出 (16通道)
Object 0x7011: Digital Outputs Extended
├── SubIndex 0: UINT8 (通道数量)
├── SubIndex 1: UINT16 (通道1-16控制位图)
└── SubIndex 2: UINT16 (通道17-32控制位图，预留)

// 模拟输入 (8通道)
Object 0x6002: Analog Inputs
├── SubIndex 0: UINT8 (通道数量)
├── SubIndex 1: INT16 (ADC通道1值, -32768~32767)
├── SubIndex 2: INT16 (ADC通道2值)
├── ...
└── SubIndex 8: INT16 (ADC通道8值)

// 模拟输出 (4通道)
Object 0x7012: Analog Outputs
├── SubIndex 0: UINT8 (通道数量)
├── SubIndex 1: INT16 (DAC通道1值, -32768~32767)
├── SubIndex 2: INT16 (DAC通道2值)
├── SubIndex 3: INT16 (DAC通道3值)
└── SubIndex 4: INT16 (DAC通道4值)
```

#### **5.2.2 PDO映射扩展**
```c
// 新的TxPDO映射 (从站→主站)
Object 0x1A01: TxPDO Mapping 2
├── SubIndex 1: 0x60010110 (数字输入扩展，16bit)
├── SubIndex 2: 0x60020110 (模拟输入CH1，16bit)
├── SubIndex 3: 0x60020210 (模拟输入CH2，16bit)
└── SubIndex 4: 0x60020310 (模拟输入CH3，16bit)

// 新的RxPDO映射 (主站→从站)
Object 0x1602: RxPDO Mapping 2
├── SubIndex 1: 0x70110110 (数字输出扩展，16bit)
├── SubIndex 2: 0x70120110 (模拟输出CH1，16bit)
└── SubIndex 3: 0x70120210 (模拟输出CH2，16bit)
```

---

## 6. 应用层交互处理代码

以下是完整的扩展应用层代码实现：

### 6.1 数据结构定义

```c
// 📁 Inc/app_io_handler.h
#ifndef __APP_IO_HANDLER_H
#define __APP_IO_HANDLER_H

#include "stm32f4xx_hal.h"
#include "ecat_def.h"

// 配置参数
#define MAX_DIGITAL_INPUTS    16    // 最大数字输入通道
#define MAX_DIGITAL_OUTPUTS   16    // 最大数字输出通道
#define MAX_ANALOG_INPUTS     8     // 最大模拟输入通道
#define MAX_ANALOG_OUTPUTS    4     // 最大模拟输出通道

// 数字IO引脚映射结构
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint8_t active_level;  // 0=低电平有效, 1=高电平有效
} digital_pin_config_t;

// 模拟输入配置结构
typedef struct {
    ADC_HandleTypeDef* hadc;
    uint32_t channel;
    float scale_factor;     // 比例因子
    int16_t offset;         // 偏移量
} analog_input_config_t;

// 模拟输出配置结构
typedef struct {
    DAC_HandleTypeDef* hdac;
    uint32_t channel;
    float scale_factor;     // 比例因子
    int16_t offset;         // 偏移量
} analog_output_config_t;

// 扩展对象结构
typedef struct {
    UINT16 u16SubIndex0;
    UINT16 digital_inputs;      // 16位数字输入状态
    UINT16 reserved;            // 预留扩展
} TOBJ6001_DigitalInputsExt;

typedef struct {
    UINT16 u16SubIndex0;
    UINT16 digital_outputs;     // 16位数字输出控制
    UINT16 reserved;            // 预留扩展
} TOBJ7011_DigitalOutputsExt;

typedef struct {
    UINT16 u16SubIndex0;
    INT16 channel[MAX_ANALOG_INPUTS];  // 8通道模拟输入值
} TOBJ6002_AnalogInputs;

typedef struct {
    UINT16 u16SubIndex0;
    INT16 channel[MAX_ANALOG_OUTPUTS]; // 4通道模拟输出值
} TOBJ7012_AnalogOutputs;

// 全局变量声明
extern TOBJ6001_DigitalInputsExt Obj0x6001;
extern TOBJ7011_DigitalOutputsExt Obj0x7011;
extern TOBJ6002_AnalogInputs Obj0x6002;
extern TOBJ7012_AnalogOutputs Obj0x7012;

// 函数声明
void App_IO_Init(void);
void App_Digital_Input_Process(void);
void App_Digital_Output_Process(void);
void App_Analog_Input_Process(void);
void App_Analog_Output_Process(void);
void App_IO_Handler(void);

#endif /* __APP_IO_HANDLER_H */
```

### 6.2 IO处理实现

```c
// 📁 Src/app_io_handler.c
#include "app_io_handler.h"
#include "main.h"

// ====================================================================
// 全局变量定义
// ====================================================================
TOBJ6001_DigitalInputsExt Obj0x6001 = {2, 0, 0};
TOBJ7011_DigitalOutputsExt Obj0x7011 = {2, 0, 0};
TOBJ6002_AnalogInputs Obj0x6002 = {8, {0}};
TOBJ7012_AnalogOutputs Obj0x7012 = {4, {0}};

// ====================================================================
// 硬件配置表
// ====================================================================

// 数字输入引脚配置 (可根据实际硬件修改)
static const digital_pin_config_t digital_input_pins[MAX_DIGITAL_INPUTS] = {
    {GPIOE, GPIO_PIN_2,  1},   // DI0  - Switch1 (原有)
    {GPIOE, GPIO_PIN_3,  1},   // DI1  - Switch2 (原有)
    {GPIOE, GPIO_PIN_4,  1},   // DI2  - 扩展输入
    {GPIOE, GPIO_PIN_5,  1},   // DI3  - 扩展输入
    {GPIOE, GPIO_PIN_6,  1},   // DI4  - 扩展输入
    {GPIOE, GPIO_PIN_7,  1},   // DI5  - 扩展输入
    {GPIOE, GPIO_PIN_8,  1},   // DI6  - 扩展输入
    {GPIOE, GPIO_PIN_9,  1},   // DI7  - 扩展输入
    {GPIOE, GPIO_PIN_10, 1},   // DI8  - 扩展输入
    {GPIOE, GPIO_PIN_11, 1},   // DI9  - 扩展输入
    {GPIOE, GPIO_PIN_12, 1},   // DI10 - 扩展输入
    {GPIOE, GPIO_PIN_13, 1},   // DI11 - 扩展输入
    {GPIOE, GPIO_PIN_14, 1},   // DI12 - 扩展输入
    {GPIOE, GPIO_PIN_15, 1},   // DI13 - 扩展输入
    {GPIOD, GPIO_PIN_8,  1},   // DI14 - 扩展输入
    {GPIOD, GPIO_PIN_9,  1},   // DI15 - 扩展输入
};

// 数字输出引脚配置 (可根据实际硬件修改)
static const digital_pin_config_t digital_output_pins[MAX_DIGITAL_OUTPUTS] = {
    {GPIOB, GPIO_PIN_11, 1},   // DO0  - Led1 (原有)
    {GPIOB, GPIO_PIN_12, 1},   // DO1  - Led2 (原有)
    {GPIOB, GPIO_PIN_13, 1},   // DO2  - 扩展输出
    {GPIOB, GPIO_PIN_14, 1},   // DO3  - 扩展输出
    {GPIOB, GPIO_PIN_15, 1},   // DO4  - 扩展输出
    {GPIOD, GPIO_PIN_10, 1},   // DO5  - 扩展输出
    {GPIOD, GPIO_PIN_11, 1},   // DO6  - 扩展输出
    {GPIOD, GPIO_PIN_12, 1},   // DO7  - 扩展输出
    {GPIOD, GPIO_PIN_13, 1},   // DO8  - 扩展输出
    {GPIOD, GPIO_PIN_14, 1},   // DO9  - 扩展输出
    {GPIOD, GPIO_PIN_15, 1},   // DO10 - 扩展输出
    {GPIOC, GPIO_PIN_6,  1},   // DO11 - 扩展输出
    {GPIOC, GPIO_PIN_7,  1},   // DO12 - 扩展输出
    {GPIOC, GPIO_PIN_8,  1},   // DO13 - 扩展输出
    {GPIOC, GPIO_PIN_9,  1},   // DO14 - 扩展输出
    {GPIOC, GPIO_PIN_10, 1},   // DO15 - 扩展输出
};

// 模拟输入配置 (需要根据实际ADC配置修改)
static analog_input_config_t analog_input_config[MAX_ANALOG_INPUTS] = {
    {&hadc1, ADC_CHANNEL_0,  1.0f, 0},    // AI0 - PA0/ADC1_CH0
    {&hadc1, ADC_CHANNEL_1,  1.0f, 0},    // AI1 - PA1/ADC1_CH1
    {&hadc1, ADC_CHANNEL_2,  1.0f, 0},    // AI2 - PA2/ADC1_CH2
    {&hadc1, ADC_CHANNEL_3,  1.0f, 0},    // AI3 - PA3/ADC1_CH3
    {&hadc1, ADC_CHANNEL_4,  1.0f, 0},    // AI4 - PA4/ADC1_CH4
    {&hadc1, ADC_CHANNEL_5,  1.0f, 0},    // AI5 - PA5/ADC1_CH5
    {&hadc1, ADC_CHANNEL_6,  1.0f, 0},    // AI6 - PA6/ADC1_CH6
    {&hadc1, ADC_CHANNEL_7,  1.0f, 0},    // AI7 - PA7/ADC1_CH7
};

// 模拟输出配置 (需要根据实际DAC配置修改)
static analog_output_config_t analog_output_config[MAX_ANALOG_OUTPUTS] = {
    {&hdac, DAC_CHANNEL_1, 1.0f, 0},      // AO0 - PA4/DAC_CH1
    {&hdac, DAC_CHANNEL_2, 1.0f, 0},      // AO1 - PA5/DAC_CH2
    {NULL, 0, 1.0f, 0},                   // AO2 - PWM模拟输出(预留)
    {NULL, 0, 1.0f, 0},                   // AO3 - PWM模拟输出(预留)
};

// ====================================================================
// 功能函数实现
// ====================================================================

/**
 * @brief IO处理模块初始化
 */
void App_IO_Init(void)
{
    // 初始化扩展对象
    Obj0x6001.u16SubIndex0 = 2;
    Obj0x6001.digital_inputs = 0;

    Obj0x7011.u16SubIndex0 = 2;
    Obj0x7011.digital_outputs = 0;

    Obj0x6002.u16SubIndex0 = MAX_ANALOG_INPUTS;
    for(int i = 0; i < MAX_ANALOG_INPUTS; i++) {
        Obj0x6002.channel[i] = 0;
    }

    Obj0x7012.u16SubIndex0 = MAX_ANALOG_OUTPUTS;
    for(int i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
        Obj0x7012.channel[i] = 0;
    }
}

/**
 * @brief 处理数字输入
 * @note 从GPIO读取状态并更新到EtherCAT对象
 */
void App_Digital_Input_Process(void)
{
    uint16_t input_state = 0;

    // 逐位读取数字输入状态
    for(int i = 0; i < MAX_DIGITAL_INPUTS; i++) {
        GPIO_PinState pin_state = HAL_GPIO_ReadPin(
            digital_input_pins[i].port,
            digital_input_pins[i].pin
        );

        // 根据有效电平设置位状态
        if((pin_state == GPIO_PIN_SET && digital_input_pins[i].active_level == 1) ||
           (pin_state == GPIO_PIN_RESET && digital_input_pins[i].active_level == 0)) {
            input_state |= (1 << i);
        }
    }

    // 更新扩展数字输入对象
    Obj0x6001.digital_inputs = input_state;

    // 保持原有对象兼容性
    Obj0x6000.Switch1 = (input_state & 0x01) ? 1 : 0;  // DI0
    Obj0x6000.Switch2 = (input_state & 0x02) ? 1 : 0;  // DI1
}

/**
 * @brief 处理数字输出
 * @note 根据EtherCAT对象值控制GPIO输出
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

        HAL_GPIO_WritePin(digital_output_pins[i].port, digital_output_pins[i].pin, pin_state);
    }

    // 保持原有对象兼容性
    Obj0x7010.Led1 = (output_state & 0x01) ? 1 : 0;    // DO0
    Obj0x7010.Led2 = (output_state & 0x02) ? 1 : 0;    // DO1
}

/**
 * @brief 处理模拟输入
 * @note 从ADC读取值并转换为标准化数据
 */
void App_Analog_Input_Process(void)
{
    for(int i = 0; i < MAX_ANALOG_INPUTS; i++) {
        if(analog_input_config[i].hadc != NULL) {
            // 配置ADC通道
            ADC_ChannelConfTypeDef sConfig = {0};
            sConfig.Channel = analog_input_config[i].channel;
            sConfig.Rank = 1;
            sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;

            if(HAL_ADC_ConfigChannel(analog_input_config[i].hadc, &sConfig) == HAL_OK) {
                // 启动转换
                HAL_ADC_Start(analog_input_config[i].hadc);

                // 等待转换完成
                if(HAL_ADC_PollForConversion(analog_input_config[i].hadc, 100) == HAL_OK) {
                    uint32_t adc_value = HAL_ADC_GetValue(analog_input_config[i].hadc);

                    // 转换为标准化值 (-32768 ~ +32767)
                    // ADC: 0-4095 (12bit) → -32768~+32767 (16bit signed)
                    float normalized = (adc_value / 4095.0f) * 2.0f - 1.0f;  // -1.0~+1.0
                    int16_t scaled_value = (int16_t)(normalized * 32767.0f * analog_input_config[i].scale_factor + analog_input_config[i].offset);

                    Obj0x6002.channel[i] = scaled_value;
                }

                HAL_ADC_Stop(analog_input_config[i].hadc);
            }
        }
    }
}

/**
 * @brief 处理模拟输出
 * @note 根据EtherCAT对象值设置DAC/PWM输出
 */
void App_Analog_Output_Process(void)
{
    for(int i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
        int16_t target_value = Obj0x7012.channel[i];

        if(analog_output_config[i].hdac != NULL) {
            // DAC输出处理
            // 标准化值 (-32768~+32767) → DAC值 (0-4095)
            float normalized = (target_value - analog_output_config[i].offset) / (32767.0f * analog_output_config[i].scale_factor);
            normalized = (normalized + 1.0f) / 2.0f;  // -1.0~+1.0 → 0~1.0

            if(normalized < 0.0f) normalized = 0.0f;
            if(normalized > 1.0f) normalized = 1.0f;

            uint32_t dac_value = (uint32_t)(normalized * 4095.0f);

            HAL_DAC_SetValue(analog_output_config[i].hdac, analog_output_config[i].channel, DAC_ALIGN_12B_R, dac_value);
        } else {
            // PWM输出处理 (可选实现)
            // TODO: 实现PWM模拟输出
        }
    }
}

/**
 * @brief IO处理主函数
 * @note 在APPL_Application()中调用
 */
void App_IO_Handler(void)
{
    // 按顺序处理各种IO
    App_Digital_Input_Process();   // 处理数字输入
    App_Analog_Input_Process();    // 处理模拟输入
    App_Digital_Output_Process();  // 处理数字输出
    App_Analog_Output_Process();   // 处理模拟输出
}
```

### 6.3 修改主应用程序

```c
// 📁 Src/SSC-Device.c (修改现有文件)

#include "app_io_handler.h"  // 添加头文件包含

// 修改 APPL_Application 函数
void APPL_Application(void)
{
    // === 原有的简单IO处理 (保持兼容性) ===
    if(Obj0x7010.Led1) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
    }

    if(Obj0x7010.Led2) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    }

    Obj0x6000.Switch1 = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2);
    Obj0x6000.Switch2 = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3);

    // === 新增扩展IO处理 ===
    App_IO_Handler();  // 处理扩展的数字和模拟IO
}

// 扩展 APPL_InputMapping 函数支持多PDO
void APPL_InputMapping(UINT16* pData)
{
    UINT16 j = 0;
    UINT16 *pTmpData = (UINT16 *)pData;

    /* 遍历所有TxPDO分配对象 */
    for (j = 0; j < sTxPDOassign.u16SubIndex0; j++)
    {
        switch (sTxPDOassign.aEntries[j])
        {
        case 0x1A00:  // 原有TxPDO 1 (兼容性)
            *pTmpData++ = SWAPWORD(((UINT16 *) &Obj0x6000)[1]);
            break;

        case 0x1A01:  // 新增TxPDO 2 (扩展IO)
            // 数字输入扩展 (16bit)
            *pTmpData++ = SWAPWORD(Obj0x6001.digital_inputs);
            // 模拟输入通道1-3 (3x16bit)
            *pTmpData++ = SWAPWORD((UINT16)Obj0x6002.channel[0]);
            *pTmpData++ = SWAPWORD((UINT16)Obj0x6002.channel[1]);
            *pTmpData++ = SWAPWORD((UINT16)Obj0x6002.channel[2]);
            break;
        }
    }
}

// 扩展 APPL_OutputMapping 函数支持多PDO
void APPL_OutputMapping(UINT16* pData)
{
    UINT16 j = 0;
    UINT16 *pTmpData = (UINT16 *)pData;

    /* 遍历所有RxPDO分配对象 */
    for (j = 0; j < sRxPDOassign.u16SubIndex0; j++)
    {
        switch (sRxPDOassign.aEntries[j])
        {
        case 0x1601:  // 原有RxPDO 1 (兼容性)
            ((UINT16 *) &Obj0x7010)[1] = SWAPWORD(*pTmpData++);
            break;

        case 0x1602:  // 新增RxPDO 2 (扩展IO)
            // 数字输出扩展 (16bit)
            Obj0x7011.digital_outputs = SWAPWORD(*pTmpData++);
            // 模拟输出通道1-2 (2x16bit)
            Obj0x7012.channel[0] = (INT16)SWAPWORD(*pTmpData++);
            Obj0x7012.channel[1] = (INT16)SWAPWORD(*pTmpData++);
            break;
        }
    }
}
```

### 6.4 硬件初始化配置

```c
// 📁 Src/main.c (添加到main函数中)

int main(void)
{
    /* 系统初始化 */
    HAL_Init();
    SystemClock_Config();

    /* 外设初始化 */
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_DAC_Init();
    // ... 其他初始化

    /* EtherCAT初始化 */
    HW_Init();
    MainInit();

    /* 应用IO初始化 */
    App_IO_Init();  // 🔥 新增扩展IO初始化

    /* 主循环 */
    bRunApplication = TRUE;
    do {
        MainLoop();
    } while (bRunApplication == TRUE);

    HW_Release();
    return 0;
}
```

---

## 7. 上位机配置示例

### 7.1 TwinCAT 3配置

#### **7.1.1 设备配置**
```xml
<!-- ESI文件配置示例 -->
<Slave>
    <Type ProductCode="#x26483052" RevisionNo="#x00010000">SSC-Device</Type>
    <Name>EtherCAT IO Slave</Name>

    <!-- 同步管理器配置 -->
    <Sm No="2" StartAddress="#x1000" ControlByte="#x24" Enable="1">
        <Name>Outputs</Name>
        <Pdo No="0x1601" Name="RxPDO 1"/>
        <Pdo No="0x1602" Name="RxPDO 2 Extended"/>
    </Sm>

    <Sm No="3" StartAddress="#x1800" ControlByte="#x20" Enable="1">
        <Name>Inputs</Name>
        <Pdo No="0x1A00" Name="TxPDO 1"/>
        <Pdo No="0x1A01" Name="TxPDO 2 Extended"/>
    </Sm>

    <!-- PDO映射定义 -->
    <RxPdo No="0x1601" Name="Digital Outputs Basic">
        <Entry No="0x7010" Sub="1" BitLen="1" Name="Led1"/>
        <Entry No="0x7010" Sub="2" BitLen="1" Name="Led2"/>
    </RxPdo>

    <RxPdo No="0x1602" Name="Extended Outputs">
        <Entry No="0x7011" Sub="1" BitLen="16" Name="Digital Outputs"/>
        <Entry No="0x7012" Sub="1" BitLen="16" Name="Analog Output 1"/>
        <Entry No="0x7012" Sub="2" BitLen="16" Name="Analog Output 2"/>
    </RxPdo>

    <TxPdo No="0x1A00" Name="Digital Inputs Basic">
        <Entry No="0x6000" Sub="1" BitLen="1" Name="Switch1"/>
        <Entry No="0x6000" Sub="2" BitLen="1" Name="Switch2"/>
    </TxPdo>

    <TxPdo No="0x1A01" Name="Extended Inputs">
        <Entry No="0x6001" Sub="1" BitLen="16" Name="Digital Inputs"/>
        <Entry No="0x6002" Sub="1" BitLen="16" Name="Analog Input 1"/>
        <Entry No="0x6002" Sub="2" BitLen="16" Name="Analog Input 2"/>
        <Entry No="0x6002" Sub="3" BitLen="16" Name="Analog Input 3"/>
    </TxPdo>
</Slave>
```

#### **7.1.2 PLC程序示例**
```iecst
// TwinCAT PLC程序 (Structured Text)
PROGRAM EtherCAT_IO_Test
VAR
    // 基础IO变量
    bLed1_Control      : BOOL;
    bLed2_Control      : BOOL;
    bSwitch1_Status    : BOOL;
    bSwitch2_Status    : BOOL;

    // 扩展IO变量
    wDigital_Outputs   : WORD;    // 16位数字输出
    wDigital_Inputs    : WORD;    // 16位数字输入
    iAnalog_Output1    : INT;     // 模拟输出1 (-32768~32767)
    iAnalog_Output2    : INT;     // 模拟输出2
    iAnalog_Input1     : INT;     // 模拟输入1
    iAnalog_Input2     : INT;     // 模拟输入2
    iAnalog_Input3     : INT;     // 模拟输入3

    // 测试变量
    bTest_Running      : BOOL;
    iTest_Counter      : INT;
END_VAR

// 基础IO测试
bLed1_Control := bSwitch1_Status;  // Led1跟随Switch1
bLed2_Control := bSwitch2_Status;  // Led2跟随Switch2

// 扩展数字IO测试
wDigital_Outputs.0 := wDigital_Inputs.0;  // DO0跟随DI0
wDigital_Outputs.1 := wDigital_Inputs.1;  // DO1跟随DI1

// 流水灯测试
IF bTest_Running THEN
    iTest_Counter := iTest_Counter + 1;
    IF iTest_Counter >= 100 THEN  // 100ms周期
        iTest_Counter := 0;
        wDigital_Outputs := ROL(wDigital_Outputs, 1);  // 循环左移
    END_IF
END_IF

// 模拟量测试 - 简单的比例控制
iAnalog_Output1 := iAnalog_Input1 / 2;     // 输出为输入的一半
iAnalog_Output2 := 32767 - iAnalog_Input2; // 输出为输入的反向

// 链接到EtherCAT IO映射
IO_Mapping_Outputs.Led1 := bLed1_Control;
IO_Mapping_Outputs.Led2 := bLed2_Control;
IO_Mapping_Outputs.Digital_Outputs := wDigital_Outputs;
IO_Mapping_Outputs.Analog_Output_1 := iAnalog_Output1;
IO_Mapping_Outputs.Analog_Output_2 := iAnalog_Output2;

bSwitch1_Status := IO_Mapping_Inputs.Switch1;
bSwitch2_Status := IO_Mapping_Inputs.Switch2;
wDigital_Inputs := IO_Mapping_Inputs.Digital_Inputs;
iAnalog_Input1 := IO_Mapping_Inputs.Analog_Input_1;
iAnalog_Input2 := IO_Mapping_Inputs.Analog_Input_2;
iAnalog_Input3 := IO_Mapping_Inputs.Analog_Input_3;
```

### 7.2 CODESYS配置示例

```iecst
// CODESYS PLC程序示例
PROGRAM EtherCAT_Control
VAR
    // EtherCAT从站输出映射
    xLED1_Out      AT %QX0.0 : BOOL;    // LED1控制
    xLED2_Out      AT %QX0.1 : BOOL;    // LED2控制
    wDigOut_Ext    AT %QW1   : WORD;    // 扩展数字输出
    iAnalogOut1    AT %QW2   : INT;     // 模拟输出1
    iAnalogOut2    AT %QW3   : INT;     // 模拟输出2

    // EtherCAT从站输入映射
    xSwitch1_In    AT %IX0.0 : BOOL;    // 开关1状态
    xSwitch2_In    AT %IX0.1 : BOOL;    // 开关2状态
    wDigIn_Ext     AT %IW1   : WORD;    // 扩展数字输入
    iAnalogIn1     AT %IW2   : INT;     // 模拟输入1
    iAnalogIn2     AT %IW3   : INT;     // 模拟输入2
    iAnalogIn3     AT %IW4   : INT;     // 模拟输入3

    // 本地变量
    bFlashEnable   : BOOL;
    tonFlash       : TON;
END_VAR

// 简单的控制逻辑
xLED1_Out := xSwitch1_In;  // LED1跟随开关1
xLED2_Out := xSwitch2_In;  // LED2跟随开关2

// 扩展IO控制 - 位操作示例
wDigOut_Ext.0 := wDigIn_Ext.15;  // 输出位0跟随输入位15
wDigOut_Ext.1 := wDigIn_Ext.14;  // 输出位1跟随输入位14

// 模拟量控制示例
iAnalogOut1 := iAnalogIn1;           // 直接复制
iAnalogOut2 := LIMIT(-32768, iAnalogIn2 * 2, 32767);  // 放大2倍限幅

// 闪烁控制
tonFlash(IN := TRUE, PT := T#500MS);
IF tonFlash.Q THEN
    tonFlash(IN := FALSE);
    bFlashEnable := NOT bFlashEnable;
    wDigOut_Ext.2 := bFlashEnable;  // 输出位2闪烁
END_IF
```

---

## 8. 调试和验证方法

### 8.1 硬件测试清单

#### **8.1.1 基础连通性测试**
- [ ] EtherCAT网络连接正常
- [ ] 从站能够进入OP状态
- [ ] 基础LED控制功能正常
- [ ] 基础开关检测功能正常

#### **8.1.2 扩展IO测试**
- [ ] 16路数字输入检测正确
- [ ] 16路数字输出控制正确
- [ ] 模拟输入读取精度满足要求
- [ ] 模拟输出控制精度满足要求

### 8.2 软件调试方法

#### **8.2.1 使用TwinCAT System Manager**
```bash
# 在线监控EtherCAT从站状态
1. 打开TwinCAT System Manager
2. 扫描EtherCAT网络
3. 查看从站状态 (INIT→PREOP→SAFEOP→OP)
4. 在线监控PDO数据变化
5. 使用Process Data在线调试功能
```

#### **8.2.2 使用Wireshark抓包分析**
```bash
# EtherCAT协议分析
1. 安装EtherCAT插件for Wireshark
2. 抓取EtherCAT网络数据包
3. 分析PDO数据格式和周期
4. 检查错误帧和重传
```

#### **8.2.3 使用串口调试输出**
```c
// 在app_io_handler.c中添加调试输出
void App_IO_Debug_Print(void)
{
    static uint32_t debug_counter = 0;

    debug_counter++;
    if(debug_counter >= 1000) {  // 每1000次输出一次
        debug_counter = 0;

        printf("=== EtherCAT IO Status ===\r\n");
        printf("Digital Inputs:  0x%04X\r\n", Obj0x6001.digital_inputs);
        printf("Digital Outputs: 0x%04X\r\n", Obj0x7011.digital_outputs);
        printf("Analog In[0-2]:  %d, %d, %d\r\n",
               Obj0x6002.channel[0], Obj0x6002.channel[1], Obj0x6002.channel[2]);
        printf("Analog Out[0-1]: %d, %d\r\n",
               Obj0x7012.channel[0], Obj0x7012.channel[1]);
        printf("EtherCAT State: %s\r\n",
               (nAlStatus == STATE_OP) ? "OP" : "NOT_OP");
    }
}

// 在App_IO_Handler()中调用
void App_IO_Handler(void)
{
    App_Digital_Input_Process();
    App_Analog_Input_Process();
    App_Digital_Output_Process();
    App_Analog_Output_Process();

#ifdef DEBUG_ENABLE
    App_IO_Debug_Print();  // 调试输出
#endif
}
```

### 8.3 常见问题排查

#### **8.3.1 从站无法进入OP状态**
```
检查项目：
1. ESC连接和SPI通信是否正常
2. PDO配置是否与主站匹配
3. 同步管理器参数是否正确
4. 看门狗超时设置是否合理
```

#### **8.3.2 数据交换异常**
```
检查项目：
1. 字节序是否正确 (SWAPWORD宏使用)
2. 数据类型映射是否匹配
3. PDO映射配置是否正确
4. 内存对齐问题
```

#### **8.3.3 模拟量精度问题**
```
检查项目：
1. ADC/DAC参考电压设置
2. 模拟量校准和标定
3. 滤波算法实现
4. 数据转换公式正确性
```

---

## 📝 总结

本文档详细介绍了基于STM32F407和LAN9252的EtherCAT从站项目中主从站数据交互的实现方法。

### 🎯 **已实现功能**
- ✅ 基础数字IO交互 (2输入+2输出)
- ✅ 完整的EtherCAT协议栈支持
- ✅ 标准PDO映射机制

### 🚀 **扩展功能**
- ✅ 16路数字输入/输出扩展
- ✅ 8路模拟输入支持 (ADC)
- ✅ 4路模拟输出支持 (DAC/PWM)
- ✅ 模块化IO处理架构
- ✅ 灵活的硬件配置表

### 🔧 **开发要点**
1. **保持兼容性**: 扩展代码不影响原有功能
2. **模块化设计**: 便于维护和扩展
3. **标准化接口**: 符合EtherCAT规范
4. **实时性保证**: 确保确定性通信

通过本方案，您可以实现完整的EtherCAT主从站数据交互，支持开关量和模拟量的双向传输，满足工业自动化应用需求。

---

**📧 技术支持**: 如有问题，请参考EtherCAT官方文档或联系技术支持。