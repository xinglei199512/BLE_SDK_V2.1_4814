/**
 ****************************************************************************************
 *
 * @file   light_lightness_server.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-21 17:15
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
 * @addtogroup MESH_light_lightness_server_API Mesh light_lightness_server API
 * @ingroup MESH_API
 * @brief Mesh light_lightness_server  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_LIGHTNESS_LIGHT_LIGHTNESS_SERVER_H_
#define APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_LIGHTNESS_LIGHT_LIGHTNESS_SERVER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"
#include "light_lightness_common.h"
//#include "access.h"
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
#define ONE_STEP_OF_LIGHTNESS 0x200
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
    uint16_t present_lightness_linear;
    uint16_t target_lightness_linear;
    
    uint16_t present_lightness_actual;
    uint16_t target_lightness_actual;
    
    uint16_t lightness_last;
    uint16_t origin_present_lightness;

    uint16_t lightness_default;
    uint8_t status_code;
    uint16_t lightness_range_min;
    uint16_t lightness_range_max;
}light_lightness_msg_format_t;


typedef struct
{
    model_server_base_t model;
    light_lightness_msg_format_t *msg_format;
    model_state_bound_field_t *state_bound;
    generic_valid_field_queue_t tid_queue;
    generic_delay_trans_param_t *delay_trans_timer;
    mesh_model_evt_cb_t cb;
    uint8_t server_state;
}light_lightness_server_t;

#define DEF_LIGHT_LIGHTNESS_SERVER_MODEL(name,app_key_max) \
    static light_lightness_server_t name;\
    static model_publish_state_t name##_publish_state;\
    static app_key_t *name##bound_key_buf[app_key_max];

#define INIT_LIGHT_LIGHTNESS_SERVER_MODEL(name,app_key_max,element_num,sub_list,sub_list_num,user_cb) \
    mesh_model_init(&name.model.base,LIGHT_LIGHTNESS_SERVER_MODEL_ID,true,app_key_max,name##bound_key_buf);\
    model_publish_subscribe_bind(&name.model.base,&name##_publish_state,sub_list,sub_list_num,(model_publish_timeout_cb_t)light_lightness_status_publish);\
    mesh_element_init(element_num, &name.model.base);\
    name.cb = user_cb;


#define SEND_LIGHT_STATUS(elmt, model, pdu, name, opcode_num)              \
{                                                              \
    model_tx_msg_param_t tx_param; \
    access_pdu_tx_t * ptr; \
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));\
    light_lightness_server_t *server = GET_SERVER_MODEL_PTR(light_lightness_server_t, model); \
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key); \
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_LIGHTNESS_TWO_OCTETS_OPCODE_OFFSET, opcode_num);  \
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu); \
    tx_param.src_addr = server->model.base.elmt->uni_addr; \
    tx_param.akf = 1; \
    tx_param.seg = 1; \
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu); \
    tx_param.rx_time = &rx_time; \
    light_##name##_status_t msg1;                          \
    light_##name##_default_status_t msg2;                          \
    LOG(3, "%s value:%x %x\n", __func__, server->msg_format->present_##name, server->msg_format->target_##name);\
    if(server->server_state == GENERIC_TRANS_PROCESS) {                                  \
        uint32_t delay_trans_expiry = 0;                                \
        tx_param.pdu_length = sizeof(light_##name##_status_t); \
        msg1.present_##name = server->msg_format->present_##name;           \
        msg1.target_##name = server->msg_format->target_##name;            \
        delay_trans_expiry = generic_get_delay_trans_expiry(           \
                server->delay_trans_timer,                      \
                server->delay_trans_timer->trans_time,     \
                server->delay_trans_timer->trans_timer_step);          \
        msg1.remaining_time = model_transition_time_encode(delay_trans_expiry); \
        LOG(3, "%s:%x remain_time:%x\n", __func__, msg1.present_##name, msg1.remaining_time);           \
        ptr = access_model_pkt_build_fill(&tx_param,light_##name##_status_tx_done,(uint8_t *)&msg1); \
    }else {                                                                      \
        tx_param.pdu_length = sizeof(light_lightness_actual_default_status_t); \
        msg2.name = server->msg_format->present_##name;                                 \
        LOG(3, "%s:%x\n", __func__, msg2.name);                                   \
        ptr = access_model_pkt_build_fill(&tx_param,light_##name##_status_tx_done,(uint8_t *)&msg2); \
    }                                                                               \
    BX_ASSERT(ptr); \
    access_send(ptr); \
}

#define LIGHT_SET_RX_HANDLE(elmt, model, pdu, publish_status, name, TYPE, evt_TYPE)  \
{                                               \
    uint16_t payload_size;                      \
    generic_valid_field_check_t msg_field;      \
    light_lightness_server_t *server = GET_SERVER_MODEL_PTR(light_lightness_server_t, model); \
    payload_size = get_access_pdu_rx_payload_size(pdu);                                  \
    uint16_t lightness = *(uint16_t *)((uint8_t *)access_get_pdu_payload(pdu) + 2); \
    msg_field.dst = access_get_pdu_dst_addr(pdu);                                                  \
    msg_field.src = access_get_pdu_src_addr(pdu);                                                  \
                                                                                         \
    server->tid_queue.inst_param.inst = (void *)server;                                  \
    if(payload_size == sizeof(light_##name##_set_t)) {                               \
        light_##name##_set_t *p_pdu = (light_##name##_set_t *)(access_get_pdu_payload(pdu) + 2); \
        msg_field.tid = p_pdu->tid;                                       \
                                                                          \
        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {\
            return;                                                       \
        }                                                                 \
        if(p_pdu->name < server->msg_format->lightness_range_min)         \
            p_pdu->name = server->msg_format->lightness_range_min;        \
        if(p_pdu->name > server->msg_format->lightness_range_max)         \
            p_pdu->name = server->msg_format->lightness_range_max;        \
                                                                          \
        server->server_state = GENERIC_TRANS_PROCESS;                     \
        server->msg_format->target_##name = p_pdu->name;                      \
        if(server->msg_format->target_##name != server->msg_format->present_##name)                      \
            publish_status = 1;                                           \
                                                                          \
        if(server->delay_trans_timer->Timer != NULL)                       \
        {                                                                 \
            mesh_timer_stop(server->delay_trans_timer->Timer);               \
            mesh_timer_delete(server->delay_trans_timer->Timer);             \
            server->delay_trans_timer->Timer = NULL;                       \
            server->delay_trans_timer->remain_tick_count = 0;             \
        }                                                                 \
        server->delay_trans_timer->inst = server->tid_queue.inst_param.inst;    \
        server->delay_trans_timer->trans_time = p_pdu->trans_time;               \
        server->delay_trans_timer->type = TYPE;               \
				server->msg_format->target_lightness_actual=lightness;\
        LOG(3,"lightness %x %x tid:%x trans_time:%x delay:%x\n", lightness, p_pdu->name, p_pdu->tid, p_pdu->trans_time, p_pdu->delay);\
        if(p_pdu->delay == 0 && p_pdu->trans_time == 0){                                         \
            set_server_status_action(server, server->delay_trans_timer);                              \
        }else {                                                                                  \
            generic_deal_delay_trans_func(server->delay_trans_timer, p_pdu->delay, p_pdu->trans_time, handler_light_lightness_transition_timer);\
        }                                                                                        \
    }else if(payload_size == sizeof(light_##name##_default_t)) {                             \
        light_##name##_default_t *p_pdu = (light_##name##_default_t *)(access_get_pdu_payload(pdu)  + 2); \
        msg_field.tid = p_pdu->tid;                                          \
        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {   \
            return;                                                          \
        }                                                                    \
        if(p_pdu->name < server->msg_format->lightness_range_min)         \
            p_pdu->name = server->msg_format->lightness_range_min;        \
        if(p_pdu->name > server->msg_format->lightness_range_max)         \
            p_pdu->name = server->msg_format->lightness_range_max;        \
                                                                          \
        server->msg_format->target_##name = p_pdu->name;                        \
        server->server_state = GENERIC_TRANS_PROCESS;                     \
        if(server->msg_format->target_##name != server->msg_format->present_##name)                      \
            publish_status = 1;                                           \
        if(server->delay_trans_timer->Timer != NULL)                          \
        {                                                                    \
            mesh_timer_stop(server->delay_trans_timer->Timer);                 \
            mesh_timer_delete(server->delay_trans_timer->Timer);                \
            server->delay_trans_timer->Timer = NULL;                           \
            server->delay_trans_timer->remain_tick_count = 0;             \
        }                                                                     \
        server->delay_trans_timer->inst = server->tid_queue.inst_param.inst;    \
        server->delay_trans_timer->trans_time = user_generic_default_transition_time_get(NULL);               \
        server->delay_trans_timer->type = TYPE;               \
        if(server->delay_trans_timer->trans_time == 0)                \
            set_server_status_action(server, server->delay_trans_timer);                              \
        else                                                          \
            generic_deal_delay_trans_func(server->delay_trans_timer, 0, server->delay_trans_timer->trans_time, handler_light_lightness_transition_timer);\
    }                                                                         \
}


void light_lightness_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void light_lightness_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void light_lightness_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void light_lightness_Linear_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void light_lightness_Linear_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void light_lightness_Linear_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void light_lightness_Last_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu);

void light_lightness_status_publish(light_lightness_server_t *server);

#endif /* APP_FREERTOS_MESH_MODEL_LIGHT_MODE_LIGHT_LIGHTNESS_LIGHT_LIGHTNESS_SERVER_H_ */ 
/// @} MESH_light_lightness_server_API

