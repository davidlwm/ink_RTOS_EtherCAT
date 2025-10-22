/**
 * @file sensor_test.h
 * @brief 传感器模拟系统测试模块 - 头文件
 * @author Claude AI Assistant
 * @version 1.0.0
 * @date 2024-10-21
 *
 * @description
 * 提供传感器模拟系统的综合测试功能，包括：
 * - 单元测试
 * - 集成测试
 * - 性能测试
 * - 故障注入测试
 */

#ifndef _SENSOR_TEST_H_
#define _SENSOR_TEST_H_

#include "main.h"
#include "sensor_simulator.h"
#include "ethercat_sensor_bridge.h"
#include <stdint.h>
#include <stdbool.h>

/* ========================================================================== */
/* 测试结果结构 */
/* ========================================================================== */

/**
 * @brief 测试结果结构
 */
typedef struct {
    uint32_t tests_total;       /**< 总测试数 */
    uint32_t tests_passed;      /**< 通过测试数 */
    uint32_t tests_failed;      /**< 失败测试数 */
    uint32_t tests_skipped;     /**< 跳过测试数 */
    uint32_t execution_time_ms; /**< 执行时间(ms) */
    char last_error[128];       /**< 最后错误信息 */
} TestResults_t;

/**
 * @brief 性能测试结果
 */
typedef struct {
    uint32_t update_count;      /**< 更新次数 */
    uint32_t min_update_time;   /**< 最小更新时间(µs) */
    uint32_t max_update_time;   /**< 最大更新时间(µs) */
    uint32_t avg_update_time;   /**< 平均更新时间(µs) */
    uint32_t total_time_ms;     /**< 总测试时间(ms) */
} PerformanceResults_t;

/* ========================================================================== */
/* 测试配置 */
/* ========================================================================== */

#define TEST_DURATION_MS        10000   /**< 默认测试持续时间 */
#define TEST_SAMPLE_COUNT       1000    /**< 默认采样次数 */
#define TEST_TOLERANCE_PERCENT  5.0f    /**< 默认容差百分比 */

/* ========================================================================== */
/* 函数声明 */
/* ========================================================================== */

/**
 * @brief 运行所有测试
 * @param results 测试结果输出
 * @return 测试总体结果
 * @retval 0 所有测试通过
 * @retval -1 有测试失败
 */
int SensorTest_RunAllTests(TestResults_t *results);

/**
 * @brief 运行传感器模拟器单元测试
 * @param results 测试结果输出
 * @return 测试结果
 */
int SensorTest_UnitTest_Simulator(TestResults_t *results);

/**
 * @brief 运行EtherCAT桥接单元测试
 * @param results 测试结果输出
 * @return 测试结果
 */
int SensorTest_UnitTest_Bridge(TestResults_t *results);

/**
 * @brief 运行集成测试
 * @param results 测试结果输出
 * @return 测试结果
 */
int SensorTest_IntegrationTest(TestResults_t *results);

/**
 * @brief 运行性能测试
 * @param duration_ms 测试持续时间
 * @param perf_results 性能测试结果输出
 * @return 测试结果
 */
int SensorTest_PerformanceTest(uint32_t duration_ms, PerformanceResults_t *perf_results);

/**
 * @brief 运行故障注入测试
 * @param results 测试结果输出
 * @return 测试结果
 */
int SensorTest_FaultInjectionTest(TestResults_t *results);

/**
 * @brief 运行数据一致性测试
 * @param sample_count 采样次数
 * @param results 测试结果输出
 * @return 测试结果
 */
int SensorTest_DataConsistencyTest(uint32_t sample_count, TestResults_t *results);

/**
 * @brief 运行边界值测试
 * @param results 测试结果输出
 * @return 测试结果
 */
int SensorTest_BoundaryValueTest(TestResults_t *results);

/**
 * @brief 运行压力测试
 * @param duration_ms 测试持续时间
 * @param results 测试结果输出
 * @return 测试结果
 */
int SensorTest_StressTest(uint32_t duration_ms, TestResults_t *results);

/**
 * @brief 验证传感器数据范围
 * @param sensor_data 传感器数据
 * @return 验证结果
 * @retval true 数据在有效范围内
 * @retval false 数据超出范围
 */
bool SensorTest_ValidateDataRange(const SensorData_t *sensor_data);

/**
 * @brief 验证EtherCAT数据映射
 * @param sensor_data 原始传感器数据
 * @param ethercat_inputs EtherCAT输入数据
 * @param tolerance_percent 容差百分比
 * @return 验证结果
 */
bool SensorTest_ValidateDataMapping(const SensorData_t *sensor_data,
                                   const EtherCAT_SensorInputs_t *ethercat_inputs,
                                   float tolerance_percent);

/**
 * @brief 打印测试结果
 * @param results 测试结果
 * @param test_name 测试名称
 */
void SensorTest_PrintResults(const TestResults_t *results, const char *test_name);

/**
 * @brief 打印性能测试结果
 * @param perf_results 性能测试结果
 */
void SensorTest_PrintPerformanceResults(const PerformanceResults_t *perf_results);

/**
 * @brief 生成测试报告
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @param results 测试结果数组
 * @param num_tests 测试数量
 * @return 生成的字符串长度
 */
int SensorTest_GenerateReport(char *buffer, size_t buffer_size,
                             const TestResults_t *results, int num_tests);

/**
 * @brief 运行自动化回归测试
 * @description 运行预定义的回归测试套件
 * @return 测试结果
 * @retval 0 所有测试通过
 * @retval -1 有测试失败
 */
int SensorTest_RunRegressionTests(void);

/**
 * @brief 初始化测试环境
 * @return 初始化结果
 */
int SensorTest_Initialize(void);

/**
 * @brief 清理测试环境
 */
void SensorTest_Cleanup(void);

#endif /* _SENSOR_TEST_H_ */