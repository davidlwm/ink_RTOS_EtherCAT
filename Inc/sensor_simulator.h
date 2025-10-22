/**
 * @file sensor_simulator.h
 * @brief 传感器数据模拟器 - 头文件
 * @author Claude AI Assistant
 * @version 1.0.0
 * @date 2024-10-21
 *
 * @description
 * 本模块提供多种传感器类型的数据模拟功能，包括：
 * - 温度传感器
 * - 湿度传感器
 * - 压力传感器
 * - 加速度传感器
 * - 光强传感器
 * - 数字开关状态
 *
 * 模拟数据使用数学函数生成，提供真实的传感器特性模拟
 */

#ifndef _SENSOR_SIMULATOR_H_
#define _SENSOR_SIMULATOR_H_

#include "main.h"
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

/* ========================================================================== */
/* 传感器类型定义 */
/* ========================================================================== */

/**
 * @brief 传感器数据结构
 */
typedef struct {
    /* 模拟传感器数据 */
    float temperature;      /**< 温度 (°C) */
    float humidity;         /**< 湿度 (%) */
    float pressure;         /**< 压力 (kPa) */
    float acceleration_x;   /**< X轴加速度 (m/s²) */
    float acceleration_y;   /**< Y轴加速度 (m/s²) */
    float acceleration_z;   /**< Z轴加速度 (m/s²) */
    float light_intensity;  /**< 光强 (lux) */

    /* 数字信号 */
    bool switch_1;          /**< 数字开关1状态 */
    bool switch_2;          /**< 数字开关2状态 */
    bool motion_detected;   /**< 运动检测 */
    bool alarm_status;      /**< 报警状态 */

    /* 系统状态 */
    uint32_t timestamp;     /**< 时间戳 */
    uint16_t sequence_id;   /**< 序列号 */
    uint8_t sensor_status;  /**< 传感器状态 */
} SensorData_t;

/**
 * @brief 传感器配置参数
 */
typedef struct {
    /* 温度传感器配置 */
    float temp_base;        /**< 基础温度 */
    float temp_amplitude;   /**< 温度波动幅度 */
    float temp_frequency;   /**< 温度变化频率 */

    /* 湿度传感器配置 */
    float humidity_base;    /**< 基础湿度 */
    float humidity_amplitude; /**< 湿度波动幅度 */
    float humidity_frequency; /**< 湿度变化频率 */

    /* 压力传感器配置 */
    float pressure_base;    /**< 基础压力 */
    float pressure_amplitude; /**< 压力波动幅度 */
    float pressure_frequency; /**< 压力变化频率 */

    /* 加速度传感器配置 */
    float accel_noise_level; /**< 加速度噪声水平 */

    /* 光强传感器配置 */
    float light_base;       /**< 基础光强 */
    float light_amplitude;  /**< 光强波动幅度 */

    /* 更新频率 */
    uint32_t update_period_ms; /**< 数据更新周期(ms) */
} SensorConfig_t;

/* ========================================================================== */
/* 默认配置常量 */
/* ========================================================================== */

/* 默认传感器参数 */
#define DEFAULT_TEMP_BASE           25.0f     /**< 默认基础温度 25°C */
#define DEFAULT_TEMP_AMPLITUDE      10.0f     /**< 默认温度波动 ±10°C */
#define DEFAULT_TEMP_FREQUENCY      0.1f      /**< 默认温度变化频率 */

#define DEFAULT_HUMIDITY_BASE       50.0f     /**< 默认基础湿度 50% */
#define DEFAULT_HUMIDITY_AMPLITUDE  20.0f     /**< 默认湿度波动 ±20% */
#define DEFAULT_HUMIDITY_FREQUENCY  0.05f     /**< 默认湿度变化频率 */

#define DEFAULT_PRESSURE_BASE       101.3f    /**< 默认基础压力 101.3kPa */
#define DEFAULT_PRESSURE_AMPLITUDE  5.0f      /**< 默认压力波动 ±5kPa */
#define DEFAULT_PRESSURE_FREQUENCY  0.02f     /**< 默认压力变化频率 */

#define DEFAULT_ACCEL_NOISE         0.1f      /**< 默认加速度噪声水平 */

#define DEFAULT_LIGHT_BASE          500.0f    /**< 默认基础光强 500lux */
#define DEFAULT_LIGHT_AMPLITUDE     300.0f    /**< 默认光强波动 ±300lux */

#define DEFAULT_UPDATE_PERIOD       100       /**< 默认更新周期 100ms */

/* ========================================================================== */
/* 状态定义 */
/* ========================================================================== */

#define SENSOR_STATUS_OK            0x00      /**< 传感器正常 */
#define SENSOR_STATUS_WARNING       0x01      /**< 传感器警告 */
#define SENSOR_STATUS_ERROR         0x02      /**< 传感器错误 */
#define SENSOR_STATUS_OFFLINE       0x03      /**< 传感器离线 */

/* ========================================================================== */
/* 函数声明 */
/* ========================================================================== */

/**
 * @brief 初始化传感器模拟器
 * @param config 传感器配置参数，传入NULL使用默认配置
 * @return 初始化结果
 * @retval 0 成功
 * @retval -1 失败
 */
int SensorSimulator_Init(const SensorConfig_t *config);

/**
 * @brief 更新传感器数据
 * @description 根据当前时间和配置参数计算新的传感器数据
 */
void SensorSimulator_Update(void);

/**
 * @brief 获取当前传感器数据
 * @return 指向当前传感器数据的指针
 */
const SensorData_t* SensorSimulator_GetData(void);

/**
 * @brief 获取传感器配置
 * @return 指向当前传感器配置的指针
 */
const SensorConfig_t* SensorSimulator_GetConfig(void);

/**
 * @brief 设置传感器配置
 * @param config 新的传感器配置
 * @return 设置结果
 * @retval 0 成功
 * @retval -1 失败
 */
int SensorSimulator_SetConfig(const SensorConfig_t *config);

/**
 * @brief 重置传感器模拟器
 * @description 重置所有状态和计数器
 */
void SensorSimulator_Reset(void);

/**
 * @brief 启用/禁用传感器模拟
 * @param enable true=启用, false=禁用
 */
void SensorSimulator_Enable(bool enable);

/**
 * @brief 获取传感器模拟器状态
 * @return true=已启用, false=已禁用
 */
bool SensorSimulator_IsEnabled(void);

/**
 * @brief 模拟传感器故障
 * @param sensor_id 传感器ID (0-6对应不同传感器)
 * @param fault_type 故障类型 (WARNING/ERROR/OFFLINE)
 */
void SensorSimulator_InjectFault(uint8_t sensor_id, uint8_t fault_type);

/**
 * @brief 清除传感器故障
 * @param sensor_id 传感器ID
 */
void SensorSimulator_ClearFault(uint8_t sensor_id);

/**
 * @brief 获取传感器数据的JSON格式字符串（调试用）
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 生成的字符串长度
 */
int SensorSimulator_GetDataString(char *buffer, size_t buffer_size);

#endif /* _SENSOR_SIMULATOR_H_ */