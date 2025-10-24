# 执行器任务V3 GPIO管脚控制实现

## 概述

根据设计文档V3中的执行器任务设计，完善了 `actuator_task_v3.c` 中缺失的GPIO管脚控制功能。

## 分析结果

### 设计文档V3中的执行器管脚分配
按照设计文档V3第2.3.1节的定义，系统包含9个执行器，每个都有明确的GPIO管脚分配：

| 执行器类型 | GPIO管脚 | 控制方式 | 功能描述 |
|-----------|----------|----------|----------|
| ACTUATOR_VALVE_1 | PE2 | 数字IO | 电磁阀1 (24V) |
| ACTUATOR_VALVE_2 | PE3 | 数字IO | 电磁阀2 (24V) |
| ACTUATOR_HEATER_1 | PE4 | 数字IO | 加热器1 (继电器) |
| ACTUATOR_HEATER_2 | PE5 | 数字IO | 加热器2 (继电器) |
| ACTUATOR_HEATER_3 | PE6 | 数字IO | 加热器3 (继电器) |
| ACTUATOR_PUMP_SPEED_1 | PF9 | PWM | 调速泵1 (TIM14_CH1) |
| ACTUATOR_PUMP_SPEED_2 | PF10 | PWM | 调速泵2 (TIM1_CH3) |
| ACTUATOR_PUMP_DC_1 | PF6 | 数字IO | 直流泵1 |
| ACTUATOR_PUMP_DC_2 | PF7 | 数字IO | 直流泵2 |

### 原始实现问题
原 `actuator_task_v3.c` 实现中发现的问题：
- ✅ 任务架构完整，有良好的FreeRTOS结构
- ❌ `Actuator_SetDigitalOutput` 函数是TODO状态
- ❌ `Actuator_SetPWMOutput` 函数是TODO状态
- ❌ 缺少GPIO管脚定义和硬件初始化

## 实现的功能

### 1. GPIO管脚定义 (`actuator_task_v3.h`)

在头文件中添加了完整的GPIO管脚定义：

```c
// 电磁阀GPIO定义
#define VALVE_1_PORT              GPIOE
#define VALVE_1_PIN               GPIO_PIN_2

// 加热器GPIO定义
#define HEATER_1_PORT             GPIOE
#define HEATER_1_PIN              GPIO_PIN_4

// 调速泵PWM定义
#define PUMP_SPEED_1_PORT         GPIOF
#define PUMP_SPEED_1_PIN          GPIO_PIN_9
#define PUMP_SPEED_1_TIM          TIM14
#define PUMP_SPEED_1_TIM_CHANNEL  TIM_CHANNEL_1

// 直流泵GPIO定义
#define PUMP_DC_1_PORT            GPIOF
#define PUMP_DC_1_PIN             GPIO_PIN_6
```

### 2. 硬件初始化函数 (`Actuator_InitializeHardware`)

实现了完整的硬件初始化功能：

#### 数字输出GPIO初始化
- 配置所有数字输出管脚为推挽输出模式
- 初始状态设置为关闭（GPIO_PIN_RESET）
- 包括：电磁阀(PE2,PE3)、加热器(PE4,PE5,PE6)、直流泵(PF6,PF7)

#### PWM输出初始化
- 配置TIM14和TIM1定时器用于PWM输出
- PWM频率设置为1kHz (84MHz/84/1000)
- 初始占空比为0%
- 管脚复用功能配置：
  - PF9 -> TIM14_CH1 (调速泵1)
  - PF10 -> TIM1_CH3 (调速泵2)

### 3. 数字输出控制 (`Actuator_SetDigitalOutput`)

完整实现了数字输出控制函数：

```c
static BaseType_t Actuator_SetDigitalOutput(uint8_t channel, bool state)
{
    // 根据通道号选择对应的GPIO端口和引脚
    switch (channel) {
        case ACTUATOR_VALVE_1:
            gpio_port = VALVE_1_PORT;
            gpio_pin = VALVE_1_PIN;
            break;
        // ... 其他通道
    }

    // 设置GPIO输出状态
    HAL_GPIO_WritePin(gpio_port, gpio_pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);

    return pdTRUE;
}
```

### 4. PWM输出控制 (`Actuator_SetPWMOutput`)

完整实现了PWM输出控制函数：

```c
static BaseType_t Actuator_SetPWMOutput(uint8_t channel, float duty_cycle)
{
    // 根据通道号选择对应的定时器和通道
    switch (channel) {
        case ACTUATOR_PUMP_SPEED_1:
            htim = &(TIM_HandleTypeDef){.Instance = TIM14};
            tim_channel = TIM_CHANNEL_1;
            break;
        // ...
    }

    // 计算PWM比较值并设置占空比
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(htim);
    uint32_t pulse = (uint32_t)((duty_cycle / 100.0f) * period);
    __HAL_TIM_SET_COMPARE(htim, tim_channel, pulse);

    return pdTRUE;
}
```

### 5. 公共接口函数

添加了公共硬件初始化接口：

```c
BaseType_t ActuatorTaskV3_InitializeHardware(void);
```

## 技术特点

### 安全性
- 所有执行器初始状态为关闭
- 输入参数范围检查和验证
- 故障检测和防抖处理
- 紧急停止功能完整

### 实时性
- 10ms任务周期确保及时响应
- PWM频率1kHz适合调速泵控制
- GPIO操作延迟极低

### 可扩展性
- 模块化设计，易于添加新执行器
- 统一的接口设计
- 完整的错误处理机制

## 使用示例

```c
// 初始化执行器任务
ActuatorTaskV3_Init();
ActuatorTaskV3_Create();

// 控制电磁阀
ActuatorTaskV3_SetValve(0, true);  // 打开电磁阀1

// 控制加热器
ActuatorTaskV3_SetHeater(1, true); // 打开加热器2

// 控制调速泵
ActuatorTaskV3_SetPumpSpeed(0, 75.0f); // 调速泵1设置75%速度

// 控制直流泵
ActuatorTaskV3_SetDCPump(1, true); // 打开直流泵2
```

## 总结

通过此次完善，执行器任务V3现在具备了完整的管脚控制功能，所有设计文档中定义的执行器都能正确控制。实现参考了sensor_task_v3的良好架构，保持了代码的一致性和可维护性。

**核心改进:**
- ✅ 完整的GPIO管脚定义和初始化
- ✅ 实际的数字输出控制功能
- ✅ 实际的PWM输出控制功能
- ✅ 完善的错误处理和状态反馈
- ✅ 符合设计文档V3的管脚分配规范