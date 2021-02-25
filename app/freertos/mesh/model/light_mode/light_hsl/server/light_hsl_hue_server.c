/**
 ****************************************************************************************
 *
 * @file   light_hsl_hue_server.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-27 18:51
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

#include "light_hsl_hue_server.h"
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
static void light_hsl_publish_hue_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"light_hsl_publish_hue_status_tx_done\n");
}
void light_hsl_hue_status_publish(light_hsl_hue_server_t *server)
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
    LOG(3,"light_hsl_hue_status_publish:%x dst_addr:%x\n", server->msg_format->present_hsl_hue, dst_addr);
    light_hsl_hue_default_status_t msg;
    msg.present_hsl_hue = server->msg_format->present_hsl_hue;
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_HSL_TWO_OCTETS_OPCODE_OFFSET, Light_HSL_Hue_Status);
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = sizeof(light_hsl_hue_default_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_hsl_publish_hue_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

static void light_hsl_hue_server_action(void *param)
{
    LOG(3, "light_hsl_hue_server_action\n");
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    light_hsl_hue_server_t *server = (light_hsl_hue_server_t *)timer_param->inst;
    mesh_model_evt_t evt;
#if 0
    HSL2RGB(server->msg_format->target_hsl_hue/65535.0,
            server->msg_format->target_hsl_saturation/65535.0,
            server->msg_format->target_hsl_lightness/65535.0,
            &target_light);

    RGB2HSL(&evt.params.model_hsl_set.target_hsl_hue,
            &evt.params.model_hsl_set.target_hsl_saturation,
            &evt.params.model_hsl_set.target_hsl_lightness,
            target_light);
#else
    if(server->delay_trans_timer->remain_tick_count) {
        evt.params.model_hsl_set.target_hsl_hue = (server->msg_format->target_hsl_hue + server->msg_format->present_hsl_hue) / 2;
    }else {
        evt.params.model_hsl_set.target_hsl_hue = server->msg_format->target_hsl_hue;
    }
#endif

    evt.type.hsl_type = LIGHT_HSL_HUE_SET;

    if(server->cb)
        server->cb(&evt);

    if(server->delay_trans_timer->remain_tick_count == 0)
        server->server_state = GENERIC_TRANS_IDALE;

    LOG(3, "%s target_state:%x %x %x target:%x %x %x server_state:%x \n", __func__,
            server->msg_format->target_hsl_hue, server->msg_format->target_hsl_saturation, server->msg_format->target_hsl_lightness,
            evt.params.model_hsl_set.target_hsl_hue, evt.params.model_hsl_set.target_hsl_saturation, evt.params.model_hsl_set.target_hsl_lightness, server->server_state);
}
static void light_hsl_hue_server_action_then_publish(void *param)
{
    LOG(3, "light_hsl_hue_server_action_then_publish\n");
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    light_hsl_hue_server_t *server = (light_hsl_hue_server_t *)timer_param->inst;

    light_hsl_hue_server_action(param);
    
    if((server->delay_trans_timer->remain_tick_count == 0)) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}

static void light_hsl_hue_status_tx_done(void *pdu,uint8_t status)
{
}

void send_light_hsl_hue_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_hsl_hue_status_t msg1;
    light_hsl_hue_default_status_t msg2;
    access_pdu_tx_t * ptr;
    light_hsl_hue_server_t *server = GET_SERVER_MODEL_PTR(light_hsl_hue_server_t, model);
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_HSL_TWO_OCTETS_OPCODE_OFFSET, Light_HSL_Hue_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    if(server->server_state == GENERIC_TRANS_PROCESS) {
        uint32_t delay_trans_expiry = 0;
    tx_param.pdu_length = sizeof(light_hsl_hue_status_t);
        msg1.present_hsl_hue = server->msg_format->present_hsl_hue;
        msg1.target_hsl_hue = server->msg_format->target_hsl_hue;
        delay_trans_expiry = generic_get_delay_trans_expiry(
                server->delay_trans_timer,
                server->delay_trans_timer->trans_time,
                server->delay_trans_timer->trans_timer_step);
        msg1.remaining_time = model_transition_time_encode(delay_trans_expiry);
        ptr = access_model_pkt_build_fill(&tx_param,light_hsl_hue_status_tx_done,(uint8_t *)&msg1);

    }else {
        tx_param.pdu_length = sizeof(light_hsl_hue_default_status_t);
        msg2.present_hsl_hue = server->msg_format->present_hsl_hue;
        ptr = access_model_pkt_build_fill(&tx_param,light_hsl_hue_status_tx_done,(uint8_t *)&msg2);

    }
    BX_ASSERT(ptr);
    access_send(ptr);
}

static int light_hsl_hue_set_handle(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu, uint8_t *publish_status)
{
    uint16_t payload_size;

    generic_valid_field_check_t msg_field;// store src ,dst ,tid
    uint8_t * access = access_get_pdu_payload(pdu);
    payload_size = get_access_pdu_rx_payload_size(pdu);
    msg_field.dst = access_get_pdu_dst_addr(pdu);
    msg_field.src = access_get_pdu_src_addr(pdu);

    light_hsl_hue_server_t *server = GET_SERVER_MODEL_PTR(light_hsl_hue_server_t, model);

    server->tid_queue.inst_param.inst = (void *)server;
    server->delay_trans_timer->inst = server->tid_queue.inst_param.inst;
    LOG(3,"light_hsl_hue_set_handle payload_size:%d dst:%x src:%x\n", payload_size, msg_field.dst, msg_field.src);
    if(payload_size == sizeof(light_hsl_hue_set_t)){
        light_hsl_hue_set_t *p_pdu = (light_hsl_hue_set_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -1;
        }

        *publish_status = 1;
        server->server_state = GENERIC_TRANS_PROCESS;

        if(p_pdu->hsl_hue < server->msg_format->hue_range_min)
            p_pdu->hsl_hue = server->msg_format->hue_range_min;
        if(p_pdu->hsl_hue > server->msg_format->hue_range_max)
            p_pdu->hsl_hue = server->msg_format->hue_range_max;

        server->msg_format->target_hsl_hue = p_pdu->hsl_hue;

        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }

        LOG(3,"tid:%x trans_time:%x delay:%x server->cb:%p\n", p_pdu->tid, p_pdu->trans_time, p_pdu->delay, server->cb);

        if(p_pdu->delay == 0 && p_pdu->trans_time == 0){//this block code is used to transition state
            light_hsl_hue_server_action(server->delay_trans_timer);
        }else {
            server->delay_trans_timer->trans_time = p_pdu->trans_time;
            server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_LIGHT_HSL_HUE;

            generic_deal_delay_trans_func(server->delay_trans_timer, p_pdu->delay, p_pdu->trans_time, light_hsl_hue_server_action_then_publish);
        }

    }else if(payload_size == sizeof(light_hsl_hue_set_default_t)) {
        light_hsl_hue_set_default_t *p_pdu = (light_hsl_hue_set_default_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -2;
        }

        if(p_pdu->hsl_hue < server->msg_format->hue_range_min)
            p_pdu->hsl_hue = server->msg_format->hue_range_min;
        if(p_pdu->hsl_hue > server->msg_format->hue_range_max)
            p_pdu->hsl_hue = server->msg_format->hue_range_max;

        *publish_status = 1;
        server->server_state = GENERIC_TRANS_PROCESS;
        server->msg_format->target_hsl_hue = p_pdu->hsl_hue;


        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }
        server->delay_trans_timer->trans_time = user_generic_default_transition_time_get(NULL);
        server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_LIGHT_HSL_HUE;

        if(server->delay_trans_timer->trans_time == 0)
            light_hsl_hue_server_action(server->delay_trans_timer);
        else
            generic_deal_delay_trans_func(server->delay_trans_timer, 0, server->delay_trans_timer->trans_time, light_hsl_hue_server_action_then_publish);
    }

    return 0;
}

void light_HSL_hue_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    send_light_hsl_hue_status(elmt, model, pdu);
}
void light_HSL_hue_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;
    if(light_hsl_hue_set_handle(elmt, model, pdu, &publish_status) == 0)
        send_light_hsl_hue_status(elmt, model, pdu);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}
void light_HSL_hue_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;

    light_hsl_hue_set_handle(elmt, model, pdu, &publish_status);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}
