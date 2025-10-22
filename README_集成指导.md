# EtherCAT项目集成指导

## 📁 文件说明

本目录包含以下新增文件用于扩展EtherCAT主从站交互功能：

### 新增文件
1. **`EtherCAT_主从站交互开发指南.md`** - 完整的开发指导文档
2. **`Inc/app_io_handler.h`** - 扩展IO处理模块头文件
3. **`Src/app_io_handler.c`** - 扩展IO处理模块实现
4. **`Src/ssc_device_extended.c`** - SSC设备扩展实现

## 🚀 快速集成步骤

### 步骤1: 添加文件到项目
1. 将`Inc/app_io_handler.h`添加到项目包含路径
2. 将`Src/app_io_handler.c`和`Src/ssc_device_extended.c`添加到编译列表

### 步骤2: 修改现有文件

#### 修改`Src/SSC-Device.c`
在文件顶部添加包含：
```c
#include "app_io_handler.h"
```

替换或修改以下函数：
```c
void APPL_Application(void)
{
    // 原有代码保持兼容性
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

    // 新增扩展IO处理
    App_IO_Handler();
}
```

#### 修改`Src/main.c`
在main()函数中添加初始化：
```c
int main(void)
{
    /* 系统初始化 */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_DAC_Init();
    // ... 其他初始化

    /* EtherCAT初始化 */
    HW_Init();
    MainInit();

    /* 扩展IO初始化 */
    App_IO_Init();  // 🔥 新增这行

    /* 主循环 */
    bRunApplication = TRUE;
    do {
        MainLoop();
    } while (bRunApplication == TRUE);

    HW_Release();
    return 0;
}
```

### 步骤3: 硬件配置
根据实际硬件修改`Src/app_io_handler.c`中的引脚配置表：

```c
// 数字输入引脚配置
static const digital_pin_config_t digital_input_pins[MAX_DIGITAL_INPUTS] = {
    {GPIOE, GPIO_PIN_2,  1, 0},   // DI0 - 根据实际硬件修改
    {GPIOE, GPIO_PIN_3,  1, 0},   // DI1
    // ... 继续配置其他引脚
};

// 数字输出引脚配置
static const digital_pin_config_t digital_output_pins[MAX_DIGITAL_OUTPUTS] = {
    {GPIOB, GPIO_PIN_11, 1, 0},   // DO0 - 根据实际硬件修改
    {GPIOB, GPIO_PIN_12, 1, 0},   // DO1
    // ... 继续配置其他引脚
};
```

### 步骤4: 编译测试
1. 编译项目，确保无错误
2. 下载到硬件进行测试
3. 使用串口查看调试输出（如果启用了调试功能）

## 📊 功能特性

### ✅ 已实现功能
- [x] **基础IO兼容**: 保持原有2输入+2输出功能
- [x] **扩展数字IO**: 支持16路输入+16路输出
- [x] **模拟量处理**: 支持8路ADC输入+4路DAC/PWM输出
- [x] **实时数据交换**: 通过EtherCAT PDO机制
- [x] **调试支持**: 串口输出IO状态和统计信息
- [x] **配置灵活**: 支持动态配置硬件引脚映射

### 🎯 支持的数据类型
- **数字量**: 布尔开关量 (0/1)
- **模拟量**: 16位有符号整数 (-32768~+32767)
- **位图**: 16位数字IO状态位图

### ⚡ 性能指标
- **响应时间**: < 1ms（取决于EtherCAT主站周期）
- **精度**: ADC 12位，DAC 12位
- **最大通道**: 16数字输入+16数字输出+8模拟输入+4模拟输出

## 🔧 使用示例

### 上位机PLC代码示例（TwinCAT）
```iecst
PROGRAM EtherCAT_IO_Control
VAR
    // 基础IO
    bLED1_Control   : BOOL;
    bSwitch1_Status : BOOL;

    // 扩展数字IO
    wDigitalOut     : WORD;  // 16位数字输出
    wDigitalIn      : WORD;  // 16位数字输入

    // 模拟量
    iAnalogOut1     : INT;   // 模拟输出1
    iAnalogIn1      : INT;   // 模拟输入1
END_VAR

// 简单控制逻辑
bLED1_Control := bSwitch1_Status;  // LED跟随开关
wDigitalOut.0 := wDigitalIn.15;    // 输出位0跟随输入位15
iAnalogOut1 := iAnalogIn1 / 2;     // 模拟输出为输入的一半
```

### 从站端API使用示例
```c
// 设置数字输出
App_Set_Digital_Output(0, 1);  // 设置DO0为高电平
App_Set_Digital_Output(1, 0);  // 设置DO1为低电平

// 读取数字输入
int di0_state = App_Get_Digital_Input(0);  // 读取DI0状态

// 设置模拟输出
App_Set_Analog_Output(0, 16384);  // 设置AO0为50%输出

// 读取模拟输入
int16_t ai0_value = App_Get_Analog_Input(0);  // 读取AI0值
```

## 🐛 故障排除

### 常见问题
1. **编译错误**: 检查头文件包含路径
2. **链接错误**: 确保所有.c文件都添加到编译列表
3. **运行时错误**: 检查硬件引脚配置是否正确
4. **通信异常**: 验证EtherCAT网络连接和配置

### 调试方法
1. 启用调试输出查看IO状态
2. 使用TwinCAT在线监控PDO数据
3. 用示波器检查硬件信号
4. 检查EtherCAT从站状态机

## 📞 技术支持

如遇到问题，请：
1. 查阅`EtherCAT_主从站交互开发指南.md`详细文档
2. 检查硬件连接和配置
3. 查看调试输出信息
4. 参考EtherCAT官方文档

---

**📝 版本信息**
- 版本: v1.0.0
- 更新日期: 2024-01-01
- 兼容SSC版本: v5.12
- 支持硬件: STM32F407 + LAN9252

**✨ 祝您开发顺利！**