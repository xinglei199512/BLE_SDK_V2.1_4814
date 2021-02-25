/**
 ****************************************************************************************
 *
 * @file   light_ctl_temperature_server.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-27 18:50
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

#include "light_ctl_temperature_server.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"
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
static void light_ctl_temperature_publish_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"light_ctl_temperature_publish_status_tx_done\n");
}
void light_ctl_temperature_status_publish(light_ctl_temperature_server_t *server)
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
    LOG(3,"light_ctl_status_publish value:%x %x dst_addr:%x\n", 
            server->msg_format->present_ctl_lightness, server->msg_format->present_ctl_temperature, dst_addr);
    light_ctl_temperature_default_status_t msg;
    msg.present_ctl_delta_uv = server->msg_format->present_ctl_delta_uv;
    msg.present_ctl_temperature = server->msg_format->present_ctl_temperature;

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_CTL_TWO_OCTETS_OPCODE_OFFSET, Light_CTL_Temperature_Status);
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = sizeof(light_ctl_temperature_default_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_ctl_temperature_publish_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

static void light_ctl_temperature_server_action(void *param)
{
    LOG(3, "light_ctl_temperature_server_action\n");
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    light_ctl_temperature_server_t *server = (light_ctl_temperature_server_t *)timer_param->inst;
    mesh_model_evt_t evt;
    int ctl_change_count = 0, ctl_count = 0;

    if(server->delay_trans_timer->remain_tick_count) {
    }else {
        ctl_change_count = ctl_count = 1;
    }

    evt.type.ctl_type = LIGHT_CTL_TEMPERATURE_SET;
    evt.params.model_ctl_set.target_ctl_temperature = server->msg_format->target_ctl_temperature;
    evt.params.model_ctl_set.target_ctl_delta_uv = server->msg_format->target_ctl_delta_uv;
    if(server->cb)
        server->cb(&evt);

    server->msg_format->present_ctl_temperature = server->msg_format->target_ctl_temperature;
    server->msg_format->present_ctl_delta_uv = server->msg_format->target_ctl_delta_uv;

    if(ctl_change_count == ctl_count && ctl_change_count == 1)
        server->server_state = GENERIC_TRANS_IDALE;
}
static void light_ctl_temperature_server_action_then_publish(void *param)
{
    LOG(3, "light_ctl_temperature_server_action_then_publish\n");
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    light_ctl_temperature_server_t *server = (light_ctl_temperature_server_t *)timer_param->inst;

    light_ctl_temperature_server_action(param);

    if((server->delay_trans_timer->remain_tick_count == 0)) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
        //light_ctl_temperature_status_publish(server);
    }
}

static void light_ctl_temperature_status_tx_done(void *pdu,uint8_t status)
{
}
//void send_light_ctl_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu, uint32_t lightness, uint16_t opcode_offset)
void send_light_ctl_temperature_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_ctl_temperature_status_t msg1;
    light_ctl_temperature_default_status_t msg2;
    access_pdu_tx_t * ptr;
    light_ctl_temperature_server_t *server = GET_SERVER_MODEL_PTR(light_ctl_temperature_server_t, model);
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_CTL_TWO_OCTETS_OPCODE_OFFSET, Light_CTL_Temperature_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;

    if(server->server_state == GENERIC_TRANS_PROCESS) {
        uint32_t delay_trans_expiry = 0;
        tx_param.pdu_length = sizeof(light_ctl_temperature_status_t);
        msg1.present_ctl_temperature = server->msg_format->present_ctl_temperature;
        msg1.target_ctl_temperature = server->msg_format->target_ctl_temperature;
        msg1.present_ctl_delta_uv = server->msg_format->present_ctl_delta_uv;
        msg1.target_ctl_delta_uv = server->msg_format->target_ctl_delta_uv;
        delay_trans_expiry = generic_get_delay_trans_expiry(
                server->delay_trans_timer,
                server->delay_trans_timer->trans_time,
                server->delay_trans_timer->trans_timer_step);
        msg1.remaining_time = model_transition_time_encode(delay_trans_expiry);
        ptr = access_model_pkt_build_fill(&tx_param,light_ctl_temperature_status_tx_done,(uint8_t *)&msg1);
    }else {
        tx_param.pdu_length = sizeof(light_ctl_temperature_default_status_t);

        msg2.present_ctl_delta_uv = server->msg_format->present_ctl_delta_uv;
        msg2.present_ctl_temperature = server->msg_format->present_ctl_temperature;
        ptr = access_model_pkt_build_fill(&tx_param,light_ctl_temperature_status_tx_done,(uint8_t *)&msg2);
    }
        BX_ASSERT(ptr);
        access_send(ptr);
}

static int light_ctl_temperature_set_handle(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu, uint8_t *publish_status)
{
    uint16_t payload_size;

    generic_valid_field_check_t msg_field;// store src ,dst ,tid
    uint8_t * access = access_get_pdu_payload(pdu);
    payload_size = get_access_pdu_rx_payload_size(pdu);
    msg_field.dst = access_get_pdu_dst_addr(pdu);
    msg_field.src = access_get_pdu_src_addr(pdu);

    light_ctl_temperature_server_t *server = GET_SERVER_MODEL_PTR(light_ctl_temperature_server_t, model);

    server->tid_queue.inst_param.inst = (void *)server;
    server->delay_trans_timer->inst = server->tid_queue.inst_param.inst;
    LOG(3,"light_ctl_temperature_set_handle payload_size:%d dst:%x src:%x\n", payload_size, msg_field.dst, msg_field.src);
    if(payload_size == sizeof(light_ctl_temperature_set_t)){
        light_ctl_temperature_set_t *p_pdu = (light_ctl_temperature_set_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -1;
        }
        if((p_pdu->ctl_temperature < T_MIN) || (p_pdu->ctl_temperature > T_MAX)) {
            return -2;
        }

        if(p_pdu->ctl_temperature < server->msg_format->range_min) {
            p_pdu->ctl_temperature = server->msg_format->range_min;
        }
        if(p_pdu->ctl_temperature > server->msg_format->range_max) {
            p_pdu->ctl_temperature = server->msg_format->range_max;
        }

        server->server_state = GENERIC_TRANS_PROCESS;
        server->msg_format->target_ctl_temperature = p_pdu->ctl_temperature;
        server->msg_format->target_ctl_delta_uv = p_pdu->ctl_delta_uv;
        if((server->msg_format->target_ctl_temperature != server->msg_format->present_ctl_temperature)
                || (server->msg_format->target_ctl_delta_uv != server->msg_format->present_ctl_delta_uv))
            *publish_status = 1;

        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }

        LOG(3,"tid:%x trans_time:%x delay:%x server->cb:%p\n", p_pdu->tid, p_pdu->trans_time, p_pdu->delay, server->cb);

        if(p_pdu->delay == 0 && p_pdu->trans_time == 0){//this block code is used to transition state
            light_ctl_temperature_server_action(server->delay_trans_timer);
        }else {
            server->delay_trans_timer->trans_time = p_pdu->trans_time;
            server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_LIGHT_CTL;

            generic_deal_delay_trans_func(server->delay_trans_timer, p_pdu->delay, p_pdu->trans_time, light_ctl_temperature_server_action_then_publish);
        }

    }else if(payload_size == sizeof(light_ctl_temperature_default_t)) {
        light_ctl_temperature_default_t *p_pdu = (light_ctl_temperature_default_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -3;
        }

        if((p_pdu->ctl_temperature < T_MIN) || (p_pdu->ctl_temperature > T_MAX)) {
            return -4;
        }
        if(p_pdu->ctl_temperature < server->msg_format->range_min) {
            p_pdu->ctl_temperature = server->msg_format->range_min;
        }
        if(p_pdu->ctl_temperature > server->msg_format->range_max) {
            p_pdu->ctl_temperature = server->msg_format->range_max;
        }

        server->server_state = GENERIC_TRANS_PROCESS;
        server->msg_format->target_ctl_temperature = p_pdu->ctl_temperature;
        server->msg_format->target_ctl_delta_uv = p_pdu->ctl_delta_uv;

        if((server->msg_format->target_ctl_temperature != server->msg_format->present_ctl_temperature)
                || (server->msg_format->target_ctl_delta_uv != server->msg_format->present_ctl_delta_uv))
            *publish_status = 1;

        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }

        server->delay_trans_timer->trans_time = user_generic_default_transition_time_get(NULL);
        server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_LIGHT_CTL;
        if(server->delay_trans_timer->trans_time == 0)
            light_ctl_temperature_server_action(server->delay_trans_timer);
        else
            generic_deal_delay_trans_func(server->delay_trans_timer, 0, server->delay_trans_timer->trans_time, light_ctl_temperature_server_action_then_publish);
    }

    return 0;
}

void light_CTL_temperature_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    send_light_ctl_temperature_status(elmt, model, pdu);
}

void light_CTL_temperature_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;

    if(light_ctl_temperature_set_handle(elmt, model, pdu, &publish_status) == 0)
        send_light_ctl_temperature_status(elmt, model, pdu);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
        //light_ctl_temperature_status_publish(server);
    }
}
void light_CTL_temperature_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;

    light_ctl_temperature_set_handle(elmt, model, pdu, &publish_status);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
        //light_ctl_temperature_status_publish(server);
    }
}

