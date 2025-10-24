# 墨路控制系统数据流程思维导图

本文档包含多个思维导图，从不同角度展示墨路控制系统的数据处理流程和任务配合机制。

## 1. 系统整体架构图

```mermaid
graph TB
    subgraph "硬件层"
        HW1[ADS8688 ADC<br/>温度/压力/液位传感器]
        HW2[GPIO<br/>浮球液位开关]
        HW3[I2C<br/>流量传感器]
        HW4[GPIO输出<br/>电磁阀/加热器/直流泵]
        HW5[PWM输出<br/>调速泵]
    end

    subgraph "FreeRTOS任务层"
        SENSOR[传感器任务<br/>SensorV3<br/>50ms/优先级8]
        CONTROL[控制器任务<br/>ControlV3<br/>20ms/优先级12]
        ACTUATOR[执行器任务<br/>ActuatorV3<br/>10ms/优先级8]
    end

    subgraph "数据结构"
        SC[sensor_context_t<br/>传感器上下文]
        CC[control_context_t<br/>控制上下文]
        AC[actuator_context_t<br/>执行器上下文]
    end

    HW1 --> SENSOR
    HW2 --> SENSOR
    HW3 --> SENSOR
    SENSOR --> SC
    SC --> CONTROL
    CONTROL --> CC
    CC --> ACTUATOR
    ACTUATOR --> AC
    ACTUATOR --> HW4
    ACTUATOR --> HW5

    style SENSOR fill:#e1f5fe
    style CONTROL fill:#f3e5f5
    style ACTUATOR fill:#e8f5e8
```

## 2. 数据流向详细图

```mermaid
flowchart LR
    subgraph "传感器数据采集"
        A1[ADS8688读取<br/>8通道ADC]
        A2[GPIO读取<br/>3个浮球开关]
        A3[I2C读取<br/>流量传感器]
        A4[数据滤波<br/>移动平均]
        A5[数据校准<br/>scale+offset]
        A6[质量评估<br/>0-100分]
    end

    subgraph "控制算法处理"
        B1[获取传感器数据<br/>SensorTaskV3_GetContext]
        B2[12个控制回路<br/>PID计算]
        B3[输出限制<br/>0-100%]
        B4[报警检查<br/>高低限]
        B5[质量评估<br/>误差分析]
    end

    subgraph "执行器输出控制"
        C1[命令队列处理<br/>FIFO队列]
        C2[安全检查<br/>故障检测]
        C3[GPIO输出<br/>数字开关]
        C4[PWM输出<br/>模拟调速]
        C5[状态反馈<br/>运行统计]
    end

    A1 --> A4
    A2 --> A4
    A3 --> A4
    A4 --> A5
    A5 --> A6
    A6 --> B1

    B1 --> B2
    B2 --> B3
    B3 --> B4
    B4 --> B5
    B5 --> C1

    C1 --> C2
    C2 --> C3
    C2 --> C4
    C3 --> C5
    C4 --> C5

    style A1 fill:#bbdefb
    style B2 fill:#e1bee7
    style C3 fill:#c8e6c9
    style C4 fill:#c8e6c9
```

## 3. 控制回路映射关系图

