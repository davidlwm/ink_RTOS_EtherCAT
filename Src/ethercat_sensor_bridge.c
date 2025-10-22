/**
 * @file ethercat_sensor_bridge.c
 * @brief EtherCAT传感器数据桥接模块 - 实现文件
 * @author Claude AI Assistant
 * @version 1.0.0
 * @date 2024-10-21
 */

#include "ethercat_sensor_bridge.h"
#include <string.h>
#include <stdio.h>

/* ========================================================================== */
/* 私有变量 */
/* ========================================================================== */

static EtherCATBridgeConfig_t g_bridge_config;       /**< 桥接配置 */
static EtherCAT_SensorInputs_t g_sensor_inputs;      /**< 扩展传感器输入数据 */
static EtherCAT_SensorOutputs_t g_sensor_outputs;    /**< 扩展传感器输出数据 */
static bool g_bridge_enabled = false;                /**< 桥接启用状态 */
static uint8_t g_bridge_status = BRIDGE_STATUS_OK;   /**< 桥接状态 */
static uint32_t g_update_counter = 0;                /**< 更新计数器 */
static uint32_t g_last_update_time = 0;              /**< 上次更新时间 */

/* 传感器统计数据 */
typedef struct {
    float min_value;
    float max_value;
    float sum_value;
    uint32_t sample_count;
} SensorStats_t;

static SensorStats_t g_sensor_stats[7];              /**< 7个模拟传感器的统计数据 */

/* ========================================================================== */
/* 私有函数声明 */
/* ========================================================================== */

static void _apply_default_config(void);
static void _update_sensor_statistics(void);
static void _convert_sensor_data_to_ethercat(void);
static void _process_control_commands(void);
static int16_t _float_to_int16_scaled(float value, float scale);
static uint16_t _float_to_uint16_scaled(float value, float scale);
static uint32_t _get_system_time_ms(void);

/* ========================================================================== */
/* 公共函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化EtherCAT传感器桥接模块
 */
int EtherCAT_SensorBridge_Init(const EtherCATBridgeConfig_t *config)
{
    /* 清零数据结构 */
    memset(&g_bridge_config, 0, sizeof(EtherCATBridgeConfig_t));
    memset(&g_sensor_inputs, 0, sizeof(EtherCAT_SensorInputs_t));
    memset(&g_sensor_outputs, 0, sizeof(EtherCAT_SensorOutputs_t));
    memset(g_sensor_stats, 0, sizeof(g_sensor_stats));

    /* 设置配置参数 */
    if (config == NULL) {
        _apply_default_config();
    } else {
        memcpy(&g_bridge_config, config, sizeof(EtherCATBridgeConfig_t));
    }

    /* 初始化状态 */
    g_bridge_enabled = false;
    g_bridge_status = BRIDGE_STATUS_OK;
    g_update_counter = 0;
    g_last_update_time = _get_system_time_ms();

    /* 初始化传感器统计数据 */
    for (int i = 0; i < 7; i++) {
        g_sensor_stats[i].min_value = 999999.0f;
        g_sensor_stats[i].max_value = -999999.0f;
        g_sensor_stats[i].sum_value = 0.0f;
        g_sensor_stats[i].sample_count = 0;
    }

    return 0;
}

/**
 * @brief 启动EtherCAT传感器桥接
 */
int EtherCAT_SensorBridge_Start(void)
{
    if (!SensorSimulator_IsEnabled()) {
        /* 如果传感器模拟器未启动，尝试启动它 */
        SensorSimulator_Enable(true);
    }

    g_bridge_enabled = true;
    g_bridge_status = BRIDGE_STATUS_OK;
    g_last_update_time = _get_system_time_ms();

    return 0;
}

/**
 * @brief 停止EtherCAT传感器桥接
 */
void EtherCAT_SensorBridge_Stop(void)
{
    g_bridge_enabled = false;
}

/**
 * @brief 更新传感器数据到EtherCAT输入对象
 */
void EtherCAT_SensorBridge_UpdateInputs(void)
{
    const SensorData_t *sensor_data;
    uint32_t current_time;

    if (!g_bridge_enabled) {
        return;
    }

    current_time = _get_system_time_ms();

    /* 检查更新频率 */
    if (g_bridge_config.data_update_rate > 0) {
        uint32_t update_period = 1000 / g_bridge_config.data_update_rate;
        if ((current_time - g_last_update_time) < update_period) {
            return;
        }
    }

    g_last_update_time = current_time;

    /* 获取传感器数据 */
    sensor_data = SensorSimulator_GetData();
    if (sensor_data == NULL) {
        g_bridge_status = BRIDGE_STATUS_SENSOR_ERROR;
        return;
    }

    /* 转换传感器数据为EtherCAT格式 */
    _convert_sensor_data_to_ethercat();

    /* 更新统计数据 */
    _update_sensor_statistics();

    /* 映射到原有的EtherCAT对象 */
    if (g_bridge_config.enable_digital_io) {
        Obj0x6000.Switch1 = g_sensor_inputs.switch_1;
        Obj0x6000.Switch2 = g_sensor_inputs.switch_2;
    }

    g_update_counter++;
    g_bridge_status = BRIDGE_STATUS_OK;
}

