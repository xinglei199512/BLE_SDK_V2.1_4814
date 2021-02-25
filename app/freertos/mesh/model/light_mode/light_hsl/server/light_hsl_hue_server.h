/**
 ****************************************************************************************
 *
 * @file   light_hsl_hue_server.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-27 18:46
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
 * @addtogroup MESH_light_hsl_hue_server_API Mesh light_hsl_hue_server API
 * @ingroup MESH_API
 * @brief Mesh light_hsl_hue_server  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_HSL_LIGHT_HSL_HUE_SERVER_H_
#define APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_HSL_LIGHT_HSL_HUE_SERVER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "light_hsl_common.h"
//#include "access.h"
#include "mesh_model.h"
#include "osapp_config.h"
#include "generic_common.h"
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
    uint16_t present_hsl_hue;
    uint16_t target_hsl_hue;
}light_hsl_hue_msg_format_t;

typedef struct
{
    model_server_base_t model;
    light_hsl_msg_format_t *msg_format;
    generic_valid_field_queue_t tid_queue;
    generic_delay_trans_param_t *delay_trans_timer;
    mesh_model_evt_cb_t cb;
    uint8_t server_state;
}light_hsl_hue_server_t;

#define DEF_LIGHT_HSL_HUE_SERVER_MODEL(name,app_key_max) \
    static light_hsl_hue_server_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_LIGHT_HSL_HUE_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,LIGHT_HSL_HUE_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,(model_publish_timeout_cb_t)light_hsl_hue_status_publish);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

void light_HSL_hue_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void light_HSL_hue_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void light_HSL_hue_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void light_hsl_hue_status_publish(light_hsl_hue_server_t *server);
#endif /* APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_HSL_LIGHT_HSL_HUE_SERVER_H_ */ 
/// @} MESH_light_hsl_hue_server_API

