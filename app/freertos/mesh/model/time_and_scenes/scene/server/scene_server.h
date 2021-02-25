/**
 ****************************************************************************************
 *
 * @file   scene_server.h
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
 * @addtogroup MESH_scene_server_API Mesh scene_server API
 * @ingroup MESH_API
 * @brief Mesh scene_server  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCENE_SCENE_SERVER_H_
#define APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCENE_SCENE_SERVER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "scene_common.h"
#include "mesh_model.h"
#include "osapp_config.h"
#include "generic_common.h"
#include "model_common.h"
#include "model_servers_events_api.h"
#include "time_common.h"
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
#define SCENE_NUMBER_MAX 15

typedef struct
{
    uint8_t status_code;
    uint16_t current_scene;
    uint16_t target_scene;
    uint16_t scene_number[SCENE_NUMBER_MAX];
}scene_server_msg_format_t;

typedef struct
{
    model_server_base_t model;
    scene_server_msg_format_t *msg_format;
    model_state_bound_field_t *state_bound;
    generic_valid_field_queue_t tid_queue;
    generic_delay_trans_param_t *delay_trans_timer;
    mesh_model_evt_cb_t cb;
    uint8_t server_state;
}scene_server_t;

#define DEF_SCENE_SERVER_MODEL(name,app_key_max) \
    static scene_server_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_SCENE_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,SCENE_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,(model_publish_timeout_cb_t)time_scene_status_publish);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

#define DEF_SCENE_SETUP_SERVER_MODEL(name,app_key_max) \
    static scene_server_t name;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_SCENE_SETUP_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,SCENE_SETUP_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,NULL,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

void scene_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void scene_recall_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void scene_recall_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void scene_register_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void scene_store_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void scene_store_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void scene_delete_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void scene_delete_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

int8_t scene_get_scene_number_count(scene_server_t *server);
int8_t scene_delete_scene_number(scene_server_t *server, uint16_t scene_number);
int8_t scene_store_scene_number(scene_server_t *server, uint16_t scene_number);
int8_t scene_search_scene_number(scene_server_t *server, uint16_t scene_number);
void scene_call_user_callback(scene_server_t *server, mesh_scene_model_evt_type_t type, uint16_t value, uint8_t repeat_flag);
void time_scene_status_publish(scene_server_t *server);

#endif /* APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCENE_SCENE_SERVER_H_ */ 
/// @} MESH_scene_server_API

