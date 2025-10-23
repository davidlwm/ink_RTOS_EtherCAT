# 执行器任务 V3 - 使用文档

## 概述

执行器任务V3（Actuator Task V3）是基于墨路控制系统详细设计文档V3实现的执行器控制模块。本模块负责控制系统中的各种执行器设备，包括电磁阀、加热器和泵等。

## 文件结构

```
Inc/actuator_task_v3.h    - 执行器任务头文件
Src/actuator_task_v3.c    - 执行器任务实现文件
```

## 执行器类型

根据设计文档V3 第2.3.1节的规定：

| 执行器类型 | 编号 | 输出方式 | 控制范围 | 描述 |
|-----------|------|----------|----------|------|
| ACTUATOR_VALVE_1 | 0 | GPIO数字输出 | ON/OFF | 电磁阀1 (24V) |
| ACTUATOR_VALVE_2 | 1 | GPIO数字输出 | ON/OFF | 电磁阀2 (24V) |
| ACTUATOR_HEATER_1 | 2 | 继电器/GPIO | ON/OFF | 加热器1 |
| ACTUATOR_HEATER_2 | 3 | 继电器/GPIO | ON/OFF | 加热器2 |
| ACTUATOR_HEATER_3 | 4 | 继电器/GPIO | ON/OFF | 加热器3 |
| ACTUATOR_PUMP_SPEED_1 | 5 | PWM输出 | 0-100% | 调速泵1 |
| ACTUATOR_PUMP_SPEED_2 | 6 | PWM输出 | 0-100% | 调速泵2 |
| ACTUATOR_PUMP_DC_1 | 7 | GPIO数字输出 | ON/OFF | 直流泵1 |
| ACTUATOR_PUMP_DC_2 | 8 | GPIO数字输出 | ON/OFF | 直流泵2 |

## 任务参数

### 任务配置（参考设计文档V3 第2.1.1节）

- **优先级**: 8
- **堆栈**: 1024字（4096字节）
- **周期**: 10ms
- **任务名**: "ActuatorV3"

### 队列参数

- **命令队列**: 16个元素 (用于接收控制命令)
- **消息队列**: 16个元素 (用于发送状态信息)

## 主要功能

### 1. 输出控制

执行器任务提供以下控制功能：

- **爬坡控制**: 平滑过渡输出值
- **范围限制**: 限制输出在安全范围内
- **输出类型**: 支持数字输出和PWM输出
- **状态监控**: 实时监控执行器状态和反馈

### 2. 安全保护

- **紧急停止**: 立即停止所有执行器
- **故障检测**: 检测执行器故障并自动处理
- **安全模式**: 故障时进入安全模式
- **看门狗**: 防止执行器失控的看门狗机制

### 3. 监控功能

提供全面的执行器监控信息：

- 输出状态和反馈值
- 运行时间统计
- 开关次数统计
- 故障状态和故障代码
- 整体健康评分

## API接口

### 初始化接口

```c
BaseType_t ActuatorTaskV3_Init(void);
```
初始化执行器任务系统，创建队列、互斥体和事件组。

**返回值**:
- `pdPASS`: 初始化成功
- `pdFAIL`: 初始化失败

### 任务创建接口

```c
BaseType_t ActuatorTaskV3_Create(void);
```
创建执行器任务。

**返回值**:
- `pdPASS`: 创建成功
- `pdFAIL`: 创建失败

### 基本控制接口

#### 设置执行器输出

```c
BaseType_t ActuatorTaskV3_SetOutput(actuator_type_t actuator_type, float value);
```

设置指定执行器的输出值。

**参数**:
- `actuator_type`: 执行器类型（0-8）
- `value`: 输出值（0.0-100.0%）

**返回值**:
- `pdTRUE`: 设置成功
- `pdFALSE`: 设置失败

#### 使能执行器

```c
BaseType_t ActuatorTaskV3_Enable(actuator_type_t actuator_type);
```

使能指定的执行器。

**参数**:
- `actuator_type`: 执行器类型

**返回值**:
- `pdTRUE`: 使能成功
- `pdFALSE`: 使能失败

#### 禁用执行器

```c
BaseType_t ActuatorTaskV3_Disable(actuator_type_t actuator_type);
```

禁用指定执行器，输出设为0。

**参数**:
- `actuator_type`: 执行器类型

**返回值**:
- `pdTRUE`: 禁用成功
- `pdFALSE`: 禁用失败

### 安全控制接口

#### 紧急停止

```c
BaseType_t ActuatorTaskV3_EmergencyStop(void);
```

立即停止所有执行器，进入紧急停止状态。

