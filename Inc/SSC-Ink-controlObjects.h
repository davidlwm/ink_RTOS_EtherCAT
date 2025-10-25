/**
* \addtogroup SSC-Ink-control SSC-Ink-control
* @{
*/

/**
\file SSC-Ink-controlObjects
\author ET9300Utilities.ApplicationHandler (Version 1.3.6.0) | EthercatSSC@beckhoff.com

\brief SSC-Ink-control specific objects<br>
\brief NOTE : This file will be overwritten if a new object dictionary is generated!<br>
*/

#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
#define PROTO
#else
#define PROTO extern
#endif
/******************************************************************************
*                    Object 0x1600 : Number of Entries process data mapping
******************************************************************************/
/**
* \addtogroup 0x1600 0x1600 | Number of Entries process data mapping
* @{
* \brief Object 0x1600 (Number of Entries process data mapping) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Reference to 0x7000.1<br>
* SubIndex 2 - Reference to 0x7000.2<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1600[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - Reference to 0x7000.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex2 - Reference to 0x7000.2 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x1600[] = "Number of Entries process data mapping\000"
"SubIndex 001\000"
"SubIndex 002\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 SI1; /* Subindex1 - Reference to 0x7000.1 */
UINT32 SI2; /* Subindex2 - Reference to 0x7000.2 */
} OBJ_STRUCT_PACKED_END
TOBJ1600;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1600 NumberOfEntriesProcessDataMapping0x1600
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={2,0x70000110,0x70000210}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1A00 : Input mapping 0
******************************************************************************/
/**
* \addtogroup 0x1A00 0x1A00 | Input mapping 0
* @{
* \brief Object 0x1A00 (Input mapping 0) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Reference to 0x6000.1<br>
* SubIndex 2 - Reference to 0x6000.2<br>
* SubIndex 3 - Reference to 0x6001.1<br>
* SubIndex 4 - Reference to 0x6001.2<br>
* SubIndex 5 - Reference to 0x6002.1<br>
* SubIndex 6 - Reference to 0x6002.2<br>
* SubIndex 7 - Reference to 0x6003.1<br>
* SubIndex 8 - Reference to 0x6003.2<br>
* SubIndex 9 - Reference to 0x6004.1<br>
* SubIndex 10 - Reference to 0x6004.2<br>
* SubIndex 11 - Reference to 0x6004.3<br>
* SubIndex 12 - Reference to 0x6005.1<br>
* SubIndex 13 - Reference to 0x6005.2<br>
* SubIndex 14 - Reference to 0x6006.1<br>
* SubIndex 15 - Reference to 0x6006.2<br>
* SubIndex 16 - Reference to 0x6007.1<br>
* SubIndex 17 - Reference to 0x6007.2<br>
* SubIndex 18 - Reference to 0x6007.3<br>
* SubIndex 19 - Reference to 0x6007.4<br>
* SubIndex 20 - Reference to 0x6007.5<br>
* SubIndex 21 - Reference to 0x6007.6<br>
* SubIndex 22 - Reference to 0x6007.7<br>
* SubIndex 23 - Reference to 0x6007.8<br>
* SubIndex 24 - Reference to 0x6007.9<br>
* SubIndex 25 - Reference to 0x6007.10<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1A00[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - Reference to 0x6000.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - Reference to 0x6000.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex3 - Reference to 0x6001.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex4 - Reference to 0x6001.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex5 - Reference to 0x6002.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex6 - Reference to 0x6002.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex7 - Reference to 0x6003.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex8 - Reference to 0x6003.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex9 - Reference to 0x6004.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex10 - Reference to 0x6004.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex11 - Reference to 0x6004.3 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex12 - Reference to 0x6005.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex13 - Reference to 0x6005.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex14 - Reference to 0x6006.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex15 - Reference to 0x6006.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex16 - Reference to 0x6007.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex17 - Reference to 0x6007.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex18 - Reference to 0x6007.3 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex19 - Reference to 0x6007.4 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex20 - Reference to 0x6007.5 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex21 - Reference to 0x6007.6 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex22 - Reference to 0x6007.7 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex23 - Reference to 0x6007.8 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex24 - Reference to 0x6007.9 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex25 - Reference to 0x6007.10 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x1A00[] = "Input mapping 0\000"
"SubIndex 001\000"
"SubIndex 002\000"
"SubIndex 003\000"
"SubIndex 004\000"
"SubIndex 005\000"
"SubIndex 006\000"
"SubIndex 007\000"
"SubIndex 008\000"
"SubIndex 009\000"
"SubIndex 010\000"
"SubIndex 011\000"
"SubIndex 012\000"
"SubIndex 013\000"
"SubIndex 014\000"
"SubIndex 015\000"
"SubIndex 016\000"
"SubIndex 017\000"
"SubIndex 018\000"
"SubIndex 019\000"
"SubIndex 020\000"
"SubIndex 021\000"
"SubIndex 022\000"
"SubIndex 023\000"
"SubIndex 024\000"
"SubIndex 025\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 SI1; /* Subindex1 - Reference to 0x6000.1 */
UINT32 SI2; /* Subindex2 - Reference to 0x6000.2 */
UINT32 SI3; /* Subindex3 - Reference to 0x6001.1 */
UINT32 SI4; /* Subindex4 - Reference to 0x6001.2 */
UINT32 SI5; /* Subindex5 - Reference to 0x6002.1 */
UINT32 SI6; /* Subindex6 - Reference to 0x6002.2 */
UINT32 SI7; /* Subindex7 - Reference to 0x6003.1 */
UINT32 SI8; /* Subindex8 - Reference to 0x6003.2 */
UINT32 SI9; /* Subindex9 - Reference to 0x6004.1 */
UINT32 SI10; /* Subindex10 - Reference to 0x6004.2 */
UINT32 SI11; /* Subindex11 - Reference to 0x6004.3 */
UINT32 SI12; /* Subindex12 - Reference to 0x6005.1 */
UINT32 SI13; /* Subindex13 - Reference to 0x6005.2 */
UINT32 SI14; /* Subindex14 - Reference to 0x6006.1 */
UINT32 SI15; /* Subindex15 - Reference to 0x6006.2 */
UINT32 SI16; /* Subindex16 - Reference to 0x6007.1 */
UINT32 SI17; /* Subindex17 - Reference to 0x6007.2 */
UINT32 SI18; /* Subindex18 - Reference to 0x6007.3 */
UINT32 SI19; /* Subindex19 - Reference to 0x6007.4 */
UINT32 SI20; /* Subindex20 - Reference to 0x6007.5 */
UINT32 SI21; /* Subindex21 - Reference to 0x6007.6 */
UINT32 SI22; /* Subindex22 - Reference to 0x6007.7 */
UINT32 SI23; /* Subindex23 - Reference to 0x6007.8 */
UINT32 SI24; /* Subindex24 - Reference to 0x6007.9 */
UINT32 SI25; /* Subindex25 - Reference to 0x6007.10 */
} OBJ_STRUCT_PACKED_END
TOBJ1A00;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1A00 InputMapping00x1A00
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={25,0x60000110,0x60000210,0x60010110,0x60010210,0x60020110,0x60020210,0x60030110,0x60030210,0x60040110,0x60040210,0x60040310,0x60050110,0x60050210,0x60060110,0x60060210,0x60070110,0x60070210,0x60070310,0x60070410,0x60070510,0x60070610,0x60070710,0x60070810,0x60070910,0x60070A10}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1C12 : SyncManager 2 assignment
******************************************************************************/
/**
* \addtogroup 0x1C12 0x1C12 | SyncManager 2 assignment
* @{
* \brief Object 0x1C12 (SyncManager 2 assignment) definition
*/
#ifdef _OBJD_
/**
* \brief Entry descriptions<br>
* 
* Subindex 0<br>
* Subindex 1 - n (the same entry description is used)<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1C12[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }};

/**
* \brief Object name definition<br>
* For Subindex 1 to n the syntax 'Subindex XXX' is used
*/
OBJCONST UCHAR OBJMEM aName0x1C12[] = "SyncManager 2 assignment\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16   u16SubIndex0;  /**< \brief Subindex 0 */
UINT16 aEntries[1];  /**< \brief Subindex 1 - 1 */
} OBJ_STRUCT_PACKED_END
TOBJ1C12;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1C12 sRxPDOassign
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={1,{0x1600}}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1C13 : SyncManager 3 assignment
******************************************************************************/
/**
* \addtogroup 0x1C13 0x1C13 | SyncManager 3 assignment
* @{
* \brief Object 0x1C13 (SyncManager 3 assignment) definition
*/
#ifdef _OBJD_
/**
* \brief Entry descriptions<br>
* 
* Subindex 0<br>
* Subindex 1 - n (the same entry description is used)<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1C13[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }};

/**
* \brief Object name definition<br>
* For Subindex 1 to n the syntax 'Subindex XXX' is used
*/
OBJCONST UCHAR OBJMEM aName0x1C13[] = "SyncManager 3 assignment\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16   u16SubIndex0;  /**< \brief Subindex 0 */
UINT16 aEntries[1];  /**< \brief Subindex 1 - 1 */
} OBJ_STRUCT_PACKED_END
TOBJ1C13;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1C13 sTxPDOassign
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={1,{0x1A00}}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6000 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x6000 0x6000 | Number of Entries
* @{
* \brief Object 0x6000 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 输入状态<br>
* SubIndex 2 - 输出状态<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6000[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ | OBJACCESS_TXPDOMAPPING },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - 输入状态 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }}; /* Subindex2 - 输出状态 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x6000[] = "Number of Entries\000"
"输入状态\000"
"输出状态\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 输入状态; /* Subindex1 - 输入状态 */
INT16 输出状态; /* Subindex2 - 输出状态 */
} OBJ_STRUCT_PACKED_END
TOBJ6000;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6000 NumberOfEntries0x6000
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={2,0x00,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6001 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x6001 0x6001 | Number of Entries
* @{
* \brief Object 0x6001 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 报警信息1<br>
* SubIndex 2 - 报警信息2<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6001[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ | OBJACCESS_TXPDOMAPPING },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - 报警信息1 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }}; /* Subindex2 - 报警信息2 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x6001[] = "Number of Entries\000"
"报警信息1\000"
"报警信息2\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 报警信息1; /* Subindex1 - 报警信息1 */
INT16 报警信息2; /* Subindex2 - 报警信息2 */
} OBJ_STRUCT_PACKED_END
TOBJ6001;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6001 NumberOfEntries0x6001
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={2,0x00,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6002 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x6002 0x6002 | Number of Entries
* @{
* \brief Object 0x6002 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 提示信息1<br>
* SubIndex 2 - 提示信息2<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6002[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ | OBJACCESS_TXPDOMAPPING },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - 提示信息1 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }}; /* Subindex2 - 提示信息2 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x6002[] = "Number of Entries\000"
"提示信息1\000"
"提示信息2\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 提示信息1; /* Subindex1 - 提示信息1 */
INT16 提示信息2; /* Subindex2 - 提示信息2 */
} OBJ_STRUCT_PACKED_END
TOBJ6002;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6002 NumberOfEntries0x6002
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={2,0x00,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6003 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x6003 0x6003 | Number of Entries
* @{
* \brief Object 0x6003 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 状态显示<br>
* SubIndex 2 - 状态码<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6003[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ | OBJACCESS_TXPDOMAPPING },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - 状态显示 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }}; /* Subindex2 - 状态码 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x6003[] = "Number of Entries\000"
"状态显示\000"
"状态码\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 状态显示; /* Subindex1 - 状态显示 */
INT16 状态码; /* Subindex2 - 状态码 */
} OBJ_STRUCT_PACKED_END
TOBJ6003;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6003 NumberOfEntries0x6003
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={2,0x00,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6004 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x6004 0x6004 | Number of Entries
* @{
* \brief Object 0x6004 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 墨盒温度<br>
* SubIndex 2 - 阻尼器温度<br>
* SubIndex 3 - 墨盒液位<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6004[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ | OBJACCESS_TXPDOMAPPING },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - 墨盒温度 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex2 - 阻尼器温度 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }}; /* Subindex3 - 墨盒液位 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x6004[] = "Number of Entries\000"
"墨盒温度\000"
"阻尼器温度\000"
"墨盒液位\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 墨盒温度; /* Subindex1 - 墨盒温度 */
INT16 阻尼器温度; /* Subindex2 - 阻尼器温度 */
INT16 墨盒液位; /* Subindex3 - 墨盒液位 */
} OBJ_STRUCT_PACKED_END
TOBJ6004;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6004 NumberOfEntries0x6004
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={3,0x00,0x00,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6005 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x6005 0x6005 | Number of Entries
* @{
* \brief Object 0x6005 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 供墨泵流量%<br>
* SubIndex 2 - 回墨泵流量%<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6005[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ | OBJACCESS_TXPDOMAPPING },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - 供墨泵流量% */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }}; /* Subindex2 - 回墨泵流量% */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x6005[] = "Number of Entries\000"
"供墨泵流量%\000"
"回墨泵流量%\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 供墨泵流量%; /* Subindex1 - 供墨泵流量% */
INT16 回墨泵流量%; /* Subindex2 - 回墨泵流量% */
} OBJ_STRUCT_PACKED_END
TOBJ6005;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6005 NumberOfEntries0x6005
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={2,0x00,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6006 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x6006 0x6006 | Number of Entries
* @{
* \brief Object 0x6006 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Pin目标值<br>
* SubIndex 2 - Pout目标值<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6006[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ | OBJACCESS_TXPDOMAPPING },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - Pin目标值 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }}; /* Subindex2 - Pout目标值 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x6006[] = "Number of Entries\000"
"Pin目标值\000"
"Pout目标值\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 Pin目标值; /* Subindex1 - Pin目标值 */
INT16 Pout目标值; /* Subindex2 - Pout目标值 */
} OBJ_STRUCT_PACKED_END
TOBJ6006;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6006 NumberOfEntries0x6006
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={2,0x00,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6007 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x6007 0x6007 | Number of Entries
* @{
* \brief Object 0x6007 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Pin实际值<br>
* SubIndex 2 - Pout实际值<br>
* SubIndex 3 - 实际Pm<br>
* SubIndex 4 - 实际DP<br>
* SubIndex 5 - 填墨泵动画<br>
* SubIndex 6 - 供墨泵动画<br>
* SubIndex 7 - 回墨泵动画<br>
* SubIndex 8 - 补墨泵动画<br>
* SubIndex 9 - 收墨电磁阀动画<br>
* SubIndex 10 - 墨桶回墨阀动画<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6007[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ | OBJACCESS_TXPDOMAPPING },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - Pin实际值 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex2 - Pout实际值 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex3 - 实际Pm */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex4 - 实际DP */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex5 - 填墨泵动画 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex6 - 供墨泵动画 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex7 - 回墨泵动画 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex8 - 补墨泵动画 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex9 - 收墨电磁阀动画 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }}; /* Subindex10 - 墨桶回墨阀动画 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x6007[] = "Number of Entries\000"
"Pin实际值\000"
"Pout实际值\000"
"实际Pm\000"
"实际DP\000"
"填墨泵动画\000"
"供墨泵动画\000"
"回墨泵动画\000"
"补墨泵动画\000"
"收墨电磁阀动画\000"
"墨桶回墨阀动画\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 Pin实际值; /* Subindex1 - Pin实际值 */
INT16 Pout实际值; /* Subindex2 - Pout实际值 */
INT16 实际Pm; /* Subindex3 - 实际Pm */
INT16 实际DP; /* Subindex4 - 实际DP */
INT16 填墨泵动画; /* Subindex5 - 填墨泵动画 */
INT16 供墨泵动画; /* Subindex6 - 供墨泵动画 */
INT16 回墨泵动画; /* Subindex7 - 回墨泵动画 */
INT16 补墨泵动画; /* Subindex8 - 补墨泵动画 */
INT16 收墨电磁阀动画; /* Subindex9 - 收墨电磁阀动画 */
INT16 墨桶回墨阀动画; /* Subindex10 - 墨桶回墨阀动画 */
} OBJ_STRUCT_PACKED_END
TOBJ6007;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6007 NumberOfEntries0x6007
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x7000 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x7000 0x7000 | Number of Entries
* @{
* \brief Object 0x7000 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 手动操作1<br>
* SubIndex 2 - 手动操作2<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x7000[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ | OBJACCESS_RXPDOMAPPING },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex1 - 手动操作1 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }}; /* Subindex2 - 手动操作2 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x7000[] = "Number of Entries\000"
"手动操作1\000"
"手动操作2\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 手动操作1; /* Subindex1 - 手动操作1 */
INT16 手动操作2; /* Subindex2 - 手动操作2 */
} OBJ_STRUCT_PACKED_END
TOBJ7000;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ7000 NumberOfEntries0x7000
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={2,0x00,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x8000 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x8000 0x8000 | Number of Entries
* @{
* \brief Object 0x8000 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 温度设置值<br>
* SubIndex 2 - 温度补偿值<br>
* SubIndex 3 - 待机温度<br>
* SubIndex 4 - 温度上限<br>
* SubIndex 5 - 温度下限<br>
* SubIndex 6 - 最小加热时间<br>
* SubIndex 7 - 加热功率百分比<br>
* SubIndex 8 - 运算周期<br>
* SubIndex 9 - 比例增益<br>
* SubIndex 10 - 积分时间<br>
* SubIndex 11 - 微分时间<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x8000[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 温度设置值 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 温度补偿值 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 待机温度 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex4 - 温度上限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex5 - 温度下限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex6 - 最小加热时间 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex7 - 加热功率百分比 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex8 - 运算周期 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex9 - 比例增益 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex10 - 积分时间 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex11 - 微分时间 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x8000[] = "Number of Entries\000"
"温度设置值\000"
"温度补偿值\000"
"待机温度\000"
"温度上限\000"
"温度下限\000"
"最小加热时间\000"
"加热功率百分比\000"
"运算周期\000"
"比例增益\000"
"积分时间\000"
"微分时间\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 温度设置值; /* Subindex1 - 温度设置值 */
INT16 温度补偿值; /* Subindex2 - 温度补偿值 */
INT16 待机温度; /* Subindex3 - 待机温度 */
INT16 温度上限; /* Subindex4 - 温度上限 */
INT16 温度下限; /* Subindex5 - 温度下限 */
INT16 最小加热时间; /* Subindex6 - 最小加热时间 */
INT16 加热功率百分比; /* Subindex7 - 加热功率百分比 */
INT16 运算周期; /* Subindex8 - 运算周期 */
INT16 比例增益; /* Subindex9 - 比例增益 */
INT16 积分时间; /* Subindex10 - 积分时间 */
INT16 微分时间; /* Subindex11 - 微分时间 */
} OBJ_STRUCT_PACKED_END
TOBJ8000;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ8000 NumberOfEntries0x8000
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={11,0x0003,0x00,0x5002,0x5003,0x0002,0x10,0x0001,0x50,0x50,0x10,0x05}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x8001 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x8001 0x8001 | Number of Entries
* @{
* \brief Object 0x8001 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 温度设置值<br>
* SubIndex 2 - 温度补偿值<br>
* SubIndex 3 - 待机温度<br>
* SubIndex 4 - 温度上限<br>
* SubIndex 5 - 温度下限<br>
* SubIndex 6 - 最小加热时间<br>
* SubIndex 7 - 加热功率百分比<br>
* SubIndex 8 - 运算周期<br>
* SubIndex 9 - 比例增益<br>
* SubIndex 10 - 积分时间<br>
* SubIndex 11 - 微分时间<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x8001[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 温度设置值 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 温度补偿值 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 待机温度 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex4 - 温度上限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex5 - 温度下限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex6 - 最小加热时间 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex7 - 加热功率百分比 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex8 - 运算周期 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex9 - 比例增益 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex10 - 积分时间 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex11 - 微分时间 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x8001[] = "Number of Entries\000"
"温度设置值\000"
"温度补偿值\000"
"待机温度\000"
"温度上限\000"
"温度下限\000"
"最小加热时间\000"
"加热功率百分比\000"
"运算周期\000"
"比例增益\000"
"积分时间\000"
"微分时间\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 温度设置值; /* Subindex1 - 温度设置值 */
INT16 温度补偿值; /* Subindex2 - 温度补偿值 */
INT16 待机温度; /* Subindex3 - 待机温度 */
INT16 温度上限; /* Subindex4 - 温度上限 */
INT16 温度下限; /* Subindex5 - 温度下限 */
INT16 最小加热时间; /* Subindex6 - 最小加热时间 */
INT16 加热功率百分比; /* Subindex7 - 加热功率百分比 */
INT16 运算周期; /* Subindex8 - 运算周期 */
INT16 比例增益; /* Subindex9 - 比例增益 */
INT16 积分时间; /* Subindex10 - 积分时间 */
INT16 微分时间; /* Subindex11 - 微分时间 */
} OBJ_STRUCT_PACKED_END
TOBJ8001;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ8001 NumberOfEntries0x8001
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={11,0x0003,0x00,0x5002,0x3003,0x0002,0x10,0x0001,0x50,0x50,0x10,0x05}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x8002 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x8002 0x8002 | Number of Entries
* @{
* \brief Object 0x8002 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 物理量下限<br>
* SubIndex 2 - 物理量上限<br>
* SubIndex 3 - 转换输出下限<br>
* SubIndex 4 - 转换输出上限<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x8002[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 物理量下限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 物理量上限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 转换输出下限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex4 - 转换输出上限 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x8002[] = "Number of Entries\000"
"物理量下限\000"
"物理量上限\000"
"转换输出下限\000"
"转换输出上限\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 物理量下限; /* Subindex1 - 物理量下限 */
INT16 物理量上限; /* Subindex2 - 物理量上限 */
INT16 转换输出下限; /* Subindex3 - 转换输出下限 */
INT16 转换输出上限; /* Subindex4 - 转换输出上限 */
} OBJ_STRUCT_PACKED_END
TOBJ8002;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ8002 NumberOfEntries0x8002
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={4,0xFE0C,0x0005,0x04,0x20}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x8003 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x8003 0x8003 | Number of Entries
* @{
* \brief Object 0x8003 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 物理量下限<br>
* SubIndex 2 - 物理量上限<br>
* SubIndex 3 - 转换输出下限<br>
* SubIndex 4 - 转换输出上限<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x8003[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 物理量下限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 物理量上限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 转换输出下限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex4 - 转换输出上限 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x8003[] = "Number of Entries\000"
"物理量下限\000"
"物理量上限\000"
"转换输出下限\000"
"转换输出上限\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 物理量下限; /* Subindex1 - 物理量下限 */
INT16 物理量上限; /* Subindex2 - 物理量上限 */
INT16 转换输出下限; /* Subindex3 - 转换输出下限 */
INT16 转换输出上限; /* Subindex4 - 转换输出上限 */
} OBJ_STRUCT_PACKED_END
TOBJ8003;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ8003 NumberOfEntries0x8003
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={4,0xFE0C,0x0005,0x04,0x20}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x8004 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x8004 0x8004 | Number of Entries
* @{
* \brief Object 0x8004 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 输入下限_X1<br>
* SubIndex 2 - 输入上限_X2<br>
* SubIndex 3 - 输出下限_Y1<br>
* SubIndex 4 - 输出上限_Y2<br>
* SubIndex 5 - 平均次数<br>
* SubIndex 6 - 补偿值<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x8004[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 输入下限_X1 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 输入上限_X2 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 输出下限_Y1 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex4 - 输出上限_Y2 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex5 - 平均次数 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex6 - 补偿值 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x8004[] = "Number of Entries\000"
"输入下限_X1\000"
"输入上限_X2\000"
"输出下限_Y1\000"
"输出上限_Y2\000"
"平均次数\000"
"补偿值\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 输入下限_X1; /* Subindex1 - 输入下限_X1 */
INT16 输入上限_X2; /* Subindex2 - 输入上限_X2 */
INT16 输出下限_Y1; /* Subindex3 - 输出下限_Y1 */
INT16 输出上限_Y2; /* Subindex4 - 输出上限_Y2 */
INT16 平均次数; /* Subindex5 - 平均次数 */
INT16 补偿值; /* Subindex6 - 补偿值 */
} OBJ_STRUCT_PACKED_END
TOBJ8004;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ8004 NumberOfEntries0x8004
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={6,0x00,0x20,0x00,0x000002,0x10,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x8005 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x8005 0x8005 | Number of Entries
* @{
* \brief Object 0x8005 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 输入下限_X1<br>
* SubIndex 2 - 输入上限_X2<br>
* SubIndex 3 - 输出下限_Y1<br>
* SubIndex 4 - 输出上限_Y2<br>
* SubIndex 5 - 平均次数<br>
* SubIndex 6 - 补偿值<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x8005[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 输入下限_X1 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 输入上限_X2 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 输出下限_Y1 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex4 - 输出上限_Y2 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex5 - 平均次数 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex6 - 补偿值 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x8005[] = "Number of Entries\000"
"输入下限_X1\000"
"输入上限_X2\000"
"输出下限_Y1\000"
"输出上限_Y2\000"
"平均次数\000"
"补偿值\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 输入下限_X1; /* Subindex1 - 输入下限_X1 */
INT16 输入上限_X2; /* Subindex2 - 输入上限_X2 */
INT16 输出下限_Y1; /* Subindex3 - 输出下限_Y1 */
INT16 输出上限_Y2; /* Subindex4 - 输出上限_Y2 */
INT16 平均次数; /* Subindex5 - 平均次数 */
INT16 补偿值; /* Subindex6 - 补偿值 */
} OBJ_STRUCT_PACKED_END
TOBJ8005;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ8005 NumberOfEntries0x8005
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={6,0x00,0x20,0x00,0x000002,0x10,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x8006 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x8006 0x8006 | Number of Entries
* @{
* \brief Object 0x8006 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 额定流量<br>
* SubIndex 2 - 启动转速<br>
* SubIndex 3 - 额定转速<br>
* SubIndex 4 - 启动转速对应模拟量<br>
* SubIndex 5 - 额定转速对应模拟量<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x8006[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 额定流量 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 启动转速 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 额定转速 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex4 - 启动转速对应模拟量 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex5 - 额定转速对应模拟量 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x8006[] = "Number of Entries\000"
"额定流量\000"
"启动转速\000"
"额定转速\000"
"启动转速对应模拟量\000"
"额定转速对应模拟量\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 额定流量; /* Subindex1 - 额定流量 */
INT16 启动转速; /* Subindex2 - 启动转速 */
INT16 额定转速; /* Subindex3 - 额定转速 */
INT16 启动转速对应模拟量; /* Subindex4 - 启动转速对应模拟量 */
INT16 额定转速对应模拟量; /* Subindex5 - 额定转速对应模拟量 */
} OBJ_STRUCT_PACKED_END
TOBJ8006;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ8006 NumberOfEntries0x8006
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={5,0x0001,0x0001,0x0050,0x0006,0x0045}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x8007 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x8007 0x8007 | Number of Entries
* @{
* \brief Object 0x8007 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 额定流量<br>
* SubIndex 2 - 启动转速<br>
* SubIndex 3 - 额定转速<br>
* SubIndex 4 - 启动转速对应模拟量<br>
* SubIndex 5 - 额定转速对应模拟量<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x8007[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 额定流量 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 启动转速 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 额定转速 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex4 - 启动转速对应模拟量 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex5 - 额定转速对应模拟量 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x8007[] = "Number of Entries\000"
"额定流量\000"
"启动转速\000"
"额定转速\000"
"启动转速对应模拟量\000"
"额定转速对应模拟量\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 额定流量; /* Subindex1 - 额定流量 */
INT16 启动转速; /* Subindex2 - 启动转速 */
INT16 额定转速; /* Subindex3 - 额定转速 */
INT16 启动转速对应模拟量; /* Subindex4 - 启动转速对应模拟量 */
INT16 额定转速对应模拟量; /* Subindex5 - 额定转速对应模拟量 */
} OBJ_STRUCT_PACKED_END
TOBJ8007;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ8007 NumberOfEntries0x8007
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={5,0x0001,0x0001,0x0050,0x0006,0x0045}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x8008 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x8008 0x8008 | Number of Entries
* @{
* \brief Object 0x8008 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 数字量下限X1<br>
* SubIndex 2 - 数字量上限X2<br>
* SubIndex 3 - 物理量下限_Y1<br>
* SubIndex 4 - 物理量上限_Y2<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x8008[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 数字量下限X1 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 数字量上限X2 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 物理量下限_Y1 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex4 - 物理量上限_Y2 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x8008[] = "Number of Entries\000"
"数字量下限X1\000"
"数字量上限X2\000"
"物理量下限_Y1\000"
"物理量上限_Y2\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 数字量下限X1; /* Subindex1 - 数字量下限X1 */
INT16 数字量上限X2; /* Subindex2 - 数字量上限X2 */
INT16 物理量下限_Y1; /* Subindex3 - 物理量下限_Y1 */
INT16 物理量上限_Y2; /* Subindex4 - 物理量上限_Y2 */
} OBJ_STRUCT_PACKED_END
TOBJ8008;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ8008 NumberOfEntries0x8008
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={4,0x00,0x000001,0x00,0x10}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x8009 : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x8009 0x8009 | Number of Entries
* @{
* \brief Object 0x8009 (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 数字量下限X1<br>
* SubIndex 2 - 数字量上限X2<br>
* SubIndex 3 - 物理量下限_Y1<br>
* SubIndex 4 - 物理量上限_Y2<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x8009[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 数字量下限X1 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 数字量上限X2 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 物理量下限_Y1 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex4 - 物理量上限_Y2 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x8009[] = "Number of Entries\000"
"数字量下限X1\000"
"数字量上限X2\000"
"物理量下限_Y1\000"
"物理量上限_Y2\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 数字量下限X1; /* Subindex1 - 数字量下限X1 */
INT16 数字量上限X2; /* Subindex2 - 数字量上限X2 */
INT16 物理量下限_Y1; /* Subindex3 - 物理量下限_Y1 */
INT16 物理量上限_Y2; /* Subindex4 - 物理量上限_Y2 */
} OBJ_STRUCT_PACKED_END
TOBJ8009;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ8009 NumberOfEntries0x8009
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={4,0x00,0x000001,0x00,0x10}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x800A : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x800A 0x800A | Number of Entries
* @{
* \brief Object 0x800A (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 运算周期<br>
* SubIndex 2 - 比例增益<br>
* SubIndex 3 - 积分增益<br>
* SubIndex 4 - 微分增益<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x800A[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 运算周期 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 比例增益 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 积分增益 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex4 - 微分增益 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x800A[] = "Number of Entries\000"
"运算周期\000"
"比例增益\000"
"积分增益\000"
"微分增益\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 运算周期; /* Subindex1 - 运算周期 */
INT16 比例增益; /* Subindex2 - 比例增益 */
INT16 积分增益; /* Subindex3 - 积分增益 */
INT16 微分增益; /* Subindex4 - 微分增益 */
} OBJ_STRUCT_PACKED_END
TOBJ800A;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ800A NumberOfEntries0x800A
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={4,0x50,0x50,0x10,0x05}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x800B : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x800B 0x800B | Number of Entries
* @{
* \brief Object 0x800B (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 运算周期<br>
* SubIndex 2 - 比例增益<br>
* SubIndex 3 - 积分增益<br>
* SubIndex 4 - 微分增益<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x800B[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 运算周期 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 比例增益 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 积分增益 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex4 - 微分增益 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x800B[] = "Number of Entries\000"
"运算周期\000"
"比例增益\000"
"积分增益\000"
"微分增益\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 运算周期; /* Subindex1 - 运算周期 */
INT16 比例增益; /* Subindex2 - 比例增益 */
INT16 积分增益; /* Subindex3 - 积分增益 */
INT16 微分增益; /* Subindex4 - 微分增益 */
} OBJ_STRUCT_PACKED_END
TOBJ800B;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ800B NumberOfEntries0x800B
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={4,0x50,0x50,0x10,0x05}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x800C : Number of Entries
******************************************************************************/
/**
* \addtogroup 0x800C 0x800C | Number of Entries
* @{
* \brief Object 0x800C (Number of Entries) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - 填墨速度<br>
* SubIndex 2 - 填墨时间<br>
* SubIndex 3 - 压墨时间<br>
* SubIndex 4 - 延迟填墨时间<br>
* SubIndex 5 - Pm<br>
* SubIndex 6 - Fl<br>
* SubIndex 7 - DP<br>
* SubIndex 8 - Ph<br>
* SubIndex 9 - Ph2<br>
* SubIndex 10 - If<br>
* SubIndex 11 - 最大允许流量差<br>
* SubIndex 12 - 墨盒液位上限<br>
* SubIndex 13 - 墨盒液位下限<br>
* SubIndex 14 - 供墨泵流量上限<br>
* SubIndex 15 - 回墨泵流量上限<br>
* SubIndex 16 - 补墨时间<br>
* SubIndex 17 - 收墨时间<br>
* SubIndex 18 - 压墨压力<br>
* SubIndex 19 - 待机DP<br>
* SubIndex 20 - 待机Pm<br>
* SubIndex 21 - 备用<br>
* SubIndex 22 - 备用<br>
* SubIndex 23 - 备用<br>
* SubIndex 24 - 备用<br>
* SubIndex 25 - 备用<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x800C[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex1 - 填墨速度 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex2 - 填墨时间 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex3 - 压墨时间 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex4 - 延迟填墨时间 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex5 - Pm */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex6 - Fl */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex7 - DP */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex8 - Ph */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex9 - Ph2 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex10 - If */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex11 - 最大允许流量差 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex12 - 墨盒液位上限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex13 - 墨盒液位下限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex14 - 供墨泵流量上限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex15 - 回墨泵流量上限 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex16 - 补墨时间 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex17 - 收墨时间 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex18 - 压墨压力 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex19 - 待机DP */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex20 - 待机Pm */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex21 - 备用 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex22 - 备用 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex23 - 备用 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }, /* Subindex24 - 备用 */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE }}; /* Subindex25 - 备用 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x800C[] = "Number of Entries\000"
"填墨速度\000"
"填墨时间\000"
"压墨时间\000"
"延迟填墨时间\000"
"Pm\000"
"Fl\000"
"DP\000"
"Ph\000"
"Ph2\000"
"If\000"
"最大允许流量差\000"
"墨盒液位上限\000"
"墨盒液位下限\000"
"供墨泵流量上限\000"
"回墨泵流量上限\000"
"补墨时间\000"
"收墨时间\000"
"压墨压力\000"
"待机DP\000"
"待机Pm\000"
"备用\000"
"备用\000"
"备用\000"
"备用\000"
"备用\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT16 填墨速度; /* Subindex1 - 填墨速度 */
INT16 填墨时间; /* Subindex2 - 填墨时间 */
INT16 压墨时间; /* Subindex3 - 压墨时间 */
INT16 延迟填墨时间; /* Subindex4 - 延迟填墨时间 */
INT16 Pm; /* Subindex5 - Pm */
INT16 Fl; /* Subindex6 - Fl */
INT16 DP; /* Subindex7 - DP */
INT16 Ph; /* Subindex8 - Ph */
INT16 Ph2; /* Subindex9 - Ph2 */
INT16 If; /* Subindex10 - If */
INT16 最大允许流量差; /* Subindex11 - 最大允许流量差 */
INT16 墨盒液位上限; /* Subindex12 - 墨盒液位上限 */
INT16 墨盒液位下限; /* Subindex13 - 墨盒液位下限 */
INT16 供墨泵流量上限; /* Subindex14 - 供墨泵流量上限 */
INT16 回墨泵流量上限; /* Subindex15 - 回墨泵流量上限 */
INT16 补墨时间; /* Subindex16 - 补墨时间 */
INT16 收墨时间; /* Subindex17 - 收墨时间 */
INT16 压墨压力; /* Subindex18 - 压墨压力 */
INT16 待机DP; /* Subindex19 - 待机DP */
INT16 待机Pm; /* Subindex20 - 待机Pm */
INT16 备用; /* Subindex21 - 备用 */
INT16 备用; /* Subindex22 - 备用 */
INT16 备用; /* Subindex23 - 备用 */
INT16 备用; /* Subindex24 - 备用 */
INT16 备用; /* Subindex25 - 备用 */
} OBJ_STRUCT_PACKED_END
TOBJ800C;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ800C NumberOfEntries0x800C
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={25,0x50,0x0010,0x0005,0x0002,0x0010,0x50,0x0002,0x0005,0x0005,0x0005,0x05,0x0008,0x0002,0x2001,0x2001,0x0005,0x0005,0x0010,0x5001,0x0008,0x00,0x00,0x00,0x00,0x00}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0xF000 : Modular Device Profile
******************************************************************************/
/**
* \addtogroup 0xF000 0xF000 | Modular Device Profile
* @{
* \brief Object 0xF000 (Modular Device Profile) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Index distance <br>
* SubIndex 2 - Maximum number of modules <br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0xF000[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }, /* Subindex1 - Index distance  */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }}; /* Subindex2 - Maximum number of modules  */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0xF000[] = "Modular Device Profile\000"
"Index distance \000"
"Maximum number of modules \000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_INKCONTROL_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT16 IndexDistance; /* Subindex1 - Index distance  */
UINT16 MaximumNumberOfModules; /* Subindex2 - Maximum number of modules  */
} OBJ_STRUCT_PACKED_END
TOBJF000;
#endif //#ifndef _SSC_INKCONTROL_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJF000 ModularDeviceProfile0xF000
#if defined(_SSC_INKCONTROL_) && (_SSC_INKCONTROL_ == 1)
={2,0x10,0}
#endif
;
/** @}*/







#ifdef _OBJD_
TOBJECT    OBJMEM ApplicationObjDic[] = {
/* Object 0x1600 */
{NULL , NULL ,  0x1600 , {DEFTYPE_PDOMAPPING , 2 | (OBJCODE_REC << 8)} , asEntryDesc0x1600 , aName0x1600 , &NumberOfEntriesProcessDataMapping0x1600, NULL , NULL , 0x0000 },
/* Object 0x1A00 */
{NULL , NULL ,  0x1A00 , {DEFTYPE_PDOMAPPING , 25 | (OBJCODE_REC << 8)} , asEntryDesc0x1A00 , aName0x1A00 , &InputMapping00x1A00, NULL , NULL , 0x0000 },
/* Object 0x1C12 */
{NULL , NULL ,  0x1C12 , {DEFTYPE_UNSIGNED16 , 1 | (OBJCODE_ARR << 8)} , asEntryDesc0x1C12 , aName0x1C12 , &sRxPDOassign, NULL , NULL , 0x0000 },
/* Object 0x1C13 */
{NULL , NULL ,  0x1C13 , {DEFTYPE_UNSIGNED16 , 1 | (OBJCODE_ARR << 8)} , asEntryDesc0x1C13 , aName0x1C13 , &sTxPDOassign, NULL , NULL , 0x0000 },
/* Object 0x6000 */
{NULL , NULL ,  0x6000 , {DEFTYPE_UNSIGNED8 , 2 | (OBJCODE_REC << 8)} , asEntryDesc0x6000 , aName0x6000 , &NumberOfEntries0x6000, NULL , NULL , 0x0000 },
/* Object 0x6001 */
{NULL , NULL ,  0x6001 , {DEFTYPE_UNSIGNED8 , 2 | (OBJCODE_REC << 8)} , asEntryDesc0x6001 , aName0x6001 , &NumberOfEntries0x6001, NULL , NULL , 0x0000 },
/* Object 0x6002 */
{NULL , NULL ,  0x6002 , {DEFTYPE_UNSIGNED8 , 2 | (OBJCODE_REC << 8)} , asEntryDesc0x6002 , aName0x6002 , &NumberOfEntries0x6002, NULL , NULL , 0x0000 },
/* Object 0x6003 */
{NULL , NULL ,  0x6003 , {DEFTYPE_UNSIGNED8 , 2 | (OBJCODE_REC << 8)} , asEntryDesc0x6003 , aName0x6003 , &NumberOfEntries0x6003, NULL , NULL , 0x0000 },
/* Object 0x6004 */
{NULL , NULL ,  0x6004 , {DEFTYPE_UNSIGNED8 , 3 | (OBJCODE_REC << 8)} , asEntryDesc0x6004 , aName0x6004 , &NumberOfEntries0x6004, NULL , NULL , 0x0000 },
/* Object 0x6005 */
{NULL , NULL ,  0x6005 , {DEFTYPE_UNSIGNED8 , 2 | (OBJCODE_REC << 8)} , asEntryDesc0x6005 , aName0x6005 , &NumberOfEntries0x6005, NULL , NULL , 0x0000 },
/* Object 0x6006 */
{NULL , NULL ,  0x6006 , {DEFTYPE_UNSIGNED8 , 2 | (OBJCODE_REC << 8)} , asEntryDesc0x6006 , aName0x6006 , &NumberOfEntries0x6006, NULL , NULL , 0x0000 },
/* Object 0x6007 */
{NULL , NULL ,  0x6007 , {DEFTYPE_UNSIGNED8 , 10 | (OBJCODE_REC << 8)} , asEntryDesc0x6007 , aName0x6007 , &NumberOfEntries0x6007, NULL , NULL , 0x0000 },
/* Object 0x7000 */
{NULL , NULL ,  0x7000 , {DEFTYPE_UNSIGNED8 , 2 | (OBJCODE_REC << 8)} , asEntryDesc0x7000 , aName0x7000 , &NumberOfEntries0x7000, NULL , NULL , 0x0000 },
/* Object 0x8000 */
{NULL , NULL ,  0x8000 , {DEFTYPE_UNSIGNED8 , 11 | (OBJCODE_REC << 8)} , asEntryDesc0x8000 , aName0x8000 , &NumberOfEntries0x8000, NULL , NULL , 0x0000 },
/* Object 0x8001 */
{NULL , NULL ,  0x8001 , {DEFTYPE_UNSIGNED8 , 11 | (OBJCODE_REC << 8)} , asEntryDesc0x8001 , aName0x8001 , &NumberOfEntries0x8001, NULL , NULL , 0x0000 },
/* Object 0x8002 */
{NULL , NULL ,  0x8002 , {DEFTYPE_UNSIGNED8 , 4 | (OBJCODE_REC << 8)} , asEntryDesc0x8002 , aName0x8002 , &NumberOfEntries0x8002, NULL , NULL , 0x0000 },
/* Object 0x8003 */
{NULL , NULL ,  0x8003 , {DEFTYPE_UNSIGNED8 , 4 | (OBJCODE_REC << 8)} , asEntryDesc0x8003 , aName0x8003 , &NumberOfEntries0x8003, NULL , NULL , 0x0000 },
/* Object 0x8004 */
{NULL , NULL ,  0x8004 , {DEFTYPE_UNSIGNED8 , 6 | (OBJCODE_REC << 8)} , asEntryDesc0x8004 , aName0x8004 , &NumberOfEntries0x8004, NULL , NULL , 0x0000 },
/* Object 0x8005 */
{NULL , NULL ,  0x8005 , {DEFTYPE_UNSIGNED8 , 6 | (OBJCODE_REC << 8)} , asEntryDesc0x8005 , aName0x8005 , &NumberOfEntries0x8005, NULL , NULL , 0x0000 },
/* Object 0x8006 */
{NULL , NULL ,  0x8006 , {DEFTYPE_UNSIGNED8 , 5 | (OBJCODE_REC << 8)} , asEntryDesc0x8006 , aName0x8006 , &NumberOfEntries0x8006, NULL , NULL , 0x0000 },
/* Object 0x8007 */
{NULL , NULL ,  0x8007 , {DEFTYPE_UNSIGNED8 , 5 | (OBJCODE_REC << 8)} , asEntryDesc0x8007 , aName0x8007 , &NumberOfEntries0x8007, NULL , NULL , 0x0000 },
/* Object 0x8008 */
{NULL , NULL ,  0x8008 , {DEFTYPE_UNSIGNED8 , 4 | (OBJCODE_REC << 8)} , asEntryDesc0x8008 , aName0x8008 , &NumberOfEntries0x8008, NULL , NULL , 0x0000 },
/* Object 0x8009 */
{NULL , NULL ,  0x8009 , {DEFTYPE_UNSIGNED8 , 4 | (OBJCODE_REC << 8)} , asEntryDesc0x8009 , aName0x8009 , &NumberOfEntries0x8009, NULL , NULL , 0x0000 },
/* Object 0x800A */
{NULL , NULL ,  0x800A , {DEFTYPE_UNSIGNED8 , 4 | (OBJCODE_REC << 8)} , asEntryDesc0x800A , aName0x800A , &NumberOfEntries0x800A, NULL , NULL , 0x0000 },
/* Object 0x800B */
{NULL , NULL ,  0x800B , {DEFTYPE_UNSIGNED8 , 4 | (OBJCODE_REC << 8)} , asEntryDesc0x800B , aName0x800B , &NumberOfEntries0x800B, NULL , NULL , 0x0000 },
/* Object 0x800C */
{NULL , NULL ,  0x800C , {DEFTYPE_UNSIGNED8 , 25 | (OBJCODE_REC << 8)} , asEntryDesc0x800C , aName0x800C , &NumberOfEntries0x800C, NULL , NULL , 0x0000 },
/* Object 0xF000 */
{NULL , NULL ,  0xF000 , {DEFTYPE_RECORD , 2 | (OBJCODE_REC << 8)} , asEntryDesc0xF000 , aName0xF000 , &ModularDeviceProfile0xF000, NULL , NULL , 0x0000 },
{NULL,NULL, 0xFFFF, {0, 0}, NULL, NULL, NULL, NULL}};
#endif    //#ifdef _OBJD_
#undef PROTO

/** @}*/
#define _SSC_INKCONTROL_OBJECTS_H_
