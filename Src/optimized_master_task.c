/**
 * @brief 优化的主站信号接收任务 - 集成变化检测机制
 * @param pvParameters 任务参数
 */
void Task_MasterSignalReceiver_Optimized(void *pvParameters)
{
    master_command_t master_command;
    uint32_t cycle_counter = 0;
    uint32_t no_change_counter = 0;
    uint32_t last_force_update = 0;
    const uint32_t FORCE_UPDATE_INTERVAL = 5000; // 5秒强制更新一次

    printf("Task_MasterSignalReceiver_Optimized: Started with change detection\r\n");

    // 初始化输出监控模块
    EtherCAT_OutputMonitor_Init();

    for (;;)
    {
        // 1. 检查主站下发数据是否有变化
        uint8_t changes = EtherCAT_OutputMonitor_CheckChanges();

        // 2. 检查是否需要强制更新（心跳机制）
        bool force_update = EtherCAT_OutputMonitor_NeedForceUpdate(FORCE_UPDATE_INTERVAL);

        // 3. 只有在数据变化或需要强制更新时才处理
        if (changes != OUTPUT_CHANGE_NONE || force_update) {

            cycle_counter++;
            task_stats.master_task_cycles++;

            if (force_update && changes == OUTPUT_CHANGE_NONE) {
                printf("[Master] 强制心跳更新 (静默 %d 秒)\r\n",
                       (HAL_GetTick() - last_force_update) / 1000);
                last_force_update = HAL_GetTick();
            }

            // 4. 根据变化类型进行不同处理
            if (changes & OUTPUT_CHANGE_DIGITAL) {
                Process_Digital_Output_Changes();
            }

            if (changes & OUTPUT_CHANGE_ANALOG) {
                Process_Analog_Output_Changes();
            }

            if (changes & OUTPUT_CHANGE_COMMAND) {
                Process_Control_Command_Changes();
            }

            if (changes & OUTPUT_CHANGE_CONFIG) {
                Process_Configuration_Changes();
            }

            // 5. 更新输出数据缓存
            EtherCAT_OutputMonitor_UpdateCache(force_update);

            // 6. 设置事件标志
            xEventGroupSetBits(xEventGroup_SensorTasks, EVENT_MASTER_COMMAND);

            task_stats.commands_executed++;
            no_change_counter = 0;

            // 7. 调试输出（减少频率）
            if ((cycle_counter % 10) == 0) {
                printf("[Master] 变化类型: 0x%02X, 周期: %lu\r\n", changes, cycle_counter);
            }

        } else {
            // 无变化，跳过处理
            no_change_counter++;

            // 每100次无变化时输出一次统计
            if ((no_change_counter % 100) == 0) {
                printf("[Master] 无变化跳过: %lu 次\r\n", no_change_counter);
            }
        }

        // 8. 从队列检查是否有新命令（非阻塞）
        if (xQueueReceive(xQueue_MasterCommands, &master_command, pdMS_TO_TICKS(10)) == pdPASS)
        {
            task_stats.commands_received++;

            // 验证并处理队列命令
            if (Sensor_Validate_Master_Command(&master_command) == pdTRUE) {
                Master_ProcessCommand(&master_command);

                // 更新全局变量
                if (xSemaphoreTake(xMutex_MasterCommands, pdMS_TO_TICKS(10)) == pdTRUE) {
                    memcpy(&latest_master_command, &master_command, sizeof(master_command_t));
                    xSemaphoreGive(xMutex_MasterCommands);
                }
            } else {
                task_stats.command_errors++;
                printf("[Master] ERROR: Invalid command ID=%lu\r\n", master_command.command_id);
            }
        }

        // 9. 周期性统计输出
        if ((cycle_counter % 1000) == 0 && cycle_counter > 0) {
            EtherCAT_OutputMonitor_PrintStats();
        }

        // 10. 任务延时 - 根据系统负载调整
        vTaskDelay(pdMS_TO_TICKS(changes != OUTPUT_CHANGE_NONE ? 5 : 20));
    }
}

/**
 * @brief 处理数字输出变化
 */
static void Process_Digital_Output_Changes(void)
{
    printf("[Master] 处理数字输出变化: 0x%04X (掩码: 0x%04X)\r\n",
           Obj0x7011.digital_outputs, Obj0x7011.digital_output_mask);

    // 应用数字输出到硬件
    for (uint8_t i = 0; i < MAX_DIGITAL_OUTPUTS; i++) {
        if (Obj0x7011.digital_output_mask & (1 << i)) {
            uint8_t value = (Obj0x7011.digital_outputs & (1 << i)) ? 1 : 0;
            App_Set_Digital_Output(i, value);
        }
    }

    task_stats.digital_output_updates++;
}

/**
 * @brief 处理模拟输出变化
 */
static void Process_Analog_Output_Changes(void)
{
    printf("[Master] 处理模拟输出变化\r\n");

    // 应用模拟输出到硬件
    for (uint8_t i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
        if (Obj0x7012.analog_output_mask & (1 << i)) {
            App_Set_Analog_Output(i, Obj0x7012.channel[i]);
        }
    }

    task_stats.analog_output_updates++;
}

/**
 * @brief 处理控制命令变化
 */
static void Process_Control_Command_Changes(void)
{
    printf("[Master] 处理控制命令变化: 传感器=%d, 系统=%d\r\n",
           Obj0x7020.sensor_config_cmd, Obj0x7020.system_control_cmd);

    // 处理传感器配置命令
    switch (Obj0x7020.sensor_config_cmd) {
        case SENSOR_CMD_RESET:
            SensorSimulator_Reset();
            printf("[Master] 执行传感器复位\r\n");
            break;

        case SENSOR_CMD_CALIBRATE:
            // 执行校准
            printf("[Master] 执行传感器校准\r\n");
            break;

        case SENSOR_CMD_INJECT_FAULT:
            SensorSimulator_InjectFault(0, SENSOR_STATUS_ERROR);
            printf("[Master] 注入传感器故障\r\n");
            break;
    }

    // 处理系统控制命令
    switch (Obj0x7020.system_control_cmd) {
        case 1:  // 正常运行
            printf("[Master] 切换到正常运行模式\r\n");
            break;

        case 2:  // 紧急停止
            printf("[Master] 执行紧急停止\r\n");
            // 关闭所有输出
            for (uint8_t i = 0; i < MAX_DIGITAL_OUTPUTS; i++) {
                App_Set_Digital_Output(i, 0);
            }
            for (uint8_t i = 0; i < MAX_ANALOG_OUTPUTS; i++) {
                App_Set_Analog_Output(i, 0);
            }
            break;
    }
}

/**
 * @brief 处理配置参数变化
 */
static void Process_Configuration_Changes(void)
{
    printf("[Master] 处理配置变化: 采样率=%d, 滤波=%d\r\n",
           Obj0x7030.sampling_rate, Obj0x7030.filter_enable);

    // 更新传感器配置
    current_sensor_config.sampling_interval_ms = 1000 / Obj0x7030.sampling_rate;
    current_sensor_config.filter_enable = Obj0x7030.filter_enable;

    printf("[Master] 配置已更新\r\n");
}