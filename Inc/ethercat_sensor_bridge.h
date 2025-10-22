/**
 * @file ethercat_sensor_bridge.h
 * @brief EtherCAT传感器数据桥接模块 - 头文件
 * @author Claude AI Assistant
 * @version 1.0.0
 * @date 2024-10-21
 *
 * @description
 * 本模块负责将传感器模拟数据映射到EtherCAT对象字典中，
 * 实现传感器数据与EtherCAT主站的通信桥接功能。
 */

#ifndef _ETHERCAT_SENSOR_BRIDGE_H_
#define _ETHERCAT_SENSOR_BRIDGE_H_

#include "main.h"
#include "sensor_simulator.h"
#include "SSC-Device.h"
#include <stdint.h>
#include <stdbool.h>

/* ========================================================================== */
/* 数据映射配置 */
/* ========================================================================== */

/**
 * @brief EtherCAT数据映射配置
 */
typedef struct {
    bool enable_temperature;    /**< 启用温度数据映射 */
    bool enable_humidity;       /**< 启用湿度数据映射 */
    bool enable_pressure;       /**< 启用压力数据映射 */
    bool enable_acceleration;   /**< 启用加速度数据映射 */
    bool enable_light;          /**< 启用光强数据映射 */
    bool enable_digital_io;     /**< 启用数字IO映射 */

    /* 数据缩放参数 */
    float temp_scale;           /**< 温度缩放系数 */
    float humidity_scale;       /**< 湿度缩放系数 */
    float pressure_scale;       /**< 压力缩放系数 */
    float accel_scale;          /**< 加速度缩放系数 */
    float light_scale;          /**< 光强缩放系数 */

    /* 数据偏移参数 */
    float temp_offset;          /**< 温度偏移量 */
    float humidity_offset;      /**< 湿度偏移量 */
    float pressure_offset;      /**< 压力偏移量 */

    /* 更新频率控制 */
    uint32_t data_update_rate;  /**< 数据更新频率(Hz) */
} EtherCATBridgeConfig_t;

/**
 * @brief 扩展的EtherCAT输入数据结构
 * @note 扩展原有的0x6000对象以支持更多传感器数据
 */
typedef struct {
    /* 原有数字输入 */
    bool switch_1;              /**< 数字开关1 */
    bool switch_2;              /**< 数字开关2 */

    /* 扩展的传感器数据 (转换为16位整数) */
    int16_t temperature_x10;    /**< 温度 × 10 (0.1°C精度) */
    uint16_t humidity_x10;      /**< 湿度 × 10 (0.1%精度) */
    uint16_t pressure_x10;      /**< 压力 × 10 (0.1kPa精度) */
    int16_t accel_x_x1000;      /**< X轴加速度 × 1000 (0.001m/s²精度) */
    int16_t accel_y_x1000;      /**< Y轴加速度 × 1000 */
    int16_t accel_z_x1000;      /**< Z轴加速度 × 1000 */
    uint16_t light_intensity;   /**< 光强度 (lux) */

    /* 状态信息 */
    bool motion_detected;       /**< 运动检测 */
    bool alarm_status;          /**< 报警状态 */
    uint8_t sensor_status;      /**< 传感器状态 */
    uint16_t sequence_id;       /**< 数据序列号 */
} EtherCAT_SensorInputs_t;

/**
 * @brief 扩展的EtherCAT输出数据结构
 * @note 扩展原有的0x7010对象以支持更多控制功能
 */
typedef struct {
    /* 原有数字输出 */
    bool led_1;                 /**< LED1控制 */
    bool led_2;                 /**< LED2控制 */

    /* 扩展的控制信号 */
    bool enable_sensor_sim;     /**< 启用传感器模拟 */
    bool reset_sensor_data;     /**< 重置传感器数据 */
    bool trigger_calibration;   /**< 触发校准 */
    uint8_t sensor_config_cmd;  /**< 传感器配置命令 */
    uint16_t config_parameter;  /**< 配置参数 */
} EtherCAT_SensorOutputs_t;

/* ========================================================================== */
/* 状态和错误代码 */
/* ========================================================================== */

