/**
 * @file sensor_test.c
 * @brief 传感器模拟系统测试模块 - 实现文件
 * @author Claude AI Assistant
 * @version 1.0.0
 * @date 2024-10-21
 */

#include "sensor_test.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ========================================================================== */
/* 私有变量 */
/* ========================================================================== */

static bool g_test_initialized = false;
static uint32_t g_test_start_time = 0;

/* ========================================================================== */
/* 私有函数声明 */
/* ========================================================================== */

static void _init_test_results(TestResults_t *results);
static void _record_test_result(TestResults_t *results, bool passed, const char *error_msg);
static uint32_t _get_time_ms(void);
static uint32_t _get_time_us(void);
static bool _is_float_equal(float a, float b, float tolerance_percent);

/* ========================================================================== */
/* 公共函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化测试环境
 */
int SensorTest_Initialize(void)
{
    /* 确保传感器模拟器已初始化 */
    if (SensorSimulator_Init(NULL) != 0) {
        return -1;
    }

    /* 确保EtherCAT桥接已初始化 */
    if (EtherCAT_SensorBridge_Init(NULL) != 0) {
        return -1;
    }

    g_test_initialized = true;
    return 0;
}

/**
 * @brief 清理测试环境
 */
void SensorTest_Cleanup(void)
{
    SensorSimulator_Enable(false);
    EtherCAT_SensorBridge_Stop();
    g_test_initialized = false;
}

/**
 * @brief 运行所有测试
 */
int SensorTest_RunAllTests(TestResults_t *results)
{
    TestResults_t sim_results, bridge_results, integration_results;
    TestResults_t fault_results, data_results, boundary_results;
    int overall_result = 0;

    if (!g_test_initialized) {
        if (SensorTest_Initialize() != 0) {
            return -1;
        }
    }

    _init_test_results(results);
    g_test_start_time = _get_time_ms();

    printf("开始运行传感器模拟系统全套测试...\r\n");

    /* 运行各项测试 */
    if (SensorTest_UnitTest_Simulator(&sim_results) != 0) {
        overall_result = -1;
    }

    if (SensorTest_UnitTest_Bridge(&bridge_results) != 0) {
        overall_result = -1;
    }

    if (SensorTest_IntegrationTest(&integration_results) != 0) {
        overall_result = -1;
    }

    if (SensorTest_FaultInjectionTest(&fault_results) != 0) {
        overall_result = -1;
    }

    if (SensorTest_DataConsistencyTest(100, &data_results) != 0) {
        overall_result = -1;
    }

    if (SensorTest_BoundaryValueTest(&boundary_results) != 0) {
        overall_result = -1;
    }

    /* 汇总结果 */
    results->tests_total = sim_results.tests_total + bridge_results.tests_total +
                          integration_results.tests_total + fault_results.tests_total +
                          data_results.tests_total + boundary_results.tests_total;

    results->tests_passed = sim_results.tests_passed + bridge_results.tests_passed +
                           integration_results.tests_passed + fault_results.tests_passed +
                           data_results.tests_passed + boundary_results.tests_passed;

    results->tests_failed = sim_results.tests_failed + bridge_results.tests_failed +
                           integration_results.tests_failed + fault_results.tests_failed +
                           data_results.tests_failed + boundary_results.tests_failed;

    results->execution_time_ms = _get_time_ms() - g_test_start_time;

    printf("全套测试完成！总计: %lu, 通过: %lu, 失败: %lu, 耗时: %lu ms\r\n",
           results->tests_total, results->tests_passed, results->tests_failed,
           results->execution_time_ms);

    return overall_result;
}

/**
 * @brief 运行传感器模拟器单元测试
 */
