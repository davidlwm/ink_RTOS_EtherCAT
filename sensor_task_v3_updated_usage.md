# Sensor Task V3 更新说明 - 基于接线图ADC-spi3.png

## 概述
根据接线图ADC-spi3.png，sensor_task_v3已经更新以支持所有模拟传感器通过ADS8688 ADC读取。

## 🔌 硬件接线配置

### ADS8688 ADC通道分配 (8通道)
根据接线图，8个模拟传感器信号接到ADS8688：

```
ADS8688 CH0 <- TEMPERATURE_ADC1  (温度传感器1)
ADS8688 CH1 <- TEMPERATURE_ADC2  (温度传感器2)
ADS8688 CH2 <- TEMPERATURE_ADC3  (温度传感器3)
ADS8688 CH3 <- PRESSURE_ADC1     (压力传感器1)
ADS8688 CH4 <- PRESSURE_ADC2     (压力传感器2)
ADS8688 CH5 <- PRESSURE_ADC3     (压力传感器3)
ADS8688 CH6 <- PRESSURE_ADC4     (压力传感器4)
ADS8688 CH7 <- LIQUID_LEVEL_ADC  (模拟液位传感器)
```

### 浮球液位开关 (开关量)
```
浮球开关1: PG9  (上拉输入)
浮球开关2: PG12 (上拉输入)
浮球开关3: PG15 (上拉输入)
```

### SPI接口
```
SPI3_SCK  -> PC10
SPI3_MISO -> PC11
SPI3_MOSI -> PC12
ADS8688_CS -> PA15
ADS8688_RST -> PB8
```

## 📊 传感器枚举更新

```c
typedef enum {
    SENSOR_TEMP_1       = 0,    // 温度传感器1 - ADS8688 CH0
    SENSOR_TEMP_2       = 1,    // 温度传感器2 - ADS8688 CH1
    SENSOR_TEMP_3       = 2,    // 温度传感器3 - ADS8688 CH2
    SENSOR_PRESSURE_1   = 3,    // 压力传感器1 - ADS8688 CH3
    SENSOR_PRESSURE_2   = 4,    // 压力传感器2 - ADS8688 CH4
    SENSOR_PRESSURE_3   = 5,    // 压力传感器3 - ADS8688 CH5
    SENSOR_PRESSURE_4   = 6,    // 压力传感器4 - ADS8688 CH6
    SENSOR_LEVEL_FLOAT_1 = 7,   // 浮球液位开关1 (PG9)
    SENSOR_LEVEL_FLOAT_2 = 8,   // 浮球液位开关2 (PG12)
    SENSOR_LEVEL_FLOAT_3 = 9,   // 浮球液位开关3 (PG15)
    SENSOR_LEVEL_ANALOG = 10,   // 模拟液位 - ADS8688 CH7
    SENSOR_FLOW         = 11,   // 流量传感器(I2C)
    SENSOR_COUNT        = 12    // 传感器总数
} sensor_type_t;
```

## 🗄️ 数据结构更新

```c
typedef struct {
    // 所有传感器数据
    sensor_data_t sensors[SENSOR_COUNT];

    // 分类数据 (便于访问)
    float temp_values[3];         // 温度值 (°C) - 来自ADS8688 CH0-2
    float pressure_values[4];     // 压力值 (kPa) - 来自ADS8688 CH3-6
    float level_values[4];        // 液位值: [0-2]=浮球开关状态(0/1), [3]=模拟液位(mm)
    float flow_value;             // 流量值 (L/min)

    // 整体状态
    uint32_t cycle_count;         // 循环计数
    uint32_t last_update_time;    // 最后更新时间
    uint8_t overall_quality;      // 整体质量 (0-100)
    bool system_ready;            // 系统就绪标志
} sensor_context_t;
```

## 🚀 使用方法

