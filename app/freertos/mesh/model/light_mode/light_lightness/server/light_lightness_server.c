/**
 ****************************************************************************************
 *
 * @file   light_lightness_server.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-21 17:16
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "light_lightness_server.h"
#include "light_lightness_common.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"
#include <math.h>
#include "node_setup.h"


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
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief   Func miaoshu
 *
 * @param[in] xxx1     Id of the message received.
 * @param[in] xxx2     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static void light_lightness_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"light_lightness_status_tx_done\n");
}
void light_lightness_status_publish(light_lightness_server_t *server)
{
    uint16_t dst_addr = 0;
    uint16_t appkey_global_idx = server->model.base.publish->appkey_idx;

    if(server->model.base.publish == NULL)
    {
        return;
    }

    if(server->model.base.publish->addr.is_virt)
    {
        dst_addr = server->model.base.publish->addr.addr.virt->virt_addr;
    }
    else
    {
	    dst_addr = server->model.base.publish->addr.addr.addr;
    }
    if(IS_UNASSIGNED_ADDR(dst_addr))
    {
        return;
    }
    LOG(3,"light_lightness_status_publish lightness:%x dst_addr:%x\n", server->msg_format->present_lightness_actual, dst_addr);
    light_lightness_publish_status_t msg;
    msg.lightness = server->msg_format->present_lightness_actual;
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_LIGHTNESS_TWO_OCTETS_OPCODE_OFFSET, Light_Lightness_Status);
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = sizeof(light_lightness_publish_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_lightness_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

void light_lightness_server_action(light_lightness_server_t *server, uint16_t target_value, mesh_light_lightness_model_evt_type_t type)
{
    mesh_model_evt_t evt;

    memset(&evt, 0, sizeof(mesh_model_evt_t));

    evt.type.lightness_type = type;
    evt.params.model_value_set.target_value = target_value;

    if(server->cb)
        server->cb(&evt);
}

static void set_lightness_actual_server_status_action(light_lightness_server_t *server)
{
    int lightness_change_count = 0;
    int lightness_count = 0;
    uint8_t first_enter_flag = 0;
    uint16_t present_lightness, target_lightness;
    uint16_t target_value, target_value_tmp;
    uint16_t trans_timer_tmp;

    if(server->delay_trans_timer->remain_tick_count) {
        first_enter_flag = (pdMS_TO_TICKS(model_transition_time_decode(server->delay_trans_timer->trans_time, &trans_timer_tmp)) 
                                    == (server->delay_trans_timer->remain_tick_count + pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step)));

        lightness_count = pdMS_TO_TICKS(model_transition_time_decode(server->delay_trans_timer->trans_time, &trans_timer_tmp)) / pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step);

        lightness_change_count = lightness_count - server->delay_trans_timer->remain_tick_count / pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step);
        if(first_enter_flag == 1) {
            server->msg_format->origin_present_lightness = server->msg_format->present_lightness_actual;
        }
    }else {
        lightness_change_count = lightness_count = 1;
    }

    present_lightness = server->msg_format->origin_present_lightness;
    target_lightness = server->msg_format->target_lightness_actual;

    if(present_lightness > target_lightness) {
        target_value_tmp = (uint16_t)((present_lightness - target_lightness) * ((lightness_change_count * 1.0) / lightness_count));
        target_value = (uint16_t)(present_lightness - target_value_tmp);
    }else {
        if(lightness_change_count == lightness_count) {
            target_value = target_lightness;
        }else {
            target_value_tmp = (uint16_t)((target_lightness - present_lightness) * ((lightness_change_count * 1.0) / lightness_count));
            target_value = (uint16_t)(present_lightness + target_value_tmp);
        }
    }

    light_lightness_server_action(server, target_value, LIGHTNESS_ACTUAL_EVT_SET);

    LOG(3,"light_actucl_action target_state:%d %d target_value:%d count:%d %d\n", 
            server->msg_format->target_lightness_actual, server->msg_format->present_lightness_actual, target_value, lightness_change_count, lightness_count);
    if(server->msg_format->present_lightness_actual == server->msg_format->target_lightness_actual)
        server->server_state = GENERIC_TRANS_IDALE;
}

static void set_lightness_linear_server_status_action(light_lightness_server_t *server)
{
    int lightness_change_count = 0;
    int lightness_count = 0;
    uint8_t first_enter_flag = 0;
    uint16_t present_lightness, target_lightness;
    uint16_t target_value, target_value_tmp;
    uint16_t trans_timer_tmp;

    if(server->delay_trans_timer->remain_tick_count) {
        first_enter_flag = (pdMS_TO_TICKS(model_transition_time_decode(server->delay_trans_timer->trans_time, &trans_timer_tmp)) 
                                    == (server->delay_trans_timer->remain_tick_count + pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step)));

        lightness_count = pdMS_TO_TICKS(model_transition_time_decode(server->delay_trans_timer->trans_time, &trans_timer_tmp)) / pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step);

        lightness_change_count = lightness_count - server->delay_trans_timer->remain_tick_count / pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step);

        if(first_enter_flag == 1) {
            server->msg_format->origin_present_lightness = server->msg_format->present_lightness_linear;
        }
    }else {
        lightness_change_count = lightness_count = 1;
    }

    present_lightness = (uint16_t)sqrt(server->msg_format->origin_present_lightness * 65535);
    target_lightness = (uint16_t)sqrt(server->msg_format->target_lightness_linear * 65535);

    if(present_lightness > target_lightness) {
        target_value_tmp = (uint16_t)((present_lightness - target_lightness) * ((lightness_change_count * 1.0) / lightness_count));
        target_value = (uint16_t)(((present_lightness - target_value_tmp) * (present_lightness - target_value_tmp)) / 65535);
    }else {
        if(lightness_change_count == lightness_count) {
            target_value = server->msg_format->target_lightness_linear;
        }else {
            target_value_tmp = (uint16_t)((target_lightness - present_lightness) * ((lightness_change_count * 1.0) / lightness_count));
            target_value = (uint16_t)(((present_lightness + target_value_tmp) * (present_lightness + target_value_tmp)) / 65535);
        }
    }
    light_lightness_server_action(server, target_value, LIGHTNESS_LINEAR_EVT_SET);

    LOG(3,"light_linear_action target_state:%d %d target_value:%d count:%d %d\n", 
            server->msg_format->target_lightness_linear, server->msg_format->present_lightness_linear, target_value, lightness_change_count, lightness_count);
    if(server->msg_format->present_lightness_linear == server->msg_format->target_lightness_linear)
        server->server_state = GENERIC_TRANS_IDALE;
}

void set_server_status_action(light_lightness_server_t *server, generic_delay_trans_param_t *timer_param)
{
    switch(server->delay_trans_timer->type) {
        case GENERIC_DELAY_TRANS_TIMER_LIGHT_LIGHTNESS_ACTUAL:
            set_lightness_actual_server_status_action(server);
            break;
        case GENERIC_DELAY_TRANS_TIMER_LIGHT_LIGHTNESS_LINEAR:
            set_lightness_linear_server_status_action(server);
            break;
        default:
            break;
    }
}

static void handler_light_lightness_transition_timer(void *thandle)
{
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)(thandle);;

    light_lightness_server_t *server = (light_lightness_server_t *)(timer_param->inst);
    set_server_status_action(server, timer_param);

    if((server->delay_trans_timer->remain_tick_count == 0 && timer_param->dt_timer_flag == USE_FOR_TRANS_TIME)) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}

static void light_lightness_actual_status_tx_done(void *pdu,uint8_t status)
{
      LOG(3,"light_lightness_actual_status_tx_done\n");
}

static void light_lightness_linear_status_tx_done(void *pdu,uint8_t status)
{
      LOG(3,"light_lightness_linear_status_tx_done\n");
}

void light_lightness_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    if(mesh_node_get_primary_element_addr() == access_get_pdu_src_addr(pdu))
        return;
    LOG(3, "light_lightness_get_rx\n");

    SEND_LIGHT_STATUS(elmt, model, pdu, lightness_actual, Light_Lightness_Status);
}

void light_lightness_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;

    if(mesh_node_get_primary_element_addr() == access_get_pdu_src_addr(pdu))
        return;

    LIGHT_SET_RX_HANDLE(elmt, model, pdu, publish_status, lightness_actual, GENERIC_DELAY_TRANS_TIMER_LIGHT_LIGHTNESS_ACTUAL, LIGHTNESS_ACTUAL_EVT_SET);

    SEND_LIGHT_STATUS(elmt, model, pdu, lightness_actual, Light_Lightness_Status);

    LOG(3, "%s:%d publish_status:%d\n", __func__, __LINE__, publish_status);

    if(publish_status) {
        model_status_publish();
    }
}

void light_lightness_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;

    if(mesh_node_get_primary_element_addr() == access_get_pdu_src_addr(pdu))
        return;

    LIGHT_SET_RX_HANDLE(elmt, model, pdu, publish_status, lightness_actual, GENERIC_DELAY_TRANS_TIMER_LIGHT_LIGHTNESS_ACTUAL, LIGHTNESS_ACTUAL_EVT_SET);

    LOG(3, "%s:%d publish_status:%d\n", __func__, __LINE__, publish_status);
    if(publish_status) {
        model_status_publish();
    }
}

void light_lightness_Linear_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    if(mesh_node_get_primary_element_addr() == access_get_pdu_src_addr(pdu))
        return;
    LOG(3, "light_lightness_Linear_get_rx\n");
    SEND_LIGHT_STATUS(elmt, model, pdu, lightness_linear, Light_Lightness_Linear_Status);
}

void light_lightness_Linear_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;
    LIGHT_SET_RX_HANDLE(elmt, model, pdu, publish_status, lightness_linear, GENERIC_DELAY_TRANS_TIMER_LIGHT_LIGHTNESS_LINEAR, LIGHTNESS_LINEAR_EVT_SET);

    SEND_LIGHT_STATUS(elmt, model, pdu, lightness_linear, Light_Lightness_Linear_Status);

    LOG(3, "%s:%d publish_status:%d\n", __func__, __LINE__, publish_status);
    if(publish_status) {
        model_status_publish();
    }
}

void light_lightness_Linear_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;
    LIGHT_SET_RX_HANDLE(elmt, model, pdu, publish_status, lightness_linear, GENERIC_DELAY_TRANS_TIMER_LIGHT_LIGHTNESS_LINEAR, LIGHTNESS_LINEAR_EVT_SET);
    LOG(3, "%s:%d publish_status:%d\n", __func__, __LINE__, publish_status);
    if(publish_status) {
        model_status_publish();
    }
}

static void light_ligntness_last_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "light_ligntness_last__tx_done\n");
}

void send_light_lightness_last(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu, uint32_t lightness_actual, uint16_t opcode_offset)
{
    light_lightness_actual_default_status_t msg;
    light_lightness_server_t *server = GET_SERVER_MODEL_PTR(light_lightness_server_t, model);
    msg.lightness_actual = lightness_actual;
    if(msg.lightness_actual == 0)
        LOG(3, "send_light_lightness_last ERR\n");
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_LIGHTNESS_TWO_OCTETS_OPCODE_OFFSET, opcode_offset);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(light_lightness_actual_default_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_ligntness_last_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}

void light_lightness_Last_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "light_lightness_Last_get_rx\n");
    light_lightness_server_t *server = GET_SERVER_MODEL_PTR(light_lightness_server_t, model);
    send_light_lightness_last(elmt, model, pdu, server->msg_format->lightness_last, Light_Lightness_Last_Status);
}