**返回值**:
- `pdTRUE`: 执行成功
- `pdFALSE`: 执行失败

#### 恢复运行

```c
BaseType_t ActuatorTaskV3_Resume(void);
```

从紧急停止状态恢复运行。

**返回值**:
- `pdTRUE`: 恢复成功
- `pdFALSE`: 恢复失败

### 专用控制接口

#### 控制电磁阀

```c
BaseType_t ActuatorTaskV3_SetValve(uint8_t valve_id, bool state);
```

控制电磁阀的开关状态。

**参数**:
- `valve_id`: 阀门ID（0或1）
- `state`: 开关状态（true=开，false=关）

**返回值**:
- `pdTRUE`: 控制成功
- `pdFALSE`: 控制失败

#### 控制加热器

```c
BaseType_t ActuatorTaskV3_SetHeater(uint8_t heater_id, bool state);
```

控制加热器的开关状态。

**参数**:
- `heater_id`: 加热器ID（0、1或2）
- `state`: 开关状态（true=开，false=关）

**返回值**:
- `pdTRUE`: 控制成功
- `pdFALSE`: 控制失败

#### 控制调速泵

```c
BaseType_t ActuatorTaskV3_SetPumpSpeed(uint8_t pump_id, float speed);
```

设置调速泵的转速。

**参数**:
- `pump_id`: 泵ID（0或1）
- `speed`: 转速（0.0-100.0%）

**返回值**:
- `pdTRUE`: 设置成功
- `pdFALSE`: 设置失败

#### 控制直流泵

```c
BaseType_t ActuatorTaskV3_SetDCPump(uint8_t pump_id, bool state);
```

控制直流泵的开关状态。

**参数**:
- `pump_id`: 泵ID（0或1）
- `state`: 开关状态（true=开，false=关）

**返回值**:
- `pdTRUE`: 控制成功
- `pdFALSE`: 控制失败

### 状态查询接口

#### 获取执行器上下文

```c
BaseType_t ActuatorTaskV3_GetContext(actuator_context_t *context);
```

获取完整的执行器上下文信息。

**参数**:
- `context`: 上下文结构体指针

**返回值**:
- `pdTRUE`: 获取成功
- `pdFALSE`: 获取失败

#### 获取执行器状态

```c
BaseType_t ActuatorTaskV3_GetStatus(actuator_type_t actuator_type, actuator_status_t *status);
```

获取指定执行器的状态信息。

**参数**:
- `actuator_type`: 执行器类型
- `status`: 状态结构体指针

**返回值**:
- `pdTRUE`: 获取成功
- `pdFALSE`: 获取失败

#### 获取电磁阀状态

```c
BaseType_t ActuatorTaskV3_GetValveStates(bool *valve_states);
```

获取所有电磁阀的状态。

**参数**:
- `valve_states`: 状态数组指针（至少2个元素）

**返回值**:
- `pdTRUE`: 获取成功
- `pdFALSE`: 获取失败

#### 获取加热器状态

```c
BaseType_t ActuatorTaskV3_GetHeaterStates(bool *heater_states);
```

获取所有加热器的状态。

**参数**:
- `heater_states`: 状态数组指针（至少3个元素）

**返回值**:
- `pdTRUE`: 获取成功
- `pdFALSE`: 获取失败

#### 获取调速泵转速

```c
BaseType_t ActuatorTaskV3_GetPumpSpeeds(float *pump_speeds);
```

获取所有调速泵的转速。

**参数**:
- `pump_speeds`: 转速数组指针（至少2个元素）

**返回值**:
- `pdTRUE`: 获取成功
- `pdFALSE`: 获取失败

### 监控接口

#### 检查系统健康

```c
uint8_t ActuatorTaskV3_CheckHealth(void);
```

检查执行器系统的整体健康状态。

**返回值**: 健康评分（0-100）
- 100: 完全健康
- 80-99: 良好
- 50-79: 一般（有轻微问题）
- 20-49: 较差
- 0-19: 严重故障

#### 获取统计信息

```c
void ActuatorTaskV3_GetStatistics(actuator_task_stats_t *stats);
```

获取执行器任务的统计信息。

**参数**:
- `stats`: 统计信息结构体指针

#### 重置统计信息

```c
void ActuatorTaskV3_ResetStatistics(void);
```

重置执行器任务的统计信息。

## 使用示例

### 示例1: 系统初始化

```c
#include "actuator_task_v3.h"

void main(void)
{
    // 系统初始化...

    // 初始化执行器任务
    if (ActuatorTaskV3_Init() == pdPASS) {
        printf("执行器任务初始化成功\n");

        // 创建执行器任务
        if (ActuatorTaskV3_Create() == pdPASS) {
            printf("执行器任务创建成功\n");
        }
    }

    // 启动调度器
    vTaskStartScheduler();
}
```

