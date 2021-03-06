/**
 ****************************************************************************************
 *
 * @file   light_ctl_setup_server.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-27 18:43
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
 * @addtogroup MESH_light_ctl_setup_server_API Mesh light_ctl_setup_server API
 * @ingroup MESH_API
 * @brief Mesh light_ctl_setup_server  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_CTL_LIGHT_CTL_SETUP_SERVER_H_
#define APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_CTL_LIGHT_CTL_SETUP_SERVER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "light_ctl_common.h"
//#include "access.h"
#include "mesh_model.h"
#include "osapp_config.h"
#include "generic_common.h"
#include "model_servers_events_api.h"
#include "access_rx_process.h"
#include "light_ctl_server.h"

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
    model_server_base_t model;
    light_ctl_msg_format_t *msg_format;
    mesh_model_evt_cb_t cb;
    uint8_t server_state;
}light_ctl_setup_server_t;

#define DEF_LIGHT_CTL_SETUP_SERVER_MODEL(name,app_key_max) \
    static light_ctl_setup_server_t name;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_LIGHT_CTL_SETUP_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,LIGHT_CTL_SETUP_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,NULL,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

void light_CTL_Default_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void light_CTL_default_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void light_CTL_default_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void light_CTL_temperature_range_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void light_CTL_temperature_range_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void light_CTL_temperature_range_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
#endif /* APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_CTL_LIGHT_CTL_SETUP_SERVER_H_ */ 
/// @} MESH_light_ctl_setup_server_API

