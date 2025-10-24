/**
 * @file sensor_simulator.c
 * @brief 传感器数据模拟器 - 实现文件
 * @author Claude AI Assistant
 * @version 1.0.0
 * @date 2024-10-21
 */

#include "sensor_simulator.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* 如果编译器没有定义M_PI，则手动定义 */
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* ========================================================================== */
/* 私有变量 */
/* ========================================================================== */

static SensorData_t g_sensor_data;           /**< 当前传感器数据 */
static SensorConfig_t g_sensor_config;       /**< 传感器配置 */
static bool g_simulator_enabled = false;     /**< 模拟器启用状态 */
static uint32_t g_last_update_time = 0;      /**< 上次更新时间 */
static uint32_t g_simulation_time = 0;       /**< 模拟运行时间 */
static uint8_t g_fault_mask = 0;             /**< 故障掩码 */

/* ========================================================================== */
/* 私有函数声明 */
/* ========================================================================== */

static void _generate_temperature_data(void);
static void _generate_humidity_data(void);
static void _generate_pressure_data(void);
static void _generate_acceleration_data(void);
static void _generate_light_data(void);
static void _generate_digital_signals(void);
static void _update_system_status(void);
static float _generate_noise(float amplitude);
static uint32_t _get_system_time_ms(void);

/* ========================================================================== */
/* 公共函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化传感器模拟器
 */
int SensorSimulator_Init(const SensorConfig_t *config)
{
    /* 清零数据结构 */
    memset(&g_sensor_data, 0, sizeof(SensorData_t));
    memset(&g_sensor_config, 0, sizeof(SensorConfig_t));

    /* 设置默认配置或使用用户配置 */
    if (config == NULL) {
        /* 使用默认配置 */
        g_sensor_config.temp_base = DEFAULT_TEMP_BASE;
        g_sensor_config.temp_amplitude = DEFAULT_TEMP_AMPLITUDE;
        g_sensor_config.temp_frequency = DEFAULT_TEMP_FREQUENCY;

        g_sensor_config.humidity_base = DEFAULT_HUMIDITY_BASE;
        g_sensor_config.humidity_amplitude = DEFAULT_HUMIDITY_AMPLITUDE;
        g_sensor_config.humidity_frequency = DEFAULT_HUMIDITY_FREQUENCY;

        g_sensor_config.pressure_base = DEFAULT_PRESSURE_BASE;
        g_sensor_config.pressure_amplitude = DEFAULT_PRESSURE_AMPLITUDE;
        g_sensor_config.pressure_frequency = DEFAULT_PRESSURE_FREQUENCY;

        g_sensor_config.accel_noise_level = DEFAULT_ACCEL_NOISE;

        g_sensor_config.light_base = DEFAULT_LIGHT_BASE;
        g_sensor_config.light_amplitude = DEFAULT_LIGHT_AMPLITUDE;

        g_sensor_config.update_period_ms = DEFAULT_UPDATE_PERIOD;
    } else {
        /* 使用用户配置 */
        memcpy(&g_sensor_config, config, sizeof(SensorConfig_t));
    }

    /* 初始化状态变量 */
    g_simulator_enabled = false;
    g_last_update_time = _get_system_time_ms();
    g_simulation_time = 0;
    g_fault_mask = 0;

    /* 初始化传感器数据为合理的初始值 */
    g_sensor_data.temperature = g_sensor_config.temp_base;
    g_sensor_data.humidity = g_sensor_config.humidity_base;
    g_sensor_data.pressure = g_sensor_config.pressure_base;
    g_sensor_data.acceleration_x = 0.0f;
    g_sensor_data.acceleration_y = 0.0f;
    g_sensor_data.acceleration_z = 9.8f; /* 重力加速度 */
    g_sensor_data.light_intensity = g_sensor_config.light_base;
    g_sensor_data.switch_1 = false;
    g_sensor_data.switch_2 = false;
    g_sensor_data.motion_detected = false;
    g_sensor_data.alarm_status = false;
    g_sensor_data.timestamp = g_last_update_time;
    g_sensor_data.sequence_id = 0;
    g_sensor_data.sensor_status = SENSOR_STATUS_OK;

    return 0;
}

/**
 * @brief 更新传感器数据
 */
void SensorSimulator_Update(void)
{
    uint32_t current_time;
    uint32_t elapsed_time;

    if (!g_simulator_enabled) {
        return;
    }

    current_time = _get_system_time_ms();
    elapsed_time = current_time - g_last_update_time;

    /* 检查是否需要更新 */
    if (elapsed_time < g_sensor_config.update_period_ms) {
        return;
    }

    /* 更新模拟时间 */
    g_simulation_time += elapsed_time;
    g_last_update_time = current_time;

    /* 生成各类传感器数据 */
    _generate_temperature_data();
    _generate_humidity_data();
    _generate_pressure_data();
    _generate_acceleration_data();
    _generate_light_data();
    _generate_digital_signals();
    _update_system_status();

    /* 更新时间戳和序列号 */
    g_sensor_data.timestamp = current_time;
    g_sensor_data.sequence_id++;
}

