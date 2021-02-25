/**
 ****************************************************************************************
 *
 * @file   scheduler_server.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-21 10:39
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
 * @addtogroup MESH_scheduler_server_API Mesh scheduler_server API
 * @ingroup MESH_API
 * @brief Mesh scheduler_server  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCHEDULER_SCHEDULER_SERVER_H_
#define APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCHEDULER_SCHEDULER_SERVER_H_

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
#include "scheduler_common.h"
#include "osapp_calendar.h"
#include "access_rx_process.h"
#include "timer_wrapper.h"

#define ONE_YEAR_TO_S (365 * 24 * 60 * 60)
#define ONE_DAY_TO_S (24 * 60 * 60)
#define ONE_HOUR_TO_S (60 * 60)
#define ONE_MINUTE_TO_S (60)
#define ONE_SECOND_TO_S (1)
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#define SCHSDULER_INDEX_MAX  16

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
typedef void(*user_scheduler_server_action_cb)(void *param, uint16_t action);

enum {
    TAI_WEEK_FLOW   = BIT(0),
    TAI_SECOND_FLOW = BIT(1),
    TAI_MINUTE_FLOW = BIT(2),
    TAI_HOUR_FLOW   = BIT(3),
    TAI_DAY_FLOW    = BIT(4),
    TAI_MONTH_FLOW  = BIT(5),
    TAI_YEAR_FLOW   = BIT(6),
};

enum {
    REPEAT_BY_YEAR          = BIT(0x0),
    REPEAT_BY_MONTH         = BIT(0x1),
    REPEAT_BY_WEEK          = BIT(0x2),
    REPEAT_BY_DAY           = BIT(0x3),
    REPEAT_BY_ANY_HOUR      = BIT(0x4),
    REPEAT_BY_ONCE_HOUR     = BIT(0x5),
    REPEAT_BY_HOUR          = BIT(0x4) | BIT(0x5),
    REPEAT_BY_ANY_MIN       = BIT(0x6),
    REPEAT_BY_EVERY15_MIN   = BIT(0x7),
    REPEAT_BY_EVERY20_MIN   = BIT(0x8),
    REPEAT_BY_ONCE_MIN      = BIT(0x9),
    REPEAT_BY_MIN           = BIT(0x6) | BIT(0x7) | BIT(0x8),
    REPEAT_BY_ANY_SEC       = BIT(0xa),
    REPEAT_BY_EVERY15_SEC   = BIT(0xb),
    REPEAT_BY_EVERY20_SEC   = BIT(0xc),
    REPEAT_BY_ONCE_SEC      = BIT(0xd),
    REPEAT_BY_SEC           = BIT(0xa) | BIT(0xb) | BIT(0xc),
};
typedef struct
{
    uint64_t year        :7;
    uint64_t month       :12;
    uint64_t day         :5;
    uint64_t hour        :5;
    uint64_t minute      :6;
    uint64_t second      :6;
    uint64_t dayofweek   :7;
    uint64_t action      :4;
    uint64_t trans_time  :8;
    uint64_t scene_number:16;
}__attribute((packed))scheduler_time_value_t;
typedef struct
{
    void *inst;
    user_scheduler_server_action_cb action_cb;
    uint8_t alarm_count:4;
    scheduler_time_value_t time_value;
    uint16_t repeat_flag;
    int8_t alarm_num:7;
    uint8_t last_timeout_flag:1;
    uint8_t day_tai_flow;
    uint64_t alarm_remain_time;
    uint64_t alarm_time[SCHSDULER_INDEX_MAX];
    int8_t  alarm_timer_index[SCHSDULER_INDEX_MAX];
    int8_t   current_alarm_index[SCHSDULER_INDEX_MAX];
    mesh_timer_t scheduler_Timer;
}__attribute((packed))scheduler_time_param_t;
typedef struct
{
    scheduler_action_set_t action_set;
    uint16_t schedules;
}scheduler_server_msg_format_t;

typedef struct
{
    model_server_base_t model;
    scheduler_server_msg_format_t *msg_format;
    model_state_bound_field_t *state_bound;
    scheduler_time_param_t *scheduler_time;
    mesh_model_evt_cb_t cb;
    uint8_t server_state;
}scheduler_server_t;

#define DEF_SCHEDULER_SERVER_MODEL(name,app_key_max) \
    static scheduler_server_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_SCHEDULER_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,SCHEDULER_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

#define DEF_SCHEDULER_SETUP_SERVER_MODEL(name,app_key_max) \
    static scheduler_server_t name;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_SCHEDULER_SETUP_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,SCHEDULER_SETUP_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,NULL,sub_list,sub_list_num,NULL);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;

void scheduler_action_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void scheduler_action_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void scheduler_action_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);
void scheduler_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

#endif /* APP_FREERTOS_MESH_MODEL_TIME_AND_SCENES_SCHEDULER_SCHEDULER_SERVER_H_ */ 
/// @} MESH_scheduler_server_API

