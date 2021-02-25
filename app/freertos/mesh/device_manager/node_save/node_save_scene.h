/**
 ****************************************************************************************
 *
 * @file   node_save_scene.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-29 15:29
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
 * @addtogroup MESH_node_save_scene_API Mesh node_save_scene API
 * @ingroup MESH_API
 * @brief Mesh node_save_scene  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_DEVICE_MANAGER_NODE_SAVE_NODE_SAVE_SCENE_H_
#define APP_FREERTOS_MESH_DEVICE_MANAGER_NODE_SAVE_NODE_SAVE_SCENE_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "mesh_model.h"

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
typedef struct {
    uint16_t scene_number;
    uint16_t lightness;
    uint16_t hue;
    uint16_t saturation;
}save_scene_value_t;


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void node_save_element_scene(model_base_t *model , save_scene_value_t *scene, uint8_t scene_index);
void node_recover_element_scene(void);
void node_delete_element_scene(model_base_t *model , uint8_t scene_index);
void node_search_element_scene(uint8_t index, uint16_t scene_number, save_scene_value_t *p_scene);


#endif /* APP_FREERTOS_MESH_DEVICE_MANAGER_NODE_SAVE_NODE_SAVE_SCENE_H_ */ 
/// @} MESH_node_save_scene_API