### 示例2: 控制电磁阀

```c
// 打开电磁阀1
ActuatorTaskV3_SetValve(0, true);

// 延时1秒
vTaskDelay(pdMS_TO_TICKS(1000));

// 关闭电磁阀1
ActuatorTaskV3_SetValve(0, false);
```

### 示例3: 控制调速泵

```c
// 设置泵1转速为50%
ActuatorTaskV3_SetPumpSpeed(0, 50.0f);

// 延时2秒
vTaskDelay(pdMS_TO_TICKS(2000));

// 设置泵1转速为80%
ActuatorTaskV3_SetPumpSpeed(0, 80.0f);

// 延时2秒
vTaskDelay(pdMS_TO_TICKS(2000));

// 停止泵1
ActuatorTaskV3_SetPumpSpeed(0, 0.0f);
```

### 示例4: 紧急停止

```c
// 检测到紧急情况
if (emergency_condition) {
    // 立即停止所有执行器
    ActuatorTaskV3_EmergencyStop();
    printf("紧急停止已激活!\n");
}

// 紧急情况解除后
if (emergency_cleared) {
    ActuatorTaskV3_Resume();
    printf("系统已恢复运行\n");
}
```

### 示例5: 状态监控

```c
actuator_status_t status;

// 获取电磁阀1的状态
if (ActuatorTaskV3_GetStatus(ACTUATOR_VALVE_1, &status) == pdTRUE) {
    printf("电磁阀1状态:\n");
    printf("  状态: %d\n", status.state);
    printf("  输出: %.1f%%\n", status.output_value);
    printf("  故障: %s\n", status.fault ? "是" : "否");
    printf("  运行时间: %lu 秒\n", status.run_time / 1000);
}
```

### 示例6: 系统健康监控

```c
void MonitorActuatorHealth(void)
{
    uint8_t health_score;
    actuator_task_stats_t stats;

    // 获取健康评分
    health_score = ActuatorTaskV3_CheckHealth();
    printf("执行器系统健康度: %d/100\n", health_score);

    // 获取统计信息
    ActuatorTaskV3_GetStatistics(&stats);
    printf("统计信息:\n");
    printf("  总周期数: %lu\n", stats.total_cycles);
    printf("  命令数: %lu\n", stats.command_count);
    printf("  错误数: %lu\n", stats.command_errors);
    printf("  紧急停止次数: %lu\n", stats.emergency_stops);
    printf("  最大周期时间: %u μs\n", stats.max_cycle_time_us);
    printf("  平均周期时间: %u μs\n", stats.avg_cycle_time_us);
}
```

### 示例7: 完整测试序列

```c
void RunTestSequence(void)
{
    printf("开始执行器测试序列...\n");

    // 1. 测试电磁阀
    printf("测试电磁阀...\n");
    ActuatorTaskV3_SetValve(0, true);
    vTaskDelay(pdMS_TO_TICKS(500));
    ActuatorTaskV3_SetValve(1, true);
    vTaskDelay(pdMS_TO_TICKS(500));
    ActuatorTaskV3_SetValve(0, false);
    ActuatorTaskV3_SetValve(1, false);

    // 2. 测试加热器
    printf("测试加热器...\n");
    for (int i = 0; i < 3; i++) {
        ActuatorTaskV3_SetHeater(i, true);
        vTaskDelay(pdMS_TO_TICKS(500));
        ActuatorTaskV3_SetHeater(i, false);
    }

    // 3. 测试调速泵
    printf("测试调速泵...\n");
    for (float speed = 0.0f; speed <= 100.0f; speed += 25.0f) {
        ActuatorTaskV3_SetPumpSpeed(0, speed);
        printf("泵转速: %.0f%%\n", speed);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    ActuatorTaskV3_SetPumpSpeed(0, 0.0f);

    // 4. 测试直流泵
    printf("测试直流泵...\n");
    ActuatorTaskV3_SetDCPump(0, true);
    vTaskDelay(pdMS_TO_TICKS(2000));
    ActuatorTaskV3_SetDCPump(0, false);

    printf("测试序列完成!\n");
}
```

## 高级功能

### 爬坡控制

对于调速泵等需要平滑变化的输出，系统提供爬坡控制功能：

```c
// 配置爬坡参数
actuator_config_t config;
config.ramp_rate = 10.0f;  // 10%/秒的爬坡速率

// 应用配置
ActuatorTaskV3_Configure(ACTUATOR_PUMP_SPEED_1, &config);
```

