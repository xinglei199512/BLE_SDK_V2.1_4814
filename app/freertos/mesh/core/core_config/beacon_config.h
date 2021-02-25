/**
 ****************************************************************************************
 *
 * @file   beacon_config.h
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2018-10-08 14:32
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_beacon_config_API Mesh beacon_config API
 * @ingroup MESH_API
 * @brief Mesh beacon_config  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef FREERTOS_APP_MESH_CORE_CORE_CONFIG_BEACON_CONFIG_H_
#define FREERTOS_APP_MESH_CORE_CORE_CONFIG_BEACON_CONFIG_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"

/*
 * MACROS
 ****************************************************************************************
 */
#define MESH_SEC_BEACON_TICK                       (2000) //unit: ms/ 10s
#define MESH_BEACON_TICK                           (200) //unit: ms/ 10s
/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

uint32_t sec_beacon_get_search_size(void);
void beacon_config_init(void);
void beacon_set_send_flag(uint32_t net_handle);
bool beacon_get_send_flag(uint32_t net_handle);
void beacon_clear_send_flag(uint32_t net_handle);
void beacon_calc_interval(void);
void beacon_clear_tx_info(dm_netkey_pos_t index);
void beacon_rx_pkt_rec(dm_netkey_pos_t index);
void beacon_set_immediately_flag(uint32_t net_handle);
bool beacon_get_immediately_flag(uint32_t net_handle);
void beacon_clear_immediately_flag(uint32_t net_handle);
void beacon_inc_tx_info(dm_netkey_pos_t index);
void mesh_sec_beacon_cache_cpy(uint8_t *data,uint8_t length);
bool mesh_sec_beacon_is_exsit_cache(uint8_t *data,uint8_t length);

#endif /* FREERTOS_APP_MESH_CORE_CORE_CONFIG_BEACON_CONFIG_H_ */ 
/// @} MESH_beacon_config_API