int SensorTest_UnitTest_Simulator(TestResults_t *results)
{
    const SensorData_t *data;
    bool test_passed;

    _init_test_results(results);
    printf("运行传感器模拟器单元测试...\r\n");

    /* 测试1: 初始化测试 */
    SensorSimulator_Enable(false);
    test_passed = (SensorSimulator_Init(NULL) == 0);
    _record_test_result(results, test_passed, "模拟器初始化失败");

    /* 测试2: 启用/禁用测试 */
    SensorSimulator_Enable(true);
    test_passed = SensorSimulator_IsEnabled();
    _record_test_result(results, test_passed, "模拟器启用失败");

    SensorSimulator_Enable(false);
    test_passed = !SensorSimulator_IsEnabled();
    _record_test_result(results, test_passed, "模拟器禁用失败");

    /* 测试3: 数据生成测试 */
    SensorSimulator_Enable(true);
    SensorSimulator_Update();
    data = SensorSimulator_GetData();
    test_passed = (data != NULL);
    _record_test_result(results, test_passed, "获取传感器数据失败");

    /* 测试4: 数据范围测试 */
    if (data != NULL) {
        test_passed = SensorTest_ValidateDataRange(data);
        _record_test_result(results, test_passed, "传感器数据超出有效范围");
    }

    /* 测试5: 重置功能测试 */
    uint16_t old_seq = data ? data->sequence_id : 0;
    SensorSimulator_Reset();
    SensorSimulator_Update();
    data = SensorSimulator_GetData();
    test_passed = (data != NULL && data->sequence_id != old_seq);
    _record_test_result(results, test_passed, "传感器重置功能失败");

    results->execution_time_ms = _get_time_ms() - g_test_start_time;
    printf("传感器模拟器单元测试完成: %lu/%lu 通过\r\n",
           results->tests_passed, results->tests_total);

    return (results->tests_failed == 0) ? 0 : -1;
}

/**
 * @brief 运行EtherCAT桥接单元测试
 */
int SensorTest_UnitTest_Bridge(TestResults_t *results)
{
    bool test_passed;
    uint8_t status;

    _init_test_results(results);
    printf("运行EtherCAT桥接单元测试...\r\n");

    /* 测试1: 桥接初始化 */
    test_passed = (EtherCAT_SensorBridge_Init(NULL) == 0);
    _record_test_result(results, test_passed, "桥接初始化失败");

    /* 测试2: 桥接启动 */
    test_passed = (EtherCAT_SensorBridge_Start() == 0);
    _record_test_result(results, test_passed, "桥接启动失败");

    /* 测试3: 状态检查 */
    status = EtherCAT_SensorBridge_GetStatus();
    test_passed = (status == BRIDGE_STATUS_OK);
    _record_test_result(results, test_passed, "桥接状态异常");

    /* 测试4: 数据更新 */
    EtherCAT_SensorBridge_UpdateInputs();
    const EtherCAT_SensorInputs_t *inputs = EtherCAT_SensorBridge_GetInputData();
    test_passed = (inputs != NULL);
    _record_test_result(results, test_passed, "桥接数据更新失败");

    /* 测试5: 输出处理 */
    EtherCAT_SensorBridge_ProcessOutputs();
    const EtherCAT_SensorOutputs_t *outputs = EtherCAT_SensorBridge_GetOutputData();
    test_passed = (outputs != NULL);
    _record_test_result(results, test_passed, "桥接输出处理失败");

    results->execution_time_ms = _get_time_ms() - g_test_start_time;
    printf("EtherCAT桥接单元测试完成: %lu/%lu 通过\r\n",
           results->tests_passed, results->tests_total);

    return (results->tests_failed == 0) ? 0 : -1;
}

/**
 * @brief 运行集成测试
 */
int SensorTest_IntegrationTest(TestResults_t *results)
{
    const SensorData_t *sensor_data;
    const EtherCAT_SensorInputs_t *ethercat_inputs;
    bool test_passed;

    _init_test_results(results);
    printf("运行集成测试...\r\n");

    /* 确保系统运行 */
    SensorSimulator_Enable(true);
    EtherCAT_SensorBridge_Start();

    /* 测试1: 端到端数据流 */
    for (int i = 0; i < 10; i++) {
        SensorSimulator_Update();
        EtherCAT_SensorBridge_UpdateInputs();

        sensor_data = SensorSimulator_GetData();
        ethercat_inputs = EtherCAT_SensorBridge_GetInputData();

        test_passed = (sensor_data != NULL && ethercat_inputs != NULL);
        _record_test_result(results, test_passed, "端到端数据流中断");

        if (test_passed) {
            /* 测试数据映射正确性 */
            test_passed = SensorTest_ValidateDataMapping(sensor_data, ethercat_inputs, 1.0f);
            _record_test_result(results, test_passed, "数据映射不正确");
        }

        /* 模拟延时 */
        HAL_Delay(10);
    }

    results->execution_time_ms = _get_time_ms() - g_test_start_time;
    printf("集成测试完成: %lu/%lu 通过\r\n",
           results->tests_passed, results->tests_total);

    return (results->tests_failed == 0) ? 0 : -1;
}

