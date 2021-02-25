#ifndef BX_SYS_CONFIG_H_
#define BX_SYS_CONFIG_H_
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "bx_app_config.h"
/*
 * DEFINES
 ****************************************************************************************
 */
#ifndef HW_BX_VERSION
#define HW_BX_VERSION                                       00
#endif  /*CFG_APP_DIS*/

#ifndef BLE_USER_CONN_NUM
#define BLE_USER_CONN_NUM                                   4
#endif

#ifndef HW_ECC_PRESENT
#define HW_ECC_PRESENT                                      1
#endif/* HW_ECC_PRESENT */

#ifndef CFG_FREERTOS_SUPPORT
#define CFG_FREERTOS_SUPPORT                                1
#endif /*CFG_FREERTOS_SUPPORT*/

#ifndef CFG_RF_APOLLO
#define CFG_RF_APOLLO                                       1
#endif /*CFG_RF_APOLLO*/

#ifndef CFG_ON_CHIP
#define CFG_ON_CHIP                                         1
#endif /*CFG_ON_CHIP*/

#ifndef CFG_SYS_LOG
#define CFG_SYS_LOG                                         1
#endif /*CFG_SYS_LOG*/

#ifndef CFG_DYNAMIC_UPDATE
#define CFG_DYNAMIC_UPDATE                                  1
#endif /*CFG_DYNAMIC_UPDATE*/

#ifndef ENABLE_CANNEL_CONN_PARA_UPD_FEATURE_PATCH
#define ENABLE_CANNEL_CONN_PARA_UPD_FEATURE_PATCH           0
#endif /*ENABLE_CANNEL_CONN_PARA_UPD_FEATURE_PATCH*/

#ifndef ENABLE_LLC_CON_UPD_REQ_IND_HANDLER_PATCH
#define ENABLE_LLC_CON_UPD_REQ_IND_HANDLER_PATCH            0
#endif /*ENABLE_LLC_CON_UPD_REQ_IND_HANDLER_PATCH*/

#ifndef ENABLE_ADV_PAYLOD_31BYTE_PATCH
#define ENABLE_ADV_PAYLOD_31BYTE_PATCH                      0
#endif /*ENABLE_ADV_PAYLOD_31BYTE_PATCH*/

#ifndef PATCH_SKIP_H4TL_READ_START
#define PATCH_SKIP_H4TL_READ_START                          0
#endif /*PATCH_SKIP_H4TL_READ_START*/

#ifndef TX_TEST_PACKET_NUM_PATCH
#define TX_TEST_PACKET_NUM_PATCH            1
#endif

#ifndef MESH_SCHED_PATCH
#define MESH_SCHED_PATCH                   0
#endif

#ifndef BX_VERF
#define BX_VERF                                             0
#endif /*BX_VERF*/

#ifndef VBAT_MILLIVOLT
#define VBAT_MILLIVOLT                                      3300
#endif /*VBAT_MILLIVOLT*/

#ifndef VDD_AWO_SLEEP_MILLIVOLT
#define VDD_AWO_SLEEP_MILLIVOLT                             800 // 950/900/850/800
#endif /*VDD_AWO_SLEEP_MILLIVOLT*/

#ifndef VDD_SRAM_SLEEP_MILLIVOLT
#define VDD_SRAM_SLEEP_MILLIVOLT                            650 // 950/900/850/800/750/700/650/600
#endif /*VDD_SRAM_SLEEP_MILLIVOLT*/

#ifndef FLASH_XIP
#define FLASH_XIP                                           1
#endif /*FLASH_XIP*/

#ifndef LOCAL_NVDS
#define LOCAL_NVDS                                          1
#endif /*LOCAL_NVDS*/

#ifndef RC32K_USED
#define RC32K_USED  0
#endif

#ifndef SYSTICK_USED
#define SYSTICK_USED  0
#endif

#ifndef MODEM_DYNAMIC_DERIV_CALIB
#define MODEM_DYNAMIC_DERIV_CALIB 0
#endif

#ifndef MAIN_CLOCK
//#define MAIN_CLOCK                                        96000000
#define MAIN_CLOCK                                          32000000
//#define MAIN_CLOCK                                        16000000
#endif /*MAIN_CLOCK*/

#ifndef RF_PARAM
//#define RF_PARAM                                            0 // original configuration
//#define RF_PARAM                                            1 // case 13.3
//#define RF_PARAM                                            2 // case 17.5
#define RF_PARAM                                            3 // all case
#endif

#if RF_PARAM == 3
#define AUTO_CAL 1         
#endif

#if RF_PARAM == 0
//#define RF_TX_POWER_8DBM         0x0
//#define RF_TX_POWER_N2DBM        0x1
#elif RF_PARAM == 1 || RF_PARAM == 2 || RF_PARAM == 3
//#define RF_TX_POWER_2DBM_DCDC_ON         0x1
//#define RF_TX_POWER_2DBM_DCDC_OFF_3V3    0x2
//#define RF_TX_POWER_2DBM_DCDC_OFF_1V8    0x3
//#define RF_TX_POWER_0DBM_DCDC_ON         0x4
//#define RF_TX_POWER_N2DBM_DCDC_ON        0x5
//#define RF_TX_POWER_4DBM_DCDC_OFF_3V3    0x6
//#define RF_TX_POWER_8DBM_DCDC_OFF_3V3    0x7
#endif
  
#ifndef RF_TX_POWER
#define RF_TX_POWER                        0x5
#endif


/*------------- NVDS ---------- */
#ifndef BX_DEV_NAME
#define BX_DEV_NAME                                         "APOLLO"
#endif  /*BX_DEV_NAME*/

