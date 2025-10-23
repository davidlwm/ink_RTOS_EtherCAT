# 传感器任务V3使用文档

## 概述

本文档描述基于墨路控制系统详细设计文档V3实现的新传感器任务模块(`sensor_task_v3.c/h`)。该模块采用四层架构设计，提供了更加模块化、可维护和可扩展的传感器数据采集功能。

## 文件位置

```
ink_RTOS_EtherCAT/
├── Inc/
│   └── sensor_task_v3.h        # 传感器任务头文件
└── Src/
    └── sensor_task_v3.c        # 传感器任务实现文件
```

## 与现有sensor_tasks的区别

| 特性 | sensor_tasks.c (现有) | sensor_task_v3.c (新) |
|------|---------------------|---------------------|
| 架构设计 | 针对EtherCAT通信 | 基于V3四层架构 |
| 传感器类型 | 数字输入/模拟输入 | 12种具体传感器类型 |
| 数据结构 | 通用传感器数据 | 按温度/压力/液位/流量分类 |
| 滤波算法 | 简单移动平均 | 可配置滤波系数 |
| 校准功能 | 基础校准 | 完整的在线校准 |
| 消息队列 | 与EtherCAT集成 | 独立的传感器消息队列 |
| 统计信息 | 基础统计 | 详细的性能统计 |

## 支持的传感器类型

根据设计文档V3第2.2.1节，支持以下传感器:

1. **温度传感器** (3路)
   - FTT518 Pt100温度传感器
   - 测量范围: -50°C ~ 200°C
   - 精度: ±0.1°C

2. **压力传感器** (4路)
   - HP10MY压力传感器
   - 测量范围: 0 ~ 1000 kPa
   - 精度: ±0.5%

3. **液位传感器** (3路数字 + 1路模拟)
   - FRD-8061液位传感器
   - 测量范围: 0 ~ 1000 mm
   - 精度: ±1 mm

4. **流量传感器** (1路 I2C)
   - I2C接口流量传感器
   - 测量范围: 0 ~ 100 L/min
   - 精度: ±2%

## 核心功能

### 1. 初始化和创建

```c
// 初始化传感器任务系统
BaseType_t result = SensorTaskV3_Init();
if (result != pdPASS) {
    printf("Sensor task initialization failed!\r\n");
}

// 创建传感器任务
result = SensorTaskV3_Create();
if (result != pdPASS) {
    printf("Sensor task creation failed!\r\n");
}
```

### 2. 获取传感器数据

#### 获取完整上下文

```c
sensor_context_t context;
if (SensorTaskV3_GetContext(&context) == pdTRUE) {
    printf("Temperature 1: %.1f°C\r\n", context.temp_values[0]);
    printf("Pressure 1: %.1f kPa\r\n", context.pressure_values[0]);
    printf("Level 1: %.1f mm\r\n", context.level_values[0]);
    printf("Flow: %.2f L/min\r\n", context.flow_value);
    printf("Overall Quality: %d%%\r\n", context.overall_quality);
}
```

#### 获取单个传感器数据

```c
sensor_data_t sensor_data;
if (SensorTaskV3_GetSensorData(SENSOR_TEMP_1, &sensor_data) == pdTRUE) {
    printf("Raw: %.2f, Filtered: %.2f, Calibrated: %.2f\r\n",
           sensor_data.raw_value,
           sensor_data.filtered_value,
           sensor_data.calibrated_value);
    printf("Quality: %d%%, Valid: %d\r\n",
           sensor_data.quality,
           sensor_data.valid);
}
```

#### 获取分类数据

```c
// 获取所有温度值
float temperatures[3];
if (SensorTaskV3_GetTemperatures(temperatures) == pdTRUE) {
    printf("Temp1=%.1f, Temp2=%.1f, Temp3=%.1f\r\n",
           temperatures[0], temperatures[1], temperatures[2]);
}

// 获取所有压力值
float pressures[4];
if (SensorTaskV3_GetPressures(pressures) == pdTRUE) {
    printf("Press1=%.1f, Press2=%.1f, Press3=%.1f, Press4=%.1f\r\n",
           pressures[0], pressures[1], pressures[2], pressures[3]);
}

// 获取所有液位值
float levels[4];
if (SensorTaskV3_GetLevels(levels) == pdTRUE) {
    printf("Level1=%.1f, Level2=%.1f, Level3=%.1f, Analog=%.1f\r\n",
           levels[0], levels[1], levels[2], levels[3]);
}

// 获取流量值
float flow = SensorTaskV3_GetFlowRate();
printf("Flow: %.2f L/min\r\n", flow);
```

### 3. 传感器配置

```c
// 配置温度传感器1
sensor_config_t temp_config = {
    .channel = 0,                   // ADC通道0
    .scale_factor = 0.1f,           // 0.1°C per unit
    .offset = -2.5f,                // 零点偏移
    .filter_coefficient = 0.8f,     // 滤波系数
    .sample_count = 10,             // 采样次数
    .enabled = true                 // 使能
};

SensorTaskV3_ConfigureSensor(SENSOR_TEMP_1, &temp_config);
```

### 4. 传感器校准

```c
// 校准温度传感器1，参考值为25.0°C
if (SensorTaskV3_CalibrateSensor(SENSOR_TEMP_1, 25.0f) == pdTRUE) {
    printf("Temperature sensor 1 calibrated successfully\r\n");
}
```

### 5. 消息队列通信

```c
// 接收传感器消息
sensor_msg_t msg;
if (SensorTaskV3_ReceiveMessage(&msg, 100) == pdTRUE) {
    switch (msg.type) {
        case MSG_SENSOR_DATA:
            printf("Received sensor data, timestamp=%lu\r\n", msg.timestamp);
            // 处理传感器数据
            break;

        case MSG_SENSOR_ERROR:
            printf("Sensor error detected!\r\n");
            break;

        case MSG_SENSOR_CALIBRATE:
            printf("Sensor calibration event\r\n");
            break;

        default:
            break;
    }
}
```