/**
 * @brief 运行性能测试
 */
int SensorTest_PerformanceTest(uint32_t duration_ms, PerformanceResults_t *perf_results)
{
    uint32_t start_time, end_time, update_start, update_end;
    uint32_t update_time;

    memset(perf_results, 0, sizeof(PerformanceResults_t));
    perf_results->min_update_time = 999999;

    printf("运行性能测试 (持续时间: %lu ms)...\r\n", duration_ms);

    start_time = _get_time_ms();
    SensorSimulator_Enable(true);
    EtherCAT_SensorBridge_Start();

    while ((_get_time_ms() - start_time) < duration_ms) {
        update_start = _get_time_us();

        SensorSimulator_Update();
        EtherCAT_SensorBridge_UpdateInputs();
        EtherCAT_SensorBridge_ProcessOutputs();

        update_end = _get_time_us();
        update_time = update_end - update_start;

        perf_results->update_count++;

        if (update_time < perf_results->min_update_time) {
            perf_results->min_update_time = update_time;
        }

        if (update_time > perf_results->max_update_time) {
            perf_results->max_update_time = update_time;
        }

        /* 简单移动平均 */
        if (perf_results->update_count == 1) {
            perf_results->avg_update_time = update_time;
        } else {
            perf_results->avg_update_time =
                (perf_results->avg_update_time * 9 + update_time) / 10;
        }

        HAL_Delay(1); /* 1ms延时 */
    }

    end_time = _get_time_ms();
    perf_results->total_time_ms = end_time - start_time;

    printf("性能测试完成: 更新次数=%lu, 平均时间=%lu µs\r\n",
           perf_results->update_count, perf_results->avg_update_time);

    return 0;
}

/**
 * @brief 运行故障注入测试
 */
int SensorTest_FaultInjectionTest(TestResults_t *results)
{
    const SensorData_t *sensor_data;
    bool test_passed;

    _init_test_results(results);
    printf("运行故障注入测试...\r\n");

    SensorSimulator_Enable(true);
    EtherCAT_SensorBridge_Start();

    /* 测试1: 注入传感器故障 */
    SensorSimulator_InjectFault(0, SENSOR_STATUS_ERROR);
    SensorSimulator_Update();
    sensor_data = SensorSimulator_GetData();
    test_passed = (sensor_data != NULL && sensor_data->sensor_status == SENSOR_STATUS_ERROR);
    _record_test_result(results, test_passed, "故障注入未生效");

    /* 测试2: 清除故障 */
    SensorSimulator_ClearFault(0);
    SensorSimulator_Update();
    sensor_data = SensorSimulator_GetData();
    test_passed = (sensor_data != NULL && sensor_data->sensor_status == SENSOR_STATUS_OK);
    _record_test_result(results, test_passed, "故障清除失败");

    /* 测试3: 多传感器故障 */
    for (uint8_t i = 0; i < 3; i++) {
        SensorSimulator_InjectFault(i, SENSOR_STATUS_WARNING);
    }
    SensorSimulator_Update();
    sensor_data = SensorSimulator_GetData();
    test_passed = (sensor_data != NULL && sensor_data->sensor_status != SENSOR_STATUS_OK);
    _record_test_result(results, test_passed, "多传感器故障检测失败");

    /* 清理故障 */
    for (uint8_t i = 0; i < 8; i++) {
        SensorSimulator_ClearFault(i);
    }

    results->execution_time_ms = _get_time_ms() - g_test_start_time;
    printf("故障注入测试完成: %lu/%lu 通过\r\n",
           results->tests_passed, results->tests_total);

    return (results->tests_failed == 0) ? 0 : -1;
}

/**
 * @brief 运行数据一致性测试
 */
