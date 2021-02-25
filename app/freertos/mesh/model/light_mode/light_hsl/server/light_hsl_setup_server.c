/**
 ****************************************************************************************
 *
 * @file   light_hsl_setup_server.c
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

#include "light_hsl_setup_server.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"
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
static void light_hsl_default_server_action(void *param)
{
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    light_hsl_setup_server_t *server = (light_hsl_setup_server_t *)timer_param->inst;
#if 0    
    mesh_model_evt_t evt;

    evt.type.level_type = LEVEL_MODEL_EVT_SET;
    evt.params.level_set.target_level = server->msg_format.target_level;
    LOG(3,"level_server_action target_state:%x cb:%p\n", server->msg_format.target_level, server->cb);
    if(server->cb)
        server->cb(&evt);
#endif

    server->server_state = GENERIC_TRANS_IDALE;
}


static void light_hsl_default_set_handle(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_hsl_setup_server_t *server = GET_SERVER_MODEL_PTR(light_hsl_setup_server_t, model);
    uint8_t * access = access_get_pdu_payload(pdu);

    server->delay_trans_timer->inst = server->tid_queue.inst_param.inst;

    light_hsl_default_set_t *p_pdu = (light_hsl_default_set_t *)(access + 2);

    if(p_pdu->hsl_hue < server->msg_format->hue_range_min)
        p_pdu->hsl_hue = server->msg_format->hue_range_min;
    else if(p_pdu->hsl_hue > server->msg_format->hue_range_max)
        p_pdu->hsl_hue = server->msg_format->hue_range_max;

    if(p_pdu->hsl_saturation < server->msg_format->saturation_range_min)
        p_pdu->hsl_saturation = server->msg_format->saturation_range_min;
    else if(p_pdu->hsl_saturation > server->msg_format->saturation_range_max)
        p_pdu->hsl_saturation = server->msg_format->saturation_range_max;


    server->server_state = GENERIC_TRANS_PROCESS;
    server->msg_format->default_hsl_lightness = p_pdu->hsl_lightness;
    server->msg_format->default_hsl_hue = p_pdu->hsl_hue;
    server->msg_format->default_hsl_saturation = p_pdu->hsl_saturation;

    light_hsl_default_server_action(server->delay_trans_timer);
}
static void light_hsl_default_status_tx_done(void *pdu,uint8_t status)
{
}

void send_light_hsl_default_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_hsl_default_status_t msg;
    light_hsl_setup_server_t *server = GET_SERVER_MODEL_PTR(light_hsl_setup_server_t, model);
    msg.hsl_lightness = server->msg_format->default_hsl_lightness;
    msg.hsl_hue = server->msg_format->default_hsl_hue;
    msg.hsl_saturation = server->msg_format->default_hsl_saturation;

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_HSL_TWO_OCTETS_OPCODE_OFFSET, Light_HSL_Default_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(light_hsl_default_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_hsl_default_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

void light_HSL_default_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    send_light_hsl_default_status(elmt, model, pdu);
}
void light_HSL_Default_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_hsl_default_set_handle(elmt, model, pdu);
    send_light_hsl_default_status(elmt, model, pdu);
}
void light_HSL_Default_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_hsl_default_set_handle(elmt, model, pdu);
}

static void light_hsl_range_server_action(void *param)
{
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    light_hsl_setup_server_t *server = (light_hsl_setup_server_t *)timer_param->inst;
#if 0    
    mesh_model_evt_t evt;
    evt.type.level_type = LEVEL_MODEL_EVT_SET;
    evt.params.level_set.target_level = server->msg_format.target_level;
    LOG(3,"level_server_action target_state:%x cb:%p\n", server->msg_format.target_level, server->cb);
    if(server->cb)
        server->cb(&evt);
#endif

    server->server_state = GENERIC_TRANS_IDALE;
}


static int light_hsl_range_set_handle(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_hsl_setup_server_t *server = GET_SERVER_MODEL_PTR(light_hsl_setup_server_t, model);
    uint8_t * access = access_get_pdu_payload(pdu);

    server->delay_trans_timer->inst = server->tid_queue.inst_param.inst;

    light_hsl_range_set_t *p_pdu = (light_hsl_range_set_t *)(access + 2);

    if((p_pdu->hue_range_min > p_pdu->hue_range_max) || (p_pdu->saturation_range_min > p_pdu->saturation_range_max))
        return -1;

    server->server_state = GENERIC_TRANS_PROCESS;
    server->msg_format->hue_range_min = p_pdu->hue_range_min;
    server->msg_format->hue_range_max = p_pdu->hue_range_max;
    server->msg_format->saturation_range_min = p_pdu->saturation_range_min;
    server->msg_format->saturation_range_max = p_pdu->saturation_range_max;

    LOG(3, "light_hsl_range_set_handle:%x %x %x %x\n",
            server->msg_format->hue_range_min, server->msg_format->hue_range_max, server->msg_format->saturation_range_min, server->msg_format->saturation_range_max);
    light_hsl_range_server_action(&server->delay_trans_timer);

    return 0;
}
static void light_hsl_range_status_tx_done(void *pdu,uint8_t status)
{
}

void send_light_hsl_range_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_hsl_range_status_t msg;
    light_hsl_setup_server_t *server = GET_SERVER_MODEL_PTR(light_hsl_setup_server_t, model);
    LOG(3, "send_light_hsl_range_status:%x %x %x %x\n",
            server->msg_format->hue_range_min, server->msg_format->hue_range_max, server->msg_format->saturation_range_min, server->msg_format->saturation_range_max);

    msg.status_code = 0x00;
    msg.hue_range_min = server->msg_format->hue_range_min;
    msg.hue_range_max = server->msg_format->hue_range_max;
    msg.saturation_range_min = server->msg_format->saturation_range_min;
    msg.saturation_range_max = server->msg_format->saturation_range_max;

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_HSL_TWO_OCTETS_OPCODE_OFFSET, Light_HSL_Range_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(light_hsl_range_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_hsl_range_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}
void light_HSL_Range_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    send_light_hsl_range_status(elmt, model, pdu);
}
void light_HSL_Range_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    if(light_hsl_range_set_handle(elmt, model, pdu) == 0)
        send_light_hsl_range_status(elmt, model, pdu);
}
void light_HSL_Range_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_hsl_range_set_handle(elmt, model, pdu);
}