#ifndef BLE_SOFT_WAKEUP_TIME
#define BLE_SOFT_WAKEUP_TIME                                130
#endif /* BLE_SOFT_WAKEUP_TIME */

#ifndef BLE_WAKEUP_TIME
#define BLE_WAKEUP_TIME                                     1700  // mininum:1200(code in ram)
#endif  /* BLE_WAKEUP_TIME */

#ifndef DEEP_SLEEP_ENABLE
#define DEEP_SLEEP_ENABLE                                   1
#endif /*DEEP_SLEEP_ENABLE*/

#ifndef EXT_WAKE_UP_ENABLE
#define EXT_WAKE_UP_ENABLE                                  1
#endif /*EXT_WAKE_UP_ENABLE*/

#if(RC32K_USED == 0)
#ifndef LPCLK_DRIFT_MAX
#define LPCLK_DRIFT_MAX                                     20
#endif
#else
#define LPCLK_DRIFT_MAX                                     500
#endif

/*------------- Debug ---------- */
#ifndef DEBUGGER_ATTACHED
#define DEBUGGER_ATTACHED                                   1
#endif /*DEBUGGER_ATTACHED*/

//#define DO_NOT_WAKEUP

#ifndef FREERTOS_WAKEUP_DELAY
#define FREERTOS_WAKEUP_DELAY                               (BLE_WAKEUP_TIME - 200)
#endif /*FREERTOS_WAKEUP_DELAY*/

#ifndef XTAL_STARTUP_TIME
#define XTAL_STARTUP_TIME                                   10
#define XTAL_STARTUP_TIME_50                                50
#endif /*XTAL_STARTUP_TIME*/

#ifndef LDO_3V1_OUTPUT_SLEEP_RET
#define LDO_3V1_OUTPUT_SLEEP_RET                            1
#endif /*LDO_3V1_OUTPUT_SLEEP_RET*/

#ifndef LDO_1V8_OUTPUT_SLEEP_RET
#define LDO_1V8_OUTPUT_SLEEP_RET                            1
#endif

#ifndef VDD_1V8_SLEEP_LDO1
#define VDD_1V8_SLEEP_LDO1                                  1
#endif /*VDD_1V8_SLEEP_LDO1*/

#define AFTER_MODIFY_PARAM  1
//#define AFTER_MODIFY_PARAM  0

#ifndef DIG_VOLTAGE_CTRL_BY_RF_REG
#if AFTER_MODIFY_PARAM
#define DIG_VOLTAGE_CTRL_BY_RF_REG                          0
#else
#define DIG_VOLTAGE_CTRL_BY_RF_REG                          1
#endif
#endif /*DIG_VOLTAGE_CTRL_BY_RF_REG*/

#ifndef RUN_WITHOUT_SLEEP
#define RUN_WITHOUT_SLEEP                                   0
#endif /*DIG_VOLTAGE_CTRL_BY_RF_REG*/

#if (defined(RUN_WITHOUT_SLEEP)&&(RUN_WITHOUT_SLEEP==1))
#undef DEEP_SLEEP_ENABLE
#define DEEP_SLEEP_ENABLE                                   0
#endif

#if (MAIN_CLOCK>32000000)
#define CPU_LDO_BYPASS                                      0
#endif/*RUN_WITHOUT_SLEEP*/

#if (defined(CFG_DYNAMIC_UPDATE)&&(CFG_DYNAMIC_UPDATE==1))
#define BATTERY_VOLTAGE_UPDATE_SECONDS                      10
#define TEMPERATURE_UPDATE_SECONDS                          10
#endif/*CFG_DYNAMIC_UPDATE*/

#ifndef BYPASS_VOLTAGE
#define BYPASS_VOLTAGE                                      3400
#endif /*BYPASS_VOLTAGE*/

#ifndef BX_BATTERY_MONITOR
#define BX_BATTERY_MONITOR                                  0
#endif



#if (defined BX_BATTERY_MONITOR) && (BX_BATTERY_MONITOR == 1)
#define APP_BAT_TIMER_MS                                    (20*1000) // Battery monitor timer interval
#endif

#ifndef BX_TEMP_SENSOR
#define BX_TEMP_SENSOR                                      0
#endif

#if (defined BX_TEMP_SENSOR) && (BX_TEMP_SENSOR == 1)
#define APP_TEMP_TIMER_MS                                   (30*1000) // Temperature sensor timer interval
#endif

//define for APOLLO_00 Version
#define APOLLO_00_V1A                                       0x0
#define APOLLO_00_V1B                                       0x1
#define APOLLO_00_V2A                                       0x2
#define APOLLO_00_V2B                                       0x3
#define APOLLO_00_V3A                                       0x4
#define APOLLO_00_V3B                                       0x5
#define APOLLO_00_V4A_V4AMC                                 0x6
#define APOLLO_00_V4B_V4BMC                                 0x7
#define APOLLO_00_A01                                       0x8
#define APOLLO_00_A02                                       0x9
#define APOLLO_00_A03                                       0xA
#define APOLLO_00_A04                                       0xB
#define APOLLO_00_A05                                       0xC
#define APOLLO_00_A06                                       0xD
#define APOLLO_00_A07                                       0xE
#define APOLLO_00_A08                                       0xF
#define APOLLO_00_A09                                       0x10
#define APOLLO_00_A10                                       0x11
#define APOLLO_00_A11                                       0x12
#define APOLLO_00_A12                                       0x13
#define APOLLO_00_B1                                        0x14
#define APOLLO_00_B2                                        0x15
#define APOLLO_00_B3                                        0x16

#endif/*BX_SYS_CONFIG_H_*/
