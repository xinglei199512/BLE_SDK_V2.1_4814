/**
 ****************************************************************************************
 *
 * @file   scene_client.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-21 10:24
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
 * @addtogroup MESH_scene_client_API Mesh scene_client API
 * @ingroup MESH_API
 * @brief Mesh scene_client  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCENE_SCENE_CLIENT_H_
#define APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCENE_SCENE_CLIENT_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "time_common.h"
#include "mesh_model.h"
#include "osapp_config.h"
#include "generic_common.h"
#include "model_common.h"
#include "model_servers_events_api.h"
#include "access_rx_process.h"

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
typedef struct
{
    /** The model base of the model. */
    model_client_base_t model;
}scene_client_t;

#define DEF_SCENE_CLIENT_MODEL(name,app_key_max) \
    static scene_client_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_SCENE_CLIENT_MODEL(name,app_key_max,element_num,sub_list,sub_list_num) \
    mesh_model_init(&name.model.base,SCENE_CLIENT_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);


void scene_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void scene_register_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);


#endif /* APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCENE_SCENE_CLIENT_H_ */ 
/// @} MESH_scene_client_API

