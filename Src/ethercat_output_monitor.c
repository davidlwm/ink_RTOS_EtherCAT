/**
 * @file ethercat_output_monitor.c
 * @brief EtherCAT主站下发数据变化监控模块实现
 * @author Claude AI Assistant
 * @version 1.0.0
 * @date 2024-10-22
 */

#include "ethercat_output_monitor.h"
#include "SSC-Device.h"
#include "SSC-DeviceObjects.h"
#include <string.h>
#include <stdio.h>

/* ========================================================================== */
/* 私有变量 */
/* ========================================================================== */

static OutputDataCache_t g_output_cache;        /**< 输出数据缓存 */
static OutputMonitorStats_t g_monitor_stats;    /**< 监控统计信息 */
static uint16_t g_analog_threshold = 10;        /**< 模拟量变化阈值（千分比，默认1%） */
static bool g_monitor_initialized = false;      /**< 初始化标志 */

/* ========================================================================== */
/* 私有函数声明 */
/* ========================================================================== */

static bool _is_analog_changed(int16_t current, int16_t previous);
static uint32_t _get_timestamp(void);
static void _update_change_rate(void);

/* ========================================================================== */
/* 公共函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化输出监控模块
 */
void EtherCAT_OutputMonitor_Init(void)
{
    /* 清零数据缓存 */
    memset(&g_output_cache, 0, sizeof(g_output_cache));

    /* 清零统计信息 */
    memset(&g_monitor_stats, 0, sizeof(g_monitor_stats));

    /* 初始化缓存为当前输出状态 */
    // g_output_cache.digital_outputs_prev = Obj0x7011.digital_outputs;
    // g_output_cache.digital_output_mask_prev = Obj0x7011.digital_output_mask;

    // for (int i = 0; i < 4; i++) {
    //     g_output_cache.analog_outputs_prev[i] = Obj0x7012.channel[i];
    // }
    // g_output_cache.analog_output_mask_prev = Obj0x7012.analog_output_mask;

    // g_output_cache.sensor_config_cmd_prev = Obj0x7020.sensor_config_cmd;
    // g_output_cache.system_control_cmd_prev = Obj0x7020.system_control_cmd;
    // g_output_cache.sampling_rate_prev = Obj0x7030.sampling_rate;
    // g_output_cache.filter_enable_prev = Obj0x7030.filter_enable;

    g_monitor_stats.last_change_timestamp = _get_timestamp();
    g_monitor_initialized = true;

    printf("[OUTPUT_MONITOR] 输出监控模块初始化完成\r\n");
}

/**
 * @brief 检测主站下发数据是否有变化
 * @return 变化类型掩码，参见 OutputChangeType_t
 */
uint8_t EtherCAT_OutputMonitor_CheckChanges(void)
{
    if (!g_monitor_initialized) {
        return OUTPUT_CHANGE_NONE;
    }

    uint8_t changes = OUTPUT_CHANGE_NONE;

    // /* 检查数字输出变化 */
    // if ((Obj0x7011.digital_outputs != g_output_cache.digital_outputs_prev) ||
    //     (Obj0x7011.digital_output_mask != g_output_cache.digital_output_mask_prev)) {
    //     changes |= OUTPUT_CHANGE_DIGITAL;
    //     g_monitor_stats.digital_changes++;

    //     printf("[OUTPUT_MONITOR] 数字输出变化: 0x%04X -> 0x%04X (掩码: 0x%04X -> 0x%04X)\r\n",
    //            g_output_cache.digital_outputs_prev, Obj0x7011.digital_outputs,
    //            g_output_cache.digital_output_mask_prev, Obj0x7011.digital_output_mask);
    // }

    // /* 检查模拟输出变化 */
    // for (int i = 0; i < 4; i++) {
    //     if (_is_analog_changed(Obj0x7012.channel[i], g_output_cache.analog_outputs_prev[i])) {
    //         changes |= OUTPUT_CHANGE_ANALOG;
    //         break;
    //     }
    // }

    // if (Obj0x7012.analog_output_mask != g_output_cache.analog_output_mask_prev) {
    //     changes |= OUTPUT_CHANGE_ANALOG;
    // }

    // if (changes & OUTPUT_CHANGE_ANALOG) {
    //     g_monitor_stats.analog_changes++;
    //     printf("[OUTPUT_MONITOR] 模拟输出变化检测到\r\n");
    // }

    // /* 检查控制命令变化 */
    // if ((Obj0x7020.sensor_config_cmd != g_output_cache.sensor_config_cmd_prev) ||
    //     (Obj0x7020.system_control_cmd != g_output_cache.system_control_cmd_prev)) {
    //     changes |= OUTPUT_CHANGE_COMMAND;
    //     g_monitor_stats.command_changes++;

    //     printf("[OUTPUT_MONITOR] 控制命令变化: 传感器cmd %d->%d, 系统cmd %d->%d\r\n",
    //            g_output_cache.sensor_config_cmd_prev, Obj0x7020.sensor_config_cmd,
    //            g_output_cache.system_control_cmd_prev, Obj0x7020.system_control_cmd);
    // }

    // /* 检查配置参数变化 */
    // if ((Obj0x7030.sampling_rate != g_output_cache.sampling_rate_prev) ||
    //     (Obj0x7030.filter_enable != g_output_cache.filter_enable_prev)) {
    //     changes |= OUTPUT_CHANGE_CONFIG;

    //     printf("[OUTPUT_MONITOR] 配置参数变化: 采样率 %d->%d, 滤波 %d->%d\r\n",
    //            g_output_cache.sampling_rate_prev, Obj0x7030.sampling_rate,
    //            g_output_cache.filter_enable_prev, Obj0x7030.filter_enable);
    // }

    /* 更新统计信息 */
    g_monitor_stats.total_updates++;

    if (changes != OUTPUT_CHANGE_NONE) {
        g_monitor_stats.last_change_timestamp = _get_timestamp();
    } else {
        g_monitor_stats.skipped_updates++;
    }

    _update_change_rate();

    return changes;
}

