/**
 ****************************************************************************************
 *
 * @file sdk_mesh_config_pro.h
 *
 * @brief Config BLE Mesh SDK Proficient Parameter.
 *
 * Copyright (C) Apollo 2018-2023
 *
 ****************************************************************************************
 */

#ifndef FREERTOS_APP_MESH_CORE_SDK_MESH_CONFIG_PRO_H_
#define FREERTOS_APP_MESH_CORE_SDK_MESH_CONFIG_PRO_H_

/**
 ****************************************************************************************
 * @addtogroup MESH_SDK_CFG_PRO_API
 * @ingroup  MESH_SDK_CFG_API
 * @brief defines for BLE MESH sdk config proficient params
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */


///< the element number in this node.
#define ELEMENT_NUM 2

 /**
  @defgroup MESH_ERRORS_ALLOC Mesh Global Error Codes
  @{

  @brief Global Error definitions
*/
#define MESH_CORE_ERROR_ALLOC  (0)    ///< Global CORE error alloc
#define MESH_GATT_ERROR_ALLOC  (0x20) ///< Global GATT error alloc
#define MESH_DM_ERROR_ALLOC    (0x40) ///< Global Device manager error alloc
/**
  @}
*/


 /**
  @defgroup MESH_NETWORK_CONFIG  Mesh Global Network config params
  @{

  @brief Network config params definitions
*/
#define NETWORK_MATCHED_KEY_MAX_NUM 3 ///< max network keys matched in cache.
#define DEVKEY_BOUND_NETKEY_MAX_NUM 3 ///< max network keys bound to devkey.
#define APPKEY_BOUND_NETKEY_MAX_NUM 3 ///< max network keys bound to appkey.

#define MODEL_SUNSCRIPTION_LENGTH   3 ///< max subscription length to a model
/**
  @}
*/


 /**
  @defgroup MESH_COMPANY_INFORMATION  Mesh company information
  @{
  @brief Mesh company information (in config composition data message)
*/
#define MESH_PARAM_CID      0x01A8      ///<company identifier
#define MESH_PARAM_PID      0x0000      ///<product identifier
#define MESH_PARAM_VID      0x0001      ///<product version identifier
#define MESH_PARAM_CRPL     0x0064      ///<the minimum number of replay protection list entries in a device
#define MESH_PARAM_LOC      0x0100      ///<location descriptor
/**
  @}
*/
 /**
  @defgroup MESH_DEVICE_MANAGER_CONFIG  Mesh Global device manager config params
  @{

  @brief device manager config params definitions
*/
#define DM_CFG_DEVKEY_MAX_NUM 1 ///< device manager buffer max device keys.
#define DM_CFG_NETKEY_MAX_NUM 3 ///< device manager buffer max network keys.
#define DM_CFG_APPKEY_MAX_NUM 3 ///< device manager buffer max application keys.
/**
  @}
*/


 /**
  @defgroup MESH_NETWORK_CONFIG mesh  mesh network parameters configuration
  @{
  @brief default value of adv_transmit_state_t
*/
//note:count max    = 0b111   = 7
//note:interval max = 0b11111 = 31
#define TRANSMIT_DEFAULT_COUNT                  1   ///send 6 times  (count + 1)
#define TRANSMIT_DEFAULT_INTERVAL               0  ///interval = (step +1)*10 ms
#define TRANSMIT_DEFAULT_RELAY_COUNT            3   ///send 7 times
#define TRANSMIT_DEFAULT_RELAY_INTERVAL         9  ///interval = (step +1)*10 ms
#define TRANSMIT_PROXY_SERVICE_COUNT            4   ///send 1 times
#define TRANSMIT_PROXY_SERVICE_INTERVALT        9  ///interval = 10*10ms=100ms
#define TRANSMIT_PROXY_SERVICE_PERIOD           1000///ADV period 1000ms  unit 1ms
#define UPROV_BEACON_DEFAULT_COUNT              1   ///send 6 times  (count + 1)
#define UPROV_BEACON_DEFAULT_INTERVAL           0  ///interval = (step +1)*10 ms 
#define UPROV_BEACON_DEFAULT_PERIOD             100  ///ADV period 1000ms  unit 1ms
#define PB_GATT_DEFAULT_COUNT                   2   ///send 6 times  (count + 1)
#define PB_GATT_DEFAULT_INTERVAL                5  ///interval = (step +1)*10 ms 
#define PB_GATT_DEFAULT_PERIOD                  1000  ///ADV period 1000ms  unit 1ms
/**
  @}
*/

#define MESH_TOOLS_CFG_UART_CTRL                0//disable uart ctrl tool
#define MESH_TOOLS_CFG_RELAY_TEST               0//enable relay ctrl by pin check
#define MESH_TOOLS_TTL_TEST                     0//TTL DEBUG
#endif /* FREERTOS_APP_MESH_CORE_SDK_MESH_CONFIG_PRO_H_ */
/// @} MESH_SDK_CFG_PRO_API
