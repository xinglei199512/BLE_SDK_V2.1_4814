/**
 ****************************************************************************************
 *
 * @file   provisioner_intf.h
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-29 11:25
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
 * @addtogroup MESH_provisioner_intf_API Mesh provisioner_intf API
 * @ingroup MESH_API
 * @brief Mesh provisioner_intf  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_PROVISION_PROVISIONER_INTF_H_
#define APP_FREERTOS_MESH_PROVISION_PROVISIONER_INTF_H_

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

void user_provisioner_dev_key_gen_done(uint8_t * dev_uuid, uint8_t * devkey, uint16_t addr);
bool get_is_provisioner(void);
void provisioner_role_init(mesh_prov_evt_cb_t cb);
void provisioner_action_send (mesh_prov_action_type_t type , mesh_prov_evt_param_t param);
void provisioner_config (mesh_prov_config_type_t opcode , mesh_prov_evt_param_t param);
int start_provision_dev(uint8_t *dev_uuid);

#endif /* APP_FREERTOS_MESH_PROVISION_PROVISIONER_INTF_H_ */ 
/// @} MESH_provisioner_intf_API