### 输出范围限制

可以为每个执行器设置输出范围限制：

```c
actuator_config_t config;
config.min_output = 10.0f;  // 最小输出10%
config.max_output = 90.0f;  // 最大输出90%

ActuatorTaskV3_Configure(ACTUATOR_PUMP_SPEED_1, &config);
```

## 硬件接口

执行器任务通过HAL层抽象与硬件接口。以下接口需要在实际实现中连接：

### 数字输出接口

1. **GPIO控制** - 用于电磁阀、加热器、直流泵
   - `HAL_GPIO_WritePin()`

2. **PWM控制** - 用于调速泵
   - `HAL_TIM_PWM_SetDutyCycle()`

3. **故障检测接口** - 用于监控执行器故障
   - 电流检测
   - 反馈检测
   - 温度检测

## 安全特性

### 1. 紧急停止功能

- 立即停止所有执行器
- 紧急停止队列具有最高优先级
- 停止后需要手动恢复

### 2. 故障检测

- 检测执行器故障状态
- 故障执行器自动停止
- 提供故障代码诊断

### 3. 安全模式

- 系统监控运行状态
- 自动进入安全模式
- 限制输出功能

### 4. 看门狗保护

- 防止执行器失控运行
- 监控运行时间
- 自动保护机制

## 性能参数

- **任务周期**: 10ms
- **队列处理延迟**: < 1个任务周期（10ms）
- **命令响应时间**: 10ms
- **统计信息更新**: 实时统计信息监控
- **日志输出频率**: 可配置的调试输出

## 调试功能

### 输出信息

系统通过printf输出调试信息，便于监控执行器运行状态：

```
[ActuatorV3] 执行器任务系统初始化成功
[ActuatorV3] 执行器任务创建成功
[ActuatorV3] 执行器任务启动
[ActuatorV3] 紧急停止触发!
[ActuatorV3] 执行器 X 故障检测: 代码 0xYY
```

### 统计信息

通过 `ActuatorTaskV3_GetStatistics()` 获取详细的运行统计信息，用于性能分析和调试。

### 健康检查

通过 `ActuatorTaskV3_CheckHealth()` 监控系统的整体健康状态。

## 注意事项

1. **初始化顺序**: 必须先调用 `ActuatorTaskV3_Init()` 再调用 `ActuatorTaskV3_Create()`

2. **硬件依赖**: 需要连接到HAL层的具体实现

3. **任务优先级**: 所有API调用都是线程安全的，确保正确的任务优先级设置

4. **输出限制**: 输出值限制在0-100%范围内

5. **爬坡控制**: 调速泵支持平滑爬坡，数字输出无需爬坡

6. **紧急停止**: 紧急停止状态下必须调用 `ActuatorTaskV3_Resume()` 才能恢复

## 与控制任务的集成

执行器任务与控制任务（Control Task）协同工作：

```c
// 在控制任务中使用执行器
void ControlTask(void)
{
    float pid_output;

    // 计算PID输出
    pid_output = PID_Calculate(...);

    // 设置执行器输出
    ActuatorTaskV3_SetPumpSpeed(0, pid_output);
}
```

## 与安全任务的集成

执行器任务与安全任务（Safety Task）协同实现安全保护：

```c
// 在安全任务中监控
void SafetyTask(void)
{
    // 检测到危险情况
    if (IsDangerousCondition()) {
        // 触发紧急停止
        ActuatorTaskV3_EmergencyStop();

        // 通知其他任务
        NotifyOtherTasks();
    }
}
```

## 参考文档

- **墨路控制系统详细设计文档V3** - 第2.3节执行器设计规范
- **传感器任务V3** - SENSOR_TASK_V3_README.md
- **FreeRTOS官方文档** - 任务管理和同步功能

## 版本历史

- **V3.0.0** (2025-01-23)
  - 初始版本发布
  - 实现完整的执行器控制功能
  - 添加安全保护和故障检测机制
  - 实现统计信息和监控功能

## 后续开发

1. **硬件集成**: 完成实际硬件驱动的集成
2. **故障诊断**: 增强故障检测和诊断功能
3. **通信接口**: 添加通信任务的集成接口
4. **参数调优**: 优化控制参数和性能表现
5. **网络控制**: 增加EtherCAT和TCP等网络控制执行器功能

## 技术支持

如遇到问题，请联系技术支持团队。

---

**作者**: Ink Supply Control System Development Team
**版本**: V3.0.0
**日期**: 2025-01-23
**文档**: 墨路控制系统详细设计文档V3