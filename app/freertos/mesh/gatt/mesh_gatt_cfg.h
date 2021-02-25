/**
 ****************************************************************************************
 *
 * @file mesh_gatt_cfg.h
 *
 * @brief BLE Mesh Gatt Config Internal.
 *
 * Copyright (C) Apollo 2018-2023
 *
 ****************************************************************************************
 */
#ifndef FREERTOS_APP_MESH_GATT_MESH_GATT_CFG_H_
#define FREERTOS_APP_MESH_GATT_MESH_GATT_CFG_H_

/**
 ****************************************************************************************
 * @addtogroup BLE_MESH_GATT_CONFIG  BLE Mesh Gatt Config Internal
 * @ingroup BLE_MESH_GATT
 * @brief defines for BLE mesh gatt  config
 *
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"
#include <stdbool.h>
/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/** Maximum number of addresses in the GATT proxy address filter, per connection. */
#ifndef MESH_GATT_PROXY_FILTER_ADDR_MAX
#define MESH_GATT_PROXY_FILTER_ADDR_MAX 32 //(2N) = (65-1)/2 = 32
#endif


/*
 * INTERNAL API TYPES
 ****************************************************************************************
 */
/**
 * @brief struct to mesh gatt config information
 */
//struct mesh_gatt_cfg_t
//{
//    /// valid or not
//    uint8_t valid;
//    /// store connect index
//    uint16_t conn_idx;
//    /// store the master address
//    bd_addr_t device_addr;
//};
///**
// * @brief struct to mesh gatt config information
// */
//struct mesh_gatt_env_t
//{
//    /// valid or not
//    uint8_t valid;
//    /// store connect index
//    uint16_t conn_idx;
//    /// store the master address
//    bd_addr_t device_addr;
//};
/*
 * MACROS
 ****************************************************************************************
 */
/// Macro used to retrieve field

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/// BX24XX Peripheral Service Table
//enum
//{
//    BX24XX_SIMPLES_SERVICE_ID,
//
//
//    BLE_PERIPHERAL_SERVICES_NUM,
//};






/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */







/// @} BLE_MESH_GATT_CONFIG

#endif /* FREERTOS_APP_MESH_GATT_MESH_GATT_CFG_H_ */
