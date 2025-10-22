/**
 * @file ethercat_output_monitor.h
 * @brief EtherCAT主站下发数据变化监控模块
 * @author Claude AI Assistant
 * @version 1.0.0
 * @date 2024-10-22
 */

#ifndef _ETHERCAT_OUTPUT_MONITOR_H_
#define _ETHERCAT_OUTPUT_MONITOR_H_

#include "stm32f4xx_hal.h"
#include "ecat_def.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* 类型定义 */
/* ========================================================================== */

/**
 * @brief 输出数据变化类型
 */
typedef enum {
    OUTPUT_CHANGE_NONE = 0,          /**< 无变化 */
    OUTPUT_CHANGE_DIGITAL = 1,       /**< 数字输出变化 */
    OUTPUT_CHANGE_ANALOG = 2,        /**< 模拟输出变化 */
    OUTPUT_CHANGE_COMMAND = 4,       /**< 控制命令变化 */
    OUTPUT_CHANGE_CONFIG = 8         /**< 配置参数变化 */
} OutputChangeType_t;

/**
 * @brief 输出监控统计信息
 */
typedef struct {
    uint32_t total_updates;              /**< 总更新次数 */
    uint32_t digital_changes;            /**< 数字输出变化次数 */
    uint32_t analog_changes;             /**< 模拟输出变化次数 */
    uint32_t command_changes;            /**< 命令变化次数 */
    uint32_t skipped_updates;            /**< 跳过的更新次数（无变化） */
    uint32_t last_change_timestamp;      /**< 最后变化时间戳 */
    uint16_t change_rate_percent;        /**< 变化率百分比 */
} OutputMonitorStats_t;

/**
 * @brief 输出数据缓存结构
 */
typedef struct {
    /* 数字输出缓存 */
    uint16_t digital_outputs_prev;       /**< 上次数字输出状态 */
    uint16_t digital_output_mask_prev;   /**< 上次数字输出掩码 */

    /* 模拟输出缓存 */
    int16_t analog_outputs_prev[4];      /**< 上次模拟输出值 */
    uint8_t analog_output_mask_prev;     /**< 上次模拟输出掩码 */

    /* 控制命令缓存 */
    uint8_t sensor_config_cmd_prev;      /**< 上次传感器配置命令 */
    uint8_t system_control_cmd_prev;     /**< 上次系统控制命令 */

    /* 配置参数缓存 */
    uint16_t sampling_rate_prev;         /**< 上次采样率 */
    uint8_t filter_enable_prev;          /**< 上次滤波使能 */
} OutputDataCache_t;

/* ========================================================================== */
/* 函数声明 */
/* ========================================================================== */

/**
 * @brief 初始化输出监控模块
 */
void EtherCAT_OutputMonitor_Init(void);

/**
 * @brief 检测主站下发数据是否有变化
 * @return 变化类型掩码，参见 OutputChangeType_t
 */
uint8_t EtherCAT_OutputMonitor_CheckChanges(void);

/**
 * @brief 更新输出数据缓存
 * @param force_update 强制更新（用于心跳或周期性检查）
 */
void EtherCAT_OutputMonitor_UpdateCache(bool force_update);

/**
 * @brief 获取监控统计信息
 * @param stats 统计信息输出
 */
void EtherCAT_OutputMonitor_GetStats(OutputMonitorStats_t *stats);

/**
 * @brief 重置监控统计信息
 */
void EtherCAT_OutputMonitor_ResetStats(void);

/**
 * @brief 设置模拟量变化阈值
 * @param threshold 阈值（0-1000，表示千分比）
 */
void EtherCAT_OutputMonitor_SetAnalogThreshold(uint16_t threshold);

/**
 * @brief 检查是否需要强制更新
 * @param max_interval_ms 最大静默间隔（毫秒）
 * @return true: 需要强制更新, false: 不需要
 */
bool EtherCAT_OutputMonitor_NeedForceUpdate(uint32_t max_interval_ms);

/**
 * @brief 打印监控统计信息（调试用）
 */
void EtherCAT_OutputMonitor_PrintStats(void);

#ifdef __cplusplus
}
#endif

#endif /* _ETHERCAT_OUTPUT_MONITOR_H_ */