#define BRIDGE_STATUS_OK            0x00    /**< 桥接正常 */
#define BRIDGE_STATUS_SENSOR_ERROR  0x01    /**< 传感器错误 */
#define BRIDGE_STATUS_MAPPING_ERROR 0x02    /**< 映射错误 */
#define BRIDGE_STATUS_CONFIG_ERROR  0x03    /**< 配置错误 */

/* 传感器配置命令 */
#define SENSOR_CMD_NOP              0x00    /**< 无操作 */
#define SENSOR_CMD_RESET            0x01    /**< 重置传感器 */
#define SENSOR_CMD_CALIBRATE        0x02    /**< 校准传感器 */
#define SENSOR_CMD_INJECT_FAULT     0x03    /**< 注入故障 */
#define SENSOR_CMD_CLEAR_FAULT      0x04    /**< 清除故障 */

/* ========================================================================== */
/* 函数声明 */
/* ========================================================================== */

/**
 * @brief 初始化EtherCAT传感器桥接模块
 * @param config 桥接配置参数，传入NULL使用默认配置
 * @return 初始化结果
 * @retval 0 成功
 * @retval -1 失败
 */
int EtherCAT_SensorBridge_Init(const EtherCATBridgeConfig_t *config);

/**
 * @brief 启动EtherCAT传感器桥接
 * @return 启动结果
 * @retval 0 成功
 * @retval -1 失败
 */
int EtherCAT_SensorBridge_Start(void);

/**
 * @brief 停止EtherCAT传感器桥接
 */
void EtherCAT_SensorBridge_Stop(void);

/**
 * @brief 更新传感器数据到EtherCAT输入对象
 * @description 将传感器数据映射到EtherCAT输入PDO对象
 */
void EtherCAT_SensorBridge_UpdateInputs(void);

/**
 * @brief 处理EtherCAT输出对象的控制命令
 * @description 处理从EtherCAT主站接收的控制命令
 */
void EtherCAT_SensorBridge_ProcessOutputs(void);

/**
 * @brief 获取当前桥接状态
 * @return 桥接状态代码
 */
uint8_t EtherCAT_SensorBridge_GetStatus(void);

/**
 * @brief 获取桥接配置
 * @return 指向当前桥接配置的指针
 */
const EtherCATBridgeConfig_t* EtherCAT_SensorBridge_GetConfig(void);

/**
 * @brief 设置桥接配置
 * @param config 新的桥接配置
 * @return 设置结果
 * @retval 0 成功
 * @retval -1 失败
 */
int EtherCAT_SensorBridge_SetConfig(const EtherCATBridgeConfig_t *config);

/**
 * @brief 重置桥接模块
 */
void EtherCAT_SensorBridge_Reset(void);

/**
 * @brief 获取扩展的传感器输入数据
 * @return 指向扩展传感器输入数据的指针
 */
const EtherCAT_SensorInputs_t* EtherCAT_SensorBridge_GetInputData(void);

/**
 * @brief 获取扩展的传感器输出数据
 * @return 指向扩展传感器输出数据的指针
 */
const EtherCAT_SensorOutputs_t* EtherCAT_SensorBridge_GetOutputData(void);

/**
 * @brief 诊断函数：获取桥接统计信息
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 生成的字符串长度
 */
int EtherCAT_SensorBridge_GetDiagnostics(char *buffer, size_t buffer_size);

/**
 * @brief 执行传感器校准
 * @param sensor_id 传感器ID
 * @return 校准结果
 * @retval 0 成功
 * @retval -1 失败
 */
int EtherCAT_SensorBridge_CalibrateSensor(uint8_t sensor_id);

/**
 * @brief 设置传感器参数
 * @param sensor_id 传感器ID
 * @param parameter 参数值
 * @return 设置结果
 * @retval 0 成功
 * @retval -1 失败
 */
int EtherCAT_SensorBridge_SetSensorParameter(uint8_t sensor_id, uint16_t parameter);

/**
 * @brief 获取传感器统计信息
 * @param sensor_id 传感器ID
 * @param min_value 输出最小值
 * @param max_value 输出最大值
 * @param avg_value 输出平均值
 * @return 获取结果
 * @retval 0 成功
 * @retval -1 失败
 */
int EtherCAT_SensorBridge_GetSensorStats(uint8_t sensor_id,
                                         float *min_value,
                                         float *max_value,
                                         float *avg_value);

#endif /* _ETHERCAT_SENSOR_BRIDGE_H_ */