int SensorTest_DataConsistencyTest(uint32_t sample_count, TestResults_t *results)
{
    const SensorData_t *sensor_data;
    const EtherCAT_SensorInputs_t *ethercat_inputs;
    bool test_passed;
    uint32_t consecutive_failures = 0;

    _init_test_results(results);
    printf("运行数据一致性测试 (采样次数: %lu)...\r\n", sample_count);

    SensorSimulator_Enable(true);
    EtherCAT_SensorBridge_Start();

    for (uint32_t i = 0; i < sample_count; i++) {
        SensorSimulator_Update();
        EtherCAT_SensorBridge_UpdateInputs();

        sensor_data = SensorSimulator_GetData();
        ethercat_inputs = EtherCAT_SensorBridge_GetInputData();

        if (sensor_data != NULL && ethercat_inputs != NULL) {
            test_passed = SensorTest_ValidateDataMapping(sensor_data, ethercat_inputs, 2.0f);

            if (test_passed) {
                consecutive_failures = 0;
            } else {
                consecutive_failures++;
                if (consecutive_failures > 5) {
                    _record_test_result(results, false, "连续数据一致性失败");
                    break;
                }
            }
        } else {
            _record_test_result(results, false, "获取数据失败");
        }

        if (i % 10 == 0) {
            _record_test_result(results, true, ""); /* 记录成功的测试 */
        }

        HAL_Delay(1);
    }

    results->execution_time_ms = _get_time_ms() - g_test_start_time;
    printf("数据一致性测试完成: %lu/%lu 通过\r\n",
           results->tests_passed, results->tests_total);

    return (results->tests_failed == 0) ? 0 : -1;
}

/**
 * @brief 运行边界值测试
 */
int SensorTest_BoundaryValueTest(TestResults_t *results)
{
    SensorConfig_t extreme_config;
    const SensorData_t *sensor_data;
    bool test_passed;

    _init_test_results(results);
    printf("运行边界值测试...\r\n");

    /* 配置极端值 */
    memset(&extreme_config, 0, sizeof(SensorConfig_t));
    extreme_config.temp_base = 80.0f;        /* 高温 */
    extreme_config.temp_amplitude = 20.0f;
    extreme_config.humidity_base = 95.0f;    /* 高湿度 */
    extreme_config.pressure_base = 120.0f;   /* 高压力 */
    extreme_config.update_period_ms = 10;

    SensorSimulator_SetConfig(&extreme_config);
    SensorSimulator_Enable(true);

    /* 运行极端条件测试 */
    for (int i = 0; i < 20; i++) {
        SensorSimulator_Update();
        sensor_data = SensorSimulator_GetData();

        if (sensor_data != NULL) {
            /* 检查数据是否仍在合理范围内 */
            test_passed = (sensor_data->temperature <= 85.0f &&
                          sensor_data->humidity <= 100.0f &&
                          sensor_data->pressure <= 125.0f);

            if (!test_passed) {
                _record_test_result(results, false, "极端值测试数据超限");
                break;
            }
        }

        HAL_Delay(20);
    }

    if (results->tests_failed == 0) {
        _record_test_result(results, true, "极端值测试通过");
    }

    /* 恢复默认配置 */
    SensorSimulator_Init(NULL);

    results->execution_time_ms = _get_time_ms() - g_test_start_time;
    printf("边界值测试完成: %lu/%lu 通过\r\n",
           results->tests_passed, results->tests_total);

    return (results->tests_failed == 0) ? 0 : -1;
}

/**
 * @brief 验证传感器数据范围
 */
bool SensorTest_ValidateDataRange(const SensorData_t *sensor_data)
{
    if (sensor_data == NULL) {
        return false;
    }

    /* 检查各传感器数据是否在合理范围内 */
    if (sensor_data->temperature < -50.0f || sensor_data->temperature > 100.0f) {
        return false;
    }

    if (sensor_data->humidity < 0.0f || sensor_data->humidity > 100.0f) {
        return false;
    }

    if (sensor_data->pressure < 50.0f || sensor_data->pressure > 150.0f) {
        return false;
    }

    if (fabs(sensor_data->acceleration_x) > 50.0f ||
        fabs(sensor_data->acceleration_y) > 50.0f ||
        fabs(sensor_data->acceleration_z) > 50.0f) {
        return false;
    }

    if (sensor_data->light_intensity < 0.0f || sensor_data->light_intensity > 100000.0f) {
        return false;
    }

    return true;
}

/**
 * @brief 验证EtherCAT数据映射
 */