/**
 * @brief 处理EtherCAT输出对象的控制命令
 */
void EtherCAT_SensorBridge_ProcessOutputs(void)
{
    if (!g_bridge_enabled) {
        return;
    }

    /* 读取原有的EtherCAT输出对象 */
    g_sensor_outputs.led_1 = Obj0x7010.Led1;
    g_sensor_outputs.led_2 = Obj0x7010.Led2;

    /* 处理控制命令 */
    _process_control_commands();
}

/**
 * @brief 获取当前桥接状态
 */
uint8_t EtherCAT_SensorBridge_GetStatus(void)
{
    return g_bridge_status;
}

/**
 * @brief 获取桥接配置
 */
const EtherCATBridgeConfig_t* EtherCAT_SensorBridge_GetConfig(void)
{
    return &g_bridge_config;
}

/**
 * @brief 设置桥接配置
 */
int EtherCAT_SensorBridge_SetConfig(const EtherCATBridgeConfig_t *config)
{
    if (config == NULL) {
        return -1;
    }

    memcpy(&g_bridge_config, config, sizeof(EtherCATBridgeConfig_t));
    return 0;
}

/**
 * @brief 重置桥接模块
 */
void EtherCAT_SensorBridge_Reset(void)
{
    g_update_counter = 0;
    g_last_update_time = _get_system_time_ms();
    g_bridge_status = BRIDGE_STATUS_OK;

    /* 重置统计数据 */
    for (int i = 0; i < 7; i++) {
        g_sensor_stats[i].min_value = 999999.0f;
        g_sensor_stats[i].max_value = -999999.0f;
        g_sensor_stats[i].sum_value = 0.0f;
        g_sensor_stats[i].sample_count = 0;
    }

    /* 重置传感器模拟器 */
    SensorSimulator_Reset();
}

/**
 * @brief 获取扩展的传感器输入数据
 */
const EtherCAT_SensorInputs_t* EtherCAT_SensorBridge_GetInputData(void)
{
    return &g_sensor_inputs;
}

/**
 * @brief 获取扩展的传感器输出数据
 */
const EtherCAT_SensorOutputs_t* EtherCAT_SensorBridge_GetOutputData(void)
{
    return &g_sensor_outputs;
}

/**
 * @brief 诊断函数：获取桥接统计信息
 */
int EtherCAT_SensorBridge_GetDiagnostics(char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return -1;
    }

    return snprintf(buffer, buffer_size,
        "EtherCAT Bridge Diagnostics:\n"
        "  Status: 0x%02X (%s)\n"
        "  Enabled: %s\n"
        "  Update Counter: %lu\n"
        "  Update Rate: %lu Hz\n"
        "  Last Update: %lu ms\n"
        "  Sensor Data:\n"
        "    Temperature: %d (0.1°C)\n"
        "    Humidity: %u (0.1%%)\n"
        "    Pressure: %u (0.1kPa)\n"
        "    Accel X/Y/Z: %d/%d/%d (0.001m/s²)\n"
        "    Light: %u lux\n"
        "    Digital: SW1=%d, SW2=%d, Motion=%d, Alarm=%d\n",
        g_bridge_status,
        (g_bridge_status == BRIDGE_STATUS_OK) ? "OK" :
        (g_bridge_status == BRIDGE_STATUS_SENSOR_ERROR) ? "SENSOR_ERROR" :
        (g_bridge_status == BRIDGE_STATUS_MAPPING_ERROR) ? "MAPPING_ERROR" : "CONFIG_ERROR",
        g_bridge_enabled ? "Yes" : "No",
        g_update_counter,
        g_bridge_config.data_update_rate,
        g_last_update_time,
        g_sensor_inputs.temperature_x10,
        g_sensor_inputs.humidity_x10,
        g_sensor_inputs.pressure_x10,
        g_sensor_inputs.accel_x_x1000,
        g_sensor_inputs.accel_y_x1000,
        g_sensor_inputs.accel_z_x1000,
        g_sensor_inputs.light_intensity,
        g_sensor_inputs.switch_1 ? 1 : 0,
        g_sensor_inputs.switch_2 ? 1 : 0,
        g_sensor_inputs.motion_detected ? 1 : 0,
        g_sensor_inputs.alarm_status ? 1 : 0
    );
}