```mermaid
graph TD
    subgraph "传感器"
        ST1[SENSOR_TEMP_1<br/>温度1]
        ST2[SENSOR_TEMP_2<br/>温度2]
        ST3[SENSOR_TEMP_3<br/>温度3]
        SP1[SENSOR_PRESSURE_1<br/>压力1]
        SP2[SENSOR_PRESSURE_2<br/>压力2]
        SP3[SENSOR_PRESSURE_3<br/>压力3]
        SP4[SENSOR_PRESSURE_4<br/>压力4]
        SL1[SENSOR_LEVEL_FLOAT_1<br/>浮球1]
        SL2[SENSOR_LEVEL_FLOAT_2<br/>浮球2]
        SL3[SENSOR_LEVEL_FLOAT_3<br/>浮球3]
        SLA[SENSOR_LEVEL_ANALOG<br/>模拟液位]
        SF[SENSOR_FLOW<br/>流量]
    end

    subgraph "控制回路"
        CT1[CONTROL_LOOP_TEMP_1<br/>温度控制1]
        CT2[CONTROL_LOOP_TEMP_2<br/>温度控制2]
        CT3[CONTROL_LOOP_TEMP_3<br/>温度控制3]
        CP1[CONTROL_LOOP_PRESSURE_1<br/>压力控制1]
        CP2[CONTROL_LOOP_PRESSURE_2<br/>压力控制2]
        CP3[CONTROL_LOOP_PRESSURE_3<br/>压力控制3]
        CP4[CONTROL_LOOP_PRESSURE_4<br/>压力控制4]
        CL1[CONTROL_LOOP_LEVEL_1<br/>液位控制1]
        CL2[CONTROL_LOOP_LEVEL_2<br/>液位控制2]
        CL3[CONTROL_LOOP_LEVEL_3<br/>液位控制3]
        CL4[CONTROL_LOOP_LEVEL_4<br/>液位控制4]
        CF[CONTROL_LOOP_FLOW<br/>流量控制]
    end

    subgraph "执行器"
        AH1[ACTUATOR_HEATER_1<br/>加热器1]
        AH2[ACTUATOR_HEATER_2<br/>加热器2]
        AH3[ACTUATOR_HEATER_3<br/>加热器3]
        APS1[ACTUATOR_PUMP_SPEED_1<br/>调速泵1]
        APS2[ACTUATOR_PUMP_SPEED_2<br/>调速泵2]
        AV1[ACTUATOR_VALVE_1<br/>电磁阀1]
        AV2[ACTUATOR_VALVE_2<br/>电磁阀2]
    end

    ST1 --> CT1 --> AH1
    ST2 --> CT2 --> AH2
    ST3 --> CT3 --> AH3
    SP1 --> CP1 --> APS1
    SP2 --> CP2 --> APS2
    SP3 --> CP3 --> APS1
    SP4 --> CP4 --> APS2
    SL1 --> CL1 --> AV1
    SL2 --> CL2 --> AV2
    SL3 --> CL3 --> AV1
    SLA --> CL4 --> AV2
    SF --> CF --> APS1

    style CT1 fill:#ffcdd2
    style CT2 fill:#ffcdd2
    style CT3 fill:#ffcdd2
    style CP1 fill:#c8e6c9
    style CP2 fill:#c8e6c9
    style CP3 fill:#c8e6c9
    style CP4 fill:#c8e6c9
    style CL1 fill:#bbdefb
    style CL2 fill:#bbdefb
    style CL3 fill:#bbdefb
    style CL4 fill:#bbdefb
    style CF fill:#fff9c4
```

## 4. 任务时序关系图

```mermaid
gantt
    title 墨路控制系统任务时序图
    dateFormat X
    axisFormat %L ms

    section 执行器任务
    EXEC1    :0, 10
    EXEC2    :10, 20
    EXEC3    :20, 30
    EXEC4    :30, 40
    EXEC5    :40, 50
    EXEC6    :50, 60
    EXEC7    :60, 70
    EXEC8    :70, 80

    section 控制器任务
    CTRL1    :0, 20
    CTRL2    :20, 40
    CTRL3    :40, 60
    CTRL4    :60, 80

    section 传感器任务
    SENS1    :0, 50
    SENS2    :50, 100
```

## 5. 通信机制图

```mermaid
graph TB
    subgraph "传感器任务"
        S1[数据采集]
        S2[数据处理]
        S3[上下文更新]
        S4[消息发送]
    end

    subgraph "控制器任务"
        C1[命令处理]
        C2[数据获取]
        C3[PID计算]
        C4[输出更新]
    end

    subgraph "执行器任务"
        A1[命令接收]
        A2[安全检查]
        A3[硬件输出]
        A4[状态反馈]
    end

    subgraph "通信机制"
        API1[API调用<br/>SensorTaskV3_GetContext]
        API2[API调用<br/>ActuatorTaskV3_SetOutput]
        Q1[消息队列<br/>xQueue_SensorMsg]
        Q2[命令队列<br/>xQueue_ActuatorCmd]
        E1[事件组<br/>EVENT_SENSOR_DATA_READY]
        E2[事件组<br/>EVENT_ACTUATOR_FAULT]
        M1[互斥体<br/>xMutex_SensorContext]
        M2[互斥体<br/>xMutex_ControlContext]
    end

    S3 --> API1
    API1 --> C2
    C4 --> API2
    API2 --> A1

    S4 --> Q1
    Q1 --> C1

    A1 --> Q2
    Q2 --> A2

    S4 --> E1
    E1 --> C2

    A4 --> E2
    E2 --> C1

    S3 --> M1
    M1 --> C2

    C3 --> M2
    M2 --> C4

    style API1 fill:#e3f2fd
    style API2 fill:#e3f2fd
    style Q1 fill:#f3e5f5
    style Q2 fill:#f3e5f5
    style E1 fill:#e8f5e8
    style E2 fill:#e8f5e8
    style M1 fill:#fff3e0
    style M2 fill:#fff3e0
```