bool SensorTest_ValidateDataMapping(const SensorData_t *sensor_data,
                                   const EtherCAT_SensorInputs_t *ethercat_inputs,
                                   float tolerance_percent)
{
    if (sensor_data == NULL || ethercat_inputs == NULL) {
        return false;
    }

    /* 检查数字信号映射 */
    if (sensor_data->switch_1 != ethercat_inputs->switch_1 ||
        sensor_data->switch_2 != ethercat_inputs->switch_2) {
        return false;
    }

    /* 检查模拟信号映射（考虑缩放和容差） */
    float temp_mapped = (float)ethercat_inputs->temperature_x10 / 10.0f;
    if (!_is_float_equal(sensor_data->temperature, temp_mapped, tolerance_percent)) {
        return false;
    }

    float humidity_mapped = (float)ethercat_inputs->humidity_x10 / 10.0f;
    if (!_is_float_equal(sensor_data->humidity, humidity_mapped, tolerance_percent)) {
        return false;
    }

    float pressure_mapped = (float)ethercat_inputs->pressure_x10 / 10.0f;
    if (!_is_float_equal(sensor_data->pressure, pressure_mapped, tolerance_percent)) {
        return false;
    }

    return true;
}

/**
 * @brief 打印测试结果
 */
void SensorTest_PrintResults(const TestResults_t *results, const char *test_name)
{
    printf("\r\n========== %s 测试结果 ==========\r\n", test_name);
    printf("总测试数: %lu\r\n", results->tests_total);
    printf("通过: %lu\r\n", results->tests_passed);
    printf("失败: %lu\r\n", results->tests_failed);
    printf("跳过: %lu\r\n", results->tests_skipped);
    printf("执行时间: %lu ms\r\n", results->execution_time_ms);

    if (results->tests_failed > 0) {
        printf("最后错误: %s\r\n", results->last_error);
    }

    float success_rate = (float)results->tests_passed / results->tests_total * 100.0f;
    printf("成功率: %.1f%%\r\n", success_rate);
    printf("=========================================\r\n\r\n");
}

/**
 * @brief 打印性能测试结果
 */
void SensorTest_PrintPerformanceResults(const PerformanceResults_t *perf_results)
{
    printf("\r\n========== 性能测试结果 ==========\r\n");
    printf("更新次数: %lu\r\n", perf_results->update_count);
    printf("最小更新时间: %lu µs\r\n", perf_results->min_update_time);
    printf("最大更新时间: %lu µs\r\n", perf_results->max_update_time);
    printf("平均更新时间: %lu µs\r\n", perf_results->avg_update_time);
    printf("总测试时间: %lu ms\r\n", perf_results->total_time_ms);

    if (perf_results->total_time_ms > 0) {
        float update_rate = (float)perf_results->update_count * 1000.0f / perf_results->total_time_ms;
        printf("平均更新频率: %.1f Hz\r\n", update_rate);
    }

    printf("===================================\r\n\r\n");
}

/* ========================================================================== */
/* 私有函数实现 */
/* ========================================================================== */

/**
 * @brief 初始化测试结果
 */
static void _init_test_results(TestResults_t *results)
{
    memset(results, 0, sizeof(TestResults_t));
}

/**
 * @brief 记录测试结果
 */
static void _record_test_result(TestResults_t *results, bool passed, const char *error_msg)
{
    results->tests_total++;

    if (passed) {
        results->tests_passed++;
    } else {
        results->tests_failed++;
        if (error_msg != NULL && strlen(error_msg) > 0) {
            strncpy(results->last_error, error_msg, sizeof(results->last_error) - 1);
            results->last_error[sizeof(results->last_error) - 1] = '\0';
        }
    }
}

/**
 * @brief 获取系统时间（毫秒）
 */
static uint32_t _get_time_ms(void)
{
    return HAL_GetTick();
}

/**
 * @brief 获取系统时间（微秒）
 */
static uint32_t _get_time_us(void)
{
    /* 简化实现，实际应用中可能需要更高精度的计时器 */
    return HAL_GetTick() * 1000;
}

/**
 * @brief 检查两个浮点数是否在容差范围内相等
 */
static bool _is_float_equal(float a, float b, float tolerance_percent)
{
    float diff = fabs(a - b);
    float avg = (fabs(a) + fabs(b)) / 2.0f;

    if (avg == 0.0f) {
        return diff < 0.001f; /* 绝对小值比较 */
    }

    float relative_error = (diff / avg) * 100.0f;
    return relative_error <= tolerance_percent;
}