/**
 * @brief 执行传感器校准
 */
int EtherCAT_SensorBridge_CalibrateSensor(uint8_t sensor_id)
{
    if (sensor_id >= 7) {
        return -1;
    }

    /* 重置该传感器的统计数据 */
    g_sensor_stats[sensor_id].min_value = 999999.0f;
    g_sensor_stats[sensor_id].max_value = -999999.0f;
    g_sensor_stats[sensor_id].sum_value = 0.0f;
    g_sensor_stats[sensor_id].sample_count = 0;

    return 0;
}

/**
 * @brief 设置传感器参数
 */
int EtherCAT_SensorBridge_SetSensorParameter(uint8_t sensor_id, uint16_t parameter)
{
    if (sensor_id >= 7) {
        return -1;
    }

    /* 这里可以根据sensor_id和parameter设置相应的传感器参数 */
    /* 具体实现根据应用需求定制 */

    return 0;
}

/**
 * @brief 获取传感器统计信息
 */
int EtherCAT_SensorBridge_GetSensorStats(uint8_t sensor_id,
                                         float *min_value,
                                         float *max_value,
                                         float *avg_value)
{
    if (sensor_id >= 7 || min_value == NULL || max_value == NULL || avg_value == NULL) {
        return -1;
    }

    *min_value = g_sensor_stats[sensor_id].min_value;
    *max_value = g_sensor_stats[sensor_id].max_value;

    if (g_sensor_stats[sensor_id].sample_count > 0) {
        *avg_value = g_sensor_stats[sensor_id].sum_value / g_sensor_stats[sensor_id].sample_count;
    } else {
        *avg_value = 0.0f;
    }

    return 0;
}

/* ========================================================================== */
/* 私有函数实现 */
/* ========================================================================== */

/**
 * @brief 应用默认配置
 */
static void _apply_default_config(void)
{
    g_bridge_config.enable_temperature = true;
    g_bridge_config.enable_humidity = true;
    g_bridge_config.enable_pressure = true;
    g_bridge_config.enable_acceleration = true;
    g_bridge_config.enable_light = true;
    g_bridge_config.enable_digital_io = true;

    g_bridge_config.temp_scale = 10.0f;
    g_bridge_config.humidity_scale = 10.0f;
    g_bridge_config.pressure_scale = 10.0f;
    g_bridge_config.accel_scale = 1000.0f;
    g_bridge_config.light_scale = 1.0f;

    g_bridge_config.temp_offset = 0.0f;
    g_bridge_config.humidity_offset = 0.0f;
    g_bridge_config.pressure_offset = 0.0f;

    g_bridge_config.data_update_rate = 10; /* 10Hz */
}

/**
 * @brief 更新传感器统计数据
 */
static void _update_sensor_statistics(void)
{
    const SensorData_t *sensor_data = SensorSimulator_GetData();
    float values[7];
    int i;  /* 循环计数器 */

    if (sensor_data == NULL) {
        return;
    }

    /* 准备传感器数值数组 */
    values[0] = sensor_data->temperature;
    values[1] = sensor_data->humidity;
    values[2] = sensor_data->pressure;
    values[3] = sensor_data->acceleration_x;
    values[4] = sensor_data->acceleration_y;
    values[5] = sensor_data->acceleration_z;
    values[6] = sensor_data->light_intensity;

    /* 更新统计数据 */
    for (i = 0; i < 7; i++) {
        if (values[i] < g_sensor_stats[i].min_value) {
            g_sensor_stats[i].min_value = values[i];
        }
        if (values[i] > g_sensor_stats[i].max_value) {
            g_sensor_stats[i].max_value = values[i];
        }
        g_sensor_stats[i].sum_value += values[i];
        g_sensor_stats[i].sample_count++;

        /* 防止计数器溢出 */
        if (g_sensor_stats[i].sample_count > 1000000) {
            g_sensor_stats[i].sum_value /= 2.0f;
            g_sensor_stats[i].sample_count /= 2;
        }
    }
}

/**
 * @brief 转换传感器数据为EtherCAT格式
 */
