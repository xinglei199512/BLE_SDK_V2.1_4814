/**
 ****************************************************************************************
 *
 * @file   time_server.h
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
 * @addtogroup MESH_time_server_API Mesh time_server API
 * @ingroup MESH_API
 * @brief Mesh time_server  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_TIME_SERVER_H_
#define APP_FREERTOS_MESH_MODEL_TIME_SERVER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "mesh_model.h"
#include "osapp_config.h"
#include "generic_common.h"
#include "model_common.h"
#include "model_servers_events_api.h"
#include "time_common.h"
#include "osapp_calendar.h"
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
enum {
    MESH_TIME_NONE = 0,
    MESH_TIME_AUTHORITY,
    MESH_TIME_RELAY,
    MESH_TIME_CLIENT,
    MESH_TIME_MAX,
};

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
    uint16_t tai_utc_delta_current;
    uint16_t tai_zone_offset_current;
    time_set_t set;
    time_zone_set_t zone_set;
    time_role_set_t role_set;
    tai_utc_delta_set_t utc_set;
    tm_date day;
}time_server_msg_format_t;

typedef struct
{
    model_server_base_t model;
    time_server_msg_format_t *msg_format;
    model_state_bound_field_t *state_bound;
    mesh_model_evt_cb_t cb;
    uint8_t server_state;
}time_server_t;

#define DEF_TIME_SERVER_MODEL(name,app_key_max) \
    static time_server_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_TIME_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,TIME_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

#define DEF_TIME_SETUP_SERVER_MODEL(name,app_key_max) \
    static time_server_t name;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_TIME_SETUP_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,TIME_SETUP_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,NULL,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

void time_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void time_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void time_role_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void time_role_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void time_zone_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void time_zone_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tai_utc_delta_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void tai_utc_delta_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void set_time_msg_format_to_time(time_server_t *server);
int getWeekdayByYearday(int iY, int iM, int iD);
void get_now_time(tm_date *now_time);

#endif /* APP_FREERTOS_MESH_MODEL_TIME_SERVER_H_ */ 
/// @} MESH_time_server_API