## 6. 数据处理流水线图

```mermaid
flowchart TB
    subgraph "输入层"
        I1[硬件传感器<br/>物理信号]
        I2[用户设定<br/>目标值]
        I3[安全参数<br/>限制值]
    end

    subgraph "感知层"
        P1[信号转换<br/>模数转换]
        P2[数据滤波<br/>噪声去除]
        P3[数据校准<br/>线性化]
        P4[质量评估<br/>有效性检查]
    end

    subgraph "决策层"
        D1[误差计算<br/>设定值-反馈值]
        D2[PID算法<br/>比例+积分+微分]
        D3[输出限制<br/>范围约束]
        D4[安全逻辑<br/>故障保护]
    end

    subgraph "执行层"
        E1[命令解析<br/>目标分解]
        E2[硬件驱动<br/>GPIO/PWM]
        E3[状态监控<br/>反馈检测]
        E4[故障处理<br/>异常响应]
    end

    subgraph "反馈层"
        F1[状态反馈<br/>执行确认]
        F2[性能监控<br/>质量评估]
        F3[故障报告<br/>异常通知]
    end

    I1 --> P1
    I2 --> D1
    I3 --> D4

    P1 --> P2
    P2 --> P3
    P3 --> P4
    P4 --> D1

    D1 --> D2
    D2 --> D3
    D3 --> D4
    D4 --> E1

    E1 --> E2
    E2 --> E3
    E3 --> E4
    E4 --> F1

    F1 --> F2
    F2 --> F3
    F3 --> P4
    E3 --> P1

    style P1 fill:#e1f5fe
    style D2 fill:#f3e5f5
    style E2 fill:#e8f5e8
    style F2 fill:#fff3e0
```

## 7. 系统状态机图

```mermaid
stateDiagram-v2
    [*] --> 系统初始化
    系统初始化 --> 传感器初始化
    传感器初始化 --> 控制器初始化
    控制器初始化 --> 执行器初始化
    执行器初始化 --> 系统就绪

    系统就绪 --> 正常运行
    正常运行 --> 数据采集
    数据采集 --> 控制计算
    控制计算 --> 输出执行
    输出执行 --> 状态监控
    状态监控 --> 数据采集

    状态监控 --> 警告状态 : 参数超限
    警告状态 --> 正常运行 : 参数恢复
    警告状态 --> 报警状态 : 持续超限

    状态监控 --> 故障状态 : 硬件故障
    报警状态 --> 故障状态 : 严重超限

    故障状态 --> 安全模式
    安全模式 --> 紧急停止 : 严重故障
    安全模式 --> 故障诊断

    故障诊断 --> 故障恢复 : 故障排除
    故障恢复 --> 系统就绪

    紧急停止 --> 手动复位
    手动复位 --> 系统初始化

    正常运行 --> 紧急停止 : 紧急命令
```

## 总结

这些思维导图从不同维度展示了墨路控制系统的运作机制：

1. **架构图**: 展示硬件、任务和数据结构的层次关系
2. **数据流图**: 详细描述数据在系统中的流转过程
3. **映射关系图**: 清晰展示传感器-控制回路-执行器的对应关系
4. **时序图**: 显示三个任务的时间关系和并发执行
5. **通信机制图**: 描述任务间的通信方式和同步机制
6. **流水线图**: 展示从输入到输出的完整数据处理流程
7. **状态机图**: 描述系统的各种运行状态和状态转换

通过这些可视化图表，可以清楚地理解墨路控制系统的工作原理和各组件之间的协调配合机制。