### 6. 统计信息

```c
// 获取统计信息
sensor_task_stats_t stats;
SensorTaskV3_GetStatistics(&stats);

printf("=== Sensor Task Statistics ===\r\n");
printf("Total Cycles: %lu\r\n", stats.total_cycles);
printf("Data Errors: %lu\r\n", stats.data_errors);
printf("Queue Full: %lu\r\n", stats.queue_full_count);
printf("Max Cycle Time: %d us\r\n", stats.max_cycle_time_us);
printf("Avg Cycle Time: %d us\r\n", stats.avg_cycle_time_us);
printf("Total Samples: %lu\r\n", stats.total_samples);

// 重置统计信息
SensorTaskV3_ResetStatistics();
```

### 7. 健康检查

```c
// 检查传感器系统健康状态
uint8_t health = SensorTaskV3_CheckHealth();
if (health < 80) {
    printf("WARNING: Sensor system health is low: %d%%\r\n", health);
} else {
    printf("Sensor system health: %d%%\r\n", health);
}
```

## 任务配置参数

在`sensor_task_v3.h`中可以配置以下参数:

```c
#define SENSOR_TASK_PRIORITY           8        // 任务优先级
#define SENSOR_TASK_STACK_SIZE         1024     // 堆栈大小 (words)
#define SENSOR_TASK_PERIOD_MS          50       // 任务周期 50ms
#define SENSOR_MSG_QUEUE_SIZE          16       // 消息队列大小
```

## 事件标志

可以使用FreeRTOS事件组监听以下事件:

```c
// 等待传感器数据就绪
EventBits_t bits = xEventGroupWaitBits(
    xEventGroup_Sensor,
    EVENT_SENSOR_DATA_READY,
    pdTRUE,     // 清除事件位
    pdFALSE,    // 不需要所有位
    pdMS_TO_TICKS(100)
);

if (bits & EVENT_SENSOR_DATA_READY) {
    // 传感器数据已就绪
}
```

支持的事件标志:
- `EVENT_SENSOR_DATA_READY` - 传感器数据就绪
- `EVENT_SENSOR_ERROR` - 传感器错误
- `EVENT_SENSOR_CALIBRATE` - 传感器校准请求
- `EVENT_SENSOR_CONFIG_UPDATE` - 传感器配置更新

## 集成到现有系统

### 1. 在main.c中初始化

```c
int main(void)
{
    // ... 其他初始化 ...

    // 初始化传感器任务V3
    if (SensorTaskV3_Init() != pdPASS) {
        Error_Handler();
    }

    // 创建传感器任务V3
    if (SensorTaskV3_Create() != pdPASS) {
        Error_Handler();
    }

    // 启动调度器
    vTaskStartScheduler();

    // ...
}
```

### 2. 在控制任务中使用

```c
void Task_Control(void *pvParameters)
{
    sensor_context_t sensor_ctx;

    for (;;) {
        // 获取传感器数据
        if (SensorTaskV3_GetContext(&sensor_ctx) == pdTRUE) {
            // 使用传感器数据进行控制
            float temp = sensor_ctx.temp_values[0];
            float pressure = sensor_ctx.pressure_values[0];

            // PID控制逻辑...
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
```

## 扩展与自定义

### 添加新的传感器类型

1. 在`sensor_task_v3.h`中添加枚举:

```c
typedef enum {
    // ... 现有传感器 ...
    SENSOR_CUSTOM = 12,     // 自定义传感器
    SENSOR_COUNT = 13       // 更新总数
} sensor_type_t;
```

2. 在`Sensor_InitializeConfigs()`中添加配置

3. 在`Sensor_ReadAllSensors()`中添加读取逻辑

### 修改滤波算法

在`Sensor_ApplyFilter()`函数中可以实现不同的滤波算法:
- 移动平均滤波 (当前实现)
- 卡尔曼滤波
- 中值滤波
- 低通滤波

## 性能指标

- **任务周期**: 50ms (可配置)
- **任务优先级**: 8 (中等优先级)
- **堆栈大小**: 1024 words (4KB)
- **最大传感器数量**: 12路
- **滤波深度**: 8个样本
- **消息队列深度**: 16条消息

## 调试建议

1. 使用串口查看调试信息:
   - 每100个周期打印一次传感器数据
   - 配置和校准操作都有日志输出

2. 监控统计信息:
   - 定期调用`SensorTaskV3_GetStatistics()`
   - 关注`data_errors`和`queue_full_count`

3. 使用事件标志:
   - 监听`EVENT_SENSOR_ERROR`
   - 检查传感器健康状态

## 未来改进

根据V3设计文档，未来可以添加:

1. **驱动层集成**:
   - 集成FRD8061液位传感器驱动
   - 集成FTT518温度传感器驱动
   - 集成HP10MY压力传感器驱动

2. **HAL层抽象**:
   - 添加ADC HAL接口
   - 添加I2C HAL接口

3. **中间件层**:
   - 集成PID控制器
   - 集成高级滤波算法

4. **安全监控**:
   - 集成安全任务接口
   - 添加传感器故障诊断

## 参考文档

- `墨路控制系统详细设计文档v3.md` - 第2.2节 传感器任务设计
- `sensor_tasks.c` - 现有传感器任务实现
- FreeRTOS官方文档

## 联系方式

如有问题，请参考设计文档或联系开发团队。

---

**版本**: V3.0.0
**日期**: 2025-01-23
**作者**: Ink Supply Control System Development Team
