/**
 ****************************************************************************************
 *
 * @file   light_ctl_client.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-27 19:03
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
 * @addtogroup MESH_light_ctl_client_API Mesh light_ctl_client API
 * @ingroup MESH_API
 * @brief Mesh light_ctl_client  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_CTL_LIGHT_CTL_CLIENT_H_
#define APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_CTL_LIGHT_CTL_CLIENT_H_

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

typedef struct
{
    /** The model base of the model. */
    model_client_base_t model;
    user_set_current_value_cb set_current_value_cb;
    user_get_current_state_cb get_current_state_cb;
}light_ctl_client_t;


#define DEF_LIGHT_CTL_CLIENT_MODEL(name,app_key_max) \
    static light_ctl_client_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_LIGHT_CTL_CLIENT_MODEL(name,app_key_max,element_num,sub_list,sub_list_num) \
    mesh_model_init(&name.model.base,LIGHT_CTL_CLIENT_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);


void light_CTL_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void light_CTL_temperature_range_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void light_CTL_temperature_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void light_CTL_Default_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void light_ctl_msg_publish(light_ctl_client_t *client,void* msg, uint32_t msgLen, uint16_t dst_addr, uint32_t opcode);


#endif /* APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_CTL_LIGHT_CTL_CLIENT_H_ */ 
/// @} MESH_light_ctl_client_API