/**
 * @brief 获取当前传感器数据
 */
const SensorData_t* SensorSimulator_GetData(void)
{
    return &g_sensor_data;
}

/**
 * @brief 获取传感器配置
 */
const SensorConfig_t* SensorSimulator_GetConfig(void)
{
    return &g_sensor_config;
}

/**
 * @brief 设置传感器配置
 */
int SensorSimulator_SetConfig(const SensorConfig_t *config)
{
    if (config == NULL) {
        return -1;
    }

    memcpy(&g_sensor_config, config, sizeof(SensorConfig_t));
    return 0;
}

/**
 * @brief 重置传感器模拟器
 */
void SensorSimulator_Reset(void)
{
    g_simulation_time = 0;
    g_last_update_time = _get_system_time_ms();
    g_sensor_data.sequence_id = 0;
    g_fault_mask = 0;
    g_sensor_data.sensor_status = SENSOR_STATUS_OK;
}

/**
 * @brief 启用/禁用传感器模拟
 */
void SensorSimulator_Enable(bool enable)
{
    g_simulator_enabled = enable;
    if (enable) {
        g_last_update_time = _get_system_time_ms();
    }
}

/**
 * @brief 获取传感器模拟器状态
 */
bool SensorSimulator_IsEnabled(void)
{
    return g_simulator_enabled;
}

/**
 * @brief 模拟传感器故障
 */
void SensorSimulator_InjectFault(uint8_t sensor_id, uint8_t fault_type)
{
    if (sensor_id < 8) {
        g_fault_mask |= (1 << sensor_id);
        g_sensor_data.sensor_status = fault_type;
    }
}

/**
 * @brief 清除传感器故障
 */
void SensorSimulator_ClearFault(uint8_t sensor_id)
{
    if (sensor_id < 8) {
        g_fault_mask &= ~(1 << sensor_id);
        if (g_fault_mask == 0) {
            g_sensor_data.sensor_status = SENSOR_STATUS_OK;
        }
    }
}

/**
 * @brief 获取传感器数据的字符串表示（调试用）
 */
int SensorSimulator_GetDataString(char *buffer, size_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return -1;
    }

    return snprintf(buffer, buffer_size,
        "Sensor Data [ID:%d, Time:%lu]:\n"
        "  Temperature: %.2f°C\n"
        "  Humidity: %.1f%%\n"
        "  Pressure: %.2f kPa\n"
        "  Acceleration: X=%.3f, Y=%.3f, Z=%.3f m/s²\n"
        "  Light: %.1f lux\n"
        "  Switches: SW1=%d, SW2=%d\n"
        "  Status: Motion=%d, Alarm=%d, Status=0x%02X\n",
        g_sensor_data.sequence_id,
        g_sensor_data.timestamp,
        g_sensor_data.temperature,
        g_sensor_data.humidity,
        g_sensor_data.pressure,
        g_sensor_data.acceleration_x,
        g_sensor_data.acceleration_y,
        g_sensor_data.acceleration_z,
        g_sensor_data.light_intensity,
        g_sensor_data.switch_1 ? 1 : 0,
        g_sensor_data.switch_2 ? 1 : 0,
        g_sensor_data.motion_detected ? 1 : 0,
        g_sensor_data.alarm_status ? 1 : 0,
        g_sensor_data.sensor_status
    );
}

/* ========================================================================== */
/* 私有函数实现 */
/* ========================================================================== */

/**
 * @brief 生成温度数据
 */
static void _generate_temperature_data(void)
{
    float time_factor = (float)g_simulation_time / 1000.0f; /* 转换为秒 */
    float sine_wave = sinf(2.0f * M_PI * g_sensor_config.temp_frequency * time_factor);
    float noise = _generate_noise(0.5f);

    g_sensor_data.temperature = g_sensor_config.temp_base +
                               g_sensor_config.temp_amplitude * sine_wave +
                               noise;

    /* 温度范围限制 */
    if (g_sensor_data.temperature < -40.0f) {
        g_sensor_data.temperature = -40.0f;
    } else if (g_sensor_data.temperature > 85.0f) {
        g_sensor_data.temperature = 85.0f;
    }
}

/**
 * @brief 生成湿度数据
 */
static void _generate_humidity_data(void)
{
    float time_factor = (float)g_simulation_time / 1000.0f;
    float sine_wave = sinf(2.0f * M_PI * g_sensor_config.humidity_frequency * time_factor);
    float noise = _generate_noise(1.0f);

    g_sensor_data.humidity = g_sensor_config.humidity_base +
                            g_sensor_config.humidity_amplitude * sine_wave +
                            noise;

    /* 湿度范围限制 0-100% */
    if (g_sensor_data.humidity < 0.0f) {
        g_sensor_data.humidity = 0.0f;
    } else if (g_sensor_data.humidity > 100.0f) {
        g_sensor_data.humidity = 100.0f;
    }
}

/**
 * @brief 生成压力数据
 */
