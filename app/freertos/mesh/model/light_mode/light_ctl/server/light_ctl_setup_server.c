/**
 ****************************************************************************************
 *
 * @file   light_ctl_setup_server.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-27 18:43
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

#include "light_ctl_setup_server.h"
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

static void light_ctl_setup_server_action(light_ctl_setup_server_t *server)
{
#if 0    
    mesh_model_evt_t evt;
    evt.type.level_type = LEVEL_MODEL_EVT_SET;
    evt.params.level_set.target_level = server->msg_format->target_level;
    LOG(3,"level_server_action target_state:%x cb:%p\n", server->msg_format->target_level, server->cb);
    if(server->cb)
        server->cb(&evt);
#endif

    server->server_state = GENERIC_TRANS_IDALE;
}

static void light_ctl_setup_status_tx_done(void *pdu,uint8_t status)
{
}
void send_light_ctl_setup_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_ctl_default_default_status_t msg;

    light_ctl_setup_server_t *server = GET_SERVER_MODEL_PTR(light_ctl_setup_server_t, model);
    msg.ctl_lightness = server->msg_format->default_ctl_lightness;
    msg.ctl_temperature = server->msg_format->default_ctl_temperature;
    msg.ctl_delta_uv = server->msg_format->default_ctl_delta_uv;

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_CTL_TWO_OCTETS_OPCODE_OFFSET, Light_CTL_Default_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(light_ctl_default_default_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_ctl_setup_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

static int light_ctl_default_set_handle(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_ctl_setup_server_t *server = GET_SERVER_MODEL_PTR(light_ctl_setup_server_t, model);
    uint8_t * access = access_get_pdu_payload(pdu);

    light_ctl_default_set_t *p_pdu = (light_ctl_default_set_t *)(access + 2);
    if(p_pdu->ctl_temperature < T_MIN || p_pdu->ctl_temperature > T_MAX)
        return -1;

    server->server_state = GENERIC_TRANS_PROCESS;
    server->msg_format->default_ctl_lightness = p_pdu->ctl_lightness;
    server->msg_format->default_ctl_temperature = p_pdu->ctl_temperature;
    server->msg_format->default_ctl_delta_uv = p_pdu->ctl_delta_uv;

    light_ctl_setup_server_action(server);

    return 0;
}

void light_CTL_Default_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    send_light_ctl_setup_status(elmt, model, pdu);
}

void light_CTL_default_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    if(light_ctl_default_set_handle(elmt, model, pdu) == 0)
        send_light_ctl_setup_status(elmt, model, pdu);
}

void light_CTL_default_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_ctl_default_set_handle(elmt, model, pdu);
}

static void light_ctl_temperature_range_server_action(light_ctl_setup_server_t *server)
{
#if 0    
    mesh_model_evt_t evt;
    evt.type.level_type = LEVEL_MODEL_EVT_SET;
    evt.params.level_set.target_level = server->msg_format->target_level;
    LOG(3,"level_server_action target_state:%x cb:%p\n", server->msg_format->target_level, server->cb);
    if(server->cb)
        server->cb(&evt);
#endif
    server->server_state = GENERIC_TRANS_IDALE;
}

static int light_ctl_temperature_range_set_handle(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_ctl_setup_server_t *server = GET_SERVER_MODEL_PTR(light_ctl_setup_server_t, model);
    uint8_t * access = access_get_pdu_payload(pdu);
    uint16_t range_min, range_max;

    light_ctl_temperature_range_set_t *p_pdu = (light_ctl_temperature_range_set_t *)(access + 2);


    server->server_state = GENERIC_TRANS_PROCESS;
    range_min = p_pdu->range_min;
    range_max = p_pdu->range_max;

    if(range_min > range_max)
        return -1;
#if 0
    if(range_max > T_MAX || range_min < T_MIN)
        return -2;
#endif

    server->msg_format->range_min = range_min;
    server->msg_format->range_max = range_max;

    light_ctl_temperature_range_server_action(server);

    return 0;
}
static void light_ctl_temperature_range_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "light_ctl_temperature_range_status_tx_done\n");
}

void send_light_ctl_temperature_range_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_ctl_temperature_range_status_t msg;

    light_ctl_setup_server_t *server = GET_SERVER_MODEL_PTR(light_ctl_setup_server_t, model);

    msg.status_code = server->msg_format->status_code;
    msg.range_min = server->msg_format->range_min;
    msg.range_max = server->msg_format->range_max;
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_CTL_TWO_OCTETS_OPCODE_OFFSET, Light_CTL_Temperature_Range_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(light_ctl_temperature_range_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_ctl_temperature_range_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

void light_CTL_temperature_range_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    send_light_ctl_temperature_range_status(elmt, model, pdu);
}
void light_CTL_temperature_range_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    if(light_ctl_temperature_range_set_handle(elmt, model, pdu) == 0)
        send_light_ctl_temperature_range_status(elmt, model, pdu);
}
void light_CTL_temperature_range_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_ctl_temperature_range_set_handle(elmt, model, pdu);
}
