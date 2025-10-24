/**
  ******************************************************************************
  * @file    mcp23017_example.h
  * @brief   MCP23017 GPIO扩展芯片驱动使用示例头文件
  ******************************************************************************
  */

#ifndef MCP23017_EXAMPLE_H
#define MCP23017_EXAMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mcp23017/MCP23017.h"

/* Function prototypes */
void MCP23017_Example_Init(void);
void MCP23017_Example_PinOperation(void);
void MCP23017_Example_PortOperation(void);
void MCP23017_Example_PinConfig(void);
void MCP23017_Example_LEDChase(void);
void MCP23017_Example_KeyScan(void);
void MCP23017_Task_Example(void *pvParameters);
void MCP23017_CreateTask(void);
void MCP23017_Example_FullTest(void);

#ifdef __cplusplus
}
#endif

#endif /* MCP23017_EXAMPLE_H */