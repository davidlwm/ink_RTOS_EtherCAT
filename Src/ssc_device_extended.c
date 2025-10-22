/**
 * @file    ssc_device_extended.c
 * @brief   SSC-Device扩展实现 - 支持多PDO和扩展IO
 * @author  EtherCAT Application Developer
 * @version 1.0.0
 * @date    2024-01-01
 *
 * @description
 * 本文件扩展了原有的SSC-Device.c，添加了对多PDO和扩展IO的支持。
 * 保持与原有代码的完全兼容性，可直接替换或补充原文件。
 *
 * @note
 * 建议将此文件的内容合并到原有的SSC-Device.c文件中，
 * 或者在SSC-Device.c中包含此文件的相关函数。
 */

#include "ecat_def.h"
#include "applInterface.h"
#include "app_io_handler.h"

#define _SSC_DEVICE_EXTENDED_ 1
#include "SSC-Device.h"
#undef _SSC_DEVICE_EXTENDED_

// ====================================================================
// 扩展对象字典条目 (需要添加到SSC-DeviceObjects.h中)
// ====================================================================

#ifdef _OBJD_
/**
 * @brief 扩展对象字典条目描述
 * 这些条目需要添加到ApplicationObjDic[]数组中
 */

// Object 0x6001: 扩展数字输入
OBJCONST TSDOINFOENTRYDESC OBJMEM asEntryDesc0x6001[] = {
    { DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
    { DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }, /* Subindex1 - Digital Inputs */
    { DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }  /* Subindex2 - Reserved */
};

OBJCONST UCHAR OBJMEM aName0x6001[] = "Digital Inputs Extended\000"
"Digital Inputs\000"
"Reserved\000\377";

// Object 0x7011: 扩展数字输出
OBJCONST TSDOINFOENTRYDESC OBJMEM asEntryDesc0x7011[] = {
    { DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
    { DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }, /* Subindex1 - Digital Outputs */
    { DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }  /* Subindex2 - Reserved */
};

OBJCONST UCHAR OBJMEM aName0x7011[] = "Digital Outputs Extended\000"
"Digital Outputs\000"
"Reserved\000\377";

// Object 0x6002: 模拟输入
OBJCONST TSDOINFOENTRYDESC OBJMEM asEntryDesc0x6002[] = {
    { DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
    { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ }, /* Subindex1 - Channel 1 */
    { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ }, /* Subindex2 - Channel 2 */
    { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ }, /* Subindex3 - Channel 3 */
    { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ }, /* Subindex4 - Channel 4 */
    { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ }, /* Subindex5 - Channel 5 */
    { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ }, /* Subindex6 - Channel 6 */
    { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ }, /* Subindex7 - Channel 7 */
    { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ }  /* Subindex8 - Channel 8 */
};

OBJCONST UCHAR OBJMEM aName0x6002[] = "Analog Inputs\000"
"Channel 1\000"
"Channel 2\000"
"Channel 3\000"
"Channel 4\000"
"Channel 5\000"
"Channel 6\000"
"Channel 7\000"
"Channel 8\000\377";

// Object 0x7012: 模拟输出
OBJCONST TSDOINFOENTRYDESC OBJMEM asEntryDesc0x7012[] = {
    { DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
    { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ }, /* Subindex1 - Channel 1 */
    { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ }, /* Subindex2 - Channel 2 */
    { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ }, /* Subindex3 - Channel 3 */
    { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ }  /* Subindex4 - Channel 4 */
};

OBJCONST UCHAR OBJMEM aName0x7012[] = "Analog Outputs\000"
"Channel 1\000"
"Channel 2\000"
"Channel 3\000"
"Channel 4\000\377";

// Object 0x1A01: 扩展TxPDO映射
OBJCONST TSDOINFOENTRYDESC OBJMEM asEntryDesc0x1A01[] = {
    { DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
    { DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - 数字输入扩展 */
    { DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - 模拟输入CH1 */
    { DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex3 - 模拟输入CH2 */
    { DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }  /* Subindex4 - 模拟输入CH3 */
};

OBJCONST UCHAR OBJMEM aName0x1A01[] = "TxPDO Mapping Extended\000"
"Digital Inputs Map\000"
"Analog Input 1 Map\000"
"Analog Input 2 Map\000"
"Analog Input 3 Map\000\377";

// Object 0x1602: 扩展RxPDO映射
OBJCONST TSDOINFOENTRYDESC OBJMEM asEntryDesc0x1602[] = {
    { DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
    { DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - 数字输出扩展 */
    { DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - 模拟输出CH1 */
    { DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }  /* Subindex3 - 模拟输出CH2 */
};

OBJCONST UCHAR OBJMEM aName0x1602[] = "RxPDO Mapping Extended\000"
"Digital Outputs Map\000"
"Analog Output 1 Map\000"
"Analog Output 2 Map\000\377";

#endif //#ifdef _OBJD_

// ====================================================================
// 扩展对象变量定义
// ====================================================================

// 扩展PDO映射对象
typedef struct OBJ_STRUCT_PACKED_START {
    UINT16 u16SubIndex0;
    UINT32 SI1; /* Subindex1 - 0x60010110 数字输入扩展16bit */
    UINT32 SI2; /* Subindex2 - 0x60020110 模拟输入CH1 */
    UINT32 SI3; /* Subindex3 - 0x60020210 模拟输入CH2 */
    UINT32 SI4; /* Subindex4 - 0x60020310 模拟输入CH3 */
} OBJ_STRUCT_PACKED_END TOBJ1A01;

typedef struct OBJ_STRUCT_PACKED_START {
    UINT16 u16SubIndex0;
    UINT32 SI1; /* Subindex1 - 0x70110110 数字输出扩展16bit */
    UINT32 SI2; /* Subindex2 - 0x70120110 模拟输出CH1 */
    UINT32 SI3; /* Subindex3 - 0x70120210 模拟输出CH2 */
} OBJ_STRUCT_PACKED_END TOBJ1602;

// 对象变量实例
TOBJ1A01 ExtendedTxPDOMapping = {4, 0x60010110, 0x60020110, 0x60020210, 0x60020310};
TOBJ1602 ExtendedRxPDOMapping = {3, 0x70110110, 0x70120110, 0x70120210};

// 扩展同步管理器分配 (支持多PDO)
typedef struct OBJ_STRUCT_PACKED_START {
    UINT16   u16SubIndex0;
    UINT16 aEntries[2];  // 支持2个PDO
} OBJ_STRUCT_PACKED_END TOBJ1C12_EXT;

typedef struct OBJ_STRUCT_PACKED_START {
    UINT16   u16SubIndex0;
    UINT16 aEntries[2];  // 支持2个PDO
} OBJ_STRUCT_PACKED_END TOBJ1C13_EXT;

// 如果需要支持多PDO，可以替换原有的分配对象
TOBJ1C12_EXT sRxPDOassign_Extended = {2, {0x1601, 0x1602}};
TOBJ1C13_EXT sTxPDOassign_Extended = {2, {0x1A00, 0x1A01}};

// ====================================================================
// 扩展函数实现
// ====================================================================

/**
 * @brief 扩展输入映射函数
 * @param pData 指向输入过程数据的指针
 * @note 支持多PDO映射，替换原有APPL_InputMapping函数
 */
void APPL_InputMapping_Extended(UINT16* pData)
{
    UINT16 j = 0;
    UINT16 *pTmpData = (UINT16 *)pData;

    /* 遍历所有TxPDO分配对象 */
    for (j = 0; j < sTxPDOassign.u16SubIndex0; j++)
    {
        switch (sTxPDOassign.aEntries[j])
        {
        case 0x1A00:  // 原有TxPDO 1 (兼容性)
            // 保持原有格式：Switch1, Switch2打包在一个WORD中
            *pTmpData++ = SWAPWORD(((UINT16 *) &Obj0x6000)[1]);
            break;

        case 0x1A01:  // 扩展TxPDO 2
            // 数字输入扩展 (16bit)
            *pTmpData++ = SWAPWORD(Obj0x6001.digital_inputs);

            // 模拟输入通道1-3 (3x16bit)
            *pTmpData++ = SWAPWORD((UINT16)Obj0x6002.channel[0]);
            *pTmpData++ = SWAPWORD((UINT16)Obj0x6002.channel[1]);
            *pTmpData++ = SWAPWORD((UINT16)Obj0x6002.channel[2]);
            break;

        default:
            // 未知PDO，跳过
            break;
        }
    }
}

/**
 * @brief 扩展输出映射函数
 * @param pData 指向输出过程数据的指针
 * @note 支持多PDO映射，替换原有APPL_OutputMapping函数
 */
void APPL_OutputMapping_Extended(UINT16* pData)
{
    UINT16 j = 0;
    UINT16 *pTmpData = (UINT16 *)pData;

    /* 遍历所有RxPDO分配对象 */
    for (j = 0; j < sRxPDOassign.u16SubIndex0; j++)
    {
        switch (sRxPDOassign.aEntries[j])
        {
        case 0x1601:  // 原有RxPDO 1 (兼容性)
            // 保持原有格式：Led1, Led2从一个WORD中解包
            ((UINT16 *) &Obj0x7010)[1] = SWAPWORD(*pTmpData++);
            break;

        case 0x1602:  // 扩展RxPDO 2
            // 数字输出扩展 (16bit)
            Obj0x7011.digital_outputs = SWAPWORD(*pTmpData++);

            // 模拟输出通道1-2 (2x16bit)
            Obj0x7012.channel[0] = (INT16)SWAPWORD(*pTmpData++);
            Obj0x7012.channel[1] = (INT16)SWAPWORD(*pTmpData++);
            break;

        default:
            // 未知PDO，跳过
            break;
        }
    }
}

/**
 * @brief 扩展应用程序主循环
 * @note 替换原有APPL_Application函数，或在其中调用
 */
void APPL_Application_Extended(void)
{
    // === 调用扩展IO处理模块 ===
    App_IO_Handler();

    // === 原有代码保持兼容性 (可选) ===
    // 这部分代码在App_IO_Handler中已经处理，可以注释掉
    /*
    if(Obj0x7010.Led1) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_RESET);
    }

    if(Obj0x7010.Led2) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    }

    Obj0x6000.Switch1 = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2);
    Obj0x6000.Switch2 = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3);
    */
}

/**
 * @brief 扩展映射生成函数
 * @param pInputSize 输入数据大小指针
 * @param pOutputSize 输出数据大小指针
 * @return AL状态码
 * @note 计算扩展PDO的数据大小
 */
UINT16 APPL_GenerateMapping_Extended(UINT16 *pInputSize, UINT16 *pOutputSize)
{
    UINT16 result = ALSTATUSCODE_NOERROR;
    UINT16 InputSize = 0;
    UINT16 OutputSize = 0;

#if COE_SUPPORTED
    UINT16 PDOAssignEntryCnt = 0;
    OBJCONST TOBJECT OBJMEM * pPDO = NULL;
    UINT16 PDOSubindex0 = 0;
    UINT32 *pPDOEntry = NULL;
    UINT16 PDOEntryCnt = 0;
    UINT16 PDOSize = 0;

    /*计算输出PDO大小 (RxPDO assign - 主站到从站)*/
    for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sRxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)
    {
        pPDO = OBJ_GetObjectHandle(sRxPDOassign.aEntries[PDOAssignEntryCnt]);
        if(pPDO != NULL)
        {
            PDOSubindex0 = *((UINT16 *)pPDO->pVarPtr);
            for(PDOEntryCnt = 0; PDOEntryCnt < PDOSubindex0; PDOEntryCnt++)
            {
                pPDOEntry = (UINT32 *)((UINT16 *)pPDO->pVarPtr + (OBJ_GetEntryOffset((PDOEntryCnt+1),pPDO)>>3));

                // 提取位长度 (bits 0-7)
                PDOSize += (UINT16) ((*pPDOEntry) & 0xFF);
            }
        }
    }
    OutputSize = (PDOSize + 7) >> 3;  // 转换为字节

    /*计算输入PDO大小 (TxPDO assign - 从站到主站)*/
    PDOSize = 0;
    for(PDOAssignEntryCnt = 0; PDOAssignEntryCnt < sTxPDOassign.u16SubIndex0; PDOAssignEntryCnt++)
    {
        pPDO = OBJ_GetObjectHandle(sTxPDOassign.aEntries[PDOAssignEntryCnt]);
        if(pPDO != NULL)
        {
            PDOSubindex0 = *((UINT16 *)pPDO->pVarPtr);
            for(PDOEntryCnt = 0; PDOEntryCnt < PDOSubindex0; PDOEntryCnt++)
            {
                pPDOEntry = (UINT32 *)((UINT16 *)pPDO->pVarPtr + (OBJ_GetEntryOffset((PDOEntryCnt+1),pPDO)>>3));

                // 提取位长度 (bits 0-7)
                PDOSize += (UINT16) ((*pPDOEntry) & 0xFF);
            }
        }
    }
    InputSize = (PDOSize + 7) >> 3;  // 转换为字节

#else
    // 静态大小定义 (如果不使用CoE)
    InputSize = 10;   // 基础2字节 + 扩展8字节 (16bit数字输入 + 3x16bit模拟输入)
    OutputSize = 6;   // 基础2字节 + 扩展4字节 (16bit数字输出 + 2x16bit模拟输出)
#endif

    *pInputSize = InputSize;
    *pOutputSize = OutputSize;

    return result;
}

/**
 * @brief 初始化扩展功能
 * @note 在MainInit()之后调用
 */
void APPL_ExtendedInit(void)
{
    // 初始化扩展IO处理模块
    App_IO_Init();

    // 可以在这里进行其他扩展初始化
    // 例如：特殊GPIO配置、ADC/DAC校准等

#if APP_IO_DEBUG_ENABLE
    printf("[APPL_EXT] Extended functionality initialized\r\n");
#endif
}

/**
 * @brief SDO访问回调函数 (可选)
 * @param index 对象索引
 * @param subindex 子索引
 * @param size 数据大小
 * @param pData 数据指针
 * @param bCompleteAccess 完整访问标志
 * @return 访问结果
 * @note 可以用于实现特殊的SDO访问处理
 */
UINT8 APPL_ExtendedSDOAccess(UINT16 index, UINT8 subindex, UINT32 size, UINT16 MBXMEM * pData, UINT8 bCompleteAccess)
{
    // 处理扩展对象的特殊访问需求
    switch(index)
    {
    case 0x6001:  // 扩展数字输入
    case 0x6002:  // 模拟输入
        // 这些对象只读，拒绝写访问
        return ABORTIDX_READ_ONLY_ENTRY;

    case 0x7011:  // 扩展数字输出
    case 0x7012:  // 模拟输出
        // 允许通过SDO直接写入
        // 实际的IO操作会在下一个应用循环中生效
        return 0;  // 成功

    default:
        // 其他对象使用默认处理
        return 0;
    }
}

// ====================================================================
// 集成函数 - 用于替换或扩展原有SSC-Device.c中的函数
// ====================================================================

/**
 * @brief 集成函数：替换原有的APPL_Application
 * @note 将此函数重命名为APPL_Application并替换原有函数
 */
void APPL_Application_Integrated(void)
{
    APPL_Application_Extended();
}

/**
 * @brief 集成函数：替换原有的APPL_InputMapping
 * @note 将此函数重命名为APPL_InputMapping并替换原有函数
 */
void APPL_InputMapping_Integrated(UINT16* pData)
{
    APPL_InputMapping_Extended(pData);
}

/**
 * @brief 集成函数：替换原有的APPL_OutputMapping
 * @note 将此函数重命名为APPL_OutputMapping并替换原有函数
 */
void APPL_OutputMapping_Integrated(UINT16* pData)
{
    APPL_OutputMapping_Extended(pData);
}

/**
 * @brief 集成函数：替换原有的APPL_GenerateMapping
 * @note 将此函数重命名为APPL_GenerateMapping并替换原有函数
 */
UINT16 APPL_GenerateMapping_Integrated(UINT16 *pInputSize, UINT16 *pOutputSize)
{
    return APPL_GenerateMapping_Extended(pInputSize, pOutputSize);
}

// ====================================================================
// 使用说明和集成指导
// ====================================================================

/*
集成步骤：

1. 将app_io_handler.h和app_io_handler.c添加到项目中

2. 在SSC-Device.c中添加以下包含：
   #include "app_io_handler.h"

3. 替换或修改SSC-Device.c中的以下函数：
   - APPL_Application() → 调用APPL_Application_Extended()
   - APPL_InputMapping() → 调用APPL_InputMapping_Extended()
   - APPL_OutputMapping() → 调用APPL_OutputMapping_Extended()
   - APPL_GenerateMapping() → 调用APPL_GenerateMapping_Extended()

4. 在main()函数中，在MainInit()之后添加：
   APPL_ExtendedInit();

5. 将扩展对象添加到SSC-DeviceObjects.h的ApplicationObjDic[]数组中

6. 根据实际硬件配置修改app_io_handler.c中的引脚配置表

7. 编译并测试功能

示例集成代码（SSC-Device.c修改）：
```c
#include "app_io_handler.h"

void APPL_Application(void)
{
    APPL_Application_Extended();
}

void APPL_InputMapping(UINT16* pData)
{
    APPL_InputMapping_Extended(pData);
}

void APPL_OutputMapping(UINT16* pData)
{
    APPL_OutputMapping_Extended(pData);
}

UINT16 APPL_GenerateMapping(UINT16 *pInputSize, UINT16 *pOutputSize)
{
    return APPL_GenerateMapping_Extended(pInputSize, pOutputSize);
}
```

示例main()修改：
```c
int main(void)
{
    // 系统初始化
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_DAC_Init();

    // EtherCAT初始化
    HW_Init();
    MainInit();

    // 扩展功能初始化
    APPL_ExtendedInit();

    // 主循环
    bRunApplication = TRUE;
    do {
        MainLoop();
    } while (bRunApplication == TRUE);

    HW_Release();
    return 0;
}
```
*/