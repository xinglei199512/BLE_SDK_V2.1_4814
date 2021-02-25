/**
 ****************************************************************************************
 *
 * @file   unprov_device_intf.h
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-22 09:43
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) BlueX Microelectronics 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_unprov_device_intf_API Mesh unprov_device_intf API
 * @ingroup MESH_API
 * @brief Mesh unprov_device_intf  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_PROVISION_UNPROV_DEVICE_INTF_H_
#define APP_FREERTOS_MESH_PROVISION_UNPROV_DEVICE_INTF_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "provision_api.h"
/*
 * MACROS
 ****************************************************************************************
 */

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
void user_unprov_dev_make_attention(uint8_t duration);

void user_unprov_dev_expose_public_key_oob(public_key_t * public_keys);

void unprov_input_auth_value(uint8_t *buff,void (*cb)());

void unprov_output_auth_value(uint8_t *buff);

void unprov_device_init(mesh_prov_evt_cb_t cb);
void unprov_device_config (mesh_prov_config_type_t opcode , mesh_prov_evt_param_t param);
void unprov_device_action_send (mesh_prov_action_type_t type , mesh_prov_evt_param_t param);
mesh_provsion_method_t unprov_priovision_method_get(void);
void unprovisioned_dev_reset(void);
void unprovisioned_dev_reset_random(void);

uint8_t * get_unprov_dev_uuid(void);

void unprov_refresh_public_keys(void);

#endif /* APP_FREERTOS_MESH_PROVISION_UNPROV_DEVICE_INTF_H_ */ 
/// @} MESH_unprov_device_intf_API