/**
 * @brief 更新输出数据缓存
 * @param force_update 强制更新（用于心跳或周期性检查）
 */
void EtherCAT_OutputMonitor_UpdateCache(bool force_update)
{
    if (!g_monitor_initialized) {
        return;
    }

    /* 更新数字输出缓存 */
    // g_output_cache.digital_outputs_prev = Obj0x7011.digital_outputs;
    // g_output_cache.digital_output_mask_prev = Obj0x7011.digital_output_mask;

    // /* 更新模拟输出缓存 */
    // for (int i = 0; i < 4; i++) {
    //     g_output_cache.analog_outputs_prev[i] = Obj0x7012.channel[i];
    // }
    // g_output_cache.analog_output_mask_prev = Obj0x7012.analog_output_mask;

    // /* 更新控制命令缓存 */
    // g_output_cache.sensor_config_cmd_prev = Obj0x7020.sensor_config_cmd;
    // g_output_cache.system_control_cmd_prev = Obj0x7020.system_control_cmd;

    // /* 更新配置参数缓存 */
    // g_output_cache.sampling_rate_prev = Obj0x7030.sampling_rate;
    // g_output_cache.filter_enable_prev = Obj0x7030.filter_enable;

    if (force_update) {
        printf("[OUTPUT_MONITOR] 强制更新缓存完成\r\n");
    }
}

/**
 * @brief 获取监控统计信息
 * @param stats 统计信息输出
 */
void EtherCAT_OutputMonitor_GetStats(OutputMonitorStats_t *stats)
{
    if (stats && g_monitor_initialized) {
        memcpy(stats, &g_monitor_stats, sizeof(OutputMonitorStats_t));
    }
}

/**
 * @brief 重置监控统计信息
 */
void EtherCAT_OutputMonitor_ResetStats(void)
{
    memset(&g_monitor_stats, 0, sizeof(g_monitor_stats));
    g_monitor_stats.last_change_timestamp = _get_timestamp();
    printf("[OUTPUT_MONITOR] 统计信息已重置\r\n");
}

/**
 * @brief 设置模拟量变化阈值
 * @param threshold 阈值（0-1000，表示千分比）
 */
void EtherCAT_OutputMonitor_SetAnalogThreshold(uint16_t threshold)
{
    if (threshold <= 1000) {
        g_analog_threshold = threshold;
        printf("[OUTPUT_MONITOR] 模拟量阈值设置为: %d‰\r\n", threshold);
    }
}

/**
 * @brief 检查是否需要强制更新
 * @param max_interval_ms 最大静默间隔（毫秒）
 * @return true: 需要强制更新, false: 不需要
 */
bool EtherCAT_OutputMonitor_NeedForceUpdate(uint32_t max_interval_ms)
{
    if (!g_monitor_initialized) {
        return false;
    }

    uint32_t current_time = _get_timestamp();
    uint32_t elapsed = current_time - g_monitor_stats.last_change_timestamp;

    return (elapsed > max_interval_ms);
}

/**
 * @brief 打印监控统计信息（调试用）
 */
void EtherCAT_OutputMonitor_PrintStats(void)
{
    if (!g_monitor_initialized) {
        printf("[OUTPUT_MONITOR] 监控未初始化\r\n");
        return;
    }

    printf("========== EtherCAT输出监控统计 ==========\r\n");
    printf("总更新次数:     %lu\r\n", g_monitor_stats.total_updates);
    printf("数字输出变化:   %lu\r\n", g_monitor_stats.digital_changes);
    printf("模拟输出变化:   %lu\r\n", g_monitor_stats.analog_changes);
    printf("命令变化:       %lu\r\n", g_monitor_stats.command_changes);
    printf("跳过更新:       %lu\r\n", g_monitor_stats.skipped_updates);
    printf("变化率:         %d%%\r\n", g_monitor_stats.change_rate_percent);
    printf("最后变化时间:   %lu ms\r\n", g_monitor_stats.last_change_timestamp);
    printf("模拟量阈值:     %d‰\r\n", g_analog_threshold);
    printf("==========================================\r\n");
}

/* ========================================================================== */
/* 私有函数实现 */
/* ========================================================================== */

/**
 * @brief 检查模拟量是否发生显著变化
 * @param current 当前值
 * @param previous 上次值
 * @return true: 有显著变化, false: 无显著变化
 */
static bool _is_analog_changed(int16_t current, int16_t previous)
{
    /* 计算绝对变化量 */
    int32_t delta = abs(current - previous);

    /* 如果上次值为0，直接比较当前值是否非零 */
    if (previous == 0) {
        return (current != 0);
    }

    /* 计算相对变化率（千分比） */
    uint32_t relative_change = (delta * 1000) / abs(previous);

    return (relative_change >= g_analog_threshold);
}

/**
 * @brief 获取系统时间戳
 * @return 时间戳（毫秒）
 */
static uint32_t _get_timestamp(void)
{
    return HAL_GetTick();
}

/**
 * @brief 更新变化率统计
 */
static void _update_change_rate(void)
{
    if (g_monitor_stats.total_updates > 0) {
        uint32_t actual_changes = g_monitor_stats.total_updates - g_monitor_stats.skipped_updates;
        g_monitor_stats.change_rate_percent = (actual_changes * 100) / g_monitor_stats.total_updates;
    }
}