static void _convert_sensor_data_to_ethercat(void)
{
    const SensorData_t *sensor_data = SensorSimulator_GetData();

    if (sensor_data == NULL) {
        return;
    }

    /* 数字信号直接映射 */
    g_sensor_inputs.switch_1 = sensor_data->switch_1;
    g_sensor_inputs.switch_2 = sensor_data->switch_2;
    g_sensor_inputs.motion_detected = sensor_data->motion_detected;
    g_sensor_inputs.alarm_status = sensor_data->alarm_status;

    /* 模拟信号转换为16位整数 */
    if (g_bridge_config.enable_temperature) {
        g_sensor_inputs.temperature_x10 = _float_to_int16_scaled(
            sensor_data->temperature + g_bridge_config.temp_offset,
            g_bridge_config.temp_scale);
    }

    if (g_bridge_config.enable_humidity) {
        g_sensor_inputs.humidity_x10 = _float_to_uint16_scaled(
            sensor_data->humidity + g_bridge_config.humidity_offset,
            g_bridge_config.humidity_scale);
    }

    if (g_bridge_config.enable_pressure) {
        g_sensor_inputs.pressure_x10 = _float_to_uint16_scaled(
            sensor_data->pressure + g_bridge_config.pressure_offset,
            g_bridge_config.pressure_scale);
    }

    if (g_bridge_config.enable_acceleration) {
        g_sensor_inputs.accel_x_x1000 = _float_to_int16_scaled(
            sensor_data->acceleration_x, g_bridge_config.accel_scale);
        g_sensor_inputs.accel_y_x1000 = _float_to_int16_scaled(
            sensor_data->acceleration_y, g_bridge_config.accel_scale);
        g_sensor_inputs.accel_z_x1000 = _float_to_int16_scaled(
            sensor_data->acceleration_z, g_bridge_config.accel_scale);
    }

    if (g_bridge_config.enable_light) {
        g_sensor_inputs.light_intensity = _float_to_uint16_scaled(
            sensor_data->light_intensity, g_bridge_config.light_scale);
    }

    /* 状态信息 */
    g_sensor_inputs.sensor_status = sensor_data->sensor_status;
    g_sensor_inputs.sequence_id = sensor_data->sequence_id;
}

/**
 * @brief 处理控制命令
 */
static void _process_control_commands(void)
{
    static uint8_t last_config_cmd = SENSOR_CMD_NOP;
    uint8_t i;  /* 循环计数器 */

    /* 检查传感器控制命令 */
    if (g_sensor_outputs.sensor_config_cmd != last_config_cmd) {
        switch (g_sensor_outputs.sensor_config_cmd) {
            case SENSOR_CMD_RESET:
                SensorSimulator_Reset();
                EtherCAT_SensorBridge_Reset();
                break;

            case SENSOR_CMD_CALIBRATE:
                /* 校准所有传感器 */
                for (i = 0; i < 7; i++) {
                    EtherCAT_SensorBridge_CalibrateSensor(i);
                }
                break;

            case SENSOR_CMD_INJECT_FAULT:
                SensorSimulator_InjectFault(0, SENSOR_STATUS_ERROR);
                break;

            case SENSOR_CMD_CLEAR_FAULT:
                for (uint8_t i = 0; i < 8; i++) {
                    SensorSimulator_ClearFault(i);
                }
                break;

            default:
                break;
        }
        last_config_cmd = g_sensor_outputs.sensor_config_cmd;
    }

    /* 处理传感器使能控制 */
    if (g_sensor_outputs.enable_sensor_sim) {
        if (!SensorSimulator_IsEnabled()) {
            SensorSimulator_Enable(true);
        }
    } else {
        if (SensorSimulator_IsEnabled()) {
            SensorSimulator_Enable(false);
        }
    }

    /* 处理重置命令 */
    if (g_sensor_outputs.reset_sensor_data) {
        SensorSimulator_Reset();
        g_sensor_outputs.reset_sensor_data = false; /* 单次命令 */
    }
}

/**
 * @brief 浮点数转有符号16位整数（带缩放）
 */
static int16_t _float_to_int16_scaled(float value, float scale)
{
    float scaled_value = value * scale;

    if (scaled_value > 32767.0f) {
        return 32767;
    } else if (scaled_value < -32768.0f) {
        return -32768;
    } else {
        return (int16_t)scaled_value;
    }
}

/**
 * @brief 浮点数转无符号16位整数（带缩放）
 */
static uint16_t _float_to_uint16_scaled(float value, float scale)
{
    float scaled_value = value * scale;

    if (scaled_value > 65535.0f) {
        return 65535;
    } else if (scaled_value < 0.0f) {
        return 0;
    } else {
        return (uint16_t)scaled_value;
    }
}

/**
 * @brief 获取系统时间（毫秒）
 */
static uint32_t _get_system_time_ms(void)
{
    #ifdef portTICK_PERIOD_MS
        return xTaskGetTickCount() * portTICK_PERIOD_MS;
    #else
        return HAL_GetTick();
    #endif
}