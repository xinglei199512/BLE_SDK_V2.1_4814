
#ifndef APP_FREERTOS_MESH_DEVICE_MANAGER_NODE_SAVE_NODE_SAVE_ONOFFCOUNT_H_
#define APP_FREERTOS_MESH_DEVICE_MANAGER_NODE_SAVE_NODE_SAVE_ONOFFCOUNT_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "mesh_model.h"


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void node_save_system_onoffcount(uint8_t count);
uint8_t node_recover_system_onoffcount(void);

#endif /* APP_FREERTOS_MESH_DEVICE_MANAGER_NODE_SAVE_NODE_SAVE_ONOFFCOUNT_H_ */ 
/// @} MESH_node_save_onoffcount_API
