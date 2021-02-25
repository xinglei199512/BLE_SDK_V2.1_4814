/**
 ****************************************************************************************
 *
 * @file   time_client.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-17 11:31
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
 * @addtogroup MESH_time_client_API Mesh time_client API
 * @ingroup MESH_API
 * @brief Mesh time_client  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_TIME_CLIENT_H_
#define APP_FREERTOS_MESH_MODEL_TIME_CLIENT_H_

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

typedef struct
{
    /** The model base of the model. */
    model_client_base_t model;
}time_client_t;

#define DEF_TIME_CLIENT_MODEL(name,app_key_max) \
    static time_client_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_TIME_CLIENT_MODEL(name,app_key_max,element_num,sub_list,sub_list_num) \
    mesh_model_init(&name.model.base,TIME_CLIENT_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);

void time_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void time_role_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void time_zone_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tai_utc_delta_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void time_msg_publish(time_client_t *client,void* msg, uint32_t msgLen, uint16_t dst_addr, uint32_t opcode);


#endif /* APP_FREERTOS_MESH_MODEL_TIME_CLIENT_H_ */ 
/// @} MESH_timeclient_API