static void _generate_pressure_data(void)
{
    float time_factor = (float)g_simulation_time / 1000.0f;
    float sine_wave = sinf(2.0f * M_PI * g_sensor_config.pressure_frequency * time_factor);
    float noise = _generate_noise(0.2f);

    g_sensor_data.pressure = g_sensor_config.pressure_base +
                            g_sensor_config.pressure_amplitude * sine_wave +
                            noise;

    /* 压力范围限制 80-120kPa */
    if (g_sensor_data.pressure < 80.0f) {
        g_sensor_data.pressure = 80.0f;
    } else if (g_sensor_data.pressure > 120.0f) {
        g_sensor_data.pressure = 120.0f;
    }
}

/**
 * @brief 生成加速度数据
 */
static void _generate_acceleration_data(void)
{
    float time_factor = (float)g_simulation_time / 1000.0f;

    /* X轴：模拟振动 */
    g_sensor_data.acceleration_x = _generate_noise(g_sensor_config.accel_noise_level) +
                                  0.1f * sinf(10.0f * time_factor);

    /* Y轴：模拟振动 */
    g_sensor_data.acceleration_y = _generate_noise(g_sensor_config.accel_noise_level) +
                                  0.1f * cosf(8.0f * time_factor);

    /* Z轴：重力加速度 + 噪声 */
    g_sensor_data.acceleration_z = 9.8f + _generate_noise(g_sensor_config.accel_noise_level);
}

/**
 * @brief 生成光强数据
 */
static void _generate_light_data(void)
{
    float time_factor = (float)g_simulation_time / 1000.0f;

    /* 模拟日光变化（周期约为24小时，这里缩短为2分钟演示） */
    float day_cycle = sinf(2.0f * M_PI * time_factor / 120.0f); /* 2分钟周期 */
    float noise = _generate_noise(50.0f);

    /* 确保非负值 */
    if (day_cycle < 0) {
        day_cycle = 0.1f; /* 夜间最小光强 */
    }

    g_sensor_data.light_intensity = g_sensor_config.light_base +
                                   g_sensor_config.light_amplitude * day_cycle +
                                   noise;

    /* 光强范围限制 */
    if (g_sensor_data.light_intensity < 0.0f) {
        g_sensor_data.light_intensity = 0.0f;
    } else if (g_sensor_data.light_intensity > 100000.0f) {
        g_sensor_data.light_intensity = 100000.0f;
    }
}

/**
 * @brief 生成数字信号
 */
static void _generate_digital_signals(void)
{
    uint32_t time_seconds = g_simulation_time / 1000;

    /* 开关1：每5秒切换一次 */
    g_sensor_data.switch_1 = (time_seconds / 5) % 2;

    /* 开关2：每3秒切换一次 */
    g_sensor_data.switch_2 = (time_seconds / 3) % 2;

    /* 运动检测：随机触发 */
    static uint32_t motion_timer = 0;
    motion_timer += g_sensor_config.update_period_ms;
    if (motion_timer > 10000) { /* 每10秒检查一次 */
        motion_timer = 0;
        g_sensor_data.motion_detected = (rand() % 4) == 0; /* 25%概率 */
    }

    /* 报警状态：温度过高或压力异常时触发 */
    g_sensor_data.alarm_status = (g_sensor_data.temperature > 60.0f) ||
                                (g_sensor_data.pressure < 90.0f) ||
                                (g_sensor_data.pressure > 110.0f);
}

/**
 * @brief 更新系统状态
 */
static void _update_system_status(void)
{
    /* 根据故障掩码和传感器值确定状态 */
    if (g_fault_mask != 0) {
        /* 有注入的故障 */
        return; /* 保持故障状态 */
    }

    /* 检查传感器数据是否异常 */
    if (g_sensor_data.temperature > 70.0f ||
        g_sensor_data.humidity > 95.0f ||
        g_sensor_data.pressure < 85.0f ||
        g_sensor_data.pressure > 115.0f) {
        g_sensor_data.sensor_status = SENSOR_STATUS_WARNING;
    } else if (g_sensor_data.temperature > 80.0f ||
               g_sensor_data.pressure < 80.0f ||
               g_sensor_data.pressure > 120.0f) {
        g_sensor_data.sensor_status = SENSOR_STATUS_ERROR;
    } else {
        g_sensor_data.sensor_status = SENSOR_STATUS_OK;
    }
}

/**
 * @brief 生成噪声
 */
static float _generate_noise(float amplitude)
{
    /* 简单的伪随机噪声生成 */
    static uint32_t seed = 12345;
    seed = seed * 1103515245 + 12345; /* Linear congruential generator */

    float normalized = (float)(seed & 0x7FFFFFFF) / (float)0x7FFFFFFF;
    return amplitude * (normalized - 0.5f) * 2.0f;
}

/**
 * @brief 获取系统时间（毫秒）
 */
static uint32_t _get_system_time_ms(void)
{
    /* 在FreeRTOS环境中使用系统时钟 */
    #ifdef portTICK_PERIOD_MS
        return xTaskGetTickCount() * portTICK_PERIOD_MS;
    #else
        /* 备用方案：使用HAL时钟 */
        return HAL_GetTick();
    #endif
}