#!/bin/bash

echo "========================================"
echo "代码语法验证"
echo "========================================"

# 验证头文件语法
echo "验证头文件语法..."
if gcc -c -fsyntax-only Inc/ethercat_sensor_bridge.h 2>/dev/null; then
    echo "✅ ethercat_sensor_bridge.h - 语法正确"
else
    echo "❌ ethercat_sensor_bridge.h - 语法错误"
    gcc -c -fsyntax-only Inc/ethercat_sensor_bridge.h
fi

echo ""

# 验证源文件语法
echo "验证源文件语法..."
if gcc -c -fsyntax-only -I Inc -I ../Drivers/STM32F4xx_HAL_Driver/Inc -I ../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I ../Drivers/CMSIS/Include Src/ethercat_sensor_bridge.c 2>/dev/null; then
    echo "✅ ethercat_sensor_bridge.c - 语法正确"
else
    echo "❌ ethercat_sensor_bridge.c - 语法错误"
    echo "具体错误："
    gcc -c -fsyntax-only -I Inc -I ../Drivers/STM32F4xx_HAL_Driver/Inc -I ../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I ../Drivers/CMSIS/Include Src/ethercat_sensor_bridge.c
fi

echo ""
echo "验证完成"