### 1. 初始化
```c
#include "sensor_task_v3.h"

int main(void) {
    // HAL初始化
    HAL_Init();
    SystemClock_Config();

    // 初始化传感器任务 (自动初始化ADS8688和GPIO)
    if (SensorTaskV3_Init() == pdPASS) {
        printf("Sensor Task V3 initialized successfully\r\n");
    }

    // 创建传感器任务
    if (SensorTaskV3_Create() == pdPASS) {
        printf("Sensor Task V3 created successfully\r\n");
    }

    // 启动FreeRTOS调度器
    vTaskStartScheduler();
    while(1);
}
```

### 2. 读取传感器数据

#### 读取温度传感器 (来自ADS8688 CH0-2)
```c
float temperatures[3];
if (SensorTaskV3_GetTemperatures(temperatures) == pdTRUE) {
    printf("Temp1=%.1f°C, Temp2=%.1f°C, Temp3=%.1f°C\r\n",
           temperatures[0], temperatures[1], temperatures[2]);
}
```

#### 读取压力传感器 (来自ADS8688 CH3-6)
```c
float pressures[4];
if (SensorTaskV3_GetPressures(pressures) == pdTRUE) {
    printf("Press1=%.1fkPa, Press2=%.1fkPa, Press3=%.1fkPa, Press4=%.1fkPa\r\n",
           pressures[0], pressures[1], pressures[2], pressures[3]);
}
```

#### 读取液位传感器
```c
// 方法1: 读取所有液位数据
float levels[4];
if (SensorTaskV3_GetLevels(levels) == pdTRUE) {
    printf("Float1=%.0f, Float2=%.0f, Float3=%.0f, AnalogLevel=%.1fmm\r\n",
           levels[0], levels[1], levels[2], levels[3]);
}

// 方法2: 单独读取浮球开关状态
bool float_switches[3];
if (SensorTaskV3_GetFloatSwitchStates(float_switches) == pdTRUE) {
    printf("Float switches: %s, %s, %s\r\n",
           float_switches[0] ? "HIGH" : "LOW",
           float_switches[1] ? "HIGH" : "LOW",
           float_switches[2] ? "HIGH" : "LOW");
}

// 方法3: 单独读取模拟液位值 (来自ADS8688 CH7)
float analog_level = SensorTaskV3_GetAnalogLevel();
printf("Analog level: %.2f mm\r\n", analog_level);
```

#### 读取流量传感器
```c
float flow_rate = SensorTaskV3_GetFlowRate();
printf("Flow rate: %.2f L/min\r\n", flow_rate);
```

### 3. 读取完整传感器上下文
```c
void read_all_sensors_example(void)
{
    sensor_context_t context;

    if (SensorTaskV3_GetContext(&context) == pdTRUE) {
        // 温度数据 (来自ADS8688 CH0-2)
        printf("Temperatures: %.1f, %.1f, %.1f °C\r\n",
               context.temp_values[0],
               context.temp_values[1],
               context.temp_values[2]);

        // 压力数据 (来自ADS8688 CH3-6)
        printf("Pressures: %.1f, %.1f, %.1f, %.1f kPa\r\n",
               context.pressure_values[0],
               context.pressure_values[1],
               context.pressure_values[2],
               context.pressure_values[3]);

        // 液位数据
        printf("Float switches: %.0f, %.0f, %.0f\r\n",
               context.level_values[0],  // 浮球开关1 (PG9)
               context.level_values[1],  // 浮球开关2 (PG12)
               context.level_values[2]); // 浮球开关3 (PG15)

        printf("Analog level: %.1f mm\r\n", context.level_values[3]); // ADS8688 CH7

        // 流量数据
        printf("Flow Rate: %.2f L/min\r\n", context.flow_value);

        // 系统状态
        printf("Quality: %d%%, Ready: %s\r\n",
               context.overall_quality,
               context.system_ready ? "YES" : "NO");
    }
}
```

