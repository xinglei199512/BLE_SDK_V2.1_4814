/**
 ****************************************************************************************
 *
 * @file   light_hsl_server.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-27 18:52
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

#include "light_hsl_server.h"
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
static void light_hsl_publish_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"light_hsl_publish_status_tx_done\n");
}
void light_hsl_status_publish(light_hsl_server_t *server)
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
    LOG(3,"light_hsl_status_publish lightness:%x dst_addr:%x\n", server->msg_format->present_hsl_lightness, dst_addr);
    light_hsl_default_status_t msg;
    msg.hsl_lightness = server->msg_format->present_hsl_lightness;
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_HSL_TWO_OCTETS_OPCODE_OFFSET, Light_HSL_Status);
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = sizeof(light_hsl_default_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_hsl_publish_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}
float Hue2RGB(float v1, float v2, float vH)
{
    if (vH < 0) vH += 1;
    if (vH > 1) vH -= 1;
    if (6.0 * vH < 1) return v1 + (v2 - v1) * 6.0 * vH;
    if (2.0 * vH < 1) return v2;
    if (3.0 * vH < 2) return v1 + (v2 - v1) * ((2.0 / 3.0) - vH) * 6.0;
    return (v1);
}

void HSL2RGB(float H,float S,float L, light_RGB *light)
{
    float var_1, var_2;
    if (S == 0)
    {
        light->R = L * 0xffff;
        light->G = L * 0xffff;
        light->B = L * 0xffff;
    }
    else
    {
        if (L < 0.5)
            var_2 = L * (1 + S);
        else
            var_2 = (L + S) - (S * L);

        var_1 = 2.0 * L - var_2;

        light->R = 0xffff * Hue2RGB(var_1, var_2, H + (1.0 / 3.0));
        light->G = 0xffff * Hue2RGB(var_1, var_2, H);
        light->B = 0xffff * Hue2RGB(var_1, var_2, H - (1.0 / 3.0));
    }
}
void RGB2HSL(uint16_t *Hue, uint16_t *Saturation, uint16_t *Lightness, light_RGB light)
{
    float R, G, B, H, S, L, Max, Min, del_R, del_G, del_B, del_Max;
    R = light.R / 65535.0;       //Where RGB values = 0 ÷ 255
    G = light.G / 65535.0;
    B = light.B / 65535.0;

    Min = MIN(R, MIN(G, B));    //Min. value of RGB
    Max = MAX(R, MAX(G, B));    //Max. value of RGB
    del_Max = Max - Min;        //Delta RGB value

    L = (Max + Min) / 2.0;

    if (del_Max == 0)           //This is a gray, no chroma...
    {
        //*H = 2.0/3.0;          //when S is 0，H always is 160（2/3*240）
        H = 0;                  //HSL results = 0 ÷ 1
        S = 0;
    }
    else                        //Chromatic data...
    {
        if (L < 0.5) S = del_Max / (Max + Min);
        else         S = del_Max / (2 - Max - Min);

        del_R = (((Max - R) / 6.0) + (del_Max / 2.0)) / del_Max;
        del_G = (((Max - G) / 6.0) + (del_Max / 2.0)) / del_Max;
        del_B = (((Max - B) / 6.0) + (del_Max / 2.0)) / del_Max;

        if      (R == Max) H = del_B - del_G;
        else if (G == Max) H = (1.0 / 3.0) + del_R - del_B;
        else if (B == Max) H = (2.0 / 3.0) + del_G - del_R;

        if (H < 0)  H += 1;
        if (H > 1)  H -= 1;
    }
    *Hue = (uint16_t) (H * 65535);
    *Saturation = (uint16_t) (S * 65535);
    *Lightness = (uint16_t) (L * 65535);
    LOG(3, "%s  HSL %d %d %d\n", __func__, *Hue, *Saturation, *Lightness);
}

static void light_hsl_get_set_target_light_value(float present, float target, float *target_value, int hsl_change_count, int hsl_count)
{
    float target_value_tmp;
    if(present > target) {
        target_value_tmp = (present - target) * ((hsl_change_count * 1.0) / hsl_count);
        *target_value = present - target_value_tmp;
    }else {
        target_value_tmp = (target - present) * ((hsl_change_count * 1.0) / hsl_count);
        *target_value = present + target_value_tmp;
    }
    LOG(3, "light_hsl_get_set_target_light_value:%x %x %x %x %x\n", 
            (uint16_t)(present), (uint16_t)(target), (uint16_t)(*target_value), hsl_change_count, hsl_count);
}
static void light_hsl_server_action(void *param)
{
    LOG(3, "light_hsl_server_action\n");
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    light_hsl_server_t *server = (light_hsl_server_t *)timer_param->inst;
    light_RGB target_light;
    light_RGB present_light;
    light_RGB target;
    mesh_model_evt_t evt;
    int hsl_change_count = 0, hsl_count = 0;
    uint8_t first_enter_flag = 0;
    uint16_t trans_timer_tmp;

    if(server->delay_trans_timer->remain_tick_count) {
        first_enter_flag = (pdMS_TO_TICKS(model_transition_time_decode(server->delay_trans_timer->trans_time, &trans_timer_tmp))
                                    == (server->delay_trans_timer->remain_tick_count + pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step)));

        hsl_count = pdMS_TO_TICKS(model_transition_time_decode(server->delay_trans_timer->trans_time, &trans_timer_tmp)) / pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step);

        hsl_change_count = hsl_count - server->delay_trans_timer->remain_tick_count / pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step);
        if(first_enter_flag == 1) {
            HSL2RGB(server->msg_format->present_hsl_hue/65535.0,
                    server->msg_format->present_hsl_saturation/65535.0,
                    server->msg_format->present_hsl_lightness/65535.0,
                    &server->msg_format->origin_light);
        }
        memcpy(&present_light, &server->msg_format->origin_light, sizeof(light_RGB));
        HSL2RGB(server->msg_format->target_hsl_hue/65535.0,
                server->msg_format->target_hsl_saturation/65535.0,
                server->msg_format->target_hsl_lightness/65535.0,
                &target_light);

        light_hsl_get_set_target_light_value(present_light.R, target_light.R, &target.R, hsl_change_count, hsl_count);
        light_hsl_get_set_target_light_value(present_light.G, target_light.G, &target.G, hsl_change_count, hsl_count);
        light_hsl_get_set_target_light_value(present_light.B, target_light.B, &target.B, hsl_change_count, hsl_count);
#if 0
        RGB2HSL(&evt.params.model_hsl_set.target_hsl_hue,
                &evt.params.model_hsl_set.target_hsl_saturation,
                &evt.params.model_hsl_set.target_hsl_lightness,
                target);
#else
        evt.params.model_hsl_set.target_hsl_hue = (server->msg_format->target_hsl_hue + server->msg_format->present_hsl_hue) / 2;
        evt.params.model_hsl_set.target_hsl_saturation = (server->msg_format->target_hsl_saturation + server->msg_format->present_hsl_saturation) / 2;
        evt.params.model_hsl_set.target_hsl_lightness = (server->msg_format->target_hsl_lightness + server->msg_format->present_hsl_lightness) / 2;
#endif
    }else {
        evt.params.model_hsl_set.target_hsl_hue = server->msg_format->target_hsl_hue;
        evt.params.model_hsl_set.target_hsl_saturation = server->msg_format->target_hsl_saturation;
        evt.params.model_hsl_set.target_hsl_lightness = server->msg_format->target_hsl_lightness;
        hsl_change_count = hsl_count = 1;
    }

    evt.type.hsl_type = LIGHT_HSL_SET;

    if(server->cb)
        server->cb(&evt);

    if(hsl_change_count == hsl_count && hsl_change_count == 1)
        server->server_state = GENERIC_TRANS_IDALE;
    LOG(3, "target_value:%x %x %x present_state:%x %x %x target_state:%x %x %x target:%x %x %x server_state:%x tick_count:%d\n", 
            (uint16_t)(target.R), (uint16_t)(target.G), (uint16_t)(target.B), 
            (uint16_t)(present_light.R), (uint16_t)(present_light.G), (uint16_t)(present_light.B), 
            server->msg_format->target_hsl_hue, server->msg_format->target_hsl_saturation, server->msg_format->target_hsl_lightness,
            evt.params.model_hsl_set.target_hsl_hue, evt.params.model_hsl_set.target_hsl_saturation, evt.params.model_hsl_set.target_hsl_lightness, server->server_state, server->delay_trans_timer->remain_tick_count);
}

static void light_hsl_server_action_then_publish(void *param)
{
    LOG(3, "light_hsl_server_action_then_publish\n");
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    light_hsl_server_t *server = (light_hsl_server_t *)timer_param->inst;

    light_hsl_server_action(param);

    if((server->delay_trans_timer->remain_tick_count == 0 && timer_param->dt_timer_flag == USE_FOR_TRANS_TIME)) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}

static void light_hsl_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "%s\n", __func__);
}

void send_light_hsl_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu, uint8_t present)
{
    light_hsl_status_t msg;
    uint32_t delay_trans_expiry = 0;
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    light_hsl_server_t *server = GET_SERVER_MODEL_PTR(light_hsl_server_t, model);

    if(server->server_state == GENERIC_TRANS_PROCESS) {
        tx_param.pdu_length = sizeof(light_hsl_status_t);
    }else {
        tx_param.pdu_length = sizeof(light_hsl_default_status_t);
    }

    if(present) {
        msg.hsl_lightness = server->msg_format->present_hsl_lightness;
        msg.hsl_hue = server->msg_format->present_hsl_hue;
        msg.hsl_saturation = server->msg_format->present_hsl_saturation;
        tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_HSL_TWO_OCTETS_OPCODE_OFFSET, Light_HSL_Status);
    }else {
        msg.hsl_lightness = server->msg_format->target_hsl_lightness;
        msg.hsl_hue = server->msg_format->target_hsl_hue;
        msg.hsl_saturation = server->msg_format->target_hsl_saturation;
        tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_HSL_TWO_OCTETS_OPCODE_OFFSET, Light_HSL_Target_Status);
    }

    delay_trans_expiry = generic_get_delay_trans_expiry(
            server->delay_trans_timer,
            server->delay_trans_timer->trans_time,
            server->delay_trans_timer->trans_timer_step);
    msg.remaining_time = model_transition_time_encode(delay_trans_expiry);

    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_hsl_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}

static int light_hsl_set_handle(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu, uint8_t *publish_status)
{
    uint16_t payload_size;

    generic_valid_field_check_t msg_field;// store src ,dst ,tid
    uint8_t * access = access_get_pdu_payload(pdu);
    payload_size = get_access_pdu_rx_payload_size(pdu);
    msg_field.dst = access_get_pdu_dst_addr(pdu);
    msg_field.src = access_get_pdu_src_addr(pdu);

    light_hsl_server_t *server = GET_SERVER_MODEL_PTR(light_hsl_server_t, model);

    server->tid_queue.inst_param.inst = (void *)server;
    server->delay_trans_timer->inst = server->tid_queue.inst_param.inst;
    LOG(3,"light_hsl_set_handle payload_size:%d dst:%x src:%x\n", payload_size, msg_field.dst, msg_field.src);
    if(payload_size == sizeof(light_hsl_set_t)){
        light_hsl_set_t *p_pdu = (light_hsl_set_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            LOG(3,"light_hsl_set_handle recv same tid:%x\n", msg_field.tid);
            return -1;
        }

        *publish_status = 1;
        server->server_state = GENERIC_TRANS_PROCESS;
        server->msg_format->target_hsl_hue = p_pdu->hsl_hue;
        server->msg_format->target_hsl_lightness = p_pdu->hsl_lightness;
        server->msg_format->target_hsl_saturation = p_pdu->hsl_saturation;

        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }

        LOG(3,"tid:%x trans_time:%x delay:%x server->cb:%p\n", p_pdu->tid, p_pdu->trans_time, p_pdu->delay, server->cb);

        if(p_pdu->delay == 0 && p_pdu->trans_time == 0){//this block code is used to transition state
            light_hsl_server_action(server->delay_trans_timer);
        }else {
            server->delay_trans_timer->trans_time = p_pdu->trans_time;
            server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_LIGHT_HSL;

            generic_deal_delay_trans_func(server->delay_trans_timer, p_pdu->delay, p_pdu->trans_time, light_hsl_server_action_then_publish);
        }

    }else if(payload_size == sizeof(light_hsl_set_default_t)) {
        light_hsl_set_default_t *p_pdu = (light_hsl_set_default_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -2;
        }

        *publish_status = 1;
        server->server_state = GENERIC_TRANS_PROCESS;
        server->msg_format->target_hsl_hue = p_pdu->hsl_hue;
        server->msg_format->target_hsl_lightness = p_pdu->hsl_lightness;
        server->msg_format->target_hsl_saturation = p_pdu->hsl_saturation;


        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }

        server->delay_trans_timer->trans_time = user_generic_default_transition_time_get(NULL);
        server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_LIGHT_HSL;

        if(server->delay_trans_timer->trans_time == 0)
            light_hsl_server_action(server->delay_trans_timer);
        else
            generic_deal_delay_trans_func(server->delay_trans_timer, 0, server->delay_trans_timer->trans_time, light_hsl_server_action_then_publish);
    }

    return 0;
}

void light_HSL_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    if(mesh_node_get_primary_element_addr() == access_get_pdu_src_addr(pdu))
        return;
    send_light_hsl_status(elmt, model, pdu, 1);
}

void light_HSL_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;

    if(light_hsl_set_handle(elmt, model, pdu, &publish_status) == 0)
        send_light_hsl_status(elmt, model, pdu, 1);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}

void light_HSL_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;

    light_hsl_set_handle(elmt, model, pdu, &publish_status);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}

void light_HSL_target_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    send_light_hsl_status(elmt, model, pdu, 0);
}
