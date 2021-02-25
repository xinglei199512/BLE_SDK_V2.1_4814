/**
 ****************************************************************************************
 *
 * @file   node_save_scheduler.h
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
 * @addtogroup MESH_node_save_scheduler_API Mesh node_save_scheduler API
 * @ingroup MESH_API
 * @brief Mesh node_save_scheduler  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_DEVICE_MANAGER_NODE_SAVE_NODE_SAVE_SCHEDULER_H_
#define APP_FREERTOS_MESH_DEVICE_MANAGER_NODE_SAVE_NODE_SAVE_SCHEDULER_H_

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
    uint64_t index       :4;
    uint64_t year        :7;
    uint64_t mouth       :12;
    uint64_t day         :5;
    uint64_t hour        :5;
    uint64_t minute      :6;
    uint64_t second      :6;
    uint64_t dayofweek   :7;
    uint64_t action      :4;
    uint64_t trans_time  :8;
    uint64_t scene_number:16;
}__attribute((packed))save_scheduler_value_t;


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void node_save_element_scheduler(model_base_t *model , save_scheduler_value_t *scheduler, uint8_t scheduler_index);
void node_recover_element_scheduler(void);
void node_delete_element_scheduler(model_base_t *model , uint8_t scheduler_index);
void node_search_element_scheduler_from_index(uint8_t elem_index, uint8_t scheduler_index, save_scheduler_value_t *p_scheduler);
void node_search_element_scheduler_from_scene_number(uint8_t elem_index, uint16_t scene_number, save_scheduler_value_t *p_scheduler);


#endif /* APP_FREERTOS_MESH_DEVICE_MANAGER_NODE_SAVE_NODE_SAVE_SCHEDULER_H_ */ 
/// @} MESH_node_save_scheduler_API