### 4. 使用消息队列
```c
void sensor_data_consumer_task(void *pvParameters)
{
    sensor_msg_t msg;

    while (1) {
        if (SensorTaskV3_ReceiveMessage(&msg, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (msg.type == MSG_SENSOR_DATA) {
                printf("Received sensor data at %lu ms\r\n", msg.timestamp);

                // 检查温度异常
                if (msg.context.temp_values[0] > 80.0f) {
                    printf("High temperature warning: %.1f°C\r\n",
                           msg.context.temp_values[0]);
                }

                // 检查压力异常
                if (msg.context.pressure_values[0] > 500.0f) {
                    printf("High pressure warning: %.1fkPa\r\n",
                           msg.context.pressure_values[0]);
                }

                // 检查浮球开关状态
                if (msg.context.level_values[0] > 0.5f) {
                    printf("Float switch 1 activated!\r\n");
                }

                // 检查模拟液位
                if (msg.context.level_values[3] > 80.0f) {
                    printf("High liquid level: %.1fmm\r\n",
                           msg.context.level_values[3]);
                }
            }
        }
    }
}
```

## 🔧 调试和诊断

### 调试输出
传感器任务每5秒(100个周期)打印状态：
```
[SensorV3] Cycle=100, Quality=95%, Temp1=25.3°C, Press1=102.5kPa, Float1=0, AnalogLevel=45.8mm
```

### 初始化日志
```
[SensorV3] ADS8688 Channel Mapping:
  CH0-2: Temperature sensors
  CH3-6: Pressure sensors
  CH7:   Analog level sensor
[SensorV3] Float switch GPIO initialized
[SensorV3] Hardware initialization completed
[SensorV3] Sensor configurations initialized (Total: 12 sensors)
```

## 📈 传感器特性

### 温度传感器 (FTT518 Pt100)
- **输入范围**: 0-5V (ADS8688 CH0,1,6,7配置)
- **温度范围**: -50°C到+150°C (假设线性关系)
- **转换公式**: `temperature = (voltage * 40.0f) - 50.0f`

### 压力传感器 (HP10MY)
- **输入范围**: 0-10V (ADS8688 CH2,3,4,5配置)
- **压力范围**: 0-1000kPa
- **转换公式**: `pressure = voltage * 100.0f`

### 模拟液位传感器 (FRD-8061)
- **输入范围**: 0-5V (ADS8688 CH7)
- **液位范围**: 0-100mm
- **转换公式**: `level = voltage * 20.0f`

### 浮球液位开关
- **输入**: 数字GPIO (PG9, PG12, PG15)
- **逻辑**: 高电平=液位触发, 低电平=液位未触发
- **上拉**: 内部上拉使能

## ⚠️ 注意事项

1. **ADS8688重复读取**: 当前每个传感器函数都会读取完整的8通道，这会导致重复的SPI通信。建议优化为一次读取，多处使用。

2. **电压范围配置**: 确保ADS8688的通道量程配置与实际传感器输出电压范围匹配。

3. **传感器转换公式**: 当前使用的是假设的线性转换关系，实际应用中需要根据传感器数据手册调整。

4. **GPIO配置**: 浮球开关使用上拉输入，确保硬件连接正确。

5. **任务优先级**: 传感器任务优先级为8，确保与其他任务的优先级协调。

## 🔄 升级要点

### 主要变化
1. **传感器枚举**: 从19个减少到12个传感器
2. **数据结构**: `level_values[4]`替代了`float_switch_states[3]`和`level_adc_values[8]`
3. **读取方式**: 所有模拟传感器都通过ADS8688读取
4. **API更新**: 新增`SensorTaskV3_GetAnalogLevel()`函数

### 向后兼容性
- `SensorTaskV3_GetLevels()`函数保持兼容，但返回数据含义变化
- `SensorTaskV3_GetFloatSwitchStates()`函数保持兼容
- 删除了`SensorTaskV3_GetLevelADCValues()`函数，替换为`SensorTaskV3_GetAnalogLevel()`

这个更新版本完全符合接线图ADC-spi3.png的硬件设计，支持通过ADS8688读取所有模拟传感器，同时保持浮球开关的GPIO读取方